/************************************************************************
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

    PROGRAM:    srch_all

    FILE:       srch_all.c
                          
    PURPOSE:    Change the "category" field of all docs with category   
                initially "p" to  "q" and set the category of all docs  
                with category initially "q" to "r".                     
                                                                        
    SYNTAX:     srch_all  <database name>                                
                                                                        
    DESCRIPTION:                                                        
        Demonstrates how to use NSFSearch to process all documents in   
        a database without the risk of processing the same note more    
        once.  This ensures that each document gets processed once and  
        only once by building an ID table that is guaranteed to contain 
        exactly one instance of each note ID. Once the search is        
        complete and the ID table is full, then this iterates over IDs
        in the table and processes each note once.
                                                                        
        Opens the specified discussion database and searches for all    
        data notes. Opens each data note and, if the "categories" field 
        contains "p", then this sets the categories field to "q". Else, 
        if the field contains "q", then this sets it to "r".

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
#if defined(NT) || defined (UNIX)
   #include <ctype.h>
#endif

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <nsfsearc.h>
#include <osmem.h>
#include <nsferr.h>
#include <idtable.h>            /* IDCreateTable */
#include <osmisc.h>
#include "../../printLog.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif


#define     ITEM_NAME_CATEGORIES    "Categories"
#define     CATEGORIES_VALUE_P      "p"
#define     CATEGORIES_VALUE_Q      "q"
#define     CATEGORIES_VALUE_R      "r"


/* Function prototypes */

STATUS LNPUBLIC AddIDUnique (void far *, SEARCH_MATCH far *, ITEM_TABLE far *);
STATUS LNPUBLIC ChangeCategory (void far *, DWORD);
void  LNPUBLIC  ProcessArgs (char *DBFileName);


/************************************************************************

   FUNCTION:   Main

   PURPOSE:    Main Notes HCL C API for Notes/Domino routine

   DESCRIPTION:
         Search the database using NSFSearch with an action routine 
         that inserts each note ID into an ID table. The action routine 
         does not itself open or modify the document. This avoids the 
         possibility of NSFSearch finding the same note again later in 
         the search and calling the action routine a second time to 
         process the same note.

         This technique is valuable when the algorithm requires each 
         note to be processed once and only once.
                                                               
*************************************************************************/
int main (int argc, char *argv[])
{
    char      * szDBName;
    DBHANDLE    hDB;
    STATUS      error;
    DHANDLE       hNoteIDTable;

    char       pname[MAXPATH] = "";         /* buffer to store the input path to database */
    char       *path_name;                  /* pathname of database */
    char       *db_name;                    /* name of database */




/* Initialize pointers to point to "" */
    path_name = pname;
    db_name = pname;


/*  Command line parameters. */

    if (argc != 2)
    {
        PRINTLOG ("\nUsage:  %s  <db filename>  \n", argv[0]);
        return (NOERROR);
    }
    
    szDBName = argv[1];


    /* Initialize Notes */
    if (error = NotesInitExtended (argc, argv))
    {
        PRINTLOG("\n Unable to initialize Notes.\n");
        return (1);
    }


    /*  Open the database. */

    if (error = NSFDbOpen (szDBName, &hDB))
    {
        PRINTLOG ("Error: unable to open database '%s'.\n", szDBName);
        return (ERR(error));
    }

    /*  Create ID table then call to NSFSearch.  NSFSearch will find 
        all data notes and call the action routine, AddIDUnique, on each.
        AddIDUnique adds the ID of the data note to the ID table if and
        only if the ID is not already in the table.  The result is a table
        of Note IDs where each ID is guaranteed to appear only once.
     */

    if (error = IDCreateTable(sizeof(NOTEID), &hNoteIDTable))
    {
        PRINTLOG ("Error: unable to create ID table.\n");
        NSFDbClose (hDB);
        return (ERR(error));
    }

    if (error = NSFSearch (
                           hDB,            /* database handle */
                           NULLHANDLE,     /* selection formula (select all notes) */
                           NULL,           /* title of view in selection formula */
                           0,              /* search flags */
                           NOTE_CLASS_DOCUMENT,/* note class to find */
                           NULL,           /* starting date (unused) */
                           AddIDUnique,    /* call for each note found */
                           &hNoteIDTable,  /* argument to AddIDUnique */
                           NULL))          /* returned ending date (unused) */

    {
        PRINTLOG ("Error: unable to search database.\n");
        IDDestroyTable(hNoteIDTable);
        NSFDbClose (hDB);
        return (ERR(error));
    }

    /*  Loop over note IDs in the table. Call ChangeCategory on each. */
    if (error = IDEnumerate(hNoteIDTable, 
                            ChangeCategory, /* called for each ID */
                            &hDB))          /* arg passed to func */
    {
        PRINTLOG ("Error: unable to enumerate documents in ID table.\n");
    }

    IDDestroyTable(hNoteIDTable);

    NSFDbClose (hDB);

    if (error == NOERROR)
        PRINTLOG("\nProgram completed successfully.\n");

    NotesTerm();
    return (0); 

}

/************************************************************************
                                                                        
   FUNCTION:   AddIDUnique

   PURPOSE:    This is the action routine called for each note found
               by NSFSearch.

*************************************************************************/

STATUS LNPUBLIC AddIDUnique    
                           (void far * phNoteIDTable,
                            SEARCH_MATCH far *pSearchMatch,
                            ITEM_TABLE far *summary_info)
{
    SEARCH_MATCH SearchMatch;
    DHANDLE    hNoteIDTable;
    STATUS       error;
    BOOL         flagOK;

    memcpy( (char*)&SearchMatch, (char*)pSearchMatch, sizeof(SEARCH_MATCH) );

    if (!(SearchMatch.SERetFlags & SE_FMATCH))
        return (NOERROR);


    hNoteIDTable = *((DHANDLE far *)phNoteIDTable);

    if (error = IDInsert(hNoteIDTable, SearchMatch.ID.NoteID, &flagOK))
    {
        PRINTLOG ("Error: unable to insert note ID into table.\n");
        return (ERR(error));
    }
    if (flagOK == TRUE)
    {
        PRINTLOG ("\tInserted note %lX into table.\n", SearchMatch.ID.NoteID);
    }
    else
    {
        PRINTLOG ("\tNote %lX is already in table.\n", SearchMatch.ID.NoteID);
    }

    return (NOERROR);
}

/************************************************************************
                                                                        
    FUNCTION:   ChangeCategory                                          
                                                                        
*************************************************************************/

STATUS LNPUBLIC ChangeCategory (void far * phDB, DWORD NoteID)
{
    DBHANDLE    hDB;
    STATUS      error;
    NOTEHANDLE  hNote;
    BOOL        field_found;
    WORD        field_len;
    char        field_text[500];


    PRINTLOG ("\tProcessing note %lX.\n", NoteID);

    hDB = *( (DBHANDLE far *)phDB );

    if (error = NSFNoteOpen (
                             hDB,
                             NoteID,
                             0,
                             &hNote))
    {
        PRINTLOG ("Error: unable to open note.\n");
        return (ERR(error));
    }

    /*  Look for the "Categories" field within this note. */

    field_found = NSFItemIsPresent ( 
                                     hNote,
                                     ITEM_NAME_CATEGORIES,       /* "Categories" */
                                     (WORD) strlen (ITEM_NAME_CATEGORIES));

    /* If the Categories field is there, get the contents of the field and 
        check to see if it equals "p".  */

    if (!field_found)
    {
        PRINTLOG ("%s field not found in this note.\n",ITEM_NAME_CATEGORIES);
        NSFNoteClose (hNote);
        return (NOERROR);
    }

    field_len = NSFItemGetText ( 
                                 hNote, 
                                 ITEM_NAME_CATEGORIES,
                                 field_text,
                                 sizeof (field_text));

    /* Category names are case insensitive.  Do a case insensitive 
       comparison. Since we are just interested in categories that 
       have names of one letter, just ensure that field names
       of only one letter is in lower case. */
   
    if (field_len == 1)
        field_text[1] = tolower (field_text[1]);
  

    if (strcmp(field_text, CATEGORIES_VALUE_P) == 0)
    {
        /* Change "p" to "q" */

        if (error = NSFItemDelete(hNote, 
                                  ITEM_NAME_CATEGORIES, 
                                  (WORD) strlen(ITEM_NAME_CATEGORIES)))
        {
            PRINTLOG ("Error: unable to delete item '%s' from note.\n",ITEM_NAME_CATEGORIES);
            NSFNoteClose (hNote);
            return (ERR(error));
        }
        if (error = NSFItemSetText(hNote,
                                   ITEM_NAME_CATEGORIES,
                                   CATEGORIES_VALUE_Q, 
                                   (WORD) strlen(CATEGORIES_VALUE_Q)))
        {
            PRINTLOG ("Error: unable to set item '%s' to value '%s' in note.\n",
                      ITEM_NAME_CATEGORIES, CATEGORIES_VALUE_Q);
            NSFNoteClose (hNote);
            return (ERR(error));
        }
    }
    else if (strcmp(field_text, CATEGORIES_VALUE_Q) == 0)
    {
        /* Change "q" to "r" */

        if (error = NSFItemDelete(hNote, 
                                  ITEM_NAME_CATEGORIES, 
                                  (WORD) strlen(ITEM_NAME_CATEGORIES)))
        {
            PRINTLOG ("Error: unable to delete item '%s' from note.\n",ITEM_NAME_CATEGORIES);
            NSFNoteClose (hNote);
            return (ERR(error));
        }
        if (error = NSFItemSetText(hNote,
                                   ITEM_NAME_CATEGORIES,
                                   CATEGORIES_VALUE_R, 
                                   (WORD) strlen(CATEGORIES_VALUE_R)))
        {
            PRINTLOG ("Error: unable to set item '%s' to value '%s' in note.\n",
                      ITEM_NAME_CATEGORIES, CATEGORIES_VALUE_R);
            NSFNoteClose (hNote);
            return (ERR(error));
        }
    }

    if (error = NSFNoteUpdate (hNote, 0))
    {
        PRINTLOG ("Error: unable to update note.\n");
        NSFNoteClose (hNote);
        return (ERR(error));
    }

    if (error = NSFNoteClose (hNote))
    {
        PRINTLOG ("Error: unable to close.\n");
        return (ERR(error));
    }

    return (NOERROR);
}

/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:

    OUTPUTS:    DBFileName, inputted by user

 *************************************************************************/

void  LNPUBLIC  ProcessArgs (char *DBFileName)
{
    printf("Enter name of database: ");
    fflush(stdout);
    gets(DBFileName);

} /* ProcessArgs */

#ifdef __cplusplus
}
#endif

