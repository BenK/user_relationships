<?php
// $Id: user_relationships_api.inc,v 1.18 2007-10-30 20:00:16 jbchristy Exp $

/**
 *  User Relationships API
 */

/**
 * Public API for retrieving a specific relationship
 *
 * @param $array
 *    An associative array of attributes to search for in selecting the
 *    relationship, such as rtid or name. Attributes must match column names
 *    in the user_relationships table.
 *
 * @return
 *    object of the requested relationship type
 *
 */
function user_relationships_relationship_type_load($criteria = array()) {
  if (!sizeof($criteria)) {
    return NULL;
  }

  $types = user_relationships_relationship_types_load();
  foreach ($types as $type) {
    $found = TRUE; // assume this is the type we're looking for

    foreach($criteria as $column => $value) {
      $column = strtolower($column);
      if ($column == 'name' || $column == 'plural_name') {
        $value = strtolower($value);
      }

      if ($type->$column != $value) {
        $found=FALSE; // value doesn't match, so this isn't the type after all
      }
    }
    if ($found) {
      return $type;
    }
  }
  return NULL;
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
      $relationship_types_list[$relationship->rtid] = $relationship;
    }
  }

  return $relationship_types_list;
}

/**
 * Public API for loading all the relationships for a specific user
 *
 * @param $uid
 *    integer user id
 *
 * @return
 *    array of relationships
 */
