<?php
// $Id: user_relationships_ui.actions.inc,v 1.1.2.2 2008-03-02 11:46:13 sprsquish Exp $

/**
 * User Relationships Actions
 */


/**
 * Validate relationship request.
 */
function user_relationships_ui_request_validate($form, &$form_state) {
  $requester = user_load($form_state['values']['requester']);
  $requestee = user_load($form_state['values']['requestee']);

  if (user_access('can have relationships', $requestee)) {
    $current_relationships = user_relationships_load(array('between' => array($requester->uid, $requestee->uid)));

    if (!db_result(db_query(
      "SELECT COUNT(*) FROM {user_relationship_types}" . ($current_relationships ? " WHERE rtid NOT IN (%s) ORDER BY name" : ''), 
      implode(',', array_keys($current_relationships))
    ))) {
      drupal_set_message(user_relationships_ui_get_message('too_many_relationships'), 'error');
      drupal_goto();
    }
    else if ($current_relationships[$form_values['rtid']]) {
      $message_name = $current_relationships[$form_values['rtid']]->approved ? 'exisitng_relationship' : 'existing_request';
      form_set_error('rtid', user_relationships_ui_get_message($message_name, array(
        '%requester'                => $requester->name,
        '%requestee'                => $requestee->name,
        '%relationship_name'        => $$current_relationships[$form_values['rtid']]->name,
        '%relationship_plural_name' => $$current_relationships[$form_values['rtid']]->plural_name
      )));
    }
  }
  else {
    drupal_set_message(user_relationships_ui_get_message('not_accepting_requests'));
    drupal_goto();
  }
}

/**
 * Process relationship request.
 */
function user_relationships_ui_request_submit($form, &$form_state) {
  $requester          = user_load($form_state['values']['requester']);
  $requestee          = user_load($form_state['values']['requestee']);
  $relationship_type  = user_relationships_type_load($form_state['values']['rtid']);

  $relationship = user_relationships_request_relationship($requester, $requestee, $relationship_type);

  if ($relationship === FALSE) {
    drupal_set_message(user_relationships_ui_get_message('unknown_error', $relationship));
  }

  $relationship->requester = $requester;
  $relationship->requestee = $requestee;
  $relationship->type = $relationship_type;

  drupal_set_message(user_relationships_ui_get_message(($relationship->approved ? 'pre_approved' : 'submitted'), $relationship));

  $form_state['redirect'] = $_GET['destination'];
}


/**
 * Approve, Disapprove, or Cancel a relationship request
 */
function user_relationships_ui_pending_requested_submit($form, &$form_state) {
  global $user;

  $relationship = user_relationships_load($form_state['values']['rid']);
  $requester    = $relationship->requester;
  $requestee    = $relationship->requestee;

  $viewing_own_account = ($user->uid == $requestee->uid);

  switch ($form_state['values']['action']) {
  case 'approve':
    $relationship->approved = TRUE;
    user_relationships_save_relationship($relationship, $form_state['values']['action']);
    drupal_set_message(user_relationships_ui_get_message('accepted', $relationship));
    break;

  case 'cancel':
  case 'disapprove':
    user_relationships_delete_relationship($relationship, $user, $form_state['values']['action']);
    drupal_set_message(user_relationships_ui_get_message($form_state['values']['action'], $relationship));
    break;

  case 'ban':
    
    break;

  default:
    drupal_set_message(user_relationships_ui_get_message('default'));
  }

  $form_state['redirect'] = str_replace('destination=', '', drupal_get_destination());
}


/**
 * Remove a relationship
 */
function user_relationships_ui_remove_submit($form, &$form_state) {
  global $user;

  $relationship = user_relationships_load($form_state['values']['rid']);

  user_relationships_delete_relationship($relationship, $user);
  drupal_set_message(user_relationships_ui_get_message('removed', $relationship));

  return "user/{$form_values['viewed_id']}/relationships";
}
