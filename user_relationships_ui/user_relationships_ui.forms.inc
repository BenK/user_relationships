<?php
// $Id: user_relationships_ui.forms.inc,v 1.1.2.1 2008-03-02 00:51:52 sprsquish Exp $

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
    $form = user_relationships_ui_request_form($requester->uid, $requestee->uid);
    if (!$form['rtid']['#options']) {
      drupal_set_message(user_relationships_ui_get_message('too_many_relationships'), 'error');
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

  $output = confirm_form(
    $form,
    t('@action relationship', array('@action' => ucfirst($action))),
    ($viewed_id == $user->id ? 'relationships/requests' : "user/{$viewed_id}/relationships/requests"),

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

    t("Are you sure you no longer wish to be a %relationship_name of !name?", array(
      '%relationship_name'  => $relationship->name,
      '!name'               => theme('username', user_load(array('uid' => ($viewed_id == $relationship->requester_id ? $relationship->requestee_id : $relationship->requester_id))))
    )),
    t('Yes'), t('No'),
    'user_relationships_remove_confirm'
  );
    
  return $output;
}

