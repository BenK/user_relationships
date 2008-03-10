<?php
// $Id: user_relationships_pending_requests.tpl.php,v 1.1.2.3 2008-03-10 17:13:32 sprsquish Exp $

  $output = '';
  $pager_id = 0;
  $section_headings = array(
    'sent_requests'     => t('Sent Requests'),
    'received_requests' => t('Received Requests')
  );
  
  foreach ($sections as $column => $section) {
    if (!$$section) { continue; }
    $rows = array();

    foreach ($$section as $relationship) {
      $rows[] = array(
        array('data' => $section_headings[$section], 'header' => TRUE, 'colspan' => 2)
      );

      $links = array();
      if ($section == 'sent_requests') {
        $links[] = theme('user_relationships_pending_request_cancel_link', $account->uid, $relationship->rid);
      }
      else {
        $links[] = theme('user_relationships_pending_request_approve_link',    $account->uid, $relationship->rid);
        $links[] = theme('user_relationships_pending_request_disapprove_link', $account->uid, $relationship->rid);
      }
      $links = implode(' | ', $links);

      $related  = $relationship->{($relationship->requester_id == $account->uid ? 'requestee' : 'requester')};
      $rows[]   = array(theme('username', $related) . ' is a ' . $relationship->name, $links);
    }

    $output .= 
      theme('table', array(), $rows).
      theme('pager', NULL, $relationships_per_page, $pager_id++);
  }

  if ($output == '') {
    $output = t('No pending relationships found');
  }

  print $output;
?>