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

    PROGRAM:    runmacro

    FILE:       runmacro.c
             
    SYNTAX:     runmacro  <database filename>  "<filter macro name>"

    PURPOSE:    shows how to run a filter macro or background macro in a
                Domino database via the API.

    DESCRIPTION:
        This sample runs the specified macro in the specified database,
        with the limitations outlined below.

        The command line - driven, character mode nature of this program
        limits the kinds of macros it can run. Specifically, 
        this program does not receive control when documents are pasted 
        or mailed into a database. Therefore, it does not execute macros
        of type MAIL (Run when documents pasted/mailed into database). 
        Since it does not take an individual Note ID or set of selected 
        notes as input, it does not execute macros of type ONCE (Execute
        once).

        This program does run Filter macros (Type MENU) and background
        macros (Type BACKGROUND). This program runs Filter macros with
        operation setting = "update existing documents when run" or 
        "create new document when run".
        
        This program does not run in the context of a view so it can not
        yield a list of selected notes as output. Therefore, it does not
        run macros with operation = "Select Documents when Run."

        Since this program is not run from within an open view, it
        can not obtain, as input, the list of the documents selected in
        the view. Therefore, this program does not run macros where the 
        run options specify "Run on selected documents in the view".

        Since this programs does not have access to the list of unread
        documents for a given user in a particular database, it does
        not run macros where the Run options specify "Run on documents
        not yet marked read by you."

        This program does not run Search macros.

        The function MacroIsSupported() in this program checks the 
        specified macro against all of the above limitations. It rejects 
        any macro that the program does not support.

        Some supported macros contain @-functions that have no effect 
        outside the Domino and Notes user interface. Specifically, 
        macros with formulas that contain any of the following 
        @-functions will run successfully but have no effect:

                        DB
                        DbLOOKUP
                        DbCOLUMN
                        QUERY
                        COMMAND
                        MAILSEND
                        DDEINITIATE
                        DDETERMINATE
                        DDEEXECUTE
                        DDEPOKE

        This limitation arises because NSFFormulaEvaluate() does not
        actually perform the above functions. The functions listed above
        have no effect unless performed by the Domino and Notes user 
        interface.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* OS and C header files */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* HCL C API for Notes/Domino Header files.*/

#include <global.h>
#include <nsf.h>
#include <nif.h>
#include <kfm.h>                        /* SECKFMGetUserName */
#include <idtable.h>                    /* IDEnumerate, etc. */
#include <osmem.h>
#include <stdnames.h>
#include <nsfobjec.h>                   /* NSFDbReadObject */
#include <nsfsearc.h>
#include <editods.h>
#include <osmisc.h>
#include "../../printLog.h"


/* program specific header file */
#include "runmacro.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

#define  STRING_LENGTH  256

void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                char *szDbName, char *szMacroName, char *szOptions);

/************************************************************************

    FUNCTION:   main

    PURPOSE:    Main Notes C API for Domino and Notes Subroutine

    ALGORITHM:
    To run a macro, this program reads the information stored in the
    macro note, determines what set of documents in the database need
    to be processed, then processes each document in turn. 

    Runmacro interprets the values of three items, $Type, $Scan, and 
    $Operation, to determine when the macro runs, what documents it will 
    process, and how it will affect those documents. 
   
    Runmacro uses the $Type field to make sure the macro is a menu or 
    background macro. It uses the $Scan field to determine what set of
    documents in the target database need to be processed. It uses the
    $Operation field to determine whether to modify the document itself
    or to create a copy of the modified document. 
   
    To process one document, runmacro applies the macro formulas to
    the documents. Macro formulas reside in the special fields named
    $Formula and $Formula2. Runmacro reads the $Formula and $Formula2 
    items but does not interpret them. To apply a macro formula to a 
    document, runmacro opens the document note, then calls the API 
    function NSFComputeEvaluate(). NSFComputeEvaluate() evaluates a
    formula in the context of a document and applies any changes to 
    the document that are specified by the formula.

    Runmacro also uses the $MachineName field and the $Query field. 
    
    If the macro is a background macro, the $MachineName field contains 
    the name of the Domino and Notes system where the macro should run. 
    READMACRO checks that the user name in the currently active ID file 
    is the same as the name stored in the $MachineName field. If the 
    name in the ID file is not the same as the name in the $MachineName 
    field, then runmacro prints an error message and does not run the 
    macro.

    The "-s" option of runmacro will update the $MachineName field with 
    the user name from the currently active ID file.

    Runmacro rejects the macro if it detects the $Query field because
    this field indicates that the macro is a full text search macro.
    
    Runmacro keeps track of which documents are processed and which
    documents get deleted by the action of the formulas. 

    If the "Run Macro on" setting specifies that the macro runs only
    on documents not previously processed by the macro, then runmacro 
    must maintain the $LeftToDo item.  
    
    The $LeftToDo item is an object that resides in all macros that 
    have the setting, "Run Macro on all documents not previously processed 
    by this macro." The $LeftToDo object contains an ID table with the 
    note IDs of all documents that the macro has not yet processed.
    runmacro maintains this ID table each time it runs a macro.
    
    Before running a macro, runmacro retrieves the ID table 
    from the $LeftToDo object in the macro note. It updates this ID table 
    with the IDs of all documents that have been updated or modified since 
    the last time the macro ran. The resulting ID table identifies the set 
    of documents that the macro needs to process this time. 
    
    After running the macro, runmacro deletes from this ID table the IDs 
    of all documents processed this time through. Runmacro stamps the ID 
    table with the time and date it was updated, and saves it in the 
    $LeftToDo object associated with the macro note. 
    
    This mechanism allows the macro note itself to store the list of 
    documents that the macro processed so that Domino and Notes or API 
    programs that run macros can avoid processing documents 
    unnecessarily.
    
*************************************************************************/

