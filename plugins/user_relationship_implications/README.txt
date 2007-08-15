/* $Id: README.txt,v 1.1 2007-08-15 20:20:01 sprsquish Exp $ */

User Relationship Implications Module
-------------------------------------
This is a plugin module for the User Relationships module.

It allows admins to set up implied relationships (ex: Manager implies Coworker).
These implies relationships will be automatically created. If a relationship that is
implied by another is deleted the implied by relationship is also deleted.

Implied relationships can be chained (ex: Manager implies Coworker implies Officemate)

Send comments to Jeff Smick: http://drupal.org/user/107579/contact


Scenarios
---------
User creates a "manager" relationship to another user: The "coworker" relationship will
be automatically created between them.

User removes a "coworker" relationship to another user: The "manager" relationship will
be automatically deleted.


Requirements
------------
Drupal 5
User Relationships Module


Installation
------------
Enable User Relationship Implications in the "Site building -> modules" administration screen.


Database Schema
---------------
MySQL
=====

-- 
-- Table structure for table `user_relationship_implications`
-- 
CREATE TABLE IF NOT EXISTS `user_relationship_implications` (
  `riid` int(10) unsigned NOT NULL default '0',
  `rtid` int(10) unsigned NOT NULL default '0',
  `implies_rtid` int(10) unsigned NOT NULL default '0',
  PRIMARY KEY  (`riid`),
  KEY `rtid` (`rtid`),
  KEY `implies_rtid` (`implies_rtid`)
) ENGINE=MyISAM DEFAULT CHARSET=utf8;


Credits
-------
Written by Jeff Smick.
Written originally for and financially supported by OurChart Inc. (http://www.ourchart.com)
Thanks to the BuddyList module team for their inspiration
