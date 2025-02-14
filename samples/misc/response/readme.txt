Licensed Materials - Property of HCL
CIR9ZEN
CIRA0EN
CIRA2EN
CIRA1EN
  
(c) Copyright IBM Corporation   1996,   2018
(c) Copyright HCL Technologies   2018,   2024
All rights reserved.


PROGRAM
---------------
response

PURPOSE
-------------
Create a main document and two response documents in a database,
read the response documents from the database, and check the usage
of NIFUpdateFilters.

ENVIRONMENTS
------------------------
Windows (32-bit)
Windows (64-bit)
IBM AIX
IBM i
Linux (64-bit)
MAC OS X


FILES
--------
readme.txt      -  This file specifies how to use this example.

response.c      -  The main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

aix.mak          - Make file for IBM AIX.

aix64.mak        - Make file for IBM AIX64.


mk_response_i.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being an IFS file.
    
mk_response_p.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being in a source physical file.     

linux64.mak    - Makefile for Linux 64 bit.

macosx.mak   - Make file for Mac.

RUNNING response:
--------------------------
Copy the sample database, "response.nsf", to the Domino or the Notes
data directory on your system.  From the Notes user interface, add 
the database's icon to your Notes workspace.

Note:
-----
Before running this sample, make sure that the variable "UserName" in the file response.c
is changed to the Domino Administrator user name. The first time you run the sample,
make sure that response.nsf is open in the Notes client.

- Under Windows or UNIX (AIX, Linux ),
  type the following at the system command prompt:
 
    response <database filename> <subject text>
	     or
    response
 
    where:
      <database filename> is the filename of the Domino database.
      <subject text> is the subject text string.  

    If no command line arguments are provided, the program will prompt for
    them.

    If entering arguments at the prompts instead of at the command line, do
    not enclose them in quotation marks.


EXAMPLE
--------------
Under Windows or UNIX:

    response response "How do you do?"

- Under IBM i, type:

	rundomcmd server(<server name>) cmd(call pgm(qntcsdk/response)
	parm(response.nsf 'How are you?')) batch(*no)

SAMPLE OUTPUT
-------------------------

Testing NIFUpdateFilter:
------------------------
Total unread documents in the database: 3.
Deleted an entry in the id table.
Total unread documents after using NIFUpdateFilters: 2.

NIFUpdateFilters was successful.

Response documents:
--------------
How do you do?    
A response to: How do you do?        
Another response to: A response to: How do you do?        

Program completed successfully.