int main(int argc, char *argv[])
{
    STATUS      error = NOERROR;
    char        szDbName[STRING_LENGTH];
    char        szMacroName[STRING_LENGTH];
    char        szOptions[STRING_LENGTH];
    DBHANDLE    hDb;
    NOTEID      nidMacro;
    NOTEHANDLE  hMacro;
    WORD        wType, wOperation, wScan;
    BLOCKID     bidFormula1, bidFormula2;
    BOOL        fSearchMacro;           /* TRUE if is a search macro */
    DHANDLE     hNoteIDTable;           /* notes to process */
    DHANDLE     hProcessedIDTable;      /* notes actually processed */
    DHANDLE     hDeletedIDTable;        /* notes deleted */
    DWORD       dwNotesDone;
    NOTEID      nidDoc;
    HCOMPUTE    hCompute1, hCompute2;
    BOOL        fCompute1Started, fCompute2Started;
    BOOL        bSetMachineName;        /* Just set the machine name */


    if (error = NotesInitExtended (argc, argv))
    {
        PRINTLOG("\n Unable to initialize Notes.\n");
        return (1);
    }

    /* Get the command line parameters that the user entered. */
    ProcessArgs(argc, argv, szDbName, szMacroName, szOptions);
    bSetMachineName = FALSE;
    if (szOptions[0] == '-')
    {
        if ( (szOptions[1] == 's') || (szOptions[1] == 'S')) bSetMachineName = TRUE;
    }

    /* Create all ID tables used in this program. Free all these later. */
    if (error = CreateIDTables( &hNoteIDTable, &hDeletedIDTable,
                                &hProcessedIDTable))
    {
        goto Exit0;
    }

    /* Open target database */
    if (error = NSFDbOpen(szDbName, &hDb))
    {
        PRINTLOG ("Error: unable to open target database '%s'\n", szDbName);
        goto Exit1;
    }

    /* Find the specified macro */
    if (error = NIFFindDesignNote( hDb, szMacroName, NOTE_CLASS_FILTER,
                                   &nidMacro ))
    {
        PRINTLOG ("Error: unable to find macro '%s' in database '%s'.\n",
                     szMacroName, szDbName);
        goto Exit2;
    }

    /* Open the macro note */
    if (error = NSFNoteOpen(hDb, nidMacro, 0, &hMacro))
    {
        PRINTLOG ("Error: unable to open macro note '%s'.\n", 
                   szMacroName);
        goto Exit2;
    }

    /* Get the type, operation, scan, and other info from Macro */
    if (error = GetMacroInfo( hMacro, &wType, &wOperation, &wScan, 
                              &bidFormula1, &bidFormula2, &fSearchMacro ))
    {
        goto Exit3;
    }

    /* Make sure macro is supported by this program */
    if (error = MacroIsSupported(hMacro, szMacroName, wType, wOperation, 
                                 wScan, fSearchMacro))
    {
        goto Exit3;
    }

    /* Option -s: Just set MacroMachineName so it can be run as a background Macro */
    if (bSetMachineName)
    {
        error = SetMacroMachineName(hMacro, wType);
        /* That's all we want to do; close the note (any errors were shown) */
        NSFNoteClose(hMacro);
        NSFDbClose(hDb);
        goto Exit1;
    }

    /* If background macro, make sure user name is same as $MachineName */
    if (error = RunMacroOnThisMachine(hMacro, wType))
    {
        goto Exit3;
    }

   /* 
    * Fill hNoteIDTable with the IDs of all the documents this macro should
    * process. This set of IDs depends on the $Scan item ("Run macro on:").
    * If Scan = NEW ("run macro on documents not yet processed by this 
    * macro"), but there are no new documents in the database, then this 
    * function will return ERR_RUNMACRO_NONEWNOTES. In this case, stop here.
    * On successful return, hNoteIDTable contains the IDs we want to 
    * process.
    */

    if (error = GetTableOfIDsToProcess(hDb, hMacro, wScan, hNoteIDTable))
    {
        if (error == ERR_RUNMACRO_NONEWNOTES)
        {
            PRINTLOG ("Macro didn't process any documents.\n");
        }
        goto Exit3;
    }

    fCompute1Started = FALSE;
    fCompute2Started = FALSE;

    /* set up for a series of formula compilations */
    error = NSFComputeStart(0, 
                            (OSLockBlock(char,bidFormula1) + sizeof(WORD)),
                            &hCompute1);
    OSUnlockBlock(bidFormula1);
    if (error)
    {
        PRINTLOG ("Error: unable to start compute of formula 1.\n");
        goto Exit3;
    }
    fCompute1Started = TRUE;
                           
    if (bidFormula2.pool != NULLHANDLE) /* bidFormula2 was initialized */
    {                                   /* earlier by GetMacroInfo     */
        error = NSFComputeStart(0,
                    (OSLockBlock(char,bidFormula2) + sizeof(WORD)),
                    &hCompute2);
        OSUnlockBlock(bidFormula2);
        if (error)
        {
            PRINTLOG ("Error: unable to start compute of formula 2.\n");
            goto Exit4;
        }
        fCompute2Started = TRUE;
    }

    /*  Loop over note IDs in the table. Call ProcessOneNote on each. */

    for (dwNotesDone=0L; 
        IDScan(hNoteIDTable, (FLAG) (dwNotesDone++ == 0), &nidDoc); )
    {
        if (error = ProcessOneNote( hDb, hMacro, nidDoc, 
                                    wType, wOperation, wScan, 
                                    fCompute1Started, hCompute1, 
                                    fCompute2Started, hCompute2,
                                    hDeletedIDTable, hProcessedIDTable ))
        {
            goto Exit4;
        }
    }

    dwNotesDone--;
    PRINTLOG("Macro processing completed for %ld %s.\n",  
              dwNotesDone, ((dwNotesDone==1) ? "document" : "documents") );

Exit4:
    if (fCompute1Started)   NSFComputeStop(hCompute1);
    if (fCompute2Started)   NSFComputeStop(hCompute2);

Exit3:
    NSFNoteClose(hMacro);
    
Exit2:
    NSFDbClose(hDb);

    if ((error == NOERROR) && (wScan == FILTER_SCAN_NEW))
    {
        /* Update $LeftToDo item in macro note with ID tables. */
        error = UpdateLeftToDo( szDbName, nidMacro, hDeletedIDTable, 
                                                    hProcessedIDTable);
    }

Exit1:
    IDDestroyTable(hNoteIDTable);
    IDDestroyTable(hProcessedIDTable);
    IDDestroyTable(hDeletedIDTable);

Exit0:
   /* 
    * If error was defined by RUNMACRO, change it to NOERROR before 
    * returning, because Domino and Notes won't recognize errors 
    * defined by RUNMACRO.
    */
    if ( (error>=ERR_RUNMACRO_MIN) && (error<=ERR_RUNMACRO_MAX) )
    {
        error = NOERROR;
    }
    if (error)
    {
        PRINTERROR(error, "main");
    }
    NotesTerm();
    return(error);
}

/************************************************************************

    FUNCTION:   GetMacroInfo 

    PURPOSE:    Get Macro note type, operation, scan, formulas,
                and a flag indicating whether this macro is
                a full text search macro (contains a $Query).

    INPUTS:     NOTEHANDLE  hMacro    - handle to open macro note

    OUTPUTS:     
        WORD         *pwType            - FILTER_TYPE_MENU, etc.
        WORD         *pwOperation       - FILTER_OP_UPDATE, etc.
        WORD         *pwScan            - FILTER_SCAN_ALL, etc. 
        BLOCKID      *pbidFormula1      - "$Formula" item value blockid 
        BLOCKID      *pbidFormula2      - "$Formula2" item value blockid,
                                          or NULLHANDLE/NULLBLOCK if not available
        BOOL         *pfSearchMacro     - gets TRUE if $Query item in macro

*************************************************************************/

