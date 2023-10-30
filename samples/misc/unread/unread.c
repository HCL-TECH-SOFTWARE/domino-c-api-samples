/*
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
 *
 *  unread.c - Display the unread note table for a user
 *
 *  Usage:
 *      unread database [username] [+/-id] ...
 *
 *      where:
 *      database   Pathname of .nsf database file
 *      username   Optional name of user
 *      id         Note ID to add to (+) or remove from (-) unread list
 *
 *      If the username is hierarchical, the name must be fully qualified.
 *      If no user name is supplied, the user name from the current ID
 *      will be used.
 */
#if defined(OS400)
#pragma convert(850)
#endif

/* C include files */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <names.h>
#include <idtable.h>
#include <osmem.h>
#include <kfm.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <osmisc.h>
#include "../../printLog.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/*
 *      Table of note IDs to add or delete from unread table
 */

   /* Program limit */

#define ACTION_COUNT_MAX   (16)

/*   Test database to demonstrate undoing unread marks */
#define TEST_DB_TITLE  "Simple Data Test"

   /* Entries in the action table */

typedef struct {
   BOOL     AddFlag;      /* TRUE: Add, FALSE: Remove */
   NOTEID   NoteID;
} ACTION_ENTRY;

   /* Table of actions to be performed */
ACTION_ENTRY ActionTable [ACTION_COUNT_MAX] = {FALSE, 0L};
int ActionCount = 0;

/*
 *      Forward references
 */

   /* Function to display unread note table */
STATUS DisplayUnread (
   DBHANDLE     hDb,
   char         NOTESPTR pName,
   WORD         nameLen
);

   /* Function to apply actions to unread note table */
STATUS UpdateUnread (
                     DBHANDLE       hDb,
                     char           NOTESPTR pName,
                     WORD           nameLen,
                     ACTION_ENTRY   *pActions,
                     int            actionCount,
                     NOTEID         *pUndoID
);

   /* Function that updates a note without changing its unread status */
STATUS UndoUnreadStatus (
                         DBHANDLE       hDb, 
                         char           NOTESPTR pName, 
                         WORD           nameLen, 
                         ACTION_ENTRY   *pActions,
                         int            actionCount,
                         NOTEID         UndoID);


/*
 *      Main program
 */

int main (int argc, char *argv[])
{

/* Local data declarations */

   STATUS      status=0;
   char        UserName [MAXUSERNAME + 1];
   WORD        UserNameLen = 0;
   char       *pEnd;
   char       *pPath;
   char       *pName;
   DBHANDLE    hDb;
   int         curAction;
   int         curArg;
   NOTEID      UndoID = 0L;
   BOOL        JustDisplay;   /* TRUE: Display, FALSE: Update */


   if (argc < 2)
   {
      PRINTLOG ("Usage:\n\tunread pathname [username] [+/-id] ...\n\n");
      PRINTLOG ("where:\n\tpathname\tPathname to .nsf file\n");
      PRINTLOG ("\tusername\tOptional user name\n");
      PRINTLOG ("\tid\t\tNote ID to add (+) or remove (-)\n\n");
      PRINTLOG ("You may supply up to %d Note IDs to be added or removed\n",
               ACTION_COUNT_MAX);
      return (NOERROR);
   }


   if (status = NotesInitExtended (argc, argv))
   {
      PRINTLOG("\n Unable to initialize Notes.\n");
      return (1);
   }


   /* Get the database pathname */
   pPath = argv [1];
   curArg = 2;
   JustDisplay = TRUE;

   /* Get the user name */
   if ((argc <= 2) || ('+' == *(argv[curArg])) || ('-' == *(argv[curArg])))
   {
      /* None supplied - get the current user name */
      status = SECKFMGetUserName (UserName);
      if (NOERROR != status)
         goto exit0;

      pName = UserName;
   }
   else
   {
      /* Use the name provided */
      pName = argv [curArg++];
   }

   UserNameLen = strlen (pName);

   /* Process any actions */
   curAction = 0;
   ActionCount = 0;
   while ((curArg < argc) && (ActionCount < ACTION_COUNT_MAX))
   {
      if ('+' == *(argv[curArg]))
         ActionTable[curAction].AddFlag = TRUE;
      else if ('-' == *(argv[curArg]))
         ActionTable[curAction].AddFlag = FALSE;
      else
      {
         PRINTLOG ("Invalid action: %s\n", argv[curArg]);
         PRINTLOG ("Expecting \"+XXXX\" or \"-XXXX\"\n");
         goto exit0;
      }

      ActionTable[curAction].NoteID =
         strtoul (argv[curArg] + 1, &pEnd, 16);

      curAction++;
      ActionCount++;
      curArg++;
      JustDisplay = FALSE;
   }

   /* Open the database */
   status = NSFDbOpen (pPath, &hDb);
   if (NOERROR   != status)
      goto exit0;

   PRINTLOG ("Database name: %s\n", pPath);
   PRINTLOG ("User name: %s\n", pName);

   if (JustDisplay)
      status = DisplayUnread (hDb, pName, UserNameLen);
   else
   {
      status = UpdateUnread (hDb, pName, UserNameLen,
         ActionTable, ActionCount, &UndoID);
      if ( (status == NOERROR) && UndoID )
         /* Demonstrate how to update a note without changing the unread 
            status */
         status = UndoUnreadStatus (hDb, pName, UserNameLen, ActionTable,
                                    ActionCount, UndoID);
   }

      /* Close the database */
   NSFDbClose (hDb);

exit0:
   if (status)
   PRINTERROR (status,"NSFDbOpen");
   NotesTerm();
   return(status);

}

