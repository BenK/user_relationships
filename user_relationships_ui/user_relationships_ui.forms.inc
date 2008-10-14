<?php
// $Id: user_relationships_ui.forms.inc,v 1.1.2.6 2008-10-14 09:14:04 alexk Exp $

/**
 * User Relationships forms
 */

require_once(drupal_get_path('module', 'user_relationships_ui').'/user_relationships_ui.actions.inc');

/**
 * Request new user to user relationship
 */
function user_relationships_ui_request_form($requester, $requestee, $form_values = NULL) {
  $current_relationships = user_relationships_load(array('between' => array($requester, $requestee)), array('sort' => 'rtid'));

  $results = db_query(
    "SELECT * FROM {user_relationship_types}" . ($current_relationships ? " WHERE rtid NOT IN (%s) ORDER BY name" : ''),
    implode(',', array_keys($current_relationships))
  );
  $relationships = array();
  while($result = db_fetch_object($results)) {
    $relationships[$result->rtid] = $result->name;
  }
  //try to find out desired relationship type
  if ($form_values['rtid']) {//given through form
    $default_relationship = $form_values['rtid'];
  }
  else if (is_numeric(arg(3)) && user_relationships_type_load(arg(3))) {//given through URL arguments, e.g. relationship/{uid}/request/{rtid}
    $default_relationship = arg(3);
  }
  else if (count($relationships) == 1) {//preselect the only one.
    $rtids = array_keys($relationships);
    $default_relationship = $rtids[0];
  }
  if (count($relationships)) {
    //hide control if the relationship is chosen
    if ($default_relationship && variable_get('user_relationships_show_direct_links', 1)) {
      $form['rtid'] = array(
        '#type' => 'hidden',
        '#default_value' => $default_relationship,
      );
    }
    else {
      $form['rtid'] = array(
        '#title'          => t('My relationship is'),
        '#type'           => 'radios',
        '#options'        => $relationships,
        '#default_value'  => $default_relationship,
      );
    }
  }
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
function user_relationships_ui_request(&$form_state, $requestee) {
  global $user;
  $requester = $user;

  if (empty($requestee->name)) {
    drupal_set_message(user_relationships_ui_get_message('non_existant_user'));
  }
  else if (!user_access('can have relationships', $requestee)) {
    drupal_set_message(user_relationships_ui_get_message('not_accepting_requests'));
  }
  else if ($user->uid == $requestee->uid) {
    drupal_set_message(user_relationships_ui_get_message('self_request'));
  }
  else {
    $form = user_relationships_ui_request_form($requester->uid, $requestee->uid, $form_state);
    if (!$form['rtid']) {
      drupal_set_message(user_relationships_ui_get_message('too_many_relationships'), 'error');
    }
    else {
      if ($form['rtid']['#default_value']) {
        $relationship = user_relationships_type_load($form['rtid']['#default_value']);
      }
      $form = confirm_form(
        $form,
        t('Request relationship'),
        "user/$requestee->uid",
        $relationship ?
          t('Are you sure you wish to send a new %rel_name request to %name?', array('%name' => $requestee->name, '%rel_name' => $relationship->name, '%rel_plural_name' => $relationship->plural_name)) :
          t('How do you relate to %name?', array('%name' => $requestee->name)),
        t('Send'), t('Cancel'),
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
function user_relationships_ui_pending_requested(&$form_state, $action, $account, $relationship) {
  $viewed_id = $account->uid;

  $form['rid'] = array(
    '#type'   => 'value',
    '#value'  => $relationship->rid
  );
  $form['action'] = array(
    '#type'   => 'value',
    '#value'  => $action
  );
  $form['viewed_id'] = array(
    '#type'   => 'value',
    '#value'  => $viewed_id
  );
  
  //pick the correct question message: approve and disapprove are requests to current user, cancel is a request from current user
  switch($action) {
    case 'approve':
    case 'disapprove':
      $confirmation_message = 'Are you sure you want to @action the %relationship_name relationship request from !name?';
      break;
    default:
      $confirmation_message = 'Are you sure you want to @action your %relationship_name relationship request to !name?';
  }

  $output = confirm_form(
    $form,
    t('@action relationship', array('@action' => ucfirst($action))),
    ($viewed_id == $user->id ? 'relationships/requests' : "user/{$viewed_id}/relationships/requests"),

    t($confirmation_message, array(
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
function user_relationships_ui_remove(&$form_state, $account, $relationship) {
  global $user;

  $viewed_id = $account->uid;

  $form['rid'] = array(
    '#type'   => 'value',
    '#value'  => $relationship->rid
  );
  $form['viewed_id'] = array(
    '#type'   => 'value',
    '#value'  => $viewed_id
  );
  $output = confirm_form(
    $form,
    t('Remove relationship'),
    ($viewed_id == $user->id ? 'relationships' : "user/{$viewed_id}/relationships/"),

    t("Are you sure you wish to delete the %relationship_name relationship with !name?", array(
      '%relationship_name'  => $relationship->name,
      '!name'               => theme('username', user_load(array('uid' => ($viewed_id == $relationship->requester_id ? $relationship->requestee_id : $relationship->requester_id))))
    )),
    t('Yes'), t('No'),
    'user_relationships_remove_confirm'
  );

  return $output;
}
