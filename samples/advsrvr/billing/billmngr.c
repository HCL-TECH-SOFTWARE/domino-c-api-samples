/***********************************************************************
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

  PROGRAM:    billmngr.dll
  
  FILE:       billmngr.c
  
  Billing Sample Program Extension Manager routines (NT Server Only)
  
  OVERVIEW:
  
  This program uses some of the Extension Manager functions and interacts 
  with the BILLSES sample addin via the Billing Message Queue in order to
  handle the billing of database events.  Specifically, it tracks all Database
  note creations on the current server.

  The routine BillHandler() is the event handler for the NSFNoteUpdateExtended
  routine.  If the Database billing class is enabled for the server, the event
  handler bills successful NSFNoteUpdateExtended calls for new notes by writing
  a NOTECREATEREC Billing record to the billing message queue.  Once registered
  with the extension manager, the event handler will be called by Domino and Notes 
  for every NSFNoteUpdateExtended.

  The routine BillRegister() creates an extension manager Recursion ID and 
  registers the Database Event Handlers. This routine is called by (and
  essentially is) the MainEntryPoint() routine of the program.

  The routine BillDeregister() deregisters the Database Event Handlers. This 
  routine is called by the DLL_PROCESS_DETACH section of the DllMain() routine
  of the program.

******************************************************************************/ 
#if defined(OS400)
#pragma convert(850)
#endif

#ifndef UNIX
#include <windows.h>
#endif

#include <string.h>
#include <stdio.h>
#include <string.h>

#include <global.h>
#include <nsfnote.h>
#include <nsferr.h>
#include <nsfdb.h>
#include <nsfdata.h>
#include <extmgr.h>
#include <osmisc.h>
#include <names.h>
#include <kfm.h>
#include <osenv.h>

#include "bill_ext.h"  /* extended Billing header for NOTECREATEREC */

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

      
/*===== GLOBAL VARIABLES =============================*/

#ifndef UNIX
EMHANDLER       gBillHandler;      /* Billing EM handler */
#endif

BOOL            gHooksRegistered=FALSE;

WORD            gRecursionID;	   /* EM Recursion ID and Reg Handles for */
HEMREGISTRATION gRegHandle1;       /* NFSNoteUpdateExtended (before call) */
HEMREGISTRATION gRegHandle2;  	   /* NFSNoteUpdateExtended (after call) */

#ifndef UNIX
/* Storage needed for synchronization under Windows NT */
CRITICAL_SECTION    gCriticalSection;
#endif

/* Note Create Billing flag */
BOOL    gCreatedNote;

/*===== LOCAL FUNCTION PROTOTYES ======================================*/
 
#ifndef UNIX
BOOL WINAPI DllMain( HINSTANCE hInstance, DWORD fdwReason, LPVOID lpReserved );
#endif

STATUS LNPUBLIC MainEntryPoint( void );
STATUS LNPUBLIC BillHandler ( EMRECORD FAR * theData );

STATUS LNPUBLIC BillRegister ( void );
STATUS LNPUBLIC BillDeregister ( void );

/*===========================================================================
   STATUS LNPUBLIC MainEntryPoint ( void )
    
   MainEntryPoint that is called after Domino and Notes loads this DLL at startup.
   It calls the local function BillRegister, to register with the Domino and Notes
   Extension Manager
===========================================================================*/ 

STATUS FAR PASCAL MainEntryPoint( void )
{
   STATUS    error = NOERROR;

/* call BillRegister and return */
   error = BillRegister();
   return( error );

}


/*===========================================================================
   STATUS LNPUBLIC BillRegister ( void )
    
   Creates Extension Manager Recursion ID and registers with Domino and Notes the 
   NSFNoteUpdateExtended event handlers.
===========================================================================*/ 

