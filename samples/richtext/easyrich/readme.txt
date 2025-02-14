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
easyrich - Shows how to use CompoundText routines.


PURPOSE
-------
This program illustrates how to use the CompoundText routines to write 
to a rich text field. It demonstrates how to append text and paragraphs
with various styles to the rich text field. It demonstrates how to 
assimilate rich text from another document into the rich text field, 
it demonstrates how to create a Doc Link in the rich text field, and it
demonstrates how to add a rendered note to the Compound text.


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
easyrich.c   - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

aix.mak          - Make file for IBM AIX.

aix64.mak    - Make file for IBM AIX64.


linux.mak    - Make for Linux RedHat.

linux64.mak    - Makefile for Linux ReadHat 64 bit.

mk_easyrich_i.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being an IFS file.
    
mk_easyrich_p.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being in a source physical file.     

macosx.mak   - Make file for Mac.

readme.txt   - This file specifies what is needed to use this example.


RUNNING easyrich:
-----------------
This program opens the Domino database richtext.nsf. The C API kit provides
this database in the notedata directory. You must copy this file to the 
Domino or Notes data directory before running easyrich.

- Under Windows, or UNIX (AIX, Linux ),
  type the following at the system command prompt:

        easyrich

  The program takes no arguments.
- Under IBM i type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/easyrich))
        batch(*no)


CHECKING THE RESULTS
--------------------
Open the target database and the document that was just created.  
The RICH_TEXT field will contain the text strings added by the program 
in various colors, font types, and sizes.

There will also be a DocLink pointing to the first note in the view and
a rendered note that contains the first note's contents.

