<?php
// $Id: user_relationships_hooks.inc,v 1.17 2008-02-01 02:29:50 sprsquish Exp $

/**
 * User Relationships hook implementations
 */

/**
 * Help
 */
function user_relationships_help($section) {
  switch ($section) {
    case 'admin/help#user_relationships':
      $output = '<p>'. t('This module allows you to create relationship types that users can use to connect to each other.') .'</p>';
      return $output;

    case 'admin/user/relationships':
      $output = '<p>'. t('This page lets you setup user relationship types.') .'</p>';
      return $output;
  }
}

/**
 * Perm
 */
function user_relationships_perm() {
  return array(
    'administer user relationships',
    'maintain relationships',
    'view user relationships',
    'can have relationship',
  );
}

/**
 * Menu
*/
function user_relationships_menu() {
  $items = array();

  $items['admin/user/relationships'] = array(
    'title'             => t('Relationships'),
    'description'       => t('Create relationship types'),
    'access callback'   => 'user_access',
    'access arguments'  => array('administer user relationships'),
    'page callback'     => 'user_relationships_types_list_page',
  );
    $items['admin/user/relationships/list'] = array(
      'title'             => t('List'),
      'type'              => MENU_DEFAULT_LOCAL_TASK,
      'page callback'     => 'user_relationships_types_list_page',
      'access callback'   => 'user_access',
      'access arguments'  => array('administer user relationships'),
    );
    $items['admin/user/relationships/add'] = array(
      'title'             => t('Add type'),
      'type'              => MENU_LOCAL_TASK,
      'weight'            => 1,
      'page callback'     => 'drupal_get_form',
      'page arguments'    => array('user_relationships_type_edit'),
      'access callback'   => 'user_access',
      'access arguments'  => array('administer user relationships'),
    );
    $items['admin/user/relationships/%relationship_type/edit'] = array(
      'title'             => t('Edit type'),
      'type'              => MENU_CALLBACK,
      'page callback'     => 'drupal_get_form',
      'page arguments'    => array('user_relationships_type_edit', 3),
      'access callback'   => 'user_access',
      'access arguments'  => array('administer user relationships'),
    );
    $items['admin/user/relationships/%relationship_type/delete'] = array(
      'title'             => t('Delete type'),
      'type'              => MENU_CALLBACK,
      'page callback'     => 'drupal_get_form',
      'page arguments'    => array('user_relationships_type_delete', 3),
      'access callback'   => 'user_access',
      'access arguments'  => array('administer user relationships'),
    );
  $items['admin/user/relationships/settings'] = array(
    'title'             => t('Settings'),
    'type'              => MENU_LOCAL_TASK,
    'weight'            => 2,
    'page callback'     => 'drupal_get_form',
    'page arguments'    => array('user_relationships_settings'),
    'access callback'   => 'user_access',
    'access arguments'  => array('administer user relationships'),
  );

  $items['relationship_types/autocomplete'] = array(
    'title'             => t('User Relationships Autocomplete'),
    'type'              => MENU_CALLBACK,
    'page callback'     => '_user_relationships_autocomplete_types',
    'access callback'   => 'user_access',
    'access arguments'  => array('administer user relationships'),
  );

  $items['relationships'] = array(
    'title'             => t('My relationships'),
    'type'              => MENU_NORMAL_ITEM,
    'access callback'   => 'user_relationships_check_access',
    'access arguments'  => array('view'),
    'page callback'     => 'theme',
    'page arguments'    => array('user_relationships_page'),
  );
    $items['relationships/list'] = array(
      'title'             => t('All'),
      'type'              => MENU_NORMAL_ITEM|MENU_DEFAULT_LOCAL_TASK,
      'weight'            => -10,
      'access callback'   => 'user_relationships_check_access',
      'access arguments'  => array('view')
    );
    $items['relationships/requests'] = array(
      'title'             => t('Pending'),
      'access callback'   => 'user_relationships_check_access',
      'access arguments'  => array('edit'),
      'type'              => MENU_NORMAL_ITEM|MENU_LOCAL_TASK,
      'weight'            => -9,
      'page callback'     => 'theme',
      'page arguments'    => array('user_relationships_pending_requests_page')
    );
    foreach (user_relationships_types_load() as $rtid => $relationship) {
      $items["relationships/{$rtid}"] = array(
        'title'             => t($relationship->plural_name),
        'type'              => MENU_NORMAL_ITEM|MENU_LOCAL_TASK,
        'access callback'   => 'user_relationships_check_access',
        'access arguments'  => array('edit'),
        'page callback'     => 'theme',
        'page arguments'    => array('user_relationships_page', NULL, 1),
      );
    }

    $items['relationships/%relationship/remove'] = array(
      'title'             => t('Remove relationship'),
      'type'              => MENU_CALLBACK,
      'access callback'   => 'user_relationships_check_access',
      'access arguments'  => array('edit'),
      'page callback'     => 'drupal_get_form',
      'page arguments'    => array('user_relationships_remove', 1),
    );

    
  $items['relationship/%user/request'] = array(
    'title'             => t('Create a relationship'),
    'type'              => MENU_CALLBACK,
    'access callback'   =>  'user_relationships_check_access',
    'access arguments'  => array('edit'),
    'page callback'     => 'drupal_get_form',
    'page arguments'    => array('user_relationships_request', 1),
  );

  $items['user/%user/relationships'] = array(
    'title'             => t('Relationships'),
    'access callback'   => 'user_relationships_check_access',
    'access arguments'  => array('admin'),
    'page callback'     => 'theme',
    'page arguments'    => array('user_relationships_page', 1),
  );
    $items['user/%user/relationships/list'] = array(
      'title'             => t('All'),
      'type'              => MENU_NORMAL_ITEM|MENU_DEFAULT_LOCAL_TASK,
      'weight'            => -10,
      'access callback'   => 'user_relationships_check_access',
      'access arguments'  => array('admin'),
    );
    $items['user/%user/relationships/requests'] = array(
      'title'             => t('Pending'),
      'type'              => MENU_NORMAL_ITEM|MENU_LOCAL_TASK,
      'weight'            => -9,
      'access callback'   => 'user_relationships_check_access',
      'access arguments'  => array('admin'),
      'page callback'     => 'theme',
      'page arguments'    => array('user_relationships_pending_requests_page', 1)
    );

    foreach (user_relationships_types_load() as $rtid => $relationship) {
      $items["user/%user/relationships/{$rtid}"] = array(
        'title'             => t($relationship->plural_name),
        'type'              => MENU_NORMAL_ITEM|MENU_LOCAL_TASK,
        'access callback'   => 'user_relationships_check_access',
        'access arguments'  => array('admin'),
        'page callback'     => 'theme',
        'page arguments'    => array('user_relationships_page', 1, 3),
      );
    }

    foreach (array('approve', 'disapprove', 'cancel') as $action) {
      $items["user/%user/relationships/requested/%relationship/{$action}"] = array(
        'title'             => t('Approve Relationship'),
        'type'              => MENU_CALLBACK,
        'access callback'   => 'user_relationships_check_access',
        'access arguments'  => array(array('admin', 'user'), 1),
        'page callback'     => 'drupal_get_form', 
        'page arguments'    => array('user_relationships_pending_requested', $action, 1, 4),
      );
    }

    $items['user/%user/relationships/%relationship/remove'] = array(
      'title'             => t('Remove relationship'),
      'type'              => MENU_CALLBACK,
      'access callback'   => 'user_relationships_check_access',
      'access arguments'  => array(array('admin', 'user'), 1),
      'page callback'     => 'drupal_get_form',
      'page arguments'    => array('user_relationships_remove', 1, 3),
    );

  return $items;
}

