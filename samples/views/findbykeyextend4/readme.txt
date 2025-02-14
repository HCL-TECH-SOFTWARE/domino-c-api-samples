Licensed Materials - Property of HCL

(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
findbykeyextend4.c - Example HCL C API for Domino and Notes Program:
                       Use a Two-Part Key to Find Documents using NIFFindByKeyExtended4 API.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
Linux 64 bit


PURPOSE
-------
This program illustrates how to find documents in a view that contains
multiple sort columns.


FILES
-----
findbykeyextend4.c        - Main program.

mswin32.mak               - Make file for Windows 32-bit.

mswin64.mak               - Make file for Windows 64-bit.

linux64.mak               - Makefile for Linux 64 bit.

readme.txt                - This file specifies what is needed to use this example.

findKeyEx4.nsf		  - This NSF file contains needed records for processing.


RUNNING findbykeyextend4
-------------------------
This sample program runs against Domino database findKeyEx4.nsf, provided with
the HCL C API for Domino and Notes toolkit. Copy findKeyEx4.nsf to the
Domino or Notes Data directory before running this program.
findbykeyextend4 will always assume that the database to be opened is findKeyEx4.nsf
and the collection to be searched is the collection of the view KEYVIEW.

- Under Windows, or UNIX (Linux),
  type the following at the system command prompt:
 
        findbykeyextend4 <nsf file name> <View name> <primary key> <secondary key> <num. of docs to return>
        
 
  where:
 
  <primary key> and <secondary key> are the search strings that Domino and 
  Notes will match in the primary and secondary sort columns of the collection.
  If no command line arguments are provided, the program will prompt for them.
 

EXAMPLE:

      findbykeyextend4 "findKeyEx4.nsf" "KeyView" "This is another line of simple text." "836" 2


Here is what findbykeyextend4 will display:
--------------------------------------------
View title is not present.
User Name [CN=DominoServer/O=Domain] View ID[29A].

Notes ID:[8FA] Item Name:[plain_text] Item Value:[This is another line of simple text.]
Notes ID:[906] Item Name:[plain_text] Item Value:[This is another line of simple text.]

Note count is 1.         NoteID is: 8FA
Note count is 2.         NoteID is: 906

No. of Notes Match found[2].

Program completed successfully.

EXAMINING THE DOMINO DATABASE TO CHECK THE RESULT:
--------------------------------------------------

In Notes, open the findKeyEx4.nsf Domino database and open KeyView.
In the menu, for each document with the sort string "This is another line of simple text." and in the
number field "836". Check for NoteId column with the value printed on the console say NT000008FA, NT00000906 
accordingly for NoteID 8FA & 906.
