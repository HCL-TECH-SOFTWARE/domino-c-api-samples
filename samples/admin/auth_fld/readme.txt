Licensed Materials - Property of HCL
CIRA0EN
CIRA2EN
CIRA1EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
-------
auth_fld  - Example HCL C API for Domino and Notes Program: Creates items 
	    with data type "Names", "Author Names", and "Reader Names".

PURPOSE
-------
This program demonstrates how to append author fields to documents. It
creates items with data type "Names", "Author Names", and "Reader Names".


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM AIX
IBM i
Linux 64 bit
MAC OS X


FILES
-----
readme.txt     - This file specifies what is needed to use this example.

maindoc.txt    - An ASCII text file you can specify as <textfile1>
respdoc.txt    - An ASCII text file you can specify as <textfile2>

auth_fld.c     - Source code file.
auth_fld.h     - Header file with declarations and definitions.

aix.mak          - Make file for IBM AIX.

aix64.mak        - Make file for IBM AIX64.


mk_auth_fld_i.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being an IFS file.
    
mk_auth_fld_p.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being in a source physical file.     

linux.mak      - Make file for Linux.

linux64.mak    - Makefile for Linux 64 bit.

macosx.mak   - Make file for Mac.

(Note: The data files maindoc.txt and respdoc.txt are also stored as IFS 
 files: /qntcsdk/notedata/maindoc.txt and /qntcsdk/notedata/respdoc.txt.) 


RUNNING auth_fld:
-----------------
Prior to compiling this sample, it is suggested that the line,

  #define READERS1        "CN=Jane Doe/OU=Sales/O=ABCorp"

in the file "auth_fld.h", be modified to reflect your fully canonicalized 
user name.  If this is not done and you specify a database on a HCL Domino 
server, the sample will run successfully, but the documents created will not 
be visible to you because you will not be listed in the Reader Names field.
 

- Under UNIX (AIX, Linux ),
  type the following at the system command prompt:

  auth_fld

      or

  auth_fld <server> <database> <textfile1> <textfile2>

where:

   <server> is the HCL Domino server where the destination database
   resides. This option is required.  You may specify "" for <server> 
   to indicate the database is in the local Domino data directory.

   <database> must specify the name of a Domino discussion database that has
   the design of DISCUSS.NSF (included with the API toolkit).
   Also, the user running this API program must have author access or
   better to this database.

   <textfile1> must name an ASCII text file residing in the current
   working directory. A sample text file, MAINDOC.TXT, is provided in
   this directory for use as <textfile1>. The contents of <texfile1>
   becomes the body of the main topic document. The first line of
   <textfile1> is used as the subject of the main topic document.

   <textfile2> must name another ASCII text file residing in the current
   working directory. A sample text file, RESPDOC.TXT, is provided in
   this directory for use as <textfile2>. The entire contents of <texfile2>
   becomes the body of the response document. The first line of
   <textfile2> is used as the subject of the response.

   If no command line arguments are provided, the program will prompt
   for them.

EXAMPLE
-------

   auth_fld "" discuss maindoc.txt respdoc.txt
under IBM i -
	change your current directory to your data directory or use absolute paths.
	To change the directory, type
	CD '<data directory>'
	To run, type
           rundomcmd server(<server name>) cmd(call 
	       pgm(qntcsdk/auth_fld) parm('' discuss.nsf maindoc.txt respdoc.txt))
	       batch(*no)
