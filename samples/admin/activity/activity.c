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

    PROGRAM:    activity

    FILE:       activity.c

    SYNTAX:     activity

    PURPOSE:    Illustrates the basic structure of a HCL C API
                NotesMain application for Domino and Notes.

    DESCRIPTION:
         A HCL C API for Notes/Domino program that reads the binary records from 
         the server's log file and displays it to standard output. Activity logging replaces 
         and surpasses the previous Billing functionality. The activity logging services are 
         a set of routines that read and write binary records to the server log (log.nsf). 
         For efficiency, many records are written to one note. 

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
/* added */
#if defined(OS400) || defined(MAC)
#include <stdlib.h>
#else
#include <malloc.h>
#endif

/* HCL C API for Notes/Domino include files containing the C API
   Version #
 */

#include <global.h>
#include <nsfdb.h>
#include <nsfdata.h>
#include <osfile.h>
/* added */
#include <log.h>
#include <misc.h>
#include <textlist.h>
#if defined(CAPI_TESTING) 
#include "printlog.h" 
#else
 #define PRINTLOG printf 
 #define PRINTERROR(api_error, api_name) {\
 char    szErrorText[256] = { 0 };\
 OSLoadString(NULLHANDLE, ERR(api_error), szErrorText, sizeof(szErrorText));\
 fprintf(stderr, "[ERROR]:%s:%d:%s - %s", __FILE__,__LINE__,api_name,szErrorText); }
#endif 
#include <stdlib.h>

#if defined(OS390) && (__STRING_CODE_SET__==ISO8859-1 /* ascii compile */)
#include <_Ascii_a.h>   /* NOTE: must be the LAST file included */
#endif /* OS390, ascii compile */

#ifndef OSMISC_DEFS
#include <osmisc.h>
#endif

#define ENVACTIVITY_AUTO "ACTIVITY_AUTO"  /* Env string for handling automation */
#define ENV_LENGTH 25		/* Length for env ACTIVITY_AUTO string */

/* Global Var */
WORD autoRecCount = 0;	/* Flag for avoid processing records in ActionRoutine for automation */

/* This is the callback function which is called for every record */
STATUS LNCALLBACK ActionRoutine(
	const char *DescName, 
	DWORD DescIdx,
	DWORD Flags,
	WORD  PrimaryKey,
   	const TIMEDATE* TimeStamp,
	void *pActivityRecord,
	void *pUserData);

/************************************************************************

    FUNCTION:   NotesMain

    PURPOSE:    Main HCL C API for Notes/Domino subroutine

*************************************************************************/

STATUS LNPUBLIC NotesMain(int argc, char far *argv[])
{
   STATUS      error = NOERROR;          /* error code from API calls */
   char        *pserver = NULL;
   void        *pstreamctx;
   int         recordcount = 0;

   if (argc != 2)
   {
	   PRINTLOG("\nUsage:  %s <server name>\n", argv[0]);
	   return (0);
   }
   else
   {
	   pserver = argv[1];
   }

   /* Open the stream. */
  if (error = LogOpenActivityStream(
  		&pstreamctx, /* Return the stream context */
  		pserver, 	 /* Server name or NULL */
		"log.nsf",			 /*	LogPath */
  		NULL, 		/* NULL means ALL activity types. */
  		0, 			/* No flags */
  		NULL))		/* No date restriction */
    PRINTERROR(error,"LogOpenActivityStream");

  /* Read the records */
  if (error = LogEnumActivityStream(
  		pstreamctx,    /* Open activity stream context */
  		ActionRoutine, /* User defined callback */
  		&recordcount,  /* Some example user data */
  		NULL, 		   /* Not saving the stream position. NULL OK here */
  		0))				/* Not saving the stream position. 0 OK here */
	PRINTERROR(error,"LogEnumActivityStream");

  /* Close the stream */
  LogCloseActivityStream(pstreamctx);

  
  /* End of subroutine. */
  PRINTLOG("\nProgram completed successfully\n");
}

