Licensed Materials - Property of HCL
CIR9ZEN
CIRA0EN
CIRA2EN
CIRA1EN
  
(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.



PROGRAM
-------
ooo


PURPOSE
-------
This program will sets and gets user Out Of Office information.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM AIX
Linux 64 bit
MAC OS X
IBM i

Settings NOTES.INI
------------------
To manipulate Notes Date Format for windows and linux is as,

Windows:
NotesDateFormat=1

Linux:
NotesDateFormat=3

Restart client/server accordingly.

FILES
-----
ooo.c           - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

aix.mak         - Makefile for IBM AIX.

aix64.mak       - Makefile for IBM AIX64.

linux64.mak    - Makefile for Linux 64 bit.

mk_copydb_i.clp  - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_copydb_p.clp  - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.

macosx.mak   - Make file for Mac.

readme.txt      - This file Specifies what is needed to use this example.


RUNNING ooo
---------------

- Program look for server name from NOTES.INI and 
  start date & end date on Out of Office are auto generated.
  start and end date are one day difference, say 09/23/2020 - 09/24/2020.

- Under UNIX (AIX), type the
  following at the system command prompt:

       ooo <user's mail database filename> <user name>
  

- Under Windows or UNIX (Linux ), type the
  following at the system command prompt:

       ooo <user's mail database filename> <user name>
  
  where:

  <user name> is the user name of the id file which run this program

  <user's mail database filename> is the file path+name of the user who run this program

  If the command line arguments are not specified, the program will throw error and exit.

EXAMPLE:
--------
 
      ooo "mail\jack.nsf" "Jack Tester/toolkit" 
 
SAMPLE OUTPUT:
-------------
Start date[ 09/23/2020 ]- End date[ 09/24/2020 ]
The version (agent, service) of the out of office functionality is agent
The state (disabled, enabled) of the out of office functionality is enabled
Gets ExcludeInternet is TRUE
Gets OOO General Subject:
RHEL User1/RHEL is out of office
Gets OOO General Message:
RHEL User1/RHEL will out of office 09/23/2020 to 09/24/2020.
I am out of office
Program completed successfully.