STATUS  LNPUBLIC  GetMacroInfo( NOTEHANDLE    hMacro, 
                                 WORD         *pwMacroType,
                                 WORD         *pwOperation,
                                 WORD         *pwScan, 
                                 BLOCKID      *pbidFormula1, 
                                 BLOCKID      *pbidFormula2,
                                 BOOL         *pfSearchMacro)
{
    STATUS      error=NOERROR;
    char        ach[4];
    WORD        wDataType;
    BLOCKID     bidValue;
    DWORD       dwValueLength;

    /* "$Type" */
    NSFItemGetText( hMacro, FILTER_TYPE_ITEM, ach, sizeof(ach)-1 );
    *pwMacroType = (WORD)(ach[0] - '0');

    /* "$Operation" */
    NSFItemGetText( hMacro, FILTER_OPERATION_ITEM, ach, sizeof(ach)-1 );
    *pwOperation = (WORD)(ach[0] - '0');

    /* "$Scan" */
    NSFItemGetText( hMacro, FILTER_SCAN_ITEM, ach, sizeof(ach)-1 );
    *pwScan = (WORD)(ach[0] - '0');

    /* "$Query" */
    error = NSFItemInfo(hMacro, FILTER_QUERY_ITEM, 
                        sizeof(FILTER_QUERY_ITEM)-1, NULL,
                        &wDataType, &bidValue, &dwValueLength);

    if( (error == NOERROR) && 
        (wDataType == TYPE_TEXT || wDataType == TYPE_TEXT_LIST) )
    {
        *pfSearchMacro = TRUE;
    }
    else
    {
        *pfSearchMacro = FALSE;
    }

    /* "$Formula" */
    error = NSFItemInfo(hMacro, FILTER_FORMULA_ITEM, 
                        sizeof(FILTER_FORMULA_ITEM)-1, NULL,
                        &wDataType, &bidValue, &dwValueLength);
    if ( (ERR(error) == ERR_ITEM_NOT_FOUND) && (*pfSearchMacro == TRUE) )
    {
        error = NOERROR;    /* search macros don't require $Formula */
    }
    else if (error)
    {
        PRINTLOG ("Error: unable to get primary formula from macro note.\n");
        return(error);
    }
    else
    {
        if (wDataType != TYPE_FORMULA)
        {
            PRINTLOG ("Error: item '%s' not TYPE_FORMULA.\n", FILTER_FORMULA_ITEM);
            return (ERR_RUNMACRO_WRONGTYPE);
        }
        if (dwValueLength == sizeof(WORD))
        {
            PRINTLOG ("Error: zero length item '%s'.\n", FILTER_FORMULA_ITEM);
            return (ERR_RUNMACRO_ZEROLENGTH);
        }
        /* else, looks like a good formula item */
        pbidFormula1->pool = bidValue.pool;
        pbidFormula1->block = bidValue.block;
    }
    /* pre-init *pbidFormula2 to NULLHANDLE/NULLBLOCK so can return */
    pbidFormula2->pool = NULLHANDLE;
    pbidFormula2->block = NULLBLOCK;

    /* "$Formula2" */
    if (error = NSFItemInfo(hMacro, FILTER_FORMULA2_ITEM, 
                            sizeof(FILTER_FORMULA2_ITEM)-1, NULL,
                            &wDataType, &bidValue, &dwValueLength))
    {
        if (ERR(error) == ERR_ITEM_NOT_FOUND)
        {
            return(NOERROR);
        }
        PRINTLOG ("Error: unable to get secondary formula from macro note.\n");
        return(error);
    }
    if (wDataType != TYPE_FORMULA)
    {
        return (NOERROR);
    }
    if (dwValueLength == sizeof(WORD))
    {
        return (NOERROR);
    }
    pbidFormula2->pool = bidValue.pool;
    pbidFormula2->block = bidValue.block;

    return(NOERROR);
}
/************************************************************************

    FUNCTION:   MacroIsSupported 

    PURPOSE:    Make sure the macro is supported by this program.

    ALGORITHM:   
        MAIL:  Not supported
        ONCE:  Not supported
        SEARCH: Not supported
        BACKGROUND:  OK
        MENU:
            if ($Operation == SELECT)
            then    Not suppored
            else switch ($Scan)
            {
                case UNREAD:   Not supported
                case SELECTED: Not supported
                case ALL:  OK
                case NEW:  OK
                case VIEW: OK
            }
*************************************************************************/
 
STATUS LNPUBLIC MacroIsSupported( NOTEHANDLE hMacro, 
                                  char * szMacroName,
                                  WORD wType, WORD wOperation,
                                  WORD wScan, BOOL fSearchMacro )
{
    if ( (wType == FILTER_TYPE_MAIL) || (wType == FILTER_TYPE_ONCE) ||
          fSearchMacro )
    {
        PRINTLOG ("Error: '%s' not a Filter nor a Background macro.\n",
                   szMacroName);
        return ERR_RUNMACRO_UNSUPPORTED;
    }

    if ( wType == FILTER_TYPE_BACKGROUND )
    {
        return NOERROR;
    }
    if ( wType != FILTER_TYPE_MENU )
    {
        PRINTLOG ("Error: unrecognized macro type %#x\n", wType);
        return ERR_RUNMACRO_UNSUPPORTED;
    }
    if (wOperation == FILTER_OP_SELECT )
    {
        PRINTLOG ("Error: Operation not supported: \n\t'%s'\n",
                  "Select documents when run");
        return ERR_RUNMACRO_UNSUPPORTED;
    }
    switch (wScan)
    {
        case FILTER_SCAN_ALL:
        case FILTER_SCAN_NEW:
        case FILTER_SCAN_VIEW:
            return NOERROR;
        case FILTER_SCAN_UNREAD:
            PRINTLOG ("Error: Setting not supported: \n\t'%s'\n",
                      "Run on documents not yet marked read by you.");
            break;
        case FILTER_SCAN_SELECTED:
            PRINTLOG ("Error: Setting not supported: \n\t'%s'\n",
                      "Run on selected documents in view.");
            break;
        case FILTER_SCAN_MAIL:    /* Domino and Notes does not use this setting */
            PRINTLOG ("Error: Setting not supported: \n\t'%s'\n",
                      "Run on documents mailed/pasted into database");
            break;
        default:
            PRINTLOG ("Error: unrecognized run option $Scan = %#x\n", wScan);
            break;;
    }
    return ERR_RUNMACRO_UNSUPPORTED;
}

/************************************************************************

    FUNCTION:   RunMacroOnThisMachine

    PURPOSE:    If background macro, make sure user name (from ID file)
                agrees with the value of $MachineName in the macro.
                  
    RETURNS:    NOERROR - ok to run macro on this machine 
                ERR_RUNMACRO_NOTTHISMACHINE - name does not match user name
                ERR_XXXX  - error codes from lower level API functions

*************************************************************************/

STATUS  LNPUBLIC  RunMacroOnThisMachine ( NOTEHANDLE hMacro,
                                                WORD wType)
{
    STATUS      error=NOERROR;
    char        szUserName[MAXUSERNAME+1];
    char        szMachineName[MAXUSERNAME+1];
    WORD        i;
    char        *strPtr=NULL;
   
    if (wType != FILTER_TYPE_BACKGROUND)
    {
        return NOERROR;
    }
    if (error = SECKFMGetUserName(szUserName))
    {
        PRINTLOG ("Error: unable to get user name from ID file.\n");
        return(error);
    }

    /* "$MachineName" */
    NSFItemGetText( hMacro, FILTER_MACHINE_ITEM, szMachineName, MAXUSERNAME );
   
    /* lower case both names because there is no stricmp on the Macintosh */
    for (i = 0,strPtr = szUserName; i < strlen(szUserName); i++,strPtr++)
        *strPtr = tolower(*strPtr);
   
    for (i = 0,strPtr = szMachineName; i < strlen(szMachineName); i++,strPtr++)
        *strPtr = tolower(*strPtr);

    if (strcmp(szUserName, szMachineName) == 0)
    {
        return NOERROR;
    }

    PRINTLOG ("Error: background macro runs only on machine '%s'.\n",
               szMachineName);

    return ERR_RUNMACRO_NOTTHISMACHINE;
}