STATUS LNCALLBACK ActionRoutine(
	const char *DescName, 
	DWORD DescIdx,
	DWORD Flags,
	WORD  PrimaryKey,
   	const TIMEDATE* TimeStamp,
	void *pActivityRecord,
	void *pUserData)
{
	char *pactivitybuf = (char *)pActivityRecord;
	ITEM_TABLE itemtable;
	ITEM* 	items=NULL;
	char* name=NULL;
	char timestr[MAXALPHATIMEDATE + 1] = {0};
	WORD  timelen;
	char numstr[MAXALPHANUMBER + 1] = {0};
	WORD  numlen;
	LIST *plist=NULL;
	WORD listentries;
	USHORT type;
	int* pcounter = (int *)pUserData;
	USHORT i;
	WORD j;
	char envAuto[ENV_LENGTH] = { 0 };

	/* Increment the record counter */
	(*pcounter)++;

	/* This check is for printing only first record from the log.nsf
	 * for automation check, on manual execution all recs are printed */
	if (autoRecCount == 1)
            return NOERROR;

	/* NOTE: See the SDK reference for a description of ITEM_TABLE */
	/* Make a local ITEM_TABLE copy */
	memmove(&itemtable, pActivityRecord, sizeof(ITEM_TABLE));
	/* Move past to the ITEM array */		
	pactivitybuf += sizeof(ITEM_TABLE);
	/* Allocate our own copy of the item array */
	items = (ITEM* )malloc(itemtable.Items * sizeof(ITEM));
	
	memmove(items, (char *)pactivitybuf, itemtable.Items * sizeof(ITEM));
	/* Move to the start of the actual data */	
	pactivitybuf += itemtable.Items * sizeof(ITEM);

	/* Read OS env ACTIVITY_AUTO variavle */
	strncpy(envAuto, getenv(ENVACTIVITY_AUTO), ENV_LENGTH-1);
	if (!autoRecCount && envAuto[0] != '\0')
        {
           if (strcmp(envAuto, "1") == 0)  /* If env value is 1 then its for automation & we process only 1 rec */
           {
               autoRecCount = 1;
           }
        }

	ConvertTIMEDATEToText(NULL, NULL, TimeStamp, timestr, MAXALPHATIMEDATE, &timelen);
	/* Output the record header */
	PRINTLOG("Record #: %d, Name: %s, DescIdx: %d, Timestamp: %.*s\n{\n", 
		*pcounter, DescName, DescIdx, timelen, timestr);
	/* Step through all of the items */
	for(i = 0; i < itemtable.Items; i++)
	{
		/* IMPORTANT. Its possible to have an existing item with a value length of 0 */
		if(!items[i].ValueLength)
			continue;
		/* Point to the item name */		
		name = pactivitybuf;
		
		pactivitybuf += items[i].NameLength;
		/* Get the type of the item */	
		memmove(&type, pactivitybuf, sizeof(USHORT));		
		/* Point to the value */		
		pactivitybuf += sizeof(USHORT);		
		
		switch(type)
			{
			case TYPE_TEXT:
				{
				PRINTLOG(
					"\t%.*s: %.*s\n", 
					items[i].NameLength, 
					name, 
					items[i].ValueLength - sizeof(USHORT),
					pactivitybuf);				
				break;
				}							
			case TYPE_TIME:
				{
				ConvertTIMEDATEToText(NULL, NULL, (TIMEDATE* )pactivitybuf, timestr, MAXALPHATIMEDATE, &timelen);
				PRINTLOG(
					"\t%.*s: %.*s\n", 
					items[i].NameLength, 
					name,
					timelen,
					timestr);												
				break;
				}
			case TYPE_NUMBER:
				{				
				ConvertFLOATToText(NULL, NULL, (NUMBER *)pactivitybuf, numstr, MAXALPHANUMBER, &numlen);
				PRINTLOG(
					"\t%.*s: %.*s\n", 
					items[i].NameLength, 
					name, 
					numlen,
					numstr);												
				break;
				}
			case TYPE_TEXT_LIST:
				{
				plist = (LIST* ) pactivitybuf;
				listentries = ListGetNumEntries(plist, FALSE);
				PRINTLOG(
					"\t%.*s:\n\t\t{\n", 
					items[i].NameLength, 
					name);
				
				for(j = 0; j < listentries; j++)
					{
					char *ptext;
					WORD len;
					ListGetText(plist, FALSE, j, &ptext, &len); 
	   				PRINTLOG("\t\t%.*s\n", len, ptext);
					}
				PRINTLOG("\t\t}\n");
				break;
				}

			default:
				PRINTLOG("type not implemented: %*.s\n", items[i].NameLength, name);			
			}
			/* Point to the start of the next item. */					
			pactivitybuf += (items[i].ValueLength - sizeof(USHORT));
	}				

	PRINTLOG("}\n");

	free(items);
	
	return NOERROR;
}

#ifdef __cplusplus
}
#endif
