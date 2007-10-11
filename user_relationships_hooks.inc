<?php
// $Id: user_relationships_hooks.inc,v 1.11 2007-10-11 19:27:04 jbchristy Exp $

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
    
    $id = is_numeric(arg(1)) ? arg(1) : $user->uid;
    $edit_access = ($id == $user->uid && user_access('maintain relationships')) || user_access('administer users');

    $items[] = array(
      'path'                => 'relationships',
      'title'               => t('My relationships'),
      'access'              => ($id && user_access('maintain relationships')),
      'callback'            => 'theme',
      'callback arguments'  => array('user_relationships_page'),
    );

    $items[] = array(
      'path'                => "relationship/request",
      'title'               => t('Create a relationship'),
      'access'              => $edit_access,
      'callback'            => 'drupal_get_form',
      'callback arguments'  => array('user_relationships_request', arg(2)),
      'type'                => MENU_CALLBACK,
    );
    $items[] = array(
      'path'                => "relationship/{$id}/remove",
      'title'               => t('Remove relationship'),
      'access'              => $edit_access,
      'callback'            => 'drupal_get_form',
      'callback arguments'  => array('user_relationships_remove', $id, arg(3)),
      'type'                => MENU_CALLBACK,
    );
    if (true || ($approval_required && $edit_access)) {
      $items[] = array(
        'path'                => "relationships/{$id}/requests",
        'title'               => t('Pending requests'),
        'access'              => $edit_access,
        'callback'            => 'theme',
        'type'                => MENU_LOCAL_TASK,
        'weight'              => 0,
        'callback arguments'  => array('user_relationships_pending_requests_page', $id)
      );
    }

    // 'view only' tabs
    $view_access = (($id == $user->uid && user_access('maintain relationships')) || user_access('view user relationships'));

    $items[] = array(
      'path'                => "relationships/{$id}",
      'title'               => t('Relationships'),
      'access'              => ($view_access && $id),
    );
      $items[] = array(
        'path'                => "relationships/{$id}/list",
        'title'               => t('All'),
        'access'              => $view_access,
        'type'                => MENU_DEFAULT_LOCAL_TASK,
        'weight'              => -1,
        'callback'            => 'theme',
        'callback arguments'  => array('user_relationships_page', $id),
      );

    // operations
    // arg(3) should be one of "approve" "disapprove" or "cancel"
    $items[] = array(
      'path'                => "relationships/{$id}/requested",
      'title'               => t('Approve Relationship'),
      'access'              => $edit_access,
      'type'                => MENU_CALLBACK,
      'callback'            => 'drupal_get_form', 
      'callback arguments'  => array('user_relationships_pending_requested', arg(3), $id, arg(4)),
    );
    $relationships = user_relationships_relationship_types_load();
    foreach ($relationships as $relationship) {
      $items[] = array(
        'path'                => "relationships/{$id}/{$relationship->rtid}",
        'title'               => t($relationship->name),
        'access'              => $view_access,
        'type'                => MENU_LOCAL_TASK,
        'callback'            => 'theme',
        'callback arguments'  => array('user_relationships_page', $id, $relationship),
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
    $viewer = user_load(array('uid' => $user->uid));

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
    cache_clear_all("user_relationships", 'cache_user_relationships', TRUE);
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

      if (variable_get('user_relationships_allow_auto_approve', FALSE) && ($relationships = user_relationships_relationship_types_load())) {
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

/**
 * Implementation for hook_block
 */
define('USER_PAGES', '_USER_PAGES_');
function user_relationships_block($op = 'list', $delta = 0, $edit = array()) {
  $all_types = 'ALL';

  if (0 != strcmp($op, 'list')) {
    $block_rtype = explode('|', $delta);
    $block =  $block_rtype[0]; // 'MY' or 'USER'
    $rtype =  $block_rtype[1]; // valid rtid or $all_types
  }

  $which_rels_options = array(
    1 => 'Newest',
    2 => 'Oldest',
    3 => 'Random',
  );
  $default_option = 1;

  switch ($op) {
    case 'list':
      // return list of all blocks defined by the module
      $relationships = user_relationships_relationship_types_load();
      $block = array();
      foreach ($relationships as $relationship) {
        $block["MY|{$relationship->rtid}"]['info']   = t(
          'My Relationships: @type',
          array('@type' => drupal_ucfirst($relationship->plural_name))
        );
        $block["USER|{$relationship->rtid}"]['info'] = t(
          'User Relationships: @type',
          array('@type' => drupal_ucfirst($relationship->plural_name))
        );
      }
      $block["MY|{$all_types}"]['info']   = t('My Relationships: All relationships');
      $block["USER|{$all_types}"]['info'] = t('User Relationships: All relationships');
      return $block;
      break;

    case 'configure':
      $form = array();
      if ($block == 'MY') {
        $block_desc = t('NOTE: This block displays relationships of the user who is currently logged in.');
      }
      else {
        $block_desc = t('NOTE: This block displays the relationships of the user whose node is being viewed.');
        $page_types = array();
        $node_types = node_get_types();
        foreach ($node_types as $ntype) {
          $page_types[$ntype->type] = $ntype->name;
        }
        asort($page_types);
        $page_types[USER_PAGES] = t('user pages (/user/<uid>)');
        $default_pages = variable_get("user_relationships_block_{$block}_t_{$rtype}_which_pages", array(USER_PAGES));
        $form['user_relationships_block_display_pages'] = array(
          '#type'          => 'select',
          '#multiple'      => TRUE,
          '#options'       => $page_types,
          '#default_value' => $default_pages,
          '#title'         => t('Pages types on which to display this block'),
          '#description'   => t('Select the page types on which you want to display this block.'),
          '#required'      => TRUE,
          '#weight'        => 5,
        );
      }
      $form['user_relationships_block_num_rels'] = array(
        '#type'          => 'textfield',
        '#title'         => t('Number of relationships to display in block'),
        '#description'   => t('Enter the maximum number of relationships to display in this block.'),
        '#size'          => 4,
        '#default_value' => variable_get("user_relationships_block_{$block}_t_{$rtype}_num_rels", 8),
        '#weight'        => 1,
        '#required'      => TRUE,
        '#validate'      => array('user_relationships_block_num_rels_validate' => array())
      );
      $form['user_relationships_block_which_rels'] = array(
        '#type'          => 'radios',
        '#title'         => t('Which relationships should be displayed'),
        '#options'       => $which_rels_options,
        '#default_value' => variable_get("user_relationships_block_{$block}_t_{$rtype}_which_rels", $default_option),
        '#required'      => TRUE,
        '#weight'        => 3,
        '#suffix'        => $block_desc,
      );
      return $form;
      break;

    case 'save':
      variable_set("user_relationships_block_{$block}_t_{$rtype}_num_rels",
                   (int)$edit['user_relationships_block_num_rels']);
      variable_set("user_relationships_block_{$block}_t_{$rtype}_which_rels",
                   (int)$edit['user_relationships_block_which_rels']);
      variable_set("user_relationships_block_{$block}_t_{$rtype}_which_pages",
                   array_keys($edit['user_relationships_block_display_pages']));
      break;

    case 'view':
      global $user;
      // determine which user's relationships to display
      if ('MY' == $block) {
        if ($user->uid) {
          $viewing_user =& $user;
        }
      }
      else {
        $page_types = variable_get("user_relationships_block_{$block}_t_{$rtype}_which_pages", array());
        if (arg(0) == 'node' && is_numeric(arg(1))) {
          $node = node_load(arg(1));
          if (in_array($node->type, $page_types)) {
            $viewing_user = user_load(array('uid' => $node->uid));
          }
        }
        elseif (arg(0) == 'user' && is_numeric(arg(1)) && in_array(USER_PAGES, $page_types)) {
          $viewing_user = user_load(array('uid' => arg(1)));
        }
      }

      // if we don't have a valid user to report on, there's nothing to display
      if (!isset($viewing_user)) {
        return NULL;
      }

      // select the appropriate set of relationships based on admin's configuration settings
      $relationships = user_relationships_block_select_relationships(
         $viewing_user,
         $rtype,
         variable_get("user_relationships_block_{$block}_t_{$rtype}_num_rels", 8),
         $which_rels_options,
         variable_get("user_relationships_block_{$block}_t_{$rtype}_which_rels", $default_option)
       );

      // get the content of the block
      $blk['subject'] = theme(
        'user_relationships_relationships_block_subject',
        $viewing_user, $relationships, $rtype, ($rtype == $all_types) ? TRUE : FALSE
      );
      if ($relationships != NULL) {
        $blk['content'] = theme(
          'user_relationships_relationships_block_content',
          $viewing_user, $relationships, $rtype, ($rtype == $all_types) ? TRUE : FALSE
        );
      }
      else {
        $blk['content'] = theme(
          'user_relationships_relationships_block_empty',
          $viewing_user, $rtype, ($rtype == $all_types) ? TRUE : FALSE
        );
      }
      return $blk;
      break;
  }
}
