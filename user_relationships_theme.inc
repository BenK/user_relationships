<?php
// $Id: user_relationships_theme.inc,v 1.15 2008-02-01 02:29:50 sprsquish Exp $

/**
 * User Relationships theme-able functions
 */

/**
 * Categorized list of relationships for a given user
 */
function theme_user_relationships_page($account = NULL, $rtid = NULL) {
  // Check that the uid is valid, not the anonymous user, and the user exists
  if (!$account) {
    global $user;
    $account = $user;
  }

  $args = array('user' => $account->uid, 'approved' => TRUE);
  if (isset($rtid) && is_numeric($rtid)) {
    $relationship_type = user_relationships_type_load($rtid);
    $args['rtid'] = $rtid;
  }

  // To Page or not to Page
  $query = _user_relationships_generate_query($args);

  if (db_result(db_query($query['count'], $query['arguments']))) {
    if ($relationships_per_page = variable_get('user_relationships_relationships_per_page', 16)) {
      $result = pager_query($query['query'], $relationships_per_page, 0, $query['count'], $query['arguments']);
    }
    else {
      $result = db_query($query['query'], $query['arguments']);
    }

    $edit_access = ($user->uid == $account->uid && user_access('maintain relationships')) || user_access('administer users');
    $online_interval = time() - variable_get('user_block_seconds_online', 180);

    while ($relation = db_fetch_object($result)) {
      $this_user = $account->uid == $relation->requestee_id ? 'requester_id' : 'requestee_id';
      $this_user = user_load($relation->$this_user);

      $rows[] = array(
        theme('username', $this_user),
        $relation->name,
        $this_user->access > $online_interval ? t('online') : t('not online'),
        $edit_access ? theme('user_relationships_remove_link', $account->uid, $relation->rid) : '&nbsp;',
      );
    }

    $output .= 
      theme('table', array(), $rows) .
      theme('pager', NULL, $relationships_per_page);
  }
  else {
    $output .= t('No relationships found');
  }

  $msg = $relationship_type ? "%username's %relationships" : "All %username's relationships";
    
  drupal_set_title(t($msg, array(
    '%username'       => $account->name, 
    '%relationships'  => $relationship_type->plural_name ? $relationship_type->plural_name : $relationship_type->name
  )));

  return $output;
}

/**
 * List of pending requests from other users
 */
function theme_user_relationships_pending_requests_page($account = NULL) {
  if (!$account) {
    global $user;
    $account = $user;
  }

  drupal_set_title(t("%username's pending relationships", array('%username' => $account->name)));

  $relationships_per_page = variable_get('user_relationships_relationships_per_page', 16);

  foreach (array(
    array(0, 'requester_id', t('Sent Requests')),
    array(1, 'requestee_id', t('Received Requests')),
  ) as $list_args) {

    $query = _user_relationships_generate_query(array($list_args[1] => $account->uid, 'approved' => FALSE));

    if (db_result(db_query($query['count'], $query['arguments']))) {
      if ($relationships_per_page = variable_get('user_relationships_relationships_per_page', 16)) {
        $result = pager_query($query['query'], $relationships_per_page, $list_args[0], $query['count'], $query['arguments']);
      }
      else {
        $result = db_query($query['query'], $query['arguments']);
      }

      $rows[] = array(array('data' => $list_args[2], 'header' => true, 'colspan' => 2));

      while ($relationship = db_fetch_object($result)) {
        if ($list_args[1] == 'requester_id') {
          $links  = theme('user_relationships_pending_request_cancel_link', $account->uid, $relationship->rid);
          
        }
        else {
          $links =
          	theme('user_relationships_pending_request_approve_link',    $account->uid, $relationship->rid) . ' | ' .
          	theme('user_relationships_pending_request_disapprove_link', $account->uid, $relationship->rid);
        }
        $related    = user_load($relationship->requester_id == $account->uid ? $relationship->requestee_id : $relationship->requester_id);
        $rows[]     = array(theme('username', $related) . ' is a ' . $relationship->name, $links);
      }

      $output .= theme('table', array(), $rows);

      if ($relationships_per_page) {
        $output .= theme('pager', NULL, $relationships_per_page, $list_args[0]);
      }

      unset($rows);
    }
  }

  if ($output == '') {
    $output = t('No pending relationships found');
  }

  return $output;
}

/**
 * Create relationship link
 */
function theme_user_relationships_request_relationship_link($relate_to) {
  return l(
    t('Create a relationship with %name', array('%name' => $relate_to->name)), 
    "relationship/{$relate_to->uid}/request", 
    array(
      'query' => drupal_get_destination(),
      'html'  => TRUE,
    )
  );
}

/**
 * Remove relationship link
 */
function theme_user_relationships_remove_link($uid, $rid) {
  return l(
    t('Remove'), 
    "user/{$uid}/relationships/{$rid}/remove", 
    array(
      'title' => array('title' => t('Remove')),
      'query' => drupal_get_destination(),
    )
  );
}

/**
 * Approve to pending relationship link
 */
function theme_user_relationships_pending_request_approve_link($uid, $rid) {
  return l(
    t('Approve'), 
    "user/{$uid}/relationships/requested/{$rid}/approve", 
    array(
      'title' => array('title' => t('Approve')),
      'query' => drupal_get_destination(),
    )
  );
}

/**
 * Disapprove to pending relationship link
 */
function theme_user_relationships_pending_request_disapprove_link($uid, $rid) {
  return l(
    t('Disapprove'), 
    "user/{$uid}/relationships/requested/{$rid}/disapprove", 
    array(
      'title' => array('title' => t('Disapprove')),
      'query' => drupal_get_destination(),
    )
  );
}

/**
 * Cancel request link
 */
function theme_user_relationships_pending_request_cancel_link($uid, $rid) {
  return l(
    t('Cancel'), 
    "user/{$uid}/relationships/requested/{$rid}/cancel", 
    array(
      'title' => array('title' => t('Disapprove')),
      'query' => drupal_get_destination(),
    )
  );
}

/**
 * Show the current status of a relationship
 */
function theme_user_relationships_approval_status($approved) {
  return isset($approved) ? ($approved ? t('Approved') : t('Not Approved')) : t('Unknown');
}

/**
 * Link to user's profile
 */
function theme_user_relationships_user_link($uid) {
  return url("user/$uid", array('absolute' => TRUE)); 
}
