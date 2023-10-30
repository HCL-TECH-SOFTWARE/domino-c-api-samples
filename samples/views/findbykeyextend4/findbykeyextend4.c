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
    PROGRAM:    findbykeyextend4

    FILE:       findbykeyextend4.c

    SYNTAX:     findbykeyextend4  <First Key Value>  <Second Key Value>

    DESCRIPTION:
   This program finds the documents in a view that have a primary key 
   (the first sorted column in the view) of TYPE_TEXT and a secondary 
   key (the second sorted column in the view) of TYPE_NUMBER.  For each 
   document that matches, we get its noteID.

   We don't actually read the data in the notes.  Reading a note, 
   given its noteID, is shown in another example.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* C include files */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>  

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <nsfdb.h>
#include <nif.h>
#include <osmem.h>
#include <miscerr.h>
#include <editods.h>
#include <osmisc.h>
#include "../../printLog.h"
#if !defined(UNIX)
#include <windows.h>
#endif

#define MALLOC_AMOUNT   6048
#define STRING_LENGTH   256

/* cleanup flag values */
#define DO_NOTHING               0x0000
#define CLOSE_DB                 0x0001
#define CLOSE_COLLECTION         0x0002
#define FREE_KEY1                0x0004
#define FREE_TRANSLATEDKEY       0x0008
#define FREE_KEY2                0x0010
#define FREE_PKEY                0x0011
#define FIND_SPECIFIC_MATCHES    0x00400000

typedef unsigned short ushort;

STATUS far PASCAL NIFFindByKeyCallBack (NIFFINDBYKEYCTX *Ctx);
STATUS GetNIFGetCollectionViewNoteIDAndDB ( HCOLLECTION );

/* Function prototypes for local functions */
 
void  ProcessArgs (int argc, char *argv[],
                               char *Key1, 
                               char *Key2, 
                               char *dbFileName,
                               char *ViewName, DWORD *numMatches);

/*
 * It is a callback function called by NIFFinffByKeyExtended4.
 * It is used to copy SummaryBuffer to hUserData which is 
 * allocated in main() function in NIFFINDBYKEYCTX.
 * The hUserData is later iterated to print the noteID, name & value
 * from the processed NSF file.
 */
STATUS far PASCAL NIFFindByKeyCallBack(NIFFINDBYKEYCTX *Ctx)
{
       STATUS stat = NOERROR;
       BYTE   *p = NULL;
       DWORD  dwCurSpot = Ctx->TotalDataInBuffer;

       if (Ctx->UserDataLen <= Ctx->TotalDataInBuffer + Ctx->wSizeOfChunk)
       {
          if (stat = OSMemoryReallocate(Ctx->hUserData, Ctx->UserDataLen + Ctx->wSizeOfChunk))
             return stat;
          Ctx->UserDataLen += Ctx->wSizeOfChunk;
       }
       p = (BYTE *)OSMemoryLock(Ctx->hUserData);
       p += dwCurSpot;
       memcpy(p, Ctx->SummaryBuffer, Ctx->wSizeOfChunk);
       OSMemoryUnlock(Ctx->hUserData);
       /* Use this for debug 
       PRINTLOG("\n Number of entries in the chunk[%d]", Ctx->EntriesThisChunk);
       */
       Ctx->TotalDataInBuffer += Ctx->wSizeOfChunk;
       return stat;
}

/*
 * This method is used to print the Item summary from the 
 * findKeyEx4.nsf file. Values are populated in NIFFinffByKeyExtended4
 * callback function.
 */
STATUS PrintSummaryValues(ITEM_TABLE *pITable)
{
       STATUS  stat = NOERROR;
       BYTE    *p, *pvalbuf;
       ushort  *pinamlength       = NULL;
       ushort  *pidatalength      = NULL;
       int     vlen               = 0;
       BYTE    tmp[STRING_LENGTH] = { 0 };
       p=pvalbuf=NULL;

       pinamlength = (ushort*)&pITable[1];
       pidatalength = pinamlength +1;  /* skip one word for ptr to item data lengths */ 
       pvalbuf = (BYTE*)&pinamlength[(pITable->Items) * 2];
       PRINTLOG(" Item Name:[%s]", (char*)pvalbuf);
       vlen = *pidatalength;
       memcpy(tmp, pvalbuf + (*pinamlength + sizeof(WORD)), (vlen - sizeof(WORD)));
	
       PRINTLOG(" Item Value:[%s]\n", (char*)tmp);
	
       return stat;
}


