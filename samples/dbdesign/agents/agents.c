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

    PROGRAM: AGENTS

    FILE:    agents.c

    SYNTAX:  Agents <database filename>

    PURPOSE: Shows how to create various Agent notes via the API

    DESCRIPTION:
        This sample creates the following four Agent notes in the 
        specified database.

        1) A manual (menu command), shared, Formula agent action run 
        against the currently selected document.
        
        2) A disabled, private, background LotusScript agent action 
        triggered and run against new and/or documents.

        3) An enabled, shared, scheduled, Formula based agent action
        run against all documents once a month.

        4) A manual (menu command), shared, Java agent action run 
        against all documents in the database.
 
        For the agents to execute successfully, you must use the sample
        database problems.nsf, which is supplied with the toolkit.
        

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* OS header files */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* HCL C API for Notes/Domino Header files.*/
#include <global.h>
#include <stdnames.h>
#include <odstypes.h>       
#include <queryods.h>
#include <editods.h>
#include <nsfdb.h>
#include <nsfdata.h>
#include <nsfnote.h>
#include <nif.h>            
#include <nsfsearc.h>       
#include <nsfobjec.h>       /* NSFDbAllocObject, etc. */
#include <kfm.h>            /* SECKFMGetUserName */
#include <osmem.h>
#include <ostime.h>
#include <agents.h>         /* Agent execution header */ 
#include <osmisc.h>
#include "../../printLog.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/* Function Prototypes */
STATUS  LNPUBLIC  AddManualAgent( DBHANDLE );
STATUS  LNPUBLIC  AddBackgroundAgent( DBHANDLE );
STATUS  LNPUBLIC  AddScheduleAgent( DBHANDLE );
STATUS  LNPUBLIC  AddJavaAgent( DBHANDLE );
STATUS  LNPUBLIC  CreateAgentNote ( DBHANDLE, NOTEHANDLE*, WORD );
STATUS  LNPUBLIC  SetAgentTitle( NOTEHANDLE, char * );
STATUS  LNPUBLIC  SetAgentComment( NOTEHANDLE, char * );
STATUS  LNPUBLIC  SetAgentDesignFlags( NOTEHANDLE, char * );
STATUS  LNPUBLIC  SetAgentMachineName( NOTEHANDLE );
STATUS  LNPUBLIC  SetAgentVersion( NOTEHANDLE );
STATUS  LNPUBLIC  SetAgentType( NOTEHANDLE, WORD );
STATUS  LNPUBLIC  SetAgentLastInfo( NOTEHANDLE );
STATUS  LNPUBLIC  SetAgentAssistFlags( NOTEHANDLE, char * );
STATUS  LNPUBLIC  SetAgentTrigger( NOTEHANDLE, WORD );
STATUS  LNPUBLIC  SetAgentInfo( NOTEHANDLE, ODS_ASSISTSTRUCT );
STATUS  LNPUBLIC  SetAgentAction( NOTEHANDLE, BYTE *, DWORD );
STATUS  LNPUBLIC  SetAgentRunInfo( DBHANDLE, NOTEHANDLE, WORD );

/* Progam constants */
char szAGENT_MANUAL[]     = "Send Reminder to Support Rep";
char szAGENT_BACKGROUND[] = "Assign Support Rep";
char szAGENT_SCHEDULED[]  = "Escalate Priority";
char szAGENT_JAVA[]       = "Assign Hot Problems";
#define SHARED_AGENT        0
#define PRIVATE_AGENT       1


/************************************************************************

    FUNCTION:   main

*************************************************************************/
int main (int argc, char *argv[])
{
    STATUS      error = NOERROR;
    char        *szDbName;
    DBHANDLE    hDb;
    int         goodAgents=0;

    if (error = NotesInitExtended (argc, argv))
    {
        PRINTLOG("\n Unable to initialize Notes.\n");
        return (1);
    }

    /* Process arguments */
    if (argc != 2)
    {
        PRINTLOG ("Usage: Agents <database filename>\n");
        goto Exit0;
    }
    szDbName = argv[1];

    /* Open input database */
    if (error = NSFDbOpen(szDbName, &hDb))
    {
        PRINTLOG ("Error: unable to open target database '%s'\n", szDbName);
        goto Exit0;
    }

    /* Open file pointer to output log */

    /* Create and add manual Agent */
    if (error = AddManualAgent(hDb))
        goto Exit1;
    goodAgents++;


    /* Create and add background Agent */
    if (error = AddBackgroundAgent(hDb))
        goto Exit1;
    goodAgents++;


    /* Create and add scheduled Agent */
    if (error = AddScheduleAgent(hDb))
        goto Exit1;
    goodAgents++;


    /* Create and add java Agent */
    if (error = AddJavaAgent(hDb))
        goto Exit1;
    goodAgents++;


    PRINTLOG("\nSuccessfully added %d Agents to '%s'.\n", goodAgents,szDbName);

    /* Close database, report completion, close log file pointer, and return */
Exit1:
    NSFDbClose(hDb);
    PRINTLOG ("Program execution to create agent notes completed.\n");

Exit0:
    if (error)
    {
       PRINTERROR(error, "NSFDbOpen");
    }

    NotesTerm();
    return(error);
}

