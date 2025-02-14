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
rsimple - Read & display database fields.


PURPOSE:      
-------       
This program is an introductory HCL C API for Domino and Notes program 
that opens a Domino database and searches for all the data notes. It opens 
and reads each data note, and displays the fields and their names.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM AIX
Linux  64 bit
MAC OS X
IBM i


FILES
-----
rsimple.c    - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

aix.mak          - Make file for IBM AIX.

aix64.mak    - Make file for IBM AIX64.

linux64.mak    - Makefile for Linux 64 bit.

mk_rsimple_i.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_rsimple_p.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.     

macosx.mak   - Make file for Mac.

readme.txt   - This file specifies what is needed to use this example.


RUNNING rsimple:
----------------
This sample program is designed to run against the Domino database, 
simple.nsf, provided with the HCL C API for Domino and Notes toolkit.  
You must copy this file to the Domino or Notes data directory before
running this program.


- Under Windows, or UNIX (AIX and Linux)
  type the following at the system command prompt:
  
        rsimple simple.nsf
               or
        rsimple



EXAMPLE:

      rsimple  simple.nsf
- Under IBM i type:
    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/rsimple)
        parm(simple.nsf)) batch(*no)
  or
    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/rsimple))
        batch(*no)

If the program runs correctly, it should display something similar to the
following:

----------------------------------------------------------------------------

Note ID is: 8FE.                                                     
PLAIN_TEXT field is: The quick brown fox jumped over the lazy dogs.   
NUMBER field is: 125.007000                                              
TIME_DATE field is: 07/14/93 03:14:44 PM                              
The last entry in TEXT_LIST is: Chuck                                 
                                                                     
Note ID is: 8F6.                                                     
PLAIN_TEXT field is: This is a line of simple text.                   
NUMBER field is: 0.000987                                             
TIME_DATE field is: 01/15/88                                          
The last entry in TEXT_LIST is: four                                  
                                                                     
Note ID is: 8FA.                                                     
PLAIN_TEXT field is: This is another line of simple text.             
NUMBER field is: 836.000000                                                  
TIME_DATE field is: 07/04/53 07:12:00 AM                              
The last entry in TEXT_LIST is: d                                     

-------------------------------------------------------------------------------


CHECKING THE RESULTS
--------------------

To see if rsimple worked correctly:

Open simple.nsf.

Double click on the documents to open them and check to see if the values in
the respective fields agree with what Notes displays.  Then, to see if the
names of the fields displayed agree with Notes, use File/Document Properties
to see the names of the fields.