/**
 * User
 */
function user_relationships_user($type, &$edit, &$account, $category = NULL) {
  switch($type) {
  case 'login':
    if (user_access('maintain relationships')) {
      _user_relationships_set_notifications($account);
    }
    break;

  case 'view':
    global $user;
    $viewer =& $user;

    if (($account->uid == $user->uid) && user_access('maintain relationships')) {
      _user_relationships_set_notifications($account);
      break;
    }

    $output = array();
    if ($list = _user_relationships_between($viewer, $account)) {
      $output['relations'] = array(
        '#title'      => t('Your relationships to this user'),
        '#type'       => 'user_profile_item',
        '#value'      => theme('item_list', $list), 
        '#attributes' => array('class' => 'user_relationships'),
      );
    }

    if ($actions = _user_relationships_actions_between($viewer, $account)) {
      $output['actions'] = array(
        '#title'      => t('Relationship actions'), 
        '#type'       => 'user_profile_item',
        '#value'      => theme('item_list', $actions), 
        '#attributes' => array('class' => 'user_relationships_actions'),
      );
    }

    if(sizeof($output)) {
      $account->content['user_relationships'] = array(
        '#type'   => 'user_profile_category',
        '#title'  => t('Relationships'),
      );
      $account->content['user_relationships'] = array_merge($account->content['user_relationships'], $output);
    }
    break;

  case 'delete':
    db_query("DELETE FROM {user_relationships} WHERE requester_id = %d OR requestee_id = %d", $account->uid, $account->uid);
    break;

  case 'form':
    if (($category == 'account') 
      && variable_get('user_relationships_require_approval', TRUE) 
      && user_access('maintain relationships', $account)
    ) {
      $form['user_relationships_settings'] = array(
        '#type'   => 'fieldset',
        '#title'  => t('Relationship settings'),
        '#weight' => 5
      );

      if (variable_get('user_relationships_allow_auto_approve', FALSE) && ($relationships = user_relationships_types_load())) {
        if (!is_array($edit['user_relationships_auto_approve'])) {
          $edit['user_relationships_auto_approve'] = array();
        }

        $options = array();
        foreach ($relationships as $relationship) {
          if ($relationship->requires_approval) {
            $options[$relationship->rtid] = $relationship->name;
          }
        }
        $form['user_relationships_settings']['user_relationships_auto_approve'] = array(
          '#type'           => 'checkboxes',
          '#title'          => t('Automatically approve the following relationship requests'),
          '#options'        => $options,
          '#default_value'  => $edit['user_relationships_auto_approve'],
          '#description'    => t("Check off the types of relationships you'd like to automatically approve.")
        );
      }
    }

    // No options have been set so don't display it
    if (sizeof($form['user_relationships_settings']) == 3) {
      unset($form['user_relationships_settings']);
    }
    return $form;
  }
}


