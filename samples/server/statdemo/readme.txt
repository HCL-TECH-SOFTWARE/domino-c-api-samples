Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN
  
(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
----------------
statdemo

PURPOSE
--------------
This is a sample HCL Domino Server based add-in task.  It creates a status line
and sets its status.

This Domino Server Add-In program uses the HCL C API for Domino and Notes
statistics reporting routines to periodically query the HCL Domino Server
for statistics information. Then it updates a database (statdemo.nsf) with
the information gathered.

ENVIRONMENTS
------------------------
Windows (32-bit)
Windows (64-bit)
Linux (64-bit)
IBM i

FILES
-----
statdemo.c   - Main program.

statdemo.rc  - Resource file for this sample program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

linux64.mak   -Make file for Linux 64bit.

readme.txt   - This file specifies what is needed to use this example.

mk_statdemo_i.clp    - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_statdemo_p.clp    - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.   

OPERATING ENVIRONMENT REQUIREMENTS
----------------------------------------------------------------
The file statdemo.nsf, the database in which statdemo will create notes,
must reside in the Domino data directory.

RUNNING statdemo
--------------------------
Under Windows and Linux 64-bit

1) Make sure the HCL Domino Server software is installed on the
current machine and the HCL Domino Server data directory contains a
HCL Domino Server log file (log.nsf).
Either the existing log file may be used or a new one may be created from
the Domino and Notes Log template.

2) Start the HCL Domino Server software.

The command line for statdemo execution is  -

    statdemo

EXAMPLE
--------------
Under Windows, Linux  64-bit

           statdemo

- Under IBM i, add the Symbolic Link to the executable file:

      ADDLNK OBJ('/QSYS.LIB/QNTCSDK.LIB/STATDEMO.PGM')
             NEWLNK('/QIBM/USERDATA/LOTUS/NOTES/STATDEMO.PGM')
             LNKTYPE(*SYMBOLIC) 

SAMPLE OUTPUT
-------------------------
[4970:0002-3340] 23-10-2020 12:20:14   StatDemo: Initialization complete.
[4970:0002-3340] 23-10-2020 12:20:24   Statdemo: Traversing statistics complete.
[4970:0002-3340] 23-10-2020 12:20:34   Statdemo: Traversing statistics complete.
[4970:0002-3340] 23-10-2020 12:20:44   Statdemo: Traversing statistics complete.
[4970:0002-3340] 23-10-2020 12:20:54   Statdemo: Traversing statistics complete.
[4970:0002-3340] 23-10-2020 12:21:04   Statdemo: Traversing statistics complete.
[4970:0002-3340] 23-10-2020 12:21:04   StatDemo: Termination complete.