/************************************************************************

    FUNCTION:   AddManualAgent(DBHANDLE hDb)

    PURPOSE:    Add a Manual (executed from menu) Agent to the database.

    DESCRIPTION:
        This function adds a manual Agent named "Send reminder to Support Rep"
        to the open database.  
        
        This agent will send a mail message, with doclink, to the support
        reps of all the selected documents in the view.  The Domino and Notes
        agent structures and flags are used.
        
        After creating this Agent, you can execute it by running the Domino 
        and Notes workstation, opening the database, and selecting
        this Agent from the "Actions" menu of the main view.  

*************************************************************************/
STATUS  LNPUBLIC  AddManualAgent( DBHANDLE hDb )
{
    STATUS              error = NOERROR;

    NOTEHANDLE          hAgent;
    char                szAssistFlags[DESIGN_FLAGS_MAX];
    char                szDesignFlags[DESIGN_FLAGS_MAX];
    ODS_ASSISTSTRUCT    AgentInfo;

    FORMULAHANDLE       hFormula;
    WORD                wFormulaLen;
    WORD                wdc;

    CDACTIONHEADER      AgentHeader;
    CDACTIONFORMULA     AgentFormula;

    BYTE                *AgentAction; 
    BYTE                *buff_ptr; 
    DWORD               dwBuffLen;

    char                szComment[] = 
"Send a reminder email to the appropriate support reps for the documents \
selected in the view.";
    char                szFormula[] = 
                                      "@MailSend(SRep; \"\"; \"\"; \"Reminder: \" + CompanyName + \
                                      \" problem still open\"; Subject; \"\"; [IncludeDoclink]);\n\
                                      SELECT @All";

    /* Create Agent note and set note class to NOTE_CLASS_FILTER */
    if (error = CreateAgentNote(hDb, &hAgent, SHARED_AGENT ))
        goto Exit0;

    /* $Title */
    if (error = SetAgentTitle(hAgent, szAGENT_MANUAL))
        goto Exit1;

    /* $Comment */
    if (error = SetAgentComment(hAgent, szComment))
        goto Exit1;

    /* $Flags */
    szDesignFlags[0] = DESIGN_FLAG_V4AGENT;
    szDesignFlags[1] = DESIGN_FLAG_HIDE_FROM_V3;                       
    szDesignFlags[2] = '\0';
    if (error = SetAgentDesignFlags(hAgent, szDesignFlags))
        goto Exit1;

    /* $AssistVersion */
    if (error = SetAgentVersion(hAgent))
        goto Exit1;

    /* $AssistType */
    if (error = SetAgentType(hAgent, SIG_ACTION_FORMULA))           
        goto Exit1;

    /* $AssistLastRun, $AssistDocCount */
    if (error = SetAgentLastInfo(hAgent))
        goto Exit1;

    /* $AssistFlags */
    szAssistFlags[0] = ASSIST_FLAG_ENABLED;                       
    szAssistFlags[1] = '\0';
    if (error = SetAgentAssistFlags(hAgent, szAssistFlags))                 
        goto Exit1;

    /* $AssistTrigger */                                
    if (error = SetAgentTrigger(hAgent, 
                                ASSISTTRIGGER_TYPE_MANUAL))
        goto Exit1;

    /* $AssistInfo */
    (void) memset (&AgentInfo, 0, sizeof(ODS_ASSISTSTRUCT)); 
    AgentInfo.wVersion = (WORD) 0;
    AgentInfo.wTriggerType = (WORD) ASSISTTRIGGER_TYPE_MANUAL;
    AgentInfo.wSearchType = (WORD) ASSISTSEARCH_TYPE_SELECTED;
    AgentInfo.dwFlags = (DWORD)0;

    if (error = SetAgentInfo(hAgent, AgentInfo))
        goto Exit1;

    /* $AssistQuery and $AssistAction */
    /* First, compile formula */
    if (error = NSFFormulaCompile(  NULL, 0, szFormula, 
                                    (WORD) strlen(szFormula),
                                    &hFormula, &wFormulaLen, &wdc, &wdc, 
                                    &wdc, &wdc, &wdc))
    {
        PRINTLOG("Error compiling formula.\n");
        goto Exit1;
    }

    /* Use ODS routines to build CDACTIONHEADER and CDACTIONFORMULA buffer */
    wFormulaLen += wFormulaLen%2;       /* even byte boundary */  
    AgentAction = (BYTE *) malloc( ODSLength (_CDACTIONHEADER) +
                                   ODSLength (_CDACTIONFORMULA) + 
                                   wFormulaLen );
    if (AgentAction == (BYTE *) NULL) 
    {
        PRINTLOG("Error: unable to allocate memory.\n");
        goto Exit1;
    }
    buff_ptr = AgentAction;

    AgentHeader.Header.Signature = SIG_ACTION_HEADER;
    AgentHeader.Header.Length = (BYTE) ODSLength(_CDACTIONHEADER);
    ODSWriteMemory( &buff_ptr, _CDACTIONHEADER, &AgentHeader, 1 );
    
    AgentFormula.Header.Signature = SIG_ACTION_FORMULA;             
    AgentFormula.Header.Length = ODSLength(_CDACTIONFORMULA) + wFormulaLen;
    AgentFormula.dwFlags = (DWORD) 0;
    AgentFormula.wFormulaLen = wFormulaLen;

    ODSWriteMemory( &buff_ptr, _CDACTIONFORMULA, &AgentFormula, 1 );
    memcpy( (char *)buff_ptr, OSLockObject(hFormula), AgentFormula.wFormulaLen );
    buff_ptr += AgentFormula.wFormulaLen;
    dwBuffLen = (DWORD)(buff_ptr - AgentAction);
        
    error = SetAgentAction(hAgent, AgentAction, dwBuffLen);
    OSUnlockObject(hFormula);
    OSMemFree(hFormula);
    free (AgentAction);
    
    if (error)
        goto Exit1;

    /* $AssistRunInfo */
    if (error = SetAgentRunInfo(hDb, hAgent, SHARED_AGENT))
        goto Exit1;

    /* $Signature */
    if (error = NSFNoteSign(hAgent))
        goto Exit1;

    /* Finally update note */
    if (error = NSFNoteUpdate(hAgent, 0))
    {
        PRINTLOG("Error: unable to update Agent note to database.\n");
        goto Exit1;
    }

Exit1:
    NSFNoteClose(hAgent);

Exit0:
    return (error);
}

