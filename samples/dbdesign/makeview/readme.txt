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
makeview - Create a new view in a database.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM AIX
Linux  64 bit
MAC OS X
IBM i


FILES
-----
makeview.c   - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

aix.mak          - Make file for IBM AIX.

aix64.mak    - Make file for IBM AIX64.


linux.mak    - Make file for Linux.

linux64.mak    - Makefile for Linux 64 bit.

mk_makeview_i.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being an IFS file.
    
mk_makeview_p.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being in a source physical file.   

macosx.mak   - Make file for Mac.  

readme.txt   - This file specifies what is needed to use this example.


RUNNING makeview
----------------
This program opens the Domino database makeview.nsf. The API kit provides
this database in the notedata directory. You must copy this file to the 
Domino or Notes data directory before running makeview.

- Under Windows, or UNIX (AIX, Linux ),
  type the following at the system command prompt:

       makeview

  The program takes no arguments.


- Under IBM i type:
    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/makeview))
        batch(*no)
To verify that the program ran successfully, use Notes to open the 
MakeView test database. The default view is "Simple View". The view
created by this sample program is "Test View".  Use the View menu
to select the Test View.

