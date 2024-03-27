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

PROGRAM:    intro

FILE:       intro.c

PURPOSE:    Shows the basic structure of a main() routine that calls
HCL C API for Notes/Domino functions. The program
gets the title of a local or remote Domino database and
prints it to the screen.

SYNTAX:     intro  [server name - optional] <database filename>

*************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* OS and C include files */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* HCL C API for Notes/Domino include files */

#include "global.h"
#include "nsfdb.h"
#include "nsfdata.h"
#include "osmisc.h"
#include "osfile.h"

#ifndef OSMISC_DEFS
#include <osmisc.h>
#endif

#if defined(CAPI_TESTING) 
#include "printlog.h" 
#else
 #define PRINTLOG printf 
 #define PRINTERROR(api_error, api_name) {\
 char    szErrorText[256] = { 0 };\
 OSLoadString(NULLHANDLE, ERR(api_error), szErrorText, sizeof(szErrorText));\
 fprintf(stderr, "[ERROR]:%s:%d:%s - %s", __FILE__,__LINE__,api_name,szErrorText); }
#endif 

/* NOTE: This code MUST be the LAST file included so that ascii versions of the system APIs are used     */
#if defined(OS390) && (__STRING_CODE_SET__==ISO8859-1 /* If os390 ascii compile                          */)     
#include <_Ascii_a.h>
#endif

/* Local function prototypes */

void  LNPUBLIC  ProcessArgs (char *ServerName, char *DBFileName, int *ArgNumber);


/* Program declaration */
int main(int argc, char *argv[])
{
	/* Local data declarations */


	char       pname[MAXPATH] = "";         /* buffer to store the input path to database */
	char       *path_name;                  /* pathname of database */
	char       *server_name;                /* server name where database lives*/
	char       *db_name;                    /* name of database */
	DBHANDLE   db_handle = NULLHANDLE;                   /* database handle */
	char       buffer[NSF_INFO_SIZE] = "";  /* database info buffer */
	char       title[NSF_INFO_SIZE] = "";   /* database title */
	STATUS     error = NOERROR;             /* error code from API calls */
	int        ArgNum = 0;

#if defined(OS390)
	char XLATE_path_name[MAXPATH] = "";     /* path name translation buffer */
	char XLATE_title[NSF_INFO_SIZE] = "";   /* database title translation buffer */
#endif

	NOTEHANDLE rethProfileNote;

	/* Initialize pointers to point to "" */
	path_name = pname;
	server_name = pname;
	db_name = pname;

	ArgNum = argc;

	error = NotesInitExtended (argc, argv);

	if (error)
	{
	    fprintf (stderr, "\nError initializing Notes.\n");
	    return (1);
	}

	PRINTLOG("sizeof HANDLE %d\n", sizeof(HANDLE));
	PRINTLOG("sizeof MEMHANDLE %d\n", sizeof(MEMHANDLE));
	PRINTLOG("sizeof DHANDLE %d\n", sizeof(DHANDLE));

	/* Terminate Domino and Notes. */

	NotesTerm();

	/* End of intro program. */

	return (0);
}

/************************************************************************

FUNCTION:   ProcessArgs

INPUTS:

OUTPUTS:    DBFileName, inputted by user

*************************************************************************/

void  LNPUBLIC  ProcessArgs (char *ServerName, char *DBFileName, int *ArgNumber)
{
	printf("Enter name of server where database resides (RETURN for local access): ");
	fflush(stdout);
	gets(ServerName);

	printf("Enter name of database: ");
	fflush(stdout);
	gets(DBFileName);

	if (!strcmp(ServerName, ""))
	{
	    *ArgNumber = 2;
	}

} /* ProcessArgs */

