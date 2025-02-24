Licensed Materials - Property of HCL
CIR9ZEN
CIRA0EN
CIRA2EN
CIRA1EN
 
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
-------
ADMINReqMoveComplete- Sample program to show usage of ADMINReqMoveComplete and ADMINReqMoveUserInHier API.
                      ADMINReqMoveUserInHier - This function creates a "Move Person in Hierarchy"
                      request in the Administration Requests database (admin4.nsf).
                      ADMINReqMoveComplete - This function creates a "Initiate Rename in Address Book"
                      request in the Administration Requests database (admin4.nsf).
                      It takes a handle to "AdminpMoveUserInHier" note in admin4
                      request database as an input parameter.

ENVIRONMENTS
------------
Windows 64-bit
Linux 64 bit

FILES
-----
ADMINReqMoveComplete.c - Main program.

mswin64.mak            - Makefile for Windows 64-bit.

linux64.mak            - Makefile for Linux 64 bit.

readme.txt             - This file specifies what is needed to use this example.


RUNNING ADMINReqMoveComplete:
-------------

The command line of ADMINReqMoveComplete is  -

    ADMINReqMoveComplete  <server name> <database filename>

where:

    <server name> is the name of a server where the database
                         resides.  

    <database filename> is the filename of the Domino database.The database
                        is Adminstartion request database "admin4.nsf"
                        This function creates a "Initiate Rename in Address Book" 
                        request in the Administration Requests database (admin4.nsf).

Please note that, It is required that "AdminpMoveUserInHier" API be run first to create
a "Move Person in Hierarchy" request in the Administration Requests. This request handle
is taken as input parameter.

