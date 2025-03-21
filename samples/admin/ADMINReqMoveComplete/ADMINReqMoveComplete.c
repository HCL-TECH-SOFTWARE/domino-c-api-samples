/*************************************************************************
 *
 * Copyright HCL Technologies 1996, 2025.
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

PROGRAM:    ADMINReqMoveComplete

FILE:       ADMINReqMoveComplete.c

PURPOSE:    Shows the usage of ADMINReqMoveComplete & ADMINReqMoveUserInHier API.

SYNTAX:     ADMINReqMoveComplete  <server name> "admin4.nsf"

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
#include "osmem.h"
#include "reg.h"
#include "adminp.h"
#include "nif.h"


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
    NOTEHANDLE  nhNote = NULLHANDLE;         /* handle to note*/ 
    HCERTIFIER  hCertCtx = NULLHANDLE;       /* handle to certifier context */
    ADMINReqParams arpPtr,ARPptr;            /* ADMINReqParam structure*/
    BOOL        logged, ferror;              /* output params*/
    char        szOrgUnitCertID[] = "cert.id";                   /* organization cert ID*/
    char        szDataPath[MAXPATH] = {0};                       /* domino data path */
    char        szPassword[] = "pa55word";                       /* password for cert id*/
    char        szNewFirstName[]= "test";                        /* new first name */
    char        szNewLastName[]= "user1";                        /* new last name */
    char        szNewMiddleName[]= "";                           /* new middle name */
    char        szServer[MAXUSERNAME]={0};                       /* server name */
    char       *pszServerName = NULL;                            /* pointer to servername*/   
    char       *pszCertIDFilePath = NULL;                        /* domino cert ID file path*/
    char       *pszAdmin4NSFFile = NULL;                         /* pointer to admin DB file*/
    char        szAdminDBName[MAXPATH]= {0};                     /* admin DB file name */ 
    char        szAdminFilePath[MAXPATH] = {0};                  /* admin DB file path */
    DBHANDLE    db_handle = NULLHANDLE;                          /* DB file handle */
    char        szViewName[] = "Name Move Requests";             /* view name */
    HCOLLECTION colHandle = NULLHANDLE;                          /* collection handle */
    COLLECTIONPOSITION colPosition;                              /* collection position */
    NOTEID      viewID = 0;                                      /* view ID */ 
    DHANDLE     hBuffer = NULLHANDLE;                            /* handle to buffer of note ids */
    DWORD       dwEntriesFound = 0;                              /* number of entries found */
    NOTEID*     IdList = NULL;                                   /* pointer to a note id list */
    WORD        wSignalFlag = 0;                                 /* signal and share warning flags */
    DWORD       i=0;                                             /* local variable */
    DWORD       notesFound=0;                                    /* notes found variable */
    DBHANDLE    hNABook = NULLHANDLE;                            /* handle to NAB */
    char       *pszNABFile = NULL;                               /* pointer to NAB file*/
    char        szNABFilePath[MAXPATH] = {0};                    /* NAB file path*/
    char        szUserName[] = "CN=test user/O=HCLPNP";          /* user to be renamed*/
    char        szUserFld[] = "$Users";                          /* view to search for the user*/
    char        szDBName[MAXPATH]= "names.nsf";                  /* NAB database name*/
    NOTEID      noteID = 0;                                      /* noteID */
	
    pszServerName = szServer;
    pszAdmin4NSFFile = szAdminDBName;
    pszNABFile = szDBName;
	

    /* Process input arguments. */
    ProcessArgs(argc, argv, pszServerName, pszAdmin4NSFFile);
	
    /* Initialize the notes. */
    if (error = NotesInitExtended(argc, argv))
    {
        PRINTERROR (error,"NotesInitExtended");
        return(error);
    }
	
    /* Reading Domino data directory. */
    OSGetDataDirectory(szDataPath);
    PRINTLOG("data path : %s\n", szDataPath);

    /* Adding trailing path separator. */
    if (error = OSPathAddTrailingPathSeparator(szDataPath, sizeof(szDataPath)-1))
    {
        PRINTERROR(error,"OSPathAddTrailingPathSeparator");
        NotesTerm();
        return (error);
    }
    PRINTLOG("data path with trailing path separator: %s\n", szDataPath);

    /* Preparing path for cert ID. */
    strncat(szDataPath, szOrgUnitCertID, sizeof(szDataPath)-1 - strlen(szDataPath));
    pszCertIDFilePath = szDataPath;
    PRINTLOG("cert ID path : %s\n", pszCertIDFilePath);

    /* Construct the path for the admin request file. */
    if (error = OSPathNetConstruct(NULL, pszServerName, pszAdmin4NSFFile, szAdminFilePath))
    {
        PRINTERROR(error,"OSPathNetConstruct");
        NotesTerm();
        return (error);
    }
	
    /* Open the admin4 DB database. */
    if (error = NSFDbOpen(szAdminFilePath, &db_handle))
    {	
	PRINTERROR (error,"NSFDbOpen");
	NotesTerm();
	return (error);
    }
	
     /* Construct the path for the NAB file */
    if (error = OSPathNetConstruct(NULL, pszServerName, pszNABFile, szNABFilePath))
    {
        PRINTERROR(error,"OSPathNetConstruct");
        NSFDbClose(db_handle);
        NotesTerm();
        return (error);
    }

    PRINTLOG("*************************************************\n");
    PRINTLOG("***********Testing-ADMINReqMoveUserInHier********\n");
    PRINTLOG("*************************************************\n");

    /* Open the names.nsf (NAB) database. */
    if (error = NSFDbOpen(szNABFilePath, &hNABook))
    {
        PRINTERROR (error,"NSFDbOpen");
        NotesTerm();
        return (error);
    }

    /* Get the Note ID of the user to be renamed. */
    if (error = REGFindAddressBookEntry (hNABook, szUserFld, szUserName, &noteID))
    {
        NSFDbClose (hNABook);
        PRINTERROR (error,"REGFindAddressBookEntry");
        NotesTerm();
        return (error);
    }

    /* Get the Note handle. */
    if (error = NSFNoteOpen(hNABook, noteID, 0, &nhNote))
    {
        NSFDbClose (hNABook);
        PRINTERROR (error,"NSFNoteOpen");
        NotesTerm();
        return (1);
    }

    /* Get the current certifier context. */
    if (error = GetCertCtx(pszCertIDFilePath, &hCertCtx, szPassword))
    {
	NSFNoteClose (nhNote);
	NSFDbClose (db_handle);
	PRINTERROR (error,"GetCertCtx");
	NotesTerm();
	return (1);
    }

    /* Intializing the ADMINReqParams structure */
    memset(&ARPptr, 0x00, sizeof(ARPptr));

    if (error = ADMINReqMoveUserInHier ( hCertCtx,
                               hNABook,
                               nhNote,
                               "O=HCLPNP", /* Target Certifier */
                               &logged,
                               &ferror,
                               &ARPptr,
                               sizeof(ARPptr)) )
    {
        SECKFMFreeCertifierCtx (hCertCtx);
        NSFNoteClose (nhNote);
        NSFDbClose (hNABook);
        PRINTERROR (error,"ADMINReqMoveUserInHier");
        NotesTerm();
        return (error);
    }

    NSFNoteClose (nhNote);

    /* Close the database. */
    if (error = NSFDbClose (hNABook))
    {
        SECKFMFreeCertifierCtx (hCertCtx);
        NSFDbClose (db_handle);
	PRINTERROR (error,"NSFDbClose");
	NotesTerm();
	return (error);
    }

    PRINTLOG("!!! ADMINReqMoveUserInHier Processed Sucessfully !!!\n ");

    PRINTLOG("\n*************************************************\n");
    PRINTLOG("*************Testing-ADMINReqMoveComplete********\n");
    PRINTLOG("*************************************************\n");

    /* Find the view ID for viewname. */
    if (error = NIFFindView(
                   db_handle, 
                   szViewName, 
                   &viewID))
    {
	NSFDbClose (db_handle);
	PRINTERROR (error,"NIFFindView");
	NotesTerm();
	return (error);
    }	
	
    /* Open the Collection. */
    if (error = NIFOpenCollection(db_handle,
					db_handle,
					viewID,
					0,
					NULLHANDLE,
					&colHandle,
					NULLHANDLE,
					NULL,
					NULLHANDLE,
					NULLHANDLE))
    {
	NSFDbClose (db_handle);
	PRINTERROR (error,"NIFOpenCollection");
	NotesTerm();
	return (error);
    }
	
    colPosition.Level = 0;
    colPosition.Tumbler[0] = 0;

    /* Get a buffer of all the note IDs for the notes in this collection.
       Check the return code to see if the collection is empty. */

    do
    {
	error = NIFReadEntries(
			colHandle,       /* handle to this collection */
			&colPosition,    /* where to start in collection */
			NAVIGATE_NEXT,   /* order to skip entries */
			1L,              /* number to skip */
			NAVIGATE_NEXT,   /* order to use after skipping */
			0xFFFF,          /* max return number */
			READ_MASK_NOTEID,/* info we want */
			&hBuffer,        /* handle to info (return) */
			NULL,            /* length of buffer (return) */
			NULL,            /* entries skipped (return) */
			&dwEntriesFound, /* number of notes (return) */
			&wSignalFlag) ;  /* share warning (return) */

	if (error) 
	{
	    NIFCloseCollection (colHandle);
	    NSFDbClose (db_handle);
	    PRINTERROR (error,"NIFReadEntries");
	    NotesTerm();
	    return (error);
	}
		

	/* Lock down (freeze the location) of the buffer of notes IDs. Cast
	   the resulting pointer to the type we need. */

	if (hBuffer == NULLHANDLE)
	{
	    PRINTLOG ("\nEmpty buffer returned by NIFReadEntries.\n");
	    NIFCloseCollection (colHandle);
	    NSFDbClose (db_handle);
	}

	IdList = (NOTEID far*)OSLockObject(hBuffer);

	if(IdList == NULL)
	{
	    PRINTLOG ("\nNote ID list is Empty.\n");
	    NIFCloseCollection (colHandle);
	    NSFDbClose (db_handle);
            OSUnlockObject(hBuffer);
            OSMemFree(hBuffer);
            NotesTerm();
            return (error);
	}
	else {
	/* Print out the list of all the note IDs in this collection. */

	    for (i=0; i<dwEntriesFound; i++)
	    {
	         if (NOTEID_CATEGORY & IdList[i])
			    continue;
	         PRINTLOG ("Note count is %lu. \t Note ID is: %lX\n",
                 ++notesFound, IdList[i]);
	    }

	    NOTEID nid = IdList[1];
            if (error = NSFNoteOpen(db_handle, nid, 0, &nhNote))
            {
	         NIFCloseCollection (colHandle);
	         NSFDbClose (db_handle);
	         PRINTERROR (error,"NSFNoteOpen");
                 OSUnlockObject(hBuffer);
                 OSMemFree(hBuffer);
	         NotesTerm();
	         return (error);
            }
	}

	/* Unlock the list of note IDs. */

	OSUnlockObject(hBuffer);

	/* Free the memory allocated by NIFReadEntries. */

	OSMemFree(hBuffer);

	/* Breaking out of the loop. */

	break;

    } while (wSignalFlag & SIGNAL_MORE_TO_DO);
	
    /* Intializing the ADMINReqParams structure. */ 
    memset(&arpPtr, 0x00, sizeof(arpPtr));
    arpPtr.chFirstName = szNewFirstName;
    arpPtr.chMiddleInitial = szNewMiddleName;
    arpPtr.chLastName = szNewLastName;
	
    if (error = ADMINReqMoveComplete ( hCertCtx, /* certifier context */
	                           db_handle,        /* handle to ADMIN file */
	                           nhNote,           /* handle to notes document */
	                           "O=ABCORP",       /* Target Certifier */
	                           &logged,          /* TRUE if request has been recorded in the certification log */
	                           &ferror,          /* TRUE if an error occurred */
	                           &arpPtr,          /* pointer to an ADMINReqParams structure */
	                           sizeof(arpPtr)))  /* size of the buffer (ADMINReqParams structure) */
    {
        SECKFMFreeCertifierCtx (hCertCtx);
	NSFNoteClose (nhNote);
	NSFDbClose (db_handle);
	PRINTERROR (error,"ADMINReqMoveComplete");
	NotesTerm();
	return (error);
    }

    PRINTLOG("!!! ADMINReqMoveComplete Processed Sucessfully !!!\n ");
	
    /* Free the certifier context and close the note */
    SECKFMFreeCertifierCtx (hCertCtx);
    NSFNoteClose (nhNote);
	
    /* Close the database. */
    if (error = NSFDbClose (db_handle))
    {
	PRINTERROR (error,"NSFDbClose");
	NotesTerm();
	return (error);
    }

    /* Terminate Domino and Notes. */
    NotesTerm();

    /* End of program. */
    return (error);

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

