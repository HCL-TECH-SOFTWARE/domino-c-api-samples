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

    PROGRAM:    response

    FILE:       response.c

    SYNTAX:     response  <ServerName> <NSF name>  "subject text"

    PURPOSE:    Shows how to create response documents in a database,
                how to read them from the database, and how to use NIFUpdateFilters.

    DESCRIPTION:
        Given an existing database and a text value for a subject field,
        this creates three notes in the database. The first is a Main
        document and has form name = "Document". The second is a response
        to the first, and has form name = "Response".  The third is a 
        response to the response and has form name = "ResponseToResponse".

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* C include files */

#include <stdio.h>
#include <string.h>

#if defined(OS400) || defined(AIX64)
#include <stdlib.h>
#endif

#if !defined(OS400) && !defined(MAC)
#include <malloc.h>
#endif

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <stdnames.h>
#include <nsf.h>
#include <misc.h>
#include <editods.h>
#include <oserr.h>
#include <osmisc.h>
#include <nsfdb.h>
#include <nif.h>
#include <textlist.h>
#include <osmem.h>
#include <miscerr.h>
#include <idtable.h>
#include <kfm.h>
#include <osfile.h>
#include "../../printLog.h"


/* Form Names */
char szFormName_Document[] = "Document";
char szFormName_Response[] = "Response";
char szFormName_ResponseToResponse[] = "ResponseToResponse";
char ViewName[] = "TestView";

/* Function prototypes */
STATUS LNPUBLIC AddNewNote(DBHANDLE db_handle, char *form_name,
                     char *text_value, NOTEID *note_id_ptr);

STATUS LNPUBLIC MakeNoteResponse (DBHANDLE db_handle, NOTEID main_nid,
                                         NOTEID resp_nid);
 
void  LNPUBLIC  ProcessArgs (int argc, char *argv[], char *server_name,
                               char *dbpath_name, char *subject); 

STATUS PrintSummary (BYTE *);

STATUS LNPUBLIC EnumProc(void far *, SEARCH_MATCH far *, ITEM_TABLE far *);

/* Global variables */
int docCount;

