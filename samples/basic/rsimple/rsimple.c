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

    PROGRAM:   rsimple

    FILE:      rsimple.c 

    PURPOSE:   Shows how to find all the data notes (documents) in a Domino
               database and read the fields with simple data types.

    SYNTAX:    rsimple  <database>
                         or
               rsimple

    COMMENTS:
       This program uses the API function NSFSearchExt2() to find all the
       data notes in the given database. For each data note that it 
       finds, this reads several fields of simple data types and prints
       their values to the screen.

*************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* C include files */

#include <stdio.h>
#include <string.h>

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <nsfsearc.h>
#include <osmem.h>
#include <nsferr.h>
#include "../../printLog.h"
#include <osmisc.h>

#if defined(OS390)
#include <_Ascii_a.h>    /* NOTE: must be the LAST file included */
#include <lapicinc.h>
#endif /* OS390 */

              
void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                         char *db_filename); 

#define  STRING_LENGTH  256
              

/* Function prototypes */

STATUS LNPUBLIC print_fields (void far *, SEARCH_MATCH far *, 
                              ITEM_TABLE far *);

/************************************************************************

    FUNCTION:   main

 ************************************************************************/

int main(int argc, char *argv[])
{
    char       *db_filename;    /* pathname of source database */
    DBHANDLE    db_handle;      /* handle of source database */
    char        formula[] = "@All"; /* an ASCII selection formula. */
    FORMULAHANDLE    formula_handle;    /* a compiled selection formula */
    WORD     wdc;                       /* a word we don't care about */
    STATUS   error = NOERROR;           /* return status from API calls */
    char     database_name[STRING_LENGTH];
   
    db_filename = database_name;

#ifdef OS390
    /* Need to convert args from EBCDIC (Native z/OS) to ASCII (how this sample is compiled) */
    __argvtoascii_a(argc,argv);
#endif
    ProcessArgs(argc, argv, db_filename); 
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

    /* Compile the selection formula. */

    if (error = NSFFormulaCompile (
                                   NULL,               /* name of formula (none) */
                                   (WORD) 0,           /* length of name */
                                   formula,            /* the ASCII formula */
                                   (WORD) strlen(formula),    /* length of ASCII formula */
                                   &formula_handle,    /* handle to compiled formula */
                                   &wdc,               /* compiled formula length (don't care) */
                                   &wdc,               /* return code from compile (don't care) */
                                   &wdc, &wdc, &wdc, &wdc)) /* compile error info (don't care) */
        
    {
        NSFDbClose (db_handle);
        PRINTERROR (error,"NSFFormulaCompile");  
        NotesTerm();
        return (1);
    }

/* Call NSFSearchExt2 to find the notes that match the selection criteria. For
each note found, the routine print_fields is called. (If you always want
to find all the documents in the database, you can set the 2nd argument
to NULLHANDLE and eliminate the formula compilation.) */

    if (error = NSFSearchExt2 (
                               db_handle,      /* database handle */
                               formula_handle, /* selection formula */
                               NULLHANDLE,   /* handle to the filter */
                               SEARCH_FILTER_NONE, /* filter flags */
                               NULL,           /* title of view in selection formula */
                               0,              /* search flags (unused) */
                               0,              /* search flags (unused) */
                               NOTE_CLASS_DOCUMENT,/* note class to find */
                               NULL,           /* starting date (unused) */
                               print_fields,   /* call for each note found */
                               &db_handle,     /* argument to print_fields */
                               NULL,           /* returned ending date (unused) */
                               5))            /* timeout seconds. if = "0" then, NO timeout */
    {
        OSMemFree (formula_handle);
        NSFDbClose (db_handle);
        PRINTERROR (error,"NSFSearchExt2");  
        NotesTerm();
        return (1);
    }

/* Free the memory allocated to the compiled formula. */

    OSMemFree (formula_handle);

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

    FUNCTION:   print_fields

    PURPOSE:    Called by NSFSearchExt2 for each note that matches the
                selection criteria

    INPUTS:
        void *db_handle - the optional argument that we supplied when 
                          we called NSFSearchExt2
        SEARCH_MATCH far *pSearchMatch - 
                          supplied by NSFSearchExt2.  This is a
                          structure of information about the note that 
                          was found.
        ITEM_TABLE *summary_info - 
                          also supplied by NSFSearchExt2.  This is
                          the summary buffer for this note.

    RETURNS:   
        NOERROR or STATUS if an error condition is encountered.

    DISCUSSION:
        This is the routine that is called by NSFSearchExt2 for each note
        that matches the selection criteria.

        This routine uses the Note ID provided in the SEARCH_MATCH 
        structure to open and read fields in the note. 

        Note that it is ok to open and read the note here in the action 
        routine, but you should not normally write new fields to the note 
        nor update the note unless the NSFSearchExt2 is time delimited. The
        reason is that updating the database while a non-time-delimited
        search is still in progress may cause the search to find the 
        same note again.

 *************************************************************************/

STATUS LNPUBLIC print_fields
            (void far *db_handle,
            SEARCH_MATCH far *pSearchMatch,
            ITEM_TABLE far *summary_info)
{
    SEARCH_MATCH SearchMatch;
    NOTEHANDLE   note_handle;
    BOOL         field_found;
    WORD         field_len;
    WORD         list_entries;
    char         field_text[500];
    NUMBER       number_field;
    STATUS       error;

    memcpy( (char*)&SearchMatch, (char*)pSearchMatch, sizeof(SEARCH_MATCH) );

/* Skip this note if it does not really match the search criteria (it is
now deleted or modified).  This is not necessary for full searches,
but is shown here in case a starting date was used in the search. */

    if (!(SearchMatch.SERetFlags & SE_FMATCH))
        return (NOERROR);

/* Print the note ID. */

    PRINTLOG ("\nNote ID is: %lX.\n", SearchMatch.ID.NoteID);

/* Open the note. */

    if (error = NSFNoteOpen (
                             *(DBHANDLE far *)db_handle,  /* database handle */
                             SearchMatch.ID.NoteID, /* note ID */
                             0,                      /* open flags */
                             &note_handle))          /* note handle (return) */
        
        return (ERR(error));

/* Look for the PLAIN_TEXT field within this note. */

    field_found = NSFItemIsPresent ( 
                                     note_handle,
                                     "PLAIN_TEXT",
                                     (WORD) strlen ("PLAIN_TEXT"));

/* If the PLAIN_TEXT field is there, get the contents of the field and 
print it. */

    if (field_found)
    {
        field_len = NSFItemGetText ( 
                                     note_handle, 
                                     "PLAIN_TEXT",
                                     field_text,
                                     (WORD) sizeof (field_text));

        PRINTLOG ("PLAIN_TEXT field is: %s\n", field_text);

    }

/* If the PLAIN_TEXT field is not there, print a message. */

    else
    {
        PRINTLOG ("PLAIN_TEXT field not found.\n");
    }
        
/* Look for (and get if it's there) the NUMBER field within this note.*/

    field_found = NSFItemGetNumber ( 
                                     note_handle, 
                                     "NUMBER",
                                     &number_field);

/* If the NUMBER field was found, print it. */

    if (field_found)
    {
#ifdef OS390
        /* Domino and Notes gave us an IEEE double which must be converted to native. */
        ConvertIEEEToDouble(&number_field, &number_field);
#endif  /* OS390 */
        PRINTLOG ("NUMBER field is: %f\n", number_field);
    }

/* If the NUMBER field was not found, print a message. */

    else 
    {
        PRINTLOG ("NUMBER field not found.\n"); 
    }
    
/* Look for the TIME_DATE field within this note. */

    field_found = NSFItemIsPresent ( 
                                     note_handle,
                                     "TIME_DATE",
                                     (WORD) strlen ("TIME_DATE"));

/* If the TIME_DATE field is there, get the contents of the field as an 
ASCII string and print it out. */

    if (field_found)
    {
        field_len = NSFItemConvertToText ( 
                                           note_handle, 
                                           "TIME_DATE",
                                           field_text,
                                           (WORD) sizeof (field_text),
                                           ';'); /* multi-value separator */

        PRINTLOG ("TIME_DATE field is: %s\n", field_text);

    }

/* If the TIME_DATE field is not there, print a message. */

    else
    {
        PRINTLOG ("TIME_DATE field not found.\n");
    }

/* Look for the TEXT_LIST field within this note. */

    field_found = NSFItemIsPresent ( note_handle,
                                     "TEXT_LIST",
                                     (WORD) strlen ("TEXT_LIST"));

/* Do the next few sections of code if the TEXT_LIST field is present. */

    if (field_found)
    { 

/* Find the number of entries in TEXT_LIST */
    
        list_entries = NSFItemGetTextListEntries ( 
                                                   note_handle, 
                                                   "TEXT_LIST");

/* Get the last entry in TEXT_LIST. (The fields are numbered from 0 to 
n-1. So we subtract one from the number returned above.) */

        field_len = NSFItemGetTextListEntry (
                                              note_handle,
                                              "TEXT_LIST",
                                              (WORD) (list_entries - 1),  /* which field to get */
                                              field_text,
                                              (WORD) (sizeof (field_text) -1) );        

/* Print out the last entry in TEXT_LIST. */

        PRINTLOG ("The last entry in TEXT_LIST is: %s\n", field_text);

    }  

/* If the TEXT_LIST field is not there, print a message. */

    else
    {
        PRINTLOG ("TEXT_LIST field not found.\n");
    }

/* Close the note. */

    if (error = NSFNoteClose (note_handle))
        return (ERR(error));

/* End of subroutine. */

    return (NOERROR);

}

/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:     argc, argv - directly from the command line
   
    OUTPUTS:    db_filename get data from the command line or from what 
                the user types at a prompt 

 *************************************************************************/

void  LNPUBLIC  ProcessArgs (int argc, char *argv[], char *db_filename)
{ 
    if (argc != 2)  
    {

        printf("Enter name of database: ");      
        fflush(stdout);
        gets(db_filename);
      
    }    
    else
    {
         strcpy(db_filename, argv[1]);    
    } /* end if */
} /* ProcessArgs */
#ifdef __cplusplus
}
#endif