int main (int argc, char *argv[])
{

/* Local data declarations */

   STATUS                error = NOERROR;
   STATUS                returnCode = NOERROR;
   WORD                  cleanup = DO_NOTHING;
   char                  dbFilename[STRING_LENGTH] = { 0 }, ViewName[STRING_LENGTH] = { 0 };    
   DBHANDLE              hDb = NULLHANDLE;            
   NOTEID                ViewID; 
   HCOLLECTION           hCollection = NULLHCOLLECTION; 
   COLLECTIONPOSITION    posCollection; 
   DHANDLE               hBuffer = NULLHANDLE, rethBuffer = NULLHANDLE; 
   NOTEHANDLE            hNote = NULLHANDLE; 
   NOTEID                *pNoteID = NULL;
   DWORD                 NumNotesFound = 0;     
   DWORD                 NumNotesMatch = 0;     
   DWORD                 NoteCount = 0;     
   char                  *Key1 = NULL;               /* primary input key */
   char                  *TranslatedKey = NULL;      /* Translated string key */
   char                  *Key2 = NULL;               /* secondary input key */
   char                  *pTemp = NULL, *pKey = NULL;
   WORD                  Item1ValueLen = 0, Item2ValueLen = 0, signal_flag;
   WORD                  TranslatedKeyLen = 0;
   BOOL                  FirstTime = TRUE;    
   BYTE		         *tmp = NULL;
   ITEM_TABLE            Itemtbl = { 0 };
   ITEM_TABLE            *pITable = NULL;
   ITEM                  Item = { 0 };
   WORD                  Word = 0;
   double                Double = 0, dValue = 0;
   NIFFINDBYKEYCTX       ctx = { 0 };
   int                   i = 0;
   DWORD                 ReturnFlags = 0;

   hBuffer = rethBuffer = NULLHANDLE;
   memset( &ctx, sizeof(NIFFINDBYKEYCTX), 0 );

/* Initialize Domino and Notes */
 
   if ( error = NotesInitExtended( argc, argv ) )
   {
      PRINTLOG( "\n Unable to initialize Notes. Error Code[0x%04x]\n", error )
      return (1);
   }
/* allocating memory for input keys 
   Do NOT forget to free it later */

   Key1 = (char *) malloc( STRING_LENGTH ); 

   if ( Key1 == NULL ) 
   {
      PRINTLOG( "Error: Out of memory when allocating Key1.\n" );
      returnCode=1;
      goto EXIT1;
   }
   else
      cleanup |= FREE_KEY1;

   /* Allocate memory for transaltedKey used to store key */
   TranslatedKey = (char *) malloc( STRING_LENGTH ); 

   if ( TranslatedKey == NULL )
   {
      PRINTLOG("Error: Out of memory when allocating TranslatedKey.\n");
      returnCode=1;
      goto EXIT1;
   }
   else
      cleanup |= FREE_TRANSLATEDKEY;

   Key2 = (char *) malloc( STRING_LENGTH ); 

   if (Key2 == NULL)    
   {
      PRINTLOG( "Error: Out of memory when allocating Key2.\n" );
      returnCode=1;
      goto EXIT1;
   }
   else
      cleanup |= FREE_KEY2;

/* Processing input arguments */

   ProcessArgs( argc, argv, Key1, Key2, dbFilename, ViewName, &NumNotesMatch );

/* Open the database. */

   if ( error = NSFDbOpen( dbFilename, &hDb ) ) 
      goto EXIT1;     
   
   cleanup |= CLOSE_DB;

/* Get the note id of the view we want. */

   if ( error = NIFFindView( hDb, 
             ViewName, 
             &ViewID ) )  
      goto EXIT1;

   PRINTLOG( "\n View ID[%x]", ViewID );
/* Get the current collection using this view. */

   if ( error = NIFOpenCollection (
                      hDb,           /* handle of db with view */
                      hDb,           /* handle of db with data */
                      ViewID,        /* noteID  of the view */
                      0,             /* collection open flags */
                      NULLHANDLE,    /* handle to unread ID list (input and return) */
                      &hCollection,  /* collection handle (return) */
                      NULLHANDLE,    /* handle to open view note (return) */
                      NULL,          /* universal noteID  of view (return) */
                      NULLHANDLE,    /* handle to collapsed list (return) */
                      NULLHANDLE) )   /* handle to selected list (return) */
      goto EXIT1;

   cleanup |= CLOSE_COLLECTION;

   /* Translate the input key to LMBCS */
#ifndef OS400
   TranslatedKeyLen = OSTranslate (
                          OS_TRANSLATE_NATIVE_TO_LMBCS,
                          Key1,
                          (WORD) strlen (Key1),
                          TranslatedKey,
                          STRING_LENGTH);
#else
   strncpy( TranslatedKey, Key1, STRING_LENGTH );
   TranslatedKeyLen = strlen( TranslatedKey );
#endif

   Item1ValueLen = TranslatedKeyLen + sizeof(WORD);
   Item2ValueLen = sizeof(double) + sizeof(WORD);

   pKey = (char *) malloc(MALLOC_AMOUNT);

   if (pKey == NULL)    
   {
      PRINTLOG("Error: Out of memory.\n");
      returnCode=1;
      goto EXIT1;
   }
   else
      cleanup |= FREE_PKEY;

   pTemp = pKey;

   /* Allocate Item Table which stores 2 items. Its like header for Keys */
   Itemtbl.Length = (sizeof(Itemtbl) + 
         (2*(sizeof(Item))) + Item1ValueLen + Item2ValueLen);
   Itemtbl.Items = 2;
   memcpy (pTemp, &Itemtbl, sizeof(Itemtbl));
   pTemp += sizeof(Itemtbl);

   /* Set first Item value header */
   Item.NameLength = 0;
   Item.ValueLength = Item1ValueLen;
   memcpy (pTemp, &Item, sizeof(Item));
   pTemp += sizeof(Item);

   /* Set second Item value header */
   Item.NameLength = 0; 
   Item.ValueLength = Item2ValueLen;
   memcpy (pTemp, &Item, sizeof(Item));
   pTemp += sizeof(Item);

   /* Set First Item Data Type */
   Word = TYPE_TEXT;
   memcpy (pTemp, &Word, sizeof(Word));
   pTemp += sizeof(Word);

   /* Set First Item Data Value */
   memcpy (pTemp, TranslatedKey, TranslatedKeyLen);                
   pTemp += TranslatedKeyLen;

   /* Set Second Item Data Type */
   Word = TYPE_NUMBER;
   memcpy (pTemp, &Word, sizeof(Word));
   pTemp += sizeof(Word);

   /* Set Second Item Data Value */
   Double = atof(Key2);
   memcpy (pTemp, &Double, sizeof(Double));
   pTemp += sizeof(Double);

   ReturnFlags = (DWORD)( READ_MASK_NOTEID | READ_MASK_SUMMARY); 
/* Search through the collection for the notes whose sort 
   column values match the given search keys: */

   ctx.UserDataLen = 64*1024;
   if ( error = OSMemoryAllocate( (PKG_ADDIN+1)| MEM_SHARE| MEM_GROWABLE, ctx.UserDataLen, &ctx.hUserData) )
   {
      goto EXIT1;
   }

   ctx.TotalDataInBuffer=0;

   if ( ( error = GetNIFGetCollectionViewNoteIDAndDB( hCollection ) ) != NOERROR )
   {
       PRINTLOG ( "\nError getting view name and view id for collection.\n" );
       goto EXIT1;
   }

   error = NIFFindByKeyExtended4 (
                        hCollection,
                        pKey,                                                                                   /* refer to key   */
                        FIND_CASE_INSENSITIVE|FIND_AND_READ_MATCHES | FIND_SPECIFIC_MATCHES | FIND_FIRST_EQUAL, /* match rules */
                        ReturnFlags, NULL,
                        &posCollection,                                                  /* where match begins (return) */
                        &NumNotesMatch, NULL, NULL, NULL, NIFFindByKeyCallBack, &ctx, 1);/* how many match (return) */

       

   if ( ERR(error) == ERR_NOT_FOUND ) 
   {
      PRINTLOG ( "\nKey not found in the collection.\n" );
      error = NOERROR;
      goto EXIT1;
   }
   
   if ( error ) 
      goto EXIT1;

   /* Check User data is populated by NIFFindByKeyExtended4 */
   if ( ctx.hUserData ) 
   {
      NOTEID *idlist = NULL;

      idlist = (NOTEID*) OSMemoryLock( ctx.hUserData );
      PRINTLOG( "\n" );
      /* Loop to each note ID and print the item name and value */
      for ( int i = 0; i < NumNotesMatch; i++ ) 
      {
          pITable = (ITEM_TABLE*)((&idlist[i])+1); // skip note id
          PRINTLOG("Notes ID:[%lX]", idlist[i]);
          tmp = (BYTE*)idlist;
          tmp += (pITable->Length);
          idlist = (NOTEID*) tmp;
          if ((error = PrintSummaryValues(pITable)) != NOERROR)
          {
             PRINTLOG("\n Error in printing summary value.");
             break;
          }
      }
      OSMemoryUnlock(ctx.hUserData);
      OSMemoryFree(ctx.hUserData);
      ctx.hUserData=NULLHANDLE;
   }
   do
   {

        /* Read entries in the collection */

      if (error = NIFReadEntries (
                          hCollection,                /* handle to this collection */
                          &posCollection,             /* where to start in collection */
                          (WORD)(FirstTime ? NAVIGATE_CURRENT : NAVIGATE_NEXT),
                                                      /* order to use when skipping */
                          FirstTime ? 0L : 1L,        /* number to skip i */
                          NAVIGATE_NEXT,              /* order to use when reading */
                          NumNotesMatch - NoteCount,  /* max number to read */
                          READ_MASK_NOTEID,           /* info we want */
                          &hBuffer,                   /* handle to info (return) */
                          NULL,                       /* length of buffer (return) */
                          NULL,                       /* entries skipped (return) */
                          &NumNotesFound,             /* entries read (return) */
                          &signal_flag) )             /* signal and share warnings (return) */
      goto EXIT1;

      if ( hBuffer == NULLHANDLE )
      {
          PRINTLOG( "\nEmpty buffer returned by NIFReadEntries.\n" );
          goto EXIT1;
      }

      pNoteID = (NOTEID *)OSLockObject( hBuffer );

      PRINTLOG ("\n" );
      /* Print the NoteID to console */
      for ( i = 0; i < NumNotesFound; i++ )
            PRINTLOG("Note count is %lu. \t noteID  is: %lX\n",
                      ++NoteCount, pNoteID[i]);

      OSUnlockObject( hBuffer ); /* Unlocking the object buffer */

      OSMemFree( hBuffer ); /* Freeing the block hbuffer */

      if ( FirstTime )
           FirstTime = FALSE;

   } while ( signal_flag & SIGNAL_MORE_TO_DO );

   
   PRINTLOG( "\nNo. of Notes Match found[%d].\n", NumNotesMatch );

/* Clean Up start before execution */
EXIT1:
   /* Close User Data Handler */
   if ( ctx.hUserData != NULLHANDLE ) 
   {
       OSMemoryFree( ctx.hUserData );
       ctx.hUserData = NULLHANDLE;
   }

   /* Close Collection Handler */
   if ( cleanup & CLOSE_COLLECTION )
      NIFCloseCollection( hCollection );

   /* Close DB */
   if ( cleanup & CLOSE_DB )
      NSFDbClose( hDb );

   /* clean up Key1 */
   if ( cleanup & FREE_KEY1 )
      free(Key1);

   /* clean up TranslatedKey */
   if ( cleanup & FREE_TRANSLATEDKEY )
      free( TranslatedKey );

   /* clean up Key2 */
   if ( cleanup & FREE_KEY2 )
      free( Key2 );

   /* clean up pKey */
   if ( cleanup & FREE_PKEY )
      free( pKey );

   /* Print error if any */
   if ( error )
      PRINTERROR( error,"NIFReadEntries" );

   /* Close notes process */
   NotesTerm();

   if ( returnCode==NOERROR && error==NOERROR ) {
      PRINTLOG("\nProgram completed successfully.\n");
   }

   /* This is to handle pre notes init failed */
   if ( returnCode )
      return( returnCode );
   else
      return( error );
}

