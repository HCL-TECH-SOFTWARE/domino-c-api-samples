Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN
 
(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.



PROGRAM
-------
archexp - export a data note from db into output file.


PURPOSE
-------
This program shows the usage of archive export database. It exports a 
single data note from the indicated source database to an output file, 
and its attachments are exported too.


ENVIRONMENTS
------------
Windows 32-bit
Windows 64-bit
Linux 64-bit
IBM i


FILES
-----
archexp.c    - Main program.

mswin32.mak  - Make file for Windows 32-bit.

mswin64.mak  - Make file for Windows 64-bit.

mk_archexp_i.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_archexp_p.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.     

readme.txt   - This file  specifies what is needed to use this example.


IDENTIFY A TEST DATABASE:
-------------------------

Determine the name of a Domino database file, such as admin.nsf, that
resides in a local or remote Domino or Notes data directory.


RUNNING archexp:
----------------
- Under Windows 32-bit or UNIX (AIX and Linux), type the
  following at the system command prompt:

        archexp <source DB> <hex note ID> <output file with full path> [server name - optional]
                  or
        archexp

  where:

  <source DB> is the filename of the source domino database

  <hex note ID> is the NoteID of the note to be exported of source DB.

  <output file with full path> is the filename of the file where note will be exported.  Please give the entire full path for the file.
  
  [options] : -s ServerName
            is the name of a server where the database resides. 
            Required for remote access, omit it for local access.

  If the command line arguments are not specified, the program will
  prompt for them.



EXAMPLE:

This example uses the Domino database admin.nsf provided with the HCL C API 
for Domino and Notes toolkit. Copy this file to the Domino or Notes 
data directory. You may also want to export the document whose NoteID is 0x00001102 
to admin.dat for archiving.

      archexp  admin.nsf  0x00001102  C:\samples\admin.dat

- Under IBM i type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/archexp)
        parm(admin.nsf  0x00001102  admin.dat)) batch(*no)


CHECKING THE RESULT:
--------------------
To see if archexp worked correctly:

Open the current program running directory.

This should display the admin.dat and the related attachment of the note.

