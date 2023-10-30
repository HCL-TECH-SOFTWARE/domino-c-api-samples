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

    PROGRAM: addmacro

    FILE:    addmacro.c

    SYNTAX:  addmacro  <database filename>

    PURPOSE: shows how to create various macro notes via the API

    DESCRIPTION:
        This sample creates three macro notes in the specified database:
        a filter macro, a background macro, and an execute-once macro.
        The target database should be PROBLEMS.NSF, the database provided
        with this sample. After running this program, you may execute
        the macros it creates via the Domino and Notes user interface.        

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* C include files */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <nsfdata.h>
#include <nsfsearc.h>                   /* NSFFormulaCompile */
#include <nsfdb.h>
#include <nsfobjec.h>                   /* NSFDbAllocObject, etc. */
#include <nsfnote.h>
#include <kfm.h>                        /* SECKFMGetUserName */
#include <osmem.h>
#include <idtable.h>                    /* IDCreateTable, etc. */
#include <stdnames.h>
#include <ods.h>
#include <odstypes.h>
#include <editods.h>
#include <osmisc.h>
#include "../../printLog.h"

/* Program-specific include files */

#include "addmacro.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/************************************************************************

    FUNCTION:   Main

*************************************************************************/

int main (int argc, char *argv[])
{
    STATUS      error = NOERROR;
    char        *szDbName;
    DBHANDLE    hDb;


    if (error = NotesInitExtended (argc, argv))
    {
        PRINTLOG("\n Unable to initialize Notes.\n");
        return (1);
    }

    /* Process arguments */
    if (argc != 2)
    {
        PRINTLOG ("Usage: addmacro  <database filename>\n");
        goto Exit0;
    }
    szDbName = argv[1];

    /* Open input database */
    if (error = NSFDbOpen(szDbName, &hDb))
    {
        PRINTLOG ("Error: unable to open target database '%s'\n", szDbName);
        goto Exit0;
    }

    /* Create filter macro */
    if (error = AddFilterMacro(hDb))
    {
        goto Exit1;
    }

    /* Create background macro */
    if (error = AddBackgroundMacro(hDb))
    {
        goto Exit1;
    }

    /* Create execute-once macro */
    if (error = AddOnceMacro(hDb))
    {
        goto Exit1;
    }

    PRINTLOG("%s: successfully added 3 macros to '%s'.\n", argv[0], szDbName);

    /* Close database and return */
Exit1:
    NSFDbClose(hDb);

Exit0:
    if (error)
        PRINTERROR (error,"NSFDbOpen");

    NotesTerm();
    return(error);
}

/************************************************************************

    FUNCTION:   AddFilterMacro(DBHANDLE hDb)

    PURPOSE:    Add a filter macro to the specified database.

    DESCRIPTION:
        This function adds a filter macro named "Transfer Silton to Sherman"
        to the open database. After creating this macro, you can run it by
        running Domino and Notes, opening the database, and selecting this 
      macro from the "Tools Run Macros" menu.  

        The 'Run options' setting for this macro is "Run via menu command". 
        Implement this setting by appending to the macro note a TYPE_TEXT 
        item named $Type with value '0' (FILTER_TYPE_MENU).

        The 'Operation' setting for this macro is "Update existing documents 
        when run."  Implement this setting by appending to the note a 
        TYPE_TEXT filed named $Operation with value '0' (FILTER_OP_UPDATE).

        The 'Run macro on' setting for this macro is "Run on all documents 
        in database." Implement this setting by appending a TYPE_TEXT item
        named $Scan with value '0' (FILTER_SCAN_ALL).

        The formula for this macro selects all open problems assigned to
        "Isabel Silton". It re-assignes all the selected problems to 
        "Alexander Sherman".

*************************************************************************/

