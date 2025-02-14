Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN
  
(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
wrich    - Example HCL C API for Domino and Notes Program:
           Create a note and write text to a RichText field.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM i


PURPOSE
-------
This program shows how to create a rich text field by dynamically
allocating a buffer, filling the buffer with CD records converted to
Domino and Notes canonical format, and appending the buffer to a note as a
rich-text field.


FILES
-----
readme.txt   - This file specifies what is needed to use this example.

wrich.c      - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

mk_wrich_i.clp - Make file (CL Program) for IBM i using ILE C 
                 with the source code being an IFS file.
    
mk_wrich_p.clp - Make file (CL Program) for IBM i using ILE C 
                 with the source code being in a source physical file.     


RUNNING wrich
-------------
Type:

        wrich <database>

where:

<db filename> is the filename of the Domino database to operate on.  It is
recommended to use the richtext.nsf database file supplied with the toolkit.

EXAMPLE:

      wrich richtext
- Under IBM i type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/wrich)
        parm(richtext.nsf)) batch(*no)

When the program runs correctly it does not display anything.



EXAMINING THE NOTES DATABASE TO CHECK THE RESULT:
------------------------------------------------

To see if wrich worked correctly:

Open the target database using Notes.

Double-click on the newly created document in order to read it.

The text should contain a left-justified and center-justified paragraph
containing the following text (and font details):

  "This is a left justified"(1) " paragraph"(2)
          "This is a centered paragraph"(3)

(1) in bold, blue, 24pt Swiss
(2) in italic, red, 18pt Roman
(3) in bold italic, magenta, 12pt Typewriter