/************************************************************************

    FUNCTION:   Main

    PURPOSE:    Main HCL C API for Notes/Domino subroutine

    ALGORITHM:
    1)  Parse the command line into the separate variables and open the
        target database.
    2)  Call NSFSearch, NSFFormulaCompile to delete all previous documents from database.
    3)  Call AddNewNote(), a local routine, three times, to create three
        new documents to the database.
    4)  Call MakeNoteResponse(), a local routine, to make the second
        document a response to the first.
    5)  Call MakeNoteResponse() again, this time to make the third
        document a response to the second.
    6)  Call NIFFindView() to return the note ID of the specified view.
    7)  Call NSFDbGetUnreadNoteTable() to get the id table of unread documents.
    8)  Call NIFOpenCollection() to open the collection, make a change to 
        the unread list of id table, and update the collection using NIFUpdateFilters().
    9)  Call NIFReadEntries to read the documents from the collection and to print them.
   10)  Close the database and return.

*************************************************************************/
int main(int argc, char *argv[])
{
    DBHANDLE           db_handle = NULLHANDLE;
    NOTEID             note_id1,
                       note_id2,
                       note_id3;
    STATUS             error = NOERROR;
    char               *szDBpathName = NULL,
                       *szSubject = NULL,
		       *szSrvPath = NULL;
    char               szThePath[MAXPATH] = {0};
    char               szTheSubject[MAXSPRINTF] = {0};
    char               szRespSubject[MAXSPRINTF] = "A response to: ";
    char               szRespRespSubject[MAXSPRINTF] = "Another response to: ";
 
    NOTEID             ViewID;                       /* Note ID of the view. */
    HCOLLECTION        hCollection = NULLHANDLE;     /* Collection handle. */
    COLLECTIONPOSITION CollPosition;                 /* Index into collection. */
    DHANDLE            hBuffer = NULLHANDLE,         /* Handle to buffer of info. */
                       hUnreadList = NULLHANDLE;     /* Handle to the unread list id table. */
    BYTE               *pBuffer = NULL;              /* Pointer into info buffer. */
    BYTE               *pSummary = NULL;             /* Pointer into info buffer. */
    NOTEID             EntryID;                      /* A collection entry ID. */
    DWORD              dwEntriesFound = 0;           /* Number of entries found. */
    ITEM_VALUE_TABLE   ItemTable;                    /* Table in pSummary buffer. */
    WORD               wSignalFlag = 0;              /* Signal and share warning flags. */
    DWORD              dwIndex = 0,                  /* A counter. */
                       dwUnreadCount = 0;            /* A count of unread documents in id table. */
    char               szFormula[] = "@All";         /* An ASCII selection formula. */
    FORMULAHANDLE      formula_handle = NULLHANDLE;  /* A compiled selection formula. */
    WORD               wIgnore = 0;                  /* A word we don't care about. */
    char               szUserName [MAXUSERNAME + 1] = "CN=Test User/O=Acme";	/* Domino Administrator user name */
    char               szServerName [MAXPATH] = {0};
    char               szFullPath[MAXPATH] = {0};
    NOTEID             noteid;
    BOOL               bFirst = TRUE;
    BOOL               bRetDeleted = FALSE;

    if (error = NotesInitExtended (argc, argv))
    {
        PRINTERROR (error,"NotesInitExtended");
        return (1);
    }


    /* Parse the command line into the separate variables and open the
       target database.
    */

    szDBpathName = szThePath;
    szSubject = szTheSubject;
    szSrvPath = szServerName;
    ProcessArgs(argc, argv, szSrvPath, szDBpathName, szSubject);

    /* To get current user name. */
    if (error = SECKFMGetUserName (szUserName))
    {
        PRINTERROR (error,"SECKFMGetUserName");  
        NotesTerm();
        return (1);
    }

    if (error = OSPathNetConstruct (NULL, szSrvPath, szDBpathName, szFullPath))
    {
        PRINTERROR (error,"OSPathNetConstruct");
        NotesTerm();
        return (1);
    }

    /* Open the database. */
    if (error = NSFDbOpen (szFullPath, &db_handle))
    {
        PRINTERROR (error,"NSFDbOpen");  
        NotesTerm();
        return (1);
    }

    error = NSFFormulaCompile (
                               NULL,               
                               (WORD) 0,           
                               szFormula,
                               (WORD) strlen(szFormula),
                               &formula_handle,
                               &wIgnore,
                               &wIgnore,
                               &wIgnore, &wIgnore, &wIgnore, &wIgnore); 
    if ( error != NOERROR )
    {
        PRINTERROR(error,"NSFFormulaCompile");
        goto exit;
    }
    /* To Delete the Documents in nsf file. */
    error = NSFSearch (
                       db_handle,      
                       formula_handle, 
                       NULL,           
                       0,              
                       NOTE_CLASS_DOCUMENT,
                       NULL,         
                       EnumProc, 
                       &db_handle, 
                       NULL);
    if ( error != NOERROR ) 
    {
        PRINTERROR(error,"NSFSearch");
        goto exit;
    }

    OSMemFree(formula_handle);
    /*  
    Call AddNewNote(), a local routine, to add the first document, the
    response note and the response to the response. This section only 
    adds the notes; the relationship of one note to another is created 
    later by MakeNoteResponse(). Each call to AddNewNote() yields the
    noteID of the note it creates.
    */   

    /* Create Main note. */
    if (error = AddNewNote (db_handle, szFormName_Document, szSubject, &note_id1))
    {
        PRINTERROR (error,"AddNewNote");
        goto exit;
    }

    /* Create Another note - soon to be a response note. */

    strcpy(&szRespSubject[strlen(szRespSubject)], szSubject);

    if (error = AddNewNote (db_handle, szFormName_Response, szRespSubject, 
                            &note_id2))
    {
        PRINTERROR (error,"AddNewNote");
        goto exit;
    }

    /* Create Another note - soon to be a response to a response. */

    strcpy(&szRespRespSubject[strlen(szRespRespSubject)],
            szRespSubject);

    if (error = AddNewNote (db_handle, szFormName_ResponseToResponse,
                              szRespRespSubject,&note_id3))
    {
        PRINTERROR (error,"AddNewNote");
        goto exit;
    }

    /* Call MakeNoteResponse() to make the note identified by note_id2 
       a response to the note identified by note_id1. MakeNoteResponse()
       is a local routine that creates a response reference list item and
       appends this item to the response note. This item establishes the
       relationship of the response note to the main note.
    */

    if (error = MakeNoteResponse (db_handle, note_id1, note_id2))
    {
        PRINTERROR (error,"MakeNoteResponse");  
        goto exit;
    }

    /*  Make the third note a response note to the second. */
    if (error = MakeNoteResponse (db_handle, note_id2, note_id3))
    {
        PRINTERROR (error,"MakeNoteResponse");
        goto exit;
    }

    /* Get the note id of the view we want. */

    if (error = NIFFindView (
                              db_handle, 
                              ViewName, 
                              &ViewID))
    {
        PRINTERROR (error,"NIFFindView");  
        goto exit;
    }

    PRINTLOG("Testing NIFUpdateFilter:\n");
    PRINTLOG("------------------------\n");
    error = NSFDbGetUnreadNoteTable (
                                     db_handle,                     /* handle of the database */
                                     szUserName,                    /* Domino Administrator user name*/
                                     strlen (szUserName),           /* length of user name */
                                     FALSE,                         /* returns only unread documents else NULL */
                                     &hUnreadList);                 /* id table handle */

    if ( error != NOERROR ) 
    {
        PRINTERROR(error,"NSFDbGetUnreadNoteTable");
        goto exit;
    }

    /* Updates the unread id table. */

    if (error = NSFDbUpdateUnread (db_handle, hUnreadList))
    {
        PRINTERROR (error,"NSFDbUpdateUnread");
        goto exit;
    }

    /* Get the current collection using this view. */

    if (error = NIFOpenCollection(
                                  db_handle,            /* handle of db with view */
                                  db_handle,            /* handle of db with data */
                                  ViewID,         /* note id of the view */
                                  0,              /* collection open flags */
                                  hUnreadList,    /* handle to unread ID list (input and return) */
                                  &hCollection,   /* collection handle (return) */
                                  NULLHANDLE,     /* handle to open view note (return) */
                                  NULL,           /* universal note id of view (return) */
                                  NULLHANDLE,     /* handle to collapsed list (return) */
                                  NULLHANDLE))    /* handle to selected list (return) */
    {
        PRINTERROR (error,"NIFOpenCollection");
        goto exit;
    }

    /* Number of entries of the id table handle. */
    dwUnreadCount = IDEntries(hUnreadList);
    PRINTLOG("Total unread documents in the database: %d.\n", dwUnreadCount);

    while(IDScan(hUnreadList,bFirst, &noteid))
    {
        bFirst = FALSE;
    }

    if (dwUnreadCount != 0)
    {
        /* Deletes an entry in the id table. */
        if (error = IDDelete (hUnreadList, noteid, &bRetDeleted))
        {
            NIFCloseCollection(hCollection);
            PRINTERROR (error,"IDDelete");
            goto exit;
        }
        PRINTLOG("Deleted an entry in the id table.\n");
        /* Updates the unread filter. */
        if (error = NIFUpdateFilters (hCollection, FILTER_UNREAD))
        {
            NIFCloseCollection(hCollection);
            PRINTERROR (error,"NIFUpdateFilters");
            goto exit;
        }

        dwUnreadCount = IDEntries(hUnreadList);
        PRINTLOG("Total unread documents after using NIFUpdateFilters: %d.\n", dwUnreadCount);
        PRINTLOG("\nNIFUpdateFilters was successful.\n");
    }

    /* Set a COLLECTIONPOSITION to the beginning of the collection. */

    CollPosition.Level = 0;
    CollPosition.Tumbler[0] = 0;

    /* Get the note ID and summary of every entry in the collection. In the 
    returned buffer, first comes all of the info about the first entry, then 
    all of the info about the 2nd entry, etc. For each entry, the info is
    arranged in the order of the bits in the READ_MASKs. */

    PRINTLOG("\nResponse documents:\n");
    PRINTLOG("--------------");
    do
    {
        if (error = NIFReadEntries(
                                    hCollection,        /* handle to this collection */
                                    &CollPosition,      /* where to start in collection */
                                    NAVIGATE_NEXT,      /* order to use when skipping */
                                    1L,                 /* number to skip */
                                    NAVIGATE_NEXT,      /* order to use when reading */
                                    0xFFFFFFFF,         /* max number to read */
                                    READ_MASK_NOTEID +  /* info we want */
                                    READ_MASK_SUMMARYVALUES,
                                    &hBuffer,           /* handle to info buffer (return)  */
                                    NULL,               /* length of info buffer (return) */
                                    NULL,               /* entries skipped (return) */
                                    &dwEntriesFound,    /* entries read (return) */
                                    &wSignalFlag))      /* share warning and more signal flag
                                    (return) */
        {
            NIFCloseCollection (hCollection);
            PRINTERROR (error,"NIFReadEntries");
            goto exit;
        }
  
        /* Check to make sure there was a buffer of information returned. */ 

        if (hBuffer == NULLHANDLE)
        {
            NIFCloseCollection (hCollection);
            PRINTLOG ("\nEmpty buffer returned by NIFReadEntries.\n");
            goto exit; 
        }

        /* Lock down (freeze the location) of the information buffer. Cast
        the resulting pointer to the type we need. */

        pBuffer = (BYTE *) OSLockObject (hBuffer);

        /* Start a loop that extracts the info about each collection entry from
        the information buffer. */

        PRINTLOG ("\n");
        for (dwIndex = 1; dwIndex <= dwEntriesFound; dwIndex++)
        {

            /* Get the ID of this entry. */

            memcpy (&EntryID, pBuffer, sizeof(EntryID));

            /* Advance the pointer over the note id. */

            pBuffer += sizeof (EntryID);

            /* Get the header for the summary items for this entry. */

            memcpy (&ItemTable, pBuffer, sizeof(ItemTable));

            /* Remember where the start of this entry's summary is. Then advance
            the main pointer over the summary. */
      
            pSummary = pBuffer;
            pBuffer += ItemTable.Length; 
 
            /* If this entry is a category, say so. */

            if (NOTEID_CATEGORY & EntryID)
                PRINTLOG ("CATEGORY: ");

            /* Call a local function to print the summary buffer. */

            if (error = PrintSummary (pSummary))
            {
                OSUnlockObject (hBuffer);
                OSMemFree (hBuffer);
                NIFCloseCollection (hCollection);
                PRINTERROR (error,"PrintSummary");
                goto exit;
            }

        /* End of loop that gets info about each entry. */

        }

        /* Unlock the list of note IDs. */

        OSUnlockObject (hBuffer);

        /* Free the memory allocated by NIFReadEntries. */

        OSMemFree (hBuffer);

    /* Loop if the end of the collection has not been reached because the buffer was full.  */

    }  while (wSignalFlag & SIGNAL_MORE_TO_DO);

    IDDestroyTable(hUnreadList);

    /* Close the collection. */

    if (error = NIFCloseCollection (hCollection))
    {
        PRINTERROR (error,"NIFCloseCollection");
        goto exit;
    }

exit:

    /* Close the database */
    if (db_handle)
        NSFDbClose(db_handle);
    NotesTerm();
    if (error == NOERROR)
    {
        PRINTLOG("\nProgram completed successfully.\n");
        return (0);
    }
    else
    {
        PRINTLOG("\nProgram completed with errors.\n");
        return 1;
    }
}

