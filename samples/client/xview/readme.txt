Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
xview -  Sample view-level export library.


ENVIRONMENTS
------------
Windows (32-bit)


FILES
-----
readme.txt   - This file specifies what is needed to use this example.

xview.c      - Program Library source file.
xview.h      - Program definitions.

mswin32.def  - Module definition file for Windows (32-bit).
mswin32.mak     - Make file for Windows 32-bit.
mswin32.rc  - Resource compiler file for Windows (32-bit).


INSTALLING AND RUNNING xview
----------------------------
To install xview, copy the executable program library to the Notes program
directory. The actual name of the DLL is:

        Windows:         nxview.dll

Then edit the notes.ini file and add the following:


        VIEWEXPxx=Example View Export,1,_XVIEW,,.TXT,

where xx is a number that starts at 1 and is incremented for each
successive view level export library.  Make sure to change the "xx"
in "VIEWEXPxx" to be one more than that of the last VIEWEXP reference.

For example, if your notes.ini file already contains a variable named
VIEWEXP5, then use VIEWEXP6 or VIEWEXP7, etc, as necessary to get a unique
variable name.

Restart Notes.  XVIEW is executed from the Notes user interface.

You may test XVIEW against any Domino database, but you must
know the item names of two TYPE_TEXT items in the documents
that appear in the view.  In order for XVIEW to find items to
export, you must specify these item names correctly in the
fields of the dialog box titled "XVIEW Sample Export".
You may discover the item names and types of notes in a database
via the menu commands File - Document Properties.

From the Notes user interface, open a database of your choice.
In the view, select one or more documents by clicking in the left
column.  This puts a check mark in the column (any documents that
contain an item of TYPE_TEXT may be exported).  Select File - Export.
"Example View Export" will appear as one of the File Types in the
"Save File as Type" scroll box. Specify a filename to export to.
Press "Export".  Notes calls XVIEW automatically.

XVIEW displays a dialog box prompting for some static text
and the names of two TYPE_TEXT items in the documents.  The static
text may be a line of dashes or asterisks that provide a visual
separation between text.  The item names must be valid item names
available in the documents you are exporting.  Then click "OK".
XVIEW creates an ASCII output file that contains, for each note
selected in the view, the static text followed by the TYPE_TEXT
items specified.
