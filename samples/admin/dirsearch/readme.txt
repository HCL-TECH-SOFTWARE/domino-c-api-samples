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
dirsearch


PURPOSE
-------
This program is intended to illustrate the practical usage of the 
Directory API implemented via a C API program.

CONTENT
-------
Using the Directory API functionality in the C API Toolkit for
Notes/Domino, this program will:

  - Perform a simple search for a directory entry based on its Notes distinguished
  name, abbreviated name, common name, first name, last name, short name, group name,
  server name, or internet address, using the Directory API

  - Print search results


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM AIX
Linux 64 bit
MAC OS X
IBM i

FILES
-----
dirsearch.c    - Main program.

mswin32.mak    - Make file for Windows 32-bit.

mswin64.mak    - Make file for Windows 64-bit.
 

aix.mak        - Makefile for IBM AIX.

aix64.mak      - Makefile for IBM AIX64.

linux64.mak    - Makefile for Linux 64 bit.

mk_copydb_i.clp  - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_copydb_p.clp  - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.

macosx.mak   - Make file for Mac.

readme.txt     - This file Specifies what is needed to use this example.


RUNNING dirsearch
---------------

- Under Windows or UNIX (AIX, Linux ), type the
  following at the system command prompt:

       dirsearch -s "servername" -d "domain" -n "user name" 

  If the command line arguments are not specified, the program will
  prompt for them.

EXAMPLE:
 
      dirsearch -s "CN=LGCToolkit/O=toolkit" -d "toolkit" -n "Jack Tester/toolkit"

- Under IBM i
type:
      RUNDOMCMD SERVER(<servername>) CMD(CALL PGM(QNTCSDK/DIRSEARCH) PARM('-s <server name>' '-d <domain name>' '-n <user name>')) BATCH(*NO)
