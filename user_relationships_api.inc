<?php
// $Id: user_relationships_api.inc,v 1.21 2007-12-22 22:24:17 sprsquish Exp $

/**
 *  User Relationships API
 */

/**
 * Public API for retrieving a specific relationship
 *
 * @param $param
 *    The rtid or an associative array of attributes to search for in selecting the
 *    relationship, such as rtid or name. Attributes must match column names
 *    in the user_relationships table.
 *
 * @return
 *    object of the requested relationship type
 *
 */
function user_relationships_type_load($param = array()) {
  $types = user_relationships_types_load();

  if (is_numeric($param)) {
    return $types[$param];
  }

  foreach ($types as $type) {
    $found = TRUE;

    foreach($param as $column => $value) {
      $column = strtolower($column);

      if ($column == 'name' || $column == 'plural_name') {
        $value = strtolower($value);
      }

      // mismatch, move to the next type
      if ($type->$column != $value) {
        $found = FALSE;
        break;
      }
    }

    if ($found) {
      return $type;
    }
  }
}


/**
 * Public API for loading the full list of relationship types
 *
 * @return
 *    array of relationship_type objects
 */
function user_relationships_types_load() {
  static $relationship_types_list = array();

  if (!sizeof($relationship_types_list)) {
    $results = db_query("SELECT * FROM {user_relationship_types}");  
    while ($relationship = db_fetch_object($results)) {
      _user_relationships_invoke('load', $relationship, 'type');
      $relationship_types_list[$relationship->rtid] = $relationship;
    }
  }

  return $relationship_types_list;
}


/**
 * Public API for creating a relationship.
 *
 * @param $requester
 *   object or ID of the requester
 * @param $requestee
 *   object  or ID of the requestee
 * @param $type
 *   object or ID of the relationship type
 * @param $approved
 *    boolean status of the relationship
 * 
 * @return
 *    object of the newly created relationship
 */
function user_relationships_request_relationship($requester, $requestee, $type, $approved = FALSE) {
  // translate an ID into an object
  foreach (array('requester' => $requester, 'requestee' => $requestee, 'type' => $type) as $key => $value) {
    if (!is_object($value)) {
      $$key = $key == 'type' ? user_relationships_type_load($value) : user_load(array('uid' => $value));
    }
  }

  if (!variable_get('user_relationships_allow_multiple', TRUE)) {
    if (user_relationships_load(array('user' => $requester->uid, TRUE) || user_relationships_load(array('user' => $requestee->uid, TRUE)))) {
      return t('Users are not allowed to have multiple relationships');
    }
  }

  if (user_relationships_load(array('between' => array($requester->uid, $requestee->uid), 'rtid' => $type->rtid), TRUE)) {
    return t('This relationship already exists');
  }

  $user_auto_approve = is_array($requestee->user_relationships_auto_approve) && $requestee->user_relationships_auto_approve[$type->rtid];
  if ($user_auto_approve || !$type->requires_approval) {
    $approved = TRUE;
  }

  $relationship = (object)array(
    'requester_id'  => $requester->uid,
    'requestee_id'  => $requestee->uid,
    'approved'      => ($approved ? 1 : 0),
    'rtid'          => $type->rtid,
  );

  return _user_relationships_save_relationship($relationship, 'request');
}


/**
 * Public API for updating a relationship.
 *
 * @param $relationship
 *   object of the current relationship
 * @param $updated_relationship
 *   object of the updated relationship
 *
 * @return
 *    object of the updated relationship
 */
function user_relationships_update_relationship(&$current_relationship, &$updated_relationship) {
  // ensure data that shouldn't change doesn't
  $updated_relationship->rid        = $current_relationship->rid;
  $updated_relationship->created_at = $current_relationship->created_at;

  _user_relationships_save_relationship($updated_relationship, 'update');

  return $updated_relationship;
}


/**
 * Public API for deleting a relationship.
 *
 * @param $relationship
 *    object of the relationship
 * @param $deleted_by
 *    object of the user that initiated the delete command
 * @param $op
 *    string reason for removal ('cancel','disapprove','remove')
 */
