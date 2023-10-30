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

    PROGRAM:    tracker

    FILE:       tracker.c

    DESCRIPTION:
        Tracker is an example NSF hook driver. After tracker is
        installed, Domino and Notes calls the entry points it provides 
        whenever Domino and Notes initializes, terminates, opens a note, 
        updates a note,or categorizes a set of notes. Updating a note 
        encompasses creation, modification, and deletion.

        Tracker is designed to work with the Service Request tracking
        database: tracker.nsf. Tracker keeps a log of all activity in
        the database. This log resides in a "TrackerLog" document in
        the database itself.

        Tracker also provides a "unique sequential request number"
        service and a "trash can" service to the Service Request Tracking
        application. Each time a user saves a newly composed service
        request form, Tracker assigns a request number to the "ReqNumber"
        field of the new document. Also, when a user deletes a document
        from the Service Request tracking database, this copies the
        document to a separate "trash can" database before the document
        is actually deleted by Domino and Notes. This trash can helps avoid 
        loss of data through accidental deletion of documents.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* C include files */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#if !defined(OS400)
#include <malloc.h>
#endif
#include <errno.h>

/* HCL C API for Notes/Domino include files */
#include <global.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <names.h>                      /* MAXUSERNAME */
#include <stdnames.h>                   /* FIELD_FORM */
#include <textlist.h>                   /* LIST, etc. */
#include <idtable.h>                    /* IDEntries() */
#include <ostime.h>
#include <osenv.h>
#include <osmisc.h>                     /* OSLoadString() */
#include <osmem.h>                      /* OSMemAlloc() */
#include <kfm.h>
#include <nsfdata.h>
#include <easycd.h>

/* Application-specific include files */
#include "tracker.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/* Global Variables */
DBHANDLE        hDB_global;
NOTEHANDLE      hLogNote_global;
DHANDLE           hCompound_global;
DWORD           dwStyleID_global;

/* Record all Tracker activity in the trace file */
FILE           *pTraceFile[MAX_TRACKER_INSTANCES];
char           TrackerLogFile[MAX_TRACKERLOG];
int            inst; /* instance index for multi-process systems */

/************************************************************************

    FUNCTION:   MainEntryPoint

    PURPOSE:    Domino and Notes calls this function whenever a Domino and 
                Notes process initializes. The main purpose of this function 
                is to fill in the DBHOOKVEC structure with function pointers
                that specify the other entry points in this DLL. This
                routine also starts the tracker log in the target DB.

*************************************************************************/

STATUS LNPUBLIC  MainEntryPoint(DBHOOKVEC * pDBHooks)
{
    char        szDBPathName[MAXENVVALUE];
    STATUS      error;
    DBHANDLE    hDB;
    char        szErrorMessage[MAXENVVALUE+50];

    /* initialize instances for multi-process systems */
    inst = 0;

    /* get the current instance number and update next */ 
    if (error = GetInstanceNumber(&inst))
    {
        /* if we have an error log it and return */
 
        strcpy(TrackerLogFile, TRACE_FILENAME);
        strcat(TrackerLogFile,".001");
  
      /* open the trace log file */
#if defined (OS400)
        /*
         * Open files in binary on the i5/OS to avoid conversion AND tag them w/ the
         * 819 CCSID for ASCII contents
         */
        if ((pTraceFile[inst] = fopen(TrackerLogFile, "wb ccsid=819")) == NULL )
#else
        if ((pTraceFile[inst] = fopen(TrackerLogFile, "w")) == NULL )
#endif
        {
            return NOERROR;
        }
        fprintf(pTraceFile[inst], "Error %d: GetInstanceNumber: exiting.\n", error);
        fclose(pTraceFile[inst]);
        return(error);
    }

    /* open the trace log file */
#if defined (OS400)
    if ( (pTraceFile[inst-1] = fopen(TrackerLogFile, "wb ccsid=819")) == NULL )
#else
    if ( (pTraceFile[inst-1] = fopen(TrackerLogFile, "w")) == NULL )
#endif
    {
        return NOERROR;
    }
    fprintf(pTraceFile[inst-1], "%s%s%s%s", "HCL Notes API.\n",
            "Tracker sample started ... \n", TRACKER_BLURB, 
            "MainEntryPoint: called.\n" );

    /* Get the name of the database to track */

    if (!OSGetEnvironmentString(TRACKER_DBNAME_VARIABLE, 
                                szDBPathName, MAXENVVALUE))
    {
        strcpy(szDBPathName, TRACKER_DEFAULT_DBNAME);
    }

    if (error = NSFDbOpen(szDBPathName, &hDB))
    {
        sprintf(szErrorMessage,
                "MainEntryPoint: unable to open target DB '%s'.",
                szDBPathName);
        TraceNotesError(error, szErrorMessage);
        return NOERROR;
    }

    if (error = StartLog(hDB))
    {
        NSFDbClose(hDB);
        return NOERROR;
    }

    /* Initialize the DBHOOK vector.*/
    pDBHooks->Term         = TrackerTerm;
    pDBHooks->NoteOpen     = TrackerNoteOpen;
    pDBHooks->NoteUpdate   = TrackerNoteUpdate;
    pDBHooks->DbStampNotes = TrackerDbStampNotes;

    hDB_global = hDB;

    fprintf(pTraceFile[inst-1], "MainEntryPoint: returning.\n" );

    return NOERROR;
}

