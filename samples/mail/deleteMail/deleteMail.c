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

    PROGRAM:    deleteMail

    FILE:       deleteMail.c

    SYNTAX:     deleteMail <database>

    PURPOSE:    This is a sample program used for automation to delete the docuements

    DESCRIPTION:
          This program scan the given DATABASE to find all notes documents,
          deletes them and print note id of them.
          
****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* HCL C API for Notes/Domino include files */
#include <stdio.h>
#include <string.h>

/* HCL C API for Notes/Domino include files */
#include <global.h>
#include <globerr.h>
#include <oserr.h>
#include <names.h>
#include <stdnames.h>                   /* FIELD_FORM, etc */
#include <osfile.h>
#include <nsf.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <osenv.h>                      /* OSGetEnvironmentString */
#include <mail.h>
#include <osmisc.h>
#include <osmem.h>
#include "../../printLog.h"


#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/* constants  */
#define NULLPTR 0
#define USASCII		"US-ASCII"
#define MAX_DB_NAME	256

STATUS LNPUBLIC EnumProc(void far *, SEARCH_MATCH far *, ITEM_TABLE far *);

/* Global variables */
int docCount;

/************************************************************************

    FUNCTION:   main

*************************************************************************/
int main(int argc, char * argv[])
{
    STATUS      error = NOERROR;

    DBHANDLE    hDB=NULLHANDLE;
    NOTEHANDLE  hNote=NULLHANDLE;
    char        szDbName[MAX_DB_NAME]; 
    char        szServerName[MAX_DB_NAME]; 
    char        szDbPath[MAXPATH+1];
    char        formula[] = "@All";     /* an ASCII selection formula. */
    FORMULAHANDLE    formula_handle;    /* a compiled selection formula */
    WORD     	wdc;                    /* a word we don't care about */
     
    /* Initiate  the environment */
    memset(szDbName, '\0', MAX_DB_NAME);
    memset(szServerName, '\0', MAX_DB_NAME);
    memset(szDbPath, '\0', MAXPATH+1);

    error = NotesInitExtended (argc, argv);
    if ( error != NOERROR )
    {
        PRINTLOG("\n Unable to initialize Notes. Error Code[0x%04x]\n", error);    
        goto exit;
    }
    
    /* Check the input */
    if ( argc > 3 )
    {   
        PRINTLOG ("Error: incorrect syntax.\n");
        PRINTLOG ("\nUsage:%s <serverName> <database>\n", argv[0]);
        PRINTLOG("\nServerName is optional\n");
        NotesTerm();
        return 0;
    }

    if ( argc == 3 ) 
    {
        strncpy( szServerName, argv[1], MAX_DB_NAME-1 );
        strncpy( szDbName, argv[2], MAX_DB_NAME-1 );
    }
    else if (argc == 2) 
    {
    if (!OSGetEnvironmentString("MAILSERVER", szServerName, MAXUSERNAME))
    {
        PRINTLOG ("\nUnable to get mail server name ...\n\n ");
        strcpy(szServerName,"");
    }
        strncpy( szDbName, argv[1], MAX_DB_NAME-1); 
    }
    else 
    {
        PRINTLOG ("Error: incorrect syntax.\n");
        PRINTLOG ("\nUsage:%s <serverName> <database>\n", argv[0]);
        PRINTLOG("\nserverName is optional\n");

        NotesTerm();
        return 0;
    }

    OSPathNetConstruct( NULL,               /* port name  */
                        szServerName,
                        szDbName,
                        szDbPath);


    /* Open the DB */
    PRINTLOG("\n db path %s", szDbPath);
    error = NSFDbOpen (szDbPath, &hDB);
    if ( error )
    {
        PRINTERROR(error,"NSFDbOpen");
        goto exit;
    }

    error = NSFFormulaCompile (
                               NULL,               
                               (WORD) 0,           
                               formula,            
                               (WORD) strlen(formula),   
                               &formula_handle,    
                               &wdc,               
                               &wdc,               
                               &wdc, &wdc, &wdc, &wdc); 
    if ( error != NOERROR )
    {
        PRINTERROR(error,"NSFFormulaCompile");
        goto exit;
    }

    error = NSFSearch (
                       hDB,      
                       formula_handle, 
                       NULL,           
                       0,              
                       NOTE_CLASS_DOCUMENT,
                       NULL,         
                       EnumProc, 
                       &hDB, 
                       NULL);
    if ( error != NOERROR ) 
    {
        PRINTERROR(error,"NSFSearch");
        goto exit;
    }

    OSMemFree(formula_handle);
exit:

    if( hDB )
       NSFDbClose(hDB);

    NotesTerm();

    if ( error )
    {
        PRINTLOG( " Fail to get doc from %s.\n", szDbName);
        return (1);
    }
    else
    {    
        PRINTLOG ( "succeed return docs deleted [%d].\n", docCount );
        return (NOERROR);
    }
}


/************************************************************************

    FUNCTION:   EnumProc

    PURPOSE:    Called by NSFSearch for each note that matches the 
                selection criteria

    INPUTS:

    RETURNS:   
        NOERROR or STATUS if an error condition is encountered.

    DISCUSSION:

 *************************************************************************/
STATUS  LNPUBLIC  EnumProc(void far *phDB, SEARCH_MATCH far *pSearchMatch, ITEM_TABLE far *summary_info)
{
    SEARCH_MATCH SearchMatch;
    STATUS       error;

    memcpy( (char*)&SearchMatch, (char*)pSearchMatch, sizeof(SEARCH_MATCH) );

    if (!(SearchMatch.SERetFlags & SE_FMATCH))
        return (NOERROR);

    /* Print the note ID. */
    PRINTLOG ("Note ID is: [%lX].\n", SearchMatch.ID.NoteID);

    /* Delete Note */
    error = NSFNoteDelete(*(DBHANDLE far *)phDB, SearchMatch.ID.NoteID, 0);
    
    if (error)
        return(error);

    docCount++;
/* End of subroutine. */

    return (NOERROR);
}
