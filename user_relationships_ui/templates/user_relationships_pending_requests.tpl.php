<?php
// $Id: user_relationships_pending_requests.tpl.php,v 1.1.2.9 2010-01-03 20:17:43 alexk Exp $
/**
 * @file
 * Page to manage sent and received relationship requests
 */

  $output = '';
  $pager_id = 0;
  $section_headings = array(
    'sent_requests'     => t('Sent Requests'),
    'received_requests' => t('Received Requests')
  );

  $edit_access = ($user->uid == $account->uid && user_access('maintain own relationships')) || user_access('administer user relationships');

  foreach ($sections as $column => $section) {
    if (!isset($$section)) { continue; }
    $rows = array();

    $rows[] = array(
      array('data' => $section_headings[$section], 'header' => TRUE, 'colspan' => 2)
    );

    foreach ($$section as $relationship) {
      $links = array();
      if ($edit_access) {
        if ($section == 'sent_requests') {
          $links[] = theme('user_relationships_pending_request_cancel_link', array('uid' => $account->uid, 'rid' => $relationship->rid));
        }
        else {
          $links[] = theme('user_relationships_pending_request_approve_link', array('uid' => $account->uid, 'rid' => $relationship->rid));
          $links[] = theme('user_relationships_pending_request_disapprove_link', array('uid' => $account->uid, 'rid' => $relationship->rid));
        }
      }
      $links = implode(' | ', $links);

      if ($relationship->requester_id == $account->uid) {
        $rows[]   = array(t('@rel_name to !requestee', array('@rel_name' => ur_tt("user_relationships:rtid:$relationship->rtid:name", $relationship->name), '!requestee' => theme('username', array('account' => $relationship->requestee)))), $links);
      }
      else {
        $rows[]   = array(t('@rel_name from !requester', array('@rel_name' => ur_tt("user_relationships:rtid:$relationship->rtid:name", $relationship->name), '!requester' => theme('username', array('account' => $relationship->requester)))), $links);
      }
    }

    $output .= theme('table', array('rows' => $rows, 'attributes' => array('class' => array('user-relationships-pending-listing-table'))));
    $output .= theme('pager');
  }

  if ($output == '') {
    $output = t('No pending relationships found');
  }

  print $output;
?>