/************************************************************************

    FUNCTION:   AddBackgroundAgent(DBHANDLE hDb)

    PURPOSE:    Add a background (event triggered) Agent to the database.

    DESCRIPTION:
        This function adds a private, disabled, background Agent named 
        "Assign Support Rep" to the open database.  
        
        This agent is LotusScript based will assign "Isabel Silton" as the 
        Support Rep for those documents that do not have one assigned. The 
        Domino and Notes agent structures and flags are used.

        After creating this Agent, you can force execution by running the
        Domino and Notes workstation, opening the database, opening the Agents 
        view, selecting this agent from the list, and executing the Run command
        from the Actions menu.

        Alternatively, since this background agent is triggered each time a
        document is added or modified to the database, you can enable 
        background execution of the agent by creating or modifying a problem
        report document in the database.  Note that you must first enable 
        the agent from the Agents view.
                                               
*************************************************************************/
STATUS  LNPUBLIC  AddBackgroundAgent( DBHANDLE hDb )
{
    STATUS              error = NOERROR;

    NOTEHANDLE          hAgent;
    char                szAssistFlags[DESIGN_FLAGS_MAX];
    char                szDesignFlags[DESIGN_FLAGS_MAX];
    ODS_ASSISTSTRUCT    AgentInfo;

    CDACTIONHEADER      AgentHeader;
    CDACTIONLOTUSSCRIPT AgentScript;

    BYTE                *AgentAction; 
    BYTE                *buff_ptr; 
    DWORD               dwBuffLen;

    char                *pFormattedLS;
    int                 FormattedLSLen;

    int                 sourceAllocated=0;
    int                 destAllocated=0;
    int                 errorBufferAllocated=0;

    DHANDLE             hSource=NULL;
    DHANDLE             hDest=NULL;
    DHANDLE             hErrorBuffer=NULL;
    DHANDLE             hUnused=NULL;

    char                szComment[] = "Assign all unassigned problems to Isabel Silton.";

    char                szScript[] = 
                                     "Sub Initialize\n\
                                      Dim updcount As Integer\n\
                                      Dim repname As String\n\
                                      Dim session As New NotesSession\n\
                                      Dim db As NotesDatabase\n\
                                      Dim dc As NotesDocumentCollection\n\
                                      Dim doc As NotesDocument\n\
                                      Dim parm As NotesDocument\n\
                                      Set db = session.CurrentDatabase\n\
                                      Set dc = db.AllDocuments\n\
                                      updcount = 0\n\
                                      Set parm = session.DocumentContext\n\
                                      'if running from UI, then hardcode new rep name\n\
                                      If parm IS NOTHING then\n\
                                      repname = \"Isabel Silton\"\n\
                                      'else, use passed parameter document value\n\
                                      Else\n\
                                      repname = parm.NewRep(0)\n\
                                      End If\n\
                                      'assign passed name to all unassigned, open problems\n\
                                      For j = 1 To dc.Count\n\
                                      Set doc = dc.GetNthDocument(j)\n\
                                      If doc.SRep(0) = \"\" AND doc.Status(0) = \"Open\" Then\n\
                                      doc.SRep = repname\n\
                                      updcount = updcount+1\n\
                                      End If\n\
                                      Call doc.Save(True,False)\n\
                                      Next\n\
                                      Print \"Support Rep parameter = \"+repname\n\
                                      Print \"Updated documents = \"+STR$(updcount)\n\
                                      End Sub\n";

    /* Create Agent note and set note class to 
     * NOTE_CLASS_PRIVATE | NOTE_CLASS_FILTER 
     */
    if (error = CreateAgentNote(hDb, &hAgent, PRIVATE_AGENT))
        goto Exit0;

    /* $Title */
    if (error = SetAgentTitle(hAgent, szAGENT_BACKGROUND))
        goto Exit1;

    /* $Comment */
    if (error = SetAgentComment(hAgent, szComment))
        goto Exit1;

    /* $Flags */
    szDesignFlags[0] = DESIGN_FLAG_V4BACKGROUND_MACRO;
    szDesignFlags[1] = DESIGN_FLAG_V4AGENT;
    szDesignFlags[2] = DESIGN_FLAG_PRIVATE_IN_DB;
    szDesignFlags[3] = DESIGN_FLAG_HIDE_FROM_V3;
    szDesignFlags[4] = DESIGN_FLAG_LOTUSSCRIPT_AGENT;                       
    szDesignFlags[5] = '\0';


    if (error = SetAgentDesignFlags(hAgent, szDesignFlags))
        goto Exit1;

    /* $AssistVersion */
    if (error = SetAgentVersion(hAgent))
        goto Exit1;

    /* $AssistType */
    if (error = SetAgentType(hAgent, SIG_ACTION_LOTUSSCRIPT))           
        goto Exit1;

    /* $AssistLastRun, $AssistDocCount */
    if (error = SetAgentLastInfo(hAgent))
        goto Exit1;

    /* $AssistFlags */
    szAssistFlags[0] = ASSIST_FLAG_PRIVATE;                       
    szAssistFlags[1] = '\0';
    if (error = SetAgentAssistFlags(hAgent, szAssistFlags))                 
        goto Exit1;

    /* $AssistTrigger */                                
    if (error = SetAgentTrigger(hAgent, 
                                ASSISTTRIGGER_TYPE_DOCUPDATE)) 
        goto Exit1;

    /* $AssistInfo */
    (void) memset (&AgentInfo, 0, sizeof(ODS_ASSISTSTRUCT)); 
    AgentInfo.wVersion = (WORD) 0;
    AgentInfo.wTriggerType = (WORD) ASSISTTRIGGER_TYPE_DOCUPDATE;
    AgentInfo.wSearchType = (WORD) 0; /* explicit search criteria not necessary */

    if (error = SetAgentInfo(hAgent, AgentInfo))
        goto Exit1;



    /* Following steps will convert raw Lotus Script to a format
       that IDE uses when rendering the script.  */

    /*** Be sure to allocate enough memory for the script and the 
       null terminator.  */
    if (error = OSMemAlloc(0,strlen(szScript)+1,&hSource))
    {
        PRINTLOG("Error: unable to alloc hSource.\n");
        goto Exit1;
    }
    sourceAllocated=1;

    /*** Copy the raw Lotus Script into the newly allocated memory 
    space. */
    pFormattedLS=OSLock(char,hSource);
    strcpy(pFormattedLS,szScript);
    OSUnlock(hSource);

    /*** Convert the raw Lotus Script to IDE compliant format.  */
    error = AgentLSTextFormat(hSource,&hDest, &hErrorBuffer,0,&hUnused);

    /*** Free hSource handle, and update memory allocation flags for
    hSource, hDest and hErrorBuffer handles. */
    OSMemFree(hSource);
    sourceAllocated=0;
    destAllocated=1;
    errorBufferAllocated=1;     

    if (error )
    { 
        PRINTLOG("Error: Error in AgentLSTextFormat.\n");
        goto Exit1;
    }

    /*** If any script error, retrieve the error text from hErrorBuffer 
    handle; otherwise, retrieve the IDE compliant script.  */
    if (hErrorBuffer)
    {
        pFormattedLS=OSLock(char,hErrorBuffer);
        PRINTLOG("\nError from AgentLSTextFormat: %s\n",pFormattedLS);
        OSUnlock(hErrorBuffer);
        error=1;
        goto Exit1;
    }
    else if (hDest)
    {
        OSMemFree(hErrorBuffer);
        errorBufferAllocated=0;
        pFormattedLS=OSLock(char,hDest);

       /*** When saving the formatted LS in the "$AssistAction" item, 
       the script should be ended with a null terminator, therefore, 
       we'll add one to the strlen(pFormattedLS) and save it in the 
       FormattedLSLen variable.  */

       FormattedLSLen=strlen(pFormattedLS)+1;
    }
   

    /* $AssistQuery and $AssistAction */
    /* Use ODS routines to build CDACTIONHEADER and CDACTIONFORMULA buffer */
    AgentAction = (BYTE *) malloc( ODSLength (_CDACTIONHEADER) +
                                   ODSLength (_CDACTIONLOTUSSCRIPT) + 
                                   FormattedLSLen);
    if (AgentAction == (BYTE *) NULL) 
    {
        PRINTLOG("Error: unable to allocate memory.\n");
        goto Exit1;
    }
    buff_ptr = AgentAction;

    AgentHeader.Header.Signature = SIG_ACTION_HEADER;
    AgentHeader.Header.Length = (BYTE) ODSLength(_CDACTIONHEADER);
    ODSWriteMemory( &buff_ptr, _CDACTIONHEADER, &AgentHeader, 1 );
    
    AgentScript.Header.Signature = SIG_ACTION_LOTUSSCRIPT;             
    AgentScript.Header.Length = ODSLength(_CDACTIONLOTUSSCRIPT) +
                                        FormattedLSLen; 
    AgentScript.dwFlags = (DWORD) 0;
    AgentScript.dwScriptLen = (DWORD) FormattedLSLen;

    ODSWriteMemory( &buff_ptr, _CDACTIONLOTUSSCRIPT, &AgentScript, 1 );
    memcpy( (char *)buff_ptr, pFormattedLS, (WORD)AgentScript.dwScriptLen);
    buff_ptr += (WORD) AgentScript.dwScriptLen;
    dwBuffLen = (DWORD)(buff_ptr - AgentAction);

    OSUnlock(hDest);
    OSMemFree(hDest);
    destAllocated=0;

    error = SetAgentAction(hAgent, AgentAction, dwBuffLen);
    free (AgentAction);
    
    if (error)
        goto Exit1;


    /* $AssistRunInfo */
    if (error = SetAgentRunInfo(hDb, hAgent, PRIVATE_AGENT))
        goto Exit1;
    

    /* $Signature */
    if (error = NSFNoteSign(hAgent))
        goto Exit1;


    /* Update note */
    if (error = NSFNoteUpdate(hAgent, 0))
    {
        PRINTLOG("Error: unable to update background Agent note to database.\n");
        goto Exit1;
    }
    /* Compile the LS script to object code.  */
    if (error = NSFNoteLSCompile(hDb, hAgent, 0))
    {
        PRINTLOG("Error: unable to compile Agent note to database.\n");
        goto Exit1;
    }

    /* Finally update note */
    if (error = NSFNoteUpdate(hAgent, 0))
    {
        PRINTLOG("Error: unable to do final update on the background Agent note to database.\n");
        goto Exit1;
    }
Exit1:
    if (sourceAllocated)
        OSMemFree(hSource);

    if (destAllocated)
        OSMemFree(hDest);

    if (errorBufferAllocated)
       OSMemFree(hErrorBuffer);

    NSFNoteClose(hAgent);

Exit0:
    return (error);
}
          
