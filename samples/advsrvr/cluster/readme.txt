Licensed Materials - Property of HCL
CIR9ZEN
CIRA0EN
CIRA2EN
  
(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
Clumon - Advanced Server Cluster Monitor sample program.

													
ENVIRONMENTS
------------
Windows (32-bit)
IBM AIX
IBM i

Note: Currently not support 64-bit UI application on Windows 64-bit

FILES
-----
clumon.c         - Main program for Windows version of sample.

clumoncm.c       - Main program for character mode version (AIX) of sample.

clumon.h         - Local function prototypes and data definitions for clumon.c.

clumon.rc        - Resource compiler file for this sample.

clfunc.c         - Service routines for the UI-specific event handlers.  Organized
		   for platform independence, and contains all relevent API
		   function calls.

clfunc.h         - External function prototypes for routines defined in clfunc.c.

clumon.ico       - Program Icon

mswin32.def      - Windows 32-bit definition file for this sample.

mswin32.mak     - Make file for 32 bit Windows.

aix.mak          - Make file for IBM AIX.

aix64.mak        - Make file for IBM AIX64.

readme.txt       - This file. Descibes the sample and specifies how to use it.

clumoni5.c       - Main program for IBM i version of sample.

mk_cluster_i.clp    - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_cluster_p.clp    - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.

DESCRIPTION
-----------
This program illustrates cluster administration through the Advanced Server
cluster API. The sample program capabilities include:

   1) displaying a Domino server's cluster information; including cluster name,
      availability index, and other servers that are cluster members,

   2) displaying a Domino server database's current mark options associated
      with a cluster;  specifically marked in service, marked out of service,
      and marked for delete,

   3) modifying a Domino server database's current mark options,

   4) restricting a Domino server to facilitate database open failover,

   5) shutting down a Domino server to facilitate database open failover,

   6) setting the Domino server availability threshold to facilitate database
      open failover,

   7) creating and copying replica databases to other Domino servers.



NOTES OPERATING ENVIRONMENT REQUIREMENTS
----------------------------------------
I) Cluster Information
In order to fully illustrate the capabilities of this sample program, you must
have access to at least one server that is configured in a Advanced Server
Cluster.  For information on how to configure a Domino server within a cluster,
refer to the R8 Release Notes and Administrator's Guide databases that were
installed with the Domino server.

If you specify the name of a server that is not a member of a Domino cluster, an
error will be returned by the program when trying to obtain the server's cluster
information.

Additionally, your Notes Workstation's location record must be configured as
follows:
	1) The "Mail domain" must be set to the domain name for the server(s) in the
	   cluster.
	2) The "Home/mail server" must be set to a server in this domain.
Refer to the Notes V8 Administrator's Guide for more information on
configuring location records.

The sample may not work properly if you are trying to obtain cluster
information for a cluster/server in a domain other than that specified in
your location record.

II) Database Mark Option Management
You can specify any database on the server when viewing the current database
mark options from the sample.  When managing/modifying the database mark
options from the sample, it is highly recommended that you use the supplied
sample database file, MarkTest.nsf. Copy this file from the NOTEDATA
subdirectory of the toolkit into the data directory of the Domino servers
in the cluster.  Note that if you supply your own database, then you must
have at least "Designer" access in order to mark the database in/out of
service, and "Manager" access to mark the database for deletion.

III) Database Replicas
In order to successfully create replica databases to the servers in your
cluster, you must have privileges to create replica databases on the targeted
servers. This information can be configured via the N&A book of the server
domain.

IV) Server Configuration Management
The CLUMON sample programatically uses remote server console commands to
manage the server configuration information related to server restriction,
threshold manipulation, shutdown. As such, you must have administrative
privileges on each server that you plan to remotely manage. This information
can be configured via the N&A book of the server domain.



RUNNING Clumon
--------------
Before you run the sample program (clumon.exe), verify that you have met the
requirements specified for accessing the exposed features of a clustered
Domino server, as described in the previous section.

Windows
-------
From the File-Run dialog of Windows NT, or the Program-Run dialog of
Windows95, or the DOS command prompt, run the program executable CLUMON.EXE.
A simple Windows interface will appear with the following three menu options:
File, Information, Administration.