/************************************************************************

    FUNCTION:   SetMacroMachineName

*************************************************************************/

STATUS  LNPUBLIC  SetMacroMachineName( NOTEHANDLE hMacro, WORD wType )
{
    STATUS      error=NOERROR;
    char        szUserName[MAXUSERNAME+1];
      
    if (wType != FILTER_TYPE_BACKGROUND)
    {
        return NOERROR;
    }
    if (error = SECKFMGetUserName(szUserName))
    {
        PRINTLOG ("Error: unable to get user name from ID file.\n");
    return(error);
    }
    if (error = NSFItemSetText(hMacro,
                FILTER_MACHINE_ITEM,    /* "$MachineName" */
                szUserName, MAXWORD))
    {
        PRINTLOG ("Error: unable to set Machine Name field in macro note.\n");
        return(error);
    }
    if (error = NSFNoteUpdate(hMacro, 0))
    {
        PRINTLOG ("Error: unable to update macro note.\n");
        return(error);
    }
    PRINTLOG("Macro updated to run on %s\n", szUserName);

    return(error);
}

/************************************************************************

    FUNCTION:   GetTableOfIDsToProcess

    PURPOSE:    Get ID table containing the NOTEIDs of all the notes
                this macro should process.
                
    INPUT:      hNoteIDTable specifies an empty, initialized ID table.  

    OUTPUT:     on successful return, hNoteIDTable gets the Note IDs
                of all the documents this macro should process.

    DESCRIPTION:
        This handles three cases: (1) Scan = ALL. The macro runs on all 
        documents in the database. This fills the table with the IDs of 
        all the notes of class NOTE_CLASS_DOCUMENT in the database. (2) 
        Scan = VIEW. The macro runs on all documents in the view.  This 
        uses the default view in the database. It fills the table with the 
        IDs of all the documents in the default view (but not the category 
        notes). (3) Scan = NEW. The macro runs on all documents not yet 
        processed by this macro. This gets the ID table from the $LeftToDo 
        object and updates it with the IDs of documents that are new or 
        modified since the time/date stored in the table. This function 
        will return ERR_RUNMACRO_NONEWNOTES if the macro is supposed to 
        run on documents not yet processed by this macro and there are no 
        new documents in the database since the macro last ran. 

*************************************************************************/

STATUS  LNPUBLIC  GetTableOfIDsToProcess( DBHANDLE    hDb,
                                          NOTEHANDLE  hMacro, 
                                          WORD        wScan, 
                                          DHANDLE       hNoteIDTable)
{
    STATUS      error=NOERROR;

    switch (wScan)
    {
        case FILTER_SCAN_ALL:           /* get IDs of all docs in DB */
            error = GetIDsOfAllDocsInDb(hDb, hNoteIDTable);
            break;
        case FILTER_SCAN_VIEW:          /* get IDs of all in default view */
            error = GetIDsOfAllDocsInView(hDb, hNoteIDTable);
            break;
        case FILTER_SCAN_NEW:           /* New or changed since last run */
            error = GetIDsOfAllNewDocs(hDb, hMacro, hNoteIDTable);
            break;
        case FILTER_SCAN_UNREAD:        /* none of these should happen */
        case FILTER_SCAN_SELECTED:
        case FILTER_SCAN_MAIL:
        default:
            PRINTLOG ("Error: bad $Scan value detected: %#x.\n", wScan);
            error = ERR_RUNMACRO_BADSCAN;
            break;
    }
    return (error);
}

/************************************************************************

    FUNCTION:   GetIDsOfAllDocsInDb 

*************************************************************************/

STATUS  LNPUBLIC  GetIDsOfAllDocsInDb( DBHANDLE hDb, DHANDLE hNoteIDTable)
{
    STATUS      error=NOERROR;

    if (error = NSFSearch ( hDb, NULLHANDLE, NULL, 0, 
                            NOTE_CLASS_DOCUMENT,/* find all document notes */
                            NULL,           /* starting date (unused) */
                            AddIDUnique,    /* call for each note found */
                            &hNoteIDTable,  /* argument to AddIDUnique */
                            NULL))          /* returned ending date (unused) */
    {
        PRINTLOG ("Error: unable to search for all docs in database.\n");
    }
    return(error);
}    
    
/************************************************************************

    FUNCTION:   GetIDsOfAllDocsInView 

    PURPOSE:    Fill ID table with the IDs of all the documents in 
                the default view.

    DESCRIPTION:
        Other applications may run in the context of a particular view,
        in which case you would want the IDs of all the documents in 
        that view. RUNMACRO does not provides a way for the user to 
        specify any particular view. Therefore, just use the default view.
        
    COMMENT:
        RUNMACRO does not support Full Text Search macros. If it did,
        however, this is the routine that would perform the full text
        query. Full Text search queries yield a collection of all the 
        notes that match the query. This would obtain the table of Note 
        IDs to process from the resulting collection.

*************************************************************************/

STATUS  LNPUBLIC  GetIDsOfAllDocsInView ( DBHANDLE    hDb, 
                                          DHANDLE       hNoteIDTable)
{
    STATUS      error=NOERROR;
    WORD        wClass;
    NOTEID      ViewID;
    HCOLLECTION hCollection;
    COLLECTIONPOSITION CollPosition;
    DHANDLE     hBuffer;
    DWORD       dwEntriesFound, i;
    WORD        wSignalFlag;
    NOTEID     *IdList;
    BOOL        flagOK;

    ViewID = (DWORD)0;

    /* Search the DB for the default view note. Get it's ID. */
    wClass = NOTE_CLASS_VIEW | NOTE_CLASS_DEFAULT;

    if (error = NSFSearch ( hDb, NULLHANDLE, NULL, 0, 
                            wClass,         /* find the default view note */
                            NULL,           /* starting date (unused) */
                            GetNoteID,      /* call for each note found */
                            (void*)(&ViewID), /* argument to GetnoteID */
                            NULL))          /* returned ending date (unused) */

    {
        PRINTLOG ("Error: unable to search for default view in database.\n");
        goto Exit0;
    }

    /* Open the view. */                                
    if (error = NIFOpenCollection ( hDb, hDb, ViewID, 0, NULLHANDLE, 
                                    &hCollection, NULLHANDLE, NULL, NULLHANDLE, NULLHANDLE))
    {
        PRINTLOG ("Error: unable to open default view collection.\n");
        goto Exit0;
    }

    /* Read all the entries in the view. Add each NOTEID to the table. */
    CollPosition.Level = 0;
    CollPosition.Tumbler[0] = 0;

    do
    {
        if (error = NIFReadEntries( hCollection, &CollPosition, 
                                    NAVIGATE_NEXT, 1L, NAVIGATE_NEXT, MAXDWORD,
                                    READ_MASK_NOTEID, &hBuffer, NULL, NULL,
                                    &dwEntriesFound, &wSignalFlag))
        {
            PRINTLOG ("Error detected reading entries in default view.\n");
            goto Exit1;
        }

        if (hBuffer == NULLHANDLE)
        {
            PRINTLOG ("Error: no documents (empty buffer) in default view.\n");
            error = ERR_RUNMACRO_EMPTYCOLL;
            goto Exit1;
        }

        IdList = (NOTEID *) OSLockObject (hBuffer);

        for (i=0; i<dwEntriesFound; i++)
        {
            if (NOTEID_CATEGORY & IdList[i])
            {
                continue;
            }
            if (error = IDInsert(hNoteIDTable, IdList[i], &flagOK))
            {
                PRINTLOG ("Error: unable to insert note ID into table.\n");
                OSUnlockObject (hBuffer);
                OSMemFree (hBuffer);
                goto Exit1;
            }
        }

        OSUnlockObject (hBuffer);
        OSMemFree (hBuffer);
    }  while (wSignalFlag & SIGNAL_MORE_TO_DO);

Exit1:
    NIFCloseCollection(hCollection);

Exit0:
    return(error);
}

