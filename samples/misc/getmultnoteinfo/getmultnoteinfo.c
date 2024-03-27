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

PROGRAM:    getmultnoteinfo

FILE:       getmultnoteinfo.c

PURPOSE:    Shows the basic usage of NSFDbGetMultNoteInfo & 
NSFDbGetMultNoteInfoByUNID. The program prints NoteIDs & 
UNIDs to the screen.

SYNTAX:     getmultnoteinfo [server name - optional] <database filename>

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
#include "osmisc.h"
#include "osfile.h"
#include "ods.h"
#include "nsfsearc.h"
#include "idtable.h"
#include "nsfnote.h"
#include "osmem.h"
#if defined(CAPI_TESTING) 
#include "printlog.h" 
#else
 #define PRINTLOG printf 
 #define PRINTERROR(api_error, api_name) {\
 char    szErrorText[256] = { 0 };\
 OSLoadString(NULLHANDLE, ERR(api_error), szErrorText, sizeof(szErrorText));\
 fprintf(stderr, "[ERROR]:%s:%d:%s - %s", __FILE__,__LINE__,api_name,szErrorText); }
#endif 

#ifndef OSMISC_DEFS
#include <osmisc.h>
#endif

/* NOTE: This code MUST be the LAST file included so that ascii versions of the system APIs are used     */
#if defined(OS390) && (__STRING_CODE_SET__==ISO8859-1 /* If os390 ascii compile                          */)     
#include <_Ascii_a.h>
#endif

#define MaxNoteNum 30

/* Local function prototypes */

STATUS LNPUBLIC AddIDUnique (void far *, SEARCH_MATCH far *, ITEM_TABLE far *);


WORD				NumIDs = 0;
UNID				unid[MaxNoteNum];

