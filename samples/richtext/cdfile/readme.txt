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
cdfwrite -  Write text as in Domino rich text format to a CD file.
cdfread  -  Read Domino rich text from a CD file into a document.


PURPOSE
-------
These programs illustrate how to use the CompoundText routines to write 
text to a CD file and read text from a CD file into a rich text field.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM AIX
Linux
MAC OS X
IBM i


FILES
-----
cdfwrite.c  - Main file for cdfwrite program.
cdfread.c   - Main file for cdfread program.
cdfile.h    - Symbolic constant definitions.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

aix.mak          - Make file for IBM AIX.

aix64.mak        - Make file for IBM AIX64.


linux.mak   - Make file for Linux.

linux64.mak    - Makefile for Linux 64 bit.

mk_cdfile_i.clp  - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.

mk_cdfile_p.clp  - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.     

macosx.mak   - Make file for Mac.

readme.txt  - This file specifies what is needed to use this example.


RUNNING cdfwrite
----------------
- Under Windows, or UNIX (AIX, Linux ),
  type the following at the system command prompt:

       cdfwrite
        or
       cdfwrite "text to write"

- Under IBM i type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/cdfwrite)
        parm('text to write')) batch(*no)

The program will prompt for the text to be included in the rich text 
field or let the user enter the text on the command line.  It then 
creates a CD file with file name, cd_recs.inp, writes the specified 
text into the file as CD records.


RUNNING cdfread
---------------
This program opens the Domino database richtext.nsf. The C API kit provides
this database in the notedata directory. You must copy this file to the 
data directory before running cdfread.

- Under Windows, or UNIX (AIX, Linux ),
  type the following at the system command prompt:

       cdfread

  The program takes no arguments.
- Under IBM i type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/cdfread))
        batch(*no)


This program creates a new document in the database, richtext.nsf
and reads the rich text from the CD file, cd_recs.inp into the 
RICH_TEXT field of the new document. 


CHECKING THE RESULTS
--------------------
Open the richtext.nsf database and display the document that was just 
created.  The RICH_TEXT field will contain the specified string.

