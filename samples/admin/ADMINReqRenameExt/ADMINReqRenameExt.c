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

PROGRAM:    ADMINReqRenameExt

FILE:       ADMINReqRenameExt.c

PURPOSE:    Shows the usage of ADMINReqRenameExt API.

SYNTAX:     ADMINReqRenameExt  <server name> "names.nsf"

*************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* OS and C include files */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* HCL C API for Notes/Domino include files */

#include "global.h"
#include "nsfdb.h"
#include "nsfdata.h"
#include "nsfnote.h"
#include "osmisc.h"
#include "osfile.h"
#include "ostime.h"
#include "reg.h"
#include "adminp.h"


#if defined(CAPI_TESTING) 
#include "printlog.h" 
#else
 #define PRINTLOG printf 
 #define PRINTERROR(api_error, api_name) {\
 char    szErrorText[256] = { 0 };\
 OSLoadString(NULLHANDLE, ERR(api_error), szErrorText, sizeof(szErrorText));\
 fprintf(stderr, "[ERROR]:%s:%d:%s - %s", __FILE__,__LINE__,api_name,szErrorText); }
#endif 

/* constant types */
 #define     STDIN                   stdin

/* Local function prototypes */

void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                               char *server_name, char *db_name);


STATUS GetCertCtx (char far *, HCERTIFIER *, char far *);

/* Program declaration */
int main(int argc, char *argv[])
{
    /* Local data declarations */

    STATUS      error = NOERROR;             /* error code from API calls */
    DBHANDLE    hNABook = NULLHANDLE;        /* handle to NAB */
    NOTEID      noteID = 0;                  /* noteID */
    NOTEHANDLE  nhNote = NULLHANDLE;         /* handle to note*/ 
    HCERTIFIER  hCertCtx = NULLHANDLE;       /* handle to certifier context */
    ADMINReqParams arpPtr;                   /* ADMINReqParam structure*/
    BOOL        logged, ferror;              /* output params*/
    char        szUserName[] = "CN=test user/O=HCLPNP";          /* user to be renamed*/
    char        szOrgUnitCertID[] = "cert.id";                   /* organization cert ID*/
    char        szDataPath[MAXPATH] = {0};                       /* domino data path */
    char        szPassword[] = "password";                       /* password for cert id*/
    char        szNewFirstName[]= "A";                           /* new first name */
    char        szNewLastName[]= "B";                            /* new last name */
    char        szNewMiddleName[]= "C";                          /* new middle name */
    char        szUserFld[] = "$Users";                          /* view to search for the user*/
    char        szServer[MAXUSERNAME]={0};                       /* server name */
    char        szDBName[MAXPATH]={0};                           /* database name*/
    char       *pszServerName = NULL;                            /* pointer to servername*/   
    char       *pszNABFile = NULL;                               /* pointer to NAB file*/
    char        szNABFilePath[MAXPATH] = {0};                    /* NAB file path*/
    char       *pszCertIDFilePath = NULL;                        /* domino cert ID file path*/
	
    pszServerName = szServer;
    pszNABFile = szDBName;
	

    /* Process input arguments */
    ProcessArgs(argc, argv, pszServerName, pszNABFile);
	
    /* Initialize the notes */
    if (error = NotesInitExtended(argc, argv))
    {
        PRINTERROR (error,"NotesInitExtended");
        return(1);
    }

    /* Reading Domino data directory */
    OSGetDataDirectory(szDataPath);
    PRINTLOG("data path : %s\n", szDataPath);

    /* Adding trailing path separator*/
    if (error = OSPathAddTrailingPathSeparator(szDataPath, sizeof(szDataPath)-1))
    {
	PRINTERROR(error,"OSPathAddTrailingPathSeparator");
        NotesTerm();
        return (1);
    }
    PRINTLOG("data path with trailing path separator: %s\n", szDataPath);

    /* Preparing path for cert ID*/
    strncat(szDataPath, szOrgUnitCertID, sizeof(szDataPath)-1 - strlen(szDataPath));
    pszCertIDFilePath = szDataPath;
    PRINTLOG("cert ID path : %s\n", pszCertIDFilePath);

    /* Construct the path for the admin request file */
    if (error = OSPathNetConstruct(NULL, pszServerName, pszNABFile, szNABFilePath))
    {
        PRINTERROR(error,"OSPathNetConstruct");
        NotesTerm();
        return (1);
    }

    /* Open the database. */
    if (error = NSFDbOpen(szNABFilePath, &hNABook))
    {	
	PRINTERROR (error,"NSFDbOpen");
	NotesTerm();
	return (1);
    }

    /* Get the Note ID of the user to be renamed. */
    if (error = REGFindAddressBookEntry (hNABook, szUserFld, szUserName, &noteID))
    {
	NSFDbClose (hNABook);
	PRINTERROR (error,"REGFindAddressBookEntry");
	NotesTerm();
	return (1);
    }

    /* Get the Note handle. */
    if (error = NSFNoteOpen(hNABook, noteID, 0, &nhNote))
    {
	NSFDbClose (hNABook);
	PRINTERROR (error,"NSFNoteOpen");
	NotesTerm();
	return (1);
    }

    /* Get the current certifier context */
	if (error = GetCertCtx(pszCertIDFilePath, &hCertCtx, szPassword))
    {
	NSFNoteClose (nhNote);
	NSFDbClose (hNABook);
	PRINTERROR (error,"GetCertCtx");
	NotesTerm();
	return (1);
    }
	
    /* Intializing the ADMINReqParams structure */ 
    memset(&arpPtr, 0x00, sizeof(arpPtr));
	
    /* Call the ADMINReqRenameExt API for renaming the user CN=test user/O=HCLPNP to CN=ACB/O=HCLPNP */
    if (error = ADMINReqRenameExt( hCertCtx,         /* certifier context */
	                           hNABook,          /* handle to NAB file */
	                           nhNote,           /* handle to notes document */
	                           szNewFirstName,   /* new first name */
	                           szNewMiddleName,  /* new middle initial */
	                           szNewLastName,    /* new last name */
	                           NULL,             /* new org name */
	                           NULL,             /* new short name */
	                           NULL,             /* new internet address */
	                           &logged,          /* TRUE if request has been recorded in the certification log */
	                           &ferror,          /* TRUE if an error occurred */
	                           &arpPtr,          /* pointer to an ADMINReqParams structure */
	                           sizeof(arpPtr)))  /* size of the buffer (ADMINReqParams structure) */
    {
	SECKFMFreeCertifierCtx (hCertCtx);
	NSFNoteClose (nhNote);
	NSFDbClose (hNABook);
	PRINTERROR (error,"ADMINReqRenameExt");
	NotesTerm();
	return (1);
    }

    PRINTLOG("!!! ADMINReqRenameExt Processed Sucessfully !!!\n ");
	
    /* Close the database. */
    if (error = NSFDbClose (hNABook))
    {
	PRINTERROR (error,"NSFDbClose");
	NotesTerm();
	return (1);
    }

    /* Terminate Domino and Notes. */

    NotesTerm();

    /* End of program. */

    return (0);

}
/************************************************************************

    FUNCTION:    GetCertCtx

    PURPOSE:     returns a handle to the certifier context.

    INPUTS:
        pCertFile  - pointer to full path spec of certifier id file
        szPassword - password

    OUTPUTS:
        phCertCtx  - pointer to handle to Certifier context

    RETURNS:    status code from API Call

*************************************************************************/

