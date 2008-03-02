<?php
// $Id: user_relationships-block.tpl.php,v 1.1.2.1 2008-03-02 00:51:52 sprsquish Exp $

// List the relationships between the viewed user and the current user
if ($relationships) {
  $the_other_uid = $settings->block_type == UR_BLOCK_MY ? $user->uid : $account->uid;

  $rows = array();
  foreach ($relationships as $rtid => $relationship) {
    if ($the_other_uid == $relationship->requester_id) {
      $extra = $relationship->is_oneway ? t(" (You to Them)") : NULL;
      $relatee = $relationship->requestee;
    }
    else {
      $extra = $relationship->is_oneway ? t(" (Them to You)") : NULL;
      $relatee = $relationship->requester;
    }

    $title = "{$relationship->plural_name}{$extra}";

    $username = theme('username', $relatee);
    if ($settings->rtid == UR_BLOCK_ALL_TYPES) {
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

  print implode('', $output);
}

// No relationships so figure out how we present that
else {
  if ($settings->rtid == UR_BLOCK_ALL_TYPES) {
    $rtype_name = 'relationships';
  }
  else {
    $rtype      = user_relationships_type_load($settings->rtid);
    $rtype_name = $rtype->plural_name;
  }

  if ($account->uid == $user->uid) {
    print t('You have no @rels', array('@rels' => $rtype_name));
  }
  else {
    print t('!name has no @rels', array('!name' => theme('username', $account), '@rels' => $rtype_name));
  }
}

?>