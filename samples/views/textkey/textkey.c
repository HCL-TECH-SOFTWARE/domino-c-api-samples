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
    PROGRAM:    textkey

    FILE:       textkey.c

    SYNTAX:     textkey  <db filename>  <view name>  <key (text)>


****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* C include files */

#include <stdio.h>
#include <string.h>

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <nsfdb.h>
#include <nif.h>
#include <osmem.h>
#include <miscerr.h>
#include "../../printLog.h"

#include <osmisc.h>

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

#define  STRING_LENGTH  256

void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                         char *db_filename, 
                         char *view_name, 
                         char *text_key); 
						 

/* HCL C API for Notes/Domino subroutine */

int main(int argc, char *argv[])
{

/* Local data declarations */

   char               db_filename[STRING_LENGTH];        /* pathname of the database */
   char               view_name[STRING_LENGTH];          /* name of the view we'll read */
   char               text_key[STRING_LENGTH];           /* key to search for in view */
   DBHANDLE           db_handle;                         /* handle of the database */
   NOTEID             view_id;                           /* note id of the view */
   HCOLLECTION        coll_handle;                       /* collection handle */
   COLLECTIONPOSITION coll_pos;                          /* position within collection */
   DHANDLE            buffer_handle;                     /* handle to buffer of note ids */
   NOTEID             *id_list;                          /* pointer to a note id */
   DWORD              notes_found;                       /* number of notes found */
   DWORD              match_size;                        /* number of notes matching key */
   DWORD              i;                                 /* a counter */
   DWORD              note_count = 0;                    /* ordinal number of the note */
   STATUS             error = NOERROR;                   /* return status from API calls */
   WORD               signal_flag;                       /* signal and share warning flag */
   BOOL               FirstTime = TRUE;                  /* used in NIFReadEntries loop */

   if (error = NotesInitExtended (argc, argv))
   {
      PRINTLOG("\n Unable to initialize Notes. Error Code[0x%04x]\n", error);
      return (1);
   }


/* Get the command line parameters that the user entered. */

   ProcessArgs (argc, argv, 
             db_filename, view_name, text_key);


/* Open the database. */

   if (error = NSFDbOpen (db_filename, &db_handle))
   {
      PRINTERROR (error,"NSFDbOpen");  
      NotesTerm();
      return (1);
   } 

/* Get the note id of the view we want. */

   if (error = NIFFindView (db_handle, view_name, &view_id))
   {
      NSFDbClose (db_handle);
      PRINTERROR (error,"NIFFindView");  
      NotesTerm();
      return (1);
   }

/* Get a collection of notes using this view. */

   if (error = NIFOpenCollection(
                      db_handle,      /* handle of db with view */
                      db_handle,      /* handle of db with data */
                      view_id,        /* note id of the view */
                      0,              /* collection open flags */
                      NULLHANDLE,     /* handle to unread ID list (input and return) */
                      &coll_handle,   /* collection handle (return) */
                      NULLHANDLE,     /* handle to open view note (return) */
                      NULL,           /* universal note id of view (return) */
                      NULLHANDLE,     /* handle to collapsed list (return) */
                      NULLHANDLE))    /* handle to selected list (return) */
   {
      NSFDbClose (db_handle);
      PRINTERROR (error,"NIFOpenCollection");  
      NotesTerm();
      return (1);
   }

/* Look for notes that have the given primary sort key (which must be of
type text). We get back a COLECTIONPOSITION structure describing where the 
first such note is in the collection, and a count of how many such notes 
there are. Check the return code for "not found" versus a real error. */

   error = NIFFindByName (
                  coll_handle,           /* collection to look in */
                  text_key,              /* string to match on */
                  FIND_CASE_INSENSITIVE, /* match rules */
                                         /* another FIND_ option to add to experiment with is
                                            FIND_PARTIAL - to do wildcard searches */
                  &coll_pos,             /* where match begins (return) */
                  &match_size);          /* how many match (return) */

   if (ERR(error) == ERR_NOT_FOUND) 
   {
      PRINTLOG ("\nKey not found in the collection.\n");
      NIFCloseCollection (coll_handle);
      NSFDbClose (db_handle);
      NotesTerm();
      return (0); 
   }
   
   if (error)
   {
      NIFCloseCollection (coll_handle);
      NSFDbClose (db_handle);
      PRINTERROR (error,"NIFFindByName");  
      NotesTerm();
      return (1);
   }

/* Get a buffer of all the note IDs that have this key. */

   do
   {

      if (error = NIFReadEntries(
                         coll_handle,         /* handle to this collection */
                         &coll_pos,           /* where to start in collection */
                         (WORD) (FirstTime ? NAVIGATE_CURRENT : NAVIGATE_NEXT),
                                              /* order to use when skipping */
                         FirstTime ? 0L : 1L, /* number to skip */
                         NAVIGATE_NEXT,       /* order to use when reading */
                         match_size - note_count,    /* max number to read */
                         READ_MASK_NOTEID,    /* info we want */
                         &buffer_handle,      /* handle to info (return)   */
                         NULL,                /* length of buffer (return) */
                         NULL,                /* entries skipped (return) */
                         &notes_found,        /* entries read (return) */
                         &signal_flag))       /* signal and share warnings (return) */

      {
         NIFCloseCollection (coll_handle);
         NSFDbClose (db_handle);
         PRINTERROR (error,"NIFReadEntries");  
         NotesTerm();
         return (1);
      }

/* Check to make sure there was a buffer of information returned.
(This is just for safety. We know that some notes matched the search
key.) */ 

      if (buffer_handle == NULLHANDLE)
      {
         NIFCloseCollection (coll_handle);
         NSFDbClose (db_handle);
         PRINTLOG ("\nEmpty buffer returned by NIFReadEntries.\n");
         NotesTerm();
         return (0); 
      }

/* Lock down (freeze the location) of the buffer of notes IDs. Cast
the resulting pointer to the type we need. */

      id_list = (NOTEID *) OSLockObject (buffer_handle);
   
/* Print out the list of note IDs found by this search. */

      PRINTLOG ("\n");
      for (i=0; i<notes_found; i++)
         PRINTLOG ("Note count is %lu. \t Note ID is: %lX\n", 
                    ++note_count, id_list[i]);

/* Unlock the list of note IDs. */

      OSUnlockObject (buffer_handle);

/* Free the memory allocated by NIFReadEntries. */

      OSMemFree (buffer_handle);

      if (FirstTime)
         FirstTime = FALSE;

   } while (signal_flag & SIGNAL_MORE_TO_DO);

/* Close the collection. */

   if (error = NIFCloseCollection(coll_handle))
   {
      NSFDbClose (db_handle);
      PRINTERROR (error,"NIFCloseCollection");  
      NotesTerm();
      return (1);
   }

/* Close the database. */

   if (error = NSFDbClose (db_handle))
   {     
       PRINTERROR (error,"NSFDbClose");  
       NotesTerm();
       return (1);
   }

/* End of subroutine. */

   NotesTerm();

   if (error==NOERROR)
      PRINTLOG("\nProgram completed successfully.\n");

   return (0); 

}

/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:     argc, argv - directly from the command line
   
    OUTPUTS:    db_filename, view_name, text_key get data
                from the command line or from what the user types
                at a prompt 

*************************************************************************/

void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                         char *db_filename, 
                         char *view_name, 
                         char *text_key)
{ 
   memset(db_filename, '\0', STRING_LENGTH);
   memset(view_name, '\0', STRING_LENGTH);
   memset(text_key, '\0', STRING_LENGTH);
   if (argc != 4)
   {       
      printf("Enter name of input database: ");   
      fflush(stdout);
      fgets(db_filename, STRING_LENGTH-1, stdin);
           
      printf("\n");
      printf("Enter name of view to use: ");
      fflush(stdout);
      fgets(view_name, STRING_LENGTH-1, stdin);

      printf("\n");
      printf("Enter the text key: ");      
      fflush(stdout);
      fgets(text_key, STRING_LENGTH-1, stdin);
   }  
   else
   {
      strncpy(db_filename, argv[1], STRING_LENGTH-1);
      strncpy(view_name, argv[2], STRING_LENGTH-1);
      strncpy(text_key, argv[3], STRING_LENGTH-1);      
   } /* end if */
} /* ProcessArgs */