STATUS  LNPUBLIC  AddFilterMacro( DBHANDLE hDb )
{
    STATUS          error = NOERROR;
    NOTEHANDLE      hMacro;
    char            szFlags[MAX_FLAGS];

    char   szTitle[] = "Transfer Silton to Sherman";
    char   szComment[] = "Re-assign all open problems assigned to Support Rep Isabel \
                          Silton to Support Rep Alexander Sherman.";
    char   szFormula[] = "SELECT (SRep = \"Isabel Silton\")\
                          & (Status = \"Open\"); FIELD SRep := \"Alexander Sherman\";";

    /* Create macro note and set note class to NOTE_CLASS_FILTER */
    if (error = CreateMacroNote(hDb, &hMacro))
    {
        goto Exit0;
    }

    if (error = SetMacroTitle(hMacro, szTitle))     /* $Title */
    {
        goto Exit1;
    }

    if (error = SetMacroComment(hMacro, szComment)) /* $Comment */
    {
        goto Exit1;
    }

    if (error = SetMacroFormula(hMacro, szFormula)) /* $FORMULA */
    {
        goto Exit1;
    }

    if (error = SetMacroType(hMacro, FILTER_TYPE_MENU)) /* $Type */
    {
        goto Exit1;
    }
    
    if (error = SetMacroOperation(hMacro, FILTER_OP_UPDATE))/* $Operation */
    {
        goto Exit1;
    }

    if (error = SetMacroScan(hMacro, FILTER_SCAN_ALL))  /* $Scan */
    {
        goto Exit1;
    }

    szFlags[0] = DESIGN_FLAG_PRESERVE;
    szFlags[1] = '\0';
    if (error = SetMacroFlags(hMacro, szFlags))
    {
        goto Exit1;
    }

    if (error = NSFNoteUpdate(hMacro, 0))
    {
        PRINTLOG ("Error: unable to update macro note to database.\n");
    }

Exit1:
    NSFNoteClose(hMacro);

Exit0:
    return(error);
}

/************************************************************************

    FUNCTION:   AddBackgroundMacro(DBHANDLE hDb)

    PURPOSE:    Add a background macro to the specified database.

    DESCRIPTION:
        This function adds a background macro named "Escalate Low Prio 
        Calls to Med" to the open database. After creating this macro, 
        run it from Domino and Notes by opening the database then selecting 
        "Tools Run Background Macros". Or, the macro will run automatically 
        if you configure 'Background Program' in Domino and Notes User Setup.

        The 'Run options' setting for this macro is "Run periodically in 
        the background". Implement this option by setting the $Type 
        item to "1" (FILTER_TYPE_BACKGROUND).

        The 'Operation' setting for this macro is "Update existing document 
        when run."  Implement this by setting the $Operation field value 
        to "0" (FILTER_OP_UPDATE).

        The 'Run macro on' setting for this macro is "Run on documents not 
        yet processed by macro." Implement this by setting the $Scan field 
        value to "5" (FILTER_SCAN_NEW).

        Background macros must specify the name of the server or workstation 
        on which they run. Get the name of the user running this program 
        from the ID file and set the $MachineName field to the user name.
        
        Background macros also require a 'Frequency' setting. Set the 
        frequency to "weekly" by setting the $Period field value to "2"
        (PERIOD_WEEKLY).

        Any macro with $Scan = FILTER_SCAN_NEW must also have a $LeftToDo
        item. This object tracks maintains information about when the 
        macro was last run, what documents still need to be processed,
        and what database contained the macro when it last ran. 

        The formula for this macro selects all documents opened more than 
        a month ago that are open and low priority. It changes the priority
        of all selected documents to "Medium".

*************************************************************************/

STATUS  LNPUBLIC  AddBackgroundMacro( DBHANDLE hDb )
{
    STATUS          error = NOERROR;
    NOTEHANDLE      hMacro;
    char            szFlags[MAX_FLAGS];

    char            szTitle[]   = "Escalate Low Prio Calls to Med";
    char            szComment[] = "Once per week, search for all open, low priority \
                                   problems opened more than one month ago and escalate to medium priority.";
    char            szFormula[] = "OneMonthAgo := @Adjust(@Now; 0; -1; 0; 0; 0; 0);\n\
                                   SELECT ((DateOpened < OneMonthAgo) & (Status = \"Open\") \
                                   & (Priority = \"Low\"));\n FIELD Priority := \"Medium\";";

    /* Create macro note and set note class to NOTE_CLASS_FILTER */
    if (error = CreateMacroNote(hDb, &hMacro))
    {
        goto Exit0;
    }

    if (error = SetMacroTitle(hMacro, szTitle))     /* $Title */
    {
        goto Exit1;
    }

    if (error = SetMacroComment(hMacro, szComment)) /* $Comment */
    {
        goto Exit1;
    }

    if (error = SetMacroFormula(hMacro, szFormula)) /* $FORMULA */
    {
        goto Exit1;
    }

    if (error = SetMacroType(hMacro, FILTER_TYPE_BACKGROUND)) /* $Type */
    {
        goto Exit1;
    }
 
    if (error = SetMacroOperation(hMacro, FILTER_OP_UPDATE))/* $Operation */
    {
        goto Exit1;
    }

    if (error = SetMacroScan(hMacro, FILTER_SCAN_NEW))  /* $Scan */
    {
        goto Exit1;
    }

    szFlags[0] = DESIGN_FLAG_BACKGROUND_FILTER;
    szFlags[1] = DESIGN_FLAG_PRESERVE;
    szFlags[2] = '\0';
    if (error = SetMacroFlags(hMacro, szFlags))
    {
        goto Exit1;
    }

    if (error = SetMacroMachineName(hMacro))
    {
        goto Exit1;
    }

    if (error = SetMacroPeriod(hMacro, PERIOD_WEEKLY))
    {
        goto Exit1;
    }

    if (error = SetMacroLeftToDo(hDb, hMacro))
    {
        goto Exit1;
    }
/**/
    if (error = NSFNoteUpdate(hMacro, 0))
    {
        PRINTLOG ("Error: unable to update macro note to database.\n");
    }

Exit1:
    NSFNoteClose(hMacro);

Exit0:
    return(error);
}

