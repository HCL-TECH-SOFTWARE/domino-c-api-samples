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
dumpfont - Locate and print out the font tables in all data notes.


PURPOSE:
-------
This program demonstrates the use of ODSReadMemory to convert a font
table from canonical format to host format. After converting to host
format, the contents of the font table is printed out to the screen.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM AIX
Linux
IBM i
MAC OS X


FILES
-----
readme.txt   - This file specifies what is needed to use this example.

dumpfont.c   - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

aix.mak          - Make file for IBM AIX.

aix64.mak    - Make file for IBM AIX64.


linux.mak    - Make file for Linux.

linux64.mak    - Makefile for Linux 64 bit.

mk_dumpfont_i.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being an IFS file.
    
mk_dumpfont_p.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being in a source physical file.    

macosx.mak   - Make file for Mac. 


RUNNING dumpfont
----------------
This program opens the Domino database richtext.nsf. The API kit provides
this database in the notedata directory. You must copy this file to the
Domino or Notes data directory before running dumpfont.

- Under Windows, or UNIX (AIX, Linux ),
  type the following at the system prompt:

    dumpfont

  The program takes no arguments.

- Under IBM i type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/dumpfont))
        batch(*no)


CHECKING THE RESULTS
--------------------
When the program runs correctly, it displays something like this:

Processing note 212A.
    Font 0:
       Face    = 5
       Family  = 34
       Name    = MS Sans Serif
    Font 1:
       Face    = 6
       Family  = 82
       Name    = Symbol
    Font 2:
       Face    = 7
       Family  = 34
       Name    = System