function user_relationships_load_all_for_user($uid) {
  static $relationships = array();
  $cache_throttle = variable_get('user_relationships_cache_throttle', 5000); // max # of relationships to cache

  if (!key_exists($uid, $relationships)) {
    $relationships[$uid] = array();
    if (($cache = cache_get("user_relationships_relationships_{$uid}", 'cache_user_relationships')) && !empty($cache->data)) {
      $relationships[$uid] = unserialize($cache->data);
    }
    else {
      $results = db_query(
         'SELECT r.*, rt.name, rt.plural_name, rt.is_oneway FROM {user_relationships} r, {user_relationship_types} rt
          WHERE (requester_id = %d OR requestee_id = %d)
            AND r.rtid = rt.rtid',
        $uid, $uid
      );
      while ($relationship = db_fetch_object($results)) {
        $relationships[$uid][] = $relationship;
      }
      if ($cache_throttle >= 0 && sizeof($relationships[$uid]) < $cache_throttle) {
        cache_set("user_relationships_relationships_{$uid}", 'cache_user_relationships', serialize($relationships[$uid]));
      }
    }
  }

  return $relationships[$uid];
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
    if (($cache = cache_get("user_relationships_relationship_{$rid}", 'cache_user_relationships')) && !empty($cache->data)) {
      $user_relationships[$rid] = unserialize($cache->data);
    }
    elseif ($relationship = db_fetch_object(db_query("SELECT * FROM {user_relationships} WHERE rid = %d", $rid))) {
      $relationship->requester    = user_load(array('uid' => $relationship->requester_id));
      $relationship->requestee    = user_load(array('uid' => $relationship->requestee_id));
      $relationship->type         = user_relationships_relationship_type_load(array('rtid' => $relationship->rtid));
      $relationship->name         = $relationship->type->name;
      $relationship->plural_name  = $relationship->type->plural_name;

      _user_relationships_invoke('load', $relationship);
      $user_relationships[$rid] = $relationship;
      cache_set("user_relationships_relationship_{$rid}", 'cache_user_relationships', serialize($relationship));
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
    if (user_relationships_count_relationships(array('uid1' => $requester->uid, 'uid2' => $requestee->uid)) > 0) {
      return t('Users are not allowed to have multiple relationships');
    }
  }

  $existing = db_result(db_query(
    " SELECT 1 
      FROM {user_relationships} ur, {user_relationship_types} urt
      WHERE ur.rtid = %d
        AND ur.rtid = urt.rtid
        AND ((requester_id = %d AND requestee_id = %d)
          OR (is_oneway = 0 AND requestee_id = %d AND requester_id = %d)
        )", 
    $relationship_type->rtid, 
    $requester->uid, $requestee->uid, 
    $requester->uid, $requestee->uid
  ));
  if ($existing) {
    return t('This relationship already exists');
  }

  $user_auto_approve = is_array($requestee->user_relationships_auto_approve) && $requestee->user_relationships_auto_approve[$relationship_type->rtid];
  if ($user_auto_approve || !$relationship_type->requires_approval) {
    $approved = TRUE;
  }

  $rid = db_next_id('{user_relationships}_id');

  db_query(
    "INSERT INTO {user_relationships} (rid, requester_id, requestee_id, rtid, approved, created_at)
     VALUES (%d, %d, %d, %d, %d, NOW())",
    $rid,
    $requester->uid, 
    $requestee->uid, 
    $relationship_type->rtid,
    ($approved ? 1 : 0)
  );
  cache_clear_all("user_relationships_relationships_{$requester->uid}", 'cache_user_relationships');
  cache_clear_all("user_relationships_relationships_{$requestee->uid}", 'cache_user_relationships');

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
function user_relationships_update_relationship(&$current_relationship, &$updated_relationship) {
  // ensure data that shouldn't change doesn't
  $updated_relationship->rid        = $current_relationship->rid;
  $updated_relationship->created_at = $current_relationship->created_at;

  // call any hooks that might want to know about updating this relationship
  _user_relationships_invoke('update', $updated_relationship);

  // update the db
  db_query("UPDATE {user_relationships}
               SET requester_id=%d,
                   requestee_id=%d,
                   rtid=%d,
                   approved=%d
             WHERE rid=%d",
    $updated_relationship->requester_id,
    $updated_relationship->requestee_id,
    $updated_relationship->rtid,
   ($updated_relationship->approved ? 1 : 0),
    $current_relationship->rid
  );
  // clear the cache of both (all) users' list of all relationships
  // requester and requestee should be the same in the old and new relationships, but better safe than sorry
  cache_clear_all("user_relationships_relationships_{$current_relationship->requester->uid}", 'cache_user_relationships');
  cache_clear_all("user_relationships_relationships_{$current_relationship->requestee->uid}", 'cache_user_relationships');
  cache_clear_all("user_relationships_relationships_{$updated_relationship->requester->uid}", 'cache_user_relationships');
  cache_clear_all("user_relationships_relationships_{$updated_relationship->requestee->uid}", 'cache_user_relationships');

  return $updated_relationship;
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
  cache_clear_all("user_relationships_relationship_{$relationship->rid}", 'cache_user_relationships');

  _user_relationships_invoke('delete', $relationship, $op);
  cache_clear_all("user_relationships_relationships_{$relationship->requester_id}", 'cache_user_relationships');
  cache_clear_all("user_relationships_relationships_{$relationship->requestee_id}", 'cache_user_relationships');
}

/**
 * Public API for counting relationships that match specified criteria
 *
 * @param $criteria
 *    An associative array of attributes to search for. Keys in the array should match
 *    column names in the user_relationships table. However, as a convenience, you can
 *    also specify the following keys:
 *      'typename': match the rtid of the type with the given name (no join)
 *      'uid', 'uid1', 'uid2': match either the requester_id or the requestee_id
 *      Examples:
 *        ('uid'=>1, 'approved'=>0) matches the pending relationships user 1 has either
 *          sent or received
 *        ('uid1'=>10, 'uid2'=>20) matches all relationships between users 10 and 20
 *
 *    The values for keys uid, uid1, uid2, rid, requester_id, requestee_id and rtid can be
 *    either scalars or arrays. If the values are arrays, the function will match
 *    relationships where the indicated column matches any of the entries in the array.
 *      Examples:
 *        ('uid1'=>10, uid2=>array(1,2,3,4)) matches all relationships between user 10 and
 *          users 1, 2, 3 or 4.
 *        ('uid'=>10, rtid=>array(1,2)) matches the relationships user 10 has of type 1 or 2
 *
 *    For the dates (created_at and updated_at), the first character of the value should
 *    be an operator, i.e. '<', '=' or '>'. The remainder should be the value, which can
 *    be either a (numeric) Unix timestamp, or a string formatted to match MySQL's
 *    date/timestamp format.
 *    e.g. 'created_at' => '>2006-12-31 23:59:59' to find relationships added in 2007 or later
 *
 * @return
 *    count of the relationships that match the criteria
 *
 */
function user_relationships_count_relationships($criteria = array()) {
  return _user_relationships_select_relationships($criteria, TRUE);
}

/**
 * Public API for loading relationships that match specified criteria
 *
 * @param $criteria
 *    An associative array of attributes to search for. Keys in the array should match
 *    See the comments for user_relationships_count_relationships() above for a description
 *    of the criteria.
 *
 * @return
 *    An array of the relationships that match the criteria
 *
 */
function user_relationships_load_relationships($criteria = array()) {
  return _user_relationships_select_relationships($criteria, FALSE);
}

/**
 * Public API for loading relationships that match specified criteria, indexed by relatee
 *
 * @param $criteria
 *    An associative array of attributes to search for. Keys in the array should match
 *    See the comments for user_relationships_count_relationships() above for a description
 *    of the criteria.
 *
 * @param $relater
 *    The uid of the user whose relatees you want to identify
 *
 * @return
 *    An array indexed by $relatee, where $relatee is whichever of (requester_id, requestee_id)
 *    doesn't match $relater. Each element of the returned array is an array of relationships
 *    between $relater and $relatee which match the criteria. If some relationships match the
 *    criteria, but don't involve $relater, they're returned in the array indexed by $relatee = -1.
 *
 */
function user_relationships_load_relationships_by_relatee($criteria = array(), $relater = -1) {
  return _user_relationships_select_relationships($criteria, FALSE, $relater);
}

/**
 * helper function that does the actual work of counting/loading relationships based on criteria
 */
function _user_relationships_select_relationships($criteria = array(), $count_only=FALSE, $relater=FALSE) {
  if ($count_only) {
    $query = "SELECT COUNT(rid) FROM {user_relationships} WHERE 1 ";
  }
  else {
    $query = "SELECT * FROM {user_relationships} WHERE 1 ";
  }
  $args  = array();

  foreach($criteria as $column => $value) {
    $column = strtolower($column);
    switch ($column) {
    case 'uid':
    case 'uid1':
    case 'uid2':
      if (is_array($value)) {
        $query  .= "AND (requester_id IN (%s) OR requestee_id IN (%s)) ";
        $args[]  = implode(', ', $value);
        $args[]  = implode(', ', $value);
      }
      else {
        $query  .= "AND (requester_id=%d OR requestee_id=%d) ";
        $args[]  = (int)$value;
        $args[]  = (int)$value;
      }
      break;
    case 'typename':
      if ($type = user_relationships_relationship_type_load(array('name' => $value))) {
        $query  .= "AND rtid=%d ";
        $args[]  = (int)$type->rtid;
      }
      else {
        return NULL; // if an invalid typename was specified, no relationships can match it
      }
      break;
    case 'rid':
    case 'requester_id':
    case 'requestee_id':
    case 'rtid':
      if (is_array($value)) {
        $query  .= "AND {$column} IN (%s) ";
        $args[]  = implode(', ', $value);
      }
      else {
        $query  .= "AND {$column}=%d ";
        $args[]  = (int)$value;
      }
      break;
    case 'approved':
      $query  .= "AND {$column}=%d ";
      $args[]  = (int)$value;
      break;
    case 'created_at':
    case 'updated_at':
      // for the dates, the first character of $value should be the operator, i.e. '<', '=' or '>'
      // the remainder should be the value: either a (numeric) Unix timestamp, or a string formatted
      //   to match MySQL's date/timestamp format
      //   e.g. 'created_at' => '>2006-12-31 23:59:59' to find relationships added in 2007 or later
      $operator = substr($value, 0, 1);
      $value    = substr($value, 1);
      if (is_numeric($value)) {
        $value = date('Y-m-d H:i:s', (int)$value);
      }
      $query  .= "AND {$column}{$operator}'%s' ";
      $args[]  = $value;
      break;
    default:
      // invalid criteria
      return NULL;
      break;
    } // switch ($column)
  } // foreach($criteria as $column => $value)

  // if all we want is a count, get it and return it
  if ($count_only) {
    return db_result(db_query($query, $args));
  }

  // load the relationships into an array
  // if $relater was not specified, load the relationships into a flat array with an arbitrary index
  // if $relater was specified, index the array by the user id of the user who relates to $relater: each
  //    element of the returned array is an array of relationships between $relater and $relatee; if
  //    a relationship matches the criteria, but doesn't involve $relater, use -1 as the $relatee
  $relationships = array();
  $results = db_query($query, $args);
  while ($relationship = db_fetch_object($results)) {
    if ($relater) {
      if ($relater == $relationship->requester_id) {
        $index = $relationship->requestee_id;
      }
      elseif ($relater == $relationship->requestee_id) {
        $index = $relationship->requester_id;
      }
      else {
        $index = -1; // don't conflict with any valid uid
      }
      $relationships[$index][] = $relationship;
    }
    else {
      $relationships[] = $relationship;
    }
  }
  return $relationships;
} // function _user_relationships_select_relationships($criteria = array(), $count_only=FALSE)


/**
 * Public API for getting the set or default message
 *
 * @param $key
 *    string message name
 *
 * @param $replacements
 *    array replaceable tokens
 *
 * @return
 *    string formatted message
 */
function user_relationships_get_message($key, $replacements = array()) {
  $msg = variable_get("user_relationships_msg_{$key}", NULL);
  if (is_null($msg)) { 
    $messages = user_relationships_default_messages();
    $msg      = _user_relationships_get_from_array($key, $messages);
  }
  return t($msg, $replacements);
}
