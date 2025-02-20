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
msimple - Modify a field of all documents in a database.


PURPOSE      
-------
This program finds all the documents in a database that contain a numeric 
field called "NUMBER", and adds one to the value in that field in all 
documents.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM AIX
Linux 64 bit
Linux for zSeries
MAC OS X
IBM i


FILES
-----
msimple.c    - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.
 

aix.mak          - Make file for IBM AIX.

aix64.mak    - Make file for IBM AIX64.

linux64.mak    - Makefile for Linux 64 bit.

mk_msimple_i.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_msimple_p.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.    

macosx.mak   - Make file for Mac. 

readme.txt   - This file Specifies what is needed to use this example.


RUNNING msimple
---------------
- Under Windows or UNIX (AIX, and Linux),
  type the following at the system command prompt:
  
        msimple  <database filename>       
                  or
        msimple

  where:
  <database filename> is the filename of the Domino database to be modified.
  If the database filename is not specified, the program will prompt for it.


- Under IBM i type:
    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/msimple)
        parm(simple.nsf)) batch(*no)
  or
    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/msimple))
        batch(*no)
NOTE:  This program searches for documents with the numeric field "NUMBER".  
In order to perform a valid test, you should run msimple against a 
database, like simple.nsf, that contains documents where this field 
is present.

