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

	PROGRAM:    itemmisc

	FILE:       itemmisc.c

	PURPOSE:    This program demonstrates how to use below APIs,
				NSFItemCopyAndRename,
		NSFItemModifyValue,
		NSFNoteHasReadersField


	SYNTAX:     itemmisc (db) ("view")


	DESCRIPTION:
		Given an existing database and a view name, this program modifies the
		existing item (plain_text) value and copies the same from one document
	to another & renames it. It checks whether documents in the given view
	has readers field in set or not.
	This program uses simple.nsf, which has two docs in a view and all manipulation
	are done based on that two documents.

 **********************************************************************/

 /* C includes */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* HCL C API for Notes/Domino includes */

#include <global.h>
#include <osmisc.h>
#include <osmem.h>
#include <miscerr.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <nif.h>
#include <stdnames.h>
#include <textlist.h>
#include <osmisc.h>
#include <osenv.h>                      /* OSGetEnvironmentString */
#include <mail.h>
#include <osmisc.h>
#include <osmem.h>
#include "../../printLog.h"


#if !defined(ND64) 
#define DHANDLE HANDLE 
#endif

#define LINEOFTEXT   256

#define DESIGN_READERS "$Readers"
#define MOD_ITEM_NAME  "plainText"


STATUS AddItemReaders(NOTEHANDLE hNote);

/************************************************************************

	FUNCTION:   main

	PURPOSE:    Main HCL C API for Notes/Domino subroutine

	ALGORITHM:
   1) Read the command line arguments. If invalid args count then return 1
	  Parse them into the appropriate variables.

   2) Get the note ID of the specified view.
	  Open the collection.
	  Create a buffer in memory of the document noteIDs in the collection.
	  Close the collection.

   3) If hBuffer not null, then lock the buffer memory.
		 For every noteID in the buffer,
			open the note,
			  if the $Reader text list field exists
			 print the note id
		  Modify the existing item value
		 unlock the buffer memory
		 free the memory.

   4) Close the Domino database
	  if error, print the error and return the error number

*************************************************************************/

