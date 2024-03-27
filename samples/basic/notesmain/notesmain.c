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

    PROGRAM:    notesmain

    FILE:       notesmain.c

    SYNTAX:     notesmain [server name - optional] <database filename>

    PURPOSE:    Illustrates the basic structure of a HCL C API
                NotesMain application for Domino and Notes.

    DESCRIPTION:
        This program illustrates the basic structure of a HCL C API
        application for Domino and Notes. Using NotesMain as the main entry
        point, this  application opens the specified local or remote
        Domino database, reads the database title, and prints the
        information on the screen.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* OS and C include files */

#include <stdio.h>
#include <string.h>

/* HCL C API for Notes/Domino include files containing the C API
   Version #
 */

#include <global.h>
#include <nsfdb.h>
#include <nsfdata.h>
#include <osfile.h>
#if defined(CAPI_TESTING) 
#include "printlog.h" 
#else
 #define PRINTLOG printf 
 #define PRINTERROR(api_error, api_name) {\
 char    szErrorText[256] = { 0 };\
 OSLoadString(NULLHANDLE, ERR(api_error), szErrorText, sizeof(szErrorText));\
 fprintf(stderr, "[ERROR]:%s:%d:%s - %s", __FILE__,__LINE__,api_name,szErrorText); }
#endif 

#ifndef OSMISC_DEFS
#include <osmisc.h>
#endif

#if defined(OS390) && (__STRING_CODE_SET__==ISO8859-1 /* ascii compile */)
#include <_Ascii_a.h>   /* NOTE: must be the LAST file included */
#endif /* OS390, ascii compile */



/************************************************************************

    FUNCTION:   NotesMain

    PURPOSE:    Main HCL C API for Notes/Domino subroutine

*************************************************************************/

STATUS LNPUBLIC NotesMain(int argc, char far *argv[])
{
    char       pname[MAXPATH] = "";         /* buffer to store the input path to database */
    char       *path_name;                  /* pathname of database */
    char       *server_name;                /* server name where database lives*/
    char       *db_name;                    /* name of database */

    DBHANDLE    db_handle = NULLHANDLE;     /* database handle */
    char        buffer[NSF_INFO_SIZE] = ""; /* database info buffer */
    char        title[NSF_INFO_SIZE] = "";  /* database title */
    STATUS      error = NOERROR;            /* error code from API calls */


#if defined(OS390)
    char XLATE_prog_name[MAXPATH] = "";     /* program name translation buffer */
    char XLATE_path_name[MAXPATH] = "";     /* path name translation buffer */
    char XLATE_title[NSF_INFO_SIZE] = "";   /* database title translation buffer */
#endif

/* Initialize pointers to point to "" */
    path_name = pname;
    server_name = pname;
    db_name = pname;

/* On OS390, all args come in LMBCS for BOTH ascii and ebcdic */

    if((argc < 2) || (argc >3))
    {
#if defined(OS390) && (__STRING_CODE_SET__!=ISO8859-1 /* ebcdic compile */)
        OSTranslate(OS_TRANSLATE_LMBCS_TO_NATIVE, argv[0], MAXWORD, XLATE_prog_name, sizeof(XLATE_prog_name));
        PRINTLOG( "\nUsage:  %s  [server name - optional] <database filename>\n", XLATE_prog_name);

#else
        PRINTLOG( "\nUsage:  %s  [server name - optional] <database filename>\n", argv[0] );
#endif
        return (0);
    }

    db_name = argv[argc - 1];
    path_name = db_name;

    if (argc == 3)
    {
        server_name = argv[1];

        if (strcmp(server_name, ""))
        {
            if (error = OSPathNetConstruct( NULL, server_name, db_name, pname))
            {
                return (ERR(error));
            }
            path_name = pname;
        }
    }

/* Open the database. */

    if (error = NSFDbOpen (path_name, &db_handle))
        return (ERR(error));

  /* Get the database title. */

    if (error = NSFDbInfoGet (db_handle, buffer))
    {
        NSFDbClose (db_handle);
        return (ERR(error));
    }

    NSFDbInfoParse (buffer, INFOPARSE_TITLE, title, NSF_INFO_SIZE - 1);


    /* Print the information. */

#if defined(OS390) && (__STRING_CODE_SET__!=ISO8859-1 /* ebcdic compile */)
    OSTranslate(OS_TRANSLATE_LMBCS_TO_NATIVE, path_name, MAXWORD, XLATE_path_name, sizeof(XLATE_path_name));
    OSTranslate(OS_TRANSLATE_LMBCS_TO_NATIVE, title, MAXWORD, XLATE_title, sizeof(XLATE_title));
    PRINTLOG ("\n\n\nThe title for the database, %s, is:\n\n%s\n", XLATE_path_name, XLATE_title);
#else
    PRINTLOG ("\n\n\nThe title for the database, %s, is:\n\n%s\n\n", path_name, title);
#endif /* OS390, ebcdic compile */

    /* Close the database. */

    if (error = NSFDbClose (db_handle))
        return (ERR(error));

    /* End of subroutine. */
    PRINTLOG("\nProgram completed successfully\n");
    return (NOERROR);
}

