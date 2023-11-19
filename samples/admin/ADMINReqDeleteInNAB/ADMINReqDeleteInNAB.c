/*************************************************************************
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

PROGRAM:    ADMINReqDeleteInNAB

FILE:       ADMINReqDeleteInNAB.c

PURPOSE:    Shows the usage of ADMINReqDeleteInNAB and ADMINReqDeleteInNABExt API.

SYNTAX:     ADMINReqDeleteInNAB  <server name> <database filename> <" "/"ext">

*************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* OS and C include files */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* HCL C API for Notes/Domino include files */

#include "global.h"
#include "nsfdb.h"
#include "nsfdata.h"
#include "osmisc.h"
#include "osfile.h"
#include "reg.h"
#include "adminp.h"

#include <osmem.h>
#include <osenv.h>
#include <nsfnote.h>
#include <editods.h>
#include <stdnames.h>
#include <mailserv.h>
#include <mail.h>
#include <textlist.h>

#if defined(OS390)
#include "lapicinc.h"
#endif
#include "lapiplat.h"
#if defined(CAPI_TESTING) 
#include "printlog.h" 
#else
 #define PRINTLOG printf 
 #define PRINTERROR(api_error, api_name) {\
 char    szErrorText[256] = { 0 };\
 OSLoadString(NULLHANDLE, ERR(api_error), szErrorText, sizeof(szErrorText));\
 fprintf(stderr, "[ERROR]:%s:%d:%s - %s", __FILE__,__LINE__,api_name,szErrorText); }
#endif 

/* NOTE: This code MUST be the LAST file included so that ascii versions of the system APIs are used     */
#if defined(OS390) && (__STRING_CODE_SET__==ISO8859-1 /* If os390 ascii compile                          */)     
#include <_Ascii_a.h>
#endif

/* constant types */
 #define     SERVER_NAME_LEN         80
 #define     STDIN                   stdin
 #define     STRING_LENGTH           256

/* function prototypes */

void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                               char *server_name, char *db_name, char *ext);

