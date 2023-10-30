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
#include "../../printLog.h"
#if defined(OS390)
#include "lapicinc.h"
#endif
#include "lapiplat.h"

/* NOTE: This code MUST be the LAST file included so that ascii versions of the system APIs are used     */
#if defined(OS390) && (__STRING_CODE_SET__==ISO8859-1 /* If os390 ascii compile                          */)     
#include <_Ascii_a.h>
#endif

/* Local function prototypes */

void  LNPUBLIC  ProcessArgs (char *ServerName, char *DBFileName, int *ArgNumber);
void PrintAPIError (STATUS);


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

    /* Initialize pointers to point to "" */
    path_name = pname;
    server_name = pname;
    db_name = pname;

    ArgNum = argc;

    /* Get the pathname of the database from the command line. */
#if defined(OS390) && (__STRING_CODE_SET__==ISO8859-1 /* If os390 ascii compile                          */)     
    __argvtoascii_a(argc,argv);                      /* Convert args from EBCDIC (Native z/OS) to ASCII */
#endif
    if((ArgNum < 2) || (ArgNum >3))
    {
        PRINTLOG( "\nUsage:  %s  [server name - optional] <database filename>\n", argv[0] );
        return (0);
    }

    db_name = argv[ArgNum - 1];
    path_name = db_name;


    if (ArgNum == 3)
    {
        server_name = argv[1];
    }


    error = NotesInitExtended (argc, argv);

    if (error)
    {
        fprintf (stderr, "\nError initializing Notes.\n");
        return (1);
    }

    if (strcmp (server_name, ""))
    {
        if (error = OSPathNetConstruct( NULL, server_name, db_name, pname))
        {
            PrintAPIError (error);
            NotesTerm();
            return (1);
        }
        path_name = pname;
    }

    /* Open the database. */

    if (error = NSFDbOpen (path_name, &db_handle))
    {
        PrintAPIError (error);
        NotesTerm();
        return (1);
    }

    /* Get the database title. */

    if (error = NSFDbInfoGet (db_handle, buffer))
    {
        PrintAPIError (error);
        NSFDbClose (db_handle);
        NotesTerm();
        return (1);
    }

    NSFDbInfoParse (buffer, INFOPARSE_TITLE, title, NSF_INFO_SIZE - 1);


    /* Print the title. */

#if defined(OS390) && (__STRING_CODE_SET__!=ISO8859-1 /* ebcdic compile */)
    OSTranslate(OS_TRANSLATE_LMBCS_TO_NATIVE, path_name, MAXWORD, XLATE_path_name, sizeof(XLATE_path_name));

    OSTranslate(OS_TRANSLATE_LMBCS_TO_NATIVE, title, MAXWORD, XLATE_title, sizeof(XLATE_title));
    PRINTLOG ("\n\n\nThe title for the database %s is...\n\n%s\n", XLATE_path_name, XLATE_title);
#else
    PRINTLOG ("\n\n\nThe title for the database, %s, is:\n\n%s\n\n", path_name, title);
#endif /* OS390, ebcdic compile */

    /* Close the database. */

    if (error = NSFDbClose (db_handle))
    {
        PrintAPIError (error);
        NotesTerm();
        return (1);
    }

    /* Terminate Domino and Notes. */

    NotesTerm();

    /* End of intro program. */

    return (0);
}


/* This function prints the HCL C API for Notes/Domino error message
associated with an error code. */

void PrintAPIError (STATUS api_error)

{
    STATUS  string_id = ERR(api_error);
    char    error_text[200];
    WORD    text_len;
#if defined(OS390) && (__STRING_CODE_SET__!=ISO8859-1 /* ebcdic compile */)
    char    NATIVE_error_text[200];
#endif /* OS390, ebcdic compile */

    /* Get the message for this HCL C API for Notes/Domino error code
    from the resource string table. */

    text_len = OSLoadString (NULLHANDLE,
                             string_id,
                             error_text,
                             sizeof(error_text));

    /* Print it. */

#if defined(OS390) && (__STRING_CODE_SET__!=ISO8859-1 /* ebcdic compile */)
    OSTranslate(OS_TRANSLATE_LMBCS_TO_NATIVE, error_text, MAXWORD, NATIVE_error_text, sizeof(NATIVE_error_text));
    fprintf (stderr, "\n%s\n", NATIVE_error_text);
#else
    fprintf (stderr, "\n%s\n", error_text);
#endif /* OS390, ebcdic compile */

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

