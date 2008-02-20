/* $Id: README.txt,v 1.3.2.4 2008-02-20 01:38:44 sprsquish Exp $ */

User Relationship Blocks Module
-------------------------
This module allows users to add blocks for user relationships.

Send comments to Jeff Smick: http://drupal.org/user/107579/contact, or post an issue at
http://drupal.org/project/user_relationships.


Requirements
------------
Drupal 6
User Relationships


Installation
------------
1. Enable User Relationship Blocks in the "Site building -> modules" administration screen.



Developers
------------
This module provides a single hook that passes in identifying information about the block. Depending on the
$op passed in it will expect different information back.

  hook_user_relationship_blocks($op, $block_type, $rtid, $extra);

  $op       | expected return
  -----------------------------------------------------------------------------------------------------------------------------
  display   | A user object. It's a way for developers to display relationship information on non-standard pages. If nothing
            | is sent back the module will check if arg(0) matches "user" or "node" and pull the account information from that.

  
  $block_type   | description
  -----------------------------------------------------------------------------------------------------------------------------
  UR_BLOCK_MY   | the block relates to the currently logged in user
  UR_BLOCK_USER | the block relates to a specified user (normally the author of the current node)
  pending       | pending relationship requests, normally for the currently logged in user
  actions       | relationships and actions between the currently logged in user and normally the author of the current node


  $rtid
  -----------------------------------------------------------------------------------------------------------------------------
  Either UR_BLOCK_ALL_TYPES for any relationship type or an rtid corresponding to a specific relationship type
  I use "normally" because those values can be overridden by the hook


  $extra
  -----------------------------------------------------------------------------------------------------------------------------
  Any extra data about the block. Currently this is only used to specify which side of a one-way relationship should be shown.
  It will be either "you_to_them" or "them_to_you" where "you" means the currently logged in user and "them" means the specified
  user.



Credits
-------
Originally Written by JB Christy.
Refactored by Jeff Smick.
Written originally for and financially supported by OurChart Inc. (http://www.ourchart.com)
Thanks to the BuddyList module team for their inspiration