STATUS LNPUBLIC BillRegister ( void )
{
   STATUS error = NOERROR;

/* When run on a server the dll is called multiple times. ths flag 
   keeps the main code from being executed more than once. */  

   if ( gHooksRegistered )
       return(NOERROR);

/* If first time, get a recursion ID for the run, and register the 
   extension manager handler functions for NSFNoteCreate */

   else
   {
       gHooksRegistered = TRUE;
       error = EMCreateRecursionID( &gRecursionID );

#ifndef UNIX
       if (error == NOERROR )
       {
           error = EMRegister( EM_NSFNOTEUPDATEXTENDED,    /* before NFSNoteUpdateExtended */
                               EM_REG_BEFORE, 
                               (EMHANDLER)gBillHandler,
                               gRecursionID,
                               &gRegHandle1 );
       }               
       if (error == NOERROR )
       {
           error = EMRegister( EM_NSFNOTEUPDATEXTENDED,    /* after NFSNoteUpdateExtended */
                               EM_REG_AFTER, 
                               (EMHANDLER)gBillHandler,
                               gRecursionID,
                               &gRegHandle2 );
       }               
#else
       if (error == NOERROR )
       {
           error = EMRegister( EM_NSFNOTEUPDATEXTENDED,    /* before NFSNoteUpdateExtended */
                               EM_REG_BEFORE, 
                               BillHandler,
                               gRecursionID,
                               &gRegHandle1 );
       }               
       if (error == NOERROR )
       {
           error = EMRegister( EM_NSFNOTEUPDATEXTENDED,    /* after NFSNoteUpdateExtended */
                               EM_REG_AFTER, 
                               BillHandler,
                               gRecursionID,
                               &gRegHandle2 );
       }               
#endif
   }

   return( error );
}


/*===========================================================================
  STATUS LNPUBLIC BillDeregister ( void )
    
   Deregisters NSFNoteUpdateExtended Event Handlers 
===========================================================================*/ 

STATUS LNPUBLIC BillDeregister ( void )
{
   STATUS error;

   error = EMDeregister( gRegHandle1 );
   if ( error == NOERROR )
       error = EMDeregister ( gRegHandle2 );

   return( error );
}


/*===========================================================================
  STATUS LNPUBLIC BillHandler ( EMRECORD FAR * )
    
   The event handler for NSFNoteUpdateExtended.  For every NFSNoteUpdateExtended
   call for a new note, this routine sends an appropriate user-extended Billing
   record to the Billing message queue.  Billing messages will only be written if
    1) the database API was successful, 
    2) the Database Billing Class is enabled for the server (in Notes.ini),
    3) the Note ID of the input Notehandle (NSFNoteUpdateExtended) is 0, or a new note,
    4) the NSFNoteUpdateExtended call is not initiated by the server.

   The following Billing API routines are used by this routine:
      BillingGetClass,  BillingWrite

   The following HCL C API for Notes/Domino routines are used by this routine:
      NSFNoteGetInfo, NSFDbIdGet, NFSDbUserNameGet, SECKFMGetUserName
 
===========================================================================*/ 

