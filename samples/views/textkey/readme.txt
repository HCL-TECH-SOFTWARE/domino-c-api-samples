Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
-------
textkey  - Find Documents with Text Keys


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM i

FILES
-----
readme.txt   - This file specifies what is needed to use this example.

textkey.c    - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

mk_textkey_i.clp    - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_textkey_p.clp    - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.  

			
RUNNING textkey:
----------------
Copy the sample database, "textkey.nsf", to the Domino Data directory on
your system.  From the Notes user interface, add the database's icon to your
Notes workspace.

- Under Windows: type the following at the system command prompt:

    textkey <database> <view> <string>

    where:
      <database> is the filename of the Domino database to operate on
      <view> is the name of the view to search
      <string> is the text to look for in the view


EXAMPLE:

      TEXTKEY  TEXTKEY PeopleView "Smith, Carl"

If the program runs correctly, it should display:

-------------------------------------------------------------------------------

Note count is 1.         Note ID is: 2112

-------------------------------------------------------------------------------

- Under IBM i:
  run:   rundomcmd server(<server>) cmd(call pgm(qntcsdk/textkey) 
            parm('textkey.nsf' 'PeopleView' 'Smith, Carl' )) batch(*no)       

EXAMINING THE DOMINO DATABASE TO CHECK THE RESULT:
--------------------------------------------------

To see if TEXTKEY worked correctly:

Open TEXTKEY.NSF.

In the menu, for each document that has the name "Smith, Carl", choose
File/Document Properties.Select the Document IDs tab of the document's
properties InfoBox and look at the last line of the ID value.
It should end with one of the NoteIDs that was in the output.