/*
 *      DisplayUnread - Print the list of unread notes
 */

STATUS DisplayUnread (
                      DBHANDLE      hDb,
                      char         *pName,
                      WORD          nameLen)
{
   STATUS         status=NOERROR;
   STATUS         stat2=NOERROR;
   DHANDLE          hTable;
   DWORD          noteID = 0L;
   BOOL           scanFlag;
   

   /* Get the unread list */
   status = NSFDbGetUnreadNoteTable (
                                      hDb,
                                      pName,
                                      nameLen,
                                      TRUE,         /* Create the list if it's not already there */
                                      &hTable);

   if (NOERROR != status)
      return (status);

   /* Update both the unread list in database as well as the cache to
      reflect any database changes made after last time the unread
      list was saved back to database. */

   status = NSFDbUpdateUnread (hDb, hTable);
   if (NOERROR != status)
   {
      IDDestroyTable (hTable);
      return (status);
   }

   scanFlag = TRUE;
   PRINTLOG ("Unread notes:\n");

       /* Print the entries in the unread list */
   while (IDScan (hTable, scanFlag, &noteID))
   {
      scanFlag = FALSE;
      PRINTLOG ("\tNote ID: %lX %s\n", noteID,
         (RRV_DELETED & noteID) ? "(Deleted)" : "");
   }
   
   stat2 = IDDestroyTable (hTable);
   if (NOERROR != status)
      return (status);
   else
      return (stat2);
}

/*
 *      UpdateUnread - Update the Unread Note list
 */

STATUS UpdateUnread (
                      DBHANDLE      hDb,
                      char          *pName,
                      WORD          nameLen,
                      ACTION_ENTRY  *pActions,
                      int           actionCount,
                      NOTEID        *pUndoID)
{
   STATUS         status=NOERROR;
   STATUS         stat2=NOERROR;
   DHANDLE          hTable;
   DHANDLE          hOriginalTable;
   NOTEHANDLE     hNote;
   int            index;
   BOOL           gotUndoID = FALSE;

   /* Get the unread list */
   status = NSFDbGetUnreadNoteTable (
                                     hDb,
                                     pName,
                                     nameLen,
                                     TRUE,         /* Create the list if it's not already there */
                                     &hTable);
   if (NOERROR != status)
      return (status);

   /* Bring table up to date */
   status = NSFDbUpdateUnread (hDb, hTable);
   if (NOERROR != status)
   {
      IDDestroyTable (hTable);
      return (status);
   }

   /* Domino and Notes requires the original unread table to merge changes. */
   /* Be sure this is done AFTER the NSFDbUpdateUnread.          */

   status = IDTableCopy (hTable, &hOriginalTable);
   if (NOERROR != status)
   {
      IDDestroyTable (hTable);
      return (status);
   }

   for (index = 0; (index < actionCount) && (NOERROR == status); index++)
   {
         if (pActions[index].AddFlag)
         {
            /* Adding a Note ID */
            /* (Marks note as Unread) */
            if (IDIsPresent (hTable, pActions[index].NoteID))
            {
               PRINTLOG ("* * Note %lX is already in the unread list\n",
                  pActions[index].NoteID);
            }
            else
            {
               /* make sure we check to see if this note really exists 
                  at all */
               status = NSFNoteOpen(hDb, pActions[index].NoteID, OPEN_SUMMARY, &hNote);
               
               /* if it does we'll add it to the unread list */
               if (status == NOERROR)
               {
                   NSFNoteClose(hNote);
                   status = IDInsert (hTable, pActions[index].NoteID,
                                      (NOTESBOOL NOTESPTR) NULL);
                   if (NOERROR == status)
                      PRINTLOG ("Note %lX marked Unread\n", 
                              pActions[index].NoteID); 
               }
            }   
         }
         else
         {
            /* Removing a Note ID */
            /* (Marks note as Read) */
            if (IDIsPresent (hTable, pActions[index].NoteID))
            {
               status = IDDelete (hTable, pActions[index].NoteID,
                  (NOTESBOOL NOTESPTR) NULL);
               if (NOERROR == status)
               {
                  PRINTLOG ("Note %lX marked Read\n", 
                          pActions[index].NoteID);
                  if (!gotUndoID)
                  {
                     *pUndoID = pActions[index].NoteID;
                     gotUndoID = TRUE;
                  }
               }
            }
            else
            {
               PRINTLOG ("* * Note %lX is not in the unread list\n",
                       pActions[index].NoteID);
            }
         }
   }

   if (NOERROR == status)
      status = NSFDbSetUnreadNoteTable (hDb, pName, nameLen,
                                        FALSE,      /* Don't force the 
                                                          write to disk */
                                        hOriginalTable,
                                        hTable);

   stat2 = IDDestroyTable (hOriginalTable);
   if (NOERROR == status)
      status = stat2;

   stat2 = IDDestroyTable (hTable);
   if (NOERROR == status)
      status = stat2;
   
   return (status);
}