/************************************************************************

    FUNCTION:   AddOnceMacro(DBHANDLE hDb)

    PURPOSE:    Add an Execute-Once macro to the specified database.

    DESCRIPTION:
        This function adds an Execute-once macro named "Send reminder 
        to Support Rep" to the open database. After adding this macro
        the database, you can run it by starting Domino and Notes, 
      highlighting one document in the view, then selecting this macro 
      from the list presented on the "Tools" menu.

        The 'Run options' setting for this macro is "Execute Once". 
        Implement this by setting the $Type field to "3" (FILTER_TYPE_ONCE).

        Execute Once macros in Notes 3.0 do not allow different settings
        for 'Operation' or 'Run macro on'. Therefore, set the $Operation 
        field to "0" (FILTER_OP_UPDATE) and the $Scan field value to "5" 
        (FILTER_SCAN_NEW).

        The formula for this macro uses the @MailSend function to send
        a reminder message to the support rep named in the SRep field
        of the problem document highlighted in the view.

*************************************************************************/

STATUS  LNPUBLIC  AddOnceMacro( DBHANDLE hDb )
{
    STATUS          error = NOERROR;
    NOTEHANDLE      hMacro;
    char            szFlags[MAX_FLAGS];

    char   szTitle[] = "Send reminder to Support Rep";
    char   szComment[] = "Send a reminder email to the appropriate support rep for the document \
                          highlighted in the view.";
    char   szFormula[] = "@If(SRep!=\"\";\n\
                         @MailSend(SRep; \"\"; \"\"; \"Reminder: \" + CompanyName + \
                         \" problem still open\"; Subject; \"\"; [IncludeDoclink]);\n\
                         @Prompt([OK];\"\";\"You need to highlight a document\"));\n\
                         SELECT @All";

    /* Create macro note and set note class to NOTE_CLASS_FILTER */
    if (error = CreateMacroNote(hDb, &hMacro))
    {
        goto Exit0;
    }

    if (error = SetMacroTitle(hMacro, szTitle))     /* $Title */
    {
        goto Exit1;
    }

    if (error = SetMacroComment(hMacro, szComment)) /* $Comment */
    {
        goto Exit1;
    }

    if (error = SetMacroFormula(hMacro, szFormula)) /* $FORMULA */
    {
        goto Exit1;
    }

    if (error = SetMacroType(hMacro, FILTER_TYPE_ONCE)) /* $Type */
    {
        goto Exit1;
    }
 
    if (error = SetMacroOperation(hMacro, FILTER_OP_UPDATE))/* $Operation */
    {
        goto Exit1;
    }

    if (error = SetMacroScan(hMacro, FILTER_SCAN_NEW))  /* $Scan */
    {
        goto Exit1;
    }

    szFlags[0] = DESIGN_FLAG_PRESERVE;
    szFlags[1] = '\0';
    if (error = SetMacroFlags(hMacro, szFlags))
    {
        goto Exit1;
    }

    if (error = NSFNoteUpdate(hMacro, 0))
    {
        PRINTLOG ("Error: unable to update macro note to database.\n");
    }

Exit1:
    NSFNoteClose(hMacro);

Exit0:
    return(error);
}

/************************************************************************

    FUNCTION:   CreateMacroNote

    PURPOSE:    Create note in database. Set note class to CLASS_FILTER.

    INPUTS:     DBHANDLE    hDb         - database handle

    OUTPUTS:    NOTEHANDLE *phMacro     - gets handle to new macro note 

*************************************************************************/