/************************************************************************

    FUNCTION:   GetIDsOfAllNewDocs

    PURPOSE:    Fill the ID table with the note IDs of all documents 
                that have not yet been processed by this macro. 
                
    DESCRIPTION:
        This function gets the $LeftToDo object in the Macro note and
        retrieves the ID table stored in this object. Next, it updates 
        this table with the IDs of all documents added to, modified in, 
        or deleted from, the database since the last time the macro ran.
        Then it copies all the Note IDs from the updated table to the 
        table specified by hNoteIDTable.

        If the $LeftToDo object is not available in the macro, or if the
        ID table is not relevant, this fills the return ID table with the
        IDs of all the documents in the DB (by calling GetIDsOfAllDocsInDb).

*************************************************************************/

STATUS LNPUBLIC GetIDsOfAllNewDocs(   DBHANDLE    hDb, 
                                        NOTEHANDLE  hMacro, 
                                        DHANDLE       hRetIDTable)
{
    STATUS      error=NOERROR;
    DHANDLE     hLeftToDo;          /* the ID table in the $LeftToDo object */
    TIMEDATE    tdLeftToDoTime;
    WORD        wLeftToDoFlags;
    BYTE       *p=NULL;
    TIMEDATE    tdSavedModificationTimedate;
    TIMEDATE    tdMacroModificationTimedate;
    DHANDLE     hIDsModified;       /* IDs of docs modified since TableTime */
    DBID        dbidSavedMacro;
    DBID        dbidCurrent;
    void        *ptable=NULL;
    DWORD       dw;
    NOTEID      nid;
    BOOL        flag;
    OBJECT_DESCRIPTOR  objLeftToDo; /* describes the $LeftToDo object */

    /* Get the "$LeftToDo" item */
    error = ReadLeftToDoObject( hDb, hMacro, 
                                &objLeftToDo,   /* gets the object Type and ID */
                                &hLeftToDo,     /* gets handle to the ID table */
                                &tdLeftToDoTime,/* gets Time member of ID table */
                                &wLeftToDoFlags);/* "" Flags member of ID table */

    if ( ERR(error) == ERR_ITEM_NOT_FOUND )
    {
        goto NewTable;
    }
    else if (error)
    {
        return(error);
    }
   /*
    * The $LeftToDo object consists of an ID Table, followed by the
    * TIMEDATE the macro was last modified, followed by the DBID of
    * the database in which the macro resided when it last ran.
    * Use the Time field in the ID Tabel to record when the table was 
    * last updated. Retrieve the last modified TIMEDATE and the DBID.
    * Check them to ensure the ID Table is relevant to us.
    */

    p = OSLock(BYTE, hLeftToDo);        /* p points to the ID Table */
    p+=IDTableSize(hLeftToDo);          /* increment p past the ID Table */
                                        /* p now points to the TIMEDATE */
    tdSavedModificationTimedate = *((TIMEDATE*)p);  /* get the TIMEDATE */
    p+=sizeof(TIMEDATE);                /* increment p past the TIMEDATE */
    dbidSavedMacro = *((DBID*)p);       /* get the DBID saved in Macro */
    OSUnlock(hLeftToDo);                /* unlock handle we are done with */

    /* Ignore ID table if macro note changed since ID table was updated */
    NSFNoteGetInfo(hMacro, _NOTE_MODIFIED, &tdMacroModificationTimedate);
    if (TimeDateCompare( &tdSavedModificationTimedate,
                         &tdMacroModificationTimedate))
    {
        goto NewTable;                  /* TIMEDATEs are different. */
    }

    /* Ignore ID table if macro was copied from a different database */
    if (error = NSFDbIDGet(hDb, &dbidCurrent))
    {
        PRINTLOG ("Error: unable to get database ID.\n");
        return (error);
    }

    if (TimeDateCompare((TIMEDATE*)&dbidSavedMacro,(TIMEDATE*)&dbidCurrent))
    {
        goto NewTable;                  /* DBIDs are different. */
    }

    /* Ignore ID table if the Time member of the ID table is all zeros */
    if ( (tdLeftToDoTime.Innards[0] == (DWORD)0) &&
         (tdLeftToDoTime.Innards[1] == (DWORD)0)   )
    {
        goto NewTable;
    }
   
   /* 
    * Now get hIDsModified, a different ID Table containing the IDs of any
    * NOTE_CLASS_DOCUMENT notes that were created or modified since the
    * Time stored in hLeftToDo. 
    */

    error = NSFDbGetModifiedNoteTable( hDb, NOTE_CLASS_DOCUMENT,
                            tdLeftToDoTime, &tdLeftToDoTime, &hIDsModified);

    if ( (error) && (error != ERR_NO_MODIFIED_NOTES) )
    {
        PRINTLOG("Error detected scanning DB for modified notes.\n");
        return(error);
    }

   /* 
    * If ERR_NO_MODIFIED_NOTES, then hIDsModified is uninitialized,
    * so don't call IDEnumerate below. However, tdLeftToDoTime may be
    * changed (even though error is nonzero) so update the table time
    * and set the IDTABLE_MODIFIED flag in the ID Table.
    */

    ptable = OSLock(void, hLeftToDo);
    IDTableSetTime(ptable, tdLeftToDoTime);
    IDTableSetFlags(ptable, (WORD)(wLeftToDoFlags | IDTABLE_MODIFIED));
    OSUnlockObject(hLeftToDo);

   /*
    * The hIDsModified table may contain added and deleted notes. Merge 
    * these two tables by adding new IDs to hLeftToDo and removing 
    * deleted IDs from hLeftToDo. Deleted note IDs are marked.
    */

    if (error == NOERROR)
    {
        ptable = OSLock(void, hLeftToDo);

        error = IDEnumerate(hIDsModified, MergeNoteIDs, (void*)(&hLeftToDo));

        OSUnlockObject(hLeftToDo);
        IDDestroyTable(hIDsModified);

        if (error)
        {
            PRINTLOG("Error: unable to enumerate IDs of new notes in DB.\n");
            return(error);
        }
    }


    if (IDEntries(hLeftToDo) == 0)
    {
        error = ERR_RUNMACRO_NONEWNOTES;
    }
    else
    {   
        /* The ID Table from $LeftToDo is valid. Copy IDs to return table. */
        for (dw=0L; IDScan(hLeftToDo, (FLAG) (dw++ == 0), &nid); )
        {
            if (error = IDInsert(hRetIDTable, nid, &flag))
            {
                PRINTLOG ("Error: unable to insert note ID into table.\n");
                break;
            }
        }
    }

    /* free the handle */
    OSMemFree(hLeftToDo);
    return (error);

NewTable:

    return( GetIDsOfAllDocsInDb(hDb, hRetIDTable) );
}

