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
userreg  - Example HCL C API for Domino and Notes Program.
	   Demonstrates user registration functions.


PURPOSE
-------
This program illustrates how to use the User Registration routines to
register a new hierarchical organizational certifier, organizational unit
certifier, server, workstation, and recertify a User ID.

First, a new Organization certifier, ABCorp, is created. The password in
the certifier ID is set to "abcorp". Then, a new Organization Unit certifier,
Sales, is created. This certifier is a descendent of the Organization
certifier. Then, a new server, OTTO, is registered and certified with the
Organization certifier.  A new user, Jayne Doe is registered and
certified with the Organization Unit certifier and then is recertified with
the Organization Unit certifier.

The entries for the Organization certifier, Organization Unit certifier,
new server, and new user are then looked up in the Domino Directory (Server's
Address book) and their Note IDs are printed out.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM AIX
IBM i
Linux 64 bit
MAC OS X



FILES
-----
readme.txt     - This file Specifies what is needed to use this example.

userreg.c      - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.


aix.mak          - Make file for IBM AIX.

aix64.mak        - Make file for IBM AIX64.

mk_userreg_i.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_userreg_p.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.     

linux64.mak    - Makefile for Linux 64 bit.

macosx.mak   - Make file for Mac.


RUNNING userreg
---------------

- Under UNIX or Windows, type the following at the system command prompt:

	userreg  <Full server name>

  where:

      <Full server name> is the name of the HCL Domino Server with which
      you would like to register your new hierarchical organizational
      certifier, organizational unit certifier, server, and a workstation.

      if an empty string " " server name is supplied the program assumes the HCL Domino
      Server is a local server.

- Under IBM i:

      rundomcmd server(<server name>) cmd(call pgm(qntcsdk/userreg))
                batch(*yes) alwmltthd(*yes)
This program creates four ID files.

If you are running this program under Unix, the ID files are written
to /tmp.

If you are running this program under IBM i, the files are written
to /home/qnotes.

For each ID created, this also creates new entries in the Domino
Directory of the server specified on the command line. Therefore,
you must have author access to the Domino Directory on the server
specified on the command line in order for this program to succeed.
When the new user, Jane Doe is registered, a mail file is created.

Status messages will be printed to the screen during registration.

This program opens the Domino Directory on the server, locates
the newly created entries, and prints the note IDs of these new
entries.


Cleaning Up After userreg
-------------------------
After this program ends, follow these steps to clean up:

  1. Open the Domino Directory on the HCL Domino Server specified on
     the command line (or local server name).

  2. Switch to the view, "Server\Certificates".

  3. Delete the entries "Sales/ABCorp/US" and "ABCorp/US".

  4. Switch to the view, "Server/Servers".

  5. Delete the entry, "OTTO/ABCorp/US".

  6. Switch to the view, "People".

  7. Delete the entry, "Doe, Jayne".

  8. Close the Domino Directory.

  9. Delete the file "jdoe.nsf" in the "mail" subdirectory of the Notes
     data directory on the server.

 10. Delete the ID files abcorp.id, absales.id, otto.id, and jdoe.id.
