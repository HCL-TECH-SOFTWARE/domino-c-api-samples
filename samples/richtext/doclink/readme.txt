Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
doclink  - Sample HCL C API for Domino and Notes Program: 
           shows how to create a Doc Link, a View Link a Database Link and 
           an Anchor Link.
       

ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM i


FILES
-----
readme.txt   - This file.
doclink.c    - Main C program.
mswin32.mak  - Makefile for Windows 32-bit.
mswin64.mak  - Makefile for Windows 64-bit.
mk_doclink_i.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_doclink_p.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.     


RUNNING doclink
---------------
This sample program uses two Domino databases, doclink1.nsf and doclink2.nsf,
provided with the HCL C API for Domino and Notes toolkit.  You must copy 
these files to the Domino or Notes data directory before running this program.

- Under Windows,
  type:

       doclink

  at the system prompt. The program takes no arguments.

- Under IBM i type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/doclink))
        batch(*no)

CHECKING THE RESULTS
--------------------
This will create one note in each of the two databases doclink1.nsf
and doclink2.nsf.  The note in doclink2.nsf will contain a Doc Link, 
a View Link, a Database Link and an Anchor link to the note in doclink1.nsf.
