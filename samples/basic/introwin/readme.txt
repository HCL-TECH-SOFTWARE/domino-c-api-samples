Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
-------
introwin - Displays the database title.


PURPOSE
-------
This program is an introductory HCL C API for Domino and Notes program
that opens a Domino database and displays the title.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)


FILES
-----
introwin.c   - Main program.
introwin.h   - Local function prototypes and data definitions.
introwin.def - Windows definition file for this example.
introwin.rc  - Resource compiler file for this example.
mswin32.mak     - Make file for Windows 32-bit.
mswin64.mak     - Make file for Windows 64-bit.
readme.txt   - This file specifies what is needed to use this example.


IDENTIFY A TEST DATABASE:
-------------------------

Determine the name of a Domino database file, such as intro.nsf, that
resides in the Domino or Notes data directory on your system. INTROWIN
will work with any valid Domino database file.


RUNNING introwin:
-----------------

To run this program from Windows 32-bit, type, introwin, at the
command prompt in the program's build directory.  The INTROWIN main window
appears.  Choose Get Database Title from INTROWIN's File menu.  The dialog
box displayed will prompt for the filename of a database. Enter the
filename of any Domino database in the workstation's local data
directory, such as, intro.nsf.  INTROWIN will open the specified database,
read the database information, and display the database title in a message
box.

