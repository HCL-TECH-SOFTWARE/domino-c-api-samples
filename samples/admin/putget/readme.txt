Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
putget - HCL C API for Domino and Notes Sample program


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM i
Linux 64 bit

FILES
-----
readme.txt    - This file specifies what is needed to use this example.

putnote.txt    - Sample import data for PutNote program.
pg_doc.nsf     - User Guide for this sample.

fields.c       - Functions for reading/writing various types of Notes fields.
fields.h       - Prototypes for reading/writing various types of Notes fields.
file_io.c      - Functions for platform-dependent file I/O.
file_io.h      - Prototypes for platform-dependent file I/O functions.
misc.c         - Miscellaneous functions for PutNote & GetNote programs.
misc.h         - Miscellaneous prototypes for PutNote & GetNote programs.
executeputnote.c      - Source for main routine of PutNote program.
getnote.c      - Source for main routine of GetNote program.
putget.h       - Prototypes for main putnote & getnote routines.
constant.h     - Global constants for GetNote & PutNote programs.
put_ui.c       - Front-end for PutNote program.
get_ui.c       - Front-end for GetNote program.

mswin32.mak   - Makefile for Windows (32-bit).

mswin64.mak   - Makefile for Windows (64-bit).

linux64.mak    - Makefile for Linux 64 bit.

misci5.h       - Miscellaneous prototypes for PutNote & GetNote programs for IBM i.

mk_putget_i.clp    - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_putget_p.clp   - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.    

RUNNING putget
--------------
All information about PutNote and GetNote is contained in the Domino database
PG_DOC.NSF.  This database is the User Guide for PutNote and GetNote.

Please add this database to your Notes workspace and refer to it for all
further information about this sample.

The command line of putget is  -

    executeputnote  /s=<servername> /D=<databasename> /T=<textfilepath>

    getnote /D=<databasename>

EXAMPLE:

Under Windows or Linux type:

The command line of executeputnote is  -

    executeputnote /S="dominoserver" /D="pg_test" /T="/samples/admin/putget/putnote.txt"

If the program runs correctly, it should display a message with the following similar output for executeputnote:

Form #1
Here is a plain text field.
a|b|c
123
1|2|3.005
Form #1
Here is a another plain text field.
09/05/90 12:07 AM
7/04/53 7:20 AM|5/30/55 9:00PM
This is a rich text field.  Here comes a new line
Here is another line...
Here is another line...
Here is another line...
Here is another line...
Phil Margolis

Program completed successfully.

The command line of getnote is  -

    getnote /D="pg_test"

If the program runs correctly, it should display a message with the following similar output for getnote:

Form #1
Here is a plain text field.
a|b|c
123
1|2|3.005
CN=dominoserver/O=HCLPNPORGTEST
Form #1
Here is a another plain text field.
09-05-1990 00:07:00
07-04-1953 07:20:00|
This is a rich text field.  Here comes a new line
Here is another line...
Here is another line...
Here is another line...
Here is another line...
Phil Margolis
CN=dominoserver/O=HCLPNPORGTEST