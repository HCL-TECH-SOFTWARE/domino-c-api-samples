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

    PROGRAM:    des_coll

    FILE:       des_coll.c

    SYNTAX:     des_coll  <database path>

    PURPOSE:    Shows how to read the design collection for a database.
                Prints the type and title of each design note found.

    DESCRIPTION:
        Every database contains a design collection. The design collection
        is like a view collection that contains entries corresponding to 
        every form, view, and macro in the database. To read the design 
        collection, call to NIFOpenCollection specifying ViewNoteID = 
        NOTE_ID_SPECIAL+NOTE_CLASS_DESIGN. This program reads through the 
        design collection and, for each design note, prints the note class 
        (Form, View, etc.) and the title.
                                                                         
*************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* OS and C include files */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <osmem.h>
#include <stdnames.h>
#include <names.h>
#include <misc.h>
#include <idtable.h>
#include <nsfdata.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <nsferr.h>
#include <nif.h>
#include <ods.h>
#include <textlist.h>
#include <osmisc.h>
#include "../../printLog.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

void  LNPUBLIC  ProcessArgs (int argc, char *argv[], char *db_filename);

void LNPUBLIC PrintTitle (DWORD dwItem, WORD wClass, BYTE *summary);


#define  STRING_LENGTH  256

/************************************************************************

    FUNCTION:   main

*************************************************************************/

int main (int argc, char *argv[])
{
   STATUS             error=0;
   char               *szPathName;
   DBHANDLE           hDB;
   char               szDBInfo[NSF_INFO_SIZE];
   char               szDBTitle[NSF_INFO_SIZE];
   HCOLLECTION        hCollection;
   COLLECTIONPOSITION CollPosition;
   DHANDLE            hBuffer;
   BYTE               *pBuffer;
   BYTE               *pSummary;
   DWORD              dwEntriesFound, i;
   ITEM_TABLE         ItemTable;
   WORD               wClass;


   if (error = NotesInitExtended (argc, argv))
   {
       PRINTERROR (error,"NotesInitExtended");
       NotesTerm();
       return (1);
   }


   /* Allocate memory for an input database path */
   szPathName = (char *) malloc(STRING_LENGTH);
   if (szPathName == NULL)
   {
       PRINTLOG("Error: Out of memory.\n");
       NotesTerm();
       return (0);
   }

   /* Get the pathname of the database. */
   ProcessArgs(argc, argv, szPathName);

  /* Open the database */
   if (error = NSFDbOpen (szPathName, &hDB))
   {
       PRINTLOG ("Error: unable to open database '%s'.\n", szPathName);
       free(szPathName);
       PRINTERROR (error,"NSFDbOpen");
       return (1);
   }

   if (error = NSFDbInfoGet (hDB, szDBInfo))
   {
       PRINTLOG("Error: unable to get title of database '%s'\n.", szPathName);
       NSFDbClose (hDB);
       free(szPathName);
       PRINTERROR (error,"NSFDbInfoGet");
       return (1);
   }

   NSFDbInfoParse (szDBInfo, INFOPARSE_TITLE, szDBTitle, NSF_INFO_SIZE-1);
 
   PRINTLOG("Printing design collection for '%s'\n", szDBTitle);

   /* Open the special "design" collection, which contains an index of
      all nondata notes. */

   if (error = NIFOpenCollection(hDB, 
                                 hDB, 
                                 NOTE_ID_SPECIAL+NOTE_CLASS_DESIGN,
                                 OPEN_DO_NOT_CREATE,
                                 NULLHANDLE,
                                 &hCollection,
                                 NULL, NULL, NULL, NULL))
   {   /* Collection may not have been setup yet by the first user to 
          open the file.*/
       PRINTLOG("Unable to open the design collection for '%s'.\n", szDBTitle);
       NSFDbClose (hDB);
       free(szPathName);
       PRINTERROR (error,"NIFOpenCollection");
       return (1);
   }

   CollPosition.Level = 0;
   CollPosition.Tumbler[0] = 1;

   if (error = NIFReadEntries(
                              hCollection,           /* handle to this collection */
                              &CollPosition,         /* where to start in collection */
                              NAVIGATE_CURRENT,      /* order to use when skipping */
                              0L,                    /* number to skip */
                              NAVIGATE_NEXT,         /* order to use when reading */
                              0xFFFFFFFF,            /* max number to read */
                              READ_MASK_NOTECLASS +  /* read the note class */
                              READ_MASK_SUMMARY,     /* and the summary buffer */
                              &hBuffer,              /* handle to info buffer (return)  */
                              NULL,                  /* length of info buffer (return) */
                              NULL,                  /* entries skipped (return) */
                              &dwEntriesFound,       /* entries read (return) */
                              NULL))
   {
       PRINTLOG("No entries found in design collection for '%s'.\n", szDBTitle);
       NIFCloseCollection (hCollection);
       NSFDbClose (hDB);
       free(szPathName);
       PRINTERROR (error,"NIFReadEntries");
       return (1);
   }

   /* Check to make sure there was a buffer of information returned. */ 

   if (hBuffer == NULLHANDLE)
   {
       PRINTLOG ("Empty buffer returned reading entries in design collection.\n");
       NIFCloseCollection (hCollection);
       NSFDbClose (hDB);
       free(szPathName);
       PRINTERROR (error,"NIFReadEntries");
       return (1);
   }

   pBuffer = (BYTE *) OSLockObject (hBuffer);

   PRINTLOG ("Found %ld design notes in '%s'.\n",dwEntriesFound, szDBTitle);
   for (i = 0; i < dwEntriesFound; i++)
   {
       wClass = *(WORD*) pBuffer;
       pBuffer += sizeof (WORD);

   ItemTable = *(ITEM_TABLE*) pBuffer;

   /* Remember where the start of this entry's summary is. Then advance
   the main pointer over the summary. */
   pSummary = pBuffer;
   pBuffer += ItemTable.Length; 
 
   /* Call PrintTitle to print the type and title of the design note. */
   PrintTitle (i, wClass, pSummary);

   } /* End of loop that processes each entry. */

   /* Unlock the buffer returned from NIFReadEntries. */
   OSUnlockObject (hBuffer);

   /* Free the memory allocated by NIFReadEntries. */
   OSMemFree (hBuffer);

   if (error = NIFCloseCollection(hCollection))
   {
       NSFDbClose(hDB);
       free(szPathName);
       PRINTERROR (error,"NIFCloseCollection");
       return (1);
   }

   NSFDbClose(hDB);
   free(szPathName);
   return (0);

}