int main(int argc, char *argv[])
{
	STATUS error = NOERROR;
	char pathName[LINEOFTEXT];
	char viewName[LINEOFTEXT];
	DBHANDLE hDB;
	HCOLLECTION hCollection;
	NOTEID viewNoteID;
	BLOCKID hBlock;
	BLOCKID bhItem;
	BLOCKID bhItemCopy;
	int nFieldLen;

	/********************************************************************/
	/* Read the command line arguments,
	   store them into the appropriate variables,
	   open the specified database.
	*/
	/********************************************************************/
	if (argc >= 2)
	{
		memset(pathName, '\0', sizeof(pathName));
		memset(viewName, '\0', sizeof(viewName));
		strncpy(pathName, argv[1], sizeof(pathName) - 1);
		strncpy(viewName, argv[2], sizeof(viewName) - 1);
	}
	else
	{
		printf("\n Invalid args: itemmisc <db> <view> \n");
		return(1);
	}

	if (error = NotesInitExtended(argc, argv))
	{
		printf("\n Unable to initialize Notes.\n");
		return(1);
	}

	/* Open the database specified on the command line. */
	if (error = NSFDbOpen(pathName, &hDB))
	{
		goto DBErr;
	}

	/********************************************************************/
	/* Get the note ID of the specified view,
	   open the collection,
	   create a buffer in memory of the document noteIDs in the collection,
	   close the collection.
	*/
	/********************************************************************/
	if (error = NIFFindView(hDB, viewName, &viewNoteID))
	{
		if (error == ERR_NOT_FOUND)
		{
			printf("View '%s' cannot be found\n", viewName);
			error = NOERROR;
		}
		goto NIFErr;
	}

	/* Open the collection of notes in the view at the current time.
	   Return a handle to the collection to the variable hCollection. */
	if (error = NIFOpenCollection(hDB, hDB, viewNoteID,
		0,
		NULLHANDLE,
		&hCollection,
		NULL, NULL, NULL, NULL))
	{
		goto NIFErr;
	}

	{
		COLLECTIONPOSITION indexPos;
		DWORD entriesReturned;
		DHANDLE hBuffer;
		WORD   signalFlag;             /* signal and share warning flags */

		indexPos.Level = 0;
		indexPos.Tumbler[0] = 0;

		do
		{
			if (error = NIFReadEntries(hCollection,
				&indexPos,
				NAVIGATE_NEXT, 1L,
				NAVIGATE_NEXT, MAXDWORD,
				READ_MASK_NOTEID,
				&hBuffer, NULL,
				NULL, &entriesReturned, &signalFlag))
			{
				goto ColErr;
			}

			/********************************************************************/

			if (hBuffer != NULLHANDLE)
			{
				char string[512];
				const char szItemname[LINEOFTEXT] = "plain_text";
				char szActValue[LINEOFTEXT] = "";
				WORD wItemtype = TYPE_TEXT;
				const char szExpValue[LINEOFTEXT] = "This is a line of modified simple text.";
				NOTEHANDLE hNoteFirstDoc, hNoteSecDoc;
				NOTEID *noteIDCpy;

				NOTEID *entry = OSLock(NOTEID, hBuffer);

				/* process each individual noteID       */

				/* skip this noteID if it is for a category entry  */
				if (*entry & NOTEID_CATEGORY)
				{
					entry++;
				}

				/* open each note separately, follow each with
				   close of note */
				if (error = NSFNoteOpen(hDB, *entry, 0, &hNoteFirstDoc))
				{
					OSLoadString(NULLHANDLE, ERR(error), string, sizeof(string) - 1);

					printf("Error '%s' reading docment %#lX -- %s\n",
						string, *entry, " skipping it");

					/* Since the error has been reported, we will
					   reset the error status and continue */
					error = NOERROR;
					continue;
				}

				noteIDCpy = entry; /* Preserve it for later use */

				/* Check given doc has readers field */
				if (NSFNoteHasReadersField(hNoteFirstDoc, &hBlock))
				{
					printf("\n Note id [%#lx] has reader field.", *entry);
				}
				else /* No readers field present */
				{
					if (error = AddItemReaders(hNoteFirstDoc)) /* Add the readers field to this doc */
					{
						goto DocErr;
					}

					if (NSFNoteHasReadersField(hNoteFirstDoc, &hBlock))
					{
						printf("\n Note id [%#lx] has reader field.", *entry);
					}
				}

				/* get item info */
				if (error = NSFItemInfo(hNoteFirstDoc,
					szItemname,
					strlen(szItemname),
					&bhItem,
					NULL,
					NULL,
					NULL))
				{
					goto DocErr;
				}

				nFieldLen = NSFItemGetText(hNoteFirstDoc, szItemname, szActValue, LINEOFTEXT);

				printf("\n Before Modifying Item's value: %s", szActValue);
				bhItemCopy = bhItem;

				/* This API is used to modify the item value */
				if (error = NSFItemModifyValue(hNoteFirstDoc,
					bhItem,
					ITEM_SUMMARY,
					wItemtype,
					szExpValue,
					strlen(szExpValue)))
				{
					goto DocErr;
				}
				nFieldLen = NSFItemGetText(hNoteFirstDoc, szItemname, szActValue, LINEOFTEXT);

				printf("\n Modified Item's value: %s \n", szActValue);

				/* Update the Notes handler to get the changes reflected in nsf */
				if (error = NSFNoteUpdate(hNoteFirstDoc, UPDATE_FORCE))
				{
					goto DocErr;
				}

				/* Processing second doc */

				/* skip this noteID if it is for a category entry  */
				if (entry++ && *entry & NOTEID_CATEGORY)
				{
					entry++;
				}

				/* open each note separately, follow each with
				   close of note */
				if (error = NSFNoteOpen(hDB, *entry, 0, &hNoteSecDoc))
				{
					OSLoadString(NULLHANDLE, ERR(error), string, sizeof(string) - 1);

					printf("Error '%s' reading docment %#lX -- %s\n",
						string, *entry, " skipping it");

					/* Since the error has been reported, we will
					   reset the error status and continue */
					error = NOERROR;
					goto DocErr;
				}

				/* Check for second doc has readers field */
				if (NSFNoteHasReadersField(hNoteSecDoc, &hBlock))
				{
					printf("\n Note id [%#lx] has reader field.", *entry);
				}
				else
				{
					printf("\n Note id [%#lx] has no reader field.", *entry);
				}

				/* Copy the item from the first document to the second document
				 * here bhItemCopy is an item from the document 1. Now we are copying
				 * the same to document 2*/
				if (error = NSFItemCopyAndRename(hNoteSecDoc, bhItemCopy, MOD_ITEM_NAME))
				{
					goto DocSecErr;
				}

				/* Update document with the added item */
				if (error = NSFNoteUpdate(hNoteSecDoc, UPDATE_FORCE))
				{
					goto DocSecErr;
				}

				nFieldLen = NSFItemGetText(hNoteSecDoc, MOD_ITEM_NAME, szActValue, LINEOFTEXT);
				printf("\n Copied doc [%#lx] modified Item value to doc [%#lx],\n", *noteIDCpy, *entry);
				printf(" Value of item is: [%s] Item Name: [%s].\n", szActValue, MOD_ITEM_NAME);

			DocSecErr:
				/* Close the Note handle */
				NSFNoteClose(hNoteSecDoc);
			DocErr:
				/* close each note   */
				NSFNoteClose(hNoteFirstDoc);
				if (error)
				{
					OSLoadString(NULLHANDLE, ERR(error), string,
						sizeof(string) - 1);
					printf("Error '%s' writing document %#lX -- %s\n",
						string, *entry, "skipping it");
					/* Since the error has been reported, we will
					   reset the error status and continue */
					error = NOERROR;
					continue;
				}

				/* finished with noteIDs, unlock memory and free it   */
				OSUnlockObject(hBuffer);
				OSMemFree(hBuffer);
			}   /* if (hBuffer != NULLHANDLE) */

		} while (signalFlag & SIGNAL_MORE_TO_DO);

		NIFCloseCollection(hCollection);

		goto ColSucc;
	}

ColErr:
	NIFCloseCollection(hCollection);

ColSucc:
NIFErr:
	/********************************************************************/
	/*
	Close the Domino database.
	*/
	/********************************************************************/
	NSFDbClose(hDB);

DBErr:
	if (error)
	{
		PRINTERROR(error,"NIFReadEntries");
	}

	NotesTerm();
	return(error);
}