/*
 *      UndoUnreadStatus - Update a note in the database without
                           changing its read/unread mark
 */

STATUS UndoUnreadStatus (
                         DBHANDLE       hDb,
                         char           *pName,
                         WORD           nameLen,
                         ACTION_ENTRY   *pActions,
                         int            actionCount,
                         NOTEID         UndoID)
{
   STATUS         status=NOERROR;
   STATUS         stat2=NOERROR;
   DHANDLE          hTable;
   DHANDLE          hOriginalTable;
   DWORD          noteID = 0L;
   NOTEHANDLE     hNote;
   char           dbinfo [NSF_INFO_SIZE];
   char           title [NSF_INFO_SIZE];
   BOOL           bWasRead = TRUE;


   /* Only do this for our sample DB */
  
   status = NSFDbInfoGet (hDb, dbinfo);
   if (status)
      return (status);
   NSFDbInfoParse(dbinfo, INFOPARSE_TITLE, title, sizeof(title));
   if (strcmp(title, TEST_DB_TITLE))
      /* do nothing */
      return (NOERROR);

   /* Get the unread list */
   status = NSFDbGetUnreadNoteTable (hDb, pName, nameLen,
                                     TRUE, /* Create the list if it's 
                                              not already there */
                                      &hTable);
   if (NOERROR != status)
      return (status);

   status = NSFDbUpdateUnread (hDb, hTable);
   if (NOERROR != status)
   {
      IDDestroyTable (hTable);
      return (status);
   }

   /* Domino and Notes requires the original unread table to merge changes */
   status = IDTableCopy (hTable, &hOriginalTable);
   if (NOERROR != status)
   {
      IDDestroyTable (hTable);
      return (status);
   }
   /* See if note to be modified is marked as read */
   bWasRead = !(IDIsPresent (hTable, UndoID));

   /* Make a change to this note and update it. */
   status = NSFNoteOpen (hDb, UndoID, 0, &hNote);
   if (status)
   {  
      IDDestroyTable (hTable);
      IDDestroyTable (hOriginalTable);
      return (status);
   }

   status = NSFItemSetText (hNote, "plain_text", "Unread undone", 
                            (WORD) strlen("Unread undone"));
   if (status)
   {
      NSFNoteClose (hNote);
      IDDestroyTable (hTable);
      IDDestroyTable (hOriginalTable);
      return (status);
   }  

   status = NSFNoteUpdate (hNote, 0L);
   if (status)
   {
      NSFNoteClose (hNote);
      IDDestroyTable (hTable);
      IDDestroyTable (hOriginalTable);
      return (status);
   }  

   status = NSFNoteClose (hNote);
   if (status)
   {
      IDDestroyTable (hTable);
      IDDestroyTable (hOriginalTable);
      return (status);
   }

   /* The note just modified is now marked as unread.
      Bring table up to date */
   status = NSFDbUpdateUnread (hDb, hTable);
   if ( (NOERROR == status) && bWasRead )
   {
      /* After NSFDbUpdateUnread, our in-memory unread list, hTable, is in
        sync with the database unread list.  Thus, re-establish our "point
        of divergence", the original unread list, to be the same as hTable.
        This is important because otherwise, we can get into the situation 
        where we add a note to hTable and later delete it, making hTable look
        unchanged from the original unread list, resulting in a subsequent
        call to NSFDbSetUnreadNoteTable not making any changes. */

      if (status = IDDestroyTable(hOriginalTable))
         PRINTLOG("IDDestroyTable failed!\n");
      if (status = IDTableCopy(hTable, &hOriginalTable))
         PRINTLOG("IDTableCopy failed!\n");

      /* Remove the Note ID that we just modified to mark it as read*/
      status = IDDelete (hTable, UndoID,
                  (NOTESBOOL NOTESPTR) NULL);
      if (NOERROR == status)
      {
           PRINTLOG ("Note %lX was updated and then marked as Read.\n",
                      UndoID);
      }
      else
      {
         PRINTLOG ("* * Note %lX is not in the unread list\n", UndoID);
      }
   }

   /* Save unread table to disk */
   if (NOERROR == status)
      status = NSFDbSetUnreadNoteTable (hDb, pName, nameLen,
                                        FALSE,  /* Don't force the write 
                                                   to disk */
                                        hOriginalTable, hTable);
   /* Clean up */
   stat2 = IDDestroyTable (hOriginalTable);
   if (NOERROR == status)
      status = stat2;

   stat2 = IDDestroyTable (hTable);
   if (NOERROR == status)
      status = stat2;
   return (status);
}
