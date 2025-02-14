Licensed Materials - Property of HCL

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
-------
findbykeyextend.c - Example HCL C API for Domino and Notes Program:
           Use a Two-Part Key to Find Documents


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM AIX
Linux 64 bit
MAC OS X


PURPOSE
-------
This program illustrates how to find documents in a view that contains
multiple sort columns.


FILES
-----
findbykeyextend.c      - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

aix.mak          - Make file for IBM AIX.

aix64.mak        - Make file for IBM AIX64.

linux.mak     - Make file for Linux.

linux64.mak    - Makefile for Linux 64 bit.

macosx.mak   - Make file for Mac.

readme.txt    - This file specifies what is needed to use this example.


RUNNING findbykeyextend
-----------------------
This sample program runs against Domino database simple.nsf, provided with
the HCL C API for Domino and Notes toolkit. Copy simple.nsf to the
Domino or Notes Data directory before running this program.
findbykeyextend will always assume that the database to be opened is simple.nsf
and the collection to be searched is the collection of the view KEYVIEW.

- Under Windows, or UNIX (AIX, HP, Linux),
  type the following at the system command prompt:
 
        findbykeyextend <nsf file name> <View name> <primary key> <secondary key>
        
 
  where:
 
  <nsf file name> is the nsf file name on which the sample will run ,<View name> is the view to be opened, <primary key> and <secondary key> are the search strings that Domino and 
  Notes will match in the primary and secondary sort columns of the collection.
  If no command line arguments are provided, the program will prompt for them.
 

EXAMPLE:

      findbykeyextend "simple.nsf" "KeyView" "This is another line of simple text." "836"


Here is what findbykeyextend will display:
-----------------------------------------

Note count is 1.         NoteID is: 8FA


EXAMINING THE DOMINO DATABASE TO CHECK THE RESULT:
--------------------------------------------------

In Notes, open the simple Domino database.
In the menu, for each document with the sort string "This is another line of simple text." and the
number "836", choose File/Document Properties... item, select the 
Document IDs tab (sixth tab from the left) of the document's properties 
InfoBox and look at the last line of the ID value.  It should end with one 
of the NoteIDs that were in the output.
