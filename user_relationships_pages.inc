<?php
// $Id: user_relationships_pages.inc,v 1.3 2007-09-03 19:24:26 sprsquish Exp $

/**
 * Pages related to User Relationships
 */


/**
 * Relationship Types List
 */
function user_relationships_types_list_page() {
  $relationship_types = user_relationships_relationship_types_load();

  $table['headers'] = array(t('Name'), t('Type'), t('Requires Approval'), t('Operations'));
  $table['data'] = array();
  $table['rows'] = array();

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