/************************************************************************

    FUNCTION:   AddNewNote

    PURPOSE:    Creates a new note in the database and return the Note ID.

    INPUTS:     takes a database handle, form name, and subject value.

    OUTPUTS:    Returns the Note ID of the new note to *note_id_ptr.

*************************************************************************/

STATUS LNPUBLIC AddNewNote(DBHANDLE db_handle, char *form_name,
                             char *text_value, NOTEID *note_id_ptr)
{
    NOTEHANDLE note_handle;
    STATUS     error = NOERROR;
    char       text_item_name[] = "Subject";

    /* Create a new note and get a handle to it */
    if (error = NSFNoteCreate (db_handle, &note_handle))
    {
        return(ERR(error));
    }

    /* Append the form name item to the new note */
    if (error = NSFItemSetText( note_handle,
                                FIELD_FORM,
                                form_name,
                                MAXWORD))
    {
        NSFNoteClose (note_handle);
        return(ERR(error));
    }

    /* Append the subject field to the new note */
    if (error = NSFItemSetText (note_handle, 
                                text_item_name,
                                text_value,
                                MAXWORD))
    {
        NSFNoteClose (note_handle);
        return(ERR(error));
    }

    /* Update the new note to disk. Then get the note ID of the new note.
       You must update before you get the note ID because the note ID 
       of a newly created note is zero until the first time it is updated 
       to disk.
    */
    if (error = NSFNoteUpdate (note_handle, 0))
    {
        NSFNoteClose (note_handle);
        return(ERR(error));
    }

    NSFNoteGetInfo (note_handle, _NOTE_ID, note_id_ptr);

    /* Close the new note */
    error = NSFNoteClose (note_handle);

    return(ERR(error));

}

