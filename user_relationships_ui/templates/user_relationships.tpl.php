<?php
// $Id: user_relationships.tpl.php,v 1.1.2.3 2008-03-10 17:13:32 sprsquish Exp $

if ($relationships) {
  foreach ($relationships as $relationship) {
    $edit_access = ($user->uid == $account->uid && user_access('maintain own relationships')) || user_access('administer users');

    $this_user = $account->uid == $relationship->requestee_id ? 'requester' : 'requestee';
    $this_user = $relationship->{$this_user};

    $rows[] = array(
      theme('username', $this_user),
      $relationship->name,
      $relationship->extra_for_display,
      $edit_access ? theme('user_relationships_remove_link', $account->uid, $relationship->rid) : '&nbsp;',
    );
  }

  print
    theme('table', array(), $rows) .
    theme('pager', NULL, $relationships_per_page);
}
else {
  print t('No relationships found');
}
?>