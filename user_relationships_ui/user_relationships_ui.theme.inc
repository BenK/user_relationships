<?php
// $Id: user_relationships_ui.theme.inc,v 1.1.2.4 2008-04-21 00:38:19 sprsquish Exp $

/**
 * User Relationships theme-able functions
 */

/**
 * Show the current status of a relationship
 */
function theme_user_relationships_approval_status($approved) {
  return isset($approved) ? ($approved ? t('Approved') : t('Not Approved')) : t('Unknown');
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
 * Link to user's profile
 */
function theme_user_relationships_user_link($uid) {
  return url("user/$uid", array('absolute' => TRUE));
}
