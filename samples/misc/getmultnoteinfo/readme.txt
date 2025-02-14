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
getmultnoteinfo - Shows the basic usage of NSFDbGetMultNoteInfo & 
NSFDbGetMultNoteInfoByUNID. The program prints NoteIDs & 
UNIDs to the screen.


ENVIRONMENTS
------------
Windows 32-bit
Windows 64-bit
IBM AIX
IBM i
Linux 64 bit
MAC OS X


FILES
-----
getmultnoteinfo.c      - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.


aix.mak          - Make file for IBM AIX.

aix64.mak        - Make file for IBM AIX64.

linux.mak    - Make file for Linux.

linux64.mak    - Makefile for Linux 64 bit.

mk_getmult_i.clp   - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_getmult_p.clp   - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.     

macosx.mak   - Make file for Mac.

readme.txt   - This file specifies what is needed to use this example.


IDENTIFY A TEST DATABASE:
-------------------------

Determine the name of a Domino database file, such as intro.nsf, that
resides in a local or remote Domino or Notes data directory.
intro will work with any valid Domino database file.


RUNNING getmultnoteinfo:
-------------

The command line of intro is  -

    getmultnoteinfo [server name - optional] <database filename>

where:

    [server name - optional] is the name of a server where the database
                         resides. 
                         Required for remote access, 
                         omit it for local access. 

    <database filename> is the filename of the Domino database to read


EXAMPLE:

Under Windows, or UNIX (AIX or Linux),
for local access type:

    getmultnoteinfo  intro.nsf

for remote access type:

    getmultnoteinfo  valid_server_name intro.nsf
Under IBM i,
1. Compile the makefile (CLP):

   WRKMBRPDM FILE(QNTCSDK/getmult)
 select the CLP "MK_GETMULT" and apply option 14=Compile.
 
2. Compile the sample:

   CALL PGM(QNTCSDK/MK_getmult)
   
3. Set up the database and data file:

   COPY OBJ('/qntcsdk/notedata/intro.nsf')
        TODIR('/dom719/notes/data')
        
4. Run the sample:
for local access type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/getmult)
    parm(intro.nsf)) batch(*no)

for remote access type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/getmult)
    parm(valid_server_name intro.nsf)) batch(*no)


If the program runs correctly, the program will show these lines below:
        Inserted note 20FA into table.
        SearchMatch has a UNID 85255E18:00630450-85255BF2:006FFBAF
        Inserted note 20FE into table.
        SearchMatch has a UNID 85255E18:00630450-85255BF2:007010A5
id[0] got by NoteID is 20FA
idByUNID[0] got by UNID is 20FA
oid[0] got by NoteID has a UNID 85255E18:00630450-85255BF2:006FFBAF
oidByUNID[0] got by UNID has a UNID 85255E18:00630450-85255BF2:006FFBAF
id[1] got by NoteID is 20FE
idByUNID[1] got by UNID is 20FE
oid[1] got by NoteID has a UNID 85255E18:00630450-85255BF2:007010A5
oidByUNID[1] got by UNID has a UNID 85255E18:00630450-85255BF2:007010A5

EXAMINING THE DOMINO DATABASE TO CHECK THE RESULT:
--------------------------------------------------

To see if 'getmultnoteinfo' worked correctly:
    Use Notes to open intro.nsf

    Choose File/Database/Properties...

    You can check the NoteID & UNID of each document in the database.



