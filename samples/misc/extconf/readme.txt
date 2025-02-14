Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
extconf - Extension manager replication conflict handler


PURPOSE:
--------
This Dynamic Link Library (DLL) demonstrates how to write an extension
manager replication conflict handler.  It intercepts the Domino
notification that a replication conflict occurred.

When a replication conflict between two documents occurs in the
sample database, Conflict Handler, this extension manager examines each
item in the "newer" document.  If the item is the Payout field, it
compares the value of this item with the one in the older document and
saves the lower value.  For all other items, it determines whether there
is a conflict between the two documents.  If there is a conflict, the
newer item value is saved.  The extension manager then returns
CONFLICT_ACTION_MERGE to the HCL Domino Server so that the two
documents will be merged and a Replication or Save Conflict document
will not be posted.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM i


FILES:
------
extconf.c    - Extension manager entry point and callback.

extcwin.c    - Windows DLL code.

extconf.h    - Header file for sample program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

mswin32.def  - DLL module definition file for Windows 32-bit.

mswin64.def  - DLL module definition file for Windows 64-bit.

readme.txt   - This file specifies what is needed to use this example.

mk_extconf_i.clp  - Make file (CL Program) for IBM i using ILE C 
                       with the source code being an IFS file. 
                          
mk_extconf_p.clp  - Make file (CL Program) for IBM i using ILE C 
                       with the source code being in a source physical file.  

USING EXTCONF
-------------

The name of the DLL depends on the platform.  The names used are:

        nextconf.dll            Windows

The DLL must be copied to the executable directory of the HCL Domino
Server and to the executable directory of your workstation.

With the HCL Domino Server and your workstation shut down, add the
following line to notes.ini on the HCL Domino Server and on your
workstation:

        EXTMGR_ADDINS=extconf

- Under IBM i:

Add the Symbolic Link to the file:
    ADDLNK OBJ('/QSYS.LIB/qntcsdk.LIB/libextconf.SRVPGM')            
        NEWLNK('/QIBM/USERDATA/LOTUS/NOTES/LIBEXTCONF.SRVPGM')       
        LNKTYPE(*SYMBOLIC) 

Add the following line to notes.ini:
	EXTMGR_ADDINS=LIBEXTCONF
			 
			 
Bring up the HCL Domino Server and bring up Notes on your workstation.

This sample program will run against the Domino database conflict.nsf,
provided with the HCL C API for Domino and Notes toolkit.  In order to
enact a replication conflict, copy this file to the Domino data directory
on a HCL Domino Server.  Then, make a replica copy of this database on
your local workstation.

From your workstation open the sample database on the server and edit the
Customer Name and Payout fields of the document.  Save the document.
Open the local replica copy of the sample database.  Edit the Payout
field to be a higher value than the edit you made on the server.  Also
change the Customer Name field using a different name than what is on the
server's document.  Save the document.

Replicate the local database with the server.  When replication has
completed open the database on the server.  Notice that there is no
replication or save conflict document.  Also notice that the Payout field
contains the lower of the two edited values.  The Customer Name field
contains the newer of the two edited values.

