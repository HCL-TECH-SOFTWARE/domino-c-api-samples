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
hotspot - Example HCL C API for Domino and Notes Program. Shows how to 
create popups, buttons, collapsible sections, action items, URLs and file 
attachments in a rich text field in a note.


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
readme.txt  -  This file specifies what is needed to use this example.

hotspot.c   -  Main program.

cdrecord.c  -  Contains some subroutines used by the main program.

hotspot.h   -  Header file for this program.

mswin32.mak - Makefile for Windows 32-bit.

mswin64.mak - Makefile for Windows 64-bit.

aix.mak          - Make file for IBM AIX.

aix64.mak    - Make file for IBM AIX64.


linux.mak    - Make file for Linux.

linux64.mak    - Makefile for Linux 64 bit.

mk_hotspot_i.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_hotspot_p.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.     

macosx.mak   - Make file for Mac.


RUNNING hotspot:
----------------
First, ensure the variable "szSourceFileName" in the "InsertFile" function in
the file hotspot.c specifies the fully-qualified path to the input file
"readme.txt" (or some other valid file on your disk), which is
supplied with this sample. If "szSourceFileName" does not correctly identify
the location of the file, either copy the file to the location specified
by szSourceFileName, or edit szSourceFileName appropriately and rebuild the
program.

Next, modify the "ToInfo" value in the "InsertAction" function in the file
hotspot.c to be an appropriate e-mail address.

And finally ensure that the database makeform.nsf resides in your Domino
or Notes data directory.


- Under Windows or UNIX (AIX, Linux ),
  at the system prompt type:

        hotspot

  The program takes no arguments.
- Under IBM i type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/hotspot))
        batch(*no)


The program will create a document in the database makeform.nsf. Contained
in the new document's rich text field will be a popup, a button with 
formula, a collapsible section, an action that sends a message, a URL, 
a file attachment and a button with LotusScript.


CHECKING THE RESULT
-------------------
After running the hotspot program, open the database makeform.nsf and 
display the document that was just created.  The "Body" field will 
contain a popup, a collapsible section, a button with formula, an action,
a URL, a file attachment, and a button with LotusScript.

Clicking on the "Click Here" popup will cause the following text to be
displayed:

   "This is the text of the popup"

Clicking on the "Print..." button will cause the formula 
"@Command([FilePrint]) to be executed, causing the "File\Print" 
dialog box to be displayed.

Clicking on the "This is my section title" collapsible section will 
cause the text associated with the section to be displayed. You will 
see "This is text in my section" displayed.

Clicking on the "This is an action that sends a message" action section 
will cause a mail message to be sent to the user specified in the 
hotspot.c source.

Clicking on the URL "http://www.lotus.com" will cause the specific Web site 
to be displayed.

Clicking on the file attachment "readme.txt" will display the Properties 
InfoBox for the file attachment.

Clicking on the "LS Button" button will cause the following text to be
displayed:

   "I love LotusScript."