/************************************************************************

    FUNCTION:   TrackerTerm

    PURPOSE:    Domino and Notes calls this function when terminating. 
                This puts the last entry in the Tracker Log document, 
                sets the End date, updates the log document, closes the
                log document, and closes the database.

*************************************************************************/

STATUS LNPUBLIC  TrackerTerm(DBHOOKVEC * pDBHooks)
{
    STATUS      error;
    NOTEHANDLE  hNote;
    DBHANDLE    hDB;

    fprintf( pTraceFile[inst-1], "TrackerTerm: called.\n" );

    hNote = hLogNote_global;
    hDB = hDB_global;

    FinishLog();

    if (error = NSFDbClose(hDB))
    {
        TraceNotesError(error, "TrackerTerm: unable to close database.");
    }

    fprintf(pTraceFile[inst-1], "TrackerTerm: returning.\n" );
    fclose(pTraceFile[inst-1]);

    return NOERROR;
}

/************************************************************************

    FUNCTION:   TrackerNoteOpen

    PURPOSE:    Domino and Notes calls this function each time a note is opened.

*************************************************************************/

STATUS LNPUBLIC  TrackerNoteOpen (
                                  DBHOOKVEC  *pDBHooks,
                                  char       *UserName, 
                                  LIST       *GroupList, 
                                  DBHANDLE    hDB, 
                                  NOTEID      NoteID, 
                                  NOTEHANDLE  hNote,
                                  WORD        UpdateFlags)
{
    STATUS      error;
    NUMBER      ReqNumber;
    char        szLine[MAX_TRACKERLOG+1];

    fprintf(pTraceFile[inst-1], "TrackerNoteOpen: called.\n" );

    PrintToTraceFile("TrackerNoteOpen", UserName, GroupList, hDB, NoteID, 
                     hNote, UpdateFlags);

    /* Check database name. Do nothing if it is not the target database. */
    if ( !IsTargetDB(hDB) )
    {                         
        goto Exit;
    }

    /* Check note Class and Form. Do nothing if not a Service Request */
    if ( !NoteIsServiceRequest(hNote) )
    {
        goto Exit;
    }

    /* Log the fact that this user is opening this request document */
    if (error = GetReqNumItem( hNote, &ReqNumber, "TrackerNoteOpen" ))
    {
        /* "ReqNumber" field is not available. Not a fatal error. */
        sprintf(szLine,
                "Service Request Log: user '%s' opened a request document.\n",
                UserName);

        fprintf(pTraceFile[inst-1], 
                "TrackerNoteOpen: user '%s' opened a request document.\n",
                UserName);
    }
    else
    {
        sprintf(szLine,
                "Service Request Log: user '%s' opened request number %f.\n",
                UserName, ReqNumber);

        fprintf(pTraceFile[inst-1], 
                "TrackerNoteOpen: user '%s' opened request number %f.\n",
                UserName, ReqNumber);
    }

    AddLogEntry(szLine, "TrackerNoteOpen");

Exit:
    fprintf(pTraceFile[inst-1], "TrackerNoteOpen: returning.\n" );
    return NOERROR;
}


/************************************************************************

    FUNCTION:   TrackerNoteUpdate

    PURPOSE:    Domino and Notes calls this function each time a note is updated.
                This includes notes that are being created, modified,
                or deleted.

*************************************************************************/

