Licensed Materials - Property of HCL
CIR9ZEN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
idbackup - Text-based server add-in task. Periodically check a database 
	and process the unhandled documents.


PURPOSE
-------
This sample demonstrates the usage of the API SECBuildEncryptedBackupIDFile,
which is one of ID recovery APIS those are exposed in C API 8.0.

ID recovery APIS will enable Notes/Domino developers have the ability to develop
customized  applications to make the notes ID administration task efficienly.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)


FILES
-----
idbackup.c   - Main program.

 mswin32.mak     - Make file for Windows 32-bit.
mswin64.mak     - Make file for Windows 64-bit.

readme.txt   - This file specifies what is needed to use this example.

 
OPERATING ENVIRONMENT REQUIREMENTS
----------------------------------
The file IDStore.nsf, the database in which the task will search,
must reside in the Domino data directory.


STARTING AN ADD-IN MANUALLY
---------------------------

To start the add-in task manually from the server console, follow these 
steps:

1) Make sure your Domino Data directory contains a HCL Domino Server 
log file (log.nsf). You can use your existing log file or create a 
new one from the Domino and Notes Log template.

2) Copy the executable file for the add-in task to the Domino Program
directory. 

3) Copy the IDStore.nsf file to domino data directory.

3) If the Notes workstation software is running, exit from it. 

4) Start the HCL Domino Server software.
Wait until all server tasks have started - about 30 seconds.

5) Press ENTER to get the HCL Domino Server console prompt, >.

6) Load the add-in task by typing

        load idbackup

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
Every 3 minutes, the add-in task will search the IDWaitProcess view within
IDStore.nsf. If it find a new doc comes and unhandled. The task will detach
the id file from the document and the build an encrypted id file from it.
