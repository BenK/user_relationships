<?php
// $Id: user_relationships-actions_block.tpl.php,v 1.1.2.3 2008-03-10 17:13:31 sprsquish Exp $

$output = array();

// List of current relationships to the viewed user
if ($current_relationships) {
  $output[] = theme('item_list', $current_relationships, t('Your relationships to this user'), 'ul', array('class' => 'user_relationships'));
}

// List of actions that can be taken between the current and viewed user
if ($actions) {
  $output[] = theme('item_list', $actions, t('Relationship actions'), 'ul', array('class' => 'user_relationships_actions'));
}

print implode('', $output);

?>