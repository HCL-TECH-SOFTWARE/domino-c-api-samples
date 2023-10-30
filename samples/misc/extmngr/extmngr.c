/*
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

PROGRAM:    extmngr.dll

FILE:       extmngr.c

Extension Manager HCL C API for Notes/Domino Example Program

OVERVIEW:
This sample program registers and deregisters all of the Extension
Manager functions to show how they interact in an application.  It also
transforms a non Domino database "animals.db" into a Domino database
"animals.nsf" when specific extension manager hooks are called.

The sample attempts to register a callback function for a selected set of
Domino and Notes extension manager hooks.  When the program traps an
EM_NSFDBCREATE and EM_NSFDBCLOSE hook for the Database "animals.nsf", the
data in the database is built from an existing file "animals.db"
containing information and transformed into a Domino Database. The
extension manager hooks are deregistered when Domino and Notes is exited.
The log file OUTPUT.LOG, tracks extension manager calls and is created in
the Domino and Notes Data directory.

*/
#if defined(OS400)
#pragma convert(850)
#endif



#if defined(NT)
#include "windows.h"
#include <string.h>
#endif

//#include <stdio.h>


#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
/*
*  Domino and Notes header files
*/
#if defined(OS390)
#include <lapicinc.h>
#endif



#include <global.h>
#include <ixport.h>
#include <dbdrv.h>
#include <dbdrverr.h>
#include <osmem.h>
#include <nsfdata.h>
#include <names.h>
#include <osfile.h>

#include <nif.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <editods.h>
#include <stdnames.h>
#include <nsfsearc.h>
#include <colorid.h>
#include <miscerr.h>
#include <actods.h>
#include <nifcoll.h>
#include <osmisc.h>
#include <viewfmt.h>

#include "extmngr.h"



#if !defined(ND64) 
#define DHANDLE HANDLE 
#endif

/*===== GLOBAL VARIABLES =============================*/

EMHANDLER   gHandlerProc;
char        gTextBuffer[1024];
char        DataDir[256];
char        FileName[256];
BOOL        gHooksRegistered=FALSE;
WORD        gRecursionID;
DBRECORD    *pDBRecord;  /* Global record pointer                 */


#if defined (OS400)
DBRECORD    r_pDBRecord[5];
#endif

#if defined(NT) || defined (OS400) || defined (UNIX)
FILE         *gFStream = (__FILE *)0;
#endif

#if defined(NT)
/* Storage needed for synchronization under Windows NT */
CRITICAL_SECTION    gCriticalSection;
#endif

