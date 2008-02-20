/* $Id: README.txt,v 1.6.2.1 2008-02-20 01:17:29 sprsquish Exp $

User Relationships Module
-------------------------
This module allows users to create named relationships between each other.

Relationship types are defined by the admins.

Multiple relationships are supported and can be enabled or disabled

Send comments to Jeff Smick: http://drupal.org/user/107579/contact, or post an issue at
http://drupal.org/project/user_relationships.


Requirements
------------
Drupal 6


Installation
------------
1. Copy the user_relationships folder to the appropriate Drupal directory.

2. Enable User Relationships in the "Site building -> modules" administration screen.

   Enabling the User Relationships module will trigger the creation of the database schema. 
   If you are shown error messages you may have to create the schema by hand. Please 
   see the database definition at the end of this file.

3. Create relationship types in "User Management -> User Relationships -> Add relationship"



Developers
------------
I tried to make this module as modular as possible (is that a horrible sentence? I don't care).
This is the core to a more robust set of features that are built as plugins. I tried to make it as
extensible as possible. If you need something more out of the core (and I hope that you don't), please
feel free to get in contact with me (http://drupal.org/user/107579/contact) and we can talk about it.

Take a look at the following files for more information about the API and theme-able functions provided
  user_relationships_api.inc
  user_relationships_theme.inc

The module also invokes a "user_relationships" hook passing in the following argumens:
  $type will be a string of the following
    $type     | $category   | Description
    ----------------------------------------------------------------------
    insert    | type        | before a new relationship type is created
    update    | type        | before a relationship type is updated
    delete    | type        | after a relationship type is deleted
    load      | type        | after a relationship type is loaded (so you can add data to it if you'd like)

    load      | NULL        | after a relationship is loaded
    insert    | NULL        | after a new relationship is created
    update    | NULL        | before a relationship is updated
    delete    | remove      | when a relationship is removed
    delete    | cancel      | when a relationship request is cancelled
    delete    | disapprove  | when a relationship request is disapprove
    

  $relationship either the relationship_type or relationship object


Database Schema
---------------
MySQL
=====

--
-- Table structure for table `user_relationships`
--

CREATE TABLE IF NOT EXISTS `user_relationships` (
  `rid` int(10) unsigned NOT NULL default '0',
  `requester_id` int(11) NOT NULL default '0',
  `requestee_id` int(11) NOT NULL default '0',
  `rtid` int(11) NOT NULL default '0',
  `approved` tinyint(1) NOT NULL default '0',
  `created_at` int(11) NOT NULL,
  `updated_at` int(11) NOT NULL,
  UNIQUE KEY `requester_id_2` (`requester_id`,`requestee_id`,`rtid`),
  KEY `rid` (`rid`),
  KEY `requester_id` (`requester_id`),
  KEY `requestee_id` (`requestee_id`),
  KEY `rtid` (`rtid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;

--
-- Table structure for table `user_relationship_types`
--
CREATE TABLE IF NOT EXISTS `user_relationship_types` (
`rtid` int(10) unsigned NOT NULL default '0',
`name` varchar(255) NOT NULL default '',
`plural_name` varchar(255) NOT NULL default '',
`is_oneway` tinyint(1) NOT NULL default '0',
`requires_approval` tinyint(1) NOT NULL default '0',
`expires_val` int(10) unsigned NOT NULL default 0,
PRIMARY KEY (`rtid`),
UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;


Credits
-------
Written by Jeff Smick.
Originally written for and financially supported by OurChart Inc. (http://www.ourchart.com)
Thanks to the BuddyList module team for their inspiration