STATUS LNPUBLIC  TrackerNoteUpdate (
                                    DBHOOKVEC  *pDBHooks,
                                    char       *UserName, 
                                    LIST       *GroupList, 
                                    DBHANDLE    hDB, 
                                    NOTEID      NoteID, 
                                    NOTEHANDLE  hNote,
                                    WORD       *pwUpdateFlags)
{
    NUMBER      ReqNumber;
    WORD        UpdateFlags;
    STATUS      error;
    char        szLine[MAX_TRACKERLOG+1];

    fprintf( pTraceFile[inst-1], "TrackerNoteUpdate: called.\n" );

   /* 
    * This checks the update flags but does not modify them. Note, 
    * however, that it could modify the update flags by writing to
    * the word specified by <pwUpdateFlags>.
    */
    UpdateFlags = *pwUpdateFlags;

    PrintToTraceFile("TrackerNoteUpdate", UserName, GroupList, hDB, NoteID, 
                     hNote, UpdateFlags);

    /* Do nothing if hDB is not the target database. */
    if ( !IsTargetDB(hDB) )
    {
        goto Exit;
    }

    if (NoteID == NOTEID_ADD || NoteID == NOTEID_ADD_OR_REPLACE ||
        NoteID == NOTEID_ADD_UNID)
    {
        if (UpdateFlags & UPDATE_DELETED)
        {
            fprintf(pTraceFile[inst-1], "Adding a deletion stub to database.\n");
            goto Exit;
        }
        /* else adding a new document to target database. Do nothing if 
           not a Service Request.
         */
        if ( !NoteIsServiceRequest(hNote) )
        {
            goto Exit;
        }

       /* Get the next service request tracking number and set the
           ReqNumber field.
         */
        if (error = GetReqNumber(&ReqNumber))
        {
            ReqNumber = (NUMBER)0;
        }

        if (error = NSFItemSetNumber (hNote,
                    TRACKER_ITEM_REQ_NUMBER, &ReqNumber))
        {
            TraceNotesError(error, 
                            "TrackerNoteUpdate: Unable to set ReqNumber.");
            goto Exit;
        }

        fprintf(pTraceFile[inst-1],
                "TrackerNoteUpdate: user '%s' composed request number %f.\n",
                UserName, ReqNumber);

        sprintf(szLine,
                "Service Request Log: user '%s' composed request number %f.\n",
                UserName, ReqNumber);

        AddLogEntry(szLine, "TrackerNoteUpdate");
    }
    else
    {
        if (UpdateFlags & UPDATE_DELETED)
        {   /* Deleting an existing document from the database. Tracker
               copies the note to the "trashcan" database then allows the
               document to be deleted.
             */
            fprintf(pTraceFile[inst-1], 
                    "TrackerNoteUpdate: Deleting a request document.\n");

            if (error = CopyNoteToTrashcan(hDB,NoteID))
            {
                /* Could not copy note being deleted to trash can database.
                   This is serious. Do not let the update succeed. 
                 */
                fprintf(pTraceFile[inst-1], "TrackerNoteUpdate: returning.\n" );
                return(error);          
            }

           /* Add a log entry that the user deleted a request document.
              You can't get the Request Number of the document from hNote
              because the contents of hNote is already gone.
            */

            sprintf(szLine,
                    "Service Request Log: user '%s' deleted a request.\n",
                    UserName);

            fprintf(pTraceFile[inst-1], 
                    "TrackerNoteUpdate: user '%s' deleted a request document.\n",
                    UserName);
                    
            AddLogEntry(szLine, "TrackerNoteUpdate");
        }
        else
        {
            /* Updating an existing document in the database */
            if ( !NoteIsServiceRequest(hNote) )
            {
                goto Exit;
            }

            if (error = GetReqNumItem( hNote, &ReqNumber, 
                                       "TrackerNoteUpdate" ))
            /* if (FALSE == NSFItemGetNumber (hNote,
                        TRACKER_ITEM_REQ_NUMBER, &ReqNumber)) */
            {
                /* Field is not available. Not a fatal error. */
                sprintf(szLine,
                        "Service Request Log: user '%s' updated a request.\n",
                        UserName);

                fprintf(pTraceFile[inst-1], 
                        "TrackerNoteUpdate: user '%s' updated a request document.\n",
                        UserName);
            }
            else
            {
                sprintf(szLine,
                        "Service Request Log: user '%s' updated request number %f.\n",
                        UserName, ReqNumber);

                fprintf(pTraceFile[inst-1], 
                        "TrackerNoteUpdate: user '%s' updated request number %f.\n",
                        UserName, ReqNumber);
            }
            AddLogEntry(szLine, "TrackerNoteUpdate");
        }
    }

Exit:
    fprintf(pTraceFile[inst-1], "TrackerNoteUpdate: returning.\n" );
    return NOERROR;
}

/************************************************************************

    FUNCTION:   TrackerDbStampNotes

    PURPOSE:    start the Tracker log document

*************************************************************************/

STATUS LNPUBLIC  TrackerDbStampNotes (DBHOOKVEC  *pDBHooks,
                                      char       *UserName, LIST       *GroupList,
                                      DBHANDLE    hDB,      DHANDLE       hIDTable,
                                      char       *ItemName, WORD        ItemNameLength,
                                      void       *Data,     WORD        Length)
{
    STATUS  error;
    char   *szItemName;                 /* zero-terminated copy of ItemName */
    char   *szItemText;                 /* text rendering of Data value */
    char    szLine[MAX_TRACKERLOG+1];

    fprintf( pTraceFile[inst-1], "TrackerDbStampNotes: called.\n" );

    fprintf(pTraceFile[inst-1], "TrackerDbStampNotes: UserName = '%s'.\n", UserName);
    PrintListToTraceFile(GroupList, "TrackerDbStampNotes");
    PrintDBPathToTraceFile(hDB, "TrackerDbStampNotes");

    /* Do nothing if hDB is not the target database. */
    if ( !IsTargetDB(hDB) )
    {
        goto Exit;
    }
    /* Log an entry that user categorized these request documents. */

    memcpy((szItemName = (char *)malloc(ItemNameLength+1)),ItemName,ItemNameLength);
    szItemName[ItemNameLength] = '\0';

    if (error = RenderDataAsText(Data, Length, &szItemText))
    {
        free(szItemName);
        goto Exit;
    }

    sprintf(szLine,
            "Service Request Log: user '%s' set field '%s' to '%s' in %ld documents.",
            UserName, szItemName, szItemText, IDEntries(hIDTable));

    AddLogEntry(szLine, "TrackerDbStampNotes");

    free(szItemName);
    free(szItemText);

Exit:
    fprintf( pTraceFile[inst-1], "TrackerDbStampNotes: returning.\n" );
    return NOERROR;
}

/************************************************************************

    FUNCTION:   StartLog

    PURPOSE:    start the Tracker log document

*************************************************************************/

