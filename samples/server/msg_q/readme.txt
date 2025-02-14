Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
-------
msg_q - Server add-in controlled through a message queue.
msg_t   Server add-in that sends messages to other add-in tasks.


PURPOSE
-------
The basic operation of the msg_q server add-in task is similar to the 
"addin" sample.  However, this version allows control over the add-in 
task from the server console, using the Message Queue facility.

The program msg_q creates a message queue, then enters a server add-in
loop.  By default, every 20 seconds, the program writes a message to the
log file.  The program also monitors the message queue for control
commands.

The basic operation of the msg_t server add-in task is to send messages 
to the msg_q server add-in task via a message queue.  All of the commands 
(with the exception of "QUIT") that are available in the msg_q program that 
can be entered at the server console, can also be sent directly by the 
msg_t program.



ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM i


FILES
-----
msg_q.c      - Main Message Queue Server program.
msg_t.c      - Main Message Queue Task program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

readme.txt   - This file specifies what is needed to use this example.

mk_msg_q_i.clp - Make file (CL Program) for IBM i using ILE C 
                 with the source code being an IFS file.
    
mk_msg_q_p.clp - Make file (CL Program) for IBM i using ILE C 
                 with the source code being in a source physical file.  


STARTING MSG_Q AUTOMATICALLY WHEN SERVER STARTS
-----------------------------------------------
To start msg_q automatically with the IBM Domino Server software, follow 
these steps:

1) Make sure the IBM Domino Server software is installed on the current 
machine and the Domino data directory contains a IBM Domino Server log 
file (log.nsf). 
Either the existing log file may be used or a new one may be created from 
the Domino and Notes Log template.

2) Copy the executable file msg_q to the Domino Program directory.
- Under IBM i, add the Symbolic Link to the executable file:
  
      ADDLNK OBJ('/QSYS.LIB/QNTCSDK.LIB/MSG_Q.PGM')
             NEWLNK('/QIBM/USERDATA/LOTUS/NOTES/MSG_Q.PGM')
             LNKTYPE(*SYMBOLIC) 

3) Make a backup copy of notes.ini, then open notes.ini for editing, and 
search for the line that begins "ServerTasks=".  This is a comma-delimited 
list of server tasks that are invoked each time the server is started.

4) Add the name msg_q to this list.

5) Close notes.ini, saving the changes that were made.

6) If the Notes workstation software is running, exit from it.

7) Start the IBM Domino Server software.

8) After the IBM Domino Server has completed its startup processing, 
the user may confirm that the add-in tasks are running by typing 
"show tasks" at the IBM Domino Server console. This will display 
a list of all server tasks that are running, and the status of each task.
You should see the "Message Queue Add-in" task displayed.



STARTING MSG_Q MANUALLY
-----------------------

To start msg_q manually from the server console, follow these steps:

1) Make sure your Domino data directory contains a IBM Domino Server 
log file (log.nsf). You can use your existing log file or create a 
new one from the Domino and Notes Log template.

2) Copy the executable file for the add-in task msg_q to the Domino Program
directory. 

- Under IBM i, add the Symbolic Link to the executable file:
  
      ADDLNK OBJ('/QSYS.LIB/QNTCSDK.LIB/MSG_Q.PGM')
             NEWLNK('/QIBM/USERDATA/LOTUS/NOTES/MSG_Q.PGM')
             LNKTYPE(*SYMBOLIC) 
3) If the Notes workstation software is running, exit from it. 

4) Start the IBM Domino Server software.
Wait until all server tasks have started - about 30 seconds.

5) Press ENTER to get the IBM Domino Server console prompt, >.

6) Load the add-in task msg_q by typing

        load msg_q

7) Confirm that the add-in task msg_q is running by typing "show tasks" at 
the server console.  You should see the "Message Queue Add-in" task 
displayed.



ISSUING MSG_Q COMMANDS
----------------------
Commands are issued using the server console's "TELL" command.  The
commands recognized by this program are:

        TELL MSG_Q QUIT
                Terminate the add-in.
        
        TELL MSG_Q TIME int
                Change the time delay between log messages to "int".
        
        TELL MSG_Q MSG string
                Change the message placed in the log file to "string".

        TELL MSG_Q RESET
                Restore original time delay and message.
        
        TELL MSG_Q WAIT
                Stop writing messages to the log, but do not end 
                the add-in.
        
        TELL MSG_Q RUN
                Resume writing messages to the log.



ISSUING MSG_T COMMANDS
----------------------
First make sure to copy the executable file msg_t to the Domino program
directory. 

- Under IBM i, add the Symbolic Link to the executable file:
  
      ADDLNK OBJ('/QSYS.LIB/QNTCSDK.LIB/MSG_T.PGM')
             NEWLNK('/QIBM/USERDATA/LOTUS/NOTES/MSG_T.PGM')
             LNKTYPE(*SYMBOLIC) 
The msg_t program sends a message to a task and then terminates.  To 
issue the above commands to the msg_q task type the following at 
the server console:

program syntax: load msg_t <task name> <command string>

example:        load msg_t MSG_Q MSG Sending Message to MSG_Q task.
                load msg_t MSG_Q TIME 2
                load msg_t MSG_Q WAIT
                load msg_t MSG_Q RUN
                load msg_t MSG_Q RESET

The MSG_Q commands recognized by this program are MSG, TIME, WAIT, RUN,
and RESET (tell MSG_Q QUIT is handled directly by the server and
not processed by the msg_t task).

The command messages sent to the msg_q program should perform the 
operations described above.
