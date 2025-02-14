Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN


(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
ldap_msc - Using the LDAP functionality of the C API Toolkit for
           Notes/Domino, this program will:
				
              - Add an entry to an LDAP directory.
              - Modify the Relative Distinguished Name (RDN) of the new
                entry (deleting an identical entry if one exists.)
              - Perform a search of the directory based on a filter.
              - Perform a comparison of attribute values.
				
           NOTE**
              A THOROUGH understanding of the open industry standard LDAP, as
              well as experience with result messages returned from LDAP, is
              required prior to successfully running this sample.  Please
              refer to the Domino Administration Guide for information
              regarding the configuration and implementation of accessing an
              LDAP directory with Notes Users and Domino servers.				


ENVIRONMENTS
------------
Windows 32-bit
Windows 64-bit
Linux 64-bit


FILES
-----
ldap_msc.c   - Main program.

ldap_msc.h   - Header file for this program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

linux64.mak    - Makefile for Linux 64 bit.

readme.txt   - This file specifies what is needed to use this example.


BEFORE RUNNING THIS SAMPLE:
---------------------------
1. Initial configuration of Domino is required if this sample is to be
   run against a server with the LDAP task enabled. Please refer to the
   Domino Administration Guide for information regarding the configuration
   and implementation of accessing an LDAP directory with Notes Users
   and Domino servers.

RUNNING ldap_msc:
-----------------
- Before compiling the program, edit the definitions: "HOST", "DN" and
  "PASSWORD", as well as all references to the definition for the
  organizational attribute "O" to ensure they are all valid LDAP values
  as defined by your individual LDAP configuration.

  It is recommended to initially retain the values for the definitions:
  "Filter", "MOD_DN", "MOD_NDN" and "MOD_NRDN" to facilitate the ease of
  sample execution and clean up.

  These definitions are all contained in the file 'ldap_msc.h'.


OPTIONAL:
- If you wish to have LDAP 'debug' information printed to your output
  screen during program execution, add the appropriate parameters
  (i.e "DEBUG_OUTFILE=[directory path]" and "LDAPDEBUG=1") to the
  notes.ini file and uncomment the appropriate lines as described in the
  file 'ldap_msc.c'.

- After the program is successfully compiled, at the system prompt type:

    ldap_msc


If the program runs correctly, it should display output similar to
the following:

  Modifcation results:

	Added entry "CN=John Public, O=API".
	Entry "CN=John Q Public, O=API" is not in the directory.
	  No need to delete.
	The RDN modification operation was successful.
 	Entry...
	  "CN=John Public, O=API"
	Has been changed to...
	  "CN=John Q Public, O=API"

  Search results:

	dn: CN=John Q Public,O=API
	cn: John Q Public
	cn: John Public
	mail: John_Q_Public/API@lotus.com
	objectclass: dominoPerson
	objectclass: inetOrgPerson
	objectclass: organizationalPerson
	objectclass: person
	objectclass: top
	telephonenumber: +1 888 555 1212
	mailsystem: 1
	messagestorage: 1
	encryptincomingmail: 0
	checkpassword: 0
	passwordchangeinterval: 0
	passwordgraceperiod: 0
	availablefordirsync: 1
	roamcleansetting: 0
	roamcleanper: 1
	givenname: John
	sn: Public
	roaminguser: 0

  Comparison results:

	The value "person" is contained in the objectclass attribute.
	The value "xyzzy" is not contained in the objectclass attribute.

Program completed successfully.


EXAMINING THE LDAP directory:
--------------------------------------------------

To see if 'ldap_msc' worked correctly:

    Check the LDAP directory for the modified entry as defined in the program.
