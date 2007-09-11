<?php
// $Id: user_relationships_forms.inc,v 1.4 2007-09-11 00:03:42 sprsquish Exp $

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

  return system_settings_form($form);
}

/**
 * Relationship type edit page.
 */
function user_relationships_type_edit($rtid = NULL) {
  if (arg(3) == "edit" && $rid >= 0) {
    $relationship_type = user_relationships_relationship_type_load(array('rtid' => $rtid));
  }

  $form['name'] = array(
    '#type'           => 'textfield',
    '#title'          => t('Name'),
    '#maxlength'      => 255,
    '#default_value'  => $relationship_type->name,
    '#description'    => t("Example: friend, coworker, spouse."),
    '#required'       => TRUE,
    '#weight'         => -10,
  );
  $form['is_oneway'] = array(
    '#type'           => 'checkbox',
    '#title'          => t('This is a one-way relationship'),
    '#default_value'  => $relationship_type->is_oneway,
    '#description'    => t('Check this if this relationship should only go one way (ex Manager, Subscriber)'),
    '#weight'         => -9
  );
  $form['requires_approval'] = array(
    '#type'           => 'checkbox',
    '#title'          => t('Requires Approval'),
    '#default_value'  => (isset($relationship_type->requires_approval) ? $relationship_type->requires_approval : 1),
    '#description'    => t('Check this if the requestee must approve the relationship'),
    '#weight'         => -9
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
  if ($relationship_type = user_relationships_relationship_type_load(array('rtid' =>$rtid))) {
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
    drupal_set_message(t('Relationship not found.'), 'error');
    drupal_goto('admin/user/relationships');
  }
}


/**
 * Request new user to user relationship
 */
function user_relationships_request_form(&$requester, &$requestee, $form_values = NULL) {
  $current_relationships = _user_relationships_current_relationships($requester, $requestee);

  $results = db_query("SELECT * FROM {user_relationship_types}" . ($current_relationships ? " WHERE rtid NOT IN (%s) ORDER BY name" : ''), implode(',', array_keys($current_relationships)));
  $relationships = array();
  while($result = db_fetch_array($results)) {
    $relationships[$result['rtid']] = $result['name'];
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
    drupal_set_message(t('This user does not exist'));
  }
  else if (!user_access('can have relationship', $requestee)) {
    drupal_set_message(t("This user is not accepting relationships"));
  }
  else if ($user->uid == $requestee_id) {
    drupal_set_message(t("You are not allowed to create a relationship to yourself"));
  }
  else {
    $form = user_relationships_request_form($requester, $requestee);
    if (!$form['rtid']['#options']) {
      drupal_set_message(t("You already relate to this user in every possible way."));
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
  $relationship = user_relationships_relationship_load($relationship);

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
    'relationships/'.$viewed_id.'/requests',
    t("Are you sure you want to @action the request to create a %relationship_name relationship to !name?", array(
      '@action'             => $action,
      '%relationship_name'  => $relationship->name,
      '!name'               => theme('username', $relationship->requester)
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
  $relationship = user_relationships_relationship_load($relationship);
  $requester    = $relationship->requester;
  $requestee    = $relationship->requestee;

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
    'relationships/' . $viewed_id,
    t("Are you sure you no longer wish to be a %relationship_name of !name?", array(
      '%relationship_name'  => $relationship->name,
      '!name'               => theme('username', $viewed_id == $requester->uid ? $requestee : $requester)
    )),
    t('Yes'), t('No'),
    'user_relationships_remove_confirm'
  );
    
  return $output;
}