STATUS  LNPUBLIC  CreateMacroNote ( DBHANDLE hDb, NOTEHANDLE * phMacro )
{
    STATUS      error;
    WORD        wNoteClass;

    if (error = NSFNoteCreate(hDb, phMacro))
    {
        PRINTLOG ("Error: unable to create macro note.\n");
        return(error);
    }

    wNoteClass = NOTE_CLASS_FILTER;
    NSFNoteSetInfo(*phMacro, _NOTE_CLASS, &wNoteClass);

    return(0);
} 

/************************************************************************

    FUNCTION:   SetMacroTitle

*************************************************************************/

STATUS  LNPUBLIC  SetMacroTitle(NOTEHANDLE hMacro, char *szTitle)
{
    STATUS      error=NOERROR;

    if (error = NSFItemSetText(hMacro, 
                VIEW_TITLE_ITEM,        /* "$Title" */
                szTitle, MAXWORD))
    {
        PRINTLOG ("Error: unable to set title of macro note.\n");
    }

    return(error);
}
/************************************************************************

    FUNCTION:   SetMacroComment

*************************************************************************/

STATUS  LNPUBLIC  SetMacroComment (NOTEHANDLE hMacro, char *szComment)
{
    STATUS      error=NOERROR;

    if (error = NSFItemSetText(hMacro, 
                FILTER_COMMENT_ITEM,    /* "$Comment" */
                szComment, MAXWORD))
    {
        PRINTLOG ("Error: unable to set Comment field in macro note.\n");
    }
    return(error);
}
/************************************************************************

    FUNCTION:   SetMacroFormula

*************************************************************************/

STATUS LNPUBLIC  SetMacroFormula( NOTEHANDLE hMacro, char *szFormula )
{
    STATUS          error=NOERROR;
    FORMULAHANDLE   hFormula;
    WORD            wFormulaLen;
    WORD            wdc;

    if (error = NSFFormulaCompile(  NULL, 0, szFormula, (WORD)strlen(szFormula),
                                    &hFormula, &wFormulaLen, &wdc, &wdc, 
                                    &wdc, &wdc, &wdc))
    {
        PRINTLOG ("Error compiling formula.\n");
        return(error);
    }

    error = NSFItemAppend(
                          hMacro,                 /* handle to note to append to */
                          ITEM_SUMMARY,           /* item flags */
                          FILTER_FORMULA_ITEM,    /* item name: "$Formula" */
                          (WORD)strlen(FILTER_FORMULA_ITEM),
                          TYPE_FORMULA,           /* item type */
                          OSLockObject(hFormula), /* item value */
                          wFormulaLen);           /* value length */

    OSUnlockObject(hFormula);
    OSMemFree(hFormula);

    if (error)
    {
        PRINTLOG ("Error: unable to append formula item to macro note.\n");
    }
    return(error);
}

/************************************************************************

    FUNCTION:   SetMacroType

    PURPOSE:    Set the $Type field in the macro note.

    INPUTS:     
        NOTEHANDLE    hMacro    - note handle
        WORD          wType     - Filter type:  must be
                                  one of FILTER_TYPE_MENU, 
                                  FILTER_TYPE_BACKGROUD,
                                  FILTER_TYPE_MAIL, or
                                  FILTER_TYPE_ONCE

*************************************************************************/

STATUS  LNPUBLIC  SetMacroType( NOTEHANDLE hMacro, WORD wType )
{
    STATUS      error=NOERROR;
    char        cType;

    cType = (char)('0' + wType);
    if (error = NSFItemSetText(hMacro,
                FILTER_TYPE_ITEM,   /* "$Type" */
                &cType, 1))
    {
        PRINTLOG("Error: unable to set Type field in macro note.\n");
    }

    return(error);    
}

/************************************************************************

    FUNCTION:   SetMacroOperation

    PURPOSE:    Set the $Operation field in the macro note.

    INPUTS:     
        NOTEHANDLE    hMacro     - note handle
        WORD          wOperation - Operation code:  must be one of
                                FILTER_OP_UPDATE, FILTER_OP_SELECT,
                                or FILTER_OP_NEW_COPY.

*************************************************************************/

STATUS  LNPUBLIC  SetMacroOperation( NOTEHANDLE hMacro, WORD wOperation )
{
    STATUS      error=NOERROR;
    char        cOperation;
                     
    cOperation = (char)('0' + wOperation);
    if (error = NSFItemSetText(hMacro,
                FILTER_OPERATION_ITEM,  /* "$Operation" */
                &cOperation, 1))
    {
        PRINTLOG ("Error: unable to set Operation field in macro note.\n");
    }

    return(error);
}

