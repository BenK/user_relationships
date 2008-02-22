<?php

/**
 * In our case it's a delegator block that just pulls data in from
 * whichever function is more appropriate
 *
 * You can just override the whole thing and send back whatever content you'd like
 */
function theme_user_relationship_block($account, $relationships, $block_type, $rtid, $extra) {
  return array(
    'subject' => theme(
      'user_relationship_block_subject',
      $account, $rtid, $relationships
    ),
    'content' => theme(
      'user_relationship_block_'.($relationships ? 'content' : 'empty'),
      $account, $rtid, $relationships
    ),
  );  
}


/**
 * Generate the title of a My/User Relationships block
 */
function theme_user_relationship_block_subject($account, $rtid, $relationships) {
  if (!$account) { return; }

  global $user;
  $user_name = ($account->uid == $user->uid) ? 'My' : "{$account->name}'s";
  if ($rtid == UR_BLOCK_ALL_TYPES) {
    $type_name = 'Relationships';
  }
  else {
    $rtype      = user_relationships_type_load($rtid);
    $type_name  = $rtype->plural_name;
  }
  $output = t('@user @type', array('@user' => $user_name, '@type' => $type_name));
  return $output;
}


/**
 * Generate the content of a non-empty My/User Relationships block
 */
function theme_user_relationship_block_content($viewing_user, $rtid, $relationships) {
  if (!$viewing_user) { return; }

  $rows = array();
  foreach ($relationships as $relationship) {
    if ($viewing_user->uid == $relationship->requester_id) {
      $extra = $relationship->is_oneway ? t(" (You to Them)") : NULL;
      $relatee = user_load(array('uid' => $relationship->requestee_id));
    }
    else {
      $extra = $relationship->is_oneway ? t(" (Them to You)") : NULL;
      $relatee = user_load(array('uid' => $relationship->requester_id));
    }

    $title = "{$relationship->plural_name}{$extra}";

    $username = theme('username', $relatee);
    if ($rtid == UR_BLOCK_ALL_TYPES) {
      $rows[$title]['data'] = $title;
      $rows[$title]['children'][] = $username;
    }
    else {
      $rows[$title][] = $username;
    }
  }

  foreach ($rows as $title => $users) {
    $output[] = theme('item_list', array($users));
  }

  return implode('', $output);
}


/**
 * Generate the content of an empty My/User Relationships block
 */
function theme_user_relationship_block_empty($account, $rtid) {
  if (!$account) { return; }

  global $user;

  if ($rtid == UR_BLOCK_ALL_TYPES) {
    $rtype_name = 'relationships';
  }
  else {
    $rtype      = user_relationships_type_load($rtid);
    $rtype_name = $rtype->plural_name;
  }

  if ($account->uid == $user->uid) {
    return t('You have no @rels', array('@rels' => $rtype_name));
  }
  else {
    return t('!name has no @rels', array('!name' => theme('username', $account), '@rels' => $rtype_name));
  }
  
}


/**
 * Generate the content for the pending block
 */
function theme_user_relationship_block_pending($account, $block_type, $rtype, $extra) {
  if (!$account) { return; }

  global $user;

  $relationships = user_relationships_load(array('user' => $account->uid, 'approved' => 0));

  $list = array();
  foreach ($relationships as $relationship) {
    if ($user->uid == $relationship->requester_id) {
      $relation_to = $relationship->requestee_id;
      $key = t('From You');
      $controls = theme('user_relationships_pending_request_cancel_link', $user->uid, $relationship->rid);
    }
    else {
      $relation_to = $relationship->requester_id;
      $key = t('To You');
      $controls = 
        theme('user_relationships_pending_request_approve_link', $user->uid, $relationship->rid).'|'.
        theme('user_relationships_pending_request_disapprove_link', $user->uid, $relationship->rid);
    }
    $list[$key][] = t('!username (!controls)', array(
      '!username' => theme('username', user_load(array('uid' => $relation_to))),
      '!controls' => $controls
    ));
  }

  foreach ($list as $title => $users) {
    $output[] = theme('item_list', $users, $title);
  }

  if ($output) {
    return implode('', $output);
  }
  else {
    return t('No Pending Requests');
  }
}


/**
 * Generate the content for the actions block
 */
function theme_user_relationship_block_actions($account, $block_type, $rtype, $extra) {
  if (!$account) { return; }

  global $user;
  $viewer =& $user;

  $output = array();
  if ($viewer != $account && $list = _user_relationships_between($viewer, $account)) {
    $output[] = theme('item_list', $list, t('Your relationships to this user'), 'ul', array('class' => 'user_relationships'));
  }

  if ($actions = _user_relationships_actions_between($viewer, $account)) {
    $output[] = theme('item_list', $actions, t('Relationship actions'), 'ul', array('class' => 'user_relationships_actions'));
  }

  return implode('', $output);
}
