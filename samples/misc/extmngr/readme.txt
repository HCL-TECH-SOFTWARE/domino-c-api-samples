Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
-------
extmngr - Example Extension Manager

This program registers and deregisters all of the Extension Manager
functions to show how they interact in an application and transforms
a non Domino database "animals.db" into a Domino database "animals.nsf"
when specific extension manager hooks are called.

When the program traps an EM_NSFDBCREATE and EM_NSFDBCLOSE hook for the
Database "animals.nsf", the data in the database is built from an the
existing file "animals.db" containing information and transformed into a
Domino Database. The extension manager hooks are deregistered when Domino or
Notes is exited.  The log file OUTPUT.LOG, tracks extension manager calls
and is created in the Domino or Notes data directory.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM i


FILES
-----
extmngr.c    - Main C source code file for Extension Manager
cdrecord.c   - Source code needed for creating cdrecords.
createdb.c   - Source code needed for creating forms, notes,
               and views in database.
extmngr.h    - Constants and function prototypes.
animals.db   - Non Notes database.

mswin32.mak     - Make file for Windows 32-bit.
mswin64.mak     - Make file for Windows 64-bit.
mswin32.def  - Module definition file for Windows 32-bit.
mswin64.def  - Module definition file for Windows 64-bit.

extppc.r     - Resources for the Power PC version

extmngr.exp  - Exports for the Power PC

mk_extmngr_i.clp  - Make file (CL Program) for IBM i using ILE C 
                       with the source code being an IFS file. 
                          
mk_extmngr_p.clp  - Make file (CL Program) for IBM i using ILE C 
                       with the source code being in a source physical file. 

readme.txt   - This file specifies what is needed to use this example.

 


OPERATING ENVIRONMENT REQUIREMENTS
----------------------------------
To install and run this program:

    1.  Exit Domino or Notes if it is running.

    2.  For Windows, copy NEXTMNGR.DLL to the Domino or Notes Program
        directory.

    3.  Edit the notes.ini file.
        For Windows programs, if an EXTMGR_ADDINS entry does not exist,
        add the line:

            EXTMGR_ADDINS=NEXTMNGR

- Under IBM i:
		Add the Symbolic Link to the file:
			ADDLNK OBJ('/QSYS.LIB/qntcsdk.LIB/libextmngr.SRVPGM')            
				NEWLNK('/QIBM/USERDATA/LOTUS/NOTES/LIBEXTMNGR.SRVPGM')       
				LNKTYPE(*SYMBOLIC) 

		Add the following line to notes.ini:
			EXTMGR_ADDINS=LIBEXTMNGR

        Save the modified notes.ini file.

        Alternatively, you may omit the one-character prefix in the
        extension manager program name.  In other words, you can use:
        EXTMGR_ADDINS=EXTMNGR

        If an EXTMGR_ADDINS entry already exists in notes.ini, add
        ",EXTMNGR" to the end of the existing line. The one-character
        prefix in the extension manager program name is optional.
		
    4.  Copy "animals.db" to the Domino or Notes data directory.

    5.  Launch Domino or Notes.

    6.  From a Notes client, select File, Database, New.

    7.  If this sample is running on a server, enter the server name.
        Tab to Title: and enter "animals".  The File Name will be
        automatically filled in with "animals.nsf".  Click on OK.  If
        the animals.nsf database already exists, click on YES to replace
        the existing database.

    8.  At the Main View of the database select "Animals View".


CHECKING THE RESULTS
--------------------

    There should be 5 documents listed under the "Animals View".  Open
    the "Grizzly Bear" document and the following should be displayed:

    Grizzly Bear

    Latin Name:  Ursus arctos horribilis

    Range:       North America

    Status:      Endangered

    EXTMNGR.DLL also creates a log file called OUTPUT.LOG in the Domino
    or Notes data directory.  It tracks all Extension Manager registration
    calls, EM_NSFDBCREATE and EM_NSFDBCLOSE Extension Manager hook calls to
    the "animals.nsf" database, and all deregistration calls when Domino or
    Notes is exited.

    Exit Domino or Notes to deregister the Extension Manager hooks.
