<?php
// $Id: user_relationships-pending_block.tpl.php,v 1.1.2.9 2009-09-23 15:22:48 aufumy Exp $
/**
 * @file
 * Template for relationships requests block
 * List all pending requests and provide links to the actions that can be taken on those requests
 */
if ($relationships) {
  $list = array();
  foreach ($relationships as $relationship) {
    $tt_rel_name = tt("user_relationships:rtid:$relationship->rtid:name", $relationship->name);
    $tt_rel_plural_name = tt("user_relationships:rtid:$relationship->rtid:plural_name", $relationship->plural_name);
    if ($user->uid == $relationship->requester_id) {
      $relation_to =& $relationship->requestee;
      $controls = theme('user_relationships_pending_request_cancel_link', $user->uid, $relationship->rid);
      $line = t('@rel_name to !username (!controls)', array('@rel_name' => $tt_rel_name, '!username' => theme('username', $relation_to), '!controls' => $controls));
      $key = t('Sent requests');
    }
    else {
      $relation_to =& $relationship->requester;
      $controls =
        theme('user_relationships_pending_request_approve_link', $user->uid, $relationship->rid).'|'.
        theme('user_relationships_pending_request_disapprove_link', $user->uid, $relationship->rid);
      $line = t('@rel_name from !username (!controls)', array('@rel_name' => $tt_rel_name, '!username' => theme('username', $relation_to), '!controls' => $controls));
      $key = t('Received requests');
    }
    $list[$key][] = $line;
  }

  foreach ($list as $title => $users) {
    $output[] = theme('item_list', $users, $title);
  }
}

print $output ? implode('', $output) : t('No Pending Requests');

?>
