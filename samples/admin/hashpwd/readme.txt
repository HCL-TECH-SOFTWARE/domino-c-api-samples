Licensed Materials - Property of HCL
CIR9ZEN
CIRA0EN
CIRA2EN
CIRA1EN
  
(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
-------
hashpwd


PURPOSE
-------
This program will use various methods to create digest for the ID file which 
run this program, and then verify the passowrd.


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
hashpass.c      - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.


aix.mak         - Makefile for IBM AIX.

aix64.mak       - Makefile for IBM AIX64.

linux64.mak    - Makefile for Linux 64 bit.

mk_copydb_i.clp  - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_copydb_p.clp  - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.

macosx.mak   	- Make file for Mac.

readme.txt      - This file specifies what is needed to use this example.


RUNNING hashpwd
---------------

- Under Windows, or UNIX,
  type:
       hashpwd
  at the system command prompt.

- Under IBM i
  type:
	RUNDOMCMD SERVER(<servername>) CMD(CALL PGM(QNTCSDK/hashpwd)) BATCH(*NO)

If the program runs correctly, it should display a message with the following similar output:

SAMPLE OUTPUT
------------------------
Verify good password pass.
Verify bad password pass.
Verify good password pass.
Verify good password pass.
Verify good password pass.
Verify bad password pass.
