Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN
 
(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
-------
dusspi - Example Domino Upgrade Services application

This program illustrates the following Domino Upgrade Service functions:
	
	DUSGetName
	DUSStart
	DUSRetrieveUsers
	DUSGetUserInformation
	DUSExtendedErrorText
	DUSStop
	DUSTerm

This program is invoked from the Domino Adminstrator application therefore
you must have administration access to register users.  Input files 'names.txt'
and 'address.txt' are provided to supply user information for functions
DUSRetrieveUsers and DUSGetUserInformation.

Functions not illustrated in this sample are:
	  ---
	DUSAdvancedDialog
	DUSRetrieveGroups
	DUSGetGroupMembers
	DUSGetGroupInformation
	DUSRegistrationNotify
	DUSConvertMailFile


ENVIRONMENTS
------------
Windows (32 bit)


FILES
-----
dusspi.c     - Main C source code file for Domino Upgrade Services
dusspi.h     - Constants
duspierr.h   - Error definitions
names.txt    - Input file for retrieving users to be migrated
address.txt  - Input file for gathering address information for users to be migrated

mswin32.mak     - Make file for Windows 32-bit.
mswin32.def  - Module definition file for Windows 32-bit.
mswin32.rc   - Resources for Windows 32-bit

readme.txt   - This file specifies what is needed to use this example.


BEFORE BUILDING THIS SAMPLE
---------------------------
Before building this sample edit 'FileName' in the functions DUSRetrieveUsers and
DUSGetUserInformation within dusspi.c.  In DUSRetrieveUsers, 'FileName' should point to
the location of file 'names.txt'.  In DUSGetUserInformation, 'FileName' should point to
the location of file 'address.txt'.


RUNNING THE DOMINO UPGRADE SERVICES DLL
---------------------------------------
To install and run this program:

    1.  Exit Notes if it is running.

    2.  Copy NDUSSPI.DLL to the Notes program directory.

    3.  Edit the notes.ini file.
	If an UpgradeApps entry does not exist, add the line:
	
	UpgradeApps=NDUSSPI.DLL

	Save the modified notes.ini file.

    4.  Launch Domino Administrator

    5.  Select People/Register to bring up the 'Register Person -- New Entry'
	screen.

    6.  Select the button 'Migrate People'

    7.  From the 'Foreign directory source' list box select the DUS 'C API DUS SPI Sample'.
	This will invoke the dus sample dll.

    8.  Users to migrate will appear under the 'Available People/Groups' list. The information is
	gathered from 'names.txt' and includes:

		      Carol Morgan
		      John Doe
		      Nancy Sullivan
		      Patrick Mister
		      Sally Smith
		      Steve Jones
	
    9.  The above names can be added to the 'People/Groups to Migrate' list.

    10. Once the list to migrate is generated, select the 'Migrate' button.
	For the users to be migrated additional information is retrived from
	the input file 'address.txt'.

    11. The users in the 'People/Groups to Migrate' list will be queued up for user registration.
	They will be added to the 'User Registration Queue' database 'userreg.nsf'.  These users
	will be listed on the 'Register Person -- New Entry' screen.  It is recommended to delete
	these users from registration since they are only used for illustration of the Domino
	Upgrade Services sample.  They can either be deleted from the
	'Register Person -- New Entry' screen or they can be manually deleted from the 'userreg.nsf'
	database.
