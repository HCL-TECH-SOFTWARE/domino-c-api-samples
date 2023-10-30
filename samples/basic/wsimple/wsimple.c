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

    PROGRAM:   wsimple

    FILE:      wsimple.c

    PURPOSE:   Shows how to create a new data note and fill in fields with
               the simple data types.

    SYNTAX:    wsimple

    COMMENTS:
        This program opens the specified database, creates a new data note,
        and fills in fields with the simple data types.

*************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* OS and C include files */
#include <stdio.h>

/* Notes API include files */
#if defined (OS390)
#include <lapicinc.h>
#endif

#include <global.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <ostime.h>
#include <osmisc.h>
#include "../../printLog.h"

/* Notes API subroutine */
int main (int argc, char *argv[])
{
    char        path_name[]="test.nsf";   /* pathname of database */
    DBHANDLE    db_handle;          /* database handle */
    NOTEHANDLE  note_handle;        /* note handle */
    NUMBER      num_field;          /* contents of a numeric field */
    TIMEDATE    timedate;           /* contents of a time/date field */
    STATUS      error = NOERROR;    /* return code from API calls */

    if (error = NotesInitExtended (argc, argv))
    {
        PRINTLOG("\n Unable to initialize Notes.\n");
        return (1);
    }

  /* Open the database. */

    if (error = NSFDbOpen (path_name, &db_handle))
    {
        PRINTERROR (error,"NSFDbOpen");
        NotesTerm();
        return (1);
    }


    /* Create a new data note. */

    if (error = NSFNoteCreate (db_handle, &note_handle))
    {
        PRINTERROR (error,"NSFNoteCreate");
        NSFDbClose (db_handle);
        NotesTerm();
        return (1);
    }

    /* Write a field named FORM to the note -- this field specifies the
       default form to use when displaying the note. */

    if (error = NSFItemSetText ( note_handle,
                                 "FORM",
                                 "SimpleDataForm",
                                 MAXWORD))
    {
        PRINTERROR (error,"NSFItemSetText");
        NSFNoteClose (note_handle);
        NSFDbClose (db_handle);
        NotesTerm();
        return (1);
    }

    /* Write a text field named PLAIN_TEXT to the note. */

    if (error = NSFItemSetText ( note_handle,
                                 "PLAIN_TEXT",
                                 "The quick brown fox jumped over the lazy dogs.",
                                 MAXWORD))
    {
        PRINTERROR (error,"NSFItemSetText");
        NSFNoteClose (note_handle);
        NSFDbClose (db_handle);
        NotesTerm();
        return (1);
    }

    /* Write a field named NUMBER to the note. */

    num_field = 125.007;

    if (error = NSFItemSetNumber (note_handle, "NUMBER", &num_field))
    {
        PRINTERROR (error,"NSFItemSetNumber");
        NSFNoteClose (note_handle);
        NSFDbClose (db_handle);
        NotesTerm();
        return (1);
    }

    /* Get the current time/date and write it to a field named TIME_DATE. */

    OSCurrentTIMEDATE(&timedate);

    if (error = NSFItemSetTime (note_handle, "TIME_DATE", &timedate))
    {
        PRINTERROR (error,"NSFItemSetTime");
        NSFNoteClose (note_handle);
        NSFDbClose (db_handle);
        NotesTerm();
        return (1);
    }

    /* Create a text-list field and add it to the note. */

    if (error = NSFItemCreateTextList ( note_handle,
                                        "TEXT_LIST",
                                        "Charles",
                                        MAXWORD))
    {
        PRINTERROR (error,"NSFItemCreateTextList");
        NSFNoteClose (note_handle);
        NSFDbClose (db_handle);
        NotesTerm();
        return (1);
    }

    /* Add several items to the text-list field. */

    if (error = NSFItemAppendTextList ( note_handle,
                                        "TEXT_LIST",
                                        "Janet",
                                        MAXWORD,
                                        TRUE))
    {
        PRINTERROR (error,"NSFItemAppendTextList");
        NSFNoteClose (note_handle);
        NSFDbClose (db_handle);
        NotesTerm();
        return (1);
    }

    if (error = NSFItemAppendTextList ( note_handle,
                                        "TEXT_LIST",
                                        "Chuck",
                                        MAXWORD,
                                        TRUE))
    {
        PRINTERROR (error,"NSFItemAppendTextList");
        NSFNoteClose (note_handle);
        NSFDbClose (db_handle);
        NotesTerm();
        return (1);
    }

    /* Add the entire new note (with all fields) to the database. */

    if (error = NSFNoteUpdate (note_handle, 0))
    {
        PRINTERROR (error,"NSFNoteUpdate");
        NSFNoteClose (note_handle);
        NSFDbClose (db_handle);
        NotesTerm();
        return (1);
    }

    /* Close the note. (Remove its structure from memory.) */

    if (error = NSFNoteClose (note_handle))
    {
        PRINTERROR (error,"NSFNoteClose");
        NSFDbClose (db_handle);
        NotesTerm();
        return (1);
    }

    /* Close the database */

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
