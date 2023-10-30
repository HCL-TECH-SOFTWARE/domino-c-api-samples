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

    PROGRAM: RAGENTS

    FILE:    ragents.c

    SYNTAX:  ragents <database filename>

    PURPOSE: Shows how to execute various V4 Agent notes via the API

    DESCRIPTION:
        This sample executes the background and scheduled agents that 
        were created with agents.exe and reads and displays the results.

        For the ragent.exe to execute successfully, you must use the sample
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
#include <nif.h>            /* NIFFindDesignNote, etc. */
#include <nsfsearc.h>       
#include <nsfobjec.h>       
#include <kfm.h>            
#include <osmem.h>
#include <ostime.h>
#include <agents.h>         /* Agent execution header */ 
#include <osmisc.h>
#include"../../printLog.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/* Function Prototypes */
STATUS  LNPUBLIC  RunTriggeredAgents ( DBHANDLE );
STATUS  LNPUBLIC  GetAgentRunInfo( DBHANDLE, char * );
STATUS  LNPUBLIC  OpenAgent ( DBHANDLE, char *, HAGENT * );
STATUS  LNPUBLIC  SetRunContext ( HAGENT, WORD, HAGENTCTX * );
STATUS  LNPUBLIC  ExecuteAgent( HAGENT, HAGENTCTX );
void    LNPUBLIC  GetScriptRunInfo( HAGENTCTX );
void    LNPUBLIC  CloseAgentContext ( HAGENTCTX );
void    LNPUBLIC  CloseAgent ( HAGENT );

/* program constants */
char szAGENT_BACKGROUND[] = "Assign Support Rep";
char szAGENT_SCHEDULED[] = "Escalate Priority";
#define LOTUSSCRIPT_ACTION  0
#define FORMULA_ACTION      1

/*log file and file pointer */
#define AGENTS_LOG   "agents.log" 
FILE    *pAgentsLog;  

/*LotusScript agent "Parameter Document" variables */
NOTEHANDLE  hParmNote;
#define     PARM_FIELD  "NewRep"
#define     PARM_VALUE  "Isabel Silton"


/************************************************************************

    FUNCTION:   main

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
        PRINTLOG ("Usage: ragents <database filename>\n");
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
    if ((pAgentsLog = fopen(AGENTS_LOG, "w+")) == NULL) 
    {
        PRINTLOG ("Error: unable to open output log file '%s'\n", AGENTS_LOG);
        goto Exit0;
    }

    /* Now force execution of the two trigerred agent notes */
    if (error = RunTriggeredAgents(hDb))
        goto Exit1;

    fprintf(pAgentsLog, "Successfully executed the triggered agent notes in '%s'.\n",
            szDbName);
    PRINTLOG ("Program execution completed.  See logfile '%s' for results.\n",
               AGENTS_LOG);
    fprintf (pAgentsLog, "Program execution completed.\n", AGENTS_LOG);


    /* Close database, close log file pointer. */
Exit1:
    NSFDbClose(hDb);
    fclose (pAgentsLog);

Exit0:
    if (error)
    {
       PRINTERROR(error, "fopen");
    }
    NotesTerm();
    return(error);
}


