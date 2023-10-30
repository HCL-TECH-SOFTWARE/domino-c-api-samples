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

    PROGRAM:    allflds

    FILE:       allflds.c

    SYNTAX:     allflds

    PURPOSE:    finds the names and data types of all the fields in all 
                the documents of a database.

    DESCRIPTION:
        This program uses NSFSearch to find all data notes in the 
        database. For each note found, it calls NSFItemScan to find
        all the fields in the note.  For each item found, it prints
        the item name and the data type.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* OS and C include files */

#include <stdio.h>
#include <string.h>
#include <memory.h>

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <nsfsearc.h>
#include <nsferr.h>
#include <osmisc.h>
#include "../../printLog.h"


/* Function prototypes */

STATUS LNPUBLIC note_action (     /* called for every document */
                              VOID far *,
                              SEARCH_MATCH far *,
                              ITEM_TABLE far *);

STATUS LNPUBLIC field_action (    /* called for every field */
                               WORD,
                               WORD,
                               char far *,
                               WORD,
                               VOID far *,
                               DWORD,
                               VOID far *);

/************************************************************************

    FUNCTION:   main

    PURPOSE:    Main HCL C API for Notes/Domino subroutine

*************************************************************************/

int main(int argc, char *argv[])
{

/* Local data declarations */

    char        db_filename[]="test.nsf";   /* pathname of source database */
    DBHANDLE    db_handle;                  /* handle of source database */ 
    STATUS      error = NOERROR;            /* return status from API calls */

   
    if (error = NotesInitExtended (argc, argv))
    {
        PRINTLOG("\n Unable to initialize Notes.\n");
        return (1);
    }

      
/* Open the database. */
   
    if (error = NSFDbOpen (db_filename, &db_handle))
    {
        PRINTERROR (error,"NSFDbOpen");  
        NotesTerm();
        return (1); 
    }

  
/* Call NSFSearch to find all data notes in the database. */

    if (error = NSFSearch (
                           db_handle,        /* database handle */
                           NULLHANDLE,       /* selection formula */
                           NULL,             /* title of view in selection formula */
                           0,                /* search flags */
                           NOTE_CLASS_DOCUMENT,  /* note class to find */
                           NULL,             /* starting date (unused) */
                           note_action,      /* action routine for notes found */
                           &db_handle,       /* argument to action routine */
                           NULL))            /* returned ending date (unused) */

    {
        NSFDbClose (db_handle);
        PRINTERROR (error,"NSFSearch");  
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


/* End of main routine. */
    PRINTLOG("\nProgram completed successfully.\n");
    NotesTerm();
    return (0); 

}



/************************************************************************

    FUNCTION:   note_action

    PURPOSE:    This is the routine that is called by NSFSearch for 
                each note that matches the selection criteria.

    INPUTS:
        The first argument to this function is the optional argument
        that we supplied when we called NSFSearch.

        The second argument is supplied by NSFSearch.    It is
        a structure of information about the note that was found.

        The third argument is also supplied by NSFSearch and is
        the summary buffer for this note. 

*************************************************************************/

STATUS LNPUBLIC note_action
                           (VOID far *db_handle,
                            SEARCH_MATCH far *pSearchMatch,
                            ITEM_TABLE far *summary_info)
{
    SEARCH_MATCH  SearchMatch;
    NOTEHANDLE    note_handle;
    STATUS        error;
    
    memcpy( (char*)&SearchMatch, (char*)pSearchMatch, sizeof(SEARCH_MATCH) );

/* Skip this note if it does not really match the search criteria (it is
now deleted or modified).  This is not necessary for full searches,
but is shown here in case a starting date was used in the search. */

/*  if (SearchMatch.MatchesFormula != SE_FMATCH)      V3 */
    if (!(SearchMatch.SERetFlags & SE_FMATCH))     /* V4 */
        return (NOERROR);

/* Print the note ID. */

    PRINTLOG ("\nNote ID is: %lX.\n", SearchMatch.ID.NoteID);

/* Open the note. */

    if (error = NSFNoteOpen (
                             *(DBHANDLE far *)db_handle,    /* database handle */
                             SearchMatch.ID.NoteID,    /* note ID */
                             0,            /* open flags */
                             &note_handle))        /* note handle (return) */
        
        return (ERR(error));

/* Scan all the fields in the note, calling an action routine for each. */

    if (error = NSFItemScan (
                             note_handle,    /* note handle */
                             field_action,    /* action routine for fields */
                             &note_handle))    /* argument to action routine */
    {
        NSFNoteClose (note_handle);
        return (ERR(error));
    }

/* Close the note. */

    if (error = NSFNoteClose (note_handle))
        return (ERR(error));

/* End of subroutine. */

    return (NOERROR);

}



/************************************************************************

    FUNCTION:   field_action

    PURPOSE:    This is the action routine that is called by NSFItemScan 
                for each field in the note.

    DESCRIPTION:
        The routine prints the name and data type of the field.

*************************************************************************/

STATUS LNPUBLIC field_action
                            (WORD unused,
                             WORD item_flags,
                             char far *name_ptr,
                             WORD name_len,
                             VOID far *item_value,
                             DWORD item_value_len,
                             VOID far *note_handle)
{
    char    item_name[50];        /* name of the field */
    WORD    binary_datatype;    /* datatype of field in binary */
    char    ascii_datatype[50];    /* datatype of field in ASCII */

/* Copy the item name into our local data area and put a null at the end
of it. We do this in order to treat the name as a standard C string. */

    strncpy(item_name, name_ptr, name_len);
    item_name[name_len] = '\0';

/* Get the data type of the field. */

    memcpy((char far *)(&binary_datatype), item_value, sizeof(WORD));

/* Translate the binary data type to ASCII. */

    switch (binary_datatype)
    {
        case TYPE_TEXT:
            strcpy (ascii_datatype, "Text");
            break;
        case TYPE_TEXT_LIST:
            strcpy (ascii_datatype, "Multi-Value Text");
            break;
        case TYPE_NUMBER:
            strcpy (ascii_datatype, "Number");
            break;
        case TYPE_NUMBER_RANGE:
            strcpy (ascii_datatype, "Multi-Value Number");
            break;
        case TYPE_TIME:
            strcpy (ascii_datatype, "Time/Date");
            break;
        case TYPE_TIME_RANGE:
            strcpy (ascii_datatype, "Multi-Value Time/Date");
            break;
        case TYPE_COMPOSITE:
            strcpy (ascii_datatype, "Rich Text");
            break;
        default:
            strcpy (ascii_datatype, "Unknown");
            break;
    }

/* Print the field name and data type. */

    PRINTLOG ("Found field named \"%s\" with data type \"%s\".\n",
    item_name, ascii_datatype);

/* End of subroutine. */

    return (NOERROR);

}
#ifdef __cplusplus
}
#endif

