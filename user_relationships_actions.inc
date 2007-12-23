<?php
// $Id: user_relationships_actions.inc,v 1.18 2007-12-23 10:02:19 sprsquish Exp $

/**
 * User Relationships Actions
 */


/**
 * Validate the submitted relationship type
 */
function user_relationships_type_edit_validate($form_id, &$form_values) {
  if (
    ($form_values['relationship_type']->rtid && 
      db_result(db_query(
        "SELECT name FROM {user_relationship_types} 
         WHERE LOWER(`name`) = LOWER('%s') 
           AND rtid != %d", 
        $form_values['name'], $form_values['relationship_type']->rtid)
      )
    ) ||
    (!$form_values['relationship_type']->rtid && 
      db_result(db_query("SELECT name FROM {user_relationship_types} WHERE LOWER(`name`) = LOWER('%s')", $form_values['name']))
    )
  ) {
    form_set_error('name', t('%name has already been used.', array('%name' => $form_values['name'])));
  }

  $expire = $form_values['expires_val'];
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
function user_relationships_type_edit_submit($form_id, &$form_values) {
  if (arg(3) == 'add') {
    $form_values['rtid'] = db_next_id('{user_relationship_types}_id');
    db_query(
      "INSERT INTO {user_relationship_types} (rtid, name, plural_name, is_oneway, requires_approval, expires_val) VALUES 
       (%d, '%s', '%s', %d, %d, %d)", 
      $form_values['rtid'],
      $form_values['name'],
      $form_values['plural_name'],
      $form_values['is_oneway'],
      $form_values['requires_approval'],
      $form_values['expires_val']
    );

    $relationship_type = user_relationships_type_load(array('rtid' => $form_values['rtid']));
    _user_relationships_invoke('insert', $relationship_type, 'type');

    drupal_set_message(t('Relationship %name has been added.', array('%name' => $form_values['name'])));
    watchdog(
      'u_relationship', 
      t('User Relationships form: relationship %name added.', array('%name' => $form_values['name'])), 
      WATCHDOG_NOTICE, 
      l(t('view'), 'admin/user/user_relationship/types')
    );
  }
  else {
    db_query(
      "UPDATE {user_relationship_types} 
       SET name = '%s', plural_name = '%s', is_oneway = %d, requires_approval = %d, expires_val = %d 
       WHERE rtid = %d", 
      $form_values['name'],
      $form_values['plural_name'],
      $form_values['is_oneway'], 
      $form_values['requires_approval'], 
      $form_values['expires_val'],
      $form_values['relationship_type']->rtid
    );

    _user_relationships_invoke('update', $form_values['relationship_type'], 'type');

    drupal_set_message(t('Relationship %name has been updated.', array('%name' => $form_values['name'])));
    watchdog(
      'u_relationship', 
      t('User Relationships form: relationship %name updated.', array('%name' => $form_values['name'])), 
      WATCHDOG_NOTICE, 
      l(t('view'), 'admin/user/relationships/types')
    );
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

  _user_relationships_invoke('delete', $form_values['relationship_type'], 'type');

  drupal_set_message(t('Relationship %name has been deleted.', array('%name' => $form_values['name'])));
  watchdog('u_relationship', t('User Relationships form: relationship %name deleted.', array('%name' => $form_values['name'])), WATCHDOG_NOTICE);
            
  return 'admin/user/relationships';
}


/**
 * Validate relationship request.
 */
function user_relationships_request_validate($form_id, &$form_values) {
  $requester = $form_values['requester'];
  $requestee = $form_values['requestee'];

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
function user_relationships_request_submit($form_id, &$form_values) {
  $requester          = $form_values['requester'];
  $requestee          = $form_values['requestee'];
  $relationship_type  = user_relationships_type_load(array('rtid' => $form_values['rtid']));
  
  $relationship = user_relationships_request_relationship($requester, $requestee, $relationship_type);

  $relationship->requester = $requester;
  $relationship->requestee = $requestee;
  $relationship->type = $relationship_type;

  drupal_set_message(user_relationships_get_message(($relationship->approved ? 'pre_approved' : 'submitted'), $relationship));

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
    drupal_set_message(user_relationships_get_message('accepted', $relationship));
    break;

  case 'cancel':
  case 'disapprove':
    user_relationships_delete_relationship($relationship, $user, $form_values['action']);
    drupal_set_message(user_relationships_get_message('denied', $relationship));
    break;

  default:
    drupal_set_message(user_relationships_get_message('default'));
  }

  return "user/{$form_values['viewed_id']}/relationships/requests";
}


/**
 * Remove a relationship
 */
function user_relationships_remove_submit($form_id, &$form_values) {
  global $user;

  $relationship = $form_values['relationship'];

  user_relationships_delete_relationship($relationship, $user);
  drupal_set_message(user_relationships_get_message('removed', $relationship));

  return "user/{$form_values['viewed_id']}/relationships";
}
