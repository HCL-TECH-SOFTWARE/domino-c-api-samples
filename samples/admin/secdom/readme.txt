Licensed Materials - Property of HCL
CIR9ZEN
CIRA0EN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
-------
secdom - DSAPI authentication handler.


PURPOSE:
--------
This Dynamic Link Library (DLL) demonstrates how to, from the web,
authenticate a Domino user through his Operating System user account
via DSAPI.


*NOTE:
------
Although set-up and execution instructions are outlined in this readme.txt
file, knowledge of Domino system administration and Operating System
specific system administration is required to effectively run this sample.


ENVIRONMENTS
------------
Windows (64-bit)
Linux RedHat 64 bit
IBM AIX
IBM i


FILES:
------
secdom.c     - main program.

w_secdom.c   - Windows specific code.

u_secdom.c   - UNIX specific code

mswin32.mak  - Make file for Windows 32-bit

mswin64.mak  - Make file for Windows 64-bit

mswin32.def  - DLL module definition file for Windows 32-bit.

mswin64.def  - DLL module definition file for Windows 64-bit.

aix.mak          - Make file for IBM AIX.

aix64.mak    - Make file for IBM AIX64.

exports.aix  - specifies exported entry points for AIX.


readme.txt   - This file specifies what is needed to use this example.

mk_secdom_i.clp    - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_secdom_p.clp   - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.

linux.mak      - Make file for Linux RedHat.

linux64.mak    - Makefile for Linux ReadHat 64 bit.				   


SETTING UP THE ENVIRONMENT
--------------------------
1. This DSAPI filter must be registered with the Domino server.  See the
   steps in the REGISTERING DSAPI FILTER WITH DOMINO section below.

2. The person used to test this program must have a Domino user account
   on this Domino server and an Operating System user account on the
   machine the server is running.
   To create a Domino user account for an existing Operating System
   user account see the CREATING DOMINO USER ACCOUNT FOR EXISTING OS
   USER section below.

3. To test this program, we will try to open a Domino server database
   from the web.  Be sure the ACL of this database is correctly set.
   For convenience, give Reader access to default, No Access to Anonymous.

- Under IBM i:
		Add the Symbolic Link to the file:
			ADDLNK OBJ('/QSYS.LIB/qntcsdk.LIB/LIBSECDOM.SRVPGM')            
				NEWLNK('/QIBM/USERDATA/LOTUS/NOTES/LIBSECDOM.SRVPGM')       
				LNKTYPE(*SYMBOLIC) 

REGISTERING DSAPI FILTER WITH DOMINO
------------------------------------
1. Compile the sample program and copy the DLL to the Domino server's
   program directory.
2. Start Domino server.
3. From Notes UI, open the Directory database of the HCL Domino server
   (the names.nsf database).
4. From the Server - Servers view, open this server's server document.
5. Under the Internet Protocols tab, enter the name of the DLL in
   the DSAPI filter file names field.
6. Save the document.


CREATING DOMINO USER ACCOUNT FOR EXISTING OS USER
-------------------------------------------------
1. Start Domino server.
2. Start Domino Administrator.
3. Be sure the Server: field points to the server, not Local.
   If it points to Local, use File - Open Server to change it to
   point to the Domino server.
4. Highlight the People view at the left panel.
5. From the People pull down menu at the right panel, click the
   Register... action.
6. Fill in the First name, Last name, Short name, and Password.
   Be sure that Short name is identical to the OS user account name.
7. Click the Register button to create the Domino user.
8. Close Domino Administrator.
9. Refresh Domino server or take it down.


RUNNING secdom
--------------
*Note (Windows):
To authenticate with the DSAPI filter on Windows, the OS user must have
the Windows "Act as part of the operating system" user security policy
enabled.  If If domain-level policies are defined, they must also grant
this right to the user.

If the Windows user does not have this security policy set, authentication
will not occur, and an error will be displayed at the server console.

To set the Windows "user security policy" for the particular user, consult
your Windows documentation or contact your Windows System Administrator.

1. Bring up the HCL Domino Server and be sure the http server task is
   running.  From the server console, you should see the following
   message:

   DSAPI Operating System Authentication Filter Loaded successfully.

2. From either this Domino server machine, or another machine, start
   the web browser.

3. Enter <Domino-server>/<Domino-server-database> URL to open the
   database from web, where

         <Domino-server> is the name of the Domino server (the IP address
	      may also be used.)
         <Domino-server-database> is the name of the database mentioned
              in the step 3 of the SETTING UP THE ENVIRONMENT section.

         for example: dserver/dsdatabase.nsf


4. From the Enter Network Password screen, enter the OS user account
   name and OS user account password.

   If the Domino server is running on a Windows NT/2000 machine, you
   should enter user name in the following fashion:

         <operating-sytem-user-name>@<operating-system-domain>

         for example: jdoe@os_domain
	
   If the Domino server is running on a Unix machine, you
   should enter user name in the following fashion:

         <operating-sytem-user-name>
	
         for example: jdoe

- Under IBM i
	If the user has been registered in the Domino, password would be any.

5. Click OK button to get authenticated.

6. You should see the content of the database.

