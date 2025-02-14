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
makeform - Create a new form, a subform, and a frameset in a database.


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
makeform.c   - Main program.

cdrecord.c   - Contains several functions used by the main program.

makeform.h   - Function prototypes and symbolic constants.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.


aix.mak          - Make file for IBM AIX.

aix64.mak    - Make file for IBM AIX64.

linux.mak    - Make file for Linux.

linux64.mak    - Makefile for Linux 64 bit.

mk_makeform_i.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being an IFS file.
    
mk_makeform_p.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being in a source physical file.     

macosx.mak   - Make file for Mac.

readme.txt   - This file specifies what is needed to use this example.

lotus.gif    - gif image file.

domino.gif   - gif image file.

(Note: The files lotus.gif and domino.gif are also stored as IFS 
 files: /qntcsdk/notedata/lotus.gif and /qntcsdk/notedata/domino.gif.) 


BEFORE RUNNING THIS SAMPLE
--------------------------
Make sure the two .gif files have read/write access. 

For IBM i, you must copy the two .gif files to the server data directory. 

RUNNING makeform
----------------
This program opens the Domino database makeform.nsf. The API kit provides
this database in the notedata directory. You must copy this file to the 
Domino or Notes data directory before running makeform.

- Under Windows, or UNIX (AIX and Linux),
  type:
        makeform

  at the system prompt. The program takes no arguments.
- Under IBM i type:
    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/makeform))
        batch(*no)


In all environments, makeform creates a form called "Test Form 1",
a subform, and a frameset in the database makeform.nsf. The subform is inserted 
into the main form and contains the gif image "domino".  The frameset is launched
when the database is opened and contains a View frame and Document frame.

To verify that the program ran successfully, use Notes to open the 
MakeForm test database, and choose "Create".  The Create menu should
list a form called "Test Form 1".  After creating and saving the form,
verify when selecting from the View frame, the Document frame
shows the contents of the form that was just created.

