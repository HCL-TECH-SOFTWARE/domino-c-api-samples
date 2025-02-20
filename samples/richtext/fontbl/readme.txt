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
fontbl  - Create a Font Table used by rich text.


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
fontbl.c      - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.


aix.mak          - Make file for IBM AIX.

aix64.mak        - Make file for IBM AIX64.

mk_fontbl_i.clp  - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_fontbl_p.clp  - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.     

linux.mak     - Make file for Linux.

linux64.mak    - Makefile for Linux 64 bit.

macosx.mak   - Make file for Mac.

readme.txt    - This file specifies what is needed to use this example.


RUNNING fontbl
--------------
- First copy the database richtext.nsf located in the Notedata directory
  to the Domino or Notes data directory.

- Under Windows, or UNIX (AIX, Linux ),
  type the following at the system command prompt:

        fontbl <database filename>
            or
        fontbl

where:

   <database filename>  is the filename of the Domino database.
   If no command line argument is provided, the program will prompt
   for it.

EXAMPLE
-------

        fontbl  richtext

- Under IBM i, type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/fontbl)
    parm(richtext.nsf) batch(*no)
   

CHECKING THE RESULTS
--------------------
FONTBL  will create a document in the database richtext.nsf.  The
document's RICH_TEXT field will contain three lines of richtext, 
each line in a different font.

After running the FONTBL program, open the database and display the
document that was just created.  The RICH_TEXT field will contain the
strings: 

"This is an example of System Monospaced font."
"This is an example of System Proportional font."
"This is an example of TYPEWRITER font."

The first line will be displayed in blue System Monospaced font.
The second line will be displayed in magenta System Proportional font.
The third line will be displayed in black TYPEWRITER (Courier) font.

Note that the TYPEWRITER font is not defined in the font table.  It is
one of the three hard coded fonts supported.                 