/************************************************************************

    FUNCTION:   AddScheduleAgent(DBHANDLE hDb)

    PURPOSE:    Add a scheduled Agent to the database.

    DESCRIPTION:
        This function adds a scheduled Agent named "Escalate Priority"
        to the open database.  
        
        On the first day of each month at 1AM, this agent will increase the 
        Priority of all open problems one level.  If the priority is already
        High, then it is unchanged.  The Domino and Notes agent structures and
        flags are used.

        After creating this Agent, you can force execution by running the
        Domino and Notes workstation, opening the database, opening the Agents 
        view, selecting this agent from the list, and executing the Run command
        from the Actions menu.

        Note that if you wish to allow the agent to run "On schedule", you 
        must enable scheduled local agents on the Domino and Notes workstation. 
                
*************************************************************************/
STATUS  LNPUBLIC  AddScheduleAgent( DBHANDLE hDb )
{
    STATUS              error = NOERROR;

    NOTEHANDLE          hAgent;
    char                szAssistFlags[DESIGN_FLAGS_MAX];
    char                szDesignFlags[DESIGN_FLAGS_MAX];
    ODS_ASSISTSTRUCT    AgentInfo;

    FORMULAHANDLE       hFormula;
    WORD                wFormulaLen;
    WORD                wdc;

    CDACTIONHEADER      AgentHeader;
    CDACTIONFORMULA     AgentFormula;

    BYTE                *AgentAction; 
    BYTE                *buff_ptr; 
    DWORD               dwBuffLen;

    char                szComment[] = "Once per month, search for all open \
                                       problems that are older than one month and escalate the priority \
                                       one level.";
    char                szFormula[] = "OneMonthAgo := @Adjust(@Now; 0; -1; 0; 0; 0; 0);\n\
                                       SELECT((DateOpened<OneMonthAgo) & (Status=\"Open\"));\n\
                                       FIELD Priority := @If(Priority=\"Low\";\"Medium\";\
                                       Priority=\"Medium\";\"High\";\"High\");";

    /* Create Agent note and set note class to NOTE_CLASS_FILTER */
    if (error = CreateAgentNote(hDb, &hAgent, SHARED_AGENT))
        goto Exit0;

    /* $Title */
    if (error = SetAgentTitle(hAgent, szAGENT_SCHEDULED))
        goto Exit1;

    /* $Comment */
    if (error = SetAgentComment(hAgent, szComment))
        goto Exit1;

    /* $Flags */
    szDesignFlags[0] = DESIGN_FLAG_V4AGENT;
    szDesignFlags[1] = DESIGN_FLAG_HIDE_FROM_V3;                       
    szDesignFlags[2] = DESIGN_FLAG_V4BACKGROUND_MACRO;                       
    szDesignFlags[3] = '\0';
    if (error = SetAgentDesignFlags(hAgent, szDesignFlags))
        goto Exit1;

    /* $MachineName */                          
    if (error = SetAgentMachineName(hAgent))                
        goto Exit1;

    /* $AssistVersion */
    if (error = SetAgentVersion(hAgent))
        goto Exit1;

    /* $AssistType */
    if (error = SetAgentType(hAgent, SIG_ACTION_FORMULA))           
        goto Exit1;

    /* $AssistLastRun, $AssistDocCount */
    if (error = SetAgentLastInfo(hAgent))
        goto Exit1;

    /* $AssistFlags */
    szAssistFlags[0] = ASSIST_FLAG_ENABLED;                       
    szAssistFlags[1] = '\0';
    if (error = SetAgentAssistFlags(hAgent, szAssistFlags))                 
        goto Exit1;

    /* $AssistTrigger */                                
    if (error = SetAgentTrigger(hAgent, 
                                ASSISTTRIGGER_TYPE_SCHEDULED))
        goto Exit1;

    /* $AssistInfo */
    (void) memset (&AgentInfo, 0, sizeof(ODS_ASSISTSTRUCT)); 
    AgentInfo.wVersion = (WORD) 0;
    AgentInfo.wTriggerType = (WORD) ASSISTTRIGGER_TYPE_SCHEDULED;
    AgentInfo.wSearchType = (WORD) ASSISTSEARCH_TYPE_ALL;
    AgentInfo.wIntervalType = (WORD) ASSISTINTERVAL_TYPE_MONTH;
    AgentInfo.wInterval = (WORD) 1;               /* every month */
    AgentInfo.dwTime1 = (DWORD) 1*TICKS_IN_HOUR;  /* start at 1AM */
    AgentInfo.dwTime2 = (DWORD) 1;                /* on first day */ 

    if (error = SetAgentInfo(hAgent, AgentInfo))
        goto Exit1;

    /* $AssistQuery and $AssistAction */
    /* First, compile formula */
    if (error = NSFFormulaCompile(  NULL, 0, szFormula, 
                                    (WORD) strlen(szFormula),
                                    &hFormula, &wFormulaLen, &wdc, &wdc, 
                                    &wdc, &wdc, &wdc))
    {
        PRINTLOG("Error compiling formula.\n");
        goto Exit1;
    }

    /* Use ODS routines to build CDACTIONHEADER and CDACTIONFORMULA buffer */
    wFormulaLen += wFormulaLen%2;       /* even byte boundary */  
    AgentAction = (BYTE *) malloc( ODSLength (_CDACTIONHEADER) +
                                   ODSLength (_CDACTIONFORMULA) + 
                                   wFormulaLen );
    if (AgentAction == (BYTE *) NULL) 
    {
        PRINTLOG("Error: unable to allocate memory.\n");
        goto Exit1;
    }
    buff_ptr = AgentAction;

    AgentHeader.Header.Signature = SIG_ACTION_HEADER;
    AgentHeader.Header.Length = (BYTE) ODSLength(_CDACTIONHEADER);
    ODSWriteMemory( &buff_ptr, _CDACTIONHEADER, &AgentHeader, 1 );
    
    AgentFormula.Header.Signature = SIG_ACTION_FORMULA;             
    AgentFormula.Header.Length = ODSLength(_CDACTIONFORMULA) + wFormulaLen;
    AgentFormula.dwFlags = (DWORD) 0;
    AgentFormula.wFormulaLen = wFormulaLen;

    ODSWriteMemory( &buff_ptr, _CDACTIONFORMULA, &AgentFormula, 1 );
    memcpy( (char *)buff_ptr, OSLockObject(hFormula), AgentFormula.wFormulaLen );
    buff_ptr += AgentFormula.wFormulaLen;
    dwBuffLen = (DWORD)(buff_ptr - AgentAction);
        
    error = SetAgentAction(hAgent, AgentAction, dwBuffLen);
    OSUnlockObject(hFormula);
    OSMemFree(hFormula);
    free (AgentAction);
    
    if (error)
        goto Exit1;

    /* $AssistRunInfo */
    if (error = SetAgentRunInfo(hDb, hAgent, SHARED_AGENT))
        goto Exit1;

    /* $Signature */
    if (error = NSFNoteSign(hAgent))
        goto Exit1;

    /* Finally update note */
    if (error = NSFNoteUpdate(hAgent, 0))
    {
        PRINTLOG("Error: unable to update Agent note to database.\n");
        goto Exit1;
    }

Exit1:
    NSFNoteClose(hAgent);

Exit0:
    return (error);
}