/************************************************************************

    FUNCTION:   MakeNoteResponse

    PURPOSE:    Make the second note a response to the first

    INPUTS:
        db_handle  - handle to the open database containing both notes
        main_nid   - handle of note to be the main (parent) document
        resp_nid   - handle of note to be the response (child) document

    ALGORITHM:

    This function obtains the OID of the first note, and builds a 
    response reference list. A response reference list consists of a 
    LIST structure followed by the UNID of the main (parent) document.
    It appends this response reference list to the second note as a 
    field with name $REF and data type TYPE_NOTEREF_LIST. This $REF 
    field makes the second note into a response to the first note.

*************************************************************************/

STATUS LNPUBLIC MakeNoteResponse (DBHANDLE db_handle,
                                         NOTEID main_nid,
                                         NOTEID resp_nid)
{
    STATUS     error;
    OID        main_oid;
    TIMEDATE   lastmod_td,retAddedToFile;
    WORD       note_class,retResponseCount;
    LIST       ListHdr;
    UNID       NoteUNID;
    NOTEID     retParentNoteID;	
    NOTEHANDLE note_handle;
               /*  block of memory to hold the response reference list */
    char       *buf;

    buf = (char *) malloc(sizeof(LIST) + sizeof(UNID));
 
    if (buf == NULL)
    {
        PRINTLOG ("malloc failed\n");
        return(ERR_MEMORY);
    }

    /* Get the OID (the ORIGINATORID) of the main (parent) document.
       NSFDbGetNoteInfo also yields the last modified time date and the 
       note class, but we do not need these values.
    */       

    if (error = NSFDbGetNoteInfoExt(db_handle,main_nid,&main_oid,&lastmod_td,&note_class,
                                &retAddedToFile,&retResponseCount,&retParentNoteID))
    {

        return(ERR(error));

    }

    
    /* Initialize the LIST header part of the response reference list */
    ListHdr.ListEntries = (USHORT) 1;

    /* Initialize the UNID part of the response reference list */
    NoteUNID.File = main_oid.File; /* user-unique identifier */
    NoteUNID.Note = main_oid.Note; /* time/date when the note was created */

    /* Pack the LIST and the UNID members of the Noteref list into
       a memory block.
    */
    memcpy(buf, (char*)&ListHdr, sizeof(LIST));
    memcpy((buf+sizeof(LIST)), (char*)&NoteUNID, sizeof(UNID));

    /* Open the second note - the one to make into a response document. */
    if (error = NSFNoteOpen(db_handle, resp_nid, 0, &note_handle))
    {
        return(ERR(error));
    }

    /* Append the completed response reference list to the second note 
       as an item of type TYPE_NOTEREF_LIST.
    */
    if (error = NSFItemAppend ( note_handle, 
                                ITEM_SUMMARY,
                                FIELD_LINK,         /* $REF */
                                (WORD) strlen(FIELD_LINK),      
                                TYPE_NOTEREF_LIST,  /* data type */
                                buf,                /* populated RESPONSE */
                                (DWORD)(sizeof(LIST) + sizeof(UNID)) ))
    {
        NSFNoteClose(note_handle);
        return(ERR(error));
    }

    free(buf);

    /* Update the note  */
    if (error = NSFNoteUpdate(note_handle, 0))
    {
        NSFNoteClose(note_handle);
        return(ERR(error));
    }

    error = NSFNoteClose (note_handle);

    return(ERR(error));
}


