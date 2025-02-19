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
readform - Sample HCL C API for Domino and Notes Program: 
           shows how to read a Form note


PURPOSE
-------
This program reads the specified form note, displays a list of the
fields on the form and the datatypes of those fields.


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
readme.txt   - This file specifies what is needed to use this example.

readform.c   - Main program
readform.h   - Function prototypes and symbolic constants

mswin32.mak     - Make file for Windows 32-bit.
mswin64.mak     - Make file for Windows 64-bit.

aix.mak          - Make file for IBM AIX.

aix64.mak    - Make file for IBM AIX64.


linux.mak    - Make file for Linux.

linux64.mak    - Makefile for Linux 64 bit.

mk_readform_i.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being an IFS file.
    
mk_readform_p.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being in a source physical file. 

macosx.mak   - Make file for Mac.    


RUNNING readform
----------------
This sample program will run against the Domino database readform.nsf,
provided with the HCL C API for Domino and Notes toolkit.  You must copy 
this file to the Domino or Notes data directory before running this program.

- Under Windows, or UNIX (AIX, Linux ),
  type the following at the system command prompt:

       readform

- Under IBM i type:
    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/readform))
        batch(*no)
