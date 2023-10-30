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

    PROGRAM:   itemlong

    FILE:      itemlong.c 

    PURPOSE:   Shows how to find all the data notes (documents) in a Domino
               database and read the fields with number and Datetime data types 
               and test compare operation with long data type.

    SYNTAX:    itemlong  <database>
                         or
               itemlong

    COMMENTS:
       This program uses the API function NSFSearch() to find all the
       data notes in the given database. For each data note that it 
       finds, this reads  fields of number,Datetime data types and prints
       their values to the screen. also prints the result of comparison operation.

*************************************************************************/

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
#include <ostime.h>

              
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
    char       *db_filename=NULL;    /* pathname of source database */
    DBHANDLE    db_handle;      /* handle of source database */
    FORMULAHANDLE    formula_handle;    /* a compiled selection formula */
    WORD     wdc;                       /* a word we don't care about */
    STATUS   error = NOERROR;           /* return status from API calls */
    char     database_name[STRING_LENGTH]={0};
   
    db_filename = database_name;


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

/* Call NSFSearch to find the notes that match the selection criteria. For 
each note found, the routine print_fields is called. (If you always want
to find all the documents in the database, you can set the 2nd argument
to NULLHANDLE and eliminate the formula compilation.) */

    if (error = NSFSearch (
                           db_handle,      /* database handle */
                           NULL, /* selection formula */
                           NULL,           /* title of view in selection formula */
                           0,              /* search flags */
                           NOTE_CLASS_DOCUMENT,/* note class to find */
                           NULL,           /* starting date (unused) */
                           print_fields,   /* call for each note found */
                           &db_handle,     /* argument to print_fields */
                           NULL))          /* returned ending date (unused) */
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

    FUNCTION:   print_fields

    PURPOSE:    Called by NSFSearch for each note that matches the 
                selection criteria and prints values and results of 
                comparison operation

    INPUTS:
        void *db_handle - the optional argument that we supplied when 
                          we called NSFSearch
        SEARCH_MATCH far *pSearchMatch - 
                          supplied by NSFSearch.  This is a
                          structure of information about the note that 
                          was found.
        ITEM_TABLE *summary_info - 
                          also supplied by NSFSearch.  This is
                          the summary buffer for this note.

    RETURNS:   
        NOERROR or STATUS if an error condition is encountered.

    DISCUSSION:
        This is the routine that is called by NSFSearch for each note
        that matches the selection criteria.

        This routine uses the Note ID provided in the SEARCH_MATCH 
        structure to open and read fields in the note. 

        Note that it is ok to open and read the note here in the action 
        routine, but you should not normally write new fields to the note 
        nor update the note unless the NSFSearch is time delimited. The
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
    char         field_text[500]={0};
    long       number_field;
    int         result=0;
    STATUS       error;
    char errStr[256]={0};
    TIMEDATE    tdNSFItemDateTime;
    TIMEDATE    tdCurrentDateTime;
    char retTextBuffer[MAXALPHATIMEDATE + 1]={0};
    WORD retTextLength;
    
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


/* Look for (and get if it's there) the NUMBER field within this note.*/

 field_found = NSFItemIsPresent ( 
                                  note_handle,
                                  "NUMBER",
                                  (WORD) strlen ("NUMBER"));
                
/* If the NUMBER field was found, print it. */
                
    if (field_found)
    {
        number_field = NSFItemGetLong ( 
                                        note_handle, 
                                        "number",
                                        0);

        PRINTLOG ("Number field is: %ld\n", number_field);
     /* checking NUMBER field is 1 */   
        if (NSFItemLongCompare(note_handle, "NUMBER", 1, &result))
        {
            if (result==0)
            {
                PRINTLOG("Number field comparison Success with Positive result.\n");
            }
            else 
            {
                PRINTLOG("Number field comparison Success with Negative result.\n");
            }
        }
        else
        {
            PRINTLOG("Error : Number field comparison failed.\n");
        }
    }
/* If the NUMBER field was not found, print a message. */
    else 
    {
        PRINTLOG ("Number field not found.\n"); 
    }

    

        
/* Look for the TIME_DATE field within this note. */

    field_found = NSFItemIsPresent ( 
                                     note_handle,
                                     "TIME_DATE",
                                     (WORD) strlen ("TIME_DATE"));

/* If the TIME_DATE field is there, get the contents of the field and compare 
it with current date to determine the item date is past date or future date */

    if (field_found)
    {
       
                         
        if (NSFItemGetTime(note_handle,"TIME_DATE",&tdNSFItemDateTime))
        {
            /* Converts binary TIMEDATE into a character text string */
            if(error=ConvertTIMEDATEToText(
                                           NULL,
                                           NULL,
                                           &tdNSFItemDateTime,
                                           retTextBuffer,
                                           MAXALPHATIMEDATE+1,
                                           &retTextLength))
            {
                OSLoadString(NULLHANDLE,ERR(error),errStr,100);
                PRINTLOG("Error in ConvertTIMEDATEToText : %s\n",errStr);
                return error;
            }
            else
            {
                PRINTLOG ("TIME_DATE field is: %s\n", retTextBuffer);
            }
        }
        else
        {
            PRINTLOG ("Error : Reading TIME_DATE value failed\n");
        }
        
        OSCurrentTIMEDATE(&tdCurrentDateTime);        
        
        result = 0;
        
        if (NSFItemTimeCompare(note_handle, "TIME_DATE",&tdCurrentDateTime, &result))
        {
            if (result == 1)
            {
                 PRINTLOG ("Item timedate %s is older.\n",retTextBuffer);
            }
            else if (result < 0)
            {
                 PRINTLOG ("Item timedate %s is later.\n",retTextBuffer);
            }
            else if (result == 0)
            {
                 PRINTLOG ("Item timedate %s is equal to current time.\n",retTextBuffer);
            }       
        }
        else
        {
            PRINTLOG ("Error : Comparing TIME_DATE value failed\n");
        }
        

    }

/* If the TIME_DATE field is not there, print a message. */

    else
    {
        PRINTLOG ("TIME_DATE field not found.\n");
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
        strncpy(db_filename, argv[1],strlen(argv[1]));            
    } /* end if */
} /* ProcessArgs */
