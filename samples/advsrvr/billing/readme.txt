Licensed Materials - Property of HCL
CIR9ZEN
CIRA0EN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAMS
--------
The Advanced Server Billing API sample is made up of the following two programs.

1) billmngr.c - Billing extension manager sample. This program generates 
     billing information for Database creations and deletions on a server, 
     extending on the basic Database billing.

2) billses.c - Billing server add-in sample. This program periodically 
     retieves Domino session billing information from a server and writes the 
     information to a Domino database on the server.

Note: The names of the DLL and the add-in depend on the platform. 
      The names used are:

	billing.dll, billing.exe      	Windows 
	libbillmgr.a, billses           	IBM AIX
	libbillmgr(*SRVPGM), billses(*PGM) in Library QNTCSDK for IBM i

ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM AIX
IBM i
Linux 64 bit.

FILES
-----
billmngr.c  - Main program for the Billing extension manager sample.   

billses.c   - Main program for the Session Billing add-in task sample.

billses.h   - Header file for the Session Billing add-in.  This file contains
	      literals for the add-in messages and the database forms.
		 
bill_ext.h  - "Extended" copy of billing.h that is included by both samples.
	      This file contains the constants and structure defintions for the
	      the extended billing messages.

aix.mak          - Make file for IBM AIX.

aix64.mak        - Make file for IBM AIX64.

exports.aix  - specifies exported entry points for AIX.

mswin32.mak - 32 bit Windows Makefile.

mswin64.mak - 64 bit Windows Makefile.

linux64.mak    - Makefile for Linux 64 bit.

mswin32.def  - 32 bit Windows Definition file.  

mswin64.def  - 64 bit Windows Definition file.  

mk_billing_i.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_billing_p.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.     

readme.txt  - This file. Describes the sample programs and specifies what is
	      needed to run them.

 
DESCRIPTION
-----------
These two programs cooperate to illustrate using the Billing service and API 
supplied with the Domino Advanced Server.  They exemplify how to generate 
billing messages via the Billing API, and how to extract these
messages from the message queue and into the Billing record datatypes.  

Compile on Windows
-----------
Since we are now using Visual Studio 2010, you may encounter a lot of errors
when run make, which likes:

"C:\Program Files\Microsoft SDKs\Windows\v7.0A\include\mq.h(2368) : 
error C2143: syntax error : missing '{' before '*'" 

This is caused by mq.h, which is one of Toolkit header files. Unfortunately,
It has the same name with one of a system header files, which resident in
"C:\Program Files\Microsoft SDKs\Windows\v7.0A\include\".

You can solve it by:
Option 1: change #include "mq.h" to #include "d:\notesapi90\include\mq.h".
Option 2: add /ID:\notesapi90\include to link, which made linker to find 
along this directory first, then directories along the paths specified by
the INCLUDE environment variable.

NOTES OPERATING ENVIRONMENT REQUIREMENTS
----------------------------------------
Billing must be configured at the server in order for this sample to work
correctly.  To configure the server for billing refer to the HCL Domino Server 
Administrator's Guide and Release Notes databases that were shipped with
Domino.  Specifically, add the following to the server's notes.ini file:

   BillingClass=Agent,Database,Document,HttpRequest,Mail,Replication,Session
   BillingAddinOutput=1

Before you can run the Billing sample you must copy the Session Billing 
database template, sessbill.ntf, to the data directory of the server that
you will be running against.  This file is supplied in the NOTEDATA
subdirectory of the toolkit. 

IMPORTANT NOTE: Since the Billing samples use the default billing message queue,
you must unload the Billing add-in task (NBILLING) that is supplied with the
server installation.  This can be done by removing the task from the
ServerTasks entry in the server's notes.ini file, and then restarting the
server.  If both the sample and installed billing add-in tasks are loaded, the
message queue will be serviced by both tasks and the information supplied to the
sample database will be incomplete.


RUNNING THE BILLING EXTENSION MANAGER SAMPLE
--------------------------------------------
Follow these steps to configure the server to load the Billing extension
manager at server startup.

1. Exit the server if it is running.

2. Copy the DLL to the Domino program directory of the server.

   For IBM i, add a link to the service program LIBBILLMGR:
	ADDLNK OBJ('/QSYS.LIB/QNTCSDK.LIB/LIBBILLMGR.SRVPGM')
		 NEWLNK('/QIBM/USERDATA/LOTUS/NOTES/LIBBILLMGR.SRVPGM')
		 LNKTYPE(*SYMBOLIC)
3. Add the following line to the notes.ini file of the server, 
   normally found in the Domino data directory, and save the modified file:

	EXTMGR_ADDINS=BILLING

   For UNIX platforms (AIX) the name of the shared library should be
   in lower case letters, e.g. for AIX:

	EXTMGR_ADDINS=libbillmgr.a 
   For IBM i: EXTMGR_ADDINS=libbillmgr  