/************************************************************************

    FUNCTION:   AddJavaAgent(DBHANDLE hDb)

    PURPOSE:    Add a Java Agent (executed from Menu) to the database.

    DESCRIPTION:
        This function adds a manual Agent named "Assign Hot Problems"
        to the open database.  
        
        This agent is Java based and will assign "Fire Fighters" as the 
        Support Rep for those documents that have an "Open" Status and
        "High" Priority.

        After creating this Agent, you can execute it by running the Domino 
        and Notes workstation, opening the database, and selecting
        this Agent from the "Actions" menu of the main view.  
                                               
*************************************************************************/
STATUS  LNPUBLIC  AddJavaAgent( DBHANDLE hDb )
{
    STATUS              error = NOERROR;

    NOTEHANDLE          hAgent;
    char                szAssistFlags[DESIGN_FLAGS_MAX];
    char                szDesignFlags[DESIGN_FLAGS_MAX];
    ODS_ASSISTSTRUCT    AgentInfo;

    CDACTIONHEADER      AgentHeader;
    CDACTIONJAVAAGENT   AgentJava;

    BYTE                *AgentAction; 
    BYTE                *buff_ptr; 
    DWORD               dwBuffLen;
    
    char                szComment[] = "Assign all hot problems (high priority) to Fire Fighters.";
    char                szClass[] = "HotAgent.class";
    char                szCode[] = "/opt/lotus/notesapi/samples/sol_2x/dbdesign/agents";
    char                szFileList[] = "HotAgent.class\0\0";
    static char         szSourceFile[128]; /* fix this */

    /* Create Agent note and set note class to NOTE_CLASS_FILTER */
    if (error = CreateAgentNote(hDb, &hAgent, SHARED_AGENT))
        goto Exit0;

    /* $Title */
    if (error = SetAgentTitle(hAgent, szAGENT_JAVA))
        goto Exit1;

    /* $Comment */
    if (error = SetAgentComment(hAgent, szComment))
        goto Exit1;

    /* $Flags */
    szDesignFlags[0] = DESIGN_FLAG_V4AGENT;
    szDesignFlags[1] = DESIGN_FLAG_HIDE_FROM_V3;                       
    szDesignFlags[2] = DESIGN_FLAG_JAVA_AGENT;                       
    szDesignFlags[3] = '\0';

    if (error = SetAgentDesignFlags(hAgent, szDesignFlags))
        goto Exit1;

    /* $AssistVersion */
    if (error = SetAgentVersion(hAgent))
        goto Exit1;

    /* $AssistType */
    if (error = SetAgentType(hAgent, SIG_ACTION_JAVAAGENT))           
        goto Exit1;

    /* $AssistLastRun, $AssistDocCount */
    if (error = SetAgentLastInfo(hAgent))
        goto Exit1;

    /* $AssistFlags */
    szAssistFlags[0] = ASSIST_FLAG_ENABLED;                       
    szAssistFlags[1] = '\0';
    if (error = SetAgentAssistFlags(hAgent, szAssistFlags))                 
        goto Exit1;

    /* $AssistTrigger */                                
    if (error = SetAgentTrigger(hAgent, ASSISTTRIGGER_TYPE_MANUAL))
        goto Exit1;

    /* $AssistInfo */
    (void) memset (&AgentInfo, 0, sizeof(ODS_ASSISTSTRUCT)); 
    AgentInfo.wVersion = (WORD) 0;
    AgentInfo.wTriggerType = (WORD) ASSISTTRIGGER_TYPE_MANUAL;
    AgentInfo.wSearchType = (WORD) ASSISTSEARCH_TYPE_ALL;
    if (error = SetAgentInfo(hAgent, AgentInfo))
        goto Exit1;

    /* $AssistQuery and $AssistAction */
    /* Use ODS routines to build CDACTIONHEADER and CDACTIONJAVA buffer */
    AgentAction = (BYTE *) malloc( ODSLength (_CDACTIONHEADER) +
                                   ODSLength (_CDACTIONJAVAAGENT) + 
                                   strlen (szClass) +
                                   strlen (szCode) +
                                   strlen (szFileList) + 2);

    if (AgentAction == (BYTE *) NULL) 
    {
        PRINTLOG("Error: unable to allocate memory.\n");
        goto Exit1;
    }
    buff_ptr = AgentAction;

    AgentHeader.Header.Signature = SIG_ACTION_HEADER;
    AgentHeader.Header.Length = (BYTE) ODSLength(_CDACTIONHEADER);
    ODSWriteMemory( &buff_ptr, _CDACTIONHEADER, &AgentHeader, 1 );
    
    memset (&AgentJava, 0, sizeof(CDACTIONJAVAAGENT));

    AgentJava.Header.Signature = SIG_ACTION_JAVAAGENT;             
    AgentJava.Header.Length = ODSLength(_CDACTIONJAVAAGENT) + 
                              strlen(szClass) + strlen(szCode) + strlen(szFileList) + 2;
    AgentJava.wClassNameLen = strlen(szClass);
    AgentJava.wCodePathLen = strlen(szCode);
    AgentJava.wFileListBytes = strlen(szFileList) + 2;
    AgentJava.wLibraryListBytes = 0;

    ODSWriteMemory( &buff_ptr, _CDACTIONJAVAAGENT, &AgentJava, 1 );
    memcpy( (char *)buff_ptr, szClass, (WORD)AgentJava.wClassNameLen );
    buff_ptr += (WORD) AgentJava.wClassNameLen;
    
    memcpy( (char *)buff_ptr, szCode, (WORD)AgentJava.wCodePathLen );
    buff_ptr += (WORD) AgentJava.wCodePathLen;
    
    memcpy( (char *)buff_ptr, szFileList, (WORD)AgentJava.wFileListBytes);
    buff_ptr += (WORD)AgentJava.wFileListBytes;
    
    dwBuffLen = (DWORD)(buff_ptr - AgentAction);
        
    error = SetAgentAction(hAgent, AgentAction, dwBuffLen);
    free (AgentAction);
    
    if (error)
        goto Exit1;

    /* $AssistRunInfo */
    if (error = SetAgentRunInfo(hDb, hAgent, SHARED_AGENT))
        goto Exit1;
    

    /* $FILE info */
        strcpy(szSourceFile,szCode);

#ifndef UNIX
    strcat(szSourceFile,"\\");
#else
    strcat(szSourceFile,"/");
#endif

    strcat(szSourceFile,szClass);

    if (error = NSFNoteAttachFile( hAgent,
                                   ITEM_NAME_ATTACHMENT,
                                   (WORD) strlen(ITEM_NAME_ATTACHMENT),
                                   szSourceFile,
                                   szClass,
                                   HOST_LOCAL | COMPRESS_NONE))
        goto Exit1;
 
    /* $Signature */
    if (error = NSFNoteSign(hAgent))
        goto Exit1;

    /* Finally update note */
    if (error = NSFNoteUpdate(hAgent, 0))
    {
        PRINTLOG("Error: unable to update Agent note to database.\n");
        goto Exit1;
    }
        
Exit1:
    NSFNoteClose(hAgent);

Exit0:
    return (error);
}
        
/************************************************************************

    FUNCTION:   CreateAgentNote

    PURPOSE:    Create note in database, and sets the note class to 
                NOTE_CLASS_FILTER if a shared agent or NOTE_CLASS_FILTER |
                NOTE_CLASS_PRIVATE if a private agent.

    INPUTS:     DBHANDLE    hDb         - database handle
                WORD        wAgentType  - SHARED_AGENT or PRIVATE_AGENT

    OUTPUTS:    NOTEHANDLE *phAgent     - handle to new Agent note 

*************************************************************************/
STATUS  LNPUBLIC  CreateAgentNote ( DBHANDLE    hDb, 
                                    NOTEHANDLE *phAgent,
                                    WORD        wAgentType )
{
    STATUS      error=NOERROR;
    WORD        wNoteClass;

    if (error = NSFNoteCreate(hDb, phAgent))
    {
        PRINTLOG("Error: unable to create Agent note.\n");
        return(error);
    }

    if (wAgentType == PRIVATE_AGENT)
        wNoteClass = (NOTE_CLASS_FILTER | NOTE_CLASS_PRIVATE);
    else
        wNoteClass = NOTE_CLASS_FILTER;
    NSFNoteSetInfo(*phAgent, _NOTE_CLASS, &wNoteClass);

    return(error);
} 

/************************************************************************

    FUNCTION:   SetAgentTitle

    PURPOSE:    Append the initial "$TITLE" field for the agent note
                being created.  

    INPUTS:     NOTEHANDLE  hAgent      - handle to open agent note

*************************************************************************/
STATUS  LNPUBLIC  SetAgentTitle(NOTEHANDLE hAgent, char *szTitle)
{
    STATUS      error=NOERROR;

    if (error = NSFItemAppend(
                              hAgent,                       /* handle to note to append to */
                              ITEM_SIGN | ITEM_SUMMARY,     /* item flags */
                              FIELD_TITLE,                  /* item name: "$TITLE" */
                              (WORD) strlen(FIELD_TITLE),
                              TYPE_TEXT,                    /* item type */
                              szTitle,                      /* item value */
                              (DWORD) strlen(szTitle)))     /* value length */
    {
        PRINTLOG("Error: unable to set Title field of Agent note.\n");
    }

    return(error);
}