/* This routine is used to set the readers field to the document
 * It takes notes document handle as input and adds the $Readers field to it
 */
STATUS AddItemReaders(NOTEHANDLE hNote)
{
	STATUS error = NOERROR;
	DHANDLE hList;          /* handle to privilege text list */
	VOID *pList;           /* pointer to the privilege text list */
	WORD wListSize;         /* total size of text list structure */
	char *pTextEntry = "Test User";


	/* Create an empty text list structure */

	if (error = ListAllocate(0, 0, FALSE, &hList, &pList, &wListSize))
	{
		return (error);
	}

	OSUnlock(hList);
	pList = NULL;
	if (error = ListAddEntry(hList, FALSE, &wListSize, 0,
		pTextEntry, (WORD)strlen(pTextEntry)))
	{
		OSMemFree(hList);
		return (error);
	}

	/* Add the completed field to the note. */

	pList = OSLockObject(hList);

	error = NSFItemAppend(hNote, ITEM_SUMMARY | ITEM_READERS,
		DESIGN_READERS, (WORD)strlen(DESIGN_READERS),
		TYPE_TEXT_LIST, pList, wListSize);

	/* Unlock and free the buffer that was holding the text list field. */

	OSUnlock(hList);
	OSMemFree(hList);
	if (error)
	{
		return (error);
	}

	/* Update the note */

	error = NSFNoteUpdate(hNote, UPDATE_NOCOMMIT);

	return error;
}