/************************************************************************

    FUNCTION:   UpdateLeftToDo

    PURPOSE:    Update the $LeftToDo object in the macro note.
    
    DESCRIPTION:
        Get the $LeftToDo object from the macro note. Retrieve the 
        ID table from this object. Remove all Processed note IDs and 
        all Deleted Note IDs from this ID table. Update the Time member 
        of the ID table, the Modification TIMEDATE that follows the ID 
        table, and the DBID that follows the Modification TIMEDATE.
        Save the resulting LeftToDo object in the Macro note and clean up.
        
*************************************************************************/

STATUS LNPUBLIC UpdateLeftToDo(char * szDbName, NOTEID nidMacro, 
                        DHANDLE hDeletedIDTable, DHANDLE hProcessedIDTable)
{
    STATUS              error=NOERROR;
    DBHANDLE            hDb;
    NOTEHANDLE          hMacro;
    OBJECT_DESCRIPTOR   objLeftToDo;
    DHANDLE             hLeftToDo;          /* the ID table in the $LeftToDo object */
    TIMEDATE            tdLeftToDoTime;
    WORD                wLeftToDoFlags;
    DHANDLE             hIDsModified;
    DWORD               dw;
    NOTEID              nid;
    BOOL                flag;
    DWORD               dwNeededSize, tdOffset, dwExistingSize;
    WORD                wClass, wPrivs;
    BYTE                *p;
    void                *ptable;

    /* Re-open the database. Re-open the macro. Get the $LeftToDo item. */
    if (error = NSFDbOpen(szDbName, &hDb))
    {
        PRINTLOG ("Error: unable to re-open target database '%s'\n", szDbName);
        goto Exit0;
    }

    if (error = NSFNoteOpen(hDb, nidMacro, 0, &hMacro))
    {
        PRINTLOG ("Error: unable to re-open macro note.\n");
        goto Exit1;
    }

    error = ReadLeftToDoObject( hDb, hMacro, 
                                &objLeftToDo,   /* gets the object Type and ID */
                                &hLeftToDo,     /* gets handle to the ID table */
                                &tdLeftToDoTime,/* gets Time member of ID table */
                                &wLeftToDoFlags);/* "" Flags member of ID table */

    if (error)
    {
        if ( ERR(error) == ERR_ITEM_NOT_FOUND )
        {
            error = NOERROR;
        }
        else
        {
            PRINTLOG ("Error: unable to get '%s' object to update in macro.\n",
                      FILTER_LEFTTODO_ITEM);
        }
        goto Exit2;
    }

    /* Update the ID Table to reflect the current state of the database. */
   
    error = NSFDbGetModifiedNoteTable( hDb, NOTE_CLASS_DOCUMENT,
                                       tdLeftToDoTime, &tdLeftToDoTime, &hIDsModified);

    ptable = OSLock(void, hLeftToDo) ;
    IDTableSetTime(ptable, tdLeftToDoTime);
    IDTableSetFlags(ptable, (WORD)(wLeftToDoFlags | IDTABLE_MODIFIED));
    OSUnlock(hLeftToDo);

    if ( (error) && (error != ERR_NO_MODIFIED_NOTES) )
    {
        PRINTLOG("Error detected scanning DB for modified notes.\n");
        goto Exit2;
    }

    if (error == NOERROR)   /* If error, hIDsModified isn't initialized. */
    {
        ptable = OSLock(void, hLeftToDo);
        error = IDEnumerate(hIDsModified, MergeNoteIDs, (void*)&hLeftToDo);
        OSUnlockObject(hLeftToDo);
        IDDestroyTable(hIDsModified);
        if (error)
        {
            PRINTLOG ("Error: unable to enumerate modified note IDs.\n");
            goto Exit2;
        }
    }
   /* 
    * Remove from hLeftToDo the IDs of all notes deleted or processed by
    * this macro.
    */

    for (dw=0L; IDScan(hDeletedIDTable, (FLAG) (dw++ == 0), &nid); )
    {
        if (error = IDDelete(hLeftToDo, nid, &flag))
        {
            PRINTLOG ("Error: unable to delete note ID from table.\n");
            goto Exit2;
        }    
    }

    for (dw=0L; IDScan(hProcessedIDTable, (FLAG) (dw++ == 0), &nid); )
    {
        if (error = IDDelete(hLeftToDo, nid, &flag))
        {
            PRINTLOG ("Error: unable to delete note ID from table.\n");
            goto Exit2;
        }
    }

    /* Calculate size needed for ID Table plus TIMEDATE plus DBID. */

    tdOffset = IDTableSize(hLeftToDo);
    //dwNeededSize = ( tdOffset + sizeof(TIMEDATE) + sizeof(DBID) );
    dwNeededSize =  tdOffset + SIZET_TO_DWORD(sizeof(TIMEDATE)) + SIZET_TO_DWORD(sizeof(DBID));
    /* Reallocate the ID Table to the needed size */

    if (error = OSMemRealloc(hLeftToDo, dwNeededSize))
    {
        PRINTLOG ("Error: unable to re-allocate ID table to %ld bytes.\n",
                   dwNeededSize);
        goto Exit2;
    }

    /* Get the modification TIMEDATE and DBID. Write them after the ID
       Table. */

    p = OSLock(BYTE, hLeftToDo);
    NSFNoteGetInfo(hMacro, _NOTE_MODIFIED, p + tdOffset);
    NSFDbIDGet(hDb, (DBID *) (p + tdOffset + sizeof(TIMEDATE)));
    OSUnlockObject(hLeftToDo);

    /* Get the size of the existing $LeftToDo object. */

    if (error = NSFDbGetObjectSize(hDb, 
                                   (WORD) objLeftToDo.RRV, 
                                   (WORD) objLeftToDo.ObjectType,
                                   &dwExistingSize, &wClass, &wPrivs))
    {
        PRINTLOG ("Error: unable to get size of object '%s' in macro.\n",
                  FILTER_LEFTTODO_ITEM);
        goto Exit2;
    }

    /* Reallocate the existing object if necessary. */

    if (dwExistingSize < dwNeededSize)
    {
        /* realloc 1K more than needed so won't have to realloc as often */
        if (error = NSFDbReallocObject(hDb,
                                       (WORD) objLeftToDo.RRV, dwNeededSize+1024))
        {
            PRINTLOG ("Error: unable to re-allocate '%s' to %ld bytes.\n",
                      FILTER_LEFTTODO_ITEM, dwNeededSize);
            goto Exit2;
        }
    }

    /*  Write the object */
    if (error = NSFDbWriteObject(hDb, (WORD) objLeftToDo.RRV, 
                                 hLeftToDo, 0,
                                 dwNeededSize))
    {
        PRINTLOG ("Error: unable to write '%s' object to macro note.\n",
                  FILTER_LEFTTODO_ITEM);
    }

Exit2:
    NSFNoteClose(hMacro);

Exit1:
    NSFDbClose(hDb);

Exit0:
    return (error);
}