/************************************************************************
 *
 * FUNCTION: GetNIFGetCollectionViewNoteIDAndDB
 *
 * INPUTS:   hCollections - Handle to open collection
 *        
 * OUTPUTS:  STATUS - NOERROR on success
 *                    notes error on otherwise
 ************************************************************************/

STATUS GetNIFGetCollectionViewNoteIDAndDB ( HCOLLECTION hCollection )
{
   DBHANDLE       hDB = NULLHANDLE;
   NOTEID         viewID = 0;
   NOTEHANDLE     hNote = NULLHANDLE;
   STATUS         error = NOERROR;
   char           viewName[STRING_LENGTH] = { 0 };
   char           userName[STRING_LENGTH] = { 0 };

   if ( ( error = NIFGetCollectionViewNoteIDAndDB(hCollection, &viewID, &hDB) ) != NOERROR )
        goto CLOSE;

   if ( ( error = NSFNoteOpen( hDB, viewID, 0, &hNote) ) != NOERROR )
        goto CLOSE;

   /* If $TITLE is not present then its fine we proceed. */
   if ( !( NSFItemIsPresent( hNote, VIEW_TITLE_ITEM, sizeof(VIEW_TITLE_ITEM) ) ) )
   {
        NSFNoteClose(hNote);
        PRINTLOG( "\nView $TITLE field/item is not present." );
        goto CLOSE;
   }

   /* Get item and make sure it has a non zero value */
   if ( ( error = NSFItemGetText( hNote, VIEW_TITLE_ITEM, viewName, sizeof(viewName)-1) ) != NOERROR )
   {
        NSFNoteClose(hNote);
        goto CLOSE;
   }

   /* Close the note now that the VIEW_TITLE_ITEM was collected.*/
   error = NSFNoteClose( hNote );  

CLOSE:
   if ( error == NOERROR )
   {
       if ( NSFDbUserNameGet(hDB, userName, (WORD) sizeof(userName)-1 ) == NOERROR ) 
       {
           PRINTLOG( "\nUser Name [%s] ", userName );
       }
       if ( viewName[0] == '\0' )
       {
           PRINTLOG( "View ID[%1X].\n", viewID );
       }
       else
       {
           PRINTLOG( "View Name [%s] View ID[%1X].\n", viewName, viewID );
       }
   }
   return error;
}

