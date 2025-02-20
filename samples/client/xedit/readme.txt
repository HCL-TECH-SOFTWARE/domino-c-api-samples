Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.

PROGRAM
-------
xedit - Sample edit-level export library.


ENVIRONMENTS
------------
Windows (32-bit)


FILES
-----
readme.txt   - This file specifies what is needed to use this example.

xedit.c      - Program Library source file.
xedit.h      - Program definitions.

mswin32.def  - Module definition file for Windows 32-bit.
mswin32.mak - Make file for Windows 32-bit
mswin32.rc  - Resource compiler file for Windows 32-bit.


INSTALLING AND RUNNING xedit
----------------------------
To install xedit, copy the executable program library to the Notes program
directory. The actual name of the DLL depends on the platform:

  Windows:  nxedit.dll

Modify your notes.ini file by adding a line for this edit-level export DLL.
This reference takes the form

  EDITEXPxx=<parameter list>,

where xx is a number that starts at 1 and is incremented for each
successive DLL reference.  The reference that must be added for
_XEDIT.DLL appears below.  The title string "Sample ASCII Export" will
appear in the export File Dialog box as the label for this export DLL.

NOTE: Make sure to change the "xx" in "EDITEXPxx" to be one more than
that of the last EDITEXP reference.

[Notes]
;
; Add this to your Notes.ini.
;
;             Edit/Form Level export Settings
;-------------------------------------------------------------------------
;        |            |0=buff 2=file|   |   |
;        |            |_______ _____|   |w4w|
;ListLoc |    Title          | |DLLname |arg|   Default Extensions
;--------|-------------------|-|--------|---|-----------------------------

EDITEXPxx=Sample ASCII Export,2,_XEDIT,     ,.TXT,.INI,
;-------------------------------------------------------------------------
;      ^ change "xx" to the appropriate number

Restart Notes so the changes to the notes.ini file take effect.  Open a
database and open a document to be exported.  Then, from the Notes menu,
choose File, then choose Export to display the File export dialog box.
Choose the subdirectory to which the file should be exported, type in
the filename under which to store the file, and choose "Sample ASCII
export" from the listbox on the right.  The note will be exported to a
file in the specified name and subdirectory, using the _XEDIT export DLL.
