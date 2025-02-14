Licensed Materials - Property of HCL
CIR9ZEN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
DISASSOC - Sample HCL C API for Domino and Notes Program:
           Disassociate file from OLE 2 object.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)


FILES
-----
disassoc.c   - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

readme.txt   - This file specifies what is needed to use this example.


RUNNING disassoc
----------------
This program shows how to disassociate a file from an OLE 2 object embedded
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
Associate a file with the OLE 2 object.  Name the associated file "Test".
	To associate a file with the OLE object in Notes:
		- click on the object in the document to select it.
		- select the object's menu pick, then Associate file to
                  object
		- in the dialog box, enter the file specification to a file
		- enter the attachment name "Test"
		- close the dialog box, close and save the document.
	The Notes API test program OLEASSOC can also be used to associated
        a file.

DISASSOC will remove a $FILE attachment name "Test" from the embedded OLE 2
object named "OLEobject" in the document in the database embedole.nsf.

- Under Windows
  type:
       disassoc
  at the system command prompt.


CHECKING THE RESULTS
--------------------
Open embedole.nsf database in Notes.  Open the document.  Select the OLE
object by clicking on it.  In the object's menu pick select:
	- Object properties.  In the infobox select the "folder" panel
          which will list the associated files.  Test should not be there.

If you run disassoc a second time, you will get an error because an
attachment of the name "Test" was removed (from the first running).
