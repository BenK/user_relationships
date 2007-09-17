<?php
// $Id: user_relationships_theme.inc,v 1.6 2007-09-17 18:44:40 gwen Exp $

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
function theme_user_relationships_pending_requests_page($uid = NULL) {
  global $user;
  if (empty($uid)) {
    $uid = $user->uid;
  }

  // Check that the uid is valid, not the anonymous user, and the user exists
  if (!(is_numeric($uid) && ($uid > 0) && $account = user_load(array('uid' => $uid)))) {
    drupal_not_found();
    exit();
  }

  if (sizeof($account->relationships)) {
    $sorted = array();
    foreach($account->relationships as $relationship) {
      if (!$relationship->approved) {
        $sort = $relationship->requester_id == $uid ? t('Sent Requests') : t('Received Requests');
        $sorted[$sort][] = user_relationships_relationship_load($relationship->rid);
      }
    }
    foreach ($sorted as $name => $relationships) {
      $rows[] = array(array('data' => $name, 'header' => true, 'colspan' => 2));
      foreach ($relationships as $relationship) {        
        $relate_to = ($relationship->requester_id == $uid ? $relationship->requestee : $relationship->requester);

        if ($account->uid == $user->uid && $relationship->requester_id == $uid) {
          $links = theme('user_relationships_pending_request_cancel_link', $uid, $relationship->rid);
        }
        else {
          $links =
            theme('user_relationships_pending_request_approve_link', $uid, $relationship->rid) . ' | ' .
            theme('user_relationships_pending_request_disapprove_link', $uid, $relationship->rid);
        }
        
        $rows[] = array(
          theme('username', $relate_to) . ' is a ' . $relationship->name,
          $links
        );
      }
    }
  }

  if (sizeof($rows)) {
    $output .= theme('table', NULL, $rows);
  }
  else {
    $output .= t('No pending relationships found');
  }


  drupal_set_title(t("%username's pending relationships", array('%username' => $account->name)));

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
