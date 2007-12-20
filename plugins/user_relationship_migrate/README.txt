/* $Id: README.txt,v 1.3 2007-12-20 16:39:20 sprsquish Exp $ */

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
Enter a relationship type for the migrated relationships, and click Migrate. Depending on
how many buddies are in your database, this could take several minutes, and the status /
confirmation page won't display until it's complete. [On my server, it took about 10
minutes to migrate 630,000 buddylist entries.] Don't click Migrate more than once, and
don't navigate away from the page, or you won't see the status / confirmation page that
tells you the results of the migration.

When the migration completes successfully, you will see the confirmation page, and the
module will automatically disable itself, since it's work is done.

Because the migration deals with a large quantity of data, it's very possible that it may
encounter errors during the migration. If it encounters more than 100 errors inserting
data, it will automatically abort the migration. PHP may also run out of memory, or other
errors may occur. The module has logic built in to keep track of its progress, and to pick
up where it left off in the event you need to restart it. It also has logic to prevent it
from being run more than once simultaneously, in case you accidentally click the Migrate
button twice. [However, if you do click Migrate more than once, you'll never see the page
telling you the results of the migration, so try not to do that.] Finally, it has logic to
prevent it from being run a second time after having completed successfully.


PLEASE NOTE: Due to overhead and performance considerations, this module creates
relationships without checking to see if the given relationship already exists. You should
therefore run this migration before making User Relationships module available to your
users, to minimize the likelihood that they manually recreate their relationships prior to
you migrating them.


Credits
-------
Written by JB Christy.
Refactored by Jeff Smick.
Written originally for and financially supported by OurChart Inc. (http://www.ourchart.com)
Thanks to Jeff Smick for the User Relationships module, and to the Buddy List folks.