/************************************************************************

    FUNCTION:   SetAgentComment

    PURPOSE:    Append the "$Comment" field for the agent note being created.
                The value is a short description of the agent.

    INPUTS:     NOTEHANDLE  hAgent      - handle to open agent note
                char        *szComment  - agent description comment

*************************************************************************/
STATUS  LNPUBLIC  SetAgentComment(NOTEHANDLE hAgent, char *szComment)
{
    STATUS      error=NOERROR;

    if (error = NSFItemAppend(
                              hAgent,                             /* handle to note to append to */
                              ITEM_SIGN | ITEM_SUMMARY,           /* item flags */
                              FILTER_COMMENT_ITEM,                /* item name: "$Comment" */
                              (WORD) strlen(FILTER_COMMENT_ITEM),
                              TYPE_TEXT,                          /* item type */
                              szComment,                          /* item value */
                              (DWORD) strlen(szComment)))         /* value length */
    {
        PRINTLOG("Error: unable to set Comment field of Agent note.\n");
    }

    return(error);
}

/************************************************************************

    FUNCTION:   SetAgentDesignFlags

    PURPOSE:    Append the "$Flags" field for the Agent note.  The following
                design flag constants are valid for agent notes.

                    DESIGN_FLAG_V4AGENT              
                    DESIGN_FLAG_HIDE_FROM_V3     
                    DESIGN_FLAG_LOTUSSCRIPT_AGENT  
                    DESIGN_FLAG_PRIVATE_IN_DB      
                        DESIGN_FLAG_V4BACKGROUND_MACRO 
                    DESIGN_FLAG_V4PASTE_AGENT
                        DESIGN_FLAG_AGENT_SHOWINSEARCH

    INPUTS:     NOTEHANDLE  hAgent    - handle to the open Agent note
                char        *szFlags  - zero-terminated string containing the
                                        appropriate design flag combinations.

*************************************************************************/
STATUS  LNPUBLIC  SetAgentDesignFlags( NOTEHANDLE hAgent, char *szFlags )
{
    STATUS      error=NOERROR;

    if (error = NSFItemAppend(
                              hAgent,                      /* handle to note to append to */
                              ITEM_SIGN | ITEM_SUMMARY,    /* item flags */
                              DESIGN_FLAGS,                /* "$Flags" */
                              (WORD) strlen(DESIGN_FLAGS),
                              TYPE_TEXT,                   /* item type */
                              szFlags,                     /* item value */
                              (DWORD) strlen(szFlags)))    /* value length */
    {
        PRINTLOG("Error: unable to set Design Flags field in Agent note.\n");
    }
    return (error);
}

/************************************************************************

    FUNCTION:   SetAgentMachineName

    PURPOSE:    Append the "$MachineName" field for the Scheduled Agent note
                and set to the local machine.  For local, scheduled agents, 
                this value is the user name in canonical format.   
                
                Note that this item is used by scheduled and mail triggered 
                agents only.  This routine needs to be modified if supporting
                server names (e.g. mail triggered).

    INPUTS:     NOTEHANDLE  hAgent    - handle to the open Agent note

*************************************************************************/
STATUS  LNPUBLIC  SetAgentMachineName( NOTEHANDLE hAgent )
{
    STATUS      error=NOERROR;
    char        szUserName[MAXUSERNAME+1];
      
    /* get local machine name */
    if (error = SECKFMGetUserName(szUserName))
    {
        PRINTLOG("Error: unable to get user name from ID file.\n");
        return(error);
    }
    if (error = NSFItemAppend(
                              hAgent,                             /* handle to note to append to */
                              ITEM_SIGN | ITEM_SUMMARY,           /* item flags */
                              FILTER_MACHINE_ITEM,                /* "$MachineName" */
                              (WORD) strlen(FILTER_MACHINE_ITEM),
                              TYPE_TEXT,                          /* item type */
                              szUserName,                         /* item value */
                              (DWORD) strlen(szUserName)))        /* value length */
    {
        PRINTLOG("Error: unable to set Machine Name field in Agent note.\n");
    }

    return(error);
}

/************************************************************************

    FUNCTION:   SetAgentVersion

    PURPOSE:    Append the initial "$AssistVersion" field for the agent note
                being created.  The value is the current time date.

    INPUTS:     NOTEHANDLE  hAgent      - handle to open agent note

*************************************************************************/
STATUS  LNPUBLIC  SetAgentVersion(NOTEHANDLE hAgent)
{
    STATUS      error=NOERROR;
    TIMEDATE    CurrTimeDate;

    OSCurrentTIMEDATE(&CurrTimeDate);
    if (error = NSFItemSetTime(hAgent, 
                      ASSIST_VERSION_ITEM,        /* "$AssistVersion" */
                      &CurrTimeDate))
    {
        PRINTLOG("Error: unable to set Version field of Agent note.\n");
    }
    return(error);
}

/************************************************************************

    FUNCTION:   SetAgentType

    PURPOSE:    Append the "$AssistType" field for the agent note being 
                created.  This value can be one of the following signature
                constants, based on the the desired agent action.
    
                    ASSIST_SIG_ACTION_NONE
                    SIG_ACTION_FORMULA  
                    SIG_ACTION_LOTUSSCRIPT

    INPUTS:     NOTEHANDLE  hAgent      - handle to the open Agent note
                WORD        wActionType - one of the above actions

*************************************************************************/
STATUS  LNPUBLIC  SetAgentType (NOTEHANDLE hAgent, WORD wActionType)
{
    STATUS      error=NOERROR;
    NUMBER      TempNumber;

    TempNumber = (NUMBER) wActionType;
    if (error = NSFItemAppend(
                              hAgent,                        /* handle to note to append to */
                              ITEM_SIGN | ITEM_SUMMARY,      /* item flags */
                              ASSIST_TYPE_ITEM,              /* item name: "$AssistType" */
                              (WORD) strlen(ASSIST_TYPE_ITEM),
                              TYPE_NUMBER,                   /* item type */
                              &TempNumber,                   /* item value */
                              (DWORD) sizeof(TempNumber)))   /* value length */
    {
        PRINTLOG("Error: unable to set AssistType field in Agent note.\n");
    }
    return(error);
}

/************************************************************************

    FUNCTION:   SetAgentLastInfo

    PURPOSE:    Append the initial "$AssistLastRun" and $AssistDocCount" 
                fields for the agent note being created.  The values get 
                updated by Domino and Notes after the agent has been executed 
                to the time date of the run and number of documents modified, 
                respectively.

    INPUTS:     NOTEHANDLE  hAgent      - handle to open agent note

*************************************************************************/
STATUS  LNPUBLIC  SetAgentLastInfo(NOTEHANDLE hAgent)
{
    STATUS      error=NOERROR;
    TIMEDATE    EmptyTimeDate;
    NUMBER      DocCount = 0;

    /* set empty Last Run info */
    EmptyTimeDate.Innards[0] = (DWORD) 0;
    EmptyTimeDate.Innards[1] = (DWORD) 0;
    if (error = NSFItemAppend(
                              hAgent,                             /* handle to note to append to */
                              ITEM_SIGN | ITEM_SUMMARY,           /* item flags */
                              ASSIST_LASTRUN_ITEM,                /* $AssistLastRun */
                              (WORD) strlen(ASSIST_LASTRUN_ITEM),
                              TYPE_TIME,                          /* item type */
                              &EmptyTimeDate,                     /* item value */
                              (DWORD) sizeof(EmptyTimeDate)))     /* value length */
    {
        PRINTLOG("Error: unable to set LastRun field of Agent note.\n");
        return(error);
    }

    /* set empty DocCountRun info */
    if (error = NSFItemAppend(
                              hAgent,                             /* handle to note to append to */
                              ITEM_SIGN | ITEM_SUMMARY,           /* item flags */
                              ASSIST_DOCCOUNT_ITEM,               /* "$AssistDocCount" */
                              (WORD) strlen(ASSIST_DOCCOUNT_ITEM),
                              TYPE_NUMBER,                        /* item type */
                              &DocCount,                          /* item value */
                              (DWORD) sizeof(DocCount)))          /* value length */
    {
        PRINTLOG("Error: unable to set DocCount field of Agent note.\n");
    }
    return(error);
}

