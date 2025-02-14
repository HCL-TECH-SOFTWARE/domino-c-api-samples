Licensed Materials - Property of HCL
CIRA0EN
CIRA2EN
CIRA1EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.



PROGRAM
-------
addmacro - Add three macro notes to a database.


ENVIRONMENTS
------------
IBM AIX
Linux 64 bit
Linux for zSeries
MAC OS X
IBM i


FILES
-----
readme.txt   - This file specifies what is needed to use this example.

addmacro.c   - Main program.

addmacro.h   - Constants and function prototypes.

aix.mak          - Make file for IBM AIX.

aix64.mak    - Make file for IBM AIX64.


linux.mak    - Make file for Linux.

linux64.mak    - Makefile for Linux 64 bit.

mk_addmacro_i.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being an IFS file.
    
mk_addmacro_p.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being in a source physical file.  

macosx.mak   - Make file for Mac.   


RUNNING addmacro:
-----------------
This program adds macro notes to the specified target database.
Therefore, before running this program, create a copy of the
target database that does not contain the new macros.  The example
database PROBLEMS.NSF, included with the API kit, does not contain
any macros.  Use OS commands or Notes to create a copy of this
database in the Domino or Notes data directory. Specify this copy
as the target when you run the program.  That way, the original will
remain unchanged.

- Under UNIX (AIX and Linux),
  type the following at the system command prompt:

  addmacro <database>

where:

  <database> is the full pathname of the Domino database to be modified.


EXAMPLE
-------

  addmacro problems
- Under IBM i type:
    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/addmacro)
        parm(problems.nsf)) batch(*no)

CHECKING THE RESULTS
--------------------

This program will print out the following messages:

  addmacro: successfully added 3 macros to 'problems'.

From the Notes user interface, add the icon for the target database
to your desktop.  Open the target database and navigate to the "Main View".

Notice that the view shows several problem tracking documents, all
open, of various levels of priority.

Run the "Transfer Silton to Sherman" Filter macro added by this program by
selecting "Transfer Silton to Sherman" from the Actions Macros menu.  Notice
that the macro runs.  Subsequently, all problems previously assigned to
"Isabel Silton" have been reassigned to "Alexander Sherman".

The macro "Escalate Low Prio Calls to Med" is a weekly scheduled macro.
To see it work in UI, go to the Designer. Open the problems.nsf - Agent,
and open this agent.  On the first tab of the infobox for the agent, 
change the Trigger" from "On schedule" to "On event". Change the Runtime 
to "Action Menu selection" and the target to "all selected documents" and 
save it.  Now, you should be able to see the "Escalate Low Prio Calls to Med" 
macro from the Actions Macros menu.  Highlighting this Agent and selecting 
"Actions - Run" from the Notes menu. Notice that the macro runs.  Subsequently, 
all problem tracking documents that were previously low priority have been 
changed to medium.

Before running the Execute-Once macro added by this program, edit one
of the problem tracking documents and enter your own name into the 
field labeled "Support Rep."  This causes the macro to send the email
reminder notification to you.  Press escape and save the modified document.
You should see your own name in the "Support Rep" column of the view.

Make sure the modified document is highlighted. The cursor in the view
is positioned on the document that has your name as the Support Rep.
Select "Send reminder to Support Rep" from the "Actions" menu.  The macro
will run.  The macro will send an email message to you containing
the company name and Subject of the selected problem tracking document.
Check that this message was indeed sent by escaping out of the target
database and checking your Notes email.

