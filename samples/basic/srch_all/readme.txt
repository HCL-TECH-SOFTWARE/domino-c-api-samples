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
srch_all - Search for and process all documents in a database.


PURPOSE
-------
Demonstrates how to use NSFSearch to process all documents in a database 
without the risk of processing the same note more than once.  This ensures 
that each document gets processed once and only once by building an ID 
table that is guaranteed to contain exactly one instance of each note ID. 
Once the search is completed and the ID table is full, then the program 
iterates over IDs in the table and processes each note once.


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
srch_all.c   - Main C source file for the program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

aix.mak          - Make file for IBM AIX.

aix64.mak    - Make file for IBM AIX64.


linux64.mak    - Makefile for Linux 64 bit.

mk_srch_all_i.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being an IFS file.
    
mk_srch_all_p.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being in a source physical file.    

macosx.mak   - Make file for Mac. 

readme.txt   - This file. Describes what is needed to use this program.


RUNNING srch_all
----------------
Run this sample program with the srch_all.nsf Domino database provided
with the HCL C API for Domino and Notes toolkit.  You must copy the 
srch_all.nsf file to the Domino or Notes data directory before running
this program.

From Notes, open the srch_all.nsf target database and familiarize 
yourself with the "categorized" view. Most documents appear in category
"p". A few appear in category "q". One appears in category "r".

Run the program:

- Under Windows or UNIX (AIX, Linux ),
  type the following at the system command prompt:

         srch_all  srch_all.nsf
    
- Under IBM i type:
    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/srch_all)
        parm(srch_all.nsf)) batch(*no)
       
CHECKING THE RESULT
-------------------
Open the target database go to the "categorized" view. Press F9 to 
refresh the view if you left the database open while running srch_all.
Note that all documents previously in category "p" now have category "q", and 
all documents previously in category "q" now have category "r". All docs
previously in category "r" remain in category "r". 

You may repeat the test by running srch_all again. After the second round,
note that all documents are in category "r".

If you attempt to run this sample again, no further change will
occur. Reset the database srch_all.nsf using the agent, "ChangeCatToP". 
This macro automatically changes the category of all documents to "p". 