/* Program declaration */
int main(int argc, char *argv[])
{
	/* Local data declarations */

	char			pname[MAXPATH] = "";         /* buffer to store the input path to database */
	char			*path_name;                  /* pathname of database */
	char			*server_name;                /* server name where database lives*/
	char			*db_name;                    /* name of database */
	int			ArgNum = 0;
	int			i = 0;	

	DBHANDLE		hDB;
	STATUS			error = NOERROR;

	char			*UNIDTable,		*NoteIDTable;
	NOTEID			ID;

	DHANDLE			hNoteIDTable,		hUNIDTable;
	DHANDLE			hIDTable;
	DHANDLE			hInfoTable,		hInfoTableByUNID;
	DWORD			InfoTableLength,		InfoTableLengthByUNID;
	char			*InfoTable,		*InfoTableByUNID;
	NOTEID			id[MaxNoteNum],		idByUNID[MaxNoteNum];
	OID			oid[MaxNoteNum],		oidByUNID[MaxNoteNum];
	BOOL			First;
	NOTEHANDLE              noteHandle;
	BOOL                    bFieldFound;
	WORD                    wFieldLen;
	char                    szFieldText[500] = {0};
	UNID                    *pUnid = NULL;

#if defined(OS390)
	char XLATE_path_name[MAXPATH] = "";     /* path name translation buffer */
	char XLATE_title[NSF_INFO_SIZE] = "";   /* database title translation buffer */
#endif

	/* Initialize pointers to point to "" */
	path_name = pname;
	server_name = pname;
	db_name = pname;

	ArgNum = argc;

	/* Get the pathname of the database from the command line. */
#if defined(OS390) && (__STRING_CODE_SET__==ISO8859-1 /* If os390 ascii compile                          */)     
	__argvtoascii_a(argc,argv);                      /* Convert args from EBCDIC (Native z/OS) to ASCII */
#endif
	if((ArgNum < 2) || (ArgNum >3))
	{
	    PRINTLOG( "\nUsage:  %s  [server name - optional] <database filename>\n", argv[0] );
	    return (0);
	}

	db_name = argv[ArgNum - 1];
	path_name = db_name;


	if (ArgNum == 3)
	{
		server_name = argv[1];
	}

	error = NotesInitExtended (argc, argv);

	if (error)
	{
	    fprintf (stderr, "\nError initializing Notes.\n");
	    return (1);
	}

	if (strcmp (server_name, ""))
	{
	    if (error = OSPathNetConstruct( NULL, server_name, db_name, pname))
	    {
	        PRINTERROR (error,"OSPathNetConstruct");
	        NotesTerm();
	        return (1);
	    }
	    path_name = pname;
	}

	/* Open the database. */

	if (error = NSFDbOpen (path_name, &hDB))
	{
	    PRINTERROR (error,"NSFDbOpen");
	    NotesTerm();
	    return (1);
	}

	if (error = IDCreateTable(sizeof(NOTEID), &hIDTable))
	{
	    PRINTERROR (error,"IDCreateTable");
	    NSFDbClose (hDB);
	    NotesTerm();
	    return (1);
	}

	if (error = NSFSearch (
	                       hDB,            /* database handle */
	                       NULLHANDLE,     /* selection formula (select all notes) */
	                       NULL,           /* title of view in selection formula */
	                       0,              /* search flags */
	                       NOTE_CLASS_DOCUMENT,/* note class to find */
	                       NULL,           /* starting date (unused) */
	                       AddIDUnique,    /* call for each note found */
	                       &hIDTable,  /* argument to AddIDUnique */
	                       NULL))          /* returned ending date (unused) */
	{
	    PRINTERROR (error,"NSFSearch");
	    IDDestroyTable(hIDTable);
	    NSFDbClose (hDB);
	    NotesTerm();
	    return (1);
	}


	if (error = OSMemAlloc(0, NumIDs * ODSLength(_NOTEID), &hNoteIDTable))
	{
	    PRINTERROR (error,"OSMemAlloc");
	    IDDestroyTable(hIDTable);
	    NSFDbClose (hDB);
	    NotesTerm();
	    return(1);
	}

	NoteIDTable = OSLock(char, hNoteIDTable);

	First = TRUE;
	i = 0;
	while ( i < NumIDs && IDScan(hIDTable, First, &ID))
	{
	    ODSWriteMemory(&NoteIDTable, _NOTEID, &ID, 1);
	    i++;
	    First = FALSE;
	}

	OSUnlock(hNoteIDTable);
	IDDestroyTable(hIDTable);

	/*************************************************************************
	**	Call NSFGetMultNoteInfo routine.
	*************************************************************************/
	if (error = NSFDbGetMultNoteInfo(hDB, NumIDs, fINFO_OID, hNoteIDTable, &InfoTableLength, &hInfoTable))
	    PRINTERROR (error,"NSFDbGetMultNoteInfo");
	OSMemFree(hNoteIDTable);
	if (error)
	{
	    NSFDbClose (hDB);
	    NotesTerm();
	    return(1);
	}

	/*************************************************************************
	**	Check the number of note infos returned.
	*************************************************************************/
	if ( NumIDs * (sizeof(NOTEID) + sizeof(OID)) != InfoTableLength)
	{
	    PRINTLOG("Error in NSFDbGetMultNoteInfo: Number of note infos returned does not match.\n");
	    OSMemFree(hInfoTable);
	    NSFDbClose (hDB);
	    NotesTerm();
	    return(1);
	}

	InfoTable =(char *) OSLockObject(hInfoTable);
	for (i=0; i<NumIDs; i++)
	{
	    ODSReadMemory(&InfoTable, _NOTE_ID, id+i, 1);
	    ODSReadMemory(&InfoTable, _OID, oid+i, 1);
	}


	/*************************************************************************
	**	Construct the UNID table.
	*************************************************************************/
	if (error = OSMemAlloc(0, NumIDs * ODSLength(_UNID), &hUNIDTable))
	{
	    PRINTERROR (error,"OSMemAlloc");
	    OSUnlockObject(hInfoTable);
	    OSMemFree(hInfoTable);
	    NSFDbClose (hDB);
	    NotesTerm();
	    return(1);
	}

	UNIDTable = OSLock(char, hUNIDTable);

	ODSWriteMemory(&UNIDTable, _UNID, unid , NumIDs); /* unid is a UNID array */

	OSUnlock(hUNIDTable);

	/*************************************************************************
	**	Call NSFGetMultNoteInfoByUNID routine.
	*************************************************************************/
	if (error = NSFDbGetMultNoteInfoByUNID(hDB, NumIDs, fINFO_NOTEID|fINFO_OID, hUNIDTable, &InfoTableLengthByUNID, &hInfoTableByUNID))
	{
		PRINTERROR (error,"NSFDbGetMultNoteInfoByUNID");
	}
	OSMemFree(hUNIDTable);
	if (error)
	{
	    OSUnlockObject(hInfoTable);
	    OSMemFree(hInfoTable);
	    NSFDbClose (hDB);
	    NotesTerm();
	    return(1);
	}

	/*************************************************************************
	**	Call NSFNoteOpenByUNIDExtended routine.
	*************************************************************************/
	pUnid = unid;
	if (error = NSFNoteOpenByUNIDExtended(hDB, pUnid, 0, &noteHandle))
	{
	    PRINTERROR (error,"NSFNoteOpenByUNIDExtended");
	    OSUnlockObject(hInfoTable);
	    OSMemFree(hInfoTable);
	    NSFDbClose (hDB);
	    NotesTerm();
	    return(1);
	}

	/* Look for the TIME_DATE field within this note. */

	bFieldFound = NSFItemIsPresent (
	                                noteHandle,
	                                "TIME_DATE",
	                                (WORD) strlen ("TIME_DATE"));

	/* If the TIME_DATE field is there, get the contents of the field as an
	ASCII string and print it out. */

	if (bFieldFound)
	{
	    wFieldLen = NSFItemConvertToText (
	                                      noteHandle,
	                                      "TIME_DATE",
	                                      szFieldText,
	                                      (WORD) sizeof (szFieldText),
	                                      ';'); /* multi-value separator */

	    PRINTLOG ("TIME_DATE field is: %s\n", szFieldText);
	}

   	/* If the TIME_DATE field is not there, print a message. */

	else
	    PRINTLOG ("TIME_DATE field not found.\n");

	/* Close the note. */

	if (error = NSFNoteClose(noteHandle))
	{
	    PRINTERROR (error,"NSFNoteClose");
	    OSUnlockObject(hInfoTable);
	    OSMemFree(hInfoTable);
	    NSFDbClose (hDB);
	    NotesTerm();
	    return(1);
	}

	/*************************************************************************
	**	Check the number of IDs returned.
	*************************************************************************/
	if (( NumIDs * (sizeof(NOTEID) + sizeof(OID) )) != InfoTableLengthByUNID)
	{
	    PRINTLOG("Error in NSFDbGetMultNoteInfoByUNID: Number of UNIDs returned does not match.\n");
	    OSMemFree(hInfoTableByUNID);
	    OSUnlockObject(hInfoTable);
	    OSMemFree(hInfoTable);
	    NSFDbClose (hDB);
	    NotesTerm();
	    return(1);
	}

	InfoTableByUNID =(char *) OSLockObject(hInfoTableByUNID);
	for (i=0; i<NumIDs; i++)
	{
	    ODSReadMemory(&InfoTableByUNID, _NOTE_ID, idByUNID + i, 1);
	    ODSReadMemory(&InfoTableByUNID, _OID, oidByUNID + i, 1);
	}


	for (i=0; i<NumIDs; i++)
	{
	    PRINTLOG("id[%d] got by NoteID is %X\n", i ,id[i]);
	    PRINTLOG("idByUNID[%d] got by UNID is %X\n", i ,idByUNID[i]);

	    PRINTLOG("oid[%d] got by NoteID has a UNID %08X:%08X-%08X:%08X\n", i ,oid[i].File.Innards[1], oid[i].File.Innards[0], oid[i].Note.Innards[1], oid[i].Note.Innards[0]);
	    PRINTLOG("oidByUNID[%d] got by UNID has a UNID %08X:%08X-%08X:%08X\n", i, oidByUNID[i].File.Innards[1], oidByUNID[i].File.Innards[0], oidByUNID[i].Note.Innards[1], oidByUNID[i].Note.Innards[0]);
	}

	OSUnlockObject(hInfoTable);
	OSMemFree(hInfoTable);
	OSUnlockObject(hInfoTableByUNID);
	OSMemFree(hInfoTableByUNID);

	if (error = NSFDbClose (hDB))
	{
	    PRINTERROR (error,"NSFDbClose");
	    NotesTerm();
	    return (1);
	}

	if (error == NOERROR)
	    PRINTLOG("\nProgram completed successfully.\n");

	NotesTerm();
	return (0);

}