STATUS LNCALLBACK BillHandler ( EMRECORD FAR * theData )
{ 
   
   STATUS     error = NOERROR;
   BILLMSG    BillMsg;
   VARARG_PTR argPtr;

   NOTEHANDLE hNote;                        /* NSFNoteUpdateExtended argument */
   DHANDLE      hDb;                          /* Associated DB handle */
   char       Username[MAXUSERNAME+1];      /* Note create user */  
   NOTEID     NoteId;                       /* NOTEID of note */
   DBID       DbId;                         /* DBID of note */
   char       Servername[MAXUSERNAME+1];    /* Server name */  
   DWORD      BillClass; 

/* Check to see if Database billing class is enabled. If not, return without
   sending billing record */

   if (error = BillingGetClass( &BillClass ))

/* Only bill if the API was successful. If not, return without sending 
   billing record. */
     
       goto Done;

   if ((BOOL)(BillClass & BILL_CLASS_DATABASE))
   {

       if ( theData->Status != NOERROR )
           goto Done;
      
/* otherwise handle Note Create billing by interpreting NSFNoteUpdateExtended calls */
      
      //argPtr = theData->Ap;

/* Before the API:  check to see if the Note ID is equal to zero.   If it is 
   then this means a new note is being created and we should track the output. */
   
       if ( theData->NotificationType == EM_BEFORE )
       {
           hNote = VARARG_GET (theData->Ap, DHANDLE);   /* save note handle */
           (void) VARARG_GET (theData->Ap, DWORD);      /* skip update flags */
            
   /* get NOTEID of input note handle */
           NSFNoteGetInfo(hNote, _NOTE_ID, &NoteId);

   /* if NOTEID = 0, then set for billing */
           if (NoteId == 0)
               gCreatedNote = TRUE;				
           else
               gCreatedNote = FALSE;				
            
           return ( ERR_EM_CONTINUE ); /* continue;, billing occurs after call */
       }

/* If after the call and a new note was created, fill in the UserName, Note ID, 
   and Replica ID of the created note in the billing record. */

/* NOTE: Since a global is used to determine if a new note was created, this 
         logic assumes that the EM_AFTER handling occurs before a different 
         NSFNoteUpdateExtended EM_BEFORE thread is handled by Domino and Notes.  
         For heavily loaded systems, it may be necessary to serialize these requests. */

       if (gCreatedNote == TRUE)
       {
           memset( &(BillMsg.rec.notecreate), (char)0, sizeof( BillMsg.rec.notecreate ) );
           hNote = VARARG_GET (theData->Ap, DHANDLE);   /* save note handle */
           (void) VARARG_GET (theData->Ap, DWORD);      /* skip update flags */

   /* get NOTEID of note handle */
           NSFNoteGetInfo(hNote, _NOTE_ID, &NoteId);

   /* get the DBHANDLE for note handle */
           NSFNoteGetInfo(hNote, _NOTE_DB, &hDb);

   /* get DBID associated with the DBHANDLE */
           NSFDbIDGet(hDb, &DbId);

   /* get USERNAME associated with the DBHANDLE */
           (void) NSFDbUserNameGet(hDb, Username, MAXUSERNAME);

   /* if user is the server, then do not bill */
           (void) SECKFMGetUserName(Servername);
           if (!strcmp(Username, Servername))
               goto Done; 	     

   /* else, set billing message fields with appropriate info*/
           strcpy(BillMsg.rec.notecreate.Username, Username);
           BillMsg.rec.notecreate.dbNoteID = NoteId;  
           BillMsg.rec.notecreate.ReplicaID = DbId;  

   /* and write the billing record */
           error = BillingWrite( (DWORD)BILL_CLASS_DATABASE, 
                                 (WORD)BILL_NOTECREATEREC, 
                                 sizeof( BillMsg ),
                                 &BillMsg,
                                 BILL_QUEUE_NAME );
       }
   }

/* Whether or not the Billing the record was written, return the original
   NSFNoteUpdateExtended (EM_AFTER) status back to Domino and Notes */    

Done:
   if (theData && theData->NotificationType == EM_BEFORE)
       return( ERR_EM_CONTINUE );
   else
       return( theData->Status );

}

#ifndef UNIX
/* ========================================================= */
/* =================== STARTUP FUNCTIONS =================== */
/* ========================================================= */

/*===========================================================================
  BOOL WINAPI DllMain( HINSTANCE hInstance, 
                       DWORD fdwReason, 
                       LPVOID lpReserved )

   Standard windows NT DLL entrypoint.  Performs initialization to get the
   FARPROC for the Extension Manager callback function.
  
===========================================================================*/ 

BOOL WINAPI DllMain( HINSTANCE hInstance, DWORD fdwReason, LPVOID lpReserved )
{

   switch(fdwReason)
   {
       case DLL_PROCESS_ATTACH:
           InitializeCriticalSection( &gCriticalSection );

           gBillHandler = (EMHANDLER)MakeProcInstance( (FARPROC)BillHandler, hInstance );
           break;

       case DLL_PROCESS_DETACH:
           (void) BillDeregister();

           FreeProcInstance( gBillHandler );
    
           DeleteCriticalSection( &gCriticalSection );
           break;
   }

   return( TRUE );

   UNREFERENCED_PARAMETER(lpReserved);
}

/* ========================================================== */
/* =================== SHUTDOWN FUNCTIONS =================== */
/* ========================================================== */
/* 
   SHUTDOWN PROCESSES ARE DONE IN THE DLLMAIN FUNCTION IN NT 
*/
#endif