/************************************************************************
 
    FUNCTION:   ProcessArgs
 
    INPUTS:     argc, argv - directly from the command line
 
    OUTPUTS:    Key1, Key1 - data from the command line or
                from what the user types when prompted.
 
*************************************************************************/
 
void ProcessArgs ( int argc, char *argv[],
                             char *Key1, char *Key2,
                             char *dbName, char *viewName, DWORD *numMatches )
{
    if (argc != 6)
    {
        printf("Enter the DB Name: ");
        fflush(stdout);
        fgets(dbName, STRING_LENGTH-1, stdin);

        printf("Enter the View Name: ");
        fflush(stdout);
        fgets(viewName, STRING_LENGTH-1, stdin);

        printf("Enter the first key: ");
        fflush(stdout);
        fgets(Key1, STRING_LENGTH-1, stdin);

        printf("\n");
        printf("Enter the second key: ");
        fflush(stdout);
        fgets(Key2, STRING_LENGTH-1, stdin);
	fflush(stdout);
        printf("Enter the No. of records: ");
        scanf("%d", numMatches);
 
    }
    else
    {
        strncpy(dbName, argv[1], STRING_LENGTH-1);
        strncpy(viewName, argv[2], STRING_LENGTH-1);
        strncpy(Key1, argv[3], STRING_LENGTH-1);
        strncpy(Key2, argv[4], STRING_LENGTH-1);
        *numMatches=atoi(argv[5]);
    } /* end if */
} /* ProcessArgs */

