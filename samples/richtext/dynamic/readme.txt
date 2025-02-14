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
dynamic - Fill a RichText Field


PURPOSE
-------
This program shows how to create a rich text field by dynamically 
allocating a buffer, filling the buffer with CD records converted to 
Domino canonical format, and appending the buffer to a note as a 
rich-text field.


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
dynamic.c    - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

aix.mak          - Make file for IBM AIX.

aix64.mak        - Make file for IBM AIX64.


linux.mak    - Make file for Linux.

linux64.mak    - Makefile for Linux 64 bit.

mk_dynamic_i.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_dynamic_p.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.     

macosx.mak   - Make file for Mac.

readme.txt   - This file specifies what is needed to use this example.


RUNNING dynamic
---------------
This program opens the Domino database richtext.nsf. The C API kit provides
this database in the notedata directory. You must copy this file to the 
Domino or Notes data directory before running dynamic.

- Under Windows, or UNIX (AIX, Linux ),
  type the following at the system command prompt:

        dynamic

  The program takes no arguments.

- Under IBM i type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/dynamic))
        batch(*no)


CHECKING THE RESULTS
--------------------
Open the target database using Notes.

Double-click on the newly created document in order to read it. 
The text should say:

"Hello World...   So long world".