/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:     argc, argv - directly from the command line
   
    OUTPUTS:    db_name -     database name obtained from command line or
                              from prompt.
                subject -     subject obtained from command line or
                              from prompt.
                server_name - server name obtained from command line or prompt.

*************************************************************************/

void  LNPUBLIC  ProcessArgs (int argc, char *argv[], char *server_name,
                               char *db_name, char *subject)
{ 
    if (argc != 4)
    {
        printf("Enter Server name: ");
        fflush (stdout);
        gets(server_name);
        printf("\n");
        printf("Enter database name: ");
        fflush (stdout);
        gets(db_name);
        printf("\n");
        printf ("Enter subject text:  ");
        fflush (stdout);
        gets(subject);
    }    
    else
    {
        strcpy(server_name, argv[1]);
        strcpy(db_name, argv[2]);
        strcpy(subject, argv[3]);
    } /* end if */
} /* ProcessArgs */



STATUS PrintSummary (BYTE *pSummary)

/* This function prints the items in the summary for one
entry in a collection.*/

{

/* Local constants */

#define  MAX_ITEMS          20
#define  MAX_ITEM_LEN       100
#define  DATATYPE_SIZE      sizeof(USHORT)
#define  ITEM_LENGTH_SIZE   sizeof(USHORT)


/* Local variables */

   BYTE *pSummaryPos;              /* current position in pSummary */
   ITEM_VALUE_TABLE ItemTable;           /* header at start of pSummary */
   USHORT  ItemCount;              /* number of items in pSummary */
   USHORT  ItemLength[MAX_ITEMS];  /* length of each item */
   USHORT  DataType;               /* type of pSummary item */
   char ItemText[MAX_ITEM_LEN];    /* text of a pSummary item */
   STATUS  error;                  /* return code from API calls */
   USHORT  i;                      /* a counter */


/* Start reading the summary at its beginning. */

   pSummaryPos = pSummary;

/* Get the header at the beginning of the pSummary buffer. */

   memcpy (&ItemTable, pSummaryPos, sizeof(ItemTable));

/* Advance the buffer pointer over the header. */

   pSummaryPos += sizeof(ItemTable);

/* Get the number of items in the summary. */

   ItemCount = ItemTable.Items;

   if (ItemCount > MAX_ITEMS)
   {
      PRINTLOG (
         "Summary contains %d items - only printing the first %d items\n",
         ItemCount, MAX_ITEMS);
      ItemCount = MAX_ITEMS;
   }

/* Get the length of each item in the summary. */

   for (i=0; i < ItemCount; i++)
   {
      memcpy (&ItemLength[i], pSummaryPos, ITEM_LENGTH_SIZE);
      pSummaryPos += ITEM_LENGTH_SIZE;
   }

/* Start a loop that extracts each item in the summary. */

   for (i=0; i < ItemCount; i++)
   {

/* If an item has zero length it indicates an "empty" item in the
summary. This might occur in a lower-level category and stand for a
higher-level category that has already appeared. Or an empty item might
be a field that is missing in a response doc. Just print * as a place
holder and go on to the next item in the pSummary. */

      if (ItemLength[i] == 0)
      {
         continue;
      }
      else if (ItemLength[i] >= MAX_ITEM_LEN)
      {
            /* Item is too long for the buffer - just truncate it */
         ItemLength[i] = MAX_ITEM_LEN - 1;
      }

/* Get the data type of this item. */

      memcpy (&DataType, pSummaryPos, DATATYPE_SIZE);
      pSummaryPos += DATATYPE_SIZE;
      memcpy (ItemText,
                    pSummaryPos,
                    ItemLength[i] - DATATYPE_SIZE);
            ItemText[ItemLength[i] - DATATYPE_SIZE] = '\0';
             PRINTLOG ("%s    ", ItemText);

/* Advance to next item in the pSummary. */

       pSummaryPos += (ItemLength[i] - DATATYPE_SIZE);

/* End of loop that is extracting each item in the pSummary. */

   }

/* Print final line feed to end this pSummary display. */

   PRINTLOG ("\n");

/* End of function */

   return (NOERROR);

}

/************************************************************************

    FUNCTION:   EnumProc

    PURPOSE:    Called by NSFSearch for each note that matches the 
                selection criteria

    INPUTS:

    RETURNS:   
        NOERROR or STATUS if an error condition is encountered.

    DISCUSSION:

 *************************************************************************/
STATUS  LNPUBLIC  EnumProc(void far *phDB, SEARCH_MATCH far *pSearchMatch, ITEM_TABLE far *summary_info)
{
    SEARCH_MATCH SearchMatch;
    STATUS       error;

    memcpy( (char*)&SearchMatch, (char*)pSearchMatch, sizeof(SEARCH_MATCH) );

    if (!(SearchMatch.SERetFlags & SE_FMATCH))
        return (NOERROR);

    fflush(stdout);

    /* Delete Note */
    error = NSFNoteDelete(*(DBHANDLE far *)phDB, SearchMatch.ID.NoteID, 0);
    
    if (error)
      return(error);

    docCount++;
/* End of subroutine. */

    return (NOERROR);
}
