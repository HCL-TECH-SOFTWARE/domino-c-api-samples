Licensed Materials - Property of HCL
CIR9ZEN
CIRA0EN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.

PROGRAM
-------
addin - Text-based server add-in task. Periodically updates a database.


PURPOSE
-------
This is a sample HCL Domino Server add-in task.  It creates a status line
and sets its status, then performs certain operations at specific time 
intervals. 

This sample is designed for portability to platform such as Unix. 
This sample differs from comparable OS/2 and Windows samples in that 
it does not use a string table. A string table is a resource that 
requires a resource compiler, normally only available under OS/2 and 
Windows.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM AIX
IBM i
Linux 64 bit


FILES
-----
addin.c      - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.


mk_addin_i.clp  - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_addin_p.clp  - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.     


linux64.mak     - Makefile for Linux 64 bit.

readme.txt   - This file specifies what is needed to use this example.

 
OPERATING ENVIRONMENT REQUIREMENTS
----------------------------------
The file addintst.nsf, the database in which addin will create notes,
must reside in the Domino data directory.


STARTING THE ADD-IN AUTOMATICALLY WHEN SERVER STARTS
----------------------------------------------------
To start addin automatically with the HCL Domino Server software, 
follow these steps:

1) Make sure the HCL Domino Server software is installed on the 
current machine and the Domino Data directory contains a HCL Domino 
Server log file (log.nsf). 
Either the existing log file may be used or a new one may be created from 
the Domino and Notes Log template.

2) Copy the executable file for the add-in task to the Domino Program 
directory.
- Under IBM i, add the Symbolic Link to the executable file:
  
      ADDLNK OBJ('/QSYS.LIB/QNTCSDK.LIB/ADDIN.PGM')
             NEWLNK('/QIBM/USERDATA/LOTUS/NOTES/ADDIN.PGM')
             LNKTYPE(*SYMBOLIC) 

3) Make a backup copy of Domino server's notes.ini file, then open the
Domino server's notes.ini file for editing, and search for the line that 
begins "ServerTasks=".  This is a comma-delimited list of server tasks 
that are invoked each time the server is started.

4) Add the name of the add-in task to this list.

5) Close notes.ini, saving the changes that were made.

6) If the Notes workstation software is running, exit from it.

7) Start the HCL Domino Server software.

8) After the HCL Domino Server has completed its startup processing, 
the user may confirm that the add-in task is running by typing 
"show tasks" at the HCL Domino Server console. This will display a 
list of all server tasks that are running, and the status of each task.
You should see the "Sample Addin" task displayed.


STARTING AN ADD-IN MANUALLY
---------------------------

To start the add-in task manually from the server console, follow these 
steps:

1) Make sure your Domino Data directory contains a HCL Domino Server 
log file (log.nsf). You can use your existing log file or create a 
new one from the Domino and Notes Log template.

2) Copy the executable file for the add-in task to the Domino Program
directory. 

- Under IBM i, add the Symbolic Link to the executable file:
  
      ADDLNK OBJ('/QSYS.LIB/QNTCSDK.LIB/ADDIN.PGM')
             NEWLNK('/QIBM/USERDATA/LOTUS/NOTES/ADDIN.PGM')
             LNKTYPE(*SYMBOLIC) 
3) If the Notes workstation software is running, exit from it. 

4) Start the HCL Domino Server software.
Wait until all server tasks have started - about 30 seconds.

5) Press ENTER to get the HCL Domino Server console prompt, >.

6) Load the add-in task by typing

        load addin

7) Confirm that the add-in task is running by typing "show tasks" at the 
server console.

Note: This program should only be started from a HCL Domino Server. 
      If started on a HCL Domino Server, addin may be terminated by 
      telling the task to "quit" (type "tell addin quit" in the server 
      command window). If addin is not started from a HCL Domino Server 
      you may need to reboot the computer to stop it.  Under Unix, you 
      may issue a "kill" command to stop it.


VALIDATING THE OPERATION OF ADDIN
---------------------------------
Every 20 seconds, the add-in task displays the following message at the 
server console:

      add-in Test: 20-second job complete.


Every 2 minutes, the add-in task displays the following message at the
server console:

      add-in Test: Two-minute job complete.
