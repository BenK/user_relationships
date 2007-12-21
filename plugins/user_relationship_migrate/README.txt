/* $Id: README.txt,v 1.4 2007-12-21 10:03:38 sprsquish Exp $

User Relationship Migrate Module
---------------------------------

This is a plugin module for the User Relationships module. It allows admins to migrate
Buddy List relationships to User Relationships.

Send comments to Jeff Smick: http://drupal.org/user/107579/contact, or post an issue at
http://drupal.org/project/user_relationships.


Requirements
------------
Drupal 5
User Relationships Module
Buddy List Module


Installation
------------
Enable User Relationship Migrate in the "Site building -> Modules" administration screen.


Usage
-----
After enabling the module, go to the "User management -> Relationships" page
(admin/user/relationships) and click on "Migrate buddylist" in the menu across the top.

Enter a relationship type for the migrated relationships, and click Migrate. 

Tested on a MacBook Pro 2.4GHz Core2Duo with 4GB of RAM a 706,000 row buddylist table
took ~40 seconds to migrate.

Running this module more than once without clearing the previous migration will cause
UNIQUE KEY errors in the table. This is to help eliminate duplicate entries. 

After running this you can disable the module as you will no longer need it.


Credits
-------
Written by JB Christy.
Refactored by Jeff Smick.
Written originally for and financially supported by OurChart Inc. (http://www.ourchart.com)
Thanks to Jeff Smick for the User Relationships module, and to the Buddy List folks.
