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
numlist - Creates a number list field.


PURPOSE
-------
This program demonstrates how to create a number list field.  It creates 
a new note in the target database and appends text fields FORM and 
PLAIN_TEXT to the new note. Appends a number list field NUMBER to the 
note with the following values:  1,3,9703.4,-7,0.11592. 


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM AIX
Linux 64 bit
Linux for zSeries
MAC OS X
IBM i


FILES
-----
numlist.c    - Main source code file for the program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

aix.mak          - Make file for IBM AIX.

aix64.mak    - Make file for IBM AIX64.

linux64.mak    - Makefile for Linux 64 bit.

mk_numlist_i.clp   - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_numlist_p.clp   - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.     

macosx.mak   - Make file for Mac.

readme.txt   - This file specifies what is needed to use this example.


RUNNING numlist
---------------
- Under Windows or UNIX (AIX, Linux ),
  type the following at the system command prompt:
  
       numlist <database filename>
                or
       numlist

  where <database filename> is the filename of the Domino database where
  the new document is created.  Use a target database that has a form
  named "SimpleDataForm" containing a text field named PLAIN_TEXT and a
  number list field named NUMBER.  The Number List Test database included
  with the API toolkit (file numlist.nsf) is appropriate.

  If the database filename is not specified, the program will prompt for it.

- Under IBM i type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/numlist)
        parm(numlist.nsf)) batch(*no)
  or
    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/numlist))
        batch(*no)
CHECKING THE RESULTS
--------------------
From Notes, open the "Number List Test" database.
Open the document with text "Created by sample NUMLIST.". Verify the
NUMBER field contains the values {1,3,9703.4,-7,0.11592}.