/************************************************************************

FUNCTION:   AddIDUnique                                             

*************************************************************************/

STATUS LNPUBLIC AddIDUnique    
(void far * phNoteIDTable,
 SEARCH_MATCH far *pSearchMatch,
 ITEM_TABLE far *summary_info)
{
	SEARCH_MATCH SearchMatch;
	DHANDLE    hNoteIDTable;
	STATUS       error = NOERROR;
	BOOL         flagOK;

	if( NumIDs >= MaxNoteNum )
	{
	    return (ERR(error));
	}

	memcpy( (char*)&SearchMatch, (char*)pSearchMatch, sizeof(SEARCH_MATCH) );

	if (!(SearchMatch.SERetFlags & SE_FMATCH))
	    return (NOERROR);

	hNoteIDTable = *((DHANDLE far *)phNoteIDTable);

	if (error = IDInsert(hNoteIDTable, SearchMatch.ID.NoteID, &flagOK))
	{
	    PRINTLOG ("Error: unable to insert note ID into table.\n");
	    return (ERR(error));
	}

	if (flagOK == TRUE)
	{
	    PRINTLOG ("\tInserted note %1X into table.\n", SearchMatch.ID.NoteID);
	    unid[NumIDs].File = SearchMatch.OriginatorID.File;
	    unid[NumIDs].Note = SearchMatch.OriginatorID.Note;
	    PRINTLOG("\tSearchMatch has a UNID %08X:%08X-%08X:%08X\n", unid[NumIDs].File.Innards[1], unid[NumIDs].File.Innards[0], unid[NumIDs].Note.Innards[1], unid[NumIDs].Note.Innards[0] );
	    ++ NumIDs;
	}
	else
	{
	    PRINTLOG ("\tNote %lX is already in table.\n", SearchMatch.ID.NoteID);
	}   

	return (ERR(error));
}
