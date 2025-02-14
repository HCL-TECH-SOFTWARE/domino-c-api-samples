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
extpwd - Extension manager password dialog.


PURPOSE:
--------
The purpose of this sample is to illustrate the work of the Extension
Manager.  This sample generates a Dynamic Link Library (DLL), or, for the
UNIX platforms, a shared library, which will intercept the Domino request
to obtain a password.

For the Window platforms, the DLL uses an application dialog box
to obtain the password.  The use of a dialog box is specific to this sample,
and is not required by the Extension Manager;  all that is required is
that a string be returned by the DLL.

For the UNIX platform, the sample does not provide Windows User
Interface, and the password is intercepted at the Domino Server console.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM AIX
Linux 64 bit
MAC OS X
IBM i


FILES:
------
extpwd.c     - Extension manager entry point and callback for Windows 32-bit.

extunix.c    - Extension manager entry point and callback for UNIX.

exti5os.c    - Extension manager entry point and callback for IBM i.

extwin.c     - Windows DLL and dialog code.

extwin.rc    - Windows resource script.

extpwd.h     - Header file for sample program.

extenv.h     - Platform-dependent definitions.

extpwdrs.h   - Resource constant declarations.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

mswin32.def  - DLL module definition file for Windows 32-bit.

mswin64.def  - DLL module definition file for Windows 32-bit.

aix.mak          - Make file for IBM AIX.

aix64.mak    - Make file for IBM AIX64.

exports.aix  - specifies exported entry points for AIX.


linux.mak    - Make file for Linux.

linux64.mak    - Makefile for Linux 64 bit.

mk_extpwd_i.clp  - Make file (CL Program) for IBM i using ILE C 
                       with the source code being an IFS file. 
                          
mk_extpwd_p.clp  - Make file (CL Program) for IBM i using ILE C 
                       with the source code being in a source physical file. 

macosx.mak   - Make file for Mac.

readme.txt   - This file specifies what is needed to use this example.



RUNNING extpwd
--------------
- Copy the DLL, or in UNIX, the shared library, to the Domino or the Notes
  Program directory.


- Modify notes.ini:


  This samples generates a platform-specific prefix to the name of the DLL,
  or in UNIX, the shared library.

  The names used are:

        nextpwd.dll             Windows 
        libextpsw.a             IBM AIX
        libextpsw.so            Linux RedHat


  -- For Windows platforms:
	1.Enable password of Domino server's ID file.
	2.Enable local encryption of log.nsf. After enabling local encryption, you need to shutdown server and run ncompact.exe log.nsf command to encrypt the database.
	3.Disable transaction logging if it is enabled.
	4.Copy NEXTPWD.DLL to Domino program directory.
	5.Add the following line to notes.ini
		EXTMSG_ADDINS=EXTPWD
	6.Start Domino server, you will see custom dialog window instead of the password prompt in server console.


  -- For i5OS platforms:

        ADDLNK OBJ('/QSYS.LIB/QNTCSDK.LIB/LIBEXTPWD.SRVPGM')            
		    NEWLNK('/QIBM/USERDATA/LOTUS/NOTES/LIBEXTPWD.SRVPGM')       
			LNKTYPE(*SYMBOLIC) 

        Configure a Domino server with password. Shutdown Domino server, 
        add the following line to server's notes.ini file:

            EXTMGR_ADDINS=LIBEXTPWD

        Create "password.txt" file. Write the Domino server's password 
        into the file. Copy this file to the Domino data directory. 
        

VERIFYING extpwd
----------------
On Windows 32-bit platforms:
   Start Notes client.  The standard Notes password prompting dialog box is
   replaced.  The new Window prompt should have a line says
   "Let's have your password:".
	

On UNIX platforms:
   Start the Domino server.  At the Domino server console, the standard
   password prompting line should be replaced with the following line:
   "Please enter your password:".

On i5OS platforms:
   Start the Domino server. There will be no password prompting line shown.
