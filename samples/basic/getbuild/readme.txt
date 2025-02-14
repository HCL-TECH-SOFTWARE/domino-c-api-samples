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
getbuild - Gets the Domino or Notes build number using the HCL C API 
           for Domino and Notes API function NSFDbGetBuildVersion. 


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
getbuild.c   - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.


aix.mak          - Make file for IBM AIX.

aix64.mak    - Make file for IBM AIX64.

linux64.mak    - Makefile for Linux 64 bit.

  
mk_getbuild_i.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being an IFS file.
    
mk_getbuild_p.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being in a source physical file.     

macosx.mak   - Make file for Mac.
      
readme.txt   - This file Specifies what is needed to use this example.


RUNNING getbuild
----------------
You may run this program against any Domino database.

- Under Windows or UNIX (AIX, and Linux) type the
  following at the system command prompt:
        
        getbuild <database filename>
                or
        getbuild

  where:

  <database filename> is the filename of the Domino database to operate on.

  If the database filename is not specified, the program will prompt for it.

- Under IBM i, type  
        rundomcmd server(<servername>) cmd(call pgm(qntcsdk/getbuild)
	      parm(<database filename>)) batch(*no)
     or
        rundomcmd server(<servername>) cmd(call pgm(qntcsdk/getbuild))
	      batch(*no)
