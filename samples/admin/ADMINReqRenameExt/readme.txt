Licensed Materials - Property of HCL
CIR9ZEN
CIRA0EN
CIRA2EN
CIRA1EN
 
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
-------
ADMINReqRenameExt - Sample program to show usage of ADMINReqRenameExt API.
                    This sample does renaming the user CN=test user/O=HCLPNP
                    to CN=ACB/O=HCLPNP.Also, creates a "Rename Person in Address Book"
                    request in the Administration Requests database (admin4.nsf).

ENVIRONMENTS
------------
Windows 64-bit
Linux 64 bit

FILES
-----
ADMINReqRenameExt.c    - Main program.

mswin64.mak            - Makefile for Windows 64-bit.

linux64.mak            - Makefile for Linux 64 bit.

readme.txt             - This file specifies what is needed to use this example.


RUNNING ADMINReqRenameExt:
-------------

The command line of ADMINReqRenameExt is  -

    ADMINReqRenameExt  <server name> "names.nsf"

where:

    <server name> is the name of a server where the database
                         resides.  

    "names.nsf"   is the NAB file of the Domino database.The ADMINReqRenameExt API 
                  creates a "Rename Person in Address Book" 
                  request in the Administration Requests database (admin4.nsf).




