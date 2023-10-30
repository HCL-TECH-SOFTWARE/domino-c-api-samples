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

    PROGRAM:    index

    FILE:       index.c

    DESCRIPTION:  
        This program updates an index in a Notes database. The program can
        be used as a performance test, or to create an index for 
        other testing.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* OS and C include files */

#include <stdio.h>

/* Notes API include files */

#include <global.h>
#include <nsfdb.h>
#include <nif.h>
#include <ostime.h>
#include <misc.h>
#include <osmisc.h>
#include "../../printLog.h"

/* Notes API subroutine */

int main (int argc, char *argv[])
{


/* Local data declarations */

   char           *db_path;      /* pathname of database */
   char           *view_name;    /* name of view to update */

   DBHANDLE       db_handle;     /* database handle */
   HCOLLECTION    coll_handle;   /* collection handle */
   NOTEID         view_id;       /* note ID of the view note */

   STATUS         error=NOERROR;

   LONG           time_delta;
   TIMEDATE       StartTime;        /* time to mark the start of the program */
   TIMEDATE       LastTime;         /*  time to mark the end of the program */

/* Get the input parameters. */

   if (argc != 3)
   {
      PRINTLOG ("\nUsage:  INDEX  <database pathname>  <name of view>\n");
      return(0);
   }
   db_path = argv[1];
   view_name = argv[2];

   PRINTLOG ("\n***** INDEX Test Begins ******\n");

   if (error = NotesInitExtended (argc, argv))
   {
      PRINTLOG("\nUnable to initialize Notes.\n");
      return(1);
   }

/* ************************************************** */
/* Get System TIME  . */
/* ************************************************** */
 
   OSCurrentTIMEDATE(&StartTime);

/* Open the database. */

   if (error = NSFDbOpen (db_path, &db_handle))
      goto Done1;

/* Find the note ID of the view note. */

   if (error = NIFFindView (db_handle, view_name, &view_id))
      goto Done2;

/* Open the view (thereby updating it). */

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
      goto Done2;

/* Close the index. */

   if (error = NIFCloseCollection (coll_handle))
      goto Done2;

/* Close the database */
Done2:
   NSFDbClose (db_handle);

Done1:
   if (error)
   {
      PRINTERROR(error,"NIFOpenCollection");
      NotesTerm();
      return(1);
   }

/* ************************************************** */
/* Get System TIME  . */
/* ************************************************** */
 
   OSCurrentTIMEDATE(&LastTime);

   time_delta= TimeDateDifference (&LastTime, &StartTime);
 
   PRINTLOG ("It took %i seconds to index\n", time_delta);
 
/* End of subroutine. */

   NotesTerm();
   return(0);
}

