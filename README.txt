/* $Id: README.txt,v 1.3 2007-10-11 19:27:04 jbchristy Exp $ */

User Relationships Module
-------------------------
This module allows users to create named relationships between each other.

Relationship types are defined by the admins.

Multiple relationships are supported and can be enabled or disabled

Send comments to Jeff Smick: http://drupal.org/user/107579/contact


Requirements
------------
Drupal 5


Installation
------------
1. Copy the user_relationships folder to the appropriate Drupal directory.

2. Enable User Relationships in the "Site building -> modules" administration screen.

   Enabling the User Relationships module will trigger the creation of the database schema. 
   If you are shown error messages you may have to create the schema by hand. Please 
   see the database definition at the end of this file.

3. Create relationship types in "User Management -> User Relationships -> Add relationship"


Memcache Users
--------------

The User Relationships module uses drupal's caching API to save results across page loads. The memcache
module replaces drupal's default API with it's own implementations of cache_set, cache_get and
cache_clear_all. Unfortunately, memcache's implementation of cache_clear_all doesn't honor the third
(wildcard) parameter. If you have memcache installed, then the cache will not be properly cleared when
you delete a user or a relationship type, resulting in stale data that may persist indefinitely.

If you have memcache installed, you should implement the following two hooks in your module:

/*
 * Implementation of hook_user()
 */
function YOURMODULE_user($op, &$edit, &$account, $category = NULL) {
  switch ($op) {
    case 'delete':
      if (module_exists('memcache')) {
        // memcache's implementation of cache_clear_all doesn't support wildcarding,
        // so wipe the entire user relationship cache by passing a null cache id
        dpm("Deleting a user - clearing UR cache");
        cache_clear_all(NULL, 'cache_user_relationships');
      }
      break;
  }
}

/*
 * Implementation of hook_user_relationships()
 */
function YOURMODULE_user_relationships($type, &$relationship, $category = NULL) {
  switch ($type) {
    case 'delete type':
      if (module_exists('memcache')) {
        // memcache's implementation of cache_clear_all doesn't support wildcarding,
        // so wipe the entire user relationship cache by passing a null cache id
        cache_clear_all(NULL, 'cache_user_relationships');
      }
      break;
  }
}


Developers
------------
I tried to make this module as modular as possible (is that a horrible sentence? I don't care).
This is the core to a more robust set of features that are built as addons. I tried to make it as
extensible as possible. If you need something more out of the core (and I hope that you don't), please
feel free to get in contact with me (http://drupal.org/user/107579/contact) and we can talk about it.

Take a look at the following files for more information about the API and theme-able functions provided
  user_relationships_api.inc
  user_relationships_theme.inc

The module also invokes a "user_relationships" hook passing in the following argumens:
  $type will be a string of the following
    'insert type'   before a new relationship type is created
    'update type'   before a relationship type is updated
    'delete type'   after a relationship type is deleted
    'load type'     after a relationship type is loaded (so you can add data to it if you'd like)

    'load'          after a relationship is loaded
    'insert'        after a new relationship is created
    'update'        before a relationship is updated
    'delete'        after a relationship is deleted

  $relationship either the relationship_type or relationship object
  
  $category a string currently only used when 'delete' is invoked
    'remove'        when a relationship is removed
    'cancel'        when a relationship request is cancelled
    'disapprove'    when a relationship request is disapprove


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
  `created_at` datetime NOT NULL,
  `updated_at` timestamp NOT NULL,
  PRIMARY KEY  (`rid`),
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
  `is_oneway` tinyint(1) NOT NULL default '0',
  `requires_approval` tinyint(1) NOT NULL default '0',
  PRIMARY KEY  (`rtid`),
  UNIQUE KEY `name` (`name`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;


Credits
-------
Written by Jeff Smick.
Written originally for and financially supported by OurChart Inc. (http://www.ourchart.com)
Thanks to the BuddyList module team for their inspiration
