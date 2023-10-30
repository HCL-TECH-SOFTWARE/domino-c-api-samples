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

PROGRAM:	sharedirectory

FILE:		sharedirectory.c

PURPOSE:	Prints the shared data directory path of the Notes client using OSGetSharedDataDirectory.

SYNTAX:		sharedirectory

*************************************************************************/
/* C include files. */

#include <stdio.h>
#include <string.h>

/* HCL C API for Notes/Domino include files. */

#include "global.h"
#include "osfile.h"
#include "osmisc.h"
#include "miscerr.h"
#include "oserr.h"
#include "../../printLog.h"

/* Program declaration. */
int main(int argc, char *argv[])
{
	
	/* Local data declarations. */
	
	char szShareDirectory[MAXPATH];
	STATUS error = NOERROR;
	memset(szShareDirectory, MAXPATH, '\0');

	error = NotesInitExtended(argc, argv);

	if (error != NOERROR)
	{
		PRINTERROR(error,"NotesInitExtended");
		return (1);
	}

	/* This api is applicable only to the client. 
	Gets share data directory of the client. */

	if (OSGetSharedDataDirectory (szShareDirectory) == 0)
	{
		PRINTLOG("OSGetSharedDataDirectory failed.\n");
	}
	else
	{
		PRINTLOG("The shared data directory of the multi-user Notes client is \"%s\".\n", szShareDirectory);
		PRINTLOG("Program completed successfully.\n");
	}
	NotesTerm();
	return (0);
}

