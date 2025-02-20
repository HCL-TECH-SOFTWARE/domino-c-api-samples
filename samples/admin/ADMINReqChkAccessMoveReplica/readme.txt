Licensed Materials - Property of HCL
CIR9ZEN
CIRA0EN
CIRA2EN
CIRA1EN
 
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
-------
ADMINReqChkAccessMoveReplica - Sample program to show usage of ADMINReqChkAccessMoveReplica.
                               This function creates a "Check Access for Move Replica Creation" 
                               request in the Administration Requests database (admin4.nsf).

ENVIRONMENTS
------------
Windows 64-bit
Linux 64 bit

FILES
-----
ADMINReqChkAccessMoveReplica.c  - Main program.

mswin64.mak            - Makefile for Windows 64-bit.

linux64.mak            - Makefile for Linux 64 bit.

readme.txt             - This file specifies what is needed to use this example.


RUNNING ADMINReqChkAccessMoveReplica:
-------------------------------------

The command line of ADMINReqChkAccessMoveReplica is  -

    ADMINReqChkAccessMoveReplica  <source server name> <source database name> <destination server name> <destination full path database name>

where:

    <source Server name> is the name of a server where the source database
                         resides.  
    <source file name> is the filename of the Domino source database.
   
    <destination Server name> is the name of a server where the destination database
                         resides.
    <destination full path database name> is the filename of the Domino destination database.

Note: For cluster setup please refer below link.
https://help.hcltechsw.com/domino/14.0.0/admin/conf_creatingacluster_t.html
    