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
fileatt  -  Sample HCL C API for Domino and Notes Program: 
            Attach a file to a note.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM AIX
IBM i
Linux 64 bit
MAC OS X

NOTES.INI ENV SETTINGS:
----------------------
Set these env variables before executing the fileatt.

DEBUG_ALLOW_ZERO_BLOCKS=1
DEBUG_ALLOW_ZERO_BLOCKS_SUPPRESS_MESSAGE=1


FILES
-----
fileatt.c        - Main program.

mswin32.mak     - Makefile for Windows 32-bit.

mswin64.mak     - Makefile for Windows 64-bit.

aix.mak          - Make file for IBM AIX.

aix64.mak        - Make file for IBM AIX64.

mk_fileatt_i.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_fileatt_p.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.     

linux64.mak    - Makefile for Linux 64 bit.

macosx.mak   - Make file for Mac.

readme.txt       - This file specifies what is needed to use this example.


RUNNING fileatt
---------------
Pass fully-qualified path to the input file "readme.txt" (or some
other file valid file on your disk), which is supplied with this sample as command line argument.

Next, ensure that the database embedole.nsf resides in your Domino or Notes
data directory.

- Under Windows,
  type the following at the system prompt:

        fileatt "C:\notesapi\samples\misc\fileatt\readme.txt"

- Under UNIX (Linux),
        fileatt "\opt\notesapi\samples\misc\fileatt\readme.txt"

- Under IBM i, type:

    Copy the file "readme.txt" to the server data directory, and 
     change your current directory to the server data directory
     or specify the fully-qualified path name for "readme.txt" for 
     the variable "szSourceFileName" in the file fileatt.c. 
     Run 
     rundomcmd server(<servername>) cmd(call pgm(qntcsdk/fileatt))
     batch(*no)

FILEATT will create in the database EMBEDOLE.NSF a document which contains
two file attachments.  The first is attached using NSFNoteAttachFile.
The second is attached using low-level, FILEOBJECT APIs.

NOTE: Before executing fileatt, set env variables accordingly, as mentioned in the NOTES.INI env section.


CHECKING THE RESULTS
--------------------

File Created[attach1.txt]
File Created[attach2.txt]
 Attaching attachment 1

 Attaching attachment 2

 Note id: [30aa]
  Object Type = OBJECT_FILE.
  Object Class = NOTE_CLASS_DOCUMENT
  File Name = 'attach1.txt'.
  Compression Type = NONE.
  Object Type = OBJECT_FILE.
  Object Class = NOTE_CLASS_DOCUMENT
  File Name = 'attach2.txt'.
  Compression Type = NONE.

Program completed successfully.