/* Program declaration */
int main(int argc, char *argv[])
{
	/* Local data declarations */

	DBHANDLE       db_handle = NULLHANDLE;             /* database handle */
	STATUS         error = NOERROR;                    /* error code from API calls */
	char           szAuthor[MAXUSERNAME+1];            /* author's name */
	char           szUserName[MAXUSERNAME+1] = "CN=e user/O=HCLPNP";          /* user that must be deleted */  
	char           szMailServerName[MAXPATH];          /* mail server name */
	char           szMailFileName[MAXPATH] = "mail\\euser.nsf";               /* mail file name */
	char          *pszDeleteMailFile = "0";            /* "0" = Don't delete mail file */
                                                       /* "1" = Delete just mail file specified in person record */
                                                       /* "2" = Delete mail file specified in person record & all replicas */
														  
	char          *pszIDVaultFlag = "2";	           /* "0" = Don't delete, just mark as inactive */
                                                       /* "1" = Delete the user from the vault */
                                                       /* "2" = Don't do anything with the user's ID in the vault */
												
	char          *pszIDVaultName = "IDVault";  												        
	ADMINReqParams ARPptr;                             /* ADMINReqParams structure */
 
    char           szServer[SERVER_NAME_LEN];          /* server names are < 80 chars >*/
    char           szDBName[MAXPATH];
	char           szExt[STRING_LENGTH];
	char          *pszServerName = "";
    char          *pszAdminFile = "";
	char          *pszExt = "";
	char           szAdminFilePath[MAXPATH+1];
	
	pszServerName = szServer;
	pszAdminFile = szDBName;
	pszExt = szExt;
	
	/* Process input arguments */
	ProcessArgs(argc, argv, pszServerName, pszAdminFile, pszExt);
	
	/* Initialize the notes */
	if (error = NotesInitExtended(argc, argv))
    {
        PRINTERROR (error,"NotesInitExtended");
        return(1);
    }

    /* Construct the path for the admin request file */
    if (error = OSPathNetConstruct(NULL, pszServerName, pszAdminFile, szAdminFilePath))
    {
        PRINTERROR(error,"OSPathNetConstruct");
        NotesTerm();
        return (1);
    }

	/* Open the database. */
	if (error = NSFDbOpen(szAdminFilePath, &db_handle))
	{	
	    PRINTERROR (error,"NSFDbOpen");
	    NotesTerm();
	    return (1);
	}
	
	/* Get the current user name */
	if (error = SECKFMGetUserName(szAuthor))
	{
		PRINTERROR(error,"SECKFMGetUserName");
		NSFDbClose (db_handle);
	    NotesTerm();
		return (1);
	}
	
	/* Get the mail servername form notes.ini */
	OSGetEnvironmentString("MAILSERVER", szMailServerName, MAXPATH);

	if(strcmp(pszExt," ") == 0)
	{
		PRINTLOG("\nTesting ADMINReqDeleteInNAB\n");
		if (error = ADMINReqDeleteInNAB(db_handle,
	                                   szAuthor,
                                       szUserName,
									   szMailServerName,
									   szMailFileName,
									   pszDeleteMailFile,
									   &ARPptr,
									   sizeof(ARPptr)))
	   {
		   PRINTERROR(error,"ADMINReqDeleteInNAB");
		   NSFDbClose (db_handle);
		   NotesTerm();
	       return (1);	
	    }
	
	PRINTLOG("!!! ADMINReqDeleteInNAB Processed Sucessfully !!!\n ");
	}
	else if(strcmp(pszExt,"ext") == 0)
	{
		PRINTLOG("\nTesting ADMINReqDeleteInNABExt\n");
		if (error = ADMINReqDeleteInNABExt(db_handle,
	                                   szAuthor,
                                       szUserName,
									   szMailServerName,
									   szMailFileName,
									   pszDeleteMailFile,
									   pszIDVaultFlag,
									   pszIDVaultName,
									   &ARPptr,
									   sizeof(ARPptr)))
	    {
		    PRINTERROR(error,"ADMINReqDeleteInNABExt");
			NSFDbClose (db_handle);
		    NotesTerm();
	        return (1);		
	    }
	
	PRINTLOG("!!! ADMINReqDeleteInNABExt Processed Sucessfully !!!\n ");
	}
	else
	{
		PRINTLOG("Please enter following values:\n 1. \" \" - For testing ADMINReqDeleteInNAB \n 2. \"ext\" - For testingADMINReqDeleteInNABExt\n");
		NSFDbClose (db_handle);
		NotesTerm();
		return(1);
	}
	
	/* Close the database. */
	if (error = NSFDbClose (db_handle))
	{
	    PRINTERROR (error,"NSFDbClose");
	    NotesTerm();
	    return (1);
	}
									   
}
									   
/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:     argc, argv - directly from the command line

    OUTPUTS:    server_name - server name obtained from command line or
                              from prompt.
                db_name -     database name obtained from command line or
                              from prompt.
				ext -         which version of API you want to test.
                              Give " " to test ADMINReqDeleteInNAB and
							  give "ext" to test ADMINReqDeleteInNABExt.
 
*************************************************************************/

void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                               char *server_name, char *db_name, char *ext)
{
    if (argc != 4)
    {

        printf("Enter server name: ");
        fflush (stdout);
        fgets(server_name, SERVER_NAME_LEN, STDIN);
        printf("\n");
        printf ("Enter database filename:  ");
        fflush (stdout);
        fgets(db_name, MAXPATH-1, STDIN);
		printf("\n");
        printf ("Enter \" \" OR \"ext\" for testing ADMINReqDeleteInNAB OR ADMINReqDeleteInNABExt:");
        fflush (stdout);
        fgets(ext, STRING_LENGTH-1, STDIN);
    }
    else
    {
        memset(server_name, '\0', SERVER_NAME_LEN);    
        strncpy(server_name, argv[1], SERVER_NAME_LEN-1);
        memset(db_name, '\0', MAXPATH);    
        strncpy(db_name, argv[2], MAXPATH-1);
		memset(ext, '\0', STRING_LENGTH);    
        strncpy(ext, argv[3], STRING_LENGTH-1);
    } /* end if */
} /* ProcessArgs */

