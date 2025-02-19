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
allflds - Finds all the fields in all the documents in a database.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM AIX 
Linux (64-bit) 
MAC OS X
IBM i


FILES
-----
allflds.c    - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.


aix.mak          - Make file for IBM AIX.

aix64.mak        - Make file for IBM AIX64.        

linux64.mak    - Makefile for Linux  64 bit.

zlinux64.mak   - Make file for zlinux64.

mk_allflds_i.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_allflds_p.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.   

macosx.mak   - Make file for Mac.  

readme.txt   - This file specifies what is needed to use this example.


RUNNING allflds
---------------
This sample program will run against the Domino database simple.nsf,
provided with the HCL C API for Domino and Notes toolkit.  You must copy 
this file to the Domino or Notes data directory before running this program.


- Under Windows, or UNIX (AIX and Linux),
  type:
       allflds
  at the system command prompt.
- Under IBM i type:
    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/allflds))
        batch(*no)

The program should print output that looks like this:

Note ID is: 8F6.                                                 
Found field named "Form" with data type "Text".                   
Found field named "plain_text" with data type "Text".             
Found field named "number" with data type "Number".               
Found field named "time_date" with data type "Time/Date".         
Found field named "text_list" with data type "Multi-Value Text".  
                                                                 
Note ID is: 8FA.                                                 
Found field named "Form" with data type "Text".                   
Found field named "plain_text" with data type "Text".             
Found field named "number" with data type "Number".               
Found field named "time_date" with data type "Time/Date".         
Found field named "text_list" with data type "Multi-Value Text".  
                                                                 
Note ID is: 8FE.                                                 
Found field named "Form" with data type "Text".                   
Found field named "plain_text" with data type "Text".             
Found field named "number" with data type "Number".               
Found field named "time_date" with data type "Time/Date".         
Found field named "text_list" with data type "Multi-Value Text".  

