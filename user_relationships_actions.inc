<?php
// $Id: user_relationships_actions.inc,v 1.20.2.1 2008-02-20 01:17:29 sprsquish Exp $

/**
 * User Relationships Actions
 */


/**
 * Validate the submitted relationship type
 */
function user_relationships_type_edit_validate($form, &$form_state) {
  if (
    (isset($form_state['values']['rtid']) && 
      db_result(db_query(
        "SELECT name FROM {user_relationship_types} 
         WHERE LOWER(`name`) = LOWER('%s') 
           AND rtid != %d", 
        $form_state['values']['name'], $form_state['values']['rtid'])
      )
    ) ||
    (!isset($form_state['values']['rtid']) && 
      db_result(db_query("SELECT name FROM {user_relationship_types} WHERE LOWER(`name`) = LOWER('%s')", $form_state['values']['name']))
    )
  ) {
    form_set_error('name', t('%name has already been used.', array('%name' => $form_state['values']['name'])));
  }

  $expire = $form_state['values']['expires_val'];
  if (!is_numeric($expire) || (int)$expire < 0) {
    form_set_error('expires_val', t('Expiration period must be an integer greater than or equal to 0.'));
  } 
}

/***
 * Generic settings validation. 
 */
function user_relationships_setting_validation($element, $validations) {
  foreach ($validations as $validation => $info) {
    $arguments = array($element['#value']);
    if (isset($info['valid_options'])) {
      $arguments[] = $info['valid_options'];
    }

    if (!call_user_func_array($validation, $arguments)) {
      form_set_error($element['#name'], $info['msg']);
    }
  }
}

/**
 * Process the relationship edit page form submission.
 */
function user_relationships_type_edit_submit($form, &$form_state) {
  $relationship_type = (object)$form_state['values'];
  // TODO: This should be taken care of in the form itself.. set $form_state['action']
  if (arg(3) == 'add') {
    drupal_write_record('user_relationship_types', $relationship_type);

    $form_state['values']['rtid'] = $relationship_type->rtid;
    $relationship_type = user_relationships_type_load($relationship_type->rtid);
    _user_relationships_invoke('insert', $relationship_type, 'type');

    drupal_set_message(t('Relationship %name has been added.', array('%name' => $relationship_type->name)));
    watchdog(
      'u_relationship', 
      t('User Relationships form: relationship %name added.', array('%name' => $relationship_type->name)), 
      WATCHDOG_NOTICE, 
      l(t('view'), 'admin/user/user_relationship/types')
    );
  }
  else {
    drupal_write_record('user_relationship_types', $relationship_type, 'rtid');

    _user_relationships_invoke('update', $relationship_type, 'type');

    drupal_set_message(t('Relationship %name has been updated.', array('%name' => $relationship_type->name)));
    watchdog(
      'u_relationship', 
      t('User Relationships form: relationship %name updated.', array('%name' => $relationship_type->name)), 
      WATCHDOG_NOTICE, 
      l(t('view'), 'admin/user/relationships/types')
    );
  }

  user_relationships_types_load(TRUE);
  menu_rebuild();

  $form_state['redirect'] = 'admin/user/relationships';
}


/**
 * Process relationship delete form submission.
 */
function user_relationships_type_delete_submit($form, &$form_state) {
  $relationship_type = user_relationships_type_load($form_state['values']['rtid']);

  db_query("DELETE FROM {user_relationship_types} WHERE rtid = %d", $relationship_type->rtid);
  db_query("DELETE FROM {user_relationships} WHERE rtid = %d", $relationship_type->rtid);

  _user_relationships_invoke('delete', $relationship_type, 'type');

  drupal_set_message(t('Relationship %name has been deleted.', array('%name' => $relationship_type->name)));
  watchdog(
    'u_relationship', 
    t('User Relationships form: relationship %name deleted.', array('%name' => $relationship_type->name)),
    WATCHDOG_NOTICE
  );
            
  $form_state['redirect'] = 'admin/user/relationships';
}


/**
 * Validate relationship request.
 */
function user_relationships_request_validate($form, &$form_state) {
  $requester = user_load($form_state['values']['requester']);
  $requestee = user_load($form_state['values']['requestee']);

  if (user_access('can have relationship', $requestee)) {
    $current_relationships = user_relationships_load(array('between' => array($requester->uid, $requestee->uid)));

    if (!db_result(db_query(
      "SELECT COUNT(*) FROM {user_relationship_types}" . ($current_relationships ? " WHERE rtid NOT IN (%s) ORDER BY name" : ''), 
      implode(',', array_keys($current_relationships))
    ))) {
      drupal_set_message(user_relationships_get_message('too_many_relationships'), 'error');
      drupal_goto();
    }
    else if ($current_relationships[$form_values['rtid']]) {
      $message_name = $current_relationships[$form_values['rtid']]->approved ? 'exisitng_relationship' : 'existing_request';
      form_set_error('rtid', user_relationships_get_message($message_name, array(
        '%requester'                => $requester->name,
        '%requestee'                => $requestee->name,
        '%relationship_name'        => $$current_relationships[$form_values['rtid']]->name,
        '%relationship_plural_name' => $$current_relationships[$form_values['rtid']]->plural_name
      )));
    }
  }
  else {
    drupal_set_message(user_relationships_get_message('not_accepting_requests'));
    drupal_goto();
  }
}

/**
 * Process relationship request.
 */
function user_relationships_request_submit($form, &$form_state) {
  $requester          = user_load($form_state['values']['requester']);
  $requestee          = user_load($form_state['values']['requestee']);
  $relationship_type  = user_relationships_type_load($form_state['values']['rtid']);

  $relationship = user_relationships_request_relationship($requester, $requestee, $relationship_type);

  if ($relationship === FALSE) {
    drupal_set_message(user_relationships_get_message('unknown_error', $relationship));
  }

  $relationship->requester = $requester;
  $relationship->requestee = $requestee;
  $relationship->type = $relationship_type;

  drupal_set_message(user_relationships_get_message(($relationship->approved ? 'pre_approved' : 'submitted'), $relationship));

  $form_state['redirect'] = $_GET['destination'];
}


/**
 * Approve, Disapprove, or Cancel a relationship request
 */
function user_relationships_pending_requested_submit($form, &$form_state) {
  global $user;

  $relationship = user_relationships_load($form_state['values']['rid']);
  $requester    = $relationship->requester;
  $requestee    = $relationship->requestee;

  $viewing_own_account = ($user->uid == $requestee->uid);

  switch ($form_state['values']['action']) {
  case 'approve':
    $updated = $relationship;
    $updated->approved = TRUE;
    user_relationships_update_relationship($relationship, $updated, $form_state['values']['action']);
    drupal_set_message(user_relationships_get_message('accepted', $relationship));
    break;

  case 'cancel':
  case 'disapprove':
    user_relationships_delete_relationship($relationship, $user, $form_state['values']['action']);
    drupal_set_message(user_relationships_get_message($form_state['values']['action'], $relationship));
    break;

  case 'ban':
    
    break;

  default:
    drupal_set_message(user_relationships_get_message('default'));
  }

  drupal_goto();
}


/**
 * Remove a relationship
 */
function user_relationships_remove_submit($form, &$form_state) {
  global $user;

  $relationship = user_relationships_load($form_state['values']['rid']);

  user_relationships_delete_relationship($relationship, $user);
  drupal_set_message(user_relationships_get_message('removed', $relationship));

  return "user/{$form_values['viewed_id']}/relationships";
}
