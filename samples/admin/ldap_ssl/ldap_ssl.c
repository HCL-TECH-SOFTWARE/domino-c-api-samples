/****************************************************************************
 *
 * Copyright HCL Technologies 1996, 2023.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
   PROGRAM:   ldap_ssl

   FILE:      ldap_ssl.c

   PURPOSE:   This program is intended to illustrate the practical usage
              of the Lightweight Directory Assistance Protocol (LDAP) with SSL
	          as implemented via C API program. 
				
              Using the LDAP with SSL functionality of the C API Toolkit for
              Notes/Domino, this program will:
				
              - Create an LDAP with SSL handle.
              - Authenticate with the user.
				
   SYNTAX:    ldap_ssl

*************************************************************************/

#include "global.h"
#include "ldap.h"
#include "../../printLog.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>



#define  STRING_LENGTH  256
#define  PORT_NO  636              /* SSL port no. */
#define  SECURE  1                 /* TRUE to attempt a secure, encrypted connection. */
#define  SSLOPTIONS  0             /* Refer LDAP_SSLOPTS_xxx */
#define  SSLPROTOCOLVER  3         /* The SSL protocol version to use.Here, 3 - SSLv3 only. */

/* Program declaration */
int main( int argc, char *argv[] )
{
    LDAP    *pLDAPHandle = NULL;                        /* LDAP handle */
    char    szHostName[STRING_LENGTH] = { 0 };          /* Hostname */
    char    szOrgName[STRING_LENGTH] = { 0 };           /* Organization name */
    char    szDName[STRING_LENGTH] = { 0 };             /* Distinguished name */
    char    szPassword[STRING_LENGTH] = { 0 };          /* Password */
    STATUS  error = NOERROR;


    if ( argc > 5 )
    {
        PRINTLOG( "\nError: incorrect syntax\n" );
        PRINTLOG( "\nUsage:%s  <Hostname> <Organization> <DN> <Password> \n", argv[0] );
        return(1);
    }
    else if ( argc == 5 )
    {
        strncpy( szHostName, argv[1], sizeof(szHostName)-1 );
        strncpy( szOrgName, argv[2], sizeof(szOrgName)-1 );
        strncpy( szDName, argv[3], sizeof(szDName)-1 );
        strncpy( szPassword, argv[4], sizeof(szPassword)-1 );
    }
    else
    {
        PRINTLOG ( "\nError: incorrect syntax\n" );
        PRINTLOG ( "\nUsage:%s  <Hostname> <Organization> <DN> <Password> \n", argv[0] );
        return(1);
    }

    /* Intialize notes */   
    if ( error = NotesInitExtended ( argc, argv ))
    {
        PRINTLOG ( "\nUnable to initialize Notes. Error Code[0x%04x]\n", error );
        return (1);
    }
    
    /* Get a handle to an LDAP connection */	
    if (( pLDAPHandle = ldap_sslinit( szHostName, PORT_NO, SECURE, SSLOPTIONS, SSLPROTOCOLVER )) == NULL)
    {
        PRINTLOG ( "\nFailed to get a handle to LDAP with SSL.\n" );
        NotesTerm();
        return(1);
    }
	
    /* Authenticate to the directory as administrator */
    if ( ldap_simple_bind_s( pLDAPHandle, szDName, szPassword ) != LDAP_SUCCESS )
    {
        PRINTLOG ( "\nFailed to authenticate.\n" );
        ldap_perror(pLDAPHandle, "ldap_simple_bind_s");
        ldap_unbind_s(pLDAPHandle);
        NotesTerm();
        return(1);
    }
    else
    {
        PRINTLOG ( "\nSuccessfully authenticated.\n" );
    }

    ldap_unbind_s(pLDAPHandle);
    NotesTerm();
    PRINTLOG ( "\nProgram completed successfully.\n" );
    return 0;
}
