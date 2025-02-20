Licensed Materials - Property of HCL
CI3N2EN
CI3N3EN
CI3N5EN
CI3N4EN
CIRA2EN
 
(c) Copyright IBM Corporation   1996, 2011
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
---------------
EntryAction


PURPOSE
-------------
This program contains two parts

chair
OneAttendee
 
chair will send a recurrence meeting invitation(10 instances) to attendee and
OneAttendee will accept them first, then decline one of the them.

We need 2 domino users here, say "chair/test" as chair, "attendee/test" as attendee.
If not then create '2' users using Domino Administrator.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
Linux (64-bit)
IBM AIX
IBM i
MAC OS X


FILES
---------
chair.c      - Main program for chair.

OneAttendee.c      - Main program for attendee.

readme.txt   - This file specifies how to run this sample.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     -Make file for Windows 64-bit.

linux64.mak      -Make file for Linux 64-bit.


RUNNING :
-------------
Note: the icalendar data is in the source code itself.

The command line is  -

    chair <server name> <database filename of organizer> <organizer username> <attendee username>

    OneAttendee <server name> <database filename of attendee> <organizer username> <attendee username>

where:

    <server name> is the name of a server. 

    <database filename> is the name of database, differs for both chair and OneAttendee execution.

    <organizer username> is the name of the user who sends invitation should be same for both chair and OneAttendee execution.

    <attendee username> is the name of the user who receives invitation should be same for both chair and OneAttendee execution.


EXAMPLE:
--------------
Under Windows, or UNIX (AIX, Solaris, or Linux),

    chair  server1/test mail\chair.nsf "chair/test" "attendee/test" 
    
              After chair execution a recurrence meeting invitation is sent to attendee.

    OneAttendee server1/test mail\attendee.nsf "chair/test" "attendee/test"

              After OneAttendee execution '1' instance out of '10' instances gets declined by attendee. 

NOTE: Use mail/chair.nsf and mail/attendee.nsf under LINUX

Under IBM i,

1. Compile the makefile (CLP):

1) WRKMBRPDM FILE(QNTCSDK/ETYACT)
select the CLP "MK_ETYACTC" and apply option 14=Compile.
select the CLP "MK_ETYACTA" and apply option 14=Compile.

or

2) CRTCLPGM PGM(QNTCSDK/MK_ETYACTC) SRCFILE(QNTCSDK/ETYACT) 
CRTCLPGM PGM(QNTCSDK/MK_ETYACTA) SRCFILE(QNTCSDK/ETYACT) 

2. Compile the sample:

   CALL PGM(QNTCSDK/MK_ETYACTC)
   CALL PGM(QNTCSDK/MK_ETYACTA)

for local access type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/etyactc)
    parm('server name' 'mail/nif.nsf')) batch(*no)

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/etyacta)
    parm('server name' 'mail/nsf.nsf')) batch(*no)
    
for remote access type:

    rundomcmd server(server name) cmd(call pgm(qntcsdk/etyactc)
    parm('valid_server_name' 'mail/nif.nsf')) batch(*no)
    
    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/etyacta)
    parm('valid_server_name' 'mail/nsf.nsf')) batch(*no)


NOTE
-------
1. chair and OneAttendee need to be executed one after another without any fail 
     otherwise the result varies w.r.t expected result.

2. For 32 bit execution copy mail files to Notes Data folder before execution.


SAMPLE OUTPUT:
--------------------------

After Executing chair:
-------------------------- 
UID Returnd DEA0710563E557A465258608001951FC-Lotus_Auto_Generated
Mail was sent to attendee/test succesfully from organiser/test

After Executing OneAttendee:
--------------------------------------
CalGetNewInvitations returned NoteID: 0x2162
Meeting Invitation came from organiser/test to attendee/test
Calender UID : DEA0710563E557A465258608001951FC-Lotus_Auto_Generated
Deleting Recurrence ID : 20201023T100000Z

Entry not found in index
Instance was deleted successfully so we can not find it now!

