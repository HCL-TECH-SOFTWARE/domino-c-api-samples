Licensed Materials - Property of HCL
CIR9ZEN
CIRA0EN
CIRA2EN
CIRA1EN
 
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
-------
ADMINReqDeleteInNAB - Sample program to show usage of ADMINReqDeleteInNAB
                      and ADMINReqDeleteInNABExt API

ENVIRONMENTS
------------
Windows 64-bit
Linux 64 bit

FILES
-----
ADMINReqDeleteInNAB.c  - Main program.

mswin64.mak            - Makefile for Windows 64-bit.

linux64.mak            - Makefile for Linux 64 bit.

readme.txt             - This file specifies what is needed to use this example.


RUNNING ADMINReqDeleteInNAB:
-------------

The command line of ADMINReqDeleteInNAB is  -

    ADMINReqDeleteInNAB  <server name> <database filename>

where:

    <server name> is the name of a server where the database
                         resides.  

    <database filename> is the filename of the Domino database.The database
                         is Adminstartion request database "admin4.nsf"
                         which captures the "Delete in Address Book" request.




