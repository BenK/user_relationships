<?php
// $Id: user_relationships_forms.inc,v 1.14 2007-12-23 10:02:19 sprsquish Exp $

/**
 * User Relationships forms
 */


/**
 * Main settings
 */
function user_relationships_settings() {
  $form['general'] = array(
    '#type'   => 'fieldset',
    '#title'  => t('General'),
    '#weight' => -10,
  );
    $form['general']['user_relationships_allow_multiple'] = array(
      '#type'           => 'checkbox',
      '#title'          => t('Allow Multiple Relationships'),
      '#description'    => t('Give users the option to create more than one relationship to each other.'),
      '#default_value'  => variable_get('user_relationships_allow_multiple', 1),
    );
    $form['general']['user_relationships_allow_auto_approve'] = array(
      '#type'           => 'checkbox',
      '#title'          => t("Allow users to auto approve"),
      '#description'    => t('This will provide an the options for users to set an "auto-approve" option to automatically approve to all requested relationships.'),
      '#default_value'  => variable_get('user_relationships_allow_auto_approve', 0),
    );
    $form['general']['user_relationships_relationships_per_page'] = array(
      '#type'           => 'textfield',
      '#title'          => t("Relationships per page"),
      '#size'           => 4,
      '#description'    => t('Number of relationships to show per page. If set to 0 all will be shown.'),
      '#default_value'  => variable_get('user_relationships_relationships_per_page', 16),
      '#validate'       => array('user_relationships_setting_validation' => array(array(
        'is_numeric' => array('msg' => t('The relationships per page setting is not an integer'))
      )))
    );

    $replaceables = array(
      '!requester',
      '!requestee',
      '%relationship_name',
      '%relationship_plural_name',
    );
    
    $form['messages'] = array(
      '#type'         => 'fieldset',
      '#title'        => t('Messages'),
      '#weight'       => 0,
      '#description'  => t('Notifications to users. Replaceable tokens are: @replaceables', array('@replaceables' => implode($replaceables, ', '))),
    );
    
    $default_messages = user_relationships_default_messages();
    _user_relationships_message_settings_form($form['messages'], $default_messages);
 
    $replaceables = array(
      '!requester',
      '!requestee',

      '%relationship_name',
      '%relationship_plural_name',
    );
    
    $form['messages'] = array(
      '#type'         => 'fieldset',
      '#title'        => t('Messages'),
      '#weight'       => 0,
      '#description'  => t('Notifications to users. Replaceable tokens are: @replaceables', array('@replaceables' => implode($replaceables, ', '))),
    );
    
    $default_messages = user_relationships_default_messages();
    _user_relationships_message_settings_form($form['messages'], $default_messages);
 
  return system_settings_form($form);
}


/**
 * Relationship type edit page.
 */
function user_relationships_type_edit($rtid = NULL) {
  if (arg(3) == "edit" && $rtid >= 0) {
    $relationship_type = user_relationships_type_load(array('rtid' => $rtid));
  }

  $form['name'] = array(
    '#type'           => 'textfield',
    '#title'          => t('Name'),
    '#maxlength'      => 255,
    '#default_value'  => $relationship_type->name,
    '#description'    => t("Example: buddy, friend, coworker, spouse."),
    '#required'       => TRUE,
    '#weight'         => -10,
  );
  $form['plural_name'] = array(
    '#type'           => 'textfield',
    '#title'          => t('Plural name'),
    '#maxlength'      => 255,
    '#default_value'  => $relationship_type->plural_name,
    '#description'    => t("Example: buddies, friends, coworkers, spouses."),
    '#weight'         => -9,
  );
  $form['is_oneway'] = array(
    '#type'           => 'checkbox',
    '#title'          => t('This is a one-way relationship'),
    '#default_value'  => $relationship_type->is_oneway,
    '#description'    => t('Check this if this relationship should only go one way (ex Manager, Subscriber)'),
    '#weight'         => -8,
  );
  $form['requires_approval'] = array(
    '#type'           => 'checkbox',
    '#title'          => t('Requires Approval'),
    '#default_value'  => (isset($relationship_type->requires_approval) ? $relationship_type->requires_approval : 1),
    '#description'    => t('Check this if the requestee must approve the relationship'),
    '#weight'         => -8,
  );
  $form['expires_val'] = array(
    '#title'          => t('Request expires in'),
    '#field_suffix'   => t('days'),
    '#type'           => 'textfield',
    '#size'           => 4,
    '#default_value'  => isset($relationship_type->expires_val) ? $relationship_type->expires_val : 0,
    '#description'    => t('After how many days should a request of this type be removed? (0 for never)'),
    '#weight'         => -8,
  );
  $form['relationship_type'] = array(
    '#type'   => 'value',
    '#value'  => $relationship_type,
  );
  $form['submit'] = array(
    '#type'   => 'submit',
    '#value'  => t('Submit'),
    '#weight' => 10,
  );

  return $form;
}

