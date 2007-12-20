<?php
// $Id: user_relationships_theme.inc,v 1.11 2007-12-20 17:28:57 sprsquish Exp $

/**
 * User Relationships theme-able functions
 */

/**
 * Categorized list of relationships for a given user
 */
function theme_user_relationships_page($uid = NULL, $rtid = NULL) {
  global $user;

  if (empty($uid) || $uid == $user->uid) {
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

  $args = array('user' => $viewed_user->uid);
  if (isset($rtid) && is_numeric($rtid)) {
    $relationship_type = user_relationships_type_load($rtid);
    $args['rtid'] = $rtid;
  }

  // To Page or not to Page
  $query = _user_relationships_generate_query($args);
  if ($relationships_per_page = variable_get('user_relationships_relationships_per_page', 16)) {
    $result = pager_query($query['query'], $relationships_per_page, 0, $query['count'], $query['arguments']);
  }
  else {
    $result = db_query($query['query'], $query['arguments']);
  }

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

    $output .= 
      theme('table', array(), $rows) .
      theme('pager', NULL, $relationships_per_page);
  }
  else {
    $output .= t('No relationships found');
  }

  $msg = $relationship_type ? "%username's %relationships" : "All %username's relationships";
    
  drupal_set_title(t($msg, array(
    '%username'       => $viewed_user->name, 
    '%relationships'  => $relationship_type->plural_name ? $relationship_type->plural_name : $relationship_type->name
  )));

  return $output;
}

/**
 * List of pending requests from other users
 */
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

  $relationships_per_page = variable_get('user_relationships_relationships_per_page', 16);

  foreach (array(
    array(0, 'requester_id', t('Sent Requests')),
    array(1, 'requestee_id', t('Received Requests')),
  ) as $list_args) {

    $query = _user_relationships_generate_query(array($list_args[1] => $account->uid, 'approved' => FALSE));
    if ($relationships_per_page = variable_get('user_relationships_relationships_per_page', 16)) {
      $result = pager_query($query['query'], $relationships_per_page, $list_args[0], $query['count'], $query['arguments']);
    }
    else {
      $result = db_query($query['query'], $query['arguments']);
    }

    if (db_num_rows($result)) {
      $rows[] = array(array('data' => $list_args[2], 'header' => true, 'colspan' => 2));

      while ($relationship = db_fetch_object($result)) {
        if ($list_args[1] == 'requester_id') {
          $links  = theme('user_relationships_pending_request_cancel_link', $account->uid, $relationship->rid);
          
        }
        else {
          $links =
          	theme('user_relationships_pending_request_approve_link',    $uid, $relationship->rid) . ' | ' .
          	theme('user_relationships_pending_request_disapprove_link', $uid, $relationship->rid);
        }
        $related    = user_load(array('uid' => ($relationship->requester_id == $account->uid ? $relationship->requestee_id : $relationship->requester_id)));
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
function theme_user_relationships_request_relationship_link(&$relate_to) {
  return l(t('Create a relationship with %name', array('%name' => $relate_to->name)), "relationship/request/{$relate_to->uid}", NULL, drupal_get_destination(), NULL, FALSE, TRUE);
}

/**
 * Remove relationship link
 */
function theme_user_relationships_remove_link($uid, &$relationship) {
  return l(t('Remove'), "user/{$uid}/relationship/remove/$relationship->rid", NULL, drupal_get_destination(), NULL, FALSE, TRUE);
}

/**
 * Approve to pending relationship link
 */
function theme_user_relationships_pending_request_approve_link($uid, $relationship) {
  return l(t('Approve'), "user/{$uid}/relationships/requested/approve/{$relationship}", array('title' => t('Approve')), drupal_get_destination(), NULL, FALSE, TRUE);
}

/**
 * Disapprove to pending relationship link
 */
function theme_user_relationships_pending_request_disapprove_link($uid, $relationship) {
  return l(t('Disapprove'), "user/{$uid}/relationships/requested/disapprove/{$relationship}", array('title' => t('Disapprove')), drupal_get_destination(), NULL, FALSE, TRUE);
}

/**
 * Cancel request link
 */
function theme_user_relationships_pending_request_cancel_link($uid, $relationship) {
  return l(t('Cancel'), "user/{$uid}/relationships/requested/cancel/{$relationship}", array('title' => t('Cancel')), drupal_get_destination(), NULL, FALSE, TRUE);
}

/**
 * Link to user page (use usernode if it's available)
 */
function theme_user_relationships_user_link($uid) {
  if (module_exists('usernode')) {
    $nid  = usernode_get_node_id($uid);
    $url  = "node/$nid";
  }
  else {
    $url = "user/$uid";
  }

  return url($url, NULL, NULL, TRUE); 
}


function theme_user_relationships_approval_status($approved) {
  return isset($approved) ? ($approved ? t('Approved') : t('Not Approved')) : t('Unknown');
}