<?php
// $Id: user_relationships_pages.inc,v 1.1 2007-08-15 20:20:01 sprsquish Exp $

/**
 * Pages related to User Relationships
 */


/**
 * Relationship Types List
 */
function user_relationships_types_list_page() {
  $relationship_types = user_relationships_relationship_types_load();

  $table['headers'] = array(t('Name'), t('Type'), t('Requires Approval'), t('Operations'));

  foreach ($relationship_types as $relationship) {
    $table['data'][] = $relationship;
    $table['rows'][] = array(
      $relationship->name,
      ($relationship->is_oneway ? t('one way') : t('mutual')),
      ($relationship->requires_approval ? t('yes') : t('no')),
      l(t('edit'),    'admin/user/relationships/edit/'. $relationship->rtid) . ' | ' .
      l(t('delete'),  'admin/user/relationships/delete/'. $relationship->rtid)
    );
  }

  foreach (module_implements('user_relationships_page_alter') as $module) {
    $function = "{$module}_user_relationships_page_alter";
    $function('types list', $page, $table);
  }

  if (!sizeof($table['rows'])) {
    $table['rows'][] = array(array('data' => t('No relationships available.'), 'colspan' => sizeof($table['headers'])));
  }

  $page['relationships'] = array(
    '#type'   => 'fieldset',
    '#title'  => t('Relationship Types'),
    '#weight' => 0
  );
    $page['relationships']['list'] = array(
      '#value' => theme('table', $table['headers'], $table['rows'])
    );

  return drupal_render($page);
}

/**
 * List of all pending relationships for a given user
 */
function user_relationships_pending_requests_page($uid) {
  global $user;
  $account = user_load(array('uid' => $uid));

  drupal_set_title(t("@username's pending relationships", array('@username' => $account->name)));

  $viewing_own_account = ($user->uid == $account->uid);
  $requests   = user_relationships_get_requests($account->uid);
  $requestees = user_relationships_get_requestees($account->uid);

  $requests_stable['headers'] = array(t('Relationships'), t('Operations'));
  foreach ($requests as $request) {
    $requests_table['data'][] = $request;
    $requests_table['rows'][] = array(
      t('%relationship_name of !username', array('%relationship_name' => $request->name, '!username' => theme('username', $request->user))),
      theme('user_relationships_cancel_relationship_link', $account->uid, $request->uid2, $request->rid),
    );
  }

  $requests_stable['headers'] = array(t('Relationships'), array('data' => t('Operations'), 'colspan' => 2));
  foreach ($requestees as $requestee) {
    $requestees_table['data'][] = $requestee;
    $requestees_table['rows'][] = array(
      t('%relationship_name of !username', array('%relationship_name' => $requestee->name, '!username' => theme('username', $requestee->user))),
      theme('user_relationships_pending_requested_approve_link', $account->uid, $requestee->uid1, $requestee->rid),
      theme('user_relationships_pending_requested_disapprove_link', $account->uid, $requestee->uid1, $requestee->rid),
    );
  }

  foreach (module_implements('user_relationships_page_alter') as $module) {
    $function = "{$module}_user_relationships_page_alter";
    $function('requests', $page, $table);
  }

  if (!sizeof($requests_table['rows'])) {
    $requests_table['rows'][] = array(array('data' => t('No relationships have been sent.'), 'colspan' => sizeof($requests_table['headers'])));
  }

  if (!sizeof($requestees_table['rows'])) {
    $requestees_table['rows'][] = array(array('data' => t('No relationships have been requested.'), 'colspan' => sizeof($requestees_table['headers'])));
  }

  $page['requests'] = array(
    '#type'   => 'fieldset',
    '#title'  => t('Sent Requests')
  );
    $page['requests']['list'] = array(
      '#value'  => theme('table', $requests_table['headers'], $requests_table['rows']),
    );

  $page['requestees'] = array(
    '#type'   => 'fieldset',
    '#title'  => t('Received Requests')
  );
    $page['requestees']['list'] = array(
      '#value'  => theme('table', $requestees_table['headers'], $requestees_table['rows']),
    );

  return drupal_render($page);
}