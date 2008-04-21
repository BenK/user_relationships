<?php
// $Id: user_relationships.tpl.php,v 1.1.2.4 2008-04-21 00:38:19 sprsquish Exp $

if ($relationships) {
  foreach ($relationships as $relationship) {
    $edit_access = ($user->uid == $account->uid && user_access('maintain own relationships')) || user_access('administer users');

    $this_user_str  = $account->uid == $relationship->requestee_id ? 'requester' : 'requestee';
    $this_user      = $relationship->{$this_user_str};

    $rows[] = array(
      theme('username', $this_user),
      $relationship->name . ($relationship->is_oneway ? ($this_user_str == 'requester' ? t(' (You to Them)') : t(' (Them to You)')) : NULL),
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