4. Start the server.


AUTOMATICALLY STARTING THE SESSION BILLING ADD-IN SAMPLE 
--------------------------------------------------------
To start add-in automatically with the server software, follow these
steps:

1. Make sure the server software is installed on the server and the 
Domino data directory contains a server log file (log.nsf). Either the
existing log file may be used or a new one may be created from the Notes 
Log template.

2. Copy the Billing server add-in to the Domino program directory of the server.

   For IBM i, add a link to the program BILLSES:
	ADDLNK OBJ('/QSYS.LIB/QNTCSDK.LIB/BILLSES.PGM')
		 NEWLNK('/QIBM/USERDATA/LOTUS/NOTES/BILLSES.PGM')
		 LNKTYPE(*SYMBOLIC)
3. Open the notes.ini file of the server and search for the line that
begins "ServerTasks=".  This is a comma-delimited list of server tasks that
are invoked each time the server is started.

4. Add BILLING to this list , and save the modified file.

5. Exit the server if it is running.

6. Start the server software, the add-in task will be loaded.

7. After the server has completed its startup processing, the user may 
confirm that the add-in task is running by typing "show tasks" at the Domino
server console. This will display a list of all server tasks that are
running, and the status of each task.


MANUALLY STARTING THE SESSION BILLING ADD-IN SAMPLE
---------------------------------------------------
To start the add-in task manually from the server console, follow these 
steps:

1. Make sure the server software is installed on the server and the 
Domino data directory contains a server log file (log.nsf). Either the
existing log file may be used or a new one may be created from the Notes 
Log template.

2. Copy the Billing server add-in to the Domino program directory of the server.
   For IBM i, add a link to the program BILLSES:
	ADDLNK OBJ('/QSYS.LIB/QNTCSDK.LIB/BILLSES.PGM')
		 NEWLNK('/QIBM/USERDATA/LOTUS/NOTES/BILLSES.PGM')
		 LNKTYPE(*SYMBOLIC)

3. Exit the server if it is running.

4. Take a backup of the existing "nbilling" and copy the CAPI "billing"
   OR "billsession" executable in domino program directory.

5. Start the server software, and wait until all the server tasks have
   started (about 30 seconds).

6. From the server console, load the add-in task by typing
	load billing

7. Confirm that the add-in task is running by typing "show tasks" at the
   server console.


NOTE: 
This add-in task should only be started from a server. If started on a 
server, the add-in may be terminated by the server task by typing
	tell billing quit
from the server console prompt.
   
If the add-in is not started from a server, you may need to reboot the 
computer to stop it.



USING THE BILLING SAMPLE AND CHECKING THE RESULTS
------------------------------------------------- 
After the Billing extension manager and Session Billing add-in samples have been
started, you can begin generating billing information on the server. The 
related billing information is written to the Session Billing database file, 
sessbill.nsf. This database is created from the supplied Session Billing template
the first time the add-in is loaded.

The Billing sample extends on the standard billing support, by providing billing
information for all new notes created on the server. The sample database file
billdrv.nsf is supplied with the toolkit, and should be copied from the NOTEDATA
directory to the data directory of the server.  This database is based on the 
Notes V4 Discussion database template, and can be opened and used from a Notes
client to generate note create events on the server.

The following table describes the Billing information that is saved, how to 
generate this information, and what database view is used to inspect the billing.

 Billing Information       User activity            SessBill database view
 -------------------       -------------            -----------------------
  
 Database Activity         Open, Close databases    Session\Database Opens
			   on the server              By Username
      
 Note Create*              Open billdrv.nsf and     Session\Note Create
			   create Main Topic and      By Username
			   Response notes
 
 Remote user sessions      Access the server        Session\Network Traffic
			   remotely with multiple     By Username
			   users

* - The Note Create billing is extended billing customized by the Billing 
    extension manager sample.

By default, the Domino Server will generate Session or database billing records
at 15 minute intervals.  In order to shorten the time it takes to generate
these records and, thus, report the activity to the database, you can add the
following line to the server's notes.ini file

  BillingSuppressTime=n

where n specifies the suppression time period in minutes.  It is recommended
to use a value of 1.

Lastly, by default the Session Billing add-in reads billing records from the message
queue and writes this information to the database every 60 seconds for 10
seconds at a time.  This tasking behavior can be modified by adding or modifying
the following two lines in the notes.ini file of the server prior to loading 
the sample Billing server add-in.

  BillingAddinWakeup=
  BillingAddinRuntime=

The value assigned to BillingAddinWakeup specifies how often (in seconds) the 
add-in should process Billing messages, and the value assigned to 
BillingAddinRuntime specifies how long (in seconds) the add-in should execute
before returning control to the HCL Domino Server.
