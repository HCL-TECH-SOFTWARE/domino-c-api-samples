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
viewids - Gets the Note ID of each note in a view.


PURPOSE
-------
This program opens the view note for the specified view, then opens the
collection corresponding to that view. Next, it reads all the entries in 
the collection, obtaining the note IDs of all the notes in the view. It
prints out all the note IDs.


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
viewids.c    - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.


aix.mak          - Make file for IBM AIX.

aix64.mak        - Make file for IBM AIX64.

linux.mak    - Make file for Linux.

linux64.mak    - Makefile for Linux 64 bit.

mk_viewids_i.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_viewids_p.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.     

macosx.mak   - Make file for Mac.

readme.txt   - This file specifies what is needed to use this example.


RUNNING viewids
---------------
This sample program will run against the Domino database simple.nsf,
provided with the HCL C API for Domino and Notes toolkit.  You must copy
this file to the Domino or Notes data directory before running this program.

- Under Windows or UNIX (AIX, or Linux) type:

       viewids

  at the system command prompt.


- Under IBM i type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/viewids))
        batch(*no)
The viewids sample will print out a count and the note ID for each note in the 
view "SimpleDataView".