STATUS  LNPUBLIC  StartLog (DBHANDLE hDB)
{
    STATUS          error;
    NOTEHANDLE      hNote;
    char            szServer[MAXUSERNAME+1];
    TIMEDATE        tdStartTime;
    DHANDLE         hCompound;      /* Compound Text context */
    COMPOUNDSTYLE   Style;
    DWORD           dwStyleID;
    char            *szStartLogMessage;

    if (error = NSFNoteCreate(hDB, &hNote))
    {
        TraceNotesError(error,
                        "MainEntryPoint: unable to open tracker log note.");
        return (error);                            
    }

    /* Set FORM field */
    if (error = NSFItemSetText(hNote, FIELD_FORM,   /* "Form" */
                               TRACKER_FORM, MAXWORD))
    {
        TraceNotesError(error,
                        "MainEntryPoint: unable to set form field in tracker log.");
        NSFNoteClose(hNote);
        return (error);
    }

    /* Set Server field */
    if (error = SECKFMGetUserName (szServer))
    {
        TraceNotesError(error,
                        "MainEntryPoint: unable to get current server or user name.");
        NSFNoteClose(hNote);
        return (error);
    }
    if (error = NSFItemSetText(hNote, TRACKER_ITEM_SERVER,  /* "Server" */
                               szServer, MAXWORD))
    {
        TraceNotesError(error,
                        "MainEntryPoint: unable to set Server field in tracker log.");
        NSFNoteClose(hNote);
        return (error);
    }

    /* set the StartTime field */
    OSCurrentTIMEDATE(&tdStartTime);
    if (error = NSFItemSetTime(hNote, TRACKER_ITEM_STARTTIME, /*"StartTime"*/
                               &tdStartTime))
    {
        TraceNotesError(error,
                        "MainEntryPoint: unable to set StartTime field in tracker log.");
        NSFNoteClose(hNote);
        return (error);
    }

    /* Create compound text context. This initializes hCompound. */
    if (error = CompoundTextCreate (
                                    hNote, 
                                    TRACKER_ITEM_BODY,  /* "Body" */
                                    &hCompound)) 
    {
        TraceNotesError(error,
                        "MainEntryPoint: unable to create Compound Text context.");
        NSFNoteClose(hNote);
        return (error);
    }

    /* Initialize Style */
    CompoundTextInitStyle (&Style);

    /*  Define a default compound text style. */
    if (error = CompoundTextDefineStyle (hCompound, "", &Style, 
                                         &dwStyleID))
    {
        TraceNotesError(error,
                        "MainEntryPoint: unable to define Compound Text style.");
        CompoundTextDiscard (hCompound);
        NSFNoteClose(hNote);
        return (error);
    }
 
    /* Add text to the compound text context */

    szStartLogMessage = "Start of Service Request Tracking Log.";

    if (error = CompoundTextAddParagraphExt (
                                             hCompound,
                                             dwStyleID,
                                             DEFAULT_FONT_ID,
                                             szStartLogMessage,      /* text to append */
                                                /* text length: omit '\0' */
                                             (DWORD)( strlen(szStartLogMessage)-1 ),
                                             NULLHANDLE))            /* CLS translation table */
    {
        TraceNotesError(error,
                        "MainEntryPoint: unable to add paragraph to tracker log.");
        CompoundTextDiscard (hCompound);
        NSFNoteClose(hNote);
        return (error);
    }

    if (error = NSFNoteUpdate(hNote, UPDATE_FORCE))
    {
        TraceNotesError(error,
                        "MainEntryPoint: unable to update tracker log note.");
        NSFNoteClose(hNote);
        return (error);
    }

    /* Save the compound text context handle in the global variable
       hCompound_global. Do not close the context here. Functions like
       TrackerNoteOpen and TrackerNoteUpdate will add to the compound
       text context from time to time. TrackerTerm closes the context.
       Also save the compound text style ID and the open note handle
       in global variables.
     */
    hLogNote_global = hNote;
    hCompound_global = hCompound;
    dwStyleID_global = dwStyleID;

    return NOERROR;
}

/************************************************************************

    FUNCTION:   FinishLog

    PURPOSE:    Finish the Tracker log document

*************************************************************************/

VOID   LNPUBLIC  FinishLog (void)
{
    STATUS          error;
    NOTEHANDLE      hNote;
    DHANDLE         hCompound;      /* Compound Text context */
    DWORD           dwStyleID;
    TIMEDATE        tdFinishTime;

    hNote = hLogNote_global;
    hCompound = hCompound_global;
    dwStyleID = dwStyleID_global;

    AddLogEntry("End of Service Request Tracking Log.", "TrackerTerm");

    /* Close the compound text context. This adds the body field to the
       tracker log document.
     */
    if (error = CompoundTextClose (hCompound, 0, 0L, NULL, 0))
    {
        TraceNotesError(error,
                        "TrackerTerm: unable to append body to tracker log document.");
        CompoundTextDiscard(hCompound);
    }

    /* set the FinishTime field */
    OSCurrentTIMEDATE(&tdFinishTime);
    if (error = NSFItemSetTime(hNote, TRACKER_ITEM_FINISHTIME,
                               &tdFinishTime))
    {
        TraceNotesError(error,
                        "TrackerTerm: unable to set FinishTime field in tracker log.");
    }

    if (error = NSFNoteUpdate(hNote, UPDATE_FORCE))
    {
        TraceNotesError(error,
                        "TrackerTerm: unable to update tracker log note.");
    }

    if (error = NSFNoteClose(hNote))
    {
        TraceNotesError(error, "TrackerTerm: unable to close log note.");
    }

    return;
}

