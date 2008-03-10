<?php
// $Id: user_relationships-pending_block.tpl.php,v 1.1.2.3 2008-03-10 17:13:31 sprsquish Exp $

// List all pending requests and provide links to the actions that can be taken on those requests
if ($relationships) {
  $list = array();
  foreach ($relationships as $relationship) {
    if ($user->uid == $relationship->requester_id) {
      $relation_to =& $relationship->requestee;
      $key = t('From You');
      $controls = theme('user_relationships_pending_request_cancel_link', $user->uid, $relationship->rid);
    }
    else {
      $relation_to =& $relationship->requester;
      $key = t('To You');
      $controls = 
        theme('user_relationships_pending_request_approve_link', $user->uid, $relationship->rid).'|'.
        theme('user_relationships_pending_request_disapprove_link', $user->uid, $relationship->rid);
    }
    $list[$key][] = t('!username (!controls)', array(
      '!username' => theme('username', $relation_to),
      '!controls' => $controls
    ));
  }

  foreach ($list as $title => $users) {
    $output[] = theme('item_list', $users, $title);
  }
}

print $output ? implode('', $output) : t('No Pending Requests');

?>