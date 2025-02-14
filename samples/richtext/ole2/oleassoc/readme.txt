Licensed Materials - Property of HCL
CIR9ZEN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
OLEASSOC - Sample HCL C API for Domino and Notes Program:
           Associate file with OLE 2 object.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)


FILES
-----
oleassoc.c   - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

readme.txt   - This file specifies what is needed to use this example.


RUNNING oleassoc
---------------
First, ensure the global variable "szFileSpec" in the file oleassoc.c
specifies the fully-qualified path to the input file "readme.txt" (or some
other valid file on your disk), which is supplied with this sample.
If "szFileSpec" does not correctly identify the location of the file,
either copy the file to the location specified by szFileSpec, or edit
szFileSpec appropriately and rebuild the program.

This program shows how to associate a file with an OLE 2 object embedded
in documents in the Domino database "embedole.nsf".

Make sure the Domino database "embedole.nsf" resides in the Domino or
Notes data directory.

Through the Notes User Interface, create a document in this database and
insert an OLE 2 object.  Name the OLE 2 object "OLEobject".
   To name an OLE object in Notes:
      - click on the object in the document to select it.
      - select the object's menu pick, then Object Properties
      - in the infobox, enter OLEobject for Name.
      - close the infobox, close and save the document

OLEASSOC will create $FILE attachment name "Test" from the file
readme.txt in the document and associate it with the embedded OLE 2
object named "OLEobject" in the document in the database embedole.nsf.

- Under Windows
  type:
       oleassoc
  at the system command prompt.


CHECKING THE RESULTS
--------------------
Open embedole.nsf database in Notes.  Open the document.  Select the OLE
object by clicking on it.  In the object's menu pick select:
   - Object properties.  In the infobox select the "folder" panel which
     will list the associated files.
   - Diassociate file. The listbox displayed will show all associated
     files.  Exit dialog by canceling.

If you run OLEASSOC a second time, you will get an error because an
attachment of the name "Test" already exists (from the first running).
You must remove the associated object first, either through the Notes UI
or the test program DISASSOC.  Another solution is to change the name
from "Test" to something else in oleassoc.c.