/************************************************************************

    FUNCTION:   ReadLeftToDoObject

    PURPOSE:    Read the $LeftToDo object, and return information from it.

    DESCRIPTION:
        Obtain the $LeftToDo object from a macro note and return to caller
        the OBJECT_DESCRIPTOR, the handle to the ID table, the Time member,
        and the Flags member of the ID table.

    INPUTS:
        DBHANDLE    hDb     handle of open database containing the macro
        NOTEHANDLE  hMacro  handle of open macro note

    OUTPUTS:     
        OBJECT_DESCRIPTOR * pObject     gets the OBJECT_DESCRIPTOR structure
        DHANDLE      *phLeftToDo      gets the handle to the $LeftToDo ID table
        TIMEDATE   *ptdLeftToDoTime gets the Time member of the ID table
        WORD       *pwLeftToDoFlags gets the Flags member of the ID table

    RETURNS:   
        NOERROR if successfully returned all the information re: $LeftToDo
        ERR_ITEM_NOT_FOUND - if the $LeftToDo item was not present in macro
        (other nonzero values) - error codes from lower level API functions

*************************************************************************/

STATUS  LNPUBLIC  ReadLeftToDoObject( DBHANDLE    hDb,
                        NOTEHANDLE          hMacro,
                        OBJECT_DESCRIPTOR  *pObject,
                        DHANDLE            *phLeftToDo,
                        TIMEDATE           *ptdLeftToDoTime,
                        WORD               *pwLeftToDoFlags )
{
    STATUS             error=NOERROR;
    WORD               wDataType;
    BLOCKID            bidValue;
    DWORD              dwValueLength;
    void               *ptable=NULL;
    OBJECT_DESCRIPTOR  tempObject;
    OBJECT_DESCRIPTOR  *tempPtr;
   
    error = NSFItemInfo(hMacro, FILTER_LEFTTODO_ITEM, 
                        sizeof(FILTER_LEFTTODO_ITEM)-1,
                        NULL, &wDataType, &bidValue, &dwValueLength);

    if ( ERR(error) == ERR_ITEM_NOT_FOUND )
    {
        return (error);
    }
    else if (error)
    {
        PRINTLOG("Error: unable get '%s' from Macro.\n", FILTER_LEFTTODO_ITEM);
        return(error);
    }
    if (wDataType != TYPE_OBJECT)
    {
        PRINTLOG ("Error: item '%s' not TYPE_OBJECT.\n", FILTER_LEFTTODO_ITEM);
        return(error);
    }
        
    *pObject = *((OBJECT_DESCRIPTOR*)(OSLockBlock(char,bidValue)+sizeof(WORD)));

    tempPtr = pObject;

    ODSReadMemory( &tempPtr, _OBJECT_DESCRIPTOR, &tempObject, 1 );

    if (tempObject.ObjectType != OBJECT_FILTER_LEFTTODO)
    {                                                 
        PRINTLOG ("Error: object '%s' unknown type.\n",FILTER_LEFTTODO_ITEM);
        OSUnlockBlock(bidValue);
        return (ERR_RUNMACRO_BADOBJECTTYPE);
    }

    error = NSFDbReadObject(hDb, tempObject.RRV, 0, MAXDWORD, phLeftToDo);
    OSUnlockBlock(bidValue);
    if (error)
    {
        PRINTLOG ("Error: unable to read object '%s'.\n",FILTER_LEFTTODO_ITEM);
        return (error);
    }

    ptable = OSLock(void, *phLeftToDo);
    *ptdLeftToDoTime = IDTableTime(ptable);
    *pwLeftToDoFlags = IDTableFlags(ptable);
    OSUnlock(*phLeftToDo);              /* unlock handle we are done with */
    
    return NOERROR;
}


/************************************************************************

    FUNCTION:   MergeNoteIDs

    PURPOSE:    Action routine for IDEnumerate. If the NoteID is a
                deleted note, remove it's ID from the ID Table.  Else,
                add it to the ID Table.

*************************************************************************/

STATUS LNPUBLIC MergeNoteIDs (void *vhUnreadList, DWORD NoteID)
{
    DHANDLE *phUnreadList = (DHANDLE *) vhUnreadList;

    if (NoteID & RRV_DELETED)
    {
        return(IDDelete(*phUnreadList, NoteID & ~RRV_DELETED, NULL));
    }
    else
    {
        return(IDInsert(*phUnreadList, NoteID, NULL));
    }
}


/************************************************************************

    FUNCTION:   AddIDUnique

    PURPOSE:    Add the ID of the note to the ID table.
    
    DESCRIPTION:
        This is an action routine specified to NSFSearch when searching 
        for a subset of the documents in the database. This adds the ID 
        of the note found by NSFSearch to the ID table specified by the
        parameter. This adds the ID only if it is not already in the 
        table. This does not insert the IDs of category notes into the 
        table. 

*************************************************************************/

STATUS LNPUBLIC AddIDUnique  (void * phNoteIDTable, 
                                SEARCH_MATCH far *pSearchMatch,
                                ITEM_TABLE *summary_info)
{
    SEARCH_MATCH SearchMatch;
    DHANDLE      hNoteIDTable;
    STATUS       error=NOERROR;
    BOOL         flagOK;

    memcpy( (char*)&SearchMatch, (char*)pSearchMatch, sizeof(SEARCH_MATCH) );

    if (!(SearchMatch.SERetFlags & SE_FMATCH))
        return (NOERROR);

    if (NOTEID_CATEGORY & (SearchMatch.ID.NoteID))
        return (NOERROR);

    hNoteIDTable = *((DHANDLE *)phNoteIDTable);

    if (error = IDInsert(hNoteIDTable, SearchMatch.ID.NoteID, &flagOK))
    {
        PRINTLOG ("Error: unable to insert note ID into table.\n");
    }
    return (error);
}

/************************************************************************

    FUNCTION:   GetNoteID

    PURPOSE:    Return the Note ID of the specified note to the variable
                pointed to by the first parameter. This is the 
                action routine specified to NSFSearch when searching
                for the default view note in the database. NSFSearch
                should only find one such note, so this routine should
                only get called once.

*************************************************************************/

STATUS LNPUBLIC GetNoteID(void * pNote, SEARCH_MATCH far *pSearchMatch,
                          ITEM_TABLE *summary_info)
{
    SEARCH_MATCH SearchMatch;

    memcpy( (char*)&SearchMatch, (char*)pSearchMatch, sizeof(SEARCH_MATCH) );

    if (!(SearchMatch.SERetFlags & SE_FMATCH))
        return (NOERROR);

    *((NOTEID*)pNote) = SearchMatch.ID.NoteID;

    return(NOERROR);
}

/************************************************************************

    FUNCTION:   RemoveNoteID

    PURPOSE:    Action routine for IDEnumerate. Remove the Note ID
                from the ID Table.

*************************************************************************/

STATUS LNPUBLIC RemoveNoteID (void *vTable, DWORD NoteID)
{
    DHANDLE *phTable = (DHANDLE *) vTable;

    IDDelete(*phTable, NoteID, NULL);

    return (NOERROR);
}

/************************************************************************

    FUNCTION:   CreateIDTables

*************************************************************************/

STATUS  LNPUBLIC CreateIDTables(DHANDLE *phIDTable1, DHANDLE *phIDTable2,
                                  DHANDLE *phIDTable3)
{
    STATUS      error=NOERROR;

    if ( (error = IDCreateTable(sizeof(NOTEID), phIDTable1)) ||
         (error = IDCreateTable(sizeof(NOTEID), phIDTable2)) ||
         (error = IDCreateTable(sizeof(NOTEID), phIDTable3)) )
    {
        PRINTLOG ("Error: unable to create ID table.\n");
    }
    return (error);
}

