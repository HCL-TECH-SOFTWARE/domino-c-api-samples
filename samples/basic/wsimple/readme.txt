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
wsimple - Write Simple Data Types


PURPOSE
-------
This program creates a note in a Domino database and writes simple data
(text, number, text list, and time/date) in the appropriate fields.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM AIX
Linux 64 bit
MAC OS X
IBM i


FILES
-----
wsimple.c    - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.


aix.mak      - Makefile for IBM AIX.

aix64.mak      - Makefile for IBM AIX64.

linux64.mak    - Makefile for Linux 64 bit.

mk_wsimple_i.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_wsimple_p.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.     

macosx.mak   - Make file for Mac.

readme.txt   - This file specifies what is needed to use this example.


RUNNING wsimple
---------------

This sample program will run against the Domino database simple.nsf,
provided with the HCL C API for Domino and Notes toolkit.  You must copy
this file to the Domino or Notes data directory before running this program.

- Under Windows, or UNIX,
  type:
       wsimple
  at the system command prompt.
- Under IBM i type:
    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/wsimple))
        batch(*no)


This program will create a note in the database simple.nsf
containing data of types text, number, text list, and time/date.


EXAMINING THE DOMINO DATABASE TO SEE THE CHANGED DOCUMENTS
----------------------------------------------------------
To see if wsimple worked correctly:

Open simple.nsf.

A new document will be displayed in the main view.  Open the document
and confirm that it contains data of types text, number, text list, and
time/date.