/************************************************************************

    FUNCTION:   SetAgentAssistFlags

    PURPOSE:    Append the "$AssistFlags" field for the Agent note being 
                created.  The value can a combination of none to any of the 
                following flag constants.

                        ASSIST_FLAG_ENABLED
                        ASSIST_FLAG_HIDDEN      
                        ASSIST_FLAG_PRIVATE

    INPUTS:     NOTEHANDLE  hAgent      - handle to open agent note
                char        *szFlags    - zero-terminated string containing the
                                          appropriate combination of the above
                                          flags.

*************************************************************************/
STATUS  LNPUBLIC  SetAgentAssistFlags( NOTEHANDLE hAgent, char *szFlags )
{
    STATUS      error=NOERROR;

    if (error = NSFItemAppend(
                              hAgent,                             /* handle to note to append to */
                              ITEM_SIGN | ITEM_SUMMARY,           /* item flags */
                              ASSIST_FLAGS_ITEM,                  /* "$AssistFlags" */
                              (WORD) strlen(ASSIST_FLAGS_ITEM),
                              TYPE_TEXT,                          /* item type */
                              szFlags,                            /* item value */
                              (DWORD) strlen(szFlags)))           /* value length */
    {
        PRINTLOG("Error: unable to set AssistFlags field in Agent note.\n"); 
    }
    return (error);
}

/************************************************************************

    FUNCTION:   SetAgentTrigger

    PURPOSE:    Append the $AssistTrigger field for the Agent note being 
                created.  This value can be set to one of the following 
                constants based on the agent's event trigger.

                        ASSISTTRIGGER_TYPE_MANUAL       
                        ASSISTTRIGGER_TYPE_PASTED       
                        ASSISTTRIGGER_TYPE_DOCUPDATE
                        ASSISTTRIGGER_TYPE_NEWMAIL      
                        ASSISTTRIGGER_TYPE_SCHEDULED
                        ASSISTTRIGGER_TYPE_NONE         

    INPUTS:     NOTEHANDLE  hAgent      - handle to open agent note
                WORD        wTrigger    - trigger  type: must be one of the 
                                          above constants
                                  
*************************************************************************/
STATUS  LNPUBLIC  SetAgentTrigger( NOTEHANDLE hAgent, WORD wTrigger )
{
    STATUS      error=NOERROR;
    char        cType;

    cType = (char)('0' + wTrigger);
    if (error = NSFItemSetText(
                               hAgent,
                               ASSIST_TRIGGER_ITEM,   /* "$AssistTrigger" */
                               &cType, 
                               (WORD)1))
    {
        PRINTLOG("Error: unable to set AssistTrigger field in Agent note.\n");
    }
    return(error);    
}

/************************************************************************

    FUNCTION:   SetAgentInfo

    PURPOSE:    Append the $AssistInfo field for the Agent note being 
                created.  This value must be set to properly configured
                ODS_ASSISTSTRUCT record for the agent type.   This structure
                contains control information for the agent trigger type, 
                document search type, time interval and type and other 
                information.

    INPUTS:     NOTEHANDLE          hAgent      - handle to open agent note
                ODS_ASSISTSTRUCT    AgentInfo   - configured agent info 
                                                  structure
                                  
*************************************************************************/
STATUS  LNPUBLIC  SetAgentInfo( NOTEHANDLE hAgent, ODS_ASSISTSTRUCT AgentInfo )
{
        STATUS          error = NOERROR;
        char            *pBuffer;
        DHANDLE         hItem;
        DWORD           dwItemLen;
        BLOCKID         bhValue;
 
        /* Allocate an object to hold the ods object and its TYPE */
 
        dwItemLen = sizeof(WORD) + ODSLength(_ODS_ASSISTSTRUCT);
 
        if (error = OSMemAlloc(MEM_GROWABLE, dwItemLen, &hItem))
                goto Exit0;
 
        pBuffer = (char *)OSLockObject(hItem);
 
        /* Set the TYPE word in HOST format */
 
        *((WORD *)pBuffer) = TYPE_ASSISTANT_INFO;
        pBuffer += sizeof(WORD);
 
        /* Now write out the structure */
 
        ODSWriteMemory(&pBuffer, _ODS_ASSISTSTRUCT, &AgentInfo, 1);
        OSUnlockObject(hItem);
 
        /* Create a BLOCKID and fill it */
 
        bhValue.pool = hItem;
        bhValue.block = NULLBLOCK;
 
        /*      Save the item to the note */
 
        if (error = NSFItemAppendByBLOCKID(hAgent, /* handle to note to append*/
                                           ITEM_SIGN,                 /* item flags */
                                           ASSIST_INFO_ITEM,          /* item name: "$AssistInfo" */
                                           sizeof(ASSIST_INFO_ITEM)-1,/* item length */
                                           bhValue,                   /* BLOCKID with value */
                                           dwItemLen, NULL))          /* value length */
        {
            OSMemFree(hItem);
            goto Exit0;
        }
Exit0:

    if (error)
    {
        PRINTLOG("Error: unable to append AgentInfo item to Agent note.\n");
    }
    return (error);
}

/************************************************************************

    FUNCTION:   SetAgentAction

    PURPOSE:    Append the $AssistQuery, $AssistAction, and $AssistActionEx
                fields for the Agent note being created.  
                
                This routine assumes that there is no $AssistQuery 
                information, ie. no search query. So, the value for
                $AssistQuery item is only the CD header record.
                
                The $AssistAction item value is set to the passed
                pointer and length for the primary agent action.  
                This address contains the appropriate CD records for
                compiled Formula or a LotusScript subroutine source.
                
                The $AssistActionEx is supposed to contain the 
                compiled version of a LotusScript agent action for 
                use with the Domino and Notes IDE.  Since there is no way 
                to compile the source via the API at this time, this
                routine simply passes the address of the agent action
                subroutine source.

    INPUTS:     NOTEHANDLE          hAgent       - handle to open agent note
                BYTE                *AgentAction - pointer to agent action
                                                   CD records
                DWORD               dwActionLen  - length of AgentAction CD
                                                   record buffer

*************************************************************************/
STATUS  LNPUBLIC  SetAgentAction( NOTEHANDLE hAgent, 
                                    BYTE *AgentAction, 
                                    DWORD dwActionLen )
{
    STATUS              error=NOERROR;

    CDQUERYHEADER       AgentQuery;

/* First append "empty" TYPE_QUERY item since we assume no front-end query */
    AgentQuery.Header.Signature = SIG_QUERY_HEADER;
    AgentQuery.Header.Length= (BYTE) ODSLength(_CDQUERYHEADER);
    AgentQuery.dwFlags = (DWORD)0;

    if (error = NSFItemAppend(
                              hAgent,                            /* handle to note to append to */
                              ITEM_SIGN,                         /* item flags */
                              ASSIST_QUERY_ITEM,                 /* item name: "$AssistQuery" */
                              (WORD) strlen(ASSIST_QUERY_ITEM),
                              TYPE_QUERY,                        /* item type */
                              &AgentQuery,                       /* item value */
                              (DWORD) AgentQuery.Header.Length)) /* value length */
    {
        PRINTLOG("Error: unable to append AssistQuery item to Agent note.\n");
        return (error);
    }

/* Then apppend TYPE_ACTION item */ 
    if (error = NSFItemAppend(
                              hAgent,                           /* handle to note to append to */
                              ITEM_SIGN,                        /* item flags */
                              ASSIST_ACTION_ITEM,               /* item name: "$AssistAction" */
                              (WORD) strlen(ASSIST_ACTION_ITEM),
                              TYPE_ACTION,                      /* item type */
                              AgentAction,                      /* item value */
                              (DWORD) dwActionLen ))           /* value length */
    {
        PRINTLOG("Error: unable to append AssistAction item to Agent note.\n");
        return (error);
    }

/* Lastly, append "empty" TYPE_LSOBJECT (extra Action data) item */
    if (error = NSFItemAppend(
                              hAgent,                           /* handle to note to append to */
                              ITEM_SIGN,                        /* item flags */
                              ASSIST_EXACTION_ITEM,             /* item name: "$AssistAction_Ex" */
                              (WORD) strlen(ASSIST_EXACTION_ITEM),
                              TYPE_LSOBJECT,                    /* item type */
                              AgentAction,                      /* item value */
                              (DWORD) sizeof(WORD)))            /* value length */
    {
        PRINTLOG("Error: unable to append AssistActionEx item to Agent note.\n");
    }

    return (error);
}

