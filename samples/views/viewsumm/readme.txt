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
viewsumm - Print the summary information of each note in a view.


PURPOSE       
-------       
This program opens the View Note for the specified view, creates a 
collection containing the note ID's and summary information of all 
the notes in the view, and then prints the summary information.


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
viewsumm.c  - Main program.

mswin32.mak     - Make file for Windows 32-bit.
mswin64.mak     - Make file for Windows 64-bit.

aix.mak          - Make file for IBM AIX.

aix64.mak        - Make file for IBM AIX64.


linux.mak   - Make file for Linux.

linux64.mak    - Makefile for Linux 64 bit.

mk_viewsumm_i.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being an IFS file.
    
mk_viewsumm_p.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being in a source physical file.

macosx.mak   - Make file for Mac.

readme.txt  - This file specifies what is needed to use this example.

          
RUNNING viewsumm
----------------
This sample program can be run against the Domino database summary.nsf,
provided with the HCL C API for Domino and Notes toolkit.  Copy this file
to the Domino or Notes data directory before running this program.

- Under Windows or UNIX (AIX, Linux),
  type the following at the system command prompt:

        viewsumm <database filename> <view name in quotes>
                 or
        viewsumm

  where:

  <database filename> is the filename of the Domino database.
  <view name> is the name of the view.  If no command line arguments 
  are provided, the program will prompt for them.

If entering arguments at the prompts instead of at the command line, 
  do not enclose your responses in quotation marks.


EXAMPLE:

      viewsumm  summary  "FlatView"

- Under IBM i type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/viewsumm)) 
		batch(*no)

  or 

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/viewsumm)
		parm(summary.nsf FlatView)) batch(*no)
Upon executing this command, viewsumm will print out the summary
information for each note in the view, FlatView. If an item in the 
summary information is an "empty" item, then a "*" will be printed for 
that item.  An "empty" item is an item that has zero length.  

