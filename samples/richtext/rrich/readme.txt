Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN
  
(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
rrich - HCL C API for Domino and Notes Sample Program: Read RichText Fields


PURPOSE
-------
This program shows how to read the text within a rich-text field.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM i


FILES
-----
readme.txt   - This file specifies what is needed to use this example.

rrich.c      - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

mk_rrich_i.clp - Make file (CL Program) for IBM i using ILE C 
                 with the source code being an IFS file.
    
mk_rrich_p.clp - Make file (CL Program) for IBM i using ILE C 
                 with the source code being in a source physical file.     


RUNNING rrich
-------------
The command line of rrich is:

  rrich <database>

where:

  <database> is the filename of the Domino database to operate on


EXAMPLE
-------

  rrich richtext

- Under IBM i type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/rrich)
        parm(richtext.nsf)) batch(*no)
If the program runs correctly, it should display something that looks like:

-----------------------------------------------------------------------------

       ************* Note ID is: 212A. *************

RICH_TEXT field is:

This is some rich text.
This is a centered paragraph
It will be used  to demonstrate  the HCL C API for Domino and Notes
CompoundText Routines.

These lines use font faces that require a font table (a
$Fonts item).
1234 asdfjkl;
(MS Sans Serf, Symbol, and System)

--------------------------------------------------------------------------------


CHECKING THE RESULTS
--------------------
To see if rrich worked correctly, open Rich Text Test database (richtext.nsf).
Open the documents. The text should be the same as displayed on the screen.
It will have different colors, sizes, alignment, and styles, but it will
be the same text as that displayed.