/************************************************************************

    FUNCTION:   RunTriggeredAgents(DBHANDLE hDb)

    PURPOSE:    This routine drives the setup, execution, and results
                processing of the background (LotusScript) and scheduled 
                (Formula) agents notes.

    INPUTS:     DBHANDLE    hDb             - handle to the open database

*************************************************************************/
STATUS  LNPUBLIC  RunTriggeredAgents( DBHANDLE hDb )
{
    STATUS      error = NOERROR;

    HAGENT      hBackgroundAgent;
    HAGENT      hScheduledAgent;
    HAGENTCTX   hAgentRunCtx;

    WORD        wNoteClass;
    NOTEID      ParmNoteID;

    /* Set up execution environment for the background and scheduled agents*/
    if (error = OpenAgent(hDb, szAGENT_BACKGROUND, &hBackgroundAgent ))
        goto Exit0;

    if (error = OpenAgent(hDb, szAGENT_SCHEDULED, &hScheduledAgent))
        goto Exit1;

    /* Create Parameter Document Note for Script agent action.  
     * This parameter specifies the Support Rep name that is to be
     * assigned to unassigned problem reports, ie. "Isabel Silton".
     */
    if (error = NSFNoteCreate(hDb, &hParmNote))
        goto Exit2;
    
    wNoteClass = NOTE_CLASS_DOCUMENT;
    NSFNoteSetInfo(hParmNote, _NOTE_CLASS, &wNoteClass);

    if (error = NSFItemSetText(hParmNote, PARM_FIELD, PARM_VALUE,
                               (WORD)strlen(PARM_VALUE)) )
        goto Exit2;

    if (error = NSFNoteUpdate(hParmNote, 0))
        goto Exit2;

    NSFNoteGetInfo(hParmNote, _NOTE_ID, &ParmNoteID);

    /* Set up agent run context for the background agent */
    if (error = SetRunContext(hBackgroundAgent, LOTUSSCRIPT_ACTION, 
                              &hAgentRunCtx))
        goto Exit3;

    /* Run the background agent */
    if (error = ExecuteAgent(hBackgroundAgent, hAgentRunCtx))
        goto Exit4;

    fprintf(pAgentsLog, "Successfully ran agent '%s'.\n", szAGENT_BACKGROUND);

    /* Report the run data results of the background agent */
    GetScriptRunInfo(hAgentRunCtx);

    if (error = GetAgentRunInfo(hDb, szAGENT_BACKGROUND ))
        goto Exit4;

    fprintf(pAgentsLog, "Successfully read run data for agent '%s'.\n\n", 
                        szAGENT_BACKGROUND);

    /* Reset the agent run context for the scheduled agent */
    if (error = SetRunContext(hScheduledAgent, FORMULA_ACTION, 
                              &hAgentRunCtx))
        goto Exit4;

    /* Run the scheduled agent */
    if (error = ExecuteAgent(hScheduledAgent, hAgentRunCtx))
        goto Exit4;

    fprintf(pAgentsLog, "Successfully ran agent '%s'.\n", szAGENT_SCHEDULED);

    /* Report the run data results of the scheduled agent */
    if (error = GetAgentRunInfo(hDb, szAGENT_SCHEDULED ))
        goto Exit4;

    fprintf(pAgentsLog, "Successfully read run data for agent '%s'.\n\n", 
                        szAGENT_SCHEDULED);

Exit4:
    /* Free the agent run context*/
    CloseAgentContext(hAgentRunCtx);
    
Exit3:
    /* Close and Delete "Parameter" Note */
    NSFNoteClose(hParmNote);
    NSFNoteDelete(hDb, ParmNoteID, (WORD)0);

Exit2:
    /* Free the handles to open scheduled agent */
    CloseAgent(hScheduledAgent);
    
Exit1:
    /* Free the handle to open background agent */
    CloseAgent(hBackgroundAgent);

Exit0:
    return(error);
}

/************************************************************************

    FUNCTION:   OpenAgent(DBHANDLE hDb, char *szAgentName, HAGENT *hOpenAgent)

    PURPOSE:    Locate the specified agent note, and create/return an open
                agent handle for subsequent execution.   Calls the HCL 
                                C API for Domino and Notes routine, AgentOpen().

    INPUTS:     DBHANDLE    hDb             - handle to the open database
                char        *szAgentName    - agent name ($TITLE)

    OUTUTS:     HAGENT      *hOpenAgent     - open agent handle

*************************************************************************/
STATUS  LNPUBLIC  OpenAgent( DBHANDLE hDb, char *szAgentName,
                                 HAGENT *hOpenAgent )
{
    STATUS          error = NOERROR;

/* Execute Agent Variables */
    NOTEID          AgentId;     

/* Given the handle of the database and the Agent name,
 * find the Note ID for the Agent.
 */
    if (error = NIFFindDesignNote (hDb, szAgentName,
                                   NOTE_CLASS_FILTER, &AgentId))
    {
        error = NIFFindPrivateDesignNote (hDb, szAgentName,
                                          NOTE_CLASS_FILTER, &AgentId);
        if (error)
            return (error);
    }

/* Now open up, and return a handle to the open agent */
    error = AgentOpen(hDb, AgentId, hOpenAgent);
    return (error);
}

