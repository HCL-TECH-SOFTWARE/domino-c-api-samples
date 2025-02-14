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
cpnotes - Selectively copies notes from one database to another.


PURPOSE
-------
This program shows how to selectively copy notes from one database to
another. The selections are based on the kinds of note and their modification
times. The code shows much of the API's time/date handling.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM AIX
Linux (64 bit)
MAC OS X
IBM i


FILES
-----
cpnotes.c    - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

aix.mak          - Make file for IBM AIX.

aix64.mak        - Make file for IBM AIX64.

linux64.mak    - Makefile for Linux 64 bit.

mk_cpnotes_i.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_cpnotes_p.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.   

macosx.mak   - Make file for Mac.  

readme.txt   - This file Specifies what is needed to use this example.


RUNNING cpnotes
---------------
- Under Windows or UNIX (AIX, Linux ), type the
  following at the system command prompt:

        cpnotes <input db> <output db> <output title>
                  or
        cpnotes

  where:

  <input db> is the filename of the Domino database to be copied

  <output db> is the filename of the resulting copy of the Domino database

  <output title> is the title of the resulting Domino database

  If the command line arguments are not specified, the program will
  prompt for them.


NOTE:
This program assumes that the syntax for time/date strings on the
target computer is:

      mm/dd/yy  hh:mm       

If your computer is set to a different time/date syntax, you can:

Edit the source code statement that contains the time/date string and 
change it to your computer's time/date format. (You must then re-compile
the program.)


EXAMPLE:
This example uses the Domino database, simple.nsf provided with the Lotus 
C API for Domino and Notes toolkit.  Copy this file to the Domino or Notes 
data directory.  You may also want to create a new document in this database
or modify an existing document to ensure there is at least one document in 
this database that is less than one month old.

      cpnotes  simple  simple2  "Partial Copy of Simple"

- Under IBM i type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/cpnotes)
        parm(simple.nsf simple2.nsf 'Partial Copy of Simple')) batch(*no)
  or
    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/cpnotes))
        batch(*no)
CHECKING THE RESULT
-------------------
To see if cpnotes worked correctly:

Open the new copy of the database.

Choose File/Database/Properties...

This should display the title specified on the command line.

Note that all the forms and views are copied into the new copy. The notes 
in the original database that are less than a month old will all have been 
copied into the copy, and the Notes Help document will only have been 
copied if it has been modified since January 1, 1996.
