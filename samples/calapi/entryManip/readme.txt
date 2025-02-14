Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
---------------
entrymanip


PURPOSE
-------------  
This program opens a mailbox file and create,update,read,delete an iCalendar entry.


ENVIRONMENTS
------------------------
Windows (32-bit)
Windows (64-bit)
Linux (64-bit)
IBM i


FILES
--------
entrymanip.c   - Main program.

mswin32.mak    - Makefile for Windows 32-bit.

mswin64.mak    - Makefile for Windows 64-bit.

linux64.mak    - Makefile for Linux 64-bit.

readme.txt     - This file specifies what is needed to use in this example.

mk_etycrud_i.clp   - Makefile (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_etycrud_p.clp   - Makefile (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file. 


IDENTIFY A TEST DATABASE:
-----------------------------------------

Determine the name of a mailbox file, such as mailxxx.nsf, that
resides in a Domino or Notes data directory. If not create one using Domino Designer.


RUNNING entrymanip:
------------------------------

The command line of entrymanip is  -

    entrymanip <mailbox filename>

where:

    <mailbox filename> is the filename of the Domino database for which we are going to create calendar entry

Note: For 32 bit execution copy mail file to Notes Data folder before execution.


EXAMPLE:
---------------

Under Windows, or UNIX (AIX or Linux),

for local access type:

    entrymanip mail\mailxxx.nsf     (for Windows)
    entrymanip mail/mailxxx.nsf     (for Linux)

for remote access type:

     entrymanip <serverName> <mailFileName>


Under IBM i,
1. Compile the makefile (CLP):

   WRKMBRPDM FILE(QNTCSDK/ETYCRUD)
 select the CLP "MK_ETYCRUD" and apply option 14=Compile.
 
2. Compile the sample:

   CALL PGM(QNTCSDK/MK_ETYCRUD)
         
3. Run the sample:
for local access type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/ETYCRUD)
    parm(mailxxx.nsf)) batch(*no)

for remote access type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/ETYCRUD)
    parm(valid_server_name mailxxx.nsf)) batch(*no)

If the program runs correctly, an iCalendar entry while be created first,
then updated, read, and deleted.


SAMPLE OUTPUT
------------------------
Given DB [L2Support/CENTOS!!mail\sadmin.nsf] is on a remote server. Remote server details:  TCP name: [mytest.com], Host name: [mytest], Domain name: [com].

DB [L2Support/CENTOS!!mail\sadmin.nsf] doesn't have full access.
UID returned E12071396C345C5665258609003074E0-Lotus_Auto_Generated
CalUpdateEntry success!
Calender data returned:
BEGIN:VCALENDAR
VERSION:2.0
PRODID:-//Lotus Development Corporation//NONSGML Notes 12.0//EN_API_S
BEGIN:VEVENT
DTSTART:20121122T180000Z
DTEND:20121122T210000Z
TRANSP:OPAQUE
LAST-MODIFIED:20201022T084916Z
DTSTAMP:20201022T084916Z
SEQUENCE:0
CLASS:PUBLIC
SUMMARY:(Delayed)Bastille Day Party
UID:E12071396C345C5665258609003074E0-Lotus_Auto_Generated
X-LOTUS-UPDATE-SEQ:2
X-LOTUS-UPDATE-WISL:$S:2;$L:2;$B:2;$R:2;$E:2;$M:2;$W:2;$O:2;RequiredAttendees:2;INetRequiredNames:2;AltRequiredNames:2;StorageRequiredNames:2;OptionalAttendees:2;INetOptionalNames:2;AltOptionalNames:2;StorageOptionalNames:2;ApptUNIDURL:2;STUnyteConferenceURL:2;STUnyteConferenceID:2;SametimeType:2;WhiteBoardContent:2;STRoomName:2;$ECPAllowedItems:2
X-LOTUS-NOTESVERSION:2
X-LOTUS-APPTTYPE:0
X-LOTUS-CHILD-UID:339E5C32C008B37A6449A242D38ED1DF
END:VEVENT
END:VCALENDAR

[5DE8:0002-3A18] [C&S] WARNING!!! Need to delete note we are not the owner of!!

CalActionDelete success!


EXAMINING THE DOMINO DATABASE TO CHECK THE RESULT:
-----------------------------------------------------------------------------------------

If you want to see the calendar entry in mailbox file, you can comment the call
to function TestCalEntryActionDelete, you will find the entry when program
run over.

As the same we can also comment the other functions to see the
calendar entry created/updated in the mailbox file.

