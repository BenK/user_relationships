<?php
// $Id: user_relationships.tpl.php,v 1.1.2.1 2008-03-02 00:51:52 sprsquish Exp $

if ($relationships) {
  foreach ($relationships as $relationship) {
    $edit_access = ($user->uid == $account->uid && user_access('maintain own relationships')) || user_access('administer users');

    $this_user = $account->uid == $relationship->requestee_id ? 'requester' : 'requestee';
    $this_user = $relationship->{$this_user};

    $rows[] = array(
      theme('username', $this_user),
      $relation->name,
      $this_user->access > $online_interval ? t('online') : t('not online'),
      $edit_access ? theme('user_relationships_remove_link', $account->uid, $relation->rid) : '&nbsp;',
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