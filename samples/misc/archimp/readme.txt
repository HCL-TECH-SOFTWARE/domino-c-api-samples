Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN
 
(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.



PROGRAM
-------
archimp - import the archived file to the specified note and restore
          the note back to the db.


PURPOSE
-------
This program shows the usage of archive import database. It imports 
the specified file and related attachments to the specified note, and then 
restore the note back to the target database. The file must first be created 
using the ArchiveExportDatabase API (see the archexp exaxmple).


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
Linux (64-bit)
IBM i


FILES
-----
archimp.c    - Main program.

mswin32.mak  - Make file for Windows 32-bit.

mswin64.mak  - Make file for Windows 64-bit.

mk_archimp_i.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_archimp_p.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.     

readme.txt   - This file specifies what is needed to use this example.


IDENTIFY A TEST DATABASE:
-------------------------

Determine the name of a Domino database file, such as admin.nsf, that
resides in a local or remote Domino or Notes data directory.


RUNNING archexp:
----------------
- Under Windows 32-bit or UNIX (AIX and Linux), type the
  following at the system command prompt:

        archimp  <Input file with full path> <Target database> [options]

  where:

  <input file with full path> is the filename of the file which will be imported to the note.  Please give the entire full path for the file.

  <Target database> is the filename of the domino database in which the note
                    will be restored.
  
  [options] : -s ServerName
            is the name of a server where the database resides. 
            Required for remote access, omit it for local access.

  If the command line arguments are not specified, the program will
  prompt for them.



EXAMPLE:

This example uses the Domino database admin.nsf provided with the HCL C API
 for Domino and Notes toolkit. Copy this file to the Domino or Notes 
data directory. You may also want to import the admin.dat and related attachments 
to a note and then restore the note back to target db.

      archimp C:\samples\admin.dat admin.nsf

NOTE:
this program takes the note represented in the input file and 
replaces any note found in the target DB with the same UNID. 
 
- Under IBM i type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/archimp)
        parm(admin.dat admin.nsf)) batch(*no)
  

CHECKING THE RESULT:
--------------------
To see if archimp worked correctly:

Open the domino database admin.nsf.

The note exported before in admin.dat and the related attachment are restored.