/************************************************************************

    FUNCTION:   SetRunContext(HAGENT hOpenAgent, WORD wActionType, 
                                HAGENTCTX *hOpenAgentCtx)

    PURPOSE:    Creates and returns the open agent runtime context for the
                specified agent handle and action type.   The runtime context
                for LotusScript based actions include support for standard
                output redirection and parameter passing.  Calls the HCL 
                                C API for Domino and Notes routines AgentCreateRunContext(), 
                                AgentRedirectStdout(),and AgentSetDocumentContext().

    INPUTS:     HAGENT      hOpenAgent      - handle to open agent
                WORD        wActionType     - either LOTUSSCRIPT_ACTION or
                                              FORMULA_ACTION
    OUTUTS:     HAGENT      *hOpenAgentCtx  - agent runtime context handle

*************************************************************************/
STATUS  LNPUBLIC  SetRunContext( HAGENT hOpenAgent, WORD wActionType,
                                 HAGENTCTX *hOpenAgentCtx )
{
    STATUS          error = NOERROR;

    if (error = AgentCreateRunContext (hOpenAgent, NULL, (DWORD) 0, hOpenAgentCtx))
        goto Exit0;


/* if LotusScript agent action, then redirect PRINT statements to memory,
   and set "Parameter Document" to current hScriptParm value (assumed set) */
    if (wActionType == LOTUSSCRIPT_ACTION)
    {
        if (error = AgentRedirectStdout (*hOpenAgentCtx, AGENT_REDIR_MEMORY))
            goto Exit0;

        if (hParmNote != NULLHANDLE)
            error = AgentSetDocumentContext (*hOpenAgentCtx, hParmNote);
    }            

Exit0:
    return (error);
}

/************************************************************************

    FUNCTION:   ExecuteAgent(HAGENT hOpenAgent, HAGENTCTX hOpenAgentCtx)

    PURPOSE:    Run the Agent, as specified by the agent handle and runtime
                context.  Calls the HCL C API for Notes/Domino 
                                routine AgentRun().

    INPUTS:     HAGENT      hOpenAgent      - handle to open agent
                HAGENTCTX   hOpenAgentCtx   - agent runtime context handle

*************************************************************************/
STATUS  LNPUBLIC  ExecuteAgent( HAGENT hOpenAgent, HAGENTCTX hOpenAgentCtx )
{
    STATUS          error = NOERROR;

    error = AgentRun (hOpenAgent, hOpenAgentCtx, (DHANDLE) 0, (DWORD) 0);
    return (error);
}

/************************************************************************

    FUNCTION:   GetScriptRunInfo(HAGENTCTX hOpenAgentCtx)

    PURPOSE:    Get and report the redirected standard output string for 
                the specified LotusScript based agent runtime context.
                Calls the HCL C API for Notes/Domino routine 
                                AgentQueryStdoutBuffer().

    INPUTS:     HAGENTCTX   hOpenAgentCtx   - agent runtime context handle

*************************************************************************/
void  LNPUBLIC  GetScriptRunInfo ( HAGENTCTX hOpenAgentCtx )
{
    DHANDLE         hStdout;
    DWORD           dwLen;
    char            *pStdout;

    AgentQueryStdoutBuffer (hOpenAgentCtx, &hStdout, &dwLen);
    pStdout = OSLock(char, hStdout);
    pStdout[dwLen]='\0';
    fprintf(pAgentsLog, "\nRedirected PRINT statements:\n%s\n",pStdout);
    OSUnlock(hStdout);

    return;
}

/************************************************************************

    FUNCTION:   CloseAgent(HAGENT hOpenAgent)

    PURPOSE:    Close the specified handle to the open agent. 
                Calls the HCL C API for Notes/Domino routines 
                AgentClose().

    INPUTS:     HAGENT      hOpenAgent      - handle to open agent 

*************************************************************************/
void  LNPUBLIC  CloseAgent ( HAGENT hOpenAgent )
{

/* Close open agent handle */
    AgentClose (hOpenAgent);

    return;
}
   

/************************************************************************

    FUNCTION:   CloseAgentContext(HAGENTCTX hOpenAgentCtx)

    PURPOSE:    Close the handle to the open agent runtime context.  
                Calls the HCL C API for Notes/Domino routine 
                AgentDestroyRunContext().

    INPUTS:     HAGENTCTX   hOpenAgentCtx   - agent runtime context handle

*************************************************************************/
void  LNPUBLIC  CloseAgentContext ( HAGENTCTX hOpenAgentCtx )
{

/* If specified, close the open agent context handle */
    if (hOpenAgentCtx != NULLHANDLE)
        AgentDestroyRunContext (hOpenAgentCtx);

    return;
}
   

