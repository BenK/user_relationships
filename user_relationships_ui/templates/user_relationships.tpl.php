<?php
// $Id: user_relationships.tpl.php,v 1.1.2.6 2009-02-26 23:07:34 alexk Exp $

//$relationships array is loaded in template_preprocess_user_relationships()
if ($relationships) {
  foreach ($relationships as $relationship) {
    $edit_access = ($user->uid == $account->uid && user_access('maintain own relationships')) || user_access('administer users');

    $this_user_str  = $account->uid == $relationship->requestee_id ? 'requester' : 'requestee';
    $this_user      = $relationship->{$this_user_str};

    $row = array(
      theme('username', $this_user),
      $relationship->name . ($relationship->is_oneway ? ($this_user_str == 'requester' ? t(' (You to Them)') : t(' (Them to You)')) : NULL),
      $relationship->extra_for_display,
      $edit_access ? theme('user_relationships_remove_link', $account->uid, $relationship->rid) : '&nbsp;',
    );
    if (variable_get('user_relationships_show_user_pictures', 0)) {
      array_unshift($row, theme('user_picture', $this_user)); 
    }
    $rows[] = $row;
  }

  print
    theme('table', array(), $rows) .
    theme('pager', NULL, $relationships_per_page);
}
else {
  print t('No relationships found');
}
?>