/************************************************************************

    FUNCTION:   AddLogEntry

    PURPOSE:    Add a line to the body of the Tracker log document.

*************************************************************************/

VOID  LNPUBLIC  AddLogEntry(char * szLine, char * szFunction)
{
    STATUS          error;
    DHANDLE         hCompound;
    DWORD           dwStyleID;
    char            szErrorMessage[MAX_NOTES_ERROR];

    hCompound = hCompound_global;
    dwStyleID = dwStyleID_global;

    if (error = CompoundTextAddParagraphExt (
                                             hCompound,
                                             dwStyleID,
                                             DEFAULT_FONT_ID,
                                             szLine,
                                             (DWORD)( strlen(szLine)-1 ),
                                             NULLHANDLE))
    {
        sprintf(szErrorMessage, "%s: unable to add line to tracker log.",
                szFunction);
        TraceNotesError(error, szErrorMessage);
    }
    return ;
}

/************************************************************************

    FUNCTION:   IsTargetDB  

    PURPOSE:    Check if given DB handle refers to the target database. 
                      
    ALGORITHM:  Get the canonical path of the target database and also
                the database specified by the given Db handle. If the
                canonical paths are identical, return TRUE.

    INPUTS:     hDB     handle to database to check  

    RETURN:     TRUE if hDB specifies target DB

*************************************************************************/

BOOL   LNPUBLIC  IsTargetDB( DBHANDLE hDB )
{
    STATUS      error;
    DBHANDLE    hTargetDB;
    char        szDBCanonicalPathName[MAXPATH];
    char        szTargetDBCanonicalPath[MAXPATH];

    hTargetDB = hDB_global;

    if (hDB == NULLHANDLE)
    {
        return FALSE;
    }
    if (error = NSFDbPathGet(hDB, szDBCanonicalPathName, NULL))
    {
        TraceNotesError(error, "IsTargetDB: Unable to get DB path.");
        return FALSE ;
    }
    if (error = NSFDbPathGet(hTargetDB, szTargetDBCanonicalPath, NULL))
    {
        TraceNotesError(error, "IsTargetDB: Unable to get target DB path.");
        return FALSE;
    }
    if (strcmp(szTargetDBCanonicalPath, szDBCanonicalPathName) != 0)
    {
        fprintf(pTraceFile[inst-1], "IsTargetDB: Given DB is not target.\n");
        return FALSE;
    }
    fprintf(pTraceFile[inst-1], "IsTargetDB: Given DB matches Target.\n");
    return TRUE;
}


/************************************************************************

    FUNCTION:   NoteIsServiceRequest

    PURPOSE:    Check if given note refers to a service request document

    INPUTS:     hNote   handle to open note to check

    RETURNS:    TRUE if hNote specifies a request doc, FALSE otherwise.

*************************************************************************/

BOOL   LNPUBLIC  NoteIsServiceRequest(NOTEHANDLE hNote)
{
    WORD        wClass;
    WORD        wFormNameLen;
    char        szFormName[DESIGN_FORM_MAX+1];


    NSFNoteGetInfo (hNote, _NOTE_CLASS, &wClass);
    if (wClass != NOTE_CLASS_DOCUMENT)
    {
        fprintf(pTraceFile[inst-1], "NoteIsServiceRequest: not NOTE_CLASS_DOCUMENT.\n");
        fprintf(pTraceFile[inst-1], "NoteIsServiceRequest: NOT a service request.\n");
        return FALSE;
    }

    /* Make sure FORM is "1 Assistance Request". */
    wFormNameLen = NSFItemGetText(hNote, FIELD_FORM,   /* "Form" */
                                szFormName, DESIGN_FORM_MAX);

    if (wFormNameLen==0)
    {
        fprintf(pTraceFile[inst-1], "NoteIsServiceRequest: no form name field.\n");
        fprintf(pTraceFile[inst-1], "NoteIsServiceRequest: NOT a service request.\n");
        return FALSE;
    }
        
    fprintf(pTraceFile[inst-1], "NoteIsServiceRequest: Form = '%s'.\n",
                           szFormName);

    if (strncmp(szFormName, REQUEST_FORM, wFormNameLen) == 0)
    {
        fprintf(pTraceFile[inst-1], "NoteIsServiceRequest: IS a service request.\n");
        return TRUE;
    }
    else
    {
        fprintf(pTraceFile[inst-1], "NoteIsServiceRequest: NOT a service request.\n");
        return FALSE;
    }
}



/************************************************************************

    FUNCTION:   PrintToTraceFile

************************************************************************/

VOID    LNPUBLIC PrintToTraceFile(
                                  char       *szFunction, 
                                  char       *UserName, 
                                  LIST       *GroupList, 
                                  DBHANDLE    hDB, 
                                  NOTEID      NoteID, 
                                  NOTEHANDLE  hNote,
                                  WORD        UpdateFlags)
{
    fprintf(pTraceFile[inst-1], "%s: UserName = '%s'.\n", szFunction, UserName);

    PrintListToTraceFile(GroupList, szFunction);

    PrintDBPathToTraceFile(hDB, szFunction);

    fprintf(pTraceFile[inst-1], "%s: Note ID = %#010lx.\n", szFunction, NoteID);

    fprintf(pTraceFile[inst-1], "%s: Note handle = %#x.\n", szFunction, hNote);

    fprintf(pTraceFile[inst-1], "%s: UpdateFlags = %#x.\n", szFunction, UpdateFlags);
    
    return;
}

