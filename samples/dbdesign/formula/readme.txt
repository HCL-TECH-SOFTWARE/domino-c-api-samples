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
formula - Evaluate a formula, return a value.


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
formula.c    - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.


aix.mak          - Make file for IBM AIX.

aix64.mak    - Make file for IBM AIX64.

linux.mak    - Make file for Linux.

linux64.mak    - Makefile for Linux 64 bit.

mk_formula_i.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_formula_p.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.     

macosx.mak   - Make file for Mac.

readme.txt   - This file specifies what is needed to use this example.


RUNNING formula
---------------
This sample program will run against the Domino database, "Formula Evaluation 
Test Database", filename, formula.nsf, provided with the HCL C API for
Domino and Notes toolkit.  You must copy this file to the Domino or Notes
data directory before running formula.

- Under Windows or UNIX (AIX, and Linux),
  type:
      formula
  at the system prompt.
- Under IBM i type:
    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/formula))
    batch(*no)


  The program takes no arguments.


This program creates a note containing a number field in the specified 
database, saves the note, and then runs a formula against the newly 
created note.  The formula simply returns the value of the number 
stored in the number field.
   
The program then displays the number value contained in the number 
field in the note.        
 
To check the results of this program, from Notes, add the icon for the 
test database to your desktop.  Open this database, then open the 
document that was just created.  Compare the value stored in the 
Number field to the value that was displayed as output from the program.


