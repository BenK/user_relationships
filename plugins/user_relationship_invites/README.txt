/* $Id: README.txt,v 1.1 2007-08-15 20:20:01 sprsquish Exp $ */

User Relationship Invites Module
--------------------------------
This is a plugin module for the User Relationships and Invite modules.

It allows users to invite friends and specify a relationship at invite time.

Send comments to Jeff Smick: http://drupal.org/user/107579/contact


Requirements
------------
Drupal 5
User Relationships Module
Invite


Installation
------------
Enable User Relationship Invites in the "Site building -> modules" administration screen.


Database Schema
---------------
MySQL
=====

-- 
-- Table structure for table `user_relationship_invites`
-- 
CREATE TABLE IF NOT EXISTS `user_relationship_invites` (
  `inviter_uid` int(10) unsigned NOT NULL default '0',
  `rtid` int(10) unsigned NOT NULL default '0',
  `invite_code` varchar(64) NOT NULL default '',
  KEY `invite_code` (`invite_code`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;


Credits
-------
Written by Jeff Smick.
Written originally for and financially supported by OurChart Inc. (http://www.ourchart.com)
Thanks to the BuddyList module team for their inspiration
