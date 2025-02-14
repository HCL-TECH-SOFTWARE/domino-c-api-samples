Licensed Materials - Property of HCL
CIR9ZEN
CIRA0EN
CIRA2EN 
  
(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
THREADS - Text-based multithreaded server add-in task. Periodically writes
          and reads a database via multiple threads.

PURPOSE
-------
This is a sample multithreaded HCL Domino Server add-in task.  It creates a 
status line and sets its status, then starts three worker threads that 
perform certain operations at specific time intervals. The main thread
communicates with the worker threads through Message Queues and tells the
threads when to perform their operations and when to terminate.
THREADS uses basic synchronization and file locking schemes to ensure that
all operations are performed to completion and without error.

This sample is designed for portability to platform such as 
Unix. This sample differs from comparable Windows samples in that it does 
not use a string table. A string table is a resource that requires a 
resource compiler, normally only available under Windows.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM AIX
Linux 64 bit
IBM i


FILES
-----
threads.c    - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

aix.mak - Makefile for IBM AIX.

aix64.mak - Makefile for IBM AIX64.

linux64.mak    - Makefile for Linux 64 bit.

readme.txt   - This file specifies what is needed to use this example.

mk_threads_i.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_threads_p.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.

 
OPERATING ENVIRONMENT REQUIREMENTS
----------------------------------
The file threads.nsf, the database in which THREADS will create notes,
must reside in the HCL Domino Server data directory.  


STARTING THE ADD-IN AUTOMATICALLY WHEN SERVER STARTS
----------------------------------------------------
To start THREADS automatically with the HCL Domino Server software, 
follow these steps:    

1) Make sure the HCL Domino Server software is installed on the 
current machine and the HCL Domino Server data directory contains a 
HCL Domino Server log file (log.nsf). 
Either the existing log file may be used or a new one may be created from 
the Domino and Notes Log template.

2) Copy the executable file for the add-in task to the HCL Domino Server 
Program directory:

     For Windows (32-bit)
         the executable file name is nthreads.exe 


     For all UNIX platforms 
         the executable file name is threads

     For IBM i, the executable file name is THREADS, and you need to add 
     the Symbolic Link to the executable file:
  
        ADDLNK OBJ('/QSYS.LIB/QNTCSDK.LIB/THREADS.PGM')
               NEWLNK('/QIBM/USERDATA/LOTUS/NOTES/THREADS.PGM')
               LNKTYPE(*SYMBOLIC) 
3) Make a backup copy of the HCL Domino Server notes.ini, then open notes.ini 
for editing, and search for the line that begins "ServerTasks=".  This is a 
comma-delimited list of server tasks that are invoked each time the server 
is started.

4) Add the name of the add-in task (threads) to this list.

5) Close notes.ini, saving the changes that were made.

6) If the Notes workstation software is running, exit from it.

7) Start the HCL Domino Server software.

8) After the HCL Domino Server has completed its startup processing, 
the user may confirm that the add-in task is running by typing "show tasks" 
at the HCL Domino Server console. This will display a list of all server 
tasks that are running, and the status of each task.  You should see a 
"Sample Multithreaded Addin" task listed.


STARTING THE ADD-IN MANUALLY
----------------------------
To start the add-in task manually from the server console, follow these 
steps:

1) Make sure your HCL Domino Server data directory contains a 
HCL Domino Server log file (log.nsf). You can use your existing log file 
or create a new one from the Domino and Notes Log template.

2) Copy the executable file for the add-in task to the HCL Domino Server 
Program directory:

     For Windows (32 bit)
         the executable file name is nthreads.exe 


     For all UNIX platforms 
         the executable file name is threads

     For IBM i, the executable file name is THREADS, and you need to add 
     the Symbolic Link to the executable file:
  
        ADDLNK OBJ('/QSYS.LIB/QNTCSDK.LIB/THREADS.PGM')
               NEWLNK('/QIBM/USERDATA/LOTUS/NOTES/THREADS.PGM')
               LNKTYPE(*SYMBOLIC) 

3) If the Notes workstation software is running, exit from it. 

4) Start the HCL Domino Server software.
Wait until all server tasks have started - about 30 seconds.

5) Press ENTER to get the HCL Domino Server console prompt, >.

6) Load the add-in task by typing

        load threads

7) Confirm that the add-in task is running by typing "show tasks" at the 
server console. You should see a "Sample Multithreaded Addin" task listed.

Note: This program should only be started from a HCL Domino Server. If 
      started on a HCL Domino Server, THREADS may be terminated by 
      terminating the server task (typing "quit" in the server command 
      window). If THREADS is not started from a HCL Domino Server you 
      may need to reboot the computer to stop it.


VALIDATING THE OPERATION OF THE ADD-IN
--------------------------------------
THREADS uses three worker threads to perform operations against the sample
database.  Specifically, 
     one thread writes a note to the database every 30 seconds, 
     another thread writes a note to the database every minute, and 
     the third thread reads and displays the notes in the database 
         every other minute.  
As THREADS runs, you should see console messages illustrating this
behavior.  

After six 30-second, three 1-minute, and two 2-minute operations,
THREADS will end the three threads and terminate.  You can then open up the
sample database and validate the written notes using the AddInView view.

Since THREADS illustrates a well behaved Notes addin task, you can validate
user invoked termination by either typing 

        tell threads quit

at the HCL Domino Server console, or by exiting the HCL Domino Server 
during execution of the sample.  You should see relevent thread 
termination messages prior to termination of the addin task or server.  
These messages mean the threads are no longer running on the server.
