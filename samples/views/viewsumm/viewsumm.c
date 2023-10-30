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

    PROGRAM:    viewsumm

    FILE:       viewsumm.c

    SYNTAX:     viewsumm  <db filename>  <view name>
                  or
                viewsumm

    PURPOSE:    Print the summary information about each entry in a view.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* C include files */

#include <stdio.h>
#include <string.h>

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <nsfdb.h>
#include <nif.h>
#include <textlist.h>
#include <osmem.h>
#include <misc.h>
#include <miscerr.h>
#include <editods.h>
#include "../../printLog.h"

#include <osmisc.h>

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif


#define  STRING_LENGTH  256

/* Function prototypes for local functions */

void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                            char *DBFileName, char *ViewName); 
STATUS PrintSummary (BYTE *);
STATUS ExtractTextList (BYTE *, char *);

/************************************************************************

    FUNCTION:   main API subroutine

*************************************************************************/

int main(int argc, char *argv[])
{


/* Local data declarations */

   char               *DBFileName;            /* pathname of the database */
   char               *ViewName;              /* name of the view we'll read */
   char               DBBuf[STRING_LENGTH];
   char               ViewBuf[STRING_LENGTH];
   DBHANDLE           hDB;                    /* handle of the database */
   NOTEID             ViewID;                 /* note id of the view */
   HCOLLECTION        hCollection;            /* collection handle */
   COLLECTIONPOSITION CollPosition;           /* index into collection */
   DHANDLE            hBuffer;                /* handle to buffer of info */
   BYTE               *pBuffer;               /* pointer into info buffer */
   BYTE               *pSummary;              /* pointer into info buffer */
   NOTEID             EntryID;                /* a collection entry id */
   DWORD              EntriesFound;           /* number of entries found */
   ITEM_VALUE_TABLE   ItemTable;              /* table in pSummary buffer */
   WORD               SignalFlag;             /* signal and share warning flags */
   DWORD              i;                      /* a counter */
   STATUS             error = NOERROR;        /* return status from API calls */


   memset(DBBuf, '\0', STRING_LENGTH);
   memset(ViewBuf, '\0', STRING_LENGTH);
   DBFileName = DBBuf;
   ViewName = ViewBuf;
   ProcessArgs(argc, argv, DBFileName, ViewName); 

/* Initialize Domino and Notes */

   if (error = NotesInitExtended (argc, argv))
   {
       PRINTLOG("\n Unable to initialize Notes. Error Code[0x%04x]\n", error);
       return (1);
   }

/* Open the database. */

   if (error = NSFDbOpen (DBFileName, &hDB))
   {
       PRINTERROR (error,"NSFDbOpen");  
       NotesTerm();
       return (1);
   } 


/* Get the note id of the view we want. */

   if (error = NIFFindView (
         hDB, 
         ViewName, 
         &ViewID))
   {
      NSFDbClose (hDB);
      PRINTERROR (error,"NIFFindView");  
      NotesTerm();
      return (1);
   }

/* Get the current collection using this view. */

   if (error = NIFOpenCollection(
                      hDB,            /* handle of db with view */
                      hDB,            /* handle of db with data */
                      ViewID,         /* note id of the view */
                      0,              /* collection open flags */
                      NULLHANDLE,     /* handle to unread ID list (input and return) */
                      &hCollection,   /* collection handle (return) */
                      NULLHANDLE,     /* handle to open view note (return) */
                      NULL,           /* universal note id of view (return) */
                      NULLHANDLE,     /* handle to collapsed list (return) */
                      NULLHANDLE))    /* handle to selected list (return) */
   {
      NSFDbClose (hDB);
      PRINTERROR (error,"NIFOpenCollection");  
      NotesTerm();
      return (1);
   }

/* Set a COLLECTIONPOSITION to the beginning of the collection. */

   CollPosition.Level = 0;
   CollPosition.Tumbler[0] = 0;

/* Get the note ID and summary of every entry in the collection. In the 
returned buffer, first comes all of the info about the first entry, then 
all of the info about the 2nd entry, etc. For each entry, the info is
arranged in the order of the bits in the READ_MASKs. */

   do
   {
      if (error = NIFReadEntries(
                         hCollection,        /* handle to this collection */
                         &CollPosition,      /* where to start in collection */
                         NAVIGATE_NEXT,      /* order to use when skipping */
                         1L,                 /* number to skip */
                         NAVIGATE_NEXT,      /* order to use when reading */
                         0xFFFFFFFF,         /* max number to read */
                         READ_MASK_NOTEID +  /* info we want */
                         READ_MASK_SUMMARYVALUES,
                         &hBuffer,           /* handle to info buffer (return)  */
                         NULL,               /* length of info buffer (return) */
                         NULL,               /* entries skipped (return) */
                         &EntriesFound,      /* entries read (return) */
                         &SignalFlag))       /* share warning and more signal flag
                                                (return) */
      {
         NIFCloseCollection (hCollection);
         NSFDbClose (hDB);
         PRINTERROR (error,"NIFReadEntries");  
         NotesTerm();
         return (1);
      }

/* Check to make sure there was a buffer of information returned. */ 

      if (hBuffer == NULLHANDLE)
      {
         NIFCloseCollection (hCollection);
         NSFDbClose (hDB);
         PRINTLOG ("\nEmpty buffer returned by NIFReadEntries.\n");
         NotesTerm();
         return (0); 
      }

/* Lock down (freeze the location) of the information buffer. Cast
   the resulting pointer to the type we need. */

      pBuffer = (BYTE *) OSLockObject (hBuffer);

/* Start a loop that extracts the info about each collection entry from
   the information buffer. */

      PRINTLOG ("\n");
      for (i = 1; i <= EntriesFound; i++)
      {

/* Get the ID of this entry. */

         memcpy (&EntryID, pBuffer, sizeof(EntryID));

/* Advance the pointer over the note id. */

         pBuffer += sizeof (EntryID);

/* Get the header for the summary items for this entry. */

         memcpy (&ItemTable, pBuffer, sizeof(ItemTable));

/* Remember where the start of this entry's summary is. Then advance
   the main pointer over the summary. */
      
         pSummary = pBuffer;
         pBuffer += ItemTable.Length; 
 
/* If this entry is a category, say so. */

         if (NOTEID_CATEGORY & EntryID)
            PRINTLOG ("CATEGORY: ");

/* Call a local function to print the summary buffer. */

         if (error = PrintSummary (pSummary))
         {
            OSUnlockObject (hBuffer);
            OSMemFree (hBuffer);
            NIFCloseCollection (hCollection);
            NSFDbClose (hDB);
            PRINTERROR (error,"PrintSummary");  
            NotesTerm();
            return (1);
         }

/* End of loop that gets info about each entry. */

      }

/* Unlock the list of note IDs. */

      OSUnlockObject (hBuffer);

/* Free the memory allocated by NIFReadEntries. */

      OSMemFree (hBuffer);

/* Loop if the end of the collection has not been reached because the buffer
   was full.  */

   }  while (SignalFlag & SIGNAL_MORE_TO_DO);


/* Close the collection. */

   if (error = NIFCloseCollection(hCollection))
   {
      NSFDbClose (hDB);
      PRINTERROR (error,"NIFCloseCollection");  
      NotesTerm();
      return (1);
   }

/* Close the database. */

   if (error = NSFDbClose (hDB))
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


STATUS PrintSummary (BYTE *pSummary)

/* This function prints the items in the summary for one
entry in a collection.

The information in a view summary is as follows:

   header (total length of pSummary, number of items in pSummary)
   length of item #1 (including data type)
   length of item #2 (including data type)
   length of item #3 (including data type)
   ...
   data type of item #1
   value of item #1
   data type of item #2
   value of item #2
   data type of item #3
   value of item #3
   ....
*/

{

/* Local constants */

#define  MAX_ITEMS          20
#define  MAX_ITEM_LEN       100
#define  DATATYPE_SIZE      sizeof(USHORT)
#define  ITEM_LENGTH_SIZE   sizeof(USHORT)
#define  NUMERIC_SIZE       sizeof(NUMBER)
#define  TIME_SIZE          sizeof(TIMEDATE)


/* Local variables */

   BYTE             *pSummaryPos;           /* current position in pSummary */
   ITEM_VALUE_TABLE ItemTable;              /* header at start of pSummary */
   USHORT           ItemCount;              /* number of items in pSummary */
   USHORT           ItemLength[MAX_ITEMS];  /* length of each item */
   USHORT           DataType;               /* type of pSummary item */
   char             ItemText[MAX_ITEM_LEN]; /* text of a pSummary item */
   NUMBER           NumericItem;            /* a numeric item */
   TIMEDATE         TimeItem;               /* a time/date item */
   WORD             TimeStringLen;          /* length of ASCII time/date */
   STATUS           error;                  /* return code from API calls */
   USHORT           i;                      /* a counter */


/* Start reading the summary at its beginning. */

   pSummaryPos = pSummary;

/* Get the header at the beginning of the pSummary buffer. */

   memcpy (&ItemTable, pSummaryPos, sizeof(ItemTable));

/* Advance the buffer pointer over the header. */

   pSummaryPos += sizeof(ItemTable);

/* Get the number of items in the summary. */

   ItemCount = ItemTable.Items;

   if (ItemCount > MAX_ITEMS)
   {
      PRINTLOG (
         "Summary contains %d items - only printing the first %d items\n",
         ItemCount, MAX_ITEMS);
      ItemCount = MAX_ITEMS;
   }

/* Get the length of each item in the summary. */

   for (i=0; i < ItemCount; i++)
   {
      memcpy (&ItemLength[i], pSummaryPos, ITEM_LENGTH_SIZE);
      pSummaryPos += ITEM_LENGTH_SIZE;
   }

/* Start a loop that extracts each item in the summary. */

   for (i=0; i < ItemCount; i++)
   {

/* If an item has zero length it indicates an "empty" item in the
   summary. This might occur in a lower-level category and stand for a
   higher-level category that has already appeared. Or an empty item might
   be a field that is missing in a response doc. Just print * as a place
   holder and go on to the next item in the pSummary. */

      if (ItemLength[i] == 0)
      {
         PRINTLOG ("  *  ");
         continue;
      }
      else if (ItemLength[i] >= MAX_ITEM_LEN)
      {
            /* Item is too long for the buffer - just truncate it */
         ItemLength[i] = MAX_ITEM_LEN - 1;
      }

/* Get the data type of this item. */

      memcpy (&DataType, pSummaryPos, DATATYPE_SIZE);
      pSummaryPos += DATATYPE_SIZE;

/* Extract the item from the summary and put it in readable
   form. The way in which we extract the item depends on its type.
   This program handles TEXT, TEXT_LIST, NUMBER, and TIME. */

      switch (DataType)
      {

/* Extract a text item from the pSummary. */

         case TYPE_TEXT:
            memcpy (ItemText,
                    pSummaryPos,
                    ItemLength[i] - DATATYPE_SIZE);
            ItemText[ItemLength[i] - DATATYPE_SIZE] = '\0';
            break;

/* Extract a text list item from the pSummary. */

         case TYPE_TEXT_LIST:
            if (error = ExtractTextList (
                   pSummaryPos,
                   ItemText))
               return (ERR(error));
            break;

/* Extract a number item from the pSummary. */

         case TYPE_NUMBER:
            memcpy (&NumericItem, pSummaryPos, NUMERIC_SIZE);
            sprintf (ItemText, "%g", NumericItem);
            break;

/* Extract a time/date item from the pSummary. */

         case TYPE_TIME: 

            memcpy (&TimeItem, pSummaryPos, TIME_SIZE);

            if (error = ConvertTIMEDATEToText (
                                   NULL,
                                   NULL,
                                   &TimeItem,
                                   ItemText,
                                   MAXALPHATIMEDATE,
                                   &TimeStringLen))
               return (ERR(error));

            ItemText[TimeStringLen] = '\0';

            break;

/* If the pSummary item is not one of the data types this program 
handles. */

         default:
            strcpy (ItemText, "");
            break;
      }

/* Print the item. (Add spaces so next item is separated.) */

      PRINTLOG ("%s    ", ItemText);

/* Advance to next item in the pSummary. */

       pSummaryPos += (ItemLength[i] - DATATYPE_SIZE);

/* End of loop that is extracting each item in the pSummary. */

   }

/* Print final line feed to end this pSummary display. */

   PRINTLOG ("\n");

/* End of function */

   return (NOERROR);

}


STATUS ExtractTextList (BYTE *pBuffer, char *TextList)

/* This function extracts an item of TYPE_TEXT_LIST from a
pSummary buffer. */

{


/* Local variables */

   USHORT  ListCount;    /* number of entries in a text list */
   char    *ListEntry;   /* pointer to list entry */
   WORD    ListLen;      /* total length of text list */
   WORD    EntryLen;     /* length of one entry */
   STATUS  error;        /* return code from API calls */
   USHORT  i;            /* a counter */


/* Clear the string that we'll fill up. */

   strcpy (TextList, "");

/* Get the number of items in the list. */

   memcpy (&ListCount, pBuffer, sizeof(ListCount));

/* Initialize the total length of the list. */

   ListLen = 0;

/* Start a loop that gets each entry out of the list. */

   for (i=0; i<ListCount; i++)
   {

/* Get the entry. */

      if (error = ListGetText (
                         pBuffer,
                         FALSE, /* DataType not prepended to list */
                         i,
                         &ListEntry,
                         &EntryLen))
         return (ERR(error));

/* Copy this entry to the string we are building and move the pointer that
keeps track of how much we have in the string. */

      memcpy (TextList+ListLen, ListEntry, EntryLen);
      ListLen += EntryLen;

/* Put a comma after the entry and move the string pointer. */

      memcpy (TextList+ListLen, ",", 1);
      ListLen++;

/* End of loop that is getting each entry. */

   }

/* Put a null in place of the last comma */

   ListLen--;
   memcpy (TextList+ListLen, "\0", 1);

/* End of function */

   return (NOERROR);

}

/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:     argc, argv - directly from the command line
   
    OUTPUTS:    DBFileName, ViewName - data from the command line or 
                from what the user types when prompted.

*************************************************************************/

void  LNPUBLIC  ProcessArgs (int argc, char *argv[], 
                               char *DBFileName, char *ViewName)
{ 
    if (argc != 3)  
    {
        printf("Enter name of database: ");      
        fflush(stdout);
        fgets(DBFileName, STRING_LENGTH-1, stdin);

        printf("\n");
        printf("Enter name of View: ");      
        fflush(stdout);
        fgets(ViewName, STRING_LENGTH-1, stdin);
    } 
    else
    {
         strncpy(DBFileName, argv[1], STRING_LENGTH-1);
         strncpy(ViewName, argv[2], STRING_LENGTH-1);
         
    } /* end if */
} /* ProcessArgs */
