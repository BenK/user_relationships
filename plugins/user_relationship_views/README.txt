/* $Id: README.txt,v 1.2 2007-12-20 17:28:58 sprsquish Exp $ */

User Relationship Views Module
---------------------------------

This is a plugin module for the User Relationships module. It allows integration
with the Views module (http://drupal.org/project/views)

Send comments to Jeff Smick: http://drupal.org/user/107579/contact, or post an issue at
http://drupal.org/project/user_relationships.


Requirements
------------
Drupal 5
User Relationships Module
Views Module


Installation
------------
Enable User Relationship Views in the "Site building -> Modules" administration screen.


Usage
------------
  FIELDS
  ------
  Author's Relationships to Current User: 
    provides a list of relationships the author of a node has with the current user
    can be tunes to show any, only approved, or only non-approved relationships

  Current User's Relationships to Author:
    provides a list of relationships the current user has with the author of a node
    can be tunes to show any, only approved, or only non-approved relationships

  Approval Status:
    shows the approval status of a relationship between the author and the current user
    this is really only useful if you've filtered by a specific relationship

  Relationship Creation Date:
    shows the date/time a relationship between the author and the current user was created
    this is really only useful if you've filtered by a specific relationship


  FILTERS
  -------
  Author's relationship with Username:
    show only nodes where the author has or does not have a specified relationship with a specified user
    OPERATOR: select if the author is or is not related and if that relationship should be approved, non-approved or either
    VALUE:    the relationship to filter by
    OPTION:   the name of the user the author should relate to

  Author's relationship with the Current User:
    show only nodes where the author has or does not have a specified relationship with the currenly logged in user
    OPERATOR: select if the author is or is not related and if that relationship should be approved, non-approved or either
    VALUE:    the relationship to filter by

  ARGUMENTS
  ---------
    UID is related to Author:
      show only nodes where the author is related to the UID argument
      VALUE: any valid user ID

    Author is related to UID through RTID:
      show only nodes where the author has a relationship type specified by RTID
      This argument should probably be used in conjunction with "UID is related to Author" as a way of filtering by relationship type
      VALUE: any valid rtid for a relationship type

    UID to Author is (Non-)Approved:
      show only nodes where the author has an approved or non-approved relationship
      This argument should probably be used in conjunction with "UID is related to Author" as a way of filtering by approval
      VALUE: 0 = Non-Approved, 1 = Approved
      
Credits
-------
Written by Jeff Smick.
Thanks to Eaton for ideas.
