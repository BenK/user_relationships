<?php
// $Id: user_relationships_ui.admin.inc,v 1.1.2.2 2008-03-02 11:46:13 sprsquish Exp $

/**
 * User Relationships forms
 */

require_once(drupal_get_path('module', 'user_relationships_ui').'/user_relationships_ui.admin_actions.inc');

/**
 * Main settings
 */
function user_relationships_ui_settings() {
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
    
    $default_messages = _user_relationships_ui_default_messages();
    _user_relationships_ui_message_settings_form($form['messages'], $default_messages);
 
  return system_settings_form($form);
}


/**
 * Relationship type edit page.
 */
function user_relationships_ui_type_edit(&$form_state, $relationship_type = NULL) {
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
  $form['rtid'] = array(
    '#type'   => 'value',
    '#value'  => (int)$relationship_type->rtid,
  );
  $form['action'] = array(
    '#type'   => 'value',
    '#value'  => ($relationship_type ? 'edit' : 'add'),
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
function user_relationships_ui_type_delete(&$form_state, $relationship_type = NULL) {
  if ($relationship_type) {
    $form['rtid'] = array(
      '#type'   => 'value',
      '#value'  => (int)$relationship_type->rtid,
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
    drupal_set_message(user_relationships_ui_get_message('non_existant_type'));
    drupal_goto('admin/user/relationships');
  }
}