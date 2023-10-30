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

 PROGRAM:   idbackup

 FILE:      idbackup.c

 PURPOSE:   This program is intended to illustrate the practical usage of the
            SECBuildEncryptedBackupIDFile function of the Notes ID Backup and Recovery API.  
            It is not intended to show exclusive use of these functions or the most
            effective program logic when using these functions.

****************************************************************************/
#include <stdio.h>
#include <string.h>

#include <global.h>
#include <addin.h>
#include <nsfdb.h>
#include <nif.h>
#include <nsfsearc.h>
#include <kfm.h>
#include <osmisc.h>
#include "../../printLog.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

char *DB_NAME	= "IDStore.nsf";
char *view_name = "IDWaitProcess";

STATUS doJob(char *);
STATUS LNPUBLIC note_action(VOID *db_handle,
                            SEARCH_MATCH far *pSearchMatch,
                            ITEM_TABLE *summary_info);
							
STATUS LNPUBLIC  AddInMain (HMODULE hModule, int argc, char *argv[])
{

    STATUS     error = 0;           /* return code from API calls */
    DHANDLE    hOldStatusLine;  /* handle to initial default status line*/
    DHANDLE    hStatusLineDesc; /* handle to new default status line */
    HMODULE    hMod;            /* add-in task's module handle */
    char       statusline[] =  "Build encrypted back up ID file from IDStire.nsf";
	
    AddInQueryDefaults (&hMod, &hOldStatusLine);
    AddInDeleteStatusLine (hOldStatusLine);
    hStatusLineDesc = AddInCreateStatusLine(statusline);
    AddInSetDefaults (hMod, hStatusLineDesc);
	
    while (!AddInIdle())
    {
    /* For non-preemptive operating systems, give up control to other tasks. 
    OSPreemptOccasionally will do nothing in preemptive operating systems. */
        OSPreemptOccasionally(); 
		
        if (AddInMinutesHaveElapsed(3))
        {
			doJob(DB_NAME);
        }
		
    } 
    
    return (NOERROR);
}

STATUS LNPUBLIC note_action(VOID *db_handle,
                            SEARCH_MATCH far *pSearchMatch,
                            ITEM_TABLE *summary_info)
{
	STATUS			err = NOERROR;
	SEARCH_MATCH    SearchMatch;   // local copy of search match 
	NOTEHANDLE		note_handle;
	BLOCKID			objectItem;
	char			idpass[MAXPATH] = {0};
	KFM_PASSWORD		retHashedPassword;
	char			*pBackupIDFileName = "d:\\idback.ide";
	char			pRepositoryBuf[MAXPATH] = {0};
	DWORD			retRepositoryBufLen = 0;
	char status[128] = {0};
    memcpy ((char*)(&SearchMatch), (char *)pSearchMatch, sizeof(SEARCH_MATCH));
	

	AddInLogMessageText("find a doc, process it", NOERROR);	
	if (err = NSFNoteOpen (
	                       *(DBHANDLE *)db_handle,         
	                       SearchMatch.ID.NoteID,         
	                       0,                          
	                       &note_handle))
	{
	    PRINTERROR(err,"NSFNoteOpen");
	    return err;
	}
	
	NSFItemGetText(note_handle, "status", status, sizeof(status));
	if (strcmp(status, "1") == 0) {
	    AddInLogMessageText("this note is processed already", NOERROR);
	    NSFNoteClose(note_handle);
	    return NOERROR;
	}

	AddInLogMessageText("note is opened", NOERROR);	
	NSFItemGetText(note_handle, "idpass", idpass, sizeof(idpass));
	AddInLogMessageText("get its password", NOERROR);
	AddInLogMessageText(idpass, NOERROR);
	
	if (err = NSFItemSetText(note_handle, "status", "1", strlen("1"))) {
	    PRINTERROR(err,"NSFItemSetText");
	    return err;
	}
	if (err = NSFNoteUpdate(note_handle, UPDATE_FORCE)) {
	    PRINTERROR(err,"NSFNoteUpdate");
	    return err;
	}
	if (NSFNoteHasObjects(note_handle, &objectItem)) {
	    AddInLogMessageText("note has object", NOERROR);
	    AddInLogMessageText("extract id file from the note", NOERROR);
	    if (err = NSFNoteExtractFile(note_handle, objectItem, "d:\\sample.id", NULL)) {
	        PRINTERROR(err,"NSFNoteExtractFile");
	        NSFNoteClose(note_handle);
	        return err;
		}
			
	    if (err = NSFNoteClose(note_handle)) {
	        PRINTERROR(err,"NSFNoteClose");
	        return err;
	    }
			
	    SECKFMCreatePassword(idpass, &retHashedPassword);
	    if (err = SECBuildEncryptedBackupIDFile("d:\\sample.id", 
	                                             &retHashedPassword, pBackupIDFileName, pRepositoryBuf,
	                                             MAXPATH, &retRepositoryBufLen, NULL, 0)) {
	                                             PRINTERROR(err,"SECBuildEncryptedBackupIDFile");
	        return err;
	    }
	}
	return NOERROR;
}

STATUS doJob(char *db_name)
{
	DBHANDLE db_handle;
	STATUS error = 0;
	AddInLogMessageText("open the db", NOERROR);
	if (error = NSFDbOpen(db_name, &db_handle)) {
	    PRINTERROR(error,"NSFDbOpen");
	    return ERR(error);
	}
	AddInLogMessageText("search the db", NOERROR);
	if (error = NSFSearch (
	                       db_handle,				/* database handle */
	                       NULL,					/* selection formula */
	                       view_name,				/* title of view in selection formula */
	                       0,						/* search flags */
	                       NOTE_CLASS_DOCUMENT,	/* note class to find */
	                       NULL,					/* starting date (unused) */
	                       note_action,			/* action routine for notes found */
	                       &db_handle,				/* parameter to action routine */
	                       NULL))					/* returned ending date (unused) */
	{
	    PRINTERROR(error,"NSFSearch");
	    NSFDbClose(db_handle);
	    return ERR(error);
	}	
	NSFDbClose(db_handle);
	return NOERROR;
}
