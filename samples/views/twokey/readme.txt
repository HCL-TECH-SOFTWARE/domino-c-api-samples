Licensed Materials - Property of HCL
CIR9ZEN


CIRA1EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
twokey.c - Example HCL C API for Domino and Notes Program:
           Use a Two-Part Key to Find Documents


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM AIX
Linux 64 bit
MAC OS X
IBM i


PURPOSE
-------
This program illustrates how to find documents in a view that contains
multiple sort columns.


FILES
-----
twokey.c      - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

aix.mak          - Make file for IBM AIX.

aix64.mak        - Make file for IBM AIX64.


mk_twokey_i.clp  - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_twokey_p.clp  - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.     

linux.mak     - Make file for Linux.

linux64.mak    - Makefile for Linux 64 bit.

macosx.mak   - Make file for Mac.

readme.txt    - This file specifies what is needed to use this example.


RUNNING twokey
--------------
This sample program runs against Domino database twokey.nsf, provided with
the HCL C API for Domino and Notes toolkit. Copy twokey.nsf to the
Domino or Notes Data directory before running this program.
twokey will always assume that the database to be opened is twokey.nsf
and the collection to be searched is the collection of the view KEYVIEW.

- Under Windows, or UNIX (AIX, HP, Linux),
  type the following at the system command prompt:
 
        twokey <nsf file name> <View name> <primary key> <secondary key>
        
 
  where:
 
  <nsf file name> is the nsf file name on which the sample will run ,<View name> is the view to be opened,<primary key> and <secondary key> are the search strings that Domino and 
  Notes will match in the primary and secondary sort columns of the collection.
  If no command line arguments are provided, the program will prompt for them.
 

EXAMPLE:

      twokey "twokey.nsf" "KeyView" "Elvis" "99"


- Under IBM i type,
      rundomcmd server(<server name>) cmd(call pgm(qntcsdk/twokey)
		parm(Elvis '99')) batch(*no)

Here is what twokey will display:
--------------------------------

Note count is 1.         NoteID is: 210A
Note count is 2.         NoteID is: 210E


EXAMINING THE DOMINO DATABASE TO CHECK THE RESULT:
--------------------------------------------------

In Notes, open the twokey Domino database.
In the menu, for each document with the sort string "Elvis" and the
number "99", choose File/Document Properties... item, select the 
Document IDs tab (sixth tab from the left) of the document's properties 
InfoBox and look at the last line of the ID value.  It should end with one 
of the NoteIDs that were in the output.
