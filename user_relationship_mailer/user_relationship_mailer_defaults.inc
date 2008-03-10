<?php
// $Id: user_relationship_mailer_defaults.inc,v 1.1.2.3 2008-03-10 17:13:32 sprsquish Exp $

global $user_relationship_mailer_ops;
$user_relationship_mailer_ops = array('request', 'cancel', 'approve', 'disapprove', 'remove');

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
  $subject = "[@site] @requester_name believes they are a @relationship_name of yours";
  $message = <<<MESSAGE
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

  return array(
    'subject' => $subject,
    'message' => $message,
  );
}

/**
 * Default cancel request message
 */
function user_relationship_mailer_cancel_default() {
  $subject = "[@site] @requester_name has cancelled their request to be your @relationship_name";
  $message = <<<MESSAGE
Hi @requestee_name,

@requester_name has cancelled their request to be a @relationship_name of yours.

Regards,
The @site team

This message was sent to you because you have user relationships email notifications turned on.
If you no longer wish to receive these emails, you can turn them off at @site_url/user/@requestee_uid/edit.
MESSAGE;

  return array(
    'subject' => $subject,
    'message' => $message,
  );
}

/**
 * Default approve to relationship message
 */
function user_relationship_mailer_approve_default() {
  $subject = "[@site] You are @requestee_name's newest @relationship_name";
  $message = <<<MESSAGE
Hi @requester_name,

@requestee_name has approved your request to create a @relationship_name relationship to them.

Here's a link to your relationships:
  @requester_list_link

Regards,
The @site team

This message was sent to you because you have user relationships email notifications turned on.
If you no longer wish to receive these emails, you can turn them off at @site_url/user/@requester_uid/edit.
MESSAGE;

  return array(
    'subject' => $subject,
    'message' => $message,
  );
}

/**
 * Default disapprove to relationship message
 */
function user_relationship_mailer_disapprove_default() {
  $subject = "[@site] @requestee_name did not approve your @relationship_name relationship with them";
  $message = <<<MESSAGE
Hi @requester_name,

@requestee_name did not approve your request to create a @relationship_name relationship with them.

Regards,
The @site team

This message was sent to you because you have user relationships email notifications turned on.
If you no longer wish to receive these emails, you can turn them off at @site_url/user/@requester_uid/edit.
MESSAGE;

  return array(
    'subject' => $subject,
    'message' => $message,
  );
}

/**
 * Default removed relationship message
 */
function user_relationship_mailer_remove_default() {
  $subject = "[@site] @relationship_name between @requester_name and @requestee_name has been removed";
  $message = <<<MESSAGE
@requester_name's @relationship_name relationship to @requestee_name has been removed by @deleted_by_name.

Regards,
The @site team

This message was sent to you because you have user relationships email notifications turned on. 
If you no longer wish to receive these emails, you can turn them off at @site_url/user/@profile_uid/edit.
MESSAGE;

  return array(
    'subject' => $subject,
    'message' => $message,
  );
}