function user_relationships_delete_relationship(&$relationship, &$deleted_by, $op = 'remove') {
  $relationship->deleted_by = $deleted_by;
  db_query("DELETE FROM {user_relationships} WHERE rid = %d", $relationship->rid);

  _user_relationships_invoke('delete', $relationship, $op);
}


/**
  * Load relationship objects from the database.
  *
  * @param $param
  *   an array of parameters with the key being the column. There are two special keys:
  *     1) array("between" => array($uid1, $uid2)) will return all relationships between the two user ids.
  *     2) array("user" => $uid) will return all relationships for the specified uid
  *
  *   arguments will process operators as well using the syntax: array(col => '> {value}'). 
  *     example: show all relationships created in 2007 
  *       $start_time = mktime(0,0,0,0,0,2007);
  *       $end_time = mktime(0,0,0,0,0,2008);
  *       user_relationships_load(array('created_at' => ">= {$start_time}", 'created_at' => '< {$end_time'}));
  * @param $count
  *   a boolean stating whether or not the return value should be the number of relationships found
  * @param $sort
  *   a string containing a valid column name which will become the key for the returned array of relationships
  * @param $order
  *   a string containing SQL stating the column and direction of the sort (ex. "requester_id ASC, rtid DESC")
  * @param $limit 
  *   a string containing SQL stating the limit (ex "10" or "10, 5")
  *
  * @return
  *   an array of relationships
  *   if the key is "rid" the array will be a single dimention: array($rid => $relationship, $rid => $relationship)
  *   otherwise it'll be multidimentional: array($rtid => array($relationship, $relationship))
  *
  *   each relationship will have the user's name, mail, and data attached as requester_name, requester_mail, requester_data
  *   or requestee_name, requestee_mail, requestee_data
 */
function user_relationships_load($param = array(), $count = FALSE, $sort = 'rid', $order = NULL, $limit = NULL, $include_user_info = FALSE) {
  static $relationships = array();

  $arguments = array();
  if (is_numeric($param)) {
    if (isset($relationships[$param])) {
      return is_object($relationships[$param]) ? drupal_clone($relationships[$param]) : $relationships[$param];
    }

    $param = array('rid' => $param);
  }

  $query = _user_relationships_generate_query($param, $order, $limit, $include_user_info);

  $results = db_query($query[($count ? 'count' : 'query')], $query['arguments']);

  if ($count) {
    return (int)db_result($results);
  }

  $relationships = array();
  while ($relationship = db_fetch_object($results)) {
    if ($sort == 'rid') {
      $relationships[$relationship->$sort] = $relationship;
    }
    else {
      $relationships[$relationship->$sort][] = $relationship;
    }
  }

  return $relationships;
}


/**
 * Public API for getting the set or default message
 *
 * @param $key
 *    string message name
 * @param $replacements
 *    array replaceable tokens to append or replace default tokens
 *
 * @return
 *    string formatted message
 */
function user_relationships_get_message($key, $relationship = NULL, $replacements = array()) {
  $msg = variable_get("user_relationships_msg_{$key}", NULL);
  if (is_null($msg)) { 
    $messages = user_relationships_default_messages();
    $msg      = _user_relationships_get_from_array($key, $messages);
  }

  if ($relationship) {
    if (!$relationship->requester) { $relationship->requester = user_load(array('uid' => $relationship->requester_id)); }
    if (!$relationship->requestee) { $relationship->requestee = user_load(array('uid' => $relationship->requestee_id)); }
    if (!($relationship->name || $relationship->type)) { $relationship->type = user_relationships_type_load($relationship->rtid); }

    $replaceables = array(
      '!requester'                => theme('username', $relationship->requester),
      '!requestee'                => theme('username', $relationship->requestee),
      '%relationship_name'        => $relationship->name ? $relationship->name : $relationship->type->name,
      '%relationship_plural_name' => $relationship->plural_name ? $relationship->plural_name : $relationship->type->plural_name,
    );
    $replacements = array_merge($replaceables, $replacements);
  }

  return t($msg, $replacements);
}
