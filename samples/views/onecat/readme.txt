Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
onecat  - Display noteIDs of documents in a category.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM i

FILES
-----
readme.txt   - This file specifies what is needed to use this example.

onecat.c     - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

mk_onecat_i.clp    - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_onecat_p.clp    - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.   


RUNNING onecat:
---------------
Copy the sample database, "onecat.nsf", to the Domino data directory on
your system.  From the Notes user interface, add the database's icon to your
Notes workspace.

- Under Windows: type the following at the system command prompt:

    onecat <db filename> <view name> <category text>

    where:
      <db filename>   is the filename of the Domino database to operate on
      <view name>     is the view to use
      <category text> is the specific category to select


COMMAND LINE EXAMPLE:

      onecat onecat.nsf SimpleCategory "Category 1"
- Under IBM i:
  run:  rundomcmd server(<server>) cmd(call pgm(qntcsdk/onecat) 
        parm('onecat.nsf' 'SimpleCategory' 'Category 1' )) batch(*no)    

CHECKING THE RESULT
-------------------

If the program runs correctly, it should display something that looks like:
-----------------------------------------------------------------------------

Main topic count is: 1.         Note ID is: 20FA.
Main topic count is: 2.         Note ID is: 2106.
Main topic count is: 3.         Note ID is: 210A.
Main topic count is: 4.         Note ID is: 210E.

-----------------------------------------------------------------------------

To see if onecat worked correctly:

Open onecat.nsf.

In the menu, for each document in the category "Category 1", choose
File/Document Properties... item, select the Info tab of the
document's properties InfoBox and look at the 3rd line of the ID value.
It should end with one of the NoteIDs that were in the output.
