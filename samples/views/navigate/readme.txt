Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM:
-------
navigate - Navigate a Two-Level Category View

PURPOSE    This program shows how to navigate within a collection.
-------    The program navigates to the 2nd major category, then
	   into the 2nd subcategory within this category. The program then
	   displays the noteIDs of the main documents.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM i

FILES
-----
readme.txt   - This file specifies what is needed to use this example.

navigate.c   - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

mk_navigate_i.clp    - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_navigate_p.clp    - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file. 

RUNNING navigate:
-----------------
Copy the sample database, "hierarch.nsf", to the Domino Data directory on
your system.  From the Notes user interface, add the database's icon to your
Notes workspace.

- Under Windows: type the following at the system command prompt:

    navigate <database> <view>

    where:
      <database> is the filename of the Domino database to operate on
      <view> is the name of the view to navigate


EXAMPLE:

      NAVIGATE  hierarch twolevels

If the program runs correctly, it should display

-------------------------------------------------------------------------------

Note ID number 1 is: 211A
Note ID number 2 is: 211E
Note ID number 3 is: 2122

-------------------------------------------------------------------------------

- Under IBM i:
  run:  rundomcmd server(<server>) cmd(call pgm(qntcsdk/navigate) 
          parm('hierarch' 'twolevels' )) batch(*no)  

EXAMINING THE DOMINO DATABASE TO CHECK THE RESULT:
--------------------------------------------------

To see if NAVIGATE worked correctly:

Open HIERARCH.NSF.

Go to the documents in Main B, sequence 2. For each one, choose
File/Document Properties... item, select the Document IDs tab
(sixth tab from the left) of the document's properties InfoBox and
look at the last line of the ID value.  It should end with one
of the NoteIDs that were in the output.
