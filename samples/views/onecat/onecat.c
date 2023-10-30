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

    PROGRAM:    onecat

    FILE:       onecat.c

    SYNTAX:     onecat  <db filename>  <view name>  <category (text)>

    DESCRIPTION:
        This program finds all the main-topic documents in a view category.
        We assume that the category is a main (left-most) category with 
        a text data type.

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


void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                         char *db_filename, 
                         char *view_name, 
                         char *category); 

#define  STRING_LENGTH  256

/************************************************************************

    FUNCTION:   Main

    PURPOSE:    HCL C API for Notes/Domino subroutine

*************************************************************************/

int main(int argc, char *argv[])
{


/* Local constants */

#define     CATEGORY_LEVEL_BEING_SEARCHED 0
#define  MAIN_TOPIC_INDENT    0

/* Local data declarations */

   char               db_filename[STRING_LENGTH];            /* pathname of the database */
   char               view_name[STRING_LENGTH];              /* name of the view we'll read */
   char               category[STRING_LENGTH];               /* category to search for in view */
   DBHANDLE           db_handle;                             /* handle of the database */
   NOTEID             view_id;                               /* note id of the view */
   HCOLLECTION        coll_handle;                           /* collection handle */
   COLLECTIONPOSITION cat_index;                             /* index of category entry */
   DHANDLE            buffer_handle;                         /* handle to buffer of info */
   BYTE               *buff_ptr;                             /* pointer into info buffer */
   NOTEID             entry_id;                              /* a collection entry id */
   WORD               entry_indent;                          /* "indent level" of entry */
   WORD               entry_index_size;                      /* size of index postion */
   COLLECTIONPOSITION entry_index;                           /* an index position */
   DWORD              entries_found;                         /* number of entries found */
   DWORD              main_topics_found=0;                   /* number of main docs found */
   DWORD              i;                                     /* a counter */
   STATUS             error = NOERROR;                       /* return status from API calls */
   WORD               signal_flag;                           /* signal and share warning flags */
   BOOL               FirstTime = TRUE;                      /* used in NIFReadEntries loop */


    /*   Start by calling Notes Init.  */

   if (error = NotesInitExtended (argc, argv))
   {
       PRINTLOG("\n Unable to initialize Notes. Error Code[0x%04x]\n", error);
       return (1);
   }


/* Get the command line parameters that the user entered. */

   ProcessArgs (argc, argv, 
             db_filename, view_name, category);
 
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

/* Get the current collection using this view. */

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

/* Look for the given category (which must be of type text). We get back a 
COLECTIONPOSITION describing where the first document in the category is. 
Ignore the "match count" that this call returns, since it does not span 
subcategories. */

   error = NIFFindByName (
                 coll_handle,             /* collection to look in */
                 category,                /* string to match on */
                 FIND_CASE_INSENSITIVE,   /* match rules */
                 &cat_index,              /* where match begins (return) */
                 NULL);                   /* how many match (return) */

   if (ERR(error) == ERR_NOT_FOUND) 
   {
      PRINTLOG ("\nCategory not found in the collection.\n");
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

/*
Starting at the first doc in this category, get a buffer with information
about all the remaining entries in the collection. For each entry get the
following pieces of info: note ID, "indent level", and position in the
collection tree. The information is returned in a packed buffer.

In the returned buffer, first comes all of the info about the first entry,
then all of the info about the 2nd entry, etc. For each entry, the info is
arranged in the order of the bits in the READ_MASKs.
*/

   do
      {
         if (error = NIFReadEntries(
                            coll_handle,            /* handle to this collection */
                            &cat_index,             /* where to start in collection */
                            (WORD) (FirstTime ? NAVIGATE_CURRENT : NAVIGATE_NEXT),
                                                    /* order to use when skipping */
                            FirstTime ? 0L : 1L,    /* number to skip */
                            NAVIGATE_NEXT,          /* order to use when reading */
                            0xFFFFFFFF,             /* max number to read */
                            READ_MASK_NOTEID +      /* info we want */
                            READ_MASK_INDENTLEVELS +
                            READ_MASK_INDEXPOSITION,
                            &buffer_handle,         /* handle to info buffer (return)  */
                            NULL,                   /* length of info buffer (return) */
                            NULL,                   /* entries skipped (return) */
                            &entries_found,         /* entries read (return) */
                            &signal_flag))          /* signal and share warnings (return) */
         {
            NIFCloseCollection (coll_handle);
            NSFDbClose (db_handle);
            PRINTERROR (error,"NIFReadEntries");  
            NotesTerm();
            return (1);
         }
 
/* Check to make sure there was a buffer of information returned. (This
is just for safety, since we know that the category exists and therefore
has some documents in it.) */ 

        if (buffer_handle == NULLHANDLE)
        {
           NIFCloseCollection (coll_handle);
           NSFDbClose (db_handle);
           PRINTLOG ("\nEmpty buffer returned by NIFReadEntries.\n");
           NotesTerm();
           return (0); 
        }

/* Lock down (freeze the location) of the information buffer. Cast
the resulting pointer to the type we need. */

        buff_ptr = (BYTE *) OSLockObject (buffer_handle);

/* Start a loop that extracts the info about each collection entry from
the information buffer. */

        PRINTLOG ("\n");
        for (i = 1; i <= entries_found; i++)
        {

/* Get the ID of this entry. */

           entry_id = *(NOTEID*) buff_ptr;
           buff_ptr += sizeof (NOTEID);

/* Get the "indent level" of this entry. For category entries, the indent
level is only used in multi-level columns (multiple levels of categories
within one column). For note entries, the indent level tells whether the
note is a main topic, response, response to response, etc. */

           entry_indent = *(WORD*) buff_ptr;
           buff_ptr += sizeof (WORD);

/* Find the size of this entry's index information. Each entry's index
info may be a different length, since it is truncated just to the length
needed. */

           entry_index_size = COLLECTIONPOSITIONSIZE
                 ((COLLECTIONPOSITION*) buff_ptr);

/* Get the COLLECTIONPOSITION of the entry. */

           memcpy (&entry_index, buff_ptr, entry_index_size);
           buff_ptr += entry_index_size;

/* If this entry is a top-level category, then we have found all the notes
in the category we want. (We will have reached the next major category.) */

           if (entry_index.Level == CATEGORY_LEVEL_BEING_SEARCHED)
           {
             /* Indicate that there is no more to do. */
               signal_flag &= ~SIGNAL_MORE_TO_DO;
               break;
           }  

/* If this entry is a lower-level category, skip over it. */

           if (NOTEID_CATEGORY & entry_id) continue;

/* If this entry is a response doc (or response to response, etc) skip
over it. */

           if (entry_indent != MAIN_TOPIC_INDENT) continue;

/* We have found a main-topic note. Print out its note ID. */

           PRINTLOG ("Main topic count is: %lu.  \tNote ID is: %lX.\n",
                  ++main_topics_found, entry_id);

/* End of loop that gets info about each entry. */

         }

/* Unlock the list of note IDs. */

        OSUnlockObject (buffer_handle);

/* Free the memory allocated by NIFReadEntries. */

        OSMemFree (buffer_handle);
        if (FirstTime)
           FirstTime = FALSE;

      }  while (signal_flag & SIGNAL_MORE_TO_DO);

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

   PRINTLOG("\nProgram completed successfully.\n");

   NotesTerm();
   return (0); 

}

/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:     argc, argv - directly from the command line
   
    OUTPUTS:    db_filename, view_name, category get data
                from the command line or from what the user types
                at a prompt 

*************************************************************************/

void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                         char *db_filename, 
                         char *view_name, 
                         char *category)
{ 
    memset(db_filename, '\0', STRING_LENGTH);  
    memset(view_name, '\0', STRING_LENGTH);  
    memset(category, '\0', STRING_LENGTH);  
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
      printf("Enter name of category to select: ");      
      fflush(stdout);
      fgets(category, STRING_LENGTH-1, stdin);
   }  
   else
    {
      strncpy(db_filename, argv[1], STRING_LENGTH-1);
      strncpy(view_name, argv[2], STRING_LENGTH-1);
      strncpy(category, argv[3], STRING_LENGTH-1);      
    } /* end if */
} /* ProcessArgs */


