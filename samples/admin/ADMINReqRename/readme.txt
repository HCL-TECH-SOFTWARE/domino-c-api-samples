Licensed Materials - Property of HCL
CIR9ZEN
CIRA0EN
CIRA2EN
CIRA1EN
 
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
ADMINReqRename -    Sample program to show usage of ADMINReqRename API.
                    This API renames an existing user to new first name,
                    last name and middle name.Also, it gives new org name
                    as well. This API first creates a"Rename Person in Address Book"
                    request the Administration Requests database (admin4.nsf)

ENVIRONMENTS
------------
Windows 64-bit
Linux 64 bit

FILES
-----
ADMINReqRename.c       - Main program.

mswin64.mak            - Makefile for Windows 64-bit.

linux64.mak            - Makefile for Linux 64 bit.

readme.txt             - This file specifies what is needed to use this example.


RUNNING ADMINReqRename:
-------------

The command line of ADMINReqRename is  -

    ADMINReqRename  <server name> "names.nsf"

where:

    <server name> is the name of a server where the database
                         resides.  

    "names.nsf"   is the NAB file of the Domino database.The ADMINReqRename API 
                  creates a "Rename Person in Address Book" 
                  request in the Administration Requests database (admin4.nsf).

Please run "tell adminp proc all" from your server console to immediately process
the admin requests.