/************************************************************************

    FUNCTION:   SetMacroScan

    PURPOSE:    Set the "$Scan" field of the macro note

    INPUTS:      
        NOTEHANDLE  hMacro  - handle to the open macro note
        WORD        wScan   - macro scan code. Must be one of
                            FILTER_SCAN_ALL, FILTER_SCAN_UNREAD,
                            FILTER_SCAN_VIEW, FILTER_SCAN_SELECTED, 
                            FILTER_SCAN_MAIL, or FILTER_SCAN_NEW.

*************************************************************************/

STATUS  LNPUBLIC  SetMacroScan( NOTEHANDLE hMacro, WORD wScan )
{
    STATUS      error=NOERROR;
    char        cScan;
     
    cScan = (char)('0' + wScan);
    if (error = NSFItemSetText(hMacro,
                FILTER_SCAN_ITEM,   /* "$Scan" */
                &cScan, 1))
    {
        PRINTLOG ("Error: unable to set Scan field in macro note.\n");
    }

    return(error);
}

/************************************************************************

    FUNCTION:   SetMacroFlags

    PURPOSE:    Set the "$Flags" field of the macro note

    INPUTS:      
        NOTEHANDLE  hMacro  - handle to the open macro note
        char       *szFlags - zero-terminated string continaing any
                            combination of design flags:
                            DESIGN_FLAG_PRESERVE, DESIGN_FLAG_QUERY_FILTER, 
                            DESIGN_FLAG_QUERY_MACRO_FILTER, 
                            DESIGN_FLAG_BACKGROUND_FILTER, 
                            DESIGN_FLAG_MAIL_FILTER, DESIGN_FLAG_NO_COMPOSE, 
                            and DESIGN_FLAG_NO_QUERY.

*************************************************************************/

STATUS  LNPUBLIC  SetMacroFlags( NOTEHANDLE hMacro, char *szFlags )
{
    STATUS      error=NOERROR;

    if (error = NSFItemSetText(hMacro,
                DESIGN_FLAGS,           /* "$Flags" */
                szFlags, MAXWORD))
    {
        PRINTLOG ("Error: unable to set Flags field in macro note.\n");
    }

    return(error);
}

/************************************************************************

    FUNCTION:   SetMacroMachineName

*************************************************************************/

STATUS  LNPUBLIC  SetMacroMachineName( NOTEHANDLE hMacro )
{
    STATUS      error=NOERROR;
    char        szUserName[MAXUSERNAME+1];
      
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
    }

    return(error);
}


/************************************************************************

    FUNCTION:   SetMacroPeriod 

    INPUTS:     
        NOTEHANDLE  hMacro      - handle to open macro note
        WORD        wPeriod     - must be PERIOD_HOURLY, PERIOD_DAILY,
                                    PERIOD_WEEKLY, or PERIOD_DISABLED

*************************************************************************/

STATUS  LNPUBLIC  SetMacroPeriod( NOTEHANDLE hMacro, WORD wPeriod )
{
    STATUS      error=NOERROR;
    char        cPeriod;
     
    cPeriod = (char)('0' + wPeriod);
    if (error = NSFItemSetText(hMacro,
                FILTER_PERIOD_ITEM,     /* "$Period" */
                &cPeriod, 1))
    {
        PRINTLOG ("Error: unable to set Scan field in macro note.\n");
    }

    return(error);
}

/************************************************************************

    FUNCTION:   SetMacroLeftToDo

    INPUTS:     
        DBHANDLE    hDb         - handle to the open database
        NOTEHANDLE  hMacro      - handle to open macro note

    DESCRIPTION:
        The $LeftToDo item is of TYPE_OBJECT. It consists of two components:
        the item (just the OBJECT_DESCRIPTOR) and the object. The object
        consists of an IDTABLE followed by the modification TIMEDATE of 
        the macro note, followed by the DBID of this database. The IDTABLE
        is empty because we are only just creating the macro note now. 

*************************************************************************/

