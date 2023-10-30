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

    PROGRAM:    mixed_2

    FILE:       mixed_2.c

    DESCRIPTION:  
        This program tests the performance of Domino and Notes for a mixed set 
		of transactions on the Name and Address book. This version of the
        test uses "worst case" transactions (index updated before each
        lookup).

        For a given transactions test size (N), the transactions are:

            .33N additions of new records with no index access at all.
    
            .33N modifications of existing records, using an index update
            and lookup to find each record.

            .33N deletions of existing records, using an index update and
            lookup to find each record.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* OS and C include files */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <nif.h>
#include <osmem.h>
#include <miscerr.h>
#include <ostime.h>
#include <misc.h>
#include <osmisc.h>
#include "../../printLog.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/* HCL C API for Notes/Domino subroutine */

int main (int argc, char *argv[])
{

/* Local data declarations */

   char               *db_path;       /* pathname of test database */
   int                transactions;   /* total number of transactions */

   DBHANDLE           db_handle;      /* database handle */
   NOTEHANDLE         note_handle;    /* note handle */
   HCOLLECTION        coll_handle;    /* collection handle */
   DHANDLE              buffer_handle;  /* handle to buffer of note ids */

   char               last_name[100]; /* last name in N&A book */
   char               count[10];      /* used to construct unique names */

   NOTEID             view_id;        /* note id of the view */
   COLLECTIONPOSITION coll_pos;       /* position within collection */
   NOTEID             *id_list;       /* pointer to a note id */
   DWORD              notes_found;    /* number of notes found */
   DWORD              match_size;     /* number of notes matching key */

   int                i;
   STATUS             error=NOERROR;

   LONG           time_delta;
   TIMEDATE       StartTime;        /* time to mark the start of the program */
   TIMEDATE       LastTime;         /* time to mark the end of the program */



/* ************************************************** */
/* Get the input parameters. */
/* ************************************************** */

   if (argc != 3)
   {
      PRINTLOG ("\nUsage:  MIXED_2  <database pathname>  <number of transaction>\n");
      return(0);
   }
   db_path = argv[1];
   transactions = atoi(argv[2]);

   PRINTLOG ("\n***** MIXED_2 Test Begins ******\n");


   if (error = NotesInitExtended (argc, argv))
   {
      PRINTLOG("\nUnable to initialize Notes.\n");
      return(1);
   }


/* ************************************************** */
/* Open the database. */
/* ************************************************** */

   if (error = NSFDbOpen (db_path, &db_handle))
   {
      PRINTERROR (error,"NSFDbOpen");
      NotesTerm();
      return(1);
   }


/* ************************************************** */
/* Get System TIME  . */
/* ************************************************** */
 
   OSCurrentTIMEDATE(&StartTime);

/* ************************************************** */
/* Add new records, each with a different last name. */
/* ************************************************** */

/* Start a big loop for adding names. */

   for (i=0; i<(transactions/3); i++)
   {

/* Construct the last name. */

      strcpy (last_name, "Connell");
      sprintf(count,"%i", i);
      strcat (last_name, count);

/* Create the record. */

      if (error = NSFNoteCreate (db_handle, &note_handle))
      {
          NSFDbClose (db_handle);
          PRINTERROR (error,"NSFNoteCreate");
          NotesTerm();
          return(1);
      }

/* Add all the fields. */

      if (error = NSFItemSetText ( note_handle, 
                                   "Form",
                                   "Person",
                                   MAXWORD))
      {
          NSFNoteClose (note_handle);
          NSFDbClose (db_handle);
          PRINTERROR (error,"NSFItemSetText");
          NotesTerm();
          return(1);
      }

      if (error = NSFItemSetText ( note_handle,
                                   "Type",
                                   "Person",
                                   MAXWORD))
      {
          NSFNoteClose (note_handle);
          NSFDbClose (db_handle);
          PRINTERROR (error,"NSFItemSetText");
          NotesTerm();
          return(1);
      }

      if (error = NSFItemSetText ( note_handle,
                                   "FirstName",
                                   "Charles",
                                   MAXWORD))
      {
          NSFNoteClose (note_handle);
          NSFDbClose (db_handle);
          PRINTERROR (error,"NSFItemSetText");
          NotesTerm();
          return(1);
      }

      if (error = NSFItemSetText ( note_handle,
                                   "LastName",
                                   last_name,
                                   MAXWORD))
      {
          NSFNoteClose (note_handle);
          NSFDbClose (db_handle);
          PRINTERROR (error,"NSFItemSetText");
          NotesTerm();
          return(1);
      }

      if (error = NSFItemSetText ( note_handle,
                                   "MailDomain",
                                   "NOTES",
                                   MAXWORD))
      {
          NSFNoteClose (note_handle);
          NSFDbClose (db_handle);
          PRINTERROR (error,"NSFItemSetText");
          NotesTerm();
          return(1);
      }

      if (error = NSFItemSetText ( note_handle,
                                   "MailFile",
                                   "MAIL\\CCONNELL",
                                   MAXWORD))
      {
          NSFNoteClose (note_handle);
          NSFDbClose (db_handle);
          PRINTERROR (error,"NSFItemSetText");
          NotesTerm();
          return(1);
      }

/* Write the new note to disk and close the note. */

      if (error = NSFNoteUpdate (note_handle, 0))
      {
          NSFNoteClose (note_handle);
          NSFDbClose (db_handle);
          PRINTERROR (error,"NSFNoteUpdate");
          NotesTerm();
          return(1);
      }

      if (error = NSFNoteClose (note_handle))
      {
          NSFDbClose (db_handle);
          PRINTERROR (error,"NSFNoteClose");
          NotesTerm();
          return(1);
      }

/* End of big loop that is adding records. */

   }


/* ************************************************** */
/* Get System TIME  . */
/* ************************************************** */
 
   OSCurrentTIMEDATE(&LastTime);
 
   time_delta= TimeDateDifference (&LastTime, &StartTime);
 
   PRINTLOG ("It took %i seconds to add %i names with no index usage at all\n", time_delta, transactions/3);

/* ************************************************** */
/* Get System TIME  . */
/* ************************************************** */
 
   OSCurrentTIMEDATE(&StartTime);

/* ************************************************** */
/* Open the index on names. */
/* ************************************************** */

   if (error = NIFFindView (db_handle, "Contacts", &view_id))
   {
      NSFDbClose (db_handle);
      PRINTERROR (error,"NIFFindView");
      NotesTerm();
      return(1);
   }

   if (error = NIFOpenCollection(
                                 db_handle,      /* handle of db with view */
                                 db_handle,      /* handle of db with data */
                                 view_id,     /* note id of the view */
                                 0,        /* collection open flags */
                                 NULLHANDLE,     /* handle to unread ID list (input and return) */
                                 &coll_handle,      /* collection handle (return) */
                                 NULLHANDLE,     /* handle to open view note (return) */
                                 NULL,        /* universal note id of view (return) */
                                 NULLHANDLE,     /* handle to collapsed list (return) */
                                 NULLHANDLE))       /* handle to selected list (return) */
   {
      NSFDbClose (db_handle);
      PRINTERROR (error,"NIFOpenCollection");
      NotesTerm();
      return(1);
   }

/* ************************************************** */
/* Get System TIME  . */
/* ************************************************** */
 
   OSCurrentTIMEDATE(&LastTime);
 
   time_delta= TimeDateDifference (&LastTime, &StartTime);
 
   PRINTLOG ("It took %i seconds to index Contacts view\n", time_delta);
 
 
/* ************************************************** */
/* Get System TIME  . */
/* ************************************************** */
 
   OSCurrentTIMEDATE(&StartTime);

/* ************************************************** */
/* Find even numbered records in the index and modify them. */
/* ************************************************** */

/* Start a big loop that will modify many records. */

   for (i=0; i<(transactions/3); i++)
   {

/* Construct a last name. */

      strcpy (last_name, "Connell");
      sprintf(count,"%i ", i);
      strcat (last_name, count);
      strcat (last_name, ", Charles");

/* Refresh the index. */

   if (error = NIFUpdateCollection(coll_handle))
   {
      NIFCloseCollection (coll_handle);
      NSFDbClose (db_handle);
      PRINTERROR (error,"NIFUpdateCollection");
      NotesTerm();
      return(1);
   }

/* Look in the index for this name. */

   error = NIFFindByName (
                          coll_handle,       /* collection to look in */
                          last_name,         /* string to match on */
                          FIND_CASE_INSENSITIVE,   /* match rules */
                          &coll_pos,         /* where match begins (return) */
                          &match_size);      /* how many match (return) */

   if (ERR(error) == ERR_NOT_FOUND) 
   {
      PRINTLOG ("\nKey %s not found in the collection.\n",last_name);
      NIFCloseCollection (coll_handle);
      NSFDbClose (db_handle);
      NotesTerm();
      return(0);
   }
   
   if (error)
   {
      NIFCloseCollection (coll_handle);
      NSFDbClose (db_handle);
      PRINTERROR (error,"NIFFindByName");
      NotesTerm();
      return(1);
   }

/* Get the note IDs of all records with this key. */

   if (error = NIFReadEntries(
                              coll_handle,      /* handle to this collection */
                              &coll_pos,        /* where to start in collection */
                              NAVIGATE_CURRENT,       /* order to use when skipping */
                              0L,                  /* number to skip */
                              NAVIGATE_NEXT,       /* order to use when reading */
                              match_size,          /* max number to read */
                              READ_MASK_NOTEID,       /* info we want */
                              &buffer_handle,      /* handle to info (return)   */
                              NULL,          /* length of buffer (return) */
                              NULL,          /* entries skipped (return) */
                              &notes_found,     /* entries read (return) */
                              NULL))         /* share warning (return) */

   {
      NIFCloseCollection (coll_handle);
      NSFDbClose (db_handle);
      PRINTERROR (error,"NIFReadEntries");
      NotesTerm();
      return(1);
   }

   if (buffer_handle == NULLHANDLE)
   {
      NIFCloseCollection (coll_handle);
      NSFDbClose (db_handle);
      PRINTLOG ("\nEmpty buffer returned by NIFReadEntries.\n");
      NotesTerm();
      return(0);
   }

   id_list = (NOTEID *) OSLockObject (buffer_handle);

/* Open the first note with this key. */

   if (error = NSFNoteOpen (db_handle, id_list[0], 0, &note_handle))
   {
      OSUnlockObject (buffer_handle);
      OSMemFree (buffer_handle);
      NIFCloseCollection (coll_handle);
      NSFDbClose (db_handle);
      PRINTERROR (error,"NSFNoteOpen");
      NotesTerm();
      return(1);
   }

/* Get rid of the buffer of note IDs since we don't need it anymore. */

   OSUnlockObject (buffer_handle);
   OSMemFree (buffer_handle);

/* Modify one of the fields in this note. */

   if (error = NSFItemSetText ( note_handle,
                                "FirstName",
                                "Janet",
                                MAXWORD))
   {
      NIFCloseCollection (coll_handle);
      NSFNoteClose (note_handle);
      NSFDbClose (db_handle);
      PRINTERROR (error,"NSFItemSetText");
      NotesTerm();
      return(1);
   }

/* Update the on-disk copy of the note and close it. */

   if (error = NSFNoteUpdate (note_handle, 0))
   {
      NIFCloseCollection (coll_handle);
      NSFNoteClose (note_handle);
      NSFDbClose (db_handle);
      PRINTERROR (error,"NSFNoteUpdate");
      NotesTerm();
      return(1);
   }

   if (error = NSFNoteClose (note_handle))
   {
      NIFCloseCollection (coll_handle);
      NSFDbClose (db_handle);
      PRINTERROR (error,"NSFNoteClose");
      NotesTerm();
      return(1);
   }

/* End of big loop that is modifying many records. */

   }

/* ************************************************** */
/* Get System TIME  . */
/* ************************************************** */
 
   OSCurrentTIMEDATE(&LastTime);
 
   time_delta= TimeDateDifference (&LastTime, &StartTime);
 
   PRINTLOG ("It took %i seconds to modify %i records \n", time_delta, transactions/3);
 
/* ************************************************** */
/* Get System TIME  . */
/* ************************************************** */
 
   OSCurrentTIMEDATE(&StartTime);

/* ************************************************** */
/* Find odd numbered records in the index and delete them. */
/* ************************************************** */

/* Start a big loop that will delete many records. */

   for (i=0; i<(transactions/3); i++)
   {

/* Construct a last name. */

      strcpy (last_name, "Connell");
      sprintf(count,"%i ", i);
      strcat (last_name, count);
      strcat (last_name, ", Janet");

/* Refresh the index. */

      if (error = NIFUpdateCollection(coll_handle))
      {
          NIFCloseCollection (coll_handle);
          NSFDbClose (db_handle);
          PRINTERROR (error,"NIFUpdateCollection");
          NotesTerm();
          return(1);
      }

/* Look in the index for this name. */

      error = NIFFindByName (
                             coll_handle,       /* collection to look in */
                             last_name,         /* string to match on */
                             FIND_CASE_INSENSITIVE,   /* match rules */
                             &coll_pos,         /* where match begins (return) */
                             &match_size);      /* how many match (return) */

      if (ERR(error) == ERR_NOT_FOUND) 
      {
          PRINTLOG ("\nKey not found in the collection.\n");
          NIFCloseCollection (coll_handle);
          NSFDbClose (db_handle);
          NotesTerm();
          return(0);
      }
   
      if (error)
      {
          NIFCloseCollection (coll_handle);
          NSFDbClose (db_handle);
          PRINTERROR (error,"NIFFindByName");
          NotesTerm();
          return(1);
      }

/* Get the note IDs of all records with this key. */

      if (error = NIFReadEntries(
                                 coll_handle,      /* handle to this collection */
                                 &coll_pos,        /* where to start in collection */
                                 NAVIGATE_CURRENT,       /* order to use when skipping */
                                 0L,                  /* number to skip */
                                 NAVIGATE_NEXT,       /* order to use when reading */
                                 match_size,          /* max number to read */
                                 READ_MASK_NOTEID,       /* info we want */
                                 &buffer_handle,      /* handle to info (return)   */
                                 NULL,          /* length of buffer (return) */
                                 NULL,          /* entries skipped (return) */
                                 &notes_found,     /* entries read (return) */
                                 NULL))         /* share warning (return) */

      {
          NIFCloseCollection (coll_handle);
          NSFDbClose (db_handle);
          PRINTERROR (error,"NIFReadEntries");
          NotesTerm();
          return(1);
      }

      if (buffer_handle == NULLHANDLE)
      {
          NIFCloseCollection (coll_handle);
          NSFDbClose (db_handle);
          PRINTLOG ("\nEmpty buffer returned by NIFReadEntries.\n");
          NotesTerm();
          return(0);
      }

      id_list = (NOTEID *) OSLockObject (buffer_handle);

/* Delete the first note with this key. */

      if (error = NSFNoteDelete (db_handle, id_list[0], 0))
      {
          OSUnlockObject (buffer_handle);
          OSMemFree (buffer_handle);
          NIFCloseCollection (coll_handle);
          NSFDbClose (db_handle);
          PRINTERROR (error,"NSFNoteDelete");
          NotesTerm();
          return(1);
      }

/* Get rid of the buffer of note IDs. */

      OSUnlockObject (buffer_handle);
      OSMemFree (buffer_handle);

/* End of big loop that is deleting many records. */

   }

/* ************************************************** */
/* Get System TIME  . */
/* ************************************************** */
 
   OSCurrentTIMEDATE(&LastTime);
 
   time_delta= TimeDateDifference (&LastTime, &StartTime);
 
   PRINTLOG ("It took %i seconds to delete %i records \n", time_delta, transactions/3);

/* ************************************************** */
/* Close the index. */
/* ************************************************** */

   if (error = NIFCloseCollection (coll_handle))
   {
      NSFDbClose (db_handle);
      PRINTERROR (error,"NIFCloseCollection");
      NotesTerm();
      return(1);
   }

/* ************************************************** */
/* Close the database */
/* ************************************************** */

   if (error = NSFDbClose (db_handle))
   {
      PRINTERROR (error,"NSFDbClose");
      NotesTerm();
      return(1);
   }

/* ************************************************** */
/* End of subroutine. */
/* ************************************************** */

   NotesTerm();
   return(0);
}