/************************************************************************

    FUNCTION:   GetAgentRunInfo (DBHANDLE hDb, char *szAgentName)

    PURPOSE:    Read the agent run data ($AssistRunInfo item) of an 
                executed agent note.
                
                This routine sequentially reads the CD records associated
                with the run data objects created by Domino and Notes and 
                                attached to the $AssistRunInfo item.  
                
                First, the number of object entries are determined by 
                reading the ODS_ASSISTRUNOBJECTHEADER record.  Next, the size
                of each object entry is determined by reading the sequence
                of ODS_ASSISTRUNOBJECTENTRY records, and used to determine
                which object has data.  Finally, if the ODS_ASSISTRUNINFO 
                record (entry 1) and the execution log entry (entry 3) have
                data, they read and/or displayed as appropriate.  

                Note that any other object entries are reserved by Domino 
                and Notes and not read.

    INPUTS:     DBHANDLE    hDb             - handle to the open database
                char        *szAgentName    - agent name ($TITLE)

*************************************************************************/
STATUS  LNPUBLIC  GetAgentRunInfo( DBHANDLE hDb, char *szAgentName )
{
    STATUS                      error = NOERROR;
    NOTEID                      AgentId;        
    NOTEHANDLE                  hAgentNote;

/* $AssistRunInfo run data object variables */
    OBJECT_DESCRIPTOR           objRunInfo;
    BLOCKID                     bidRunInfo;
    WORD                        wDataType;
    DWORD                       dwItemSize;
    char                       *pObject;
    DWORD                       dwObjectSize;
    WORD                        wClass;
    WORD                        wPrivs;
    WORD                        wCounter;
    DHANDLE                     hBuffer;
    DWORD                       dwOffset;
    ODS_ASSISTRUNOBJECTHEADER   RunHeader;
    ODS_ASSISTRUNOBJECTENTRY    RunEntry[5];
    ODS_ASSISTRUNINFO           RunInfo;
    char                        RunTime[MAXALPHATIMEDATE+1];
    WORD                        wLength;

/* Given the handle of the database and the Agent name,
 * find the Note ID for the Agent.
 */
    if (error = NIFFindDesignNote (hDb, szAgentName,
                                   NOTE_CLASS_FILTER, &AgentId))
    {
        if (error = NIFFindPrivateDesignNote (hDb, szAgentName, NOTE_CLASS_FILTER, &AgentId))
            goto Exit0; 
    }

/* Now report the results of the agent execution stored in the 
   $AssistRunInfo object item of the Agent note */ 
    if (error = NSFNoteOpen (hDb, AgentId, (WORD) 0, &hAgentNote))
        goto Exit0;

    if (error = NSFItemInfo(hAgentNote, ASSIST_RUNINFO_ITEM, 
                        (WORD) strlen (ASSIST_RUNINFO_ITEM),
                        NULL, &wDataType, &bidRunInfo, &dwItemSize))
        goto Exit1;

/* assign pointer to item, and skip over TYPE_OBJECT word */
    pObject = OSLockBlock(char, bidRunInfo);
    pObject += ODSLength(_WORD);

/* Read in the OBJECT_DESCRIPTOR ODS record and ensure that there are 
data objects */
    ODSReadMemory( &pObject, _OBJECT_DESCRIPTOR, &objRunInfo, 1 );
    OSUnlockBlock(bidRunInfo);
    if (error = NSFDbGetObjectSize (hDb, objRunInfo.RRV, objRunInfo.ObjectType,
                                        &dwObjectSize, &wClass, &wPrivs ))
        goto Exit1;
    
    if (dwObjectSize == 0)
    {
        fprintf(pAgentsLog, "Invalid run data object item\n\n");
        goto Exit1;
    }

/* 
 * Sequentially read in the header, entries, and related run information objects
 * that are created during an agent execution.
 */
    dwOffset = 0;

/* 
 * ODS_ASSISTRUNOBJECTHEADER - contains the number of run data object entries. 
 * By default, there are at least 5 objects reserved for run data.  Of 
 * importance, the first of these always contains the RunInfo data.   The 
 * third of these always contains an agent execution log.  
 */
    if (error = NSFDbReadObject(hDb, objRunInfo.RRV, dwOffset, 
                        ODSLength(_ODS_ASSISTRUNOBJECTHEADER), &hBuffer))
        goto Exit1;
    pObject = OSLock (char, hBuffer);
    ODSReadMemory( &pObject, _ODS_ASSISTRUNOBJECTHEADER, &RunHeader, 1 );
    OSUnlock(hBuffer);
    OSMemFree(hBuffer);
    if (RunHeader.wEntries == 0)
    {
        fprintf(pAgentsLog, "No run data entries to report; Agent never executed\n\n");
        goto Exit1;
    }
    dwOffset += ODSLength(_ODS_ASSISTRUNOBJECTHEADER);

/* 
 * ODS_ASSISTRUNOBJECTENTRY - determines which entry has run data.
 * If the entry length of both the run info data and log objects are greater 
 * than zero, than the agent has been executed at least once and run data 
 * exists.  */

    if (error = NSFDbReadObject(hDb,objRunInfo.RRV, dwOffset, 
                ODSLength(_ODS_ASSISTRUNOBJECTENTRY)*RunHeader.wEntries, &hBuffer))
        goto Exit1;
    pObject = OSLock (char, hBuffer);

/* read the first entry and make sure that a RunInfo data object exists. */
    ODSReadMemory( &pObject, _ODS_ASSISTRUNOBJECTENTRY, &RunEntry[0], 1 );
    if (RunEntry[0].dwLength == 0)
    {
        fprintf(pAgentsLog, "No run information data to report; Agent never executed\n\n");
        goto Exit1;
    }
    fprintf(pAgentsLog, "Agent Run Information:\n");

/* read the third entry and make sure that an execution log object exists.
 * Note - we check this only because the Notes UI creates an empty RunInfo 
 * data object when an agent is executed, but only creates the log object 
 * if executed. 
 */
    ODSReadMemory( &pObject, _ODS_ASSISTRUNOBJECTENTRY, &RunEntry[1], 1 );
    ODSReadMemory( &pObject, _ODS_ASSISTRUNOBJECTENTRY, &RunEntry[2], 1 );
    if (RunEntry[2].dwLength == 0)
    {
        fprintf(pAgentsLog, "Agent log does not exist; Never executed\n\n");
        goto Exit1;
    }

/* and skip over the remaining reserved run data objects */
    for (wCounter = 3; wCounter < RunHeader.wEntries; wCounter++)
        ODSReadMemory( &pObject, _ODS_ASSISTRUNOBJECTENTRY, &RunEntry[wCounter], 1 );

    dwOffset += ODSLength(_ODS_ASSISTRUNOBJECTENTRY)*RunHeader.wEntries;
    OSUnlockObject(hBuffer);
    OSMemFree(hBuffer);

/* 
 * ODS_ASSISTRUNINFO - contain the Run Information.
 * It is always the first run data object. 
 */
    if (error = NSFDbReadObject(hDb, objRunInfo.RRV, dwOffset, 
                ODSLength(_ODS_ASSISTRUNINFO), &hBuffer))
        goto Exit1;
    pObject = OSLock (char, hBuffer);
    ODSReadMemory( &pObject, _ODS_ASSISTRUNINFO, &RunInfo, 1 );
    OSUnlock(hBuffer);
    OSMemFree(hBuffer);
    (void) ConvertTIMEDATEToText (NULL, NULL, &RunInfo.LastRun, RunTime, 
                                        MAXALPHATIMEDATE, &wLength);
    RunTime[wLength] = '\0'; 
    fprintf(pAgentsLog, "TimeDate of Execution: %s\n", RunTime);
    fprintf(pAgentsLog, "Documents Processed By Formula: %ld\n", RunInfo.dwProcessed);
    fprintf(pAgentsLog, "Exit Code: %ld\n\n", RunInfo.dwExitCode);

/* skip the next one, display the execution log, and dump the rest */
    dwOffset += RunEntry[0].dwLength;
    for (wCounter = 1;wCounter < RunHeader.wEntries; wCounter++)
    {
        if (RunEntry[wCounter].dwLength != 0)
        {
            if (error = NSFDbReadObject(hDb, objRunInfo.RRV, dwOffset, 
                        RunEntry[wCounter].dwLength, &hBuffer))
                goto Exit1;
            pObject = OSLock (char, hBuffer);
            if (wCounter == 2)
            {
                pObject[RunEntry[wCounter].dwLength]='\0';
                fprintf(pAgentsLog, "Agent Log <Start>:\n%s", pObject);
                fprintf(pAgentsLog, "Agent Log <End>:\n");
            }
            dwOffset += RunEntry[wCounter].dwLength;
            OSUnlock(hBuffer);
            OSMemFree(hBuffer);
        }
    }

Exit1:
    NSFNoteClose (hAgentNote);

Exit0:
    return (error);
}
