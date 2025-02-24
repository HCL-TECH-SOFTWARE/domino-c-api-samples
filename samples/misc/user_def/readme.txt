Licensed Materials - Property of HCL
CIRA0EN
CIRA2EN
CIRA1EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
-------
c_usrdef - Create and Write a field of a user defined data type to a note
r_usrdef - Read a field of a user defined data type from a note


PURPOSE
-------
These programs illustrate how to create a field that has a user-defined
data type in a Domino document.  User-defined types are useful if you want
to store your own kind of object in a document.  Some examples of these
objects are: graphics formats that Domino and Notes do not understand, time
stamps of your own creation, encrypted data that uses your own encryption
scheme, etc.

The program C_USRDEF creates a Domino document with three fields.  Two
of the fields are standard types (text and time).  The third field contains
a user-defined data type.

The program R_USRDEF scans a Domino database to find documents that 
contain the special field created by C_USRDEF.  On finding such a field,
the program extracts the user-defined data and prints it out.


ENVIRONMENT
-----------  
IBM AIX
Linux 64 bit
MAC OS X
IBM i
Windows (32-bit)
Windows (64-bit)


FILES
-----
readme.txt   - This file specifies what is needed to use this example.

c_usrdef.c   - The sample program that creates a user-defined field.

r_usrdef.c   - The sample program that reads a user-defined field.

proc_arg.c   - Function to process command line arguments.

user_def.h   - A local include file used by both sample programs.

aix.mak          - Make file for IBM AIX.

aix64.mak    - Make file for IBM AIX64.


linux.mak    - Make file for Linux.

linux64.mak    - Makefile for Linux 64 bit.

mk_user_def_i.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being an IFS file.
    
mk_user_def_p.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being in a source physical file.     

macosx.mak   - Make file for Mac.


RUNNING w_usrdef
----------------
Copy the Domino database file user_def.nsf (included with the toolkit) to
the the Domino Data directory.

- Under UNIX (AIX, Linux ),
  type the following at the system command prompt:

  c_usrdef user_def

  If no command line argument is provided, the program will prompt for it.
- Under IBM i type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/w_usrdef)
		parm(user_def.nsf)) batch(*no)

CHECKING THE RESULTS
--------------------
Using Notes, open the database user_def.nsf. Select the default view.

You will see a document with the current time in its "Time/Date" field.
Highlight this document.

From the File - Document Properties InfoBox, select the "Fields" tab.
You will see all of the fields in the document, including the field with
the user-defined data type.


RUNNING r_usrdef
----------------
- Under UNIX (AIX, Linux ),
  type the following at the system command prompt:

  r_usrdef user_def

  If no command line argument is provided, the program will prompt for it.

- Under IBM i type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/r_usrdef)
		parm(user_def.nsf)) batch(*no)
CHECKING THE RESULTS
--------------------
The program will find all the documents that contain the user-defined
field, display the document's Node ID, and print out the contents of the
user-defined field.
