Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
uiaddin2  -  Sample Notes Client Menu Add-in Program


ENVIRONMENT
-----------
Windows (32-bit)


PURPOSE
-------
This program shows how to implement a Notes menu Add-In module that
writes a text value into the field of an open document.


FILES
-----
readme.txt   -  This file specifies what is needed to use this example.

uiaddin2.c   -  Main program.

mswin32.def  -  Module definition file for Windows 32-bit.

mswin32.mak  -  Makefile for Windows 32-bit.


RUNNING uiaddin2
----------------
This sample program uses the Domino database, uiaddin2.nsf, provided
with the HCL C API for Domino and Notes toolkit.  Make sure this database
file is copied into the Notes data directory before running this program.

From the Notes user interface, add the database's icon to your Notes
workspace using the File/Database Open... menu item.

Exit Notes. Edit your notes.ini file and specify the add-in DLL
as one of the values of the AddInMenus variable:

AddInMenus=UIADDIN2.DLL

Copy UIADDIN2.DLL to a directory in your path. Alternatively, the full
pathname of the DLL may be specified in the notes.ini file.

The AddInMenus variable in the notes.ini file may have multiple
values:

AddInMenus=UIADDIN1.DLL, UIADDIN2.DLL

Start the Notes client and pull down the Actions menu.  The menu items
added by the DLLs assigned to the AddInMenus variable of the notes.ini file
will be added to the end of the Actions menu.  This sample adds the
following menu items:  Display Date/Time, Insert Date/Time, Insert
Company Name, and Insert Location.  The menu item, Insert Date/Time, is
enabled only when editing a field of type TIME in a document.  Likewise,
the menu item, Insert Company Name, is enabled only when editing a field
called CompanyName and the menu item, Insert Location, is enabled only
when editing a field of type text.


EXAMPLE
-------
Start up Notes.  Pull down the Actions menu and choose Display Date/Time.
The system date and time is displayed in a message box.

Open up the Domino database and create a new document.  Move the cursor to
one of the fields.  Pull down the Actions menu and depending upon the field
that the cursor is on, choose one of the following menu items:
Insert Date/Time, Insert Company Name, or Insert Location.  The
information is inserted into your document.

