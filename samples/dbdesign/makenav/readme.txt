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
MAKENAV - Creates a custom navigator in a database.   This program creates
a Navigator view and/or object (ellipse graphic) that acts as a doclink 
to the first note created in the database today.


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
makenav.c    - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

aix.mak          - Make file for IBM AIX.

aix64.mak    - Make file for IBM AIX64.


linux.mak    - Make file for Linux.

linux64.mak    - Makefile for Linux 64 bit.

mk_makenav_i.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_makenav_p.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.    

macosx.mak   - Make file for Mac. 

readme.txt   - This file specifies what is needed to use this example.


RUNNING MAKENAV
---------------
This program opens and updates the Domino database makenav.nsf. The API kit 
provides this database in the notedata directory. You must copy this file to
the Domino or Notes data directory before running this sample.

Before running this sample, you need to create a note in the database.
From the Notes Workstation, open the makenav sample database.  Use the 
Create-MainForm menu to create a new document. Type some text in the edit 
field, for example "This document was created today.", and save.  You will 
see the document added to the MainView view.

- Under Windows, or UNIX (AIX and Linux),
  type:

       makenav

  at the system prompt. The program takes no arguments.
- Under IBM i type:
    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/makenav))
        batch(*no)


VERIFYING MAKENAV
-----------------
Upon successful program execution, you can verify the result as follows:
1. Open the makenav.nsf database
2. From the "File-Database-Properties..." menu, click the "Launch" tab 
   (rocket). 
3. From the pull down list for "When opened in the Notes client", 
      select "Open designated Navigator"
   Then, from the pull down list for "Type of Navigator:",
      select "Standard Navigator"
   Finally, at the pull down list for "Name:",
      select "Sample Navigator".
4. Close the Database Properties window, and close the database.
5. Re-open the makenav.nsf database, and
   you should see an ellipse that is labelled with today's date. 
   When you click this navigator, the (first) document created on that date 
   will be opened.

You can create more navigators to the Sample Navigator view by rerunning 
the MAKENAV sample. If a document does not exist for the current date, 
no additional navigators will be added.  This sample simulates an 
environment where the database is updated daily by a different program, 
and subsequent MAKENAV execution would dynamically add a navigator to 
link to that document.
