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

    PROGRAM:    w_usrdef

    FILE:       w_usrdef.c

    SYNTAX:     w_usrdef   <database filename>

    PURPOSE:    creates a field with a user-defined data type in a Domino 
                    and Notes document.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* C include files */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <ostime.h>
#include <osmisc.h>
#include "../../printLog.h"

/* Local include files */

#include "user_def.h"



int main(int argc, char *argv[])
{

    /* Local data declarations */
    char           *path_name;                  /* pathname of database */
    DBHANDLE       db_handle;                   /* database handle */
    NOTEHANDLE     note_handle;                 /* note handle */
    TIMEDATE       timedate;                    /* contents of a time/date field */
    BYTE           my_field[MAX_USERDEF_FIELD]; /* field with custom data type */
    WORD           descrip_len;                 /* len of type description */
    WORD           my_field_len;                /* actual len of custom field */
    STATUS         error=NOERROR;               /* return code from API calls */


    if (error = NotesInitExtended (argc, argv))
    {
        PRINTLOG("\n Unable to initialize Notes.\n");
        return (1);
    }

    /* Allocate memory for an input database path */
    path_name = (char *) malloc(STRING_LENGTH);
    if (path_name == NULL)
    {
        PRINTLOG("Error: Out of memory.\n");
        goto exit0;
    }
 
    /* Get the pathname of the database. */
    ProcessArgs(argc, argv, path_name);

    /* Open the database. */
    if (error = NSFDbOpen (path_name, &db_handle))
       goto exitFreeDBFile;

    /* Create a new data note. */
    if (error = NSFNoteCreate (db_handle, &note_handle))
       goto exitCloseDB;

    /* Write a field named FORM to the note -- this field specifies the
    default form to use when displaying the note. */
    if (error = NSFItemSetText ( note_handle, 
                                 "Form",
                                 "Form #1",
                                 MAXWORD))
       goto exitCloseNote;

    /* Write a text field named PLAIN_TEXT to the note. */
    if (error = NSFItemSetText(note_handle,
                               "PLAIN_TEXT",
                               "This doc has a user-defined field.",
                               MAXWORD))
       goto exitCloseNote;
	


    /* Get the current time/date and write it to a field named TIME_DATE. */
    OSCurrentTIMEDATE(&timedate);

    if (error = NSFItemSetTime (note_handle, "TIME_DATE", &timedate))
       goto exitCloseNote;

    /* Create a field with a user-defined data type. */
    /* The first byte of the field is the length of the type description
    string. */

    descrip_len = strlen (MY_DATA_TYPE);
    my_field[0] = (BYTE) descrip_len;
    my_field_len = 1;

    /* Next comes the type description string. You may put whatever you want
    in this string. Its purpose is to allow you to distinguish between
    different user-defined data types. */

    memcpy (my_field+my_field_len, MY_DATA_TYPE, descrip_len);
    my_field_len += descrip_len;
	PRINTLOG("\nNew userdefined datatype is %s", MY_DATA_TYPE);

    /* Next comes the actual data. Domino and Notes will make no attempt to 
	interpret the structure of this data or its meaning. (In this example, we 
	simply put some arbitrary hex numbers in the data.) */

    memset (my_field+my_field_len, 0xC3, MY_DATA_LEN);
    my_field_len += MY_DATA_LEN;

    /* Add the custom field to the note. We set the ITEM_SUMMARY flag so that
    the field can appear in a view selection formula or in an NSFSearch
    formula. */

    if (error = NSFItemAppend (note_handle, ITEM_SUMMARY,
                               MY_FIELD_NAME, (WORD)strlen(MY_FIELD_NAME),
                               TYPE_USERDATA,
                               &my_field, my_field_len))
		goto exitCloseNote;

    /* Add the entire new note (with all fields) to the database. */

    if (error = NSFNoteUpdate (note_handle, 0))
		goto exitCloseNote;
	else
		PRINTLOG("\nAdded userdefined datatype and documnets sucessfully to the database");
		


exitCloseNote:

    /* Close the note. (Remove its structure from memory.) */
    NSFNoteClose (note_handle);

exitCloseDB:
    /* Close the database */
    NSFDbClose (db_handle);

exitFreeDBFile:
    /* Free the the allocated space for database name*/
    free(path_name);   

exit0:
    if (error)
    PRINTERROR (error,"(char *) malloc");
    NotesTerm();
    return(error);

}
#ifdef __cplusplus
}
#endif
