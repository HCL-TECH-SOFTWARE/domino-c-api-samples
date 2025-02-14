Licensed Materials - Property of HCL

(c) Copyright HCL Technologies 2024
All rights reserved.


PROGRAM
-------
ldap_ssl - Using the LDAP with SSL functionality of the C API Toolkit for
              Notes/Domino, this program will:
				
              - Create an LDAP with SSL handle.
              - Authenticate with the user.
				
           NOTE**
              A THOROUGH understanding of the open industry standard LDAP, as
              well as experience with result messages returned from LDAP, is
              required prior to successfully running this sample.  Please
              refer to the Domino Administration Guide for information
              regarding the configuration and implementation of accessing an
              LDAP directory with Notes Users and Domino servers.				


ENVIRONMENTS
------------
Windows 64-bit
Linux 64-bit


FILES
-----
ldap_ssl.c   - Main program.

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
   
2. If the certStore.nsf does not exist in the domino "data" directory, 
   then create one by command "load certMgr" on the domino console.
   
3. We can get a TLS Credentials doc created in certstore.nsf.
   In that case we can use the new micro CA that was introduced in 12.0.1.
   please do not use kyr files any more and we don't need openssl to create
   it any more. The link for the procedure is 
   https://help.hcltechsw.com/domino/12.0.0/admin/secu_creating_micro_certificates.html

RUNNING/CHECKING the result ldap_ssl:
-------------------------------------
Usage -

   ldap_ssl  <Hostname> <Organization> <DN> <Password>

- After the program is successfully compiled, at the system prompt type:

   Case 1 : (Success)

   C:\CAPI\notes-capi\samples\admin\ldap_ssl>ldap_ssl "HOSTNAME.PROD.ABC.COM" "ABC" "CN=local server, O=ABC" "PASSWORD"

   Successfully authenticated.

   Program completed successfully.


   case 2 : (Failure)

   C:\CAPI\notes-capi\samples\admin\ldap_ssl>ldap_ssl "HOSTNAME.PROD.ABC.COM" "ABC" "CN=local server, O=ABC" "WRONGPASSWORD"

   Failed to authenticate.
   ldap_simple_bind_s: resultCode 49 (Invalid credentials)
   ldap_simple_bind_s: errorMessage: Failed, invalid credentials for CN=local server, O=ABC