/************************************************************************

    FUNCTION:   PrintListToTraceFile

************************************************************************/

VOID    LNPUBLIC PrintListToTraceFile (LIST * GroupList, char * szFunction)
{
    USHORT      usListEntries;          /* gets number of entries in list */
    USHORT      usEntryLen;             /* length of next entry in list */
    USHORT      usEntry;                /* used to control loop over list */
    char       *achEntry;               /* points to text of next entry */
    char        szGroupName[MAXUSERNAME+1]; /* copy one entry into here */

    if (GroupList == (LIST*)NULL)
    {
        return;
    }
    usListEntries = ListGetNumEntries(GroupList, FALSE);
    for (usEntry=0; usEntry < usListEntries; usEntry++)
    {
        ListGetText(GroupList, FALSE, usEntry, &achEntry, &usEntryLen);
        usEntryLen = (usEntryLen<MAXUSERNAME) ? usEntryLen : MAXUSERNAME;
        memcpy(szGroupName, achEntry, usEntryLen);
        szGroupName[usEntryLen] = '\0';
        fprintf(pTraceFile[inst-1], "%s: Group #%d = '%s'.\n",
                szFunction, usEntry+1, szGroupName);
    }

    return;
}

/************************************************************************

    FUNCTION:   PrintDBPathToTraceFile

************************************************************************/

VOID   LNPUBLIC PrintDBPathToTraceFile( DBHANDLE hDB, char * szFunction )
{
    STATUS      error;
    char        szCanonicalPathName[MAXPATH+1];
    char        szExpandedPathName[MAXPATH+1];
    char        szMessage[MAX_NOTES_ERROR];

    if (error = NSFDbPathGet(hDB, szCanonicalPathName, szExpandedPathName))
    {
        sprintf(szMessage, "%s: Unable to get DB path.", szFunction);
        TraceNotesError(error, szMessage);
    }
    else
    {
        fprintf(pTraceFile[inst-1], "%s: Canonical DB path name = '%s'.\n",
                        szFunction, szCanonicalPathName);
        fprintf(pTraceFile[inst-1], "%s: Expanded DB path name = '%s'.\n",
                        szFunction, szExpandedPathName);
    }
    return ;
}


/************************************************************************

    FUNCTION:   GetReqNumItem

    PURPOSE:    Get the value of the "ReqNumber" item from the specified
                data note. Return the number to *pReqNumber. 

************************************************************************/

STATUS  LNPUBLIC  GetReqNumItem( NOTEHANDLE hNote, 
                                 NUMBER * pReqNumber,
                                 char * szFunction )
{
    STATUS      error;
    BLOCKID     bhReqNumberItem;
    WORD        wReqNumberType;
    BLOCKID     bhReqNumberValue;
    DWORD       dwReqNumberValueLen;
    WORD       *pReqNumberValue;
    RANGE       Range;
    RANGE      *pRange;
    NUMBER      Number;
    NUMBER     *pNumber;

    if (error = NSFItemInfo( hNote, 
                             TRACKER_ITEM_REQ_NUMBER, /* "ReqNumber" */
                             (WORD)strlen(TRACKER_ITEM_REQ_NUMBER),
                             &bhReqNumberItem,
                             &wReqNumberType,
                             &bhReqNumberValue,
                             &dwReqNumberValueLen ))
    {
        fprintf(pTraceFile[inst-1], "%s: Unable to get ReqNumber item.\n", 
                szFunction);
        return(error);
    }
    
    if (wReqNumberType == TYPE_NUMBER)
    {
        if (FALSE == NSFItemGetNumber (hNote,
                        TRACKER_ITEM_REQ_NUMBER, pReqNumber))
        {
            fprintf(pTraceFile[inst-1], "%s: Unable to get ReqNumber item.\n",
                    szFunction);
            return (ERR_TRACKER_REQNUMITEM);
        }

        return NOERROR;
    }
    else if (wReqNumberType != TYPE_NUMBER_RANGE)
    {
        fprintf(pTraceFile[inst-1], "%s: Unable to get ReqNumber item.\n", 
                szFunction);
        return (ERR_TRACKER_REQNUMITEM);
    }

    pReqNumberValue = OSLockBlock( WORD, bhReqNumberValue );

    pRange = (RANGE*)(&pReqNumberValue[1]); /* step over data type word */
    
   /* pRange now points to an item of type TYPE_NUMBER_RANGE
      in host format. An item of TYPE_NUMBER_RANGE consists of
      a RANGE structure followed by an array of NUMBERs.
    */
    memcpy( (char*)(&Range), (char*)pRange, sizeof(RANGE) );

    if (Range.ListEntries == 0)
    {
        fprintf(pTraceFile[inst-1], "%s: Unable to get ReqNumber item.\n", 
                szFunction);
        OSUnlockBlock( bhReqNumberValue );    
        return (ERR_TRACKER_REQNUMITEM);
    }

    pNumber = (NUMBER*)(&pRange[1]);   /* step over RANGE structure */

   /* pNumber now points to the first NUMBER in the array. This
      NUMBER is in host format, but it may not be properly aligned.
      Move this NUMBER to a variable of type NUMBER to assure proper
      alignment of the data.
    */
    
    memcpy( (char*)(&Number), (char*)pNumber, sizeof(NUMBER) );

    *pReqNumber = Number;

    OSUnlockBlock( bhReqNumberValue );
    return NOERROR;
}

