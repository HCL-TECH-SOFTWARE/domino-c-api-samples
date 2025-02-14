Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN
 
(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
iedit - Example edit level import library


PURPOSE
-------
Demonstrates building a DLL which Notes can use to import data in a
non-Notes format.


ENVIRONMENTS
------------
Windows (32-bit)


FILES
-----
iedit.c      - Main C program source file.
ieditwin.c   - Windows (32-bit) -specific wrapper functions.
iedit.h      - Symbolic constants and function prototypes.
iedit.txt    - This is a short ASCII text file you may import
               into a document using iedit from the Notes UI.

mswin32.def  - Module definition file for Windows 32-bit.
mswin32.mak     - Make file for Windows 32-bit.

readme.txt   - This file specifies what is needed to use this example.


INSTALLING AND RUNNING iedit
----------------------------
To install iedit, copy the executable program library to the Notes program
directory. The actual name of the DLL :

     Windows:         niedit.dll
  
Then edit the notes.ini file and add a new line of the form:

EDITIMPxx=<File Format Name>,0,<Library Name>,<w4w arg>,[<extension>,...]

where xx is a number that starts at 1 and is incremented for each
successive edit level import library.  Make sure to change the "xx"
in "EDITIMPxx" to be one more than that of the last EDITIMP reference.

For example, the line

EDITIMP34=Sample ASCII Import,0,_IEDIT,,.TXT,.PRN,.C,.H,.RC,.DEF,

adds iedit to the list of file import libraries available. The
string "Sample ASCII Import" will appear in the List of File
Types in the File Import Dialog box.

In order for changes to the notes.ini file to take effect, you
must exit and re-start Notes if it is running.

To test iedit, start the Notes workstation. From Notes, open a document
for editing, move the cursor to a rich text field, and issue the File
Import command from the Notes menu. The File Import dialog box appears.
In the File Name list box, select the names of one or more ASCII text
files. In the List of File Types, select "Sample ASCII Import". Click
the Import button. Notes transfers control to the iedit progam library.
Iedit converts the selected ASCII text files to Rich Text format. Notes
inserts the rich text data into the document at the cursor position.
