Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN
  
(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
sel_rep   - Sample HCL C API for Domino and Notes Program:
            Create a replication formula note.


ENVIRONMENTS
------------
Windows (32-bit)



PURPOSE
-------
This program shows how to create a replication formula note. A replication
formula note allows a database to perform selective replication.


FILES
-----
readme.txt  - This file.

sel_rep.c   - Main program.

sel_rep.h   - Contains symbolic definitions used by the main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

mk_selrep_i.clp    - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_selrep_p.clp   - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.  


RUNNING sel_rep
---------------
This sample program uses the Domino database, reptest.nsf, provided with
the HCL C API for Domino and Notes toolkit.  You must copy this file to
the Domino data directory on a HCL Domino Server.

Next, edit the source code file "sel_rep.c" and change the string defined
in szServerName[] to the name of the server on which "reptest.nsf" was
stored. Build the program executable.

- Under Windows, from your Notes workstation type:
         sel_rep <ServerName>
  at the system prompt. The program takes no arguments.
    
- Under IBM i, type  
    RUNDOMCMD SERVER(<servername>) CMD(CALL PGM(QNTCSDK/SEL_REP)) 
    BATCH(*NO)

sel_rep will create on the local workstation a replica copy of reptest.nsf.
It will then create in the local copy of the database a replication
formula.

- Under IBM i,
    The replica copy of reptest.nsf will be newreptest.nsf.


CHECKING THE RESULTS
--------------------
To check that the program worked properly:

Add the server database and local database to your workstation.
On the server copy of the database, use form 1 to create several documents.
Create some documents with numberfield greater than 100 and some with
numberfield less than 100.  On the workstation, open the local copy of
the database, and force it to replicate with the server copy by choosing
the menu item, File/Replication/Replicate.  Then choose Replicate with
options.  Choose OK.  Check to make sure that the name of the server
containing the database is specified.  Choose OK.  When Notes has finished
replicating, press F9 to refresh the local copy of the database. The
documents with values less than 100 in numberfield will have been
replicated from the server copy to the local copy.


SAMPLE OUTPUT
------------------------

Creating replica database.

Replica database created.

Creating selective replication formula.

Selective replication formula created.

Done.
