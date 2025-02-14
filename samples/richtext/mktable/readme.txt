Licensed Materials - Property of HCL
CIR9ZEN
CIRA0EN
CIRA2EN
CIRA1EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
mktable - Sample HCL C API for Domino and Notes program: 
          shows how to make tables in rich text fields.

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

mktable.c    - Main program.
cdrecord.c   - A collection of buffer definition and creation routines.
cdtable.c    - A collection of richtext field table creation routines.
mktable.h    - Header file  for this sample.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

aix.mak          - Make file for IBM AIX.

aix64.mak    - Make file for IBM AIX64.


linux.mak    - Make file for Linux.

linux64.mak    - Makefile for Linux 64 bit.

mk_mktable_i.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_mktable_p.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.    

macosx.mak   - Make file for Mac. 


RUNNING mktable
---------------
- Under Windows, or UNIX (AIX, Linux ),
  type the following at the system command prompt:

       mktable [<database>]

where:

  <database> is the filename of the Domino database to operate on.
  If this argument is omitted, the program will prompt for it.
  For testing, you should use the "Test of Tables" database, tabletst.nsf,
  included with the HCL C API for Domino and Notes toolkit.

EXAMPLE
-------

  mktable tabletst
- Under IBM i type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/mktable)
        parm(tabletst.nsf)) batch(*no)


CHECKING THE RESULTS
--------------------
To see if mktable worked correctly:

  Open the Test of Tables database (tabletst.nsf).

  Open the newly created document.  You should see a brief paragraph
  followed by three different tables.