/************************************************************************

    FUNCTION:   GetReqNumber

    PURPOSE:    Get the "next Request Number to Use" from reqnum.dat
                file.  Return the number to *pReqNumber. Increment the
                number. Update file reqnum.dat with the incremented number.

************************************************************************/

STATUS  LNPUBLIC  GetReqNumber (NUMBER * pReqNumber)
{
    FILE       *pReqNumFile = NULL;
    NUMBER      ReqNumber;
    size_t      count;

    pReqNumFile = fopen(REQNUM_FILENAME, "r+b");

    if (pReqNumFile == (FILE*)NULL)
    {
        fprintf(pTraceFile[inst-1],"TrackerNoteOpen: unable to open file '%s': %d.\n",
                REQNUM_FILENAME, errno);
        return (ERR_TRACKER_REQNUMFILE);
    }

    count = fread(&ReqNumber, sizeof(NUMBER), 1, pReqNumFile);

    if (count != 1)
    {
        fclose(pReqNumFile);
        fprintf(pTraceFile[inst-1], "TrackerNoteOpen: error reading file '%s': %d.\n",
                REQNUM_FILENAME, errno);
        return (ERR_TRACKER_REQNUMFILE);
    }

    *pReqNumber = ReqNumber;
    ReqNumber++;

    fseek(pReqNumFile, 0L, SEEK_SET);

    count = fwrite(&ReqNumber, sizeof(NUMBER), 1, pReqNumFile);

    if (count != 1)
    {
        fclose(pReqNumFile);
        fprintf(pTraceFile[inst-1], "TrackerNoteOpen: error writing file '%s': %d.\n",
                REQNUM_FILENAME, errno);
        return (ERR_TRACKER_REQNUMFILE);
    }

    if (fclose(pReqNumFile))
    {
        fprintf(pTraceFile[inst-1], "TrackerNoteOpen: unable to close file '%s'.\n",
                            REQNUM_FILENAME);
    }

    return NOERROR;
}

/************************************************************************

    FUNCTION:   CopyNoteToTrashcan

************************************************************************/

STATUS  LNPUBLIC  CopyNoteToTrashcan (DBHANDLE hDB,
                                      NOTEID NoteID)
{
    STATUS          error;
    DBREPLICAINFO   TargetDBRepInfo;
    DBID            TargetDBID;
    char            szTrashcanName[MAXENVVALUE];
    DBHANDLE        hTrashcanDB;
    DBREPLICAINFO   TrashcanRepInfo;
    DBID            TrashcanDBID;

    if (error = NSFDbReplicaInfoGet (hDB, &TargetDBRepInfo))
    {
        TraceNotesError(error, 
            "TrackerNoteUpdate: unable to get replica info from Target DB.");
        return(error);
    }

    if (error = NSFDbIDGet (hDB, &TargetDBID))
    {
        TraceNotesError(error, 
            "TrackerNoteUpdate: unable to get DBID from Target DB.");
        return(error);
    }

    /* Get the name of the trashcan database */

    if (!OSGetEnvironmentString(TRACKER_TRASHCAN_VARIABLE,
                                szTrashcanName, MAXENVVALUE))
    {
        strcpy(szTrashcanName, TRACKER_DEFAULT_TRASHCAN);
    }

    if (error = NSFDbOpen (szTrashcanName, &hTrashcanDB))
    {
        TraceNotesError(error,
                        "TrackerNoteUpdate: unable to open trashcan DB.");
        return(error);
    }

    if (error = NSFDbReplicaInfoGet (hTrashcanDB, &TrashcanRepInfo))
    {
        TraceNotesError(error,
                        "TrackerNoteUpdate: unable to get trashcan DB replica info.");
        NSFDbClose(hTrashcanDB);
        return(error);
    }

    if (error = NSFDbIDGet (hTrashcanDB, &TrashcanDBID))
    {
        TraceNotesError(error, 
                        "TrackerNoteUpdate: unable to get trashcan DBID.");
        NSFDbClose(hTrashcanDB);
        return(error);
    }

    if (error = NSFDbCopyNote(hDB,      /* source database handle */
                              NULL,               /* source database DBID */
                              &(TargetDBRepInfo.ID),  /* source database Replica ID */
                              NoteID,             /* ID of note to copy */
                              hTrashcanDB,        /* destination database handle */
                              &TrashcanDBID,      /* destination database DBID */
                              &(TrashcanRepInfo.ID),  /* destination database Rep ID */
                              (NOTEID*)NULL,      /* ret detination NoteID */
                              (WORD*)NULL))       /* ret note class */
    {
        TraceNotesError(error, 
                        "TrackerNoteUpdate: unable to copy note to trashcan.\n");
    }
    NSFDbClose(hTrashcanDB);
    return(error);
}

