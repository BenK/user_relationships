<?php
// $Id: user_relationships_theme.inc,v 1.7 2007-09-19 18:27:57 sprsquish Exp $

/**
 * User Relationships theme-able functions
 */

/**
 * Categorized list of relationships for a given user
 */
function theme_user_relationships_page($uid = NULL, $relationship = NULL) {
  global $user;

  if (empty($uid)) {
    $viewed_user =& $user;
  }
  else {
    $viewed_user = user_load(array('uid' => $uid));
  }

  // Check that the uid is valid, not the anonymous user, and the user exists
  if ($viewed_user->uid == 0) {
    drupal_not_found();
    exit();
  }

  $query = 
     "SELECT r.*, rt.name, rt.plural_name, rt.is_oneway 
      FROM {user_relationships} r, {user_relationship_types} rt 
      WHERE (requester_id = %d OR requestee_id = %d) 
        AND r.rtid = rt.rtid 
        AND r.approved = 1";
  $args = array($viewed_user->uid, $viewed_user->uid);

  if (isset($relationship->rtid)) {
    $query .= " AND r.rtid = %d";
    $args[] = $relationship->rtid;
  }

  $relationships_per_page = variable_get('user_relationships_relationships_per_page', 16);
  $result = $relationships_per_page ? pager_query($query, $relationships_per_page, 0, NULL, $args) : db_query($query, $args);

  if (db_num_rows($result)) {
    $edit_access = ($user->uid == $viewed_user->uid && user_access('maintain relationships')) || user_access('administer users');
    $online_interval = time() - variable_get('user_block_seconds_online', 180);

    while ($relation = db_fetch_object($result)) {
      $this_user = $viewed_user->uid == $relation->requestee_id ? 'requester_id' : 'requestee_id';
      $this_user = user_load(array('uid' => $relation->$this_user));

      $rows[] = array(
        theme('username', $this_user),
        $relation->name,
        $this_user->access > $online_interval ? t('online') : t('not online'),
        $edit_access ? theme('user_relationships_remove_link', $viewed_user->uid, $relation) : '&nbsp;',
      );
    }
    $output .= theme('table', array(), $rows);
  }
  else {
    $output .= t('No relationships found');
  }

  $output .= theme('pager', NULL, $relationships_per_page);

  drupal_set_title(t("%username's %relationships", array(
    '%username' => $viewed_user->name, 
    '%relationships' => $relationship->plural_name ? $relationship->plural_name : $relationship->name
  )));

  return $output;
}

/**
 * List of pending requests from other users
 */
define('RECEIVED_PAGER', 0);
define('SENT_PAGER', 1);

function theme_user_relationships_pending_requests_page($uid = NULL) {
  global $user;

  if (!$uid) {
    $account =& $user;
  }
  else if (!(is_numeric($uid) && ($uid > 0) && $account = user_load(array('uid' => $uid)))) {
    drupal_not_found();
    exit();
  }

  drupal_set_title(t("%username's pending relationships", array('%username' => $account->name)));

  $output = '';
  $relationships_per_page = variable_get('user_relationships_relationships_per_page', 16);
  $args = array($account->uid);

  /**
   * Sent Requests
   */
  $sql = 
    "SELECT u.*, r.*, rt.name as rname 
     FROM {user_relationships} r, {user_relationship_types} rt, {users} u
     WHERE  r.requester_id = %d 
       AND r.approved = 0 
       AND rt.rtid = r.rtid
       AND r.requestee_id = u.uid";

  $requests = $relationships_per_page ? pager_query($sql, $relationships_per_page, SENT_PAGER, NULL, $args) : db_query($sql, $args);

  if (db_num_rows($requests)) {
    $rows[] = array(array('data' => t('Sent Requests'), 'header' => true, 'colspan' => 2));

    while ($relationship = db_fetch_object($requests)) {
      $links     = theme('user_relationships_pending_request_cancel_link', $account->uid, $relationship->rid);
      $rows[]    = array(theme('username', $relationship) . ' is a ' . $relationship->rname, $links);
    }

    $output .= theme('table', array(), $rows);

    if ($relationships_per_page) {
      $output .= theme('pager', NULL, $relationships_per_page, SENT_PAGER);
    }

    unset($rows);
  }

  /**
   * Received Requests
   */
  $sql = 
    "SELECT u.*, r.*, rt.name as rname 
     FROM {user_relationships} r, {user_relationship_types} rt, {users} u
     WHERE  r.requestee_id = %d 
       AND r.approved = 0 
       AND rt.rtid = r.rtid
       AND r.requester_id = u.uid";

  $requests = $relationships_per_page ? pager_query($sql, $relationships_per_page, RECEIVED_PAGER, NULL, $args) : db_query($sql, $args);

  if (db_num_rows($requests)) {
    $rows[] = array(array('data' => t('Received Requests'), 'header' => true, 'colspan' => 2));

    while ($relationship = db_fetch_object($requests)) {
      $links =
      	theme('user_relationships_pending_request_approve_link',    $uid, $relationship->rid) . ' | ' .
      	theme('user_relationships_pending_request_disapprove_link', $uid, $relationship->rid);
      $rows[] = array(theme('username', $relationship) . ' is a ' . $relationship->rname, $links);
    }

    $output .= theme('table', array(), $rows);

    if ($relationships_per_page) {
      $output .= theme('pager', NULL, $relationships_per_page, RECEIVED_PAGER);
    }

    unset($rows);
  }

  if ($output == '') {
    $output = t('No pending relationships found');
  }

  return $output;
}

/**
 * Create relationship link
 */
function theme_user_relationships_request_relationship_link(&$relate_to) {
  return l(t('Create a relationship with %name', array('%name' => $relate_to->name)), 'relationship/request/' . $relate_to->uid, NULL, drupal_get_destination(), NULL, FALSE, TRUE);
}

/**
 * Remove relationship link
 */
function theme_user_relationships_remove_link($uid, &$relationship) {
  return l(t('Remove'), "relationship/{$uid}/remove/$relationship->rid", NULL, drupal_get_destination(), NULL, FALSE, TRUE);
}

/**
 * Approve to pending relationship link
 */
function theme_user_relationships_pending_request_approve_link($uid, $relationship) {
  return l(t('Approve'), "relationships/$uid/requested/approve/$relationship", array('title' => t('Approve')), drupal_get_destination(), NULL, FALSE, TRUE);
}

/**
 * Disapprove to pending relationship link
 */
function theme_user_relationships_pending_request_disapprove_link($uid, $relationship) {
  return l(t('Disapprove'), "relationships/$uid/requested/disapprove/$relationship", array('title' => t('Disapprove')), drupal_get_destination(), NULL, FALSE, TRUE);
}

/**
 * Cancel request link
 */
function theme_user_relationships_pending_request_cancel_link($uid, $relationship) {
  return l(t('Cancel'), "relationships/$uid/requested/cancel/$relationship", array('title' => t('Cancel')), drupal_get_destination(), NULL, FALSE, TRUE);
}

/**
 * Link to user page (use usernode if it's available)
 */
function theme_user_relationship_user_link($uid) {
  if (module_exists('usernode')) {
    $nid  = usernode_get_node_id($uid);
    $url  = "node/$nid";
  }
  else {
    $url = "user/$uid";
  }

  return url($url, NULL, NULL, TRUE); 
}
