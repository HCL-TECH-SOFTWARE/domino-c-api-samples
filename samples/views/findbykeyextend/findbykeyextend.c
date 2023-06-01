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

    PROGRAM:    twokey

    FILE:       twokey.c

    SYNTAX:     twokey  <First Key Value>  <Second Key Value>

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

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif


#define MALLOC_AMOUNT   6048
#define STRING_LENGTH   256

/* cleanup flag values */
#define DO_NOTHING      0x0000
#define CLOSE_DB        0x0001
#define CLOSE_COLLECTION         0x0002
#define FREE_KEY1       0x0004
#define FREE_TRANSLATEDKEY       0x0008
#define FREE_KEY2       0x0010
#define FREE_PKEY       0x0011


/* Function prototypes for local functions */
 
void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                               char *Key1, 
                               char *Key2, 
                               char *dbFileName,
                               char *ViewName);
void PrintAPIError (STATUS);


int main (int argc, char *argv[])
{

/* Local data declarations */

   STATUS         error=NOERROR;
   STATUS         returnCode=NOERROR;
   WORD           cleanup=DO_NOTHING;
   char           dbFilename[STRING_LENGTH], ViewName[STRING_LENGTH];    
   DBHANDLE       hDb;            
   NOTEID         ViewID;         
   HCOLLECTION    hCollection; 
   COLLECTIONPOSITION    posCollection;   
   DHANDLE          hBuffer;        
   NOTEID        *pNoteID;        
   DWORD          NumNotesFound=0;     
   DWORD          NumNotesMatch=0;     
   DWORD          NoteCount = 0;     
   DWORD          i;     
   char          *Key1;               /* primary input key */
   char          *TranslatedKey;      /* Translated string key */
   char          *Key2;               /* secondary input key */
   char          *pTemp, *pKey;
   WORD           Item1ValueLen, Item2ValueLen, signal_flag;
   WORD           TranslatedKeyLen;
   BOOL           FirstTime = TRUE;    

   ITEM_TABLE     Itemtbl;
   ITEM           Item;
   WORD           Word;
   double         Double, dValue=0;

/* Initialize Domino and Notes */
 
   if (error = NotesInitExtended(argc, argv))
   {
      printf("\n Unable to initialize Notes. Error Code[0x%04x]\n", error);
      return (1);
   }

/* allocating memory for input keys 
   Do NOT forget to free it later */

   Key1 = (char *) malloc(STRING_LENGTH); 

   if (Key1 == NULL) 
   {
      printf("Error: Out of memory when allocating Key1.\n");
      returnCode=1;
      goto exit1;
   }
   else
      cleanup |= FREE_KEY1;

   TranslatedKey = (char *) malloc(STRING_LENGTH); 

   if (TranslatedKey == NULL)    
   {
      printf("Error: Out of memory when allocating TranslatedKey.\n");
      returnCode=1;
      goto exit1;
   }
   else
      cleanup |= FREE_TRANSLATEDKEY;

   Key2 = (char *) malloc(STRING_LENGTH); 

   if (Key2 == NULL)    
   {
      printf("Error: Out of memory when allocating Key2.\n");
      returnCode=1;
      goto exit1;
   }
   else
      cleanup |= FREE_KEY2;

/* Processing input arguments */

   ProcessArgs(argc, argv, Key1, Key2, dbFilename, ViewName);

 
/* Open the database. */

   if (error = NSFDbOpen (dbFilename, &hDb))  
      goto exit1;     

   cleanup |= CLOSE_DB;

/* Get the note id of the view we want. */

   if (error = NIFFindView (hDb, 
             ViewName, 
             &ViewID))  
      goto exit1;

/* Get the current collection using this view. */

   if (error = NIFOpenCollection(
    hDb,           /* handle of db with view */
    hDb,           /* handle of db with data */
    ViewID,        /* noteID  of the view */
    0,             /* collection open flags */
    NULLHANDLE,    /* handle to unread ID list (input and return) */
    &hCollection,  /* collection handle (return) */
    NULLHANDLE,    /* handle to open view note (return) */
    NULL,          /* universal noteID  of view (return) */
    NULLHANDLE,    /* handle to collapsed list (return) */
    NULLHANDLE))   /* handle to selected list (return) */
      goto exit1;

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
   strcpy(TranslatedKey, Key1);
   TranslatedKeyLen = strlen(TranslatedKey);
#endif

   Item1ValueLen = TranslatedKeyLen + sizeof(WORD);
   Item2ValueLen = sizeof(double) + sizeof(WORD);

   pKey = (char *) malloc(MALLOC_AMOUNT);

   if (pKey == NULL)    
   {
      printf("Error: Out of memory.\n");
      returnCode=1;
      goto exit1;
   }
   else
      cleanup |= FREE_PKEY;

   pTemp = pKey;

   Itemtbl.Length = (   sizeof(Itemtbl) + 
         (2*(sizeof(Item))) + Item1ValueLen + Item2ValueLen);
   Itemtbl.Items = 2;
   memcpy (pTemp, &Itemtbl, sizeof(Itemtbl));
   pTemp += sizeof(Itemtbl);

   Item.NameLength = 0;
   Item.ValueLength = Item1ValueLen;
   memcpy (pTemp, &Item, sizeof(Item));
   pTemp += sizeof(Item);

   Item.NameLength = 0;  
   Item.ValueLength = Item2ValueLen;
   memcpy (pTemp, &Item, sizeof(Item));
   pTemp += sizeof(Item);

   Word = TYPE_TEXT;
   memcpy (pTemp, &Word, sizeof(Word));
   pTemp += sizeof(Word);

   memcpy (pTemp, TranslatedKey, TranslatedKeyLen);                
   pTemp += TranslatedKeyLen;

   Word = TYPE_NUMBER;
   memcpy (pTemp, &Word, sizeof(Word));
   pTemp += sizeof(Word);

   Double = atof(Key2);
   memcpy (pTemp, &Double, sizeof(Double));
   pTemp += sizeof(Double);

/* Search through the collection for the notes whose sort 
   column values match the given search keys: */

   error = NIFFindByKey(
       hCollection,
       pKey,          /* refer to key   */
       FIND_CASE_INSENSITIVE,     /* match rules */
       &posCollection, /* where match begins (return) */
       &NumNotesMatch);/* how many match (return) */

       

   if (ERR(error) == ERR_NOT_FOUND) 
   {
      printf ("\nKey not found in the collection.\n");
      error=NOERROR;
      goto exit1;
   }
   
   if (error) 
      goto exit1;

  
   do
   {

  /* Read entries in the collection */

      if (error = NIFReadEntries(
        hCollection,         /* handle to this collection           */
        &posCollection,      /* where to start in collection        */
        (WORD) (FirstTime ? NAVIGATE_CURRENT : NAVIGATE_NEXT),
                                      /* order to use when skipping */
        FirstTime ? 0L : 1L, /* number to skip i                    */
        NAVIGATE_NEXT,       /* order to use when reading           */
        NumNotesMatch - NoteCount,  /* max number to read           */
        READ_MASK_NOTEID,    /* info we want                        */
        &hBuffer,            /* handle to info (return)             */
        NULL,                /* length of buffer (return)           */
        NULL,                /* entries skipped (return)            */
        &NumNotesFound,      /* entries read (return)               */
        &signal_flag))       /* signal and share warnings (return)  */
         goto exit1;

      if (hBuffer == NULLHANDLE)
      {
         printf ("\nEmpty buffer returned by NIFReadEntries.\n");
         goto exit1;
      }

      pNoteID = (NOTEID *) OSLockObject (hBuffer);

      printf ("\n");
      for (i=0; i<NumNotesFound; i++)
          printf ("Note count is %lu. \t noteID  is: %lX\n", 
             ++NoteCount, pNoteID[i]);
   
      OSUnlockObject (hBuffer);

      OSMemFree (hBuffer);

      if (FirstTime)
          FirstTime = FALSE;

   }  while (signal_flag & SIGNAL_MORE_TO_DO);
   

exit1:
   if (cleanup & CLOSE_COLLECTION)
      NIFCloseCollection (hCollection);

   if (cleanup & CLOSE_DB)
      NSFDbClose (hDb);

   if (cleanup & FREE_KEY1)
      free(Key1);

   if (cleanup & FREE_TRANSLATEDKEY)
      free(TranslatedKey);

   if (cleanup & FREE_KEY2)
      free(Key2);

   if (cleanup & FREE_PKEY)
      free(pKey);

   if (error)
      PrintAPIError(error);

   NotesTerm();

   if (returnCode==NOERROR && error==NOERROR)
      printf("\nProgram completed successfully.\n");

   if (returnCode)
      return(returnCode);
   else
      return(error);
}


/************************************************************************
 
    FUNCTION:   ProcessArgs
 
    INPUTS:     argc, argv - directly from the command line
 
    OUTPUTS:    Key1, Key1 - data from the command line or
                from what the user types when prompted.
 
*************************************************************************/
 
void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                             char *Key1, char *Key2,
                             char *dbName, char *viewName)
{
    if (argc != 5)
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
 
    }
    else
    {
         strncpy(dbName, argv[1], STRING_LENGTH-1);
         strncpy(viewName, argv[2], STRING_LENGTH-1);
         strncpy(Key1, argv[3], STRING_LENGTH-1);
         strncpy(Key2, argv[4], STRING_LENGTH-1);
 
    } /* end if */
} /* ProcessArgs */


/* This function prints the HCL C API for Notes/Domino error message
   associated with an error code. */

void PrintAPIError (STATUS api_error)

{
    STATUS  string_id = ERR(api_error);
    char    error_text[200];
    WORD    text_len;

    /* Get the message for this HCL C API for Notes/Domino error code
       from the resource string table. */

    text_len = OSLoadString (NULLHANDLE,
                             string_id,
                             error_text,
                             sizeof(error_text));

    /* Print it. */
    fprintf (stderr, "\n%s\n", error_text);

}

