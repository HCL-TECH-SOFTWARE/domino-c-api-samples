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
copydb - Creates a replica copy of a Domino database and sets the title.


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
copydb.c     - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

aix.mak          - Make file for IBM AIX.

aix64.mak        - Make file for IBM AIX64.

linux64.mak    - Makefile for Linux 64 bit.

mk_copydb_i.clp  - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_copydb_p.clp  - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.   

macosx.mak   - Make file for Mac.  

readme.txt   - This file Specifies what is needed to use this example.


RUNNING copydb
--------------
- Under Windows or UNIX (AIX and Linux),
  type the following at the system command prompt:

         copydb  <input db>  <output db>  <output title>
                          or
         copydb

  where:

  <input db> is the filename of the Domino database to be copied

  <output db> is the filename of the resulting copy of the Domino database

  <output title> is the title of the resulting Domino database

  If no command line arguments are typed, then you will be prompted
  to enter them.


EXAMPLE:

      copydb  simple  simple2  "Copy of Simple"


- Under IBM i, type  
        rundomcmd server(<servername>) cmd(call pgm(qntcsdk/copydb))
	      batch(*no)
     or
        rundomcmd server(<servername>) cmd(call pgm(qntcsdk/copydb)
	      parm('<input db>' 'output db' 'output title')) batch(*no)
CHECKING THE RESULT
-------------------
To see if copydb worked correctly:

From Notes, add the icon for the input database to your desktop. Open the
database and examine the contents.

Do the same for the output database. 

The icons, forms, views, and data notes of the two databases should be
identical. Only the title should be different.

