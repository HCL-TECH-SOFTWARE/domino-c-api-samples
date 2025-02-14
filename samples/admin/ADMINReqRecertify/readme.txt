Licensed Materials - Property of HCL
CIR9ZEN
CIRA0EN
CIRA2EN
CIRA1EN
 
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
-------
ADMINReqRecertify - Sample program to show usage of ADMINReqRecertify API.
                    This function creates a "Recertify Person in Address Book" 
					   request in the Administration Requests database (admin4.nsf).

ENVIRONMENTS
------------
Windows 64-bit
Linux 64 bit

FILES
-----
ADMINReqRecertify.c       - Main program.

mswin64.mak            - Makefile for Windows 64-bit.

linux64.mak            - Makefile for Linux 64 bit.

readme.txt             - This file specifies what is needed to use this example.


RUNNING ADMINReqRecertify:
-------------

The command line of ADMINReqRecertify is  -

    ADMINReqRecertify  <server name> "names.nsf"

where:

    <server name> is the name of a server where the database
                         resides.  

    "names.nsf"   is the NAB file of the Domino database.The ADMINReqRecertify API 
                  creates a "Recertify Person in Address Book" 
				  request in the Administration Requests database (admin4.nsf)

Please run "tell adminp proc all" from your server console to immediately process
the admin requests.
