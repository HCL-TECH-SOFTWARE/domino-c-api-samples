Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

 
(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
-------
servlist -  Sample HCL C API for Domino and Notes Program:
            Get a list of database servers.


PURPOSE
-------
This program demonstrates how to call NSGetServerList() to obtain a list
of all HCL Domino Servers reachable via all available LAN ports.


ENVIRONMENTS
------------
Windows (32-bit)

Note: Currently not support 64-bit UI application on Windows 64-bit

FILES
-----
readme.txt   - This file specifies what is needed to use this example.

servlist.c   - Main program.
servlist.h   - Program definitions.

mswin32.def  - Module definition file for Windows 32-bit.
mswin32.mak     - Make file for Windows 32-bit.
mswin32.rc  - Resource compiler file for Windows 32-bit.

RUNNING servlist
----------------
- Under Windows type in the full path and filename of this program.

SERVLIST takes no parameters.  SERVLIST will display a listbox containing the
names of all available database servers listed in the user's home/mail server's
Domino Directory, known on all of the workstation's LAN ports.


CHECKING THE RESULTS
--------------------
To show that SERVLIST returned a valid list of servers, run Notes and
SERVLIST at the same time, sizing each window so that both are visible.
In Notes, choose File/Database/Open and select "Other..." from the Server
list, and compare the list of servers displayed there with the list
displayed by SERVLIST -- the list displayed by SERVLIST should contain
the servers in the File/Database/Open list.  Note that there may be
more servers listed in the Notes client than those listed by SERVLIST.
Also, it is possible that changes in network availability during the
time lag between displaying the first list and displaying the second one
may occur.