AIX
---
From the command shell, run the program executable clumon.  The Main
Menu for the the sample program will be displayed.

I5OS
----
    RUNDOMCMD SERVER(<servername>) CMD(CALL PGM(QNTCSDK/CLUMONI5)) 
    BATCH(*NO)


USING Clumon
------------
The following describes the menu item options for the Windows program version.
The relevant Character Mode program menu items are referenced in parentheses
below the Windows items.  Instead of using Windows dialogs, the Character Mode
program prompts the user for the information.

Information-Cluster...
(Main Menu-1. Cluster Information)
----------------------------------
This menu item displays a modal dialog box, titled "Cluster Information".

When a server name is entered in the "Server Name:" editbox, clicking on the
"Info" button (or hitting <Enter>) will display the Cluster Name, Availability,
the number of Cluster Mates, and a list of Cluster Mates for that server in
the relevant "Cluster Information" group text fields and listbox.

If the server cannot be found, is not a member of a cluster, or does not have
any clustermates, the sample will display an appropriate messagebox. Likewise,
all API errors and their appropriate descriptions will be displayed.

Clicking the "Done" button will end the dialog box and return focus to the
main window.

NOTE:
The server's availability index is a value (between 0 and 100) that is
computed based on the current activity occurring on the server, for example
session and database activity. The server availability index works in
conjunction with the configured server availability threshold (also a value
from 0 to 100).  When the availability drops below the threshold, the server
is in a "busy" state, meaning that it will not accept any new session activity.
This is true until the availability index rises above the threshold.  Refer to
the "Administration-Server Availability Threshold" section below for
information on configuring this threshold value.
								
									
Information-Database Mark Options...
(Main Menu-2. Cluster Administration)
-------------------------------------
This menu item displays a modal dialog box, titled "DB Mark Options
Information".  After a server/domain name is entered in the "Server Name:"
editbox and a database name (e.g. MarkTest) is entered in the "Database Name:"
editbox, clicking on the "Current" button (or hitting <Enter>) will display
the current database mark option(s) of the database in the appropriate
"Database Mark Options" group checkboxes.

Database server failover is supported by this operation if the targeted server
is configured in a cluster and any one of the following conditions exist:
    1) the database has been marked out of service and/or for deletion
    2) the server is restricted
    3) the server is busy (i.e., the availability threshold > the index)
    4) the server is shut down, or unreachable
    5) the server has reached maximum user sessions
If the failover scenario exists, then if the an available replica database on
a different server in the cluster will be opened and the current database mark
option(s) will be displayed.  A message box will be displayed informing you
of failover.  If there is no available replica database, then the existing
database mark option will be displayed.  An appropriate messagebox is displayed
if failover has occurred or if a database could not be opened due to restricted
servers and/or databases.

Note that clustered database files that are marked for deletion can never be
unmarked and will be permantly removed by the Cluster Database Directory Manager
server task (CLDBDIR) at some point after all sessions with this database have
been closed.  Note that any database that is marked for deletion will also be
marked out of service by the Domino server.

