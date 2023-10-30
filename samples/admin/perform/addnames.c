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

    PROGRAM:    addnames

    FILE:       addnames.c

    DESCRIPTION:  
        This program adds any number of unique names to a Domino and Notes Name 
        and Address book. The program may be used as its own performance 
        test or to pre-load a N&A book for other tests.

        NOTE: This program DOES NOT build any index (view) on the N&A
        book. It simply adds the names. You can use the companion program
        named INDEX to build an index in the N&A book.


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
#include <ostime.h>
#include <misc.h>
#include <osmisc.h>
#include "../../printLog.h"

/* HCL C API for Notes/Domino subroutine */

int main (int argc, char *argv[])
{


/* Local data declarations */

    char           *db_path;         /* pathname of test database */
    int            transactions;     /* total number of transactions */

    DBHANDLE       db_handle;        /* database handle */
    NOTEHANDLE     note_handle;      /* note handle */

    char           last_name[100];   /* last name in N&A book */
    char           count[10];        /* used to construct unique names */

    LONG           time_delta;
    TIMEDATE       StartTime;        /* time to mark the start of the program */
    TIMEDATE       LastTime;         /*  time to mark the end of the program */

    int            i;
    STATUS         error = NOERROR;


/* ************************************************** */
/* Get the input parameters. */
/* ************************************************** */

    if (argc != 3)
    {
        PRINTLOG ("\nUsage:  ADDNAMES  <database pathname>  <number of names>\n");
        return(0);
    }
    db_path = argv[1];
    transactions = atoi(argv[2]);

    PRINTLOG ("\n***** ADDNAMES Test Begins ******\n");

    if (error = NotesInitExtended (argc, argv))
    {
        PRINTLOG("\nUnable to initialize Notes.\n");
        return(1);
    }

/* ************************************************** */
/* Get System TIME  . */
/* ************************************************** */

    OSCurrentTIMEDATE(&StartTime);

/* ************************************************** */
/* Open the database. */
/* ************************************************** */

    if (error = NSFDbOpen (db_path, &db_handle))
        goto Done1;


/* ************************************************** */
/* Add new records, each with a different last name. */
/* ************************************************** */

/* Start a big loop for adding names. */

    for (i=0; i<transactions; i++)
    {

/* Construct the last name. */

        strcpy (last_name, "Smith");
        sprintf(count, "%i", i); /* itoa (i, count, 10); */
        strcat (last_name, count);

/* Create the record. */

        if (error = NSFNoteCreate (db_handle, &note_handle))
            goto Done2;


/* Add all the fields. */

        if (error = NSFItemSetText ( note_handle, 
                                     "Form",
                                     "Person",
                                     MAXWORD))
            goto Done3;

        if (error = NSFItemSetText ( note_handle,
                                     "Type",
                                     "Person",
                                     MAXWORD))
            goto Done3;

        if (error = NSFItemSetText ( note_handle,
                                     "FirstName",
                                     "John",
                                     MAXWORD))
            goto Done3;

        if (error = NSFItemSetText ( note_handle,
                                     "LastName",
                                     last_name,
                                     MAXWORD))
            goto Done3;

        if (error = NSFItemSetText ( note_handle,
                                     "MailDomain",
                                     "NOTES",
                                     MAXWORD))
            goto Done3;

        if (error = NSFItemSetText ( note_handle,
                                     "MailFile",
                                     "MAIL\\JSMITH",
                                     MAXWORD))
            goto Done3;

/* Write the new note to disk and close the note. */

        if (error = NSFNoteUpdate (note_handle, 0))
            goto Done3;

        if (error = NSFNoteClose (note_handle))
            goto Done2;

/* End of big loop that is adding records. */

    }

/* ************************************************** */
/* Close the database */
/* ************************************************** */

    goto Done2;


Done3:
    NSFNoteClose (note_handle);

Done2:
    NSFDbClose (db_handle);

Done1:
    if (error)
    {
        PRINTERROR(error,"NSFDbOpen");
        NotesTerm();
        return(1);
    }

/* ************************************************** */
/* Get System TIME  . */
/* ************************************************** */

    OSCurrentTIMEDATE(&LastTime);

    time_delta= TimeDateDifference (&LastTime, &StartTime);

    PRINTLOG ("It took %i seconds to add %i names\n", time_delta, transactions);

/* ************************************************** */
/* End of subroutine. */
/* ************************************************** */

    NotesTerm();
    return(0);
}