STATUS GetCertCtx (char far *pCertFile, HCERTIFIER *phCertCtx,
                   char far *szPassword)
{
    TIMEDATE tdExpDate;
    STATUS error = NOERROR;
    char szCertName[MAXSPRINTF] = {0};            /* certifier name */
    BOOL bIsHierarchical = FALSE;
    WORD wFileVersion = 0;
    KFM_PASSWORD KFMPassword, *pKFMPassword;    /* encoded password */

    OSCurrentTIMEDATE(&tdExpDate);

    /* set the expiration date to two years from today (Domino and Notes default) */
    error = TimeDateAdjust(&tdExpDate, 0, 0, 0, 0, 0, 2);

    if (error)
    {
        return (error);
    }

   /* get the encoded password */
    if (szPassword == NULL)
    {
        pKFMPassword = NULL;
    }
    else
    {
        pKFMPassword = &KFMPassword;
        SECKFMCreatePassword (szPassword, pKFMPassword);
    }

   /* get the certifier context */
    error = SECKFMGetCertifierCtx (
                                   pCertFile,       /* Certifier file path spec */
                                   pKFMPassword,    /* encoded password */
                                   NULL,            /* no certifier log */
                                   &tdExpDate,      /* expiration date */
                                   szCertName,      /* returned name of certifier */
                                   phCertCtx,       /* returned handle to certifier context */
                                   &bIsHierarchical,/* returned TRUE if certifier is hierarchical */
                                   &wFileVersion);  /* returned file version */


    return (error);
}

/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:     argc, argv - directly from the command line

    OUTPUTS:    server_name - server name obtained from command line or
                              from prompt.
                db_name -     database name obtained from command line or
                              from prompt.

*************************************************************************/

void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                               char *server_name, char *db_name)
{
    if (argc != 3)
    {

        printf("Enter server name: ");
        fflush (stdout);
        fgets(server_name, MAXUSERNAME, STDIN);
        printf("\n");
        printf ("Enter database filename:  ");
        fflush (stdout);
        fgets(db_name, MAXPATH-1, STDIN);
    }
    else
    {
        memset(server_name, '\0', MAXUSERNAME);    
        strncpy(server_name, argv[1], MAXUSERNAME-1);
        memset(db_name, '\0', MAXPATH);    
        strncpy(db_name, argv[2], MAXPATH-1);
    } /* end if */
} /* ProcessArgs */

