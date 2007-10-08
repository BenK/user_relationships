<?php
// $Id: user_relationships_actions.inc,v 1.11 2007-10-08 19:49:37 sprsquish Exp $

/**
 * User Relationships Actions
 */


/**
 * Validate the submitted relationship type
 */
function user_relationships_type_edit_validate($form_id, &$form_values) {
  if (
    ($form_values['relationship_type']->rtid && db_result(db_query("SELECT name FROM {user_relationship_types} WHERE LOWER(`name`) = LOWER('%s') AND rtid != %d", $form_values['name'], $form_values['relationship_type']->rtid))) ||
    (!$form_values['relationship_type']->rtid && db_result(db_query("SELECT name FROM {user_relationship_types} WHERE LOWER(`name`) = LOWER('%s')", $form_values['name'])))
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
      "INSERT INTO {user_relationship_types} (rtid, name, plural_name, is_oneway, requires_approval, expires_val) VALUES (%d, '%s', '%s', %d, %d, %d)", 
      $form_values['rtid'],
      $form_values['name'],
      $form_values['plural_name'],
      $form_values['is_oneway'],
      $form_values['requires_approval'],
      $form_values['expires_val']
    );

    $relationship_type = user_relationships_relationship_type_load(array('rtid' => $form_values['rtid']));
    _user_relationships_invoke('insert type', $relationship_type);

    drupal_set_message(t('Relationship %name has been added.', array('%name' => $form_values['name'])));
    watchdog('userreltnships', t('User Relationships form: relationship %name added.', array('%name' => $form_values['name'])), WATCHDOG_NOTICE, l(t('view'), 'admin/user/user_relationship/types'));
  }
  else {
    db_query(
      "UPDATE {user_relationship_types} SET name = '%s', plural_name = '%s', is_oneway = %d, requires_approval = %d, expires_val = %d WHERE rtid = %d", 
      $form_values['name'],
      $form_values['plural_name'],
      $form_values['is_oneway'], 
      $form_values['requires_approval'], 
      $form_values['expires_val'],
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
      drupal_set_message(user_relationships_get_message('too_many_relationships'));
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
  $relationship_type  = user_relationships_relationship_type_load(array('rtid' => $form_values['rtid']));
  
  user_relationships_request_relationship($requester, $requestee, $relationship_type);
  drupal_set_message(user_relationships_get_message('submitted', array(
    '%requester'                => $requester->name,
    '%requestee'                => $requestee->name,
    '%relationship_name'        => $relationship_type->name,
    '%relationship_plural_name' => $relationship_type->plural_name,
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
    drupal_set_message(user_relationships_get_message('accepted', array(
      '!requester'                => theme('username', $requester),
      '!requestee'                => theme('username', $requester),
      '%relationship_name'        => $relationship->name,
      '%relationship_plural_name' => $relationship->plural_name,
    )));
    break;

  case 'cancel':
  case 'disapprove':
    user_relationships_delete_relationship($relationship, $user, $form_values['action']);
    drupal_set_message(user_relationships_get_message('denied', array(
      '!requester'                => theme('username', $requester),
      '!requestee'                => theme('username', $requestee),
      '%relationship_name'        => $relationship->name,
      '%relationship_plural_name' => $relationship->plural_name,
    )));
    break;

  default:
    drupal_set_message(user_relationships_get_message('default'));
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
  drupal_set_message(user_relationships_get_message('removed', array(
    '!requester'                => $relationship->requester->name,
    '!requestee'                => $relationship->requestee->name,
    '%relationship_name'        => $relationship->name,
    '%relationship_plural_name' => $relationship->plural_name,
  )));

  return 'relationships/'. $form_values['viewed_id'];
}

/**
 * Validate settings on the block configuration page(s)
 */
function user_relationships_block_num_rels_validate($form) {
  $num_rels = $form['#value'];
  if (!is_numeric($num_rels) || (int)$num_rels != $num_rels || (int)$num_rels <= 0) {
    form_set_error('user_relationships_block_num_rels', t('Number of users must be an integer greater than 0.'));
    return FALSE;
  } 
  return TRUE;
}

/**
 * function user_relationships_block_select_relationships
 *   given a $user object, select which of their relationships to display in the block
 * 
 * @param $user - user object of user whose relationships we want to select
 * @param $rtid - type id of relationships we're interested in, or 'ALL' for all relationships
 * @param $how_many - how many relationships to return
 * @param $options - array of (text) options presented to admins to allow them to select which n relationships to display
 * @param $options_index - index into the $options array indicating which option the admin has selected for this type
 *
 * @return array of relationship objects
 */
function user_relationships_block_select_relationships($user, $rtid, $how_many, $options, $options_index) {
  $relationships = user_relationships_load_all_for_user($viewer->uid);

  if (is_null($relationships)) return NULL;

  // get rid of irrelevant relationships
  for ($i = 0; isset($relationships[$i]); $i++) {
    if (1 != $relationships[$i]->approved) {
      unset($relationships[$i]);
    }
    else if ('ALL' != $rtid) {
      if ($relationships[$i]->rtid != $rtid) unset($relationships[$i]);
    }
  }

  if (count($relationships) == 0) return NULL;

  // trim the list as indicated by $how_many and $options[$options_index]
  if (count($relationships) > $how_many) {
    switch (drupal_strtolower($options[$options_index])) {
    case 'newest':
      usort($relationships, '_user_relationships_compare_newest_first');
      $relationships = array_slice($relationships, 0, $how_many);
      break;
    case 'oldest':
      usort($relationships, '_user_relationships_compare_oldest_first');
      $relationships = array_slice($relationships, 0, $how_many);
      break;
    case 'random':
      $random_rels = array();
      $keys = array_rand($relationships, $how_many);
      foreach ($keys as $key) {
        $random_rels[] = $relationships[$key];
      } // foreach ($keys as $key)
      $relationships = $random_rels;
      break;
    }
  }

  return $relationships;
}

/**
 * sort relationships by date in ascending order
 */
function _user_relationships_compare_oldest_first($r1, $r2) {
  return strcmp($r1->updated_at, $r2->updated_at);
}

/**
 * sort relationships by date in descending order
 */
function _user_relationships_compare_newest_first($r1, $r2) {
  return -strcmp($r1->updated_at, $r2->updated_at);
}
