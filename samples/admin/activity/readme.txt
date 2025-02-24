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
activity -  A HCL C API for Domino and Notes program that reads the binary records from 
	    the server's log file and displays it to standard output. Activity 
	    logging replaces and surpasses the previous Billing functionality. The 
	    activity logging services are a set of routines that read and write binary 
	    records to the server log (log.nsf). For efficiency, many records are written 
	    to one note. 

	    ** IMPORTANT NOTE ** Activity logging MUST be enabled in order
	    retrieve the contents of the binary records using the Activity Logging API.
	    Please refer to the Domino documentation for a complete description of
	    activity logging.
	    Set ENVACTIVITY_AUTO OS environment variable to get the consistent output.
	    As log.nsf has variable number of records based on user activity, so in-order 
	    to maintain consistent output for automation validation, setting 
	    this environment to 1 will provide consistent output. 

ENVIRONMENTS
------------
Windows 32-bit
Windows 64-bit
IBM AIX
Linux 64 bit
MAC OS X
IBM i


FILES
-----
activity.c       - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

aix.mak          - Make file for IBM AIX.

aix64.mak        - Make file for IBM AIX64.

linux64.mak    - Makefile for Linux 64 bit.

readme.txt       - This file specifies what is needed to use this example.

mk_activity_i.clp    - Make file (CL Program) for IBM i using ILE C 
                       with the source code being an IFS file.
    
mk_activity_p.clp    - Make file (CL Program) for IBM i using ILE C 
                       with the source code being in a source physical file.     

macosx.mak   - Make file for Mac.


RUNNING activity:
------------------

The command line of activity is  -

    activity <servername>

Set this environment varialbe as follows for automation only.
set ENVACTIVITY_AUTO=1

EXAMPLE:

Under Windows or UNIX (AIX or Linux), type:

    activity  "domino server"

- Under IBM i, type:
      rundomcmd server(<server>) cmd(call pgm(qntcsdk/activity))
	        batch(*no)


If the program runs correctly, it should display a message with the following similar output:



Record #: 1, Name: System.LogInitialized, DescIdx: 1, Timestamp: 02/06/2002 01:26:32 PM
{
	Source: CN=testServer/O=testOrg
}
Record #: 2, Name: Domino.NOTES.Session, DescIdx: 272, Timestamp: 02/06/2002 01:34:01 PM
{
	OrgName: testOrg
	UserName: CN=admin/O=testOrg
	ServerName: CN=testServer/O=testOrg
	SessionId: 85256B580065FE05
	EventType: 1
	ClientAddress: TCPIP: 127.0.0.1
	BytesFromServer: 0
	BytesToServer: 0
	DocumentsRead: 0
	DocumentsWritten: 0
	TotalOpenTime: 0
	Transactions: 0
	Port: TCPIP
}
Record #: 3, Name: Domino.NOTES.Database, DescIdx: 273, Timestamp: 02/06/2002 01:34:02 PM
{
	OrgName: testOrg
	DatabaseName: names.nsf
	CheckpointId: 85256B580065FE43
	UserName: CN=admin/O=testOrg
	ServerName: CN=testServer/O=testOrg
	SessionId: 85256B580065FE05
	EventType: 1
	BytesFromServer: 0
	BytesToServer: 0
	DocumentsRead: 0
	DocumentsWritten: 0
	TotalOpenTime: 0
	Transactions: 0
}
Record #: 4, Name: Domino.NOTES.Database, DescIdx: 273, Timestamp: 02/06/2002 01:34:03 PM
{
	OrgName: testOrg
	DatabaseName: activity.nsf
	CheckpointId: 85256B5800660D2F
	UserName: CN=admin/O=testOrg
	ServerName: CN=testServer/O=testOrg
	SessionId: 85256B5800660D2A
	EventType: 4
	BytesFromServer: 1764
	BytesToServer: 912
	DocumentsRead: 0
	DocumentsWritten: 0
	TotalOpenTime: 16
	Transactions: 12
}

Program completed successfully