/**
 * Relationship type delete page.
 */
function user_relationships_type_delete($rtid = NULL) {
  if ($relationship_type = user_relationships_type_load(array('rtid' => $rtid))) {
    $form['relationship_type'] = array(
      '#type'   => 'value',
      '#value'  => $relationship_type,
    );

    return confirm_form(
      $form, 
      t('Are you sure you want to delete %name?', array('%name' => $relationship_type->name)), 
      'admin/user/relationships', 
      t('This action cannot be undone.'),
      t('Delete'), t('Cancel')
    );
  }
  else {
    drupal_set_message(user_relationships_get_message('non_existant_user'));
    drupal_goto('admin/user/relationships');
  }
}


/**
 * Request new user to user relationship
 */
function user_relationships_request_form(&$requester, &$requestee, $form_values = NULL) {
  $current_relationships = user_relationships_load(array('between' => array($requester->uid, $requestee->uid)), FALSE, 'rtid');

  $results = db_query(
    "SELECT * FROM {user_relationship_types}" . ($current_relationships ? " WHERE rtid NOT IN (%s) ORDER BY name" : ''), 
    implode(',', array_keys($current_relationships))
  );
  $relationships = array();
  while($result = db_fetch_object($results)) {
    $relationships[$result->rtid] = $result->name;
  }

  $form['rtid'] = array(
    '#title'          => t('My relationship is'),
    '#type'           => 'select',
    '#options'        => $relationships,
    '#default_value'  => $form_values['rtid'],
  );
  $form['requester'] = array(
    '#type'   => 'value',
    '#value'  => $requester
  );    
  $form['requestee'] = array(
    '#type'   => 'value',
    '#value'  => $requestee
  );

  return $form;
}

/**
 * Request a new relationship with another user
*/
function user_relationships_request($requestee_id) {
  global $user;
  $requester = user_load(array('uid' => $user->uid));
  $requestee = user_load(array('uid' => $requestee_id));

  if (empty($requestee->name)) {
    drupal_set_message(user_relationships_get_message('non_existant_user'));
  }
  else if (!user_access('can have relationship', $requestee)) {
    drupal_set_message(user_relationships_get_message('not_accepting_requests'));
  }
  else if ($user->uid == $requestee_id) {
    drupal_set_message(user_relationships_get_message('self_request'));
  }
  else {
    $form = user_relationships_request_form($requester, $requestee);
    if (!$form['rtid']['#options']) {
      drupal_set_message(user_relationships_get_message('too_many_relationships'), 'error');
    }
    else {
      $form = confirm_form(
        $form,
        t('How do you relate to %name?', array('%name' => $requestee->name)),
        "user/$requestee->uid",
        '',
        t('Create'), t('Cancel'),
        'user_relationships_request_confirm'
      );

      return $form;
    }
  }

  drupal_goto();
}


/**
 * Approve, Disapprove, or Cancel confirmation form
 */
function user_relationships_pending_requested($action, $viewed_id, $relationship) {
  $relationship = user_relationships_load($relationship);
  $relationship = array_shift($relationship);

  $form['relationship'] = array(
    '#type'   => 'value',
    '#value'  => $relationship
  );
  $form['action'] = array(
    '#type'   => 'value',
    '#value'  => $action
  );
  $form['viewed_id'] = array(
    '#type'   => 'value',
    '#value'  => $viewed_id
  );

  $output = confirm_form(
    $form,
    t('@action relationship', array('@action' => ucfirst($action))),
    "user/{$viewed_id}/relationships/requests",
    t("Are you sure you want to @action the request to create a %relationship_name relationship to !name?", array(
      '@action'             => $action,
      '%relationship_name'  => $relationship->name,
      '!name'               => theme('username', user_load(array('uid' => ($viewed_id == $relationship->requester_id ? $relationship->requestee_id : $relationship->requester_id))))
    )),
    t('Yes'), t('No'),
    'user_relationships_approve_confirm'
  );
    
  return $output;
}

/**
 * Confirm relationship removal.
 */
function user_relationships_remove($viewed_id, $relationship) {
  $relationship = user_relationships_load($relationship);
  $relationship = array_shift($relationship);

  $form['relationship'] = array(
    '#type'   => 'value',
    '#value'  => $relationship
  );
  $form['viewed_id'] = array(
    '#type'   => 'value',
    '#value'  => $viewed_id
  );
  $output = confirm_form(
    $form,
    t('Remove relationship'),
    "user/{$viewed_id}/relationships/",
    t("Are you sure you no longer wish to be a %relationship_name of !name?", array(
      '%relationship_name'  => $relationship->name,
      '!name'               => theme('username', user_load(array('uid' => ($viewed_id == $relationship->requester_id ? $relationship->requestee_id : $relationship->requester_id))))
    )),
    t('Yes'), t('No'),
    'user_relationships_remove_confirm'
  );
    
  return $output;
}

