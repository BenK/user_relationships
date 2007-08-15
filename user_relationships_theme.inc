<?php
// $Id: user_relationships_theme.inc,v 1.1 2007-08-15 20:20:01 sprsquish Exp $

/**
 * User Relationships theme-able functions
 */

/**
 * Categorized list of relationships for a given user
 */
function theme_user_relationships_page($uid = NULL) {
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
    $edit_access = ($user->uid == $uid && user_access('maintain relationships')) || user_access('administer users');
    $sorted = array();
    foreach($account->relationships as $relationship) {
      if ($relationship->approved) {
        $sorted[$relationship->name][] = user_relationships_relationship_load($relationship->rid);
      }
    }
    foreach ($sorted as $name => $relationships) {
      $rows[] = array(array('data' => $relationship->name, 'header' => true, 'colspan' => 2));
      foreach ($relationships as $relationship) {
        $relate_to = ($relationship->type->is_oneway || ($relationship->requester->uid == $account->uid) ? $relationship->requestee : $relationship->requester);
        $rows[] = array(
          theme('username', $relate_to),
          $edit_access ? theme('user_relationships_remove_link', $uid, $relationship) : '&nbsp;',
        );
      }
    }
    $output .= theme('table', NULL, $rows);
  }
  else {
    $output .= t('No relationships found');
  }

  drupal_set_title(t("%username's relationships", array('%username' => $account->name)));

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
