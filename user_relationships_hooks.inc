<?php
// $Id: user_relationships_hooks.inc,v 1.14 2007-12-21 10:03:37 sprsquish Exp $

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
function user_relationships_menu($may_cache) {
  $items = array();
  if ($may_cache) {
    $items[] = array(
      'path'                => 'admin/user/relationships',
      'title'               => t('Relationships'),
      'description'         => t('Create relationship types'),
      'access'              => user_access('administer user relationships'),
      'callback'            => 'user_relationships_types_list_page',
    );
      $items[] = array(
        'path'      => 'admin/user/relationships/list',
        'title'     => t('List'),
        'callback'  => 'user_relationships_types_list_page',
        'access'    => user_access('administer user relationships'),
        'type'      => MENU_DEFAULT_LOCAL_TASK,
      );
      $items[] = array(
        'path'                => 'admin/user/relationships/add',
        'title'               => t('Add type'),
        'callback'            => 'drupal_get_form',
        'callback arguments'  => array('user_relationships_type_edit'),
        'access'              => user_access('administer user relationships'),
        'type'                => MENU_LOCAL_TASK,
        'weight'              => 1,
      );
      $items[] = array(
        'path'                => 'admin/user/relationships/edit',
        'title'               => t('Edit type'),
        'callback'            => 'drupal_get_form',
        'callback arguments'  => array('user_relationships_type_edit'),
        'access'              => user_access('administer user relationships'),
        'type'                => MENU_CALLBACK,
      );
      $items[] = array(
        'path'                => 'admin/user/relationships/delete',
        'title'               => t('Delete type'),
        'callback'            => 'drupal_get_form',
        'callback arguments'  => array('user_relationships_type_delete'),
        'access'              => user_access('administer user relationships'),
        'type'                => MENU_CALLBACK,
      );
    $items[] = array(
      'path'                => 'admin/user/relationships/settings',
      'title'               => t('Settings'),
      'callback'            => 'drupal_get_form',
      'callback arguments'  => array('user_relationships_settings'),
      'access'              => user_access('administer user relationships'),
      'type'                => MENU_LOCAL_TASK,
      'weight'              => 2,
    );

    $items[] = array(
      'path'                => 'relationship_types/autocomplete',
      'title'               => t('User Relationships Autocomplete'),
      'callback'            => '_user_relationships_autocomplete_types',
      'access'              => user_access('administer user relationships'),
      'type'                => MENU_CALLBACK
    );
  }
  else {
    global $user;

    $relationships = user_relationships_types_load();

    $id = is_numeric(arg(1)) ? arg(1) : $user->uid;
    $edit_access = ($id == $user->uid && user_access('maintain relationships')) || user_access('administer users');

    $items[] = array(
      'path'                => 'relationships',
      'title'               => t('My relationships'),
      'access'              => ($id && user_access('maintain relationships')),
      'type'                => MENU_NORMAL_ITEM,
      'callback'            => 'theme',
      'callback arguments'  => array('user_relationships_page'),
    );
      $items[] = array(
        'path'                => "relationships/list",
        'title'               => t('All'),
        'access'              => $view_access,
        'type'                => MENU_NORMAL_ITEM|MENU_DEFAULT_LOCAL_TASK,
        'weight'              => -10,
        'callback'            => 'theme',
        'callback arguments'  => array('user_relationships_page', $user->id),
      );
      if (true || ($approval_required && $edit_access)) {
        $items[] = array(
          'path'                => "relationships/requests",
          'title'               => t('Pending'),
          'access'              => $edit_access,
          'callback'            => 'theme',
          'type'                => MENU_NORMAL_ITEM|MENU_LOCAL_TASK,
          'weight'              => -9,
          'callback arguments'  => array('user_relationships_pending_requests_page', $user->uid)
        );
      }
      foreach ($relationships as $rtid => $relationship) {
        $items[] = array(
          'path'                => "relationships/{$rtid}",
          'title'               => t($relationship->name),
          'access'              => $view_access,
          'type'                => MENU_NORMAL_ITEM|MENU_LOCAL_TASK,
          'callback'            => 'theme',
          'callback arguments'  => array('user_relationships_page', $id, $rtid),
        );
      }

      
    $items[] = array(
      'path'                => "relationship/request",
      'title'               => t('Create a relationship'),
      'access'              => $edit_access,
      'callback'            => 'drupal_get_form',
      'callback arguments'  => array('user_relationships_request', arg(2)),
      'type'                => MENU_CALLBACK,
    );

    // 'view only' tabs
    $view_access = (($id == $user->uid && user_access('maintain relationships')) || user_access('view user relationships'));

    $items[] = array(
      'path'                => "user/{$id}/relationships",
      'title'               => t('Relationships'),
      'access'              => ($view_access && $id),
    );
      $items[] = array(
        'path'                => "relationships/{$id}/list",
        'title'               => t('All'),
        'access'              => $view_access,
        'type'                => MENU_DEFAULT_LOCAL_TASK,
        'weight'              => -10,
        'callback'            => 'theme',
        'callback arguments'  => array('user_relationships_page', $id),
      );
      if (true || ($approval_required && $edit_access)) {
        $items[] = array(
          'path'                => "user/{$id}/relationships/requests",
          'title'               => t('Pending'),
          'access'              => $edit_access,
          'callback'            => 'theme',
          'type'                => MENU_LOCAL_TASK,
          'weight'              => -9,
          'callback arguments'  => array('user_relationships_pending_requests_page', $id)
        );
      }

      // operations
      if (in_array(arg(4), array('approve', 'disapprove', 'cancel'))) {
        $items[] = array(
          'path'                => "user/{$id}/relationships/requested",
          'title'               => t('Approve Relationship'),
          'access'              => $edit_access,
          'type'                => MENU_CALLBACK,
          'callback'            => 'drupal_get_form', 
          'callback arguments'  => array('user_relationships_pending_requested', arg(4), $id, arg(5)),
        );
      }
      $items[] = array(
        'path'                => "user/{$id}/relationship/remove",
        'title'               => t('Remove relationship'),
        'access'              => $edit_access,
        'callback'            => 'drupal_get_form',
        'callback arguments'  => array('user_relationships_remove', $id, arg(4)),
        'type'                => MENU_CALLBACK,
      );

      foreach ($relationships as $rtid => $relationship) {
        $items[] = array(
          'path'                => "user/{$id}/relationships/{$rtid}",
          'title'               => t($relationship->name),
          'access'              => $view_access,
          'type'                => MENU_LOCAL_TASK,
          'callback'            => 'theme',
          'callback arguments'  => array('user_relationships_page', $id, $rtid),
        );
      }
    }

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
    }

    $output = array();
    if ($viewer != $account && $list = _user_relationships_between($viewer, $account)) {
      $output[] = array(
        'title' => t('Your relationships to this user'),
        'value' => theme('item_list', $list), 
        'class' => 'user_relationships'
      );
    }

    if ($actions = _user_relationships_actions_between($viewer, $account)) {
      $output[] = array(
        'title' => t('Relationship actions'), 
        'value' => theme('item_list', $actions), 
        'class' => 'user_relationships_actions'
      );
    }

    if(sizeof($output)) {
      return array(t('Relationships') => $output);
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

  switch ($GLOBALS['db_type']) {
  case 'mysql':
  case 'mysqli':
    db_query(
      " DELETE ur
        FROM user_relationships ur, user_relationship_types urt 
        WHERE ur.approved = 0 
          AND ur.rtid = urt.rtid 
          AND urt.expires_val > 0 
          AND NOW() > DATE_ADD(ur.updated_at, INTERVAL urt.expires_val DAY)"
    );
    break;
  }

  // remember when we last expired relationships
  variable_set('user_relationships_last_expire', $now);
  return TRUE;
}

