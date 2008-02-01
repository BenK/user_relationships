<?php
// $Id: user_relationship_mailer_defaults.inc,v 1.7.2.2 2008-02-01 23:43:25 sprsquish Exp $

/**
 * Default subject lines
 */
define('USER_RELATIONSHIP_MAILER_REQUEST_SUBJECT',    variable_get('user_relationships_request_subject',      "[@site] @requester_name believes they are a @relationship_name of yours"));
define('USER_RELATIONSHIP_MAILER_CANCEL_SUBJECT',     variable_get('user_relationships_cancel_subject',       "[@site] @requester_name has cancelled their request to be your @relationship_name"));
define('USER_RELATIONSHIP_MAILER_APPROVE_SUBJECT',    variable_get('user_relationships_approve_subject',      "[@site] You are @requestee_name's newest @relationship_name"));
define('USER_RELATIONSHIP_MAILER_DISAPPROVE_SUBJECT', variable_get('user_relationships_disapproves_subject',  "[@site] @requestee_name did not approve your @relationship_name relationship with them"));
define('USER_RELATIONSHIP_MAILER_REMOVE_SUBJECT',     variable_get('user_relationships_remove_subject',       "[@site] @relationship_name between @requester_name and @requestee_name has been removed"));

/**
 * Replacements for mail messages
 */
function user_relationship_mailer_replacements(&$relationship) {
  $requester = $relationship->requester;
  $requestee = $relationship->requestee;
  $replacements = array(
    '@site'                     => variable_get('site_name', 'Drupal'),
    '@site_url'                 => $GLOBALS['base_url'],

    '@requester_name'           => $requester->name,
    '@requester_link'           => theme('user_relationships_user_link', $requester->uid),
    '@requester_uid'            => $requester->uid,
    '@requester_list_link'      => url("user/{$requester->uid}/relationships/list", array('absolute' => TRUE)),
    '@requester_pending_link'   => url("user/{$requester->uid}/relationships/requests", array('absolute' => TRUE)),

    '@requestee_name'           => $requestee->name,
    '@requestee_link'           => theme('user_relationships_user_link', $requestee->uid),
    '@requestee_uid'            => $requestee->uid,
    '@requestee_list_link'      => url("user/{$requestee->uid}/relationships/list", array('absolute' => TRUE)),
    '@requestee_pending_link'   => url("user/{$requestee->uid}/relationships/requests", array('absolute' => TRUE)),

    '@relationship_name'        => $relationship->name,
    '@relationship_plural_name' => $relationship->plural_name,
  );

  if ($deleted_by = $relationship->deleted_by) {
    $replacements['@deleted_by_name'] = $deleted_by->name;
    $replacements['@deleted_by_link'] = url("user/{$deleted_by->uid}", array('absolute' => TRUE));
    $replacements['@deleted_by_uid']  = $deleted_by->uid;
  }

  return $replacements;
}

/**
 * Default request relationship message
 */
function user_relationship_mailer_request_default() {
  return <<<MESSAGE
Hi @requestee_name,

@requester_name believes they are a @relationship_name of yours.

Here's a link to @requester_name's profile:
  @requester_link

To approve/disapprove this, log in to @site_url and see your pending relationship requests at @requestee_pending_link.

Regards,
The @site team
This message was sent to you because you have user relationships email notifications turned on.
If you no longer wish to receive these emails, you can turn them off at @site_url/user/@requestee_uid/edit.
MESSAGE;
}

/**
 * Default cancel request message
 */
function user_relationship_mailer_cancel_default() {
  return <<<MESSAGE
Hi @requestee_name,

@requester_name has cancelled their request to be a @relationship_name of yours.

Regards,
The @site team

This message was sent to you because you have user relationships email notifications turned on.
If you no longer wish to receive these emails, you can turn them off at @site_url/user/@requestee_uid/edit.
MESSAGE;
}

/**
 * Default approve to relationship message
 */
function user_relationship_mailer_approve_default() {
  return <<<MESSAGE
Hi @requester_name,

@requestee_name has approved your request to create a @relationship_name relationship to them.

Here's a link to your relationships:
  @requester_list_link

Regards,
The @site team

This message was sent to you because you have user relationships email notifications turned on.
If you no longer wish to receive these emails, you can turn them off at @site_url/user/@requester_uid/edit.
MESSAGE;
}

/**
 * Default disapprove to relationship message
 */
function user_relationship_mailer_disapprove_default() {
  return <<<MESSAGE
Hi @requester_name,

@requestee_name did not approve your request to create a @relationship_name relationship with them.

Regards,
The @site team

This message was sent to you because you have user relationships email notifications turned on.
If you no longer wish to receive these emails, you can turn them off at @site_url/user/@requester_uid/edit.
MESSAGE;
}

/**
 * Default removed relationship message
 */
function user_relationship_mailer_remove_default() {
  return <<<MESSAGE
@requester_name's @relationship_name relationship to @requestee_name has been removed by @deleted_by_name.

Regards,
The @site team

This message was sent to you because you have user relationships email notifications turned on. 
If you no longer wish to receive these emails, you can turn them off at @site_url/user/@profile_uid/edit.
MESSAGE;
}
