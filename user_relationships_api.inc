<?php
// $Id: user_relationships_api.inc,v 1.3 2007-08-28 16:33:58 sprsquish Exp $

/**
 *  User Relationships API
 */

/**
 * Public API for retrieving a specific relationship
 *
 * @param $array
 *    An associative array of attributes to search for in selecting the
 *    relationship, such as id or name.
 *
 * @return
 *    object of the requested relationship type
 *
 */
function user_relationships_relationship_type_load($array = array()) {
  static $user_relationship_types = array();
  $cache_key = implode('', array_keys($array) + $array);

  if (!$user_relationship_types[$cache_key]) {
    $query  = array();
    $params = array();

    foreach($array as $key => $value) {
      if ($key == 'rtid') {
        $query[] = "$key = %d";
        $params[] = $value;
      }
      else {
        $query[]= "LOWER($key) = LOWER('%s')";
        $params[] = $value;
      }
    }
    $relationship = db_fetch_object(db_query('SELECT * FROM {user_relationship_types} WHERE '. implode(' AND ', $query), $params));

    _user_relationships_invoke('load type', $relationship);

    $user_relationship_types[$cache_key] = $relationship;
  }

  return $user_relationship_types[$cache_key];
}

/**
 * Public API for loading the full list of relationship types
 *
 * @return
 *    array of relationship_type objects
 */
function user_relationships_relationship_types_load() {
  static $relationship_types_list = array();

  if (!sizeof($relationship_types_list)) {
    $results = db_query("SELECT * FROM {user_relationship_types}");  
    while ($relationship = db_fetch_object($results)) {
      _user_relationships_invoke('load type', $relationship);
      $relationship_types_list[] = $relationship;
    }
  }

  return $relationship_types_list;
}

/**
 * Public API for loading a relationship.
 *
 * @param $rid
 *    integer relationship id
 *
 * @return
 *    object of the requested relationship
 */
function user_relationships_relationship_load($rid) {
  // we may have to do this more than once per page view. so cache it
  static $user_relationships = array(); 

  if (!$user_relationships[$rid]) {
    if ($relationship = db_fetch_object(db_query("SELECT * FROM {user_relationships} WHERE rid = %d", $rid))) {
      $relationship->requester  = user_load(array('uid' => $relationship->requester_id));
      $relationship->requestee  = user_load(array('uid' => $relationship->requestee_id));
      $relationship->type       = user_relationships_relationship_type_load(array('rtid' => $relationship->rtid));
      $relationship->name       = $relationship->type->name;

      _user_relationships_invoke('load', $relationship);
      $user_relationships[$rid] = $relationship;
    }
  }

  return $user_relationships[$rid];
}

/**
 * Public API for creating a relationship.
 *
 * @param $requester
 *   object of the requester
 *
 * @param $requestee
 *   object of the requestee
 *
 * @param $relationship_type
 *   object of the relationship type
 *
 * @param $status
 *    string status of the relationship
 * 
 * @return
 *    object of the newly created relationship
 */
function user_relationships_request_relationship(&$requester, &$requestee, &$relationship_type, $approved = FALSE) {
  if (!variable_get('user_relationships_allow_multiple', TRUE)) {
    foreach ($requester->relationships as $relationship) {
      if ($relationship->requestee_id == $requestee->uid) {
        return t('Users are not allowed to have multiple relationships');
      }
    }
  }

  $existing = db_result(db_query(
    " SELECT 1 
      FROM {user_relationships} 
      WHERE rtid = %d
        AND ((requester_id = %d AND requestee_id = %d)
          OR (requestee_id = %d AND requester_id = %d)
        )
    ", $relationship_type->rtid, $requester->uid, $requestee->uid, $requester->uid, $requestee->uid
  ));
  if ($existing) {
    return t('This relationship already exists');
  }

  $user_auto_approve = is_array($requestee->user_relationships_auto_approve) && $requestee->user_relationships_auto_approve[$relationship->rid];
  if ($user_auto_approve || !$relationship_type->requires_approval) {
    $approved = TRUE;
  }

  $rid = db_next_id('{user_relationships}_id');

  db_query(
    "INSERT INTO {user_relationships} (rid, requester_id, requestee_id, rtid, approved) VALUES (%d, %d, %d, %d, %d)",
    $rid,
    $requester->uid, 
    $requestee->uid, 
    $relationship_type->rtid,
    ($approved ? 1 : 0)
  );

  $relationship = user_relationships_relationship_load($rid);
  _user_relationships_invoke('insert', $relationship);

  return $relationship;
}

/**
 * Public API for updating a relationship.
 *
 * @param $relationship
 *   object of the current relationship
 *
 * @param $updated_relationship
 *   object of the updated relationship
 * 
 * @return
 *    object of the updated relationship
 */
function user_relationships_update_relationship(&$relationship, &$updated_relationship) {
  _user_relationships_invoke('update', $updated_relationship);

  db_query("DELETE FROM {user_relationships} WHERE rid = %d", $relationship->rid);

  $updated_relationship->rid = db_next_id('{user_relationships}_id');
  db_query(
    "INSERT INTO {user_relationships} (rid, requester_id, requestee_id, rtid, approved) VALUES (%d, %d, %d, %d, %d)",
    $updated_relationship->rid,
    $updated_relationship->requester->uid, 
    $updated_relationship->requestee->uid, 
    $updated_relationship->type->rtid,
    ($relationship->approved ? 1 : 0)
  );

  return $update_relationship;
}

/**
 * Public API for deleting a relationship.
 *
 * @param $relationship
 *    object of the relationship
 * 
 * @param $deleted_by
 *    object of the user that initiated the delete command
 *
 * @param $op
 *    string reason for removal ('cancel','disapprove','remove')
 */
function user_relationships_delete_relationship(&$relationship, &$deleted_by, $op = 'remove') {
  $relationship->deleted_by = $deleted_by;
  db_query("DELETE FROM {user_relationships} WHERE rid = %d", $relationship->rid);
  if (!is_array($relationship->type->implied_by)) {
    die("<pre>".print_r($relationship, true)."</pre>");
  }

  _user_relationships_invoke('delete', $relationship, $op);
}