/************************************************************************

    FUNCTION:   PrintTitle

    PURPOSE:    print title and type of a design note

    DESCRIPTION:
        Prints the type of the design note (e.g. "Form") based on the
        Class (e.g. NOTE_CLASS_FORM). Then it gets the title of the
        form or view from the summary buffer and prints it.

*************************************************************************/

void LNPUBLIC PrintTitle (DWORD dwItem, WORD wClass, BYTE *summary)
{
   char       *szNoteType; /* e.g. "Form" */
   char       *pItemValue; /* initialized by NSFLocateSummaryValue */
   WORD        Length, Type;
   char        szTitleString[DESIGN_NAME_MAX];
   char        HelpAbout[] = "help-about";
   char        Form[] = "Form";
   char        View[] = "View";
   char        Icon[] = "Icon";
   char        DesColl[] = "design collection";
   char        ACL[] = "A.C.L.";
   char        NotesHelp[] = "Notes Help";
   char        DBHelp[] = "Database Help";
   char        Macro[] = "Macro";
   char        Field[] = "Field";
   char        RepFormula[] = "replication formula";
   char        Unknown[] = "unknown";
        
   if (wClass & NOTE_CLASS_INFO)
       szNoteType = HelpAbout;
   else if (wClass & NOTE_CLASS_FORM)
       szNoteType = Form;
   else if (wClass & NOTE_CLASS_VIEW)
       szNoteType = View;
   else if (wClass & NOTE_CLASS_ICON)
       szNoteType = Icon;
   else if (wClass & NOTE_CLASS_DESIGN)
       szNoteType = DesColl;
   else if (wClass & NOTE_CLASS_ACL)
       szNoteType = ACL;
   else if (wClass & NOTE_CLASS_HELP_INDEX)
       szNoteType = NotesHelp;
   else if (wClass & NOTE_CLASS_HELP)
       szNoteType = DBHelp;
   else if (wClass & NOTE_CLASS_FILTER)
       szNoteType  = Macro;
   else if (wClass & NOTE_CLASS_FIELD)
       szNoteType  = Field;          /* Notes 3.0 shared field */
   else if (wClass & NOTE_CLASS_REPLFORMULA)
       szNoteType  = RepFormula;
   else 
       szNoteType = Unknown;

   if (NSFLocateSummaryValue(summary, 
                             ITEM_NAME_TEMPLATE_NAME, /* "$TITLE" */
                             &pItemValue, 
                             &Length, 
                             &Type))
   {
       if (TYPE_TEXT_LIST == Type)
       {
           WORD    ListCount = 0;
           char    *pText;

           ListGetText((LIST *)pItemValue, FALSE, 0, &pText, &Length );
           memset( szTitleString, '\0', DESIGN_NAME_MAX );
           memcpy( szTitleString, pText, Length );
       }

       else if (TYPE_TEXT == Type) 
       {
           /* Make sure string is smaller than buffer! */
           if (Length >= DESIGN_NAME_MAX)
               Length = DESIGN_NAME_MAX - 1;
               memcpy (szTitleString, pItemValue, Length);
               szTitleString[Length] = '\0';
       }
       else
           strcpy (szTitleString, "unknown data type");
   }
   else
   {
       strcpy (szTitleString, "not available");
   }
        
   PRINTLOG ("\tDesign Note %ld : Class = %s", dwItem+1, szNoteType);
   PRINTLOG ("\tTitle = '%s'\n", szTitleString);

   return;
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