/************************************************************************

    FUNCTION:   SetAgentRunInfo

    PURPOSE:    Append the $AssistRunInfo field for the Agent note being 
                created. The value of this field it is an attached run 
                information object used by Domino and Notes to store agent 
                execution data.  If a shared agent, then the object is a 
                public note and has a note class of NOTE_CLASS_DOCUMENT. 
                If a private agent, it has a NOTE_CLASS_FILTER | 
                NOTES_CLASS_PRIVATE note class.   
                
                The information contained by the object is split into a two
                components;  the object descriptor item followed by the 
                object run data. The object descriptor item consists of the
                TYPE_OBJECT object type. This is followed by an 
                OBJECT_DESCRIPTOR structure, in which the RRV field is set 
                to the allocated object ID and the ObjectType field set to 
                the OBJECT_ASSIST_RUNDATA constant (defined in nsfdata.h).

                The object run data that follows consists of a sequence of 
                CD records that contain the run data information. When 
                creating an agent note, this is an initial placeholder for 
                the first two CD records of the object run data.  The 
                appended CD records consist of one ODS_ASSISTRUNOBJECTHEADER
                structure followed by one ODS_ASSISTRUNOBJECTENTRY structure. 
                Domino and Notes will add and/or modify the remaining CD 
                records after subsequent agent executions. 

    INPUTS:     DBHANDLE    hDb         - handle to the open database
                NOTEHANDLE  hAgent      - handle to open agent note
                WORD        wAgentType  - SHARED_AGENT or PRIVATE_AGENT

*************************************************************************/
STATUS  LNPUBLIC  SetAgentRunInfo( DBHANDLE hDb, 
                                   NOTEHANDLE hAgent, 
                                   WORD wAgentType )
{
    STATUS      error=NOERROR;

    DHANDLE       hRunInfo;
    DWORD       dwObjectSize;
    WORD        *pw; 
    WORD        *pRunInfo;
    DWORD       dwObjectID;
    BLOCKID     bidRunInfo;            /* item value block ID (a handle) */
    DWORD       dwItemSize;
    OBJECT_DESCRIPTOR          objRunInfo;
    ODS_ASSISTRUNOBJECTHEADER  RunHeader;
    ODS_ASSISTRUNOBJECTENTRY   RunEntry;

    dwObjectSize = ODSLength(_ODS_ASSISTRUNOBJECTHEADER) + 
                   ODSLength(_ODS_ASSISTRUNOBJECTENTRY);

    /* if Private agent, then allocate private, filter note */
    if ( wAgentType == PRIVATE_AGENT )
    {
        if (error = NSFDbAllocObject(hDb, dwObjectSize, 
                                     (NOTE_CLASS_FILTER | NOTE_CLASS_PRIVATE),
                                     0, &dwObjectID))
        {
            PRINTLOG("Error: unable to allocate private RunInfo Object.\n");
            goto Exit1;
        }
    }

    /* else Shared agent use public data note */
    else
    {
        if (error = NSFDbAllocObject(hDb, dwObjectSize, 
                                     NOTE_CLASS_DOCUMENT, 
                                     0, &dwObjectID))
        {
            PRINTLOG("Error: unable to allocate shared RunInfo Object.\n");
            goto Exit1;
        }
    }

    /* set up OBJECT_DESCRIPTOR info */
    objRunInfo.ObjectType = OBJECT_ASSIST_RUNDATA;
    objRunInfo.RRV = dwObjectID;

    dwItemSize = (DWORD) (ODSLength(_WORD) + ODSLength(_OBJECT_DESCRIPTOR));

    if (error = OSMemAlloc(0, dwItemSize, &(bidRunInfo.pool)))
    {
        PRINTLOG("Error: unable to allocate %ld bytes for RunInfo item.\n",
                                    dwItemSize);
        NSFDbFreeObject(hDb, dwObjectID);
        goto Exit1;
    }
    bidRunInfo.block = NULLBLOCK;
    
    /* Set RunInfo OBJECT_TYPE to TYPE_OBJECT */
    pRunInfo = OSLockBlock(WORD, bidRunInfo);
    *pRunInfo = TYPE_OBJECT;
    pRunInfo++;

    /* and copy the OBJECT_DESCRIPTOR */
    ODSWriteMemory (&pRunInfo, _OBJECT_DESCRIPTOR, &objRunInfo, 1);
    OSUnlockBlock(bidRunInfo);

    /* finally append RunInfo object item */
    if (error = NSFItemAppendObject(hAgent, ITEM_SUMMARY,
                                    ASSIST_RUNINFO_ITEM, 
                                    (WORD) strlen (ASSIST_RUNINFO_ITEM),
                                    bidRunInfo,
                                    dwItemSize,
                                    TRUE))      /* Domino and Notes will deallocate memory */
    {
        PRINTLOG("Error: unable to append %s item.\n", ASSIST_RUNINFO_ITEM);
        OSMemFree(bidRunInfo.pool);
        goto Exit1;
    }

        /* set up initial Run object header and entry placeholder */
    if (error = OSMemAlloc(0, dwObjectSize, &hRunInfo))
    {
        PRINTLOG("Error: unable to allocate empty initial RunInfo structures.\n");
        goto Exit1;
    }

    pw = OSLock(WORD, hRunInfo);
    memset(&RunHeader, 0, sizeof(ODS_ASSISTRUNOBJECTHEADER));
    RunHeader.wEntries = 1;  /* placeholder entry */ 
    ODSWriteMemory(&pw, _ODS_ASSISTRUNOBJECTHEADER, &RunHeader, 1);
    memset(&RunEntry, 0, sizeof(ODS_ASSISTRUNOBJECTENTRY));
    ODSWriteMemory(&pw, _ODS_ASSISTRUNOBJECTENTRY, &RunEntry, 1);
    OSUnlock(hRunInfo);

    /* and add to object */
    if (error = NSFDbWriteObject(hDb, dwObjectID, hRunInfo, 0, dwObjectSize))
    {
        PRINTLOG("Error: unable to assign initial AssistRunInfo field.\n");
        goto Exit1;
    }

Exit1:
    return (error);
}