STATUS  LNPUBLIC  SetMacroLeftToDo( DBHANDLE hDb, NOTEHANDLE hMacro )
{
    STATUS            error=NOERROR;
    DHANDLE           hLeftToDo;      /* LeftToDo Object */
    DWORD             dwTableSize;
    DWORD             dwObjectSize;
    DWORD             dwObjectID;
    BLOCKID           bidLeftToDo;    /* LeftToDo Item */     
    DWORD             dwItemSize;
    OBJECT_DESCRIPTOR objLeftToDo;   
    BYTE              *pLTDObject;    /* ptr to LeftToDo Object data */
    WORD              *pLTDItem;      /* ptr to LefToDo Item data */
    DBID              temp_dbid;

    if (error = IDCreateTable(ODSLength(_NOTEID), &hLeftToDo))
    {
        PRINTLOG ("Error: unable to create ID table.\n");
        goto Exit0;
    }

    dwTableSize = IDTableSize(hLeftToDo);
    dwObjectSize = dwTableSize + ODSLength(_TIMEDATE) + ODSLength (_DBID);

    if (error = OSMemRealloc(hLeftToDo, dwObjectSize))
    {
        PRINTLOG ("Error: unable to re-allocate ID table to %ld bytes.\n",
                   dwObjectSize);
        goto Exit1;
    }

    if (error = NSFDbAllocObject(hDb, dwObjectSize, NOTE_CLASS_DOCUMENT, 0, 
                &dwObjectID))
    {
        PRINTLOG ("Error: unable to allocate LeftToDo Object.\n");
        goto Exit1;
    }
    
    /* Set OBJECT_DESCRIPTOR info */
    objLeftToDo.ObjectType = OBJECT_FILTER_LEFTTODO ;
    objLeftToDo.RRV = dwObjectID;

    /* Allocate LeftToDo item block */
    dwItemSize = (DWORD) (ODSLength(_WORD) + ODSLength(_OBJECT_DESCRIPTOR));
    if (error = OSMemAlloc(0, dwItemSize, &(bidLeftToDo.pool)))
    {
        PRINTLOG ("Error: unable to allocate %ld bytes for LeftToDo Item.\n",
                   dwItemSize);
        NSFDbFreeObject(hDb, dwObjectID);
        goto Exit1;
    }
    bidLeftToDo.block = NULLBLOCK;

    /* Assign OBJECT_TYPE and DESCRIPTOR values */
    pLTDItem = OSLockBlock(WORD, bidLeftToDo);
    *pLTDItem = TYPE_OBJECT;
    pLTDItem++;
    ODSWriteMemory (&pLTDItem, _OBJECT_DESCRIPTOR, &objLeftToDo, 1);
    OSUnlockBlock(bidLeftToDo);

  /* Append LeftToDo item block */
    if (error = NSFItemAppendObject(hMacro, ITEM_SUMMARY,
                FILTER_LEFTTODO_ITEM, 
                sizeof(FILTER_LEFTTODO_ITEM)-1,
                bidLeftToDo, 
                dwItemSize, 
                TRUE))      /* Domino and Notes will deallocate memory */
    {
        PRINTLOG ("Error: unable to append %s item.\n", FILTER_LEFTTODO_ITEM);
        OSMemFree(bidLeftToDo.pool);
        goto Exit1;
    }

    /* Update the macro note to get the latest modification timedate */
    NSFNoteUpdate(hMacro, UPDATE_FORCE);

    /* Write the Macro note modification timedate after the IDTABLE. */
    pLTDObject = OSLock(BYTE, hLeftToDo);        /* pLTDObject points to start if IDTABLE */
    pLTDObject += dwTableSize;                   /* pLTDObject points to the TIMEDATE */
    NSFNoteGetInfo(hMacro, _NOTE_MODIFIED, (TIMEDATE*)pLTDObject);

    /* Write the DBID after the modification timedate */
    pLTDObject += ODSLength(_TIMEDATE);         /* pLTDObject points to the DBID */

    /*  Do NOT attempt to access or pass an integral data type at 
        an address that is not a multiple of the data type size, or
        is not a multiple of the size of the data structure's 
        largest simple element.
        Use memcpy to copy the data structure into a local variable */
     
    NSFDbIDGet(hDb, &temp_dbid);
    memcpy (pLTDObject, &temp_dbid, sizeof(DBID));
    OSUnlockObject(hLeftToDo);

    /* Write the hLeftToDo object to the database */
    if (error = NSFDbWriteObject(hDb, dwObjectID, hLeftToDo, 0, 
                dwObjectSize))
    {
        PRINTLOG ("Error: unable to write %s object to database.\n",
                   FILTER_LEFTTODO_ITEM);
    }

Exit1:
    IDDestroyTable(hLeftToDo);

Exit0:
    return(error);
}