TExtensionInfo  ExtensionHookTable[] = {
	{EM_NSFDBCLOSESESSION, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBCLOSESESSION"},
	{EM_NSFDBCLOSE, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBCLOSE"},
	{EM_NSFDBCREATE, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBCREATE"},
	{EM_NSFDBDELETE, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBDELETE"},
	{EM_NSFNOTEOPEN, EM_REG_BEFORE, 0, FALSE, "EM_NSFNOTEOPEN"},
	{EM_NSFNOTECLOSE, EM_REG_BEFORE, 0, FALSE, "EM_NSFNOTECLOSE"},
	{EM_NSFNOTECREATE, EM_REG_BEFORE, 0, FALSE, "EM_NSFNOTECREATE"},
	{EM_NSFNOTEDELETE, EM_REG_BEFORE, 0, FALSE, "EM_NSFNOTEDELETE"},
	{EM_NSFNOTEOPENBYUNID, EM_REG_BEFORE, 0, FALSE, "EM_NSFNOTEOPENBYUNID"},
	{EM_FTGETLASTINDEXTIME, EM_REG_BEFORE, 0, FALSE, "EM_FTGETLASTINDEXTIME"},
	{EM_FTINDEX, EM_REG_BEFORE, 0, FALSE, "EM_FTINDEX"},
	{EM_FTSEARCH, EM_REG_BEFORE, 0, FALSE, "EM_FTSEARCH"},
	{EM_NIFFINDBYKEY, EM_REG_BEFORE, 0, FALSE, "EM_NIFFINDBYKEY"},
	{EM_NIFFINDBYNAME, EM_REG_BEFORE, 0, FALSE, "EM_NIFFINDBYNAME"},
	{EM_NIFREADENTRIES, EM_REG_BEFORE, 0, FALSE, "EM_NIFREADENTRIES"},
	{EM_NIFUPDATECOLLECTION, EM_REG_BEFORE, 0, FALSE, "EM_NIFUPDATECOLLECTION"},
	{EM_NSFDBALLOCOBJECT, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBALLOCOBJECT"},
	{EM_NSFDBCOMPACT, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBCOMPACT"},
	{EM_NSFDBDELETENOTES, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBDELETENOTES"},
	{EM_NSFDBFREEOBJECT, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBFREEOBJECT"},
	{EM_NSFDBGETMODIFIEDNOTETABLE, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBGETMODIFIEDNOTETABLE"},
	{EM_NSFDBGETNOTEINFO, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBGETNOTEINFO"},
	{EM_NSFDBGETNOTEINFOBYUNID, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBGETNOTEINFOBYUNID"},
	{EM_NSFDBGETOBJECTSIZE, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBGETOBJECTSIZE"},
	{EM_NSFDBGETSPECIALNOTEID, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBGETSPECIALNOTEID"},
	{EM_NSFDBINFOGET, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBINFOGET"},
	{EM_NSFDBINFOSET, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBINFOSET"},
	{EM_NSFDBLOCATEBYREPLICAID, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBLOCATEBYREPLICAID"},
	{EM_NSFDBMODIFIEDTIME, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBMODIFIEDTIME"},
	{EM_NSFDBREADOBJECT, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBREADOBJECT"},
	{EM_NSFDBREALLOCOBJECT, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBREALLOCOBJECT"},
	{EM_NSFDBREPLICAINFOGET, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBREPLICAINFOGET"},
	{EM_NSFDBREPLICAINFOSET, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBREPLICAINFOSET"},
	{EM_NSFDBSPACEUSAGE, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBSPACEUSAGE"},
	{EM_NSFDBSTAMPNOTES, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBSTAMPNOTES"},
	{EM_NSFDBWRITEOBJECT, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBWRITEOBJECT"},
	{EM_NSFNOTEUPDATE, EM_REG_BEFORE, 0, FALSE, "EM_NSFNOTEUPDATE"},
	{EM_NIFOPENCOLLECTION, EM_REG_BEFORE, 0, FALSE, "EM_NIFOPENCOLLECTION"},
	{EM_NIFCLOSECOLLECTION, EM_REG_BEFORE, 0, FALSE, "EM_NIFCLOSECOLLECTION"},
	{EM_NSFDBGETBUILDVERSION, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBGETBUILDVERSION"},
	{EM_NSFDBRENAME, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBRENAME"},
	{EM_NSFDBITEMDEFTABLE, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBITEMDEFTABLE"},
	{EM_NSFDBREOPEN, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBREOPEN"},
	{EM_NSFDBOPENEXTENDED, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBOPENEXTENDED"},
	{EM_NSFNOTEDECRYPT, EM_REG_BEFORE, 0, FALSE, "EM_NSFNOTEDECRYPT"},
	{EM_GETPASSWORD, EM_REG_BEFORE, 0, FALSE, "EM_GETPASSWORD"},
	{EM_SETPASSWORD, EM_REG_BEFORE, 0, FALSE, "EM_SETPASSWORD"},
	{EM_CLEARPASSWORD, EM_REG_BEFORE, 0, FALSE, "EM_CLEARPASSWORD"},
	{EM_NSFCONFLICTHANDLER, EM_REG_BEFORE, 0, FALSE, "EM_NSFCONFLICTHANDLER"},
	{EM_NSFNOTEUPDATEXTENDED, EM_REG_BEFORE, 0, FALSE, "NSFNOTEUPDATEXTENDED"},
	{EM_SCHFREETIMESEARCH, EM_REG_BEFORE, 0, FALSE, "SCHFREETIMESEARCH"},
	{EM_SCHRETRIEVE, EM_REG_BEFORE, 0, FALSE, "SCHRETRIEVE"},
	{EM_SCHSRVRETRIEVE, EM_REG_BEFORE, 0, FALSE, "SCHSRVRETRIEVE"},
	{EM_NSFDBCOMPACTEXTENDED, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBCOMPACTEXTENDED"},
	{EM_ADMINPPROCESSREQUEST, EM_REG_BEFORE, 0, FALSE, "EM_ADMINPPROCESSREQUEST"},
	{EM_NIFGETCOLLECTIONDATA, EM_REG_BEFORE, 0, FALSE, "NIFGETCOLLECTIONDATA"},
	{EM_NSFDBCOPYNOTE, EM_REG_BEFORE, 0, FALSE, "NSFDBCOPYNOTE"},
	{EM_NSFNOTECOPY, EM_REG_BEFORE, 0, FALSE, "NSFNOTECOPY"},
	{EM_NSFNOTEATTACHFILE, EM_REG_BEFORE, 0, FALSE, "NSFNOTEATTACHFILE"},
	{EM_NSFNOTEDETACHFILE, EM_REG_BEFORE, 0, FALSE, "NSFNOTEDETACHFILE"},
	{EM_NSFNOTEEXTRACTFILE, EM_REG_BEFORE, 0, FALSE, "NSFNOTEEXTRACTFILE"},
	{EM_NSFNOTEATTACHOLE2OBJECT, EM_REG_BEFORE, 0, FALSE, "NSFNOTEATTACHOLE2OBJECT"},
	{EM_NSFNOTEDELETEOLE2OBJECT, EM_REG_BEFORE, 0, FALSE, "NSFNOTEDELETEOLE2OBJECT"},
	{EM_NSFNOTEEXTRACTOLE2OBJECT, EM_REG_BEFORE, 0, FALSE, "NSFNOTEEXTRACTOLE2OBJECT"},
	{EM_NSGETSERVERLIST, EM_REG_BEFORE, 0, FALSE, "NSGETSERVERLIST"},
	{EM_NSFDBCOPY, EM_REG_BEFORE, 0, FALSE, "NSFDBCOPY"},
	{EM_NSFDBCREATEANDCOPY, EM_REG_BEFORE, 0, FALSE, "NSFDBCREATEANDCOPY"},
	{EM_NSFDBCOPYACL, EM_REG_BEFORE, 0, FALSE, "NSFDBCOPYACL"},
	{EM_NSFDBCOPYTEMPLATEACL, EM_REG_BEFORE, 0, FALSE, "NSFDBCOPYTEMPLATEACL"},
	{EM_NSFDBCREATEACLFROMTEMPLATE, EM_REG_BEFORE, 0, FALSE, "NSFDBCREATEACLFROMTEMPLATE"},
	{EM_NSFDBREADACL, EM_REG_BEFORE, 0, FALSE, "NSFDBREADACL"},
	{EM_NSFDBSTOREACL, EM_REG_BEFORE, 0, FALSE, "NSFDBSTOREACL"},
	{EM_NSFDBFILTER, EM_REG_BEFORE, 0, FALSE, "NSFDBFILTER"},
	{EM_FTDELETEINDEX, EM_REG_BEFORE, 0, FALSE, "FTDELETEINDEX"},
	{EM_NSFNOTEGETINFO, EM_REG_BEFORE, 0, FALSE, "NSFNOTEGETINFO"},
	{EM_NSFNOTESETINFO, EM_REG_BEFORE, 0, FALSE, "NSFNOTESETINFO"},
	{EM_NSFNOTECOMPUTEWITHFORM, EM_REG_BEFORE, 0, FALSE, "NSFNOTECOMPUTEWITHFORM"},
	{EM_NIFFINDDESIGNNOTE, EM_REG_BEFORE, 0, FALSE, "NIFFINDDESIGNNOTE"},
	{EM_NIFFINDPRIVATEDESIGNNOTE, EM_REG_BEFORE, 0, FALSE, "NIFFINDPRIVATEDESIGNNOTE"},
	{EM_NIFGETLASTMODIFIEDTIME, EM_REG_BEFORE, 0, FALSE, "NIFGETLASTMODIFIEDTIME"},
	{EM_FTSEARCHEXT, EM_REG_BEFORE, 0, FALSE, "FTSEARCHEXT"},
	{EM_NAMELOOKUP, EM_REG_BEFORE, 0, FALSE, "NAMELOOKUP"},
	{EM_NSFNOTEUPDATEMAILBOX, EM_REG_BEFORE, 0, FALSE, "NSFNOTEUPDATEMAILBOX"},
	{EM_NIFFINDDESIGNNOTEEXT, EM_REG_BEFORE, 0, FALSE, "NIFFINDDESIGNNOTEEXT"},
	{EM_AGENTOPEN, EM_REG_BEFORE, 0, FALSE, "AGENTOPEN"},
	{EM_AGENTCLOSE, EM_REG_BEFORE, 0, FALSE, "AGENTCLOSE"},
	{EM_AGENTISENABLED, EM_REG_BEFORE, 0, FALSE, "AGENTISENABLED"},
	{EM_AGENTCREATERUNCONTEXT, EM_REG_BEFORE, 0, FALSE, "AGENTCREATERUNCONTEXT"},
	{EM_AGENTDESTROYRUNCONTEXT, EM_REG_BEFORE, 0, FALSE, "AGENTDESTROYRUNCONTEXT"},
	{EM_AGENTSETDOCUMENTCONTEXT, EM_REG_BEFORE, 0, FALSE, "AGENTSETDOCUMENTCONTEXT"},
	{EM_AGENTSETTIMEEXECUTIONLIMIT, EM_REG_BEFORE, 0, FALSE, "AGENTSETTIMEEXECUTIONLIMIT"},
	{EM_AGENTQUERYSTDOUTBUFFER, EM_REG_BEFORE, 0, FALSE, "AGENTQUERYSTDOUTBUFFER"},
	{EM_AGENTREDIRECTSTDOUT, EM_REG_BEFORE, 0, FALSE, "AGENTREDIRECTSTDOUT"},
	{EM_NAMELOOKUP2, EM_REG_BEFORE, 0, FALSE, "NAMELOOKUP2"},
	{EM_NSFNOTEOPENEXTENDED, EM_REG_BEFORE, 0, FALSE, "EM_NSFNOTEOPENEXTENDED"},
	{EM_TERMINATENSF, EM_REG_BEFORE, 0, FALSE, "EM_TERMINATENSF"},
	{EM_MAILSENDNOTE, EM_REG_BEFORE, 0, FALSE, "EM_MAILSENDNOTE"},
	{EM_AGENTRUN, EM_REG_BEFORE, 0, FALSE, "EM_AGENTRUN"},
	{EM_SECAUTHENTICATION, EM_REG_BEFORE, 0, FALSE, "EM_SECAUTHENTICATION"},
	{EM_NSFMARKREAD, EM_REG_BEFORE, 0, FALSE, "EM_NSFMARKREAD"},
	{EM_NSFADDTOFOLDER, EM_REG_BEFORE, 0, FALSE, "EM_NSFADDTOFOLDER"},
	{EM_NSFDBSPACEUSAGESCALED, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBSPACEUSAGESCALED"},
	{EM_NSFDBGETMAJMINVERSION, EM_REG_BEFORE, 0, FALSE, "EM_NSFDBGETMAJMINVERSION"},
	{EM_ROUTERJOURNALMESSAGE, EM_REG_BEFORE, 0, FALSE, "EM_ROUTERJOURNALMESSAGE"},
	{EM_SMTPCONNECT, EM_REG_BEFORE, 0, FALSE, "EM_SMTPCONNECT"},
	{EM_SMTPCOMMAND, EM_REG_BEFORE, 0, FALSE, "EM_SMTPCOMMAND"},
	{EM_SMTPMESSAGEACCEPT, EM_REG_BEFORE, 0, FALSE, "EM_SMPTMESSAGEACCEPT"},
	{EM_SMTPDISCONNECT, EM_REG_BEFORE, 0, FALSE, "EM_SMTPDISCONNECT"},
	{EM_NSFARCHIVECOPYNOTES, EM_REG_BEFORE, 0, FALSE, "EM_NSFARCHIVECOPYNOTES"},
	{EM_NSFARCHIVEDELETENOTES, EM_REG_BEFORE, 0, FALSE, "EM_NSFARCHIVEDELETENOTES"},
	{EM_NSFNOTEEXTRACTWITHCALLBACK, EM_REG_BEFORE, 0, FALSE, "EM_NSFNOTEEXTRACTWITHCALLBACK"},

	/*            Here down is all EM_REG_AFTER                     */


	{EM_NSFDBCLOSESESSION, EM_REG_AFTER, 0, FALSE, "EM_NSFDBCLOSESESSION"},
	{EM_NSFDBCLOSE, EM_REG_AFTER, 0, FALSE, "EM_NSFDBCLOSE"},
	{EM_NSFDBCREATE, EM_REG_AFTER, 0, FALSE, "EM_NSFDBCREATE"},
	{EM_NSFDBDELETE, EM_REG_AFTER, 0, FALSE, "EM_NSFDBDELETE"},
	{EM_NSFNOTEOPEN, EM_REG_AFTER, 0, FALSE, "EM_NSFNOTEOPEN"},
	{EM_NSFNOTECLOSE, EM_REG_AFTER, 0, FALSE, "EM_NSFNOTECLOSE"},
	{EM_NSFNOTECREATE, EM_REG_AFTER, 0, FALSE, "EM_NSFNOTECREATE"},
	{EM_NSFNOTEDELETE, EM_REG_AFTER, 0, FALSE, "EM_NSFNOTEDELETE"},
	{EM_NSFNOTEOPENBYUNID, EM_REG_AFTER, 0, FALSE, "EM_NSFNOTEOPENBYUNID"},
	{EM_FTGETLASTINDEXTIME, EM_REG_AFTER, 0, FALSE, "EM_FTGETLASTINDEXTIME"},
	{EM_FTINDEX, EM_REG_AFTER, 0, FALSE, "EM_FTINDEX"},
	{EM_FTSEARCH, EM_REG_AFTER, 0, FALSE, "EM_FTSEARCH"},
	{EM_NIFFINDBYKEY, EM_REG_AFTER, 0, FALSE, "EM_NIFFINDBYKEY"},
	{EM_NIFFINDBYNAME, EM_REG_AFTER, 0, FALSE, "EM_NIFFINDBYNAME"},
	{EM_NIFREADENTRIES, EM_REG_AFTER, 0, FALSE, "EM_NIFREADENTRIES"},
	{EM_NIFUPDATECOLLECTION, EM_REG_AFTER, 0, FALSE, "EM_NIFUPDATECOLLECTION"},
	{EM_NSFDBALLOCOBJECT, EM_REG_AFTER, 0, FALSE, "EM_NSFDBALLOCOBJECT"},
	{EM_NSFDBCOMPACT, EM_REG_AFTER, 0, FALSE, "EM_NSFDBCOMPACT"},
	{EM_NSFDBDELETENOTES, EM_REG_AFTER, 0, FALSE, "EM_NSFDBDELETENOTES"},
	{EM_NSFDBFREEOBJECT, EM_REG_AFTER, 0, FALSE, "EM_NSFDBFREEOBJECT"},
	{EM_NSFDBGETMODIFIEDNOTETABLE, EM_REG_AFTER, 0, FALSE, "EM_NSFDBGETMODIFIEDNOTETABLE"},
	{EM_NSFDBGETNOTEINFO, EM_REG_AFTER, 0, FALSE, "EM_NSFDBGETNOTEINFO"},
	{EM_NSFDBGETNOTEINFOBYUNID, EM_REG_AFTER, 0, FALSE, "EM_NSFDBGETNOTEINFOBYUNID"},
	{EM_NSFDBGETOBJECTSIZE, EM_REG_AFTER, 0, FALSE, "EM_NSFDBGETOBJECTSIZE"},
	{EM_NSFDBGETSPECIALNOTEID, EM_REG_AFTER, 0, FALSE, "EM_NSFDBGETSPECIALNOTEID"},
	{EM_NSFDBINFOGET, EM_REG_AFTER, 0, FALSE, "EM_NSFDBINFOGET"},
	{EM_NSFDBINFOSET, EM_REG_AFTER, 0, FALSE, "EM_NSFDBINFOSET"},
	{EM_NSFDBLOCATEBYREPLICAID, EM_REG_AFTER, 0, FALSE, "EM_NSFDBLOCATEBYREPLICAID"},
	{EM_NSFDBMODIFIEDTIME, EM_REG_AFTER, 0, FALSE, "EM_NSFDBMODIFIEDTIME"},
	{EM_NSFDBREADOBJECT, EM_REG_AFTER, 0, FALSE, "EM_NSFDBREADOBJECT"},
	{EM_NSFDBREALLOCOBJECT, EM_REG_AFTER, 0, FALSE, "EM_NSFDBREALLOCOBJECT"},
	{EM_NSFDBREPLICAINFOGET, EM_REG_AFTER, 0, FALSE, "EM_NSFDBREPLICAINFOGET"},
	{EM_NSFDBREPLICAINFOSET, EM_REG_AFTER, 0, FALSE, "EM_NSFDBREPLICAINFOSET"},
	{EM_NSFDBSPACEUSAGE, EM_REG_AFTER, 0, FALSE, "EM_NSFDBSPACEUSAGE"},
	{EM_NSFDBSTAMPNOTES, EM_REG_AFTER, 0, FALSE, "EM_NSFDBSTAMPNOTES"},
	{EM_NSFDBWRITEOBJECT, EM_REG_AFTER, 0, FALSE, "EM_NSFDBWRITEOBJECT"},
	{EM_NSFNOTEUPDATE, EM_REG_AFTER, 0, FALSE, "EM_NSFNOTEUPDATE"},
	{EM_NIFOPENCOLLECTION, EM_REG_AFTER, 0, FALSE, "EM_NIFOPENCOLLECTION"},
	{EM_NIFCLOSECOLLECTION, EM_REG_AFTER, 0, FALSE, "EM_NIFCLOSECOLLECTION"},
	{EM_NSFDBGETBUILDVERSION, EM_REG_AFTER, 0, FALSE, "EM_NSFDBGETBUILDVERSION"},
	{EM_NSFDBRENAME, EM_REG_AFTER, 0, FALSE, "EM_NSFDBRENAME"},
	{EM_NSFDBITEMDEFTABLE, EM_REG_AFTER, 0, FALSE, "EM_NSFDBITEMDEFTABLE"},
	{EM_NSFDBREOPEN, EM_REG_AFTER, 0, FALSE, "EM_NSFDBREOPEN"},
	{EM_NSFDBOPENEXTENDED, EM_REG_AFTER, 0, FALSE, "EM_NSFDBOPENEXTENDED"},
	{EM_NSFNOTEDECRYPT, EM_REG_AFTER, 0, FALSE, "EM_NSFNOTEDECRYPT"},
	{EM_NSFCONFLICTHANDLER, EM_REG_AFTER, 0, FALSE, "EM_NSFCONFLICTHANDLER"},
	{EM_NSFNOTEUPDATEXTENDED, EM_REG_AFTER, 0, FALSE, "NSFNOTEUPDATEXTENDED"},
	{EM_SCHFREETIMESEARCH, EM_REG_AFTER, 0, FALSE, "SCHFREETIMESEARCH"},
	{EM_SCHRETRIEVE, EM_REG_AFTER, 0, FALSE, "SCHRETRIEVE"},
	{EM_SCHSRVRETRIEVE, EM_REG_AFTER, 0, FALSE, "SCHSRVRETRIEVE"},
	{EM_NSFDBCOMPACTEXTENDED, EM_REG_AFTER, 0, FALSE, "EM_NSFDBCOMPACTEXTENDED"},
	{EM_ADMINPPROCESSREQUEST, EM_REG_AFTER, 0, FALSE, "EM_ADMINPPROCESSREQUEST"},
	{EM_NIFGETCOLLECTIONDATA, EM_REG_AFTER, 0, FALSE, "NIFGETCOLLECTIONDATA"},
	{EM_NSFDBCOPYNOTE, EM_REG_AFTER, 0, FALSE, "NSFDBCOPYNOTE"},
	{EM_NSFNOTECOPY, EM_REG_AFTER, 0, FALSE, "NSFNOTECOPY"},
	{EM_NSFNOTEATTACHFILE, EM_REG_AFTER, 0, FALSE, "NSFNOTEATTACHFILE"},
	{EM_NSFNOTEDETACHFILE, EM_REG_AFTER, 0, FALSE, "NSFNOTEDETACHFILE"},
	{EM_NSFNOTEEXTRACTFILE, EM_REG_AFTER, 0, FALSE, "NSFNOTEEXTRACTFILE"},
	{EM_NSFNOTEATTACHOLE2OBJECT, EM_REG_AFTER, 0, FALSE, "NSFNOTEATTACHOLE2OBJECT"},
	{EM_NSFNOTEDELETEOLE2OBJECT, EM_REG_AFTER, 0, FALSE, "NSFNOTEDELETEOLE2OBJECT"},
	{EM_NSFNOTEEXTRACTOLE2OBJECT, EM_REG_AFTER, 0, FALSE, "NSFNOTEEXTRACTOLE2OBJECT"},
	{EM_NSGETSERVERLIST, EM_REG_AFTER, 0, FALSE, "NSGETSERVERLIST"},
	{EM_NSFDBCOPY, EM_REG_AFTER, 0, FALSE, "NSFDBCOPY"},
	{EM_NSFDBCREATEANDCOPY, EM_REG_AFTER, 0, FALSE, "NSFDBCREATEANDCOPY"},
	{EM_NSFDBCOPYACL, EM_REG_AFTER, 0, FALSE, "NSFDBCOPYACL"},
	{EM_NSFDBCOPYTEMPLATEACL, EM_REG_AFTER, 0, FALSE, "NSFDBCOPYTEMPLATEACL"},
	{EM_NSFDBCREATEACLFROMTEMPLATE, EM_REG_AFTER, 0, FALSE, "NSFDBCREATEACLFROMTEMPLATE"},
	{EM_NSFDBREADACL, EM_REG_AFTER, 0, FALSE, "NSFDBREADACL"},
	{EM_NSFDBSTOREACL, EM_REG_AFTER, 0, FALSE, "NSFDBSTOREACL"},
	{EM_NSFDBFILTER, EM_REG_AFTER, 0, FALSE, "NSFDBFILTER"},
	{EM_FTDELETEINDEX, EM_REG_AFTER, 0, FALSE, "FTDELETEINDEX"},
	{EM_NSFNOTEGETINFO, EM_REG_AFTER, 0, FALSE, "NSFNOTEGETINFO"},
	{EM_NSFNOTESETINFO, EM_REG_AFTER, 0, FALSE, "NSFNOTESETINFO"},
	{EM_NSFNOTECOMPUTEWITHFORM, EM_REG_AFTER, 0, FALSE, "NSFNOTECOMPUTEWITHFORM"},
	{EM_NIFFINDDESIGNNOTE, EM_REG_AFTER, 0, FALSE, "NIFFINDDESIGNNOTE"},
	{EM_NIFFINDPRIVATEDESIGNNOTE, EM_REG_AFTER, 0, FALSE, "NIFFINDPRIVATEDESIGNNOTE"},
	{EM_NIFGETLASTMODIFIEDTIME, EM_REG_AFTER, 0, FALSE, "NIFGETLASTMODIFIEDTIME"},
	{EM_FTSEARCHEXT, EM_REG_AFTER, 0, FALSE, "FTSEARCHEXT"},
	{EM_NAMELOOKUP, EM_REG_AFTER, 0, FALSE, "NAMELOOKUP"},
	{EM_NSFNOTEUPDATEMAILBOX, EM_REG_AFTER, 0, FALSE, "NSFNOTEUPDATEMAILBOX"},
	{EM_NIFFINDDESIGNNOTEEXT, EM_REG_AFTER, 0, FALSE, "NIFFINDDESIGNNOTEEXT"},
	{EM_AGENTOPEN, EM_REG_AFTER, 0, FALSE, "AGENTOPEN"},
	{EM_AGENTCLOSE, EM_REG_AFTER, 0, FALSE, "AGENTCLOSE"},
	{EM_AGENTISENABLED, EM_REG_AFTER, 0, FALSE, "AGENTISENABLED"},
	{EM_AGENTCREATERUNCONTEXT, EM_REG_AFTER, 0, FALSE, "AGENTCREATERUNCONTEXT"},
	{EM_AGENTDESTROYRUNCONTEXT, EM_REG_AFTER, 0, FALSE, "AGENTDESTROYRUNCONTEXT"},
	{EM_AGENTSETDOCUMENTCONTEXT, EM_REG_AFTER, 0, FALSE, "AGENTSETDOCUMENTCONTEXT"},
	{EM_AGENTSETTIMEEXECUTIONLIMIT, EM_REG_AFTER, 0, FALSE, "AGENTSETTIMEEXECUTIONLIMIT"},
	{EM_AGENTQUERYSTDOUTBUFFER, EM_REG_AFTER, 0, FALSE, "AGENTQUERYSTDOUTBUFFER"},
	{EM_AGENTREDIRECTSTDOUT, EM_REG_AFTER, 0, FALSE, "AGENTREDIRECTSTDOUT"},
	{EM_NAMELOOKUP2, EM_REG_AFTER, 0, FALSE, "NAMELOOKUP2"},
	{EM_NSFNOTEOPENEXTENDED, EM_REG_AFTER, 0, FALSE, "EM_NSFNOTEOPENEXTENDED"},
	{EM_TERMINATENSF, EM_REG_AFTER, 0, FALSE, "EM_TERMINATENSF"},
	{EM_MAILSENDNOTE, EM_REG_AFTER, 0, FALSE, "EM_MAILSENDNOTE"},
	{EM_AGENTRUN, EM_REG_AFTER, 0, FALSE, "EM_AGENTRUN"},
	{EM_SECAUTHENTICATION, EM_REG_AFTER, 0, FALSE, "EM_SECAUTHENTICATION"},
	{EM_NSFMARKREAD, EM_REG_AFTER, 0, FALSE, "EM_NSFMARKREAD"},
	{EM_NSFADDTOFOLDER, EM_REG_AFTER, 0, FALSE, "EM_NSFADDTOFOLDER"},
	{EM_NSFDBSPACEUSAGESCALED, EM_REG_AFTER, 0, FALSE, "EM_NSFDBSPACEUSAGESCALED"},
	{EM_NSFDBGETMAJMINVERSION, EM_REG_AFTER, 0, FALSE, "EM_NSFDBGETMAJMINVERSION"},
	{EM_ROUTERJOURNALMESSAGE, EM_REG_AFTER, 0, FALSE, "EM_ROUTERJOURNALMESSAGE"},
	{EM_SMTPCONNECT, EM_REG_AFTER, 0, FALSE, "EM_SMTPCONNECT"},
	{EM_SMTPCOMMAND, EM_REG_AFTER, 0, FALSE, "EM_SMTPCOMMAND"},
	{EM_SMTPMESSAGEACCEPT, EM_REG_AFTER, 0, FALSE, "EM_SMPTMESSAGEACCEPT"},
	{EM_SMTPDISCONNECT, EM_REG_AFTER, 0, FALSE, "EM_SMTPDISCONNECT"},
	{EM_NSFARCHIVECOPYNOTES, EM_REG_AFTER, 0, FALSE, "EM_NSFARCHIVECOPYNOTES"},
	{EM_NSFARCHIVEDELETENOTES, EM_REG_AFTER, 0, FALSE, "EM_NSFARCHIVEDELETENOTES"},
	{EM_NSFNOTEEXTRACTWITHCALLBACK, EM_REG_AFTER, 0, FALSE, "EM_NSFNOTEEXTRACTWITHCALLBACK"},
	{EM_NSFDBSTAMPNOTESMULTIITEM, EM_REG_AFTER, 0, FALSE, "EM_NSFDBSTAMPNOTESMULTIITEM"},				  										
	{EM_MEDIARECOVERY_NOTE, EM_REG_AFTER, 0, FALSE, "EM_MEDIARECOVERY_NOTE"},							  									
	{EM_NSFGETCHANGEDDBS, EM_REG_AFTER, 0, FALSE, "EM_NSFGETCHANGEDDBS"},								  									
	{EM_NSFDBMODIFIEDTIMEBYNAME, EM_REG_AFTER, 0, FALSE, "EM_NSFDBMODIFIEDTIMEBYNAME"},															
	{EM_NSFGETDBCHANGES, EM_REG_AFTER, 0, FALSE, "EM_NSFGETDBCHANGES"},																	
	{EM_NSFNOTECIPHERDECRYPT, EM_REG_AFTER, 0, FALSE, "EM_NSFNOTECIPHERDECRYPT"},						  										
	{EM_NSFNOTECIPHEREXTRACTFILE, EM_REG_AFTER, 0, FALSE, "EM_NSFNOTECIPHEREXTRACTFILE"},				  											
	{EM_NSFNOTECIPHEREXTRACTWITHCALLBACK, EM_REG_AFTER, 0, FALSE, "EM_NSFNOTECIPHEREXTRACTWITHCALLBACK"}, 
	{EM_NSFNOTECIPHEREXTRACTOLE2OBJECT, EM_REG_AFTER, 0, FALSE, "EM_NSFNOTECIPHEREXTRACTOLE2OBJECT"},   
	{EM_NSFNOTECIPHERDELETEOLE2OBJECT, EM_REG_AFTER, 0, FALSE, "EM_NSFNOTECIPHERDELETEOLE2OBJECT"},    
	{EM_NSFDBNOTELOCK, EM_REG_AFTER, 0, FALSE, "EM_NSFDBNOTELOCK"},												
	{EM_NSFDBNOTEUNLOCK, EM_REG_AFTER, 0, FALSE, "EM_NSFDBNOTEUNLOCK"},	

	{0,  0,  0, 0, NULL}
};
/************************* End of ExtensionHookTable ************************
*****************************************************************************/


/*===== LOCAL FUNCTION PROTOTYES ======================================*/

DLL_EXPORT_PREFIX STATUS LNPUBLIC MainEntryPoint( void );

#if defined (NT)

BOOL WINAPI DllMain( HINSTANCE hInstance, DWORD fdwReason,
	LPVOID lpReserved );
#endif


DLL_EXPORT_PREFIX  STATUS LNPUBLIC EMHandlerProc( EMRECORD FAR * pExRecord);
void CleanUp( void );
STATUS GetDBTitle(DBHANDLE hDb, char *title);

/*
* LogLine: Attempts to write a line to the output.log
*
*   in: Line => The Line to log
*
*   out: If gFStream is not NULL an attempt will be made to
*        ship out the line. If that fails gFStream will
*        be dropped.
*/
void LogLine(char *Line)
{
	if (gFStream) {
	    __WriteLine(Line, gFStream);
	    if (__FileError(gFStream)) {
	        __CloseFile(gFStream);
	        gFStream = (__FILE *)0;
	    }
	}
}

/*
* RegisterEntry: Attempts to Register one entry.
*
*   in: x = Offset into table
*
*   out: 0 if ok, returned error otherwise
*/
STATUS RegisterEntry(int x)
{
	STATUS error = NOERROR;

	if (ExtensionHookTable[x].m_Name == NULL) return(error);

	//LogLine("-------------------------------------------------\n");

	sprintf(gTextBuffer, "EMRegister with %s \n",
            ExtensionHookTable[x].m_Name );
	LogLine( gTextBuffer );

	error = EMRegister( ExtensionHookTable[x].m_Identifier,
	                    ExtensionHookTable[x].m_Notification,
	                    (EMHANDLER)gHandlerProc,
	                    gRecursionID,
	                    &ExtensionHookTable[x].m_RegistrationHandle);

	if (error) {
	    LogLine("EMRegister Failed\n");
	    return(error);
	}
	//LogLine("-------------------------------------------------\n");

	return(error);
}

/*
* DeregisterEntry: Attempts to Deregister one entry.
*
*   in: x = Offset into table
*
*   out: 0 if ok, returned error otherwise
*/
STATUS DeregisterEntry(int x)
{
	STATUS error = NOERROR;
	if (gFStream)
	    __CloseFile(gFStream);
	gFStream = __OpenFile(FileName, APPEND_PERMISSION);
	if (ExtensionHookTable[x].m_Name == NULL) return(error);


	//LogLine("-------------------------------------------------\n");

	sprintf(gTextBuffer, "Calling EMDeregister with %s\n",
	        ExtensionHookTable[x].m_Name);
	LogLine( gTextBuffer );

	error = EMDeregister(ExtensionHookTable[x].m_RegistrationHandle);

	if (error) {
	    LogLine("EMDregister Failed\n");
	        if (gFStream) __CloseFile(gFStream);
	            gFStream = (__FILE *)0;
	    return(error);
	}
	//LogLine("-------------------------------------------------\n");
	__CloseFile(gFStream);

	return(error);
}

DLL_EXPORT_PREFIX STATUS LNPUBLIC MainEntryPoint( void )
{
	STATUS    error;
	WORD      x;

	error = NOERROR;

	//__asm int 3;
	/*
	*    When run on a server the dll is called multiple times. the flag
	*    keeps the main code from being executed more than once.
	*/
	if ( gHooksRegistered )
	    return(NOERROR);
	gHooksRegistered = TRUE;


	/* Open log __FILE for DLL. */

	x = OSGetDataDirectory(DataDir);
	strcpy(FileName, DataDir);

	strcat(FileName, LOG_FILE_NAME);

	gFStream = __OpenFile( FileName, WRITE_PERMISSION );

	/* Get proc instance for the handler callback. */
#if defined (NT)
	/* Done in the DllMain function at startup. */
	//gHandlerProc = EMHandlerProc;
#else
	gHandlerProc = EMHandlerProc;
#endif

	/*  Next get a recursion ID  for the run */

	LogLine("-------------------------------------------------\n");
	LogLine("Calling EMCreateRecursionID\n");
    fflush(gFStream);

	error = EMCreateRecursionID( &gRecursionID );

	if (error)
	    LogLine("EMCreateRecursionID Failed\n");

	/* Loop through the table testing each entry */

	else {
	    LogLine("EMCreateRecursionID Returned Success\n");

	    for ( x = 0; ExtensionHookTable[x].m_Name != NULL; x += 1 ) {
	        error = RegisterEntry(x);
	        if (error)
	            break;
	    }
	}
	if ( gFStream ) {
	    __CloseFile(gFStream);
	    gFStream = (__FILE *)0;
	}
	return( error );
}

/*==========================================================================*/

DLL_EXPORT_PREFIX STATUS LNPUBLIC EMHandlerProc( EMRECORD FAR * pExRecord )
{

	STATUS error = 0;
	DBRECORD *pDBHead=0;
	DBRECORD *pDBNext=0;
	static int inHook=0;
	static int inHookNoteClose=0;

	LogLine("In EMHandlerProc");

	switch (pExRecord->EId)
	{
	    case EM_NSFDBCREATE:
	    {
	        char far *DBName;
	        USHORT DbClass;
	        BOOL Force;
	        VARARG_PTR ap;
	    
	        char PathName[256];
	
	        /* get arguments */
	        ap = pExRecord->Ap;
	        DBName = VARARG_GET (ap, char far *);
	        DbClass = VARARG_GET (ap, USHORT);
	        Force = VARARG_GET (ap, BOOL);
		
	        /* check error code */
	        if (pExRecord->Status != NOERROR) {
	            sprintf(gTextBuffer, "Error Status[%d]\n",pExRecord->Status);
	            LogLine( gTextBuffer);
	            fflush(gFStream);
	            break;
	        }
		
	        if (pExRecord->NotificationType != EM_AFTER)
	            break;
		
	        /* construct database path name */
	        strcpy(PathName,DataDir);
	        #if defined(OS400)
	            strcat(PathName,"/");
	        #else
	            strcat(PathName,"\\");
	        #endif
	            strcat(PathName,"animals.nsf");
	    
	        /* if this isn't the database we're looking for then break */
	        if (strcmp(DBName, PathName) && strcmp(DBName,"animals.nsf"))
	            break;
		
	        inHook = 0; /* init tracking routine cnt to zero on creation */
		
	        inHook += 1;
		
	        LogLine("-------------------------------------------------\n");
		
	        /* read the non-Domino database and parse info into records */ 
	        LogLine("Calling ReadDataBase \n");
	        error = ReadDataBase(&pDBRecord);
	        pDBHead = pDBRecord;
		
	        if (error)
	        {
	            sprintf(gTextBuffer, "Read Database Error: %d\n",error);
	            LogLine( gTextBuffer );
	            goto FreeMem;
	        }
		
	        /* create the Domino and Notes form in the Domino database */
	        error = CreateDBForm(pDBHead);
	        if (error)
	        {
	            sprintf(gTextBuffer, "Create Database Form Error: %d\n",error);
	            LogLine( gTextBuffer );
	            goto FreeMem;
	        }
		
	        /* create all the notes from the data records */
	         error = CreateDBNotes(pDBHead);

	        if (error)
	        {
	            sprintf(gTextBuffer, "Create Database Note Error: %d\n",error);
	            LogLine( gTextBuffer );
	            goto FreeMem;
	        }
	    
	        /* create the view in the Domino database */
	        error = CreateDBView();
	        if (error)
	        {
	            sprintf(gTextBuffer, "Create Database View Error: %d\n",error);
	            LogLine( gTextBuffer );
	            goto FreeMem;
	        }

	        LogLine("-------------------------------------------------\n");
	        LogLine("Done create DB \n");
	        fflush(gFStream);

	        FreeMem:
	        
	            /* free up memory */
	            if (pDBHead)
	            {
	                pDBRecord = pDBHead;
	                while (pDBRecord != NULL)
	                {
	                    pDBNext = pDBRecord->Next;
	                    __FreeMemory(pDBRecord);
	                    pDBRecord = pDBNext;
	                }
	                pDBHead = 0;
	            }
	            break;
	    }
		
	    /* before the Database is closed, create all forms, notes, and views */
	    case EM_NSFDBCLOSE:
	    {
	        DHANDLE hDb;
	        VARARG_PTR ap;
	        char title [MAXSPRINTF];
		
	        /* get arguments */
	        ap = pExRecord->Ap;
	        hDb = VARARG_GET(ap,DHANDLE);
	    
	        /* check error code */
	        if (pExRecord->Status != NOERROR)
	            break;
		
	        if (pExRecord->NotificationType != EM_BEFORE)
	            break;
		
	        /* Only do this for our sample DB */
	        if (error = GetDBTitle (hDb, title))
	            break;
		
	        if (strcmp(title, "animals"))
	            break;
		
	        LogLine("-------------------------------------------------\n");
		
	        sprintf(gTextBuffer, "Calling EM_NSFDBCLOSE for Database: %s\n",title);
	        LogLine( gTextBuffer );
	        LogLine("-------------------------------------------------\n");
	        break;
	    } 

	//case EM_NSFDBCREATE:
	//case EM_NSFNOTEOPEN:
	//case EM_NSFDBCLOSE:
	//case EM_NSFARCHIVECOPYNOTES:
	//	{
	//		LogLine("EM_NSFARCHIVECOPYNOTES");
	//	}
	}

	return( ERR_EM_CONTINUE );
}


/*===========================================================================

void CleanUp(void)

===========================================================================*/
void CleanUp(void)
{
	gHooksRegistered = FALSE;
}


/*===========================================================================

STATUS GetDBTitle(DHANDLE, char *)

===========================================================================*/
STATUS GetDBTitle(DHANDLE hDb, char *title)
{

	STATUS error = NOERROR;
	char dbinfo[NSF_INFO_SIZE];
	char the_title[MAXSPRINTF];

	if (error = NSFDbInfoGet (hDb, dbinfo))
	    return(error);

	NSFDbInfoParse(dbinfo, INFOPARSE_TITLE, the_title, sizeof(the_title));
	strcpy(title, &the_title[0]);

	return(error);

}


/* ========================================================= */
/* =================== STARTUP FUNCTIONS =================== */
/* ========================================================= */

#if defined (NT)
/*===========================================================================

Startup and Shutdown Function DllMain for Windows NT
----------------------------------------------------

BOOL WINAPI DllMain( HINSTANCE hInstance, DWORD fdwReason,
LPVOID lpReserved )

OVERVIEW:
---------
Standard windows NT DLL entrypoint, does initialzation required to get the
FARPROC for the Extension Manager callback function.

===========================================================================*/
BOOL WINAPI DllMain( HINSTANCE hInstance, DWORD fdwReason,
	LPVOID lpReserved )
{
	int x;
	STATUS error=NOERROR;

	//__asm int 3;
	switch(fdwReason)
	{
	    case DLL_PROCESS_ATTACH:
	        InitializeCriticalSection(&gCriticalSection);

	        gHandlerProc = (EMHANDLER)MakeProcInstance((FARPROC)EMHandlerProc,
	        hInstance);
	        break;

	    case DLL_PROCESS_DETACH:
	        CleanUp();

	        /* Free procedure instance */
	        FreeProcInstance( gHandlerProc );

	        /* Deregister Extension Manager routines */
	        for ( x = 0; ExtensionHookTable[x].m_Name != NULL; x += 1 )
	        {
	            error = DeregisterEntry(x);
	            if (error)
	                break;
		    }

	        DeleteCriticalSection(&gCriticalSection);

	        break;
	}

	return( TRUE );

	UNREFERENCED_PARAMETER(lpReserved);
}

#endif


/* ========================================================== */
/* =================== SHUTDOWN FUNCTIONS =================== */
/* ========================================================== */
#if defined (NT)
/*
NOTE:
-----
SHUTDOWN PROCESSES ARE DONE IN THE DLLMAIN FUNCTION IN NT
*/
#endif