/************************************************************************

    FUNCTION:   RenderDataAsText

    PURPOSE:    Convert item value to printable text. Allocate a buffer
                and return a pointer to the buffer containing the text.

    INPUTS:     Data  - pointer to item data, starting with data type word
                Length- length, in bytes, of data including data type word

    OUTPUTS:    pszItemText - The specified char pointer receives a pointer
                to an allocated memory area containing the text rendering
                of the item. Caller must free this memory after use.

************************************************************************/

STATUS  LNPUBLIC  RenderDataAsText(void *Data, WORD Length,
                                   char * * pszItemText)
{
    STATUS      error;
    WORD        wDataType;
    DWORD       dwValueLen;
    BLOCKID     bidValue;
    char       *szTextBuff;
    WORD        wTextLength;
    char        szErrorMessage[MAX_NOTES_ERROR];

    wDataType = *((WORD *)Data);
    dwValueLen = (DWORD)Length;

    /* Make a BLOCKID. Set up bidValue to input to NSFItemConvertToText */

    if (error = OSMemAlloc(0, dwValueLen, &(bidValue.pool)))
    {
        sprintf(szErrorMessage,
                "TrackerDbStampNotes: unable to allocate %ld bytes memory.\n",
                dwValueLen);
        TraceNotesError(error, szErrorMessage);
        return(error);
    }
    bidValue.block = (unsigned short)NULLHANDLE;
    memcpy( OSLockBlock(void, bidValue), Data, Length );
    OSUnlockBlock(bidValue);

    /* Temporarily allocate a very big memory block to receive the text */
    #define VERY_BIG_MALLOC ((size_t)64000)
    szTextBuff = (char *)malloc(VERY_BIG_MALLOC);

    /* Given a BLOCKID and a big text buffer, can convert value to text */
    wTextLength = NSFItemConvertValueToText (wDataType,
                                             bidValue, dwValueLen, szTextBuff, VERY_BIG_MALLOC, ',');

    /* Now allocate a return buffer just the size we really need. */
    *pszItemText = (char *)malloc(wTextLength);

    /* return the text string to the return buffer, then clean up */
    strcpy(*pszItemText, szTextBuff);
    free(szTextBuff);
    OSMemFree(bidValue.pool);

    return NOERROR;
}
/************************************************************************

    FUNCTION:   TraceNotesError

    PURPOSE:    Interpret Notes error status and log a message to the
                Tracker error trace file.

*************************************************************************/

VOID    LNPUBLIC  TraceNotesError( STATUS error, char * szMessage )
{
    char        szNotesError[MAX_NOTES_ERROR];

    OSLoadString(NULLHANDLE, ERR(error), szNotesError, MAX_NOTES_ERROR);

    fprintf(pTraceFile[inst-1], "%s\n%s", szMessage, szNotesError);

    return;
}

/************************************************************************

    FUNCTION:   GetInstanceNumber

    PURPOSE:    Get the next instance number from instance.dat
                file.  Return the number to *pInstNum. Increment the
                number. Update file instance.dat with the incremented number.

************************************************************************/

STATUS  LNPUBLIC  GetInstanceNumber (int *pInstNum)
{
    FILE       *pFile = NULL;
    int        InstNum=1;
    int        count;
    char       tmpnum[6];

    /* copy tracker log file name */
    strcpy(TrackerLogFile, TRACE_FILENAME);

    *pInstNum = 0;

    /* attempt to open the file */
    pFile = fopen(INSTANCE_FILENAME, "r+b");

    /* if we can't open the file create it */
    if (pFile == (FILE*)NULL)
    {
#if defined(OS400)
        pFile = fopen(INSTANCE_FILENAME, "wb ccsid=819");
#else        
        pFile = fopen(INSTANCE_FILENAME, "wb");
#endif

        if (pFile == (FILE*)NULL)
        {
            return errno;
        }

        /* write 1 to the file */
        count = fwrite(&InstNum, sizeof(int), 1, pFile);

        if (count != 1)
        {
            fclose(pFile);
            return (ERR_TRACKER_INSTANCEFILE);
        }
    }
    else /* read the current number than update it */
    {
        /* read the number in the file */
        count = fread(&InstNum, sizeof(int), 1, pFile);

        if (count != 1)
        {
            fclose(pFile);
            return (ERR_TRACKER_INSTANCEFILE);
        }

        /* increment the instance number */
        InstNum++;

        /* check to see if we have exceeded instances */
        if (InstNum > MAX_TRACKER_INSTANCES)
        {
            return (ERR_TRACKER_MAX_INSTANCES);
        }

        fseek(pFile, 0L, SEEK_SET);

        /* write the updated number to the file */
        count = fwrite(&InstNum, sizeof(int), 1, pFile);

        if (count != 1)
        {
            fclose(pFile);
            return (ERR_TRACKER_INSTANCEFILE);
        }
    }

    if (fclose(pFile))
    {
        return (ERR_TRACKER_INSTANCEFILE);
    }

    *pInstNum = InstNum;

    /* convert instance number to ascii */
    sprintf (tmpnum, "%d", InstNum);
    
    /* add part of number extension to file name */
    if (InstNum <= 9)
        strcat(TrackerLogFile,".00");
    else
        strcat(TrackerLogFile,".0");
    
    /* append number to log file name (ex. TRACKER.001 ) */
    strcat(TrackerLogFile,tmpnum);

    return NOERROR;
}

