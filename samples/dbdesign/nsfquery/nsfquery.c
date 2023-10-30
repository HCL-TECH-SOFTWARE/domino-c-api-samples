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

        PROGRAM:        nsfquery

        FILE:                nsfquery.c
        
        PURPOSE:
        This program demonstrates how to use the 
        NSFQueryDBExt2 API.

        SYNTAX:
        nsfquery  <db filename>
                or
        nsfquery
        
        Example: nsfquery simple.nsf

        Description:
        This program writes the queries to a file, extracts the query from 
        the file to query the database and then deletes the file.
        
****************************************************************************/

/* C include files. */
#include <stdio.h>
#include <string.h>

/* HCL C API for Notes/Domino include files. */

#include <global.h>
#include <nsfdb.h>
#include <osmem.h>
#include <dbmiscods.h>
#include <misc.h>
#include <dbmisc.h>
#include <miscerr.h>
#include <oserr.h>
#include <osmisc.h>
#include "../../printLog.h"

#define  STRING_LENGTH  256
#define  OUTFILE  "query.tmp"

/* Function prototypes for local functions. */

void  LNPUBLIC  ProcessArgs (int argc, char *argv[], char *szDBFileName);
BOOL LNPUBLIC NSFQueryTest (DHANDLE hDB);

/************************************************************************

        FUNCTION:   main API subroutine

*************************************************************************/

int main(int argc, char *argv[])
{

    /* Local data declarations. */
    FILE     *outFile = NULL;
    char     *szDBFileName = NULL;                                /* name of the database */
    char     szDBBuf[STRING_LENGTH] = {0};
    DBHANDLE hDB = NULLHANDLE;                                /* handle of the database */
    STATUS   error = NOERROR;                                /* return status from API calls */

    szDBFileName = szDBBuf;
    
    ProcessArgs (argc, argv, szDBFileName);

    /* Initialize Domino and Notes. */

    if (error = NotesInitExtended (argc, argv))
    {
            PRINTERROR (error,"NotesInitExtended");
            return (1);
    }

    outFile = fopen (OUTFILE, "w");
    if (!outFile)
    {
        PRINTLOG("Error opening the file %s.", OUTFILE);
        return (1);
    }
    else
    {
        fprintf(outFile, "PLAIN_TEXT = \'This is another line of simple text.\'\n");
        fprintf(outFile, "NUMBER = 836\n");
        fprintf(outFile, "TIME_DATE = @dt('2019-01-01T16:42:00')\n");
        fclose(outFile);
    }

    /* Open the database. */

    if (error = NSFDbOpen (szDBFileName, &hDB))
    {
        PRINTERROR (error,"NSFDbOpen");
        NotesTerm();
        return (1);
    }

    PRINTLOG("Testing NSFQueryDBExt2:\n");
    PRINTLOG("-----------------------\n");

    if (NSFQueryTest (hDB))
    {
        PRINTLOG("NSFQueryDBExt2 was successful.\n");
    }
    else
    {
        PRINTLOG("NSFQueryDBExt2 failed.\n");
    }

    /* Close the database. */

    if (error = NSFDbClose (hDB))
    {
        PRINTERROR (error,"NSFDbClose");
        NotesTerm();
        return (1);
    }

    /* End of subroutine. */
    NotesTerm ();
    if (error == NOERROR)
    {
        PRINTLOG("\nProgram completed successfully.\n");
        return (0);
    }
}

/************************************************************************

        FUNCTION:        NSFQueryTest

        PURPOSE:
        Extracts query from the file and runs it on the database.

        INPUTS:
        Database handle.

        OUTPUTS:
        Returns TRUE if the database is queried, else returns FALSE.

*************************************************************************/

BOOL LNPUBLIC NSFQueryTest (DHANDLE hDB)
{
    FILE         *inFile = NULL;
    STATUS       error = NOERROR;
    char         szQuery[MAXPATH] = {0};           /* Place to store query */
    DHANDLE      hRetResults = NULLHANDLE;         /* To store results */
    MEMHANDLE    hRetError = NULLMEMHANDLE;        /* To store the error */
    MEMHANDLE    hRetExplain = NULLMEMHANDLE;      /* To store the explanation of query */
    WORD         wTranslatedKeyLen = 0;
    char         szTranslatedKey[MAXPATH] = {0};   /* To store LMBCS query */
    
    inFile = fopen (OUTFILE, "r");
    if (!inFile)
    {
        PRINTLOG("Error opening the file %s.", OUTFILE);
        return FALSE;
    }
    else
    {
        while (fgets (szQuery, MAXPATH, inFile))
        {
            /* Removes trailing new line character from szQuery */

            szQuery[strlen(szQuery) - 1] = '\0';

            /* Converts query to LMBCS. */

            wTranslatedKeyLen = OSTranslate (OS_TRANSLATE_NATIVE_TO_LMBCS, szQuery, (WORD) strlen (szQuery), szTranslatedKey, sizeof(szTranslatedKey));

            /* To run DQL queries on the database. */

            if (error = NSFQueryDBExt2 (hDB, szTranslatedKey, wTranslatedKeyLen, QUEP_EXPLAIN, 0, 0, 0, &hRetResults, &hRetError, &hRetExplain, NULL))
            {
                goto errout;
            }
            if (hRetExplain != NULLMEMHANDLE)
            {
                char *explain = (char *)OSMemoryLock(hRetExplain);
                PRINTLOG("%s\n", explain);
                OSMemoryUnlock(hRetExplain);
                OSMemoryFree(hRetExplain);
                hRetExplain = NULLMEMHANDLE;
            }
            if (hRetResults != NULLHANDLE)
            {
                OSMemFree(hRetResults);
                hRetResults = NULLHANDLE;
            }
            if (hRetError != NULLMEMHANDLE)
            {
                OSMemoryFree(hRetError);
                hRetError = NULLMEMHANDLE;
            }
            memset(szQuery, '\0', sizeof(szQuery));
            memset(szTranslatedKey, '\0', sizeof(szTranslatedKey));
        }
            fclose(inFile); 
            remove(OUTFILE);
    }
    return TRUE;

errout:
    PRINTERROR (error,"NSFQueryDBExt2");
    return FALSE;
}

/************************************************************************

        FUNCTION:        ProcessArgs

        PURPOSE:
        To parse command line arguments.

        INPUTS:
        argc, argv - directly from the command line.
   
        OUTPUTS:
        szDBFileName - data from the command line or 
        the user input when prompted.

*************************************************************************/

void  LNPUBLIC  ProcessArgs (int argc, char *argv[], char *szDBFileName)
{ 
    if (argc != 2)  
    {
        printf("Enter name of database: ");      
        fflush(stdout);
        fgets(szDBFileName, STRING_LENGTH-1, stdin);
    }
    else
    {
        strncpy(szDBFileName, argv[1], STRING_LENGTH-1);
    } /* end if */
} /* ProcessArgs */