/**
 * Cron
 */
function user_relationships_cron() {
  $now = time();

  // only expire relationships once a day
  $last_cron = variable_get('user_relationships_last_expire', 0);
  if ($now >  $last_cron + (24 * 60 * 60)) {
    return FALSE;
  }

  db_query(
    " DELETE ur
      FROM user_relationships ur, user_relationship_types urt 
      WHERE ur.approved = 0 
        AND ur.rtid = urt.rtid 
        AND urt.expires_val > 0 
        AND %d > (ur.updated_at + (urt.expires_val * 24 * 60 *60))",
    $now
  );

  // remember when we last expired relationships
  variable_set('user_relationships_last_expire', $now);
  return TRUE;
}


/**
 * Theme
 */
function user_relationships_theme() {
  return array(
    'user_relationships_page' => array(
      'file'      => 'user_relationships_theme.inc',
      'arguments' => array('account', 'rtid')
    ),

    'user_relationships_pending_requests_page' => array(
      'file'      => 'user_relationships_theme.inc',
      'arguments' => array('account')
    ),

    'user_relationships_request_relationship_link' => array(
      'file'      => 'user_relationships_theme.inc',
      'arguments' => array('relate_to')
    ),

    'user_relationships_remove_link' => array(
      'file'      => 'user_relationships_theme.inc',
      'arguments' => array('uid', 'rid')
    ),

    'user_relationships_pending_request_approve_link' => array(
      'file'      => 'user_relationships_theme.inc',
      'arguments' => array('uid', 'rid')
    ),

    'user_relationships_pending_request_disapprove_link' => array(
      'file'      => 'user_relationships_theme.inc',
      'arguments' => array('uid', 'rid')
    ),

    'user_relationships_pending_request_cancel_link' => array(
      'file'      => 'user_relationships_theme.inc',
      'arguments' => array('uid', 'rid')
    ),

    'user_relationships_approval_status' => array(
      'file'      => 'user_relationships_theme.inc',
      'arguments' => array('approved')
    ),

    'user_relationships_user_link' => array(
      'file'      => 'user_relationships_theme.inc',
      'arguments' => array('uid')
    ),
  );
}