Database files that are marked out of service can still be marked in service at
any time within an active cluster (see the the "Administration-Database
Mark Options..." menu option description below).

All API errors that occur will be displayed, along with an appropriate
description.

Clicking the "Done" button will end the dialog box and return
focus to the main window.


Administration-Database Mark Options...
(Cluster Information Menu-2.View Database Mark Options)
(Cluster Administration Menu-1. Modify Database Mark Options)
-------------------------------------------------------------
This menu item displays a modal dialog box, titled "DB Mark Options
Management".

After a server name is entered in the "Server Name:" editbox and a database
name (e.g. MarkTest) is entered in the "Database Name:" editbox, clicking on the
"Current" button (or hitting <Enter>) will display the current database mark
option(s) for the database in the appropriate "Database Mark Options" group
checkboxes.  Database failover is supported by this operation.  Refer to the
previous section for information on failover criterion.

Setting the "Marked ..." checkbox(es) associated with the three options and then
clicking the "Update" button will mark the database file as appropriate. Note
that you must have at least designer access to mark the database in service or
out of service.  You must have manager access to mark the database for deletion.

A database can be marked out of service and then back in service through this
dialog box.  As noted in the "Information-Database Mark Options..." menu option
description, marking a database file for delete is permanent, so only use the
supplied MarkTest.nsf database file when exercising this feature. Note that the
"Marked ..." checkboxes in this dialog are designed to be exclusive of each
other for certain scenarios.  For example, you can not mark a database both in
service and out of service, or both in service and for delete.

All API errors that occur will be displayed, along with an appropriate
description.

Clicking the "Done" button will end the dialog box and return focus to the
main window.


Administration-Server Restriction...
(Cluster Information Menu-3.View Server Restriction Setting)
(Cluster Administration Menu-2. Restrict Server)
------------------------------------------------------------
This menu item displays a modal dialog box, titled "Server Restriction
Management".

After a server name is entered in the "Server Name:" editbox, clicking on the
"Current" button (or hitting <Enter>) will display the server's current
restriction configuration in the "Server Restricted" checkbox.  If checked the
server is restricted, if unchecked the server is not restricted.

Checking or unchecking the "Server Restricted" checkbox and then clicking the
"Update" button will either restrict or unrestrict the server, respectively.

Clicking the "Shutdown" button will start the shutdown process for the server.
Note that the underlying API call may return a "Server not responding"
error due to the time overhead of the shutdown.  This error can be expected.
Also note that the server can not be restarted remotely from this sample so
care should be taken when using this operation.

All API errors that occur will be displayed, along with an appropriate
description.

Clicking the "Done" button will end the dialog box and return focus to the
main window.

NOTE:
Since the server restriction settings are accessed via the Domino Remote
Console API, you must be an administrator of the server to execute the relevant
remote console commands.


Administration-Server Availability Threshold...
(Cluster Information Menu-4.View Server Availability Threshold)
(Cluster Administration Menu-3. Modify Server Availability)
---------------------------------------------------------------
This menu item displays a modal dialog box, titled "Availability Threshold
Management".

After a server name is entered in the "Server Name:" editbox, clicking on the
"Current" button (or hitting <Enter>) will display the server's current
availability index and availability threshold values in the "Availability
Information" group text field and listbox, respectively.

Entering a value in the "Availability Threshold:" editbox and then clicking the
"Update" button will update the availability threshold value configuration on
the server as specified.  Note that the entered value must be between the range
of 0-100, or an error messagebox will be displayed before the update is
attempted.

All API errors that occur will be displayed, along with an appropriate
description.

Clicking the "Done" button will end the dialog box and return focus to the
main window.

NOTE:
Since the server availability settings are accessed via the Domino Remote
Console API, you must be an administrator of the server to execute the relevant
remote console commands.


Administration-Database Copy...
(Cluster Administration Menu-4. Database Replicate/Copy)
--------------------------------------------------------
This menu item displays a modal dialog box, titled "DB Copy Management".

Clicking the "Copy" button will perform a database create and copy from the
servername and database name entered in the "DB Source Information" group
editboxes to the server and database names entered in the "DB Copy Information"
editboxes.  If the "Replica Copy" checbox is checked, then a replica copy of
specified source database will be created and copied.   If the "New Copy"
checkbox is checked, then a new copy of the specified source database will
be created and copied.

All API errors that occur will be displayed, along with an appropriate
description.
																		
Clicking the "Done" button will end the dialog box and return focus to the
main window.

NOTE:
You must have the appropriate privileges on the destination (copy) server to
create new copy and replica databases.


WINDOWS USABILITY NOTE
----------------------
To reduce repetition of server and database name entry, the sample user
interface for Windows was designed to maintain the current server and/or
database name context as you use the menu options. For example, the server name
and database file name that you entered in the "Database Mark Options
Information" dialog will automatically be entered into the "DB Mark Options
Management" dialog.  Also, when the dialog box is initialized, the
appropriate information (as related to the server and database names that are
in context) is retrieved.  The current context is displayed on the program title
bar, following the "Advanced Server Cluster Monitor Sample" string.  This
information is not persistent, and will be cleared once the program is exited.
