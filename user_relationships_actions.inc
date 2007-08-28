<?php
// $Id: user_relationships_actions.inc,v 1.2 2007-08-28 01:48:02 gwen Exp $

/**
 * User Relationships Actions
 */


/**
 * Validate the submitted relationship type
 */
function user_relationships_type_edit_validate($form_id, &$form_values) {
  if (arg(3) == 'add' && db_result(db_query("SELECT name FROM {user_relationship_types} WHERE LOWER(`name`) = LOWER('%s')", $form_values['name']))) {
    form_set_error('name', t('%name has already been used.', array('%name' => $form_values['name'])));
  }
}


/**
 * Process the relationship edit page form submission.
 */
function user_relationships_type_edit_submit($form_id, &$form_values) {
  if (arg(3) == 'add') {
    $form_values['rtid'] = db_next_id('{user_relationship_types}_id');
    db_query(
      "INSERT INTO {user_relationship_types} (rtid, name, is_oneway, requires_approval) VALUES (%d, '%s', %d, %d)", 
      $form_values['rtid'],
      $form_values['name'],
      $form_values['is_oneway'],
      $form_values['requires_approval']
    );

    $relationship_type = user_relationships_relationship_type_load(array('rtid' => $form_values['rtid']));
    _user_relationships_invoke('insert type', $relationship_type);

    drupal_set_message(t('Relationship %name has been added.', array('%name' => $form_values['name'])));
    watchdog('userreltnships', t('User Relationships form: relationship %name added.', array('%name' => $form_values['name'])), WATCHDOG_NOTICE, l(t('view'), 'admin/user/user_relationship/types'));
  }
  else {
    db_query(
      "UPDATE {user_relationship_types} SET name = '%s', is_oneway = %d, requires_approval = %d WHERE rtid = %d", 
      $form_values['name'], 
      $form_values['is_oneway'], 
      $form_values['requires_approval'], 
      $form_values['relationship_type']->rtid
    );

    _user_relationships_invoke('update type', $form_values['relationship_type']);

    drupal_set_message(t('Relationship %name has been updated.', array('%name' => $form_values['name'])));
    watchdog('userreltnships', t('User Relationships form: relationship %name updated.', array('%name' => $form_values['name'])), WATCHDOG_NOTICE, l(t('view'), 'admin/user/user_relationship/types'));
  }

  return 'admin/user/relationships';
}


/**
 * Process relationship delete form submission.
 */
function user_relationships_type_delete_submit($form_id, &$form_values) {
  $rtid = arg(4);

  db_query("DELETE FROM {user_relationship_types} WHERE rtid = %d", $rtid);
  db_query("DELETE FROM {user_relationships} WHERE rtid = %d", $rtid);

  _user_relationships_invoke('delete type', $form_values['relationship_type']);

  drupal_set_message(t('Relationship %name has been deleted.', array('%name' => $form_values['name'])));
  watchdog('userreltnships', t('User Relationships form: relationship %name deleted.', array('%name' => $form_values['name'])), WATCHDOG_NOTICE);

  return 'admin/user/relationships';
}


/**
 * Validate relationship request.
 */
function user_relationships_request_validate($form_id, &$form_values) {
  $requester = $form_values['requester'];
  $requestee = $form_values['requestee'];

  if (user_access('can have relationship', $requestee)) {
    $current_relationships = _user_relationships_current_relationships($requester, $requestee);

    if (!db_result(db_query("SELECT COUNT(*) FROM {user_relationship_types}" . ($current_relationships ? " WHERE rtid NOT IN (%s) ORDER BY name" : ''), implode(',', array_keys($current_relationships))))) {
      drupal_set_message(t('You already relate to this user in every possible way.'), 'error');
      drupal_goto();
    }
    else if ($current_relationships[$form_values['rtid']]) {
      if ($current_relationships[$form_values['rtid']]->approved) {
        $message = 'You are already a %relationship_name of %username';
      }
      else {
        $message = 'You have already requested to be a %relationship_name of %username';
      }
      form_set_error('rtid', t($message, array(
        '%username'           => $requestee->name,
        '%relationship_name'  => $$current_relationships[$form_values['rtid']]->name
      )));
    }
  }
  else {
    drupal_set_message(t('This user is not accepting relationships.'), 'error');
    drupal_goto();
  }
}

/**
 * Process relationship request.
 */
function user_relationships_request_submit($form_id, &$form_values) {
  $requester          = $form_values['requester'];
  $requestee          = $form_values['requestee'];
  $relationship_type  = user_relationships_relationship_type_load(array('rtid' => $form_values['rtid']));
  
  user_relationships_request_relationship($requester, $requestee, $relationship_type);

  drupal_set_message(t('Your request to be a %relationship_name of %username has been submitted.', array(
    '%relationship_name'  => $relationship_type->name,
    '%username'           => $requestee->name,
  )));

  return 'user';
}


/**
 * Approve, Disapprove, or Cancel a relationship request
 */
function user_relationships_pending_requested_submit($form_id, &$form_values) {
  global $user;

  $relationship = $form_values['relationship'];
  $requester    = $relationship->requester;
  $requestee    = $relationship->requestee;

  $viewing_own_account = ($user->uid == $requestee->uid);

  switch ($form_values['action']) {
  case 'approve':
    $updated = $relationship;
    $updated->approved = TRUE;

    user_relationships_update_relationship($relationship, $updated);
    drupal_set_message(t("!Person !is_or_are now a %relationship_name of !requestee.", array(
      '!Person'             => ($viewing_own_account ? t('You') : theme('username', $requester)),
      '!is_or_are'          => ($viewing_own_account ? t('are') : t('is')),
      '%relationship_name'  => $relationship->name,
      '!requestee'          => theme('username', $requester),
    )));
    break;

  case 'cancel':
  case 'disapprove':
    user_relationships_delete_relationship($relationship, $user, $form_values['action']);
    drupal_set_message(t("!requester's request to be a %relationship_name of !requestee has been removed.", array(
      '!requester'          => theme('username', $requester),
      '%relationship_name'  => $relationship->name,
      '!requestee'          => theme('username', $requestee),
    )));
    break;

  default:
    drupal_set_message(t('No relationship action has been taken.'));
  }

  return 'relationships/'. $form_values['viewed_id'] .'/requests';
}


/**
 * Remove a relationship
 */
function user_relationships_remove_submit($form_id, &$form_values) {
  global $user;

  $relationship = $form_values['relationship'];

  user_relationships_delete_relationship($relationship, $user);
  drupal_set_message(t("!Person is no longer a %relationship_name of !requestee.", array(
    '!Person'             => $relationship->requester->name,
    '%relationship_name'  => $relationship->name,
    '!requestee'          => ($relationship->requestee->uid == $user->uid ? t("yours") : $relationship->requestee->name)
  )));

  return 'relationships/'. $form_values['viewed_id'];
}
