Licensed Materials - Property of HCL
CIRA0EN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2021
All rights reserved.


PROGRAM
---------------
idvault

PURPOSE
-------------
Execute this program to illustrate how to upload id file to ID vault,
get id file from ID vault, sync id file contents with the ID vault and get user name of the provided ID file.

ENVIRONMENTS
-----------------------
Windows (32-bit)
Windows (64-bit)
Linux (64-bit)
IBM i

FILES
-----
readme.txt       - This file specifies what is needed to use this example.

idvault.c        - Main program.

mswin32.mak      - Make file for Windows 32bit.

mswin64.mak     - Make file for Windows 64bit.

linux64.mak      - Make file for Linux 64bit.

mk_copydb_i.clp  - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_copydb_p.clp  - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.

readme.txt       - This file Specifies what is needed to use this example.

RUNNING idvault:
------------------------
Before running this program, please create an ID vault on Domino Server.
(Detail information please see "Creating and configuring an ID vault" section in Domino Administrator help documents)

Also create four users whose last names(equals short name) are s1, s2, s3, ss, whose password should be "123456".

The id file names should be  "s1.id","s2.id","s3.id","ss.id".
Then save these id files to domino data directory. 

In Domino Administrator select each user from "People" and select "Edit Person". Now goto "Administration" tab,
in "Policy Management" for "Assigned policy" select the idvault created and then Save & Close.

Refresh the Domino Administrator after making changes to each user since it takes time to accept the changes.
Otherwise, we may get "Entry not found in index" during execution.

- Under Windows or Linux
  at the system command prompt:

  idvault <server name> <domain>

Note: Provide full path for id file in code incase of error like Illegal use of noteid.
Note: For Windows 32 bit execution make sure admin.id is already uploaded to idvault along with above steps.

EXAMPLE:
--------------
      idvault "CN=LGCToolkit/O=toolkit" "toolkit"

- Under IBM i, type  

        RUNDOMCMD SERVER(<servername>) CMD(CALL PGM(QNTCSDK/IDVAULT) PARM('server name' 'domain name'))

CHECKING THE RESULTS
------------------------------------
Open idvault db.

You can find "s1.id","s2.id","s3.id" document in this db.

SAMPLE OUTPUT
-------------------------

SECKFMGetIDFlags returned ID flags 1.
SECKFMGetIDFlags returned ID as password protected.
SECKFMGetUserInfo returned username CN=ss/O=HCL.
Put ID file using the hKFC...OK
Put ID file using local file name and hKFC - only hKFC should be used...OK
Put ID file using local file name...OK
Get id file to local file name and hKFC...OK
Get id file to just hKFC...OK
Get id file to just local file name...OK
Sync the hKFC with the ID file in the Vault and write new hKFC...OK
Sync the ID file with the ID file in the Vault ...OK
Sync the hKFC with the ID file in the Vault and hKFC...OK

Program completed successfully.

