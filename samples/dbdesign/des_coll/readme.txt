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
des_coll - Example Program: Read the design collection for the database
           and print the type and title of each design note found.


PURPOSE      
-------
This program demonstrates the use of the design collection. It uses 
class NOTE_CLASS_DESIGN to open the design collection then reads all 
the entries in it.  The design collection is a collection that contains 
entries for every form, view, macro, and replication formula in the 
database. This reads through the design collection and for each design 
note prints the note class (Form, View, etc.) and the title, of each.

NOTE:  This sample does not open the private design collection.  Therefore,
       design notes that are private will not be included in the output.
              

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

des_coll.c   - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

aix.mak          - Make file for IBM AIX.

aix64.mak    - Make file for IBM AIX64.


linux.mak    - Make file for Linux.

linux64.mak    - Makefile for Linux 64 bit.

mk_des_coll_i.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being an IFS file.
    
mk_des_coll_p.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being in a source physical file.    

macosx.mak   - Make file for Mac. 


RUNNING des_coll:
-----------------

- Under Windows, or UNIX (AIX, Linux),
  type the following at the system command prompt:

  des_coll <database>

           or

  des_coll

where:

  <database> is the full pathname of a Domino database.
  If no command line argument is provided, the program will prompt for it.
- Under IBM i type:
    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/des_coll))
        batch(*no)
    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/des_coll)
		parm(des_coll.nsf)) batch(*no)


EXAMPLE
-------

  des_coll des_coll


If the program runs correctly, it should display:

Printing design collection for 'Design Collection Test'
Found 5 design notes in 'Design Collection Test'.
        Design Note 1 : Class = Field   Title = 'ColorField'
        Design Note 2 : Class = replication formula     Title = 'Martin Cox'
        Design Note 3 : Class = Form    Title = 'SimpleDataForm'
        Design Note 4 : Class = View    Title = 'SimpleDataView'
        Design Note 5 : Class = Macro   Title = 'Status Update'
