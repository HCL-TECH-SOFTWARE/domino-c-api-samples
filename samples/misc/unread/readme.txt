Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN
CIRA1EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
-------
unread - Manipulate unread lists


PURPOSE:      
-------       
This program will display or modify a user's unread note list.

This program also demonstrates how to update a note and then change
its unread status back to what it was before the update occurred.  To
activate this demonstration, specify the simple.nsf sample database and 
specify a currently unread noteid to be deleted from the unread list.  
This note will be deleted from the unread list and then it will be modified 
and updated.  The update will cause the note to become unread.  It will 
then be converted back to read.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM AIX
Linux  64 bit
MAC OS X
IBM i


FILES
-----
readme.txt   - This file specifies what is needed to use this example.

unread.c     - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

aix.mak      - Makefile for IBM AIX.

aix64.mak      - Makefile for IBM AIX64.


linux.mak    - Make file for Linux RedHat.

linux64.mak    - Makefile for Linux ReadHat 64 bit.

mk_unread_i.clp  - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_unread_p.clp  - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.    

macosx.mak   - Make file for Mac. 


IDENTIFY A TEST DATABASE:
-------------------------
Determine the name of a Domino database file, such as simple.nsf, that
resides in the Domino or Notes data directory on your system. Unread will
work with any valid Domino database file.


RUNNING unread
--------------
- Under Windows, or UNIX use the following syntax at the
  system command prompt:

  unread <database> [<username>] [+/-<NoteID>] ...

where:

  <database> is the filename of the Domino database to operate on
  <username> Optional user name.  This must be the complete user
             name, as it appears in the user's User ID.
             If no name is supplied, the name from the current
             User ID will be used.  If the name is hierarchical,
             the fully-qualified user name must be provided.
  <NoteID>   Optional Note ID to be added to (+) or removed
             from (-) the unread note list.

EXAMPLES
--------

To display the unread list for the current user, type:

  unread simple

To display the unread list for a specific user, say, Richard Roe, type:

  unread simple "Richard Roe"

To remove a Note from the unread list (so the document will appear as Read):

  unread simple -8FA
In the above example, the simple.nsf database is used, the sample removes
note 8FA from the unread list.  Then, it updates the "plain_text" field
in the note, this update forces the note to become unread again.  The 
unread mark is then reset to be what is was before the note was updated.

To add a Note to the unread list (so the document will appear as Unread):

  unread simple +8FA
- Under IBM i:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/unread)
        parm(simple.nsf)) batch(*no)

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/unread)
        parm(simple.nsf 'Richard Roe')) batch(*no)

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/unread)
        parm(simple.nsf '-8FA')) batch(*no)

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/unread)
        parm(simple.nsf '+8FA')) batch(*no)