/************************************************************************

    FUNCTION:   ProcessOneNote 

*************************************************************************/

STATUS  LNPUBLIC  ProcessOneNote( 
                                 DBHANDLE hDb, NOTEHANDLE hMacro, NOTEID nidDoc, 
                                 WORD wType, WORD wOperation, WORD wScan,
                                 BOOL fCompute1Started, HCOMPUTE hCompute1,
                                 BOOL fCompute2Started, HCOMPUTE hCompute2,
                                 DHANDLE hDeletedIDTab, DHANDLE hProcessedIDTab)
{
    STATUS      error=NOERROR;
    NOTEHANDLE  hDoc;
    BOOL        fDocMatchesFormula1, fDocMatchesFormula2;
    BOOL        fDocShouldBeDeleted1, fDocShouldBeDeleted2;
    BOOL        fDocModified1, fDocModified2;
    OID         oidNewDoc;
    BOOL        fUpdate;
    WORD        wFlags;

    /* initialize all NSFComputeEvaluate output flags */
    fDocMatchesFormula1 = fDocMatchesFormula2 = FALSE;
    fDocShouldBeDeleted1 = fDocShouldBeDeleted2 = FALSE;
    fDocModified1 = fDocModified2 = FALSE;

    /* Open the note */
    if (error = NSFNoteOpen( hDb, nidDoc, 
                             (OPEN_EXPAND |          /* makes all text into text lists */
                             OPEN_NOVERIFYDEFAULT), 
                             &hDoc))                 /* hDoc gets handle to open note */
    {
        PRINTLOG ("Error: unable to open document to process.\n");
        goto Exit0;
    }

    /* Compute the first formula on the document. */
    if (fCompute1Started)
    {
        if (error = NSFComputeEvaluate( 
                                        hCompute1,           /* returned from NSFComputeStart */
                                        hDoc,                /* This note provides any additional
                                                               variable used by the formula. It
                                                               also gets FIELD variables assigned
                                                               during the evaluation. */
                                        NULLHANDLE,          /* don't need the ComputeResult */
                                        NULL,                /* don't need Compute Result Len */
                                        &fDocMatchesFormula1,/* gets TRUE if note matches the
                                                                SELECT portion of the formula */
                                        &fDocShouldBeDeleted1,/* gets TRUE if formula 
                                                                 indicates note should be deleted*/
                                        &fDocModified1 ))    /* gets TRUE if any of the fields
                                                                of hDoc were modified as result
                                                                of the formulat evaluation. */
        {
            PRINTLOG ("Error: unable to compute forumula on document.\n");
            goto Exit1;
        }
    }
   /* 
    * Proceed to compute the second (optional) formula on the note if
    * there is a second formula, and the note matched the SELECT portion
    * of the first formula, and the first formula does not indicate that
    * the note should be deleted.
    */
    if (fCompute2Started && fDocMatchesFormula1 && !fDocShouldBeDeleted1)
    {
        if (error = NSFComputeEvaluate( hCompute2, hDoc, NULLHANDLE, NULL,
                                        &fDocMatchesFormula2,
                                        &fDocShouldBeDeleted2,
                                        &fDocModified2 ))
        {
            PRINTLOG ("Error: unable to compute formula 2 on document.\n");
            goto Exit1;
        }
    }

   /* 
    * If the macro is supposed to create a new document when run,
    * then make the in-memory copy of the note look like a new note
    * in the database. Do this by creating a new ORIGINATORID, setting
    * the NOTEID to zero, and setting the new ORIGINATORID into the
    * note header data. NSFNoteUpdate will create a new NOTEID when
    * it saves this in-memory copy of the note to disk.
    */
    if ( (wOperation == FILTER_OP_NEW_COPY) && 
        !fDocShouldBeDeleted1 && !fDocShouldBeDeleted2 )
    {
        if (error = NSFDbGenerateOID(hDb, &oidNewDoc))
        {
            goto Exit1;
        }
        NSFNoteSetInfo(hDoc, _NOTE_ID,  0);
        NSFNoteSetInfo(hDoc, _NOTE_OID, &oidNewDoc);         
    }

    /* This note has been processed. Add it's ID to table if SCAN_NEW */
    if (wScan == FILTER_SCAN_NEW)
    {
        IDInsert(hProcessedIDTab, nidDoc, NULL);
    }

    /* If the note should be deleted, delete it now. */
    if ( fDocShouldBeDeleted1 || fDocShouldBeDeleted2 )
    {
        NSFNoteDelete(hDb, nidDoc, UPDATE_NOCOMMIT);
        if (wScan == FILTER_SCAN_NEW)
        {
            IDInsert(hDeletedIDTab, nidDoc, NULL);
        }
    }
    else
    {   /* Decide whether or not note needs to be updated. */
        fUpdate = fDocModified1 || fDocModified2;

        /* Don't update if note is abstracted (e.g. a partial replica) */
        NSFNoteGetInfo(hDoc, _NOTE_FLAGS, &wFlags);
        if (wFlags & NOTE_FLAG_ABSTRACTED)
        {   
            fUpdate = FALSE;
        }

        /* Don't update if note is a replication conflict */
        if (NOERROR == NSFItemInfo( hDoc, VIEW_CONFLICT_ITEM, 
                                    sizeof(VIEW_CONFLICT_ITEM)-1,
                                    NULL, NULL, NULL, NULL))
        {
            fUpdate = FALSE;
        }
        
        /* Update the note if necessary */
        if (fUpdate)
        {
            error = NSFNoteUpdate(hDoc,
                                  UPDATE_FORCE | UPDATE_NOCOMMIT | UPDATE_DUPLICATES );
            if (error == ERR_CONFLICT)
            {
                PRINTLOG ("Nonfatal error updating modified document %#lx.\n",
                            nidDoc);
                PRINTLOG ("\tSomeone else modified document at the same time.\n");
                PRINTLOG ("\tContinuing...\n");
                error = NOERROR;
            }
            if (error)
            {
                PRINTLOG ("Error: unable to update modified document %#lx\n",
                          nidDoc);
                goto Exit1;
            }
        }
    }
Exit1:
    NSFNoteClose(hDoc);

Exit0:
    return(error);
}


/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:     argc, argv -  directly from the command line
   
    OUTPUTS:    db_name    -  database name obtained from command line or
                              from prompt.
                      
                macro_name -  the name of the macro to run

*************************************************************************/

void  LNPUBLIC  ProcessArgs (int argc, char *argv[], 
                               char *db_name, char *macro_name, char *options)
{ 
    if (argc < 3)
    {
        printf("Enter database name: ");      
        fflush (stdout);
        gets(db_name);
        printf("\n");
        printf ("Enter the macro name:  ");
        fflush (stdout);
        gets(macro_name);
        printf("\n");
        printf("Enter option or press enter for no special options\n");
        printf("  -s     just set machine name in macro; do not run it");
        gets(options);
    }
    else
    {
        strcpy(db_name, argv[1]);
        strcpy(macro_name, argv[2]);
        if (argc == 4) strcpy(options, argv[3]);
    } /* end if */
} /* ProcessArgs */

