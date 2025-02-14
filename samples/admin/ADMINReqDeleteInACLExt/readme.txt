Licensed Materials - Property of HCL
CIR9ZEN
CIRA0EN
CIRA2EN
CIRA1EN
 
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
ADMINReqDeleteInACLExt - Sample program to show usage of ADMINReqDeleteInACLExt.

ENVIRONMENTS
------------
Windows 64-bit
Linux 64 bit

FILES
-----
ADMINReqDeleteInACLExt.c  - Main program.

mswin64.mak            - Makefile for Windows 64-bit.

linux64.mak            - Makefile for Linux 64 bit.

readme.txt             - This file specifies what is needed to use this example.


RUNNING ADMINReqDeleteInACLExt:
-------------

The command line of ADMINReqDeleteInACLExt is  -

    ADMINReqDeleteInACLExt  <server name> <database filename> <mail flag> <idvault flag>

where:

    <server name> is the name of a server where the database
                         resides.  

    <database filename> is the filename of the Domino database.The database
                         is Adminstartion request database "admin4.nsf"
                         which captures the "Delete in Access Control List" request.

    <maile flag> is there list of falgs.
                 "0" = Don't delete mail file.
                 "1" = Delete just mail file specified in person record.
                 "2" = Delete mail file specified in person record & all replicas.

    <idvault flag> is there list of flags.
                   "0" = Don't delete, just mark as inactive.
                   "1" = Delete the user from the vault.
                   "2" = Don't do anything with the user's ID in the vault.