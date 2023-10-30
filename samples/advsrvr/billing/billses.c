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

PROGRAM:        BILLSES

FILE:			billses.c

DESCRIPTION:
   A sample HCL Domino Server add-in program that demonstrates the process
   of retrieving HCL Domino Server-generated session and database billing 
   records from the billing message queue.  This program outputs the 
   session billing nformation to a billing database on the server 
   (sessbill.nsf) which is created by the program if it does not exist.  
   This server addin can be loaded on a HCL Domino Server using the console
   command, "load billses".
		
************************************************************************/ 
#if defined(OS400)
#pragma convert(850)
#endif

/* System Includes */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifndef OS400
#include <malloc.h>
#endif

/* Local Includes */
#include "billses.h"

/* Domino and Notes Includes */

#include "global.h"
#include "stdnames.h"
#include "mq.h"
#include "miscerr.h"
#include "misc.h"
#include "addin.h"
#include "ostime.h"
#include "osenv.h"
#include "oserr.h"
#include "nsfdata.h"
#include "nsfnote.h"
#include "nsfdb.h"
#include "nsferr.h"
#include "acl.h"

#include "bill_ext.h"  /* extended billing header for Note creation support */

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif


char BillingTemplateName[]="sessbill.ntf";
char BillingPathName[] = "sessbill.nsf";

/* Function Prototypes */
STATUS LNPUBLIC BillingDBOpen (DHANDLE *hDB);
STATUS LNPUBLIC BillingCreateDB (char *BillingPathName, DHANDLE *RethDb);
STATUS LNPUBLIC BillingMessageToDB (BILLMSG *Message, DHANDLE hDB, DWORD len);

/**************************************************************************

Function:  AddinMain

Description:
   The main entry point of this program.  All HCL Domino Server add-in
   programs must have an AddinMain function.  This is substituted
   for main().  Refer to the HCL C API for Notes/Domino User's Guide 
   for more information regarding this.  This function is responsible for 
   the following:
				
   1) Retrieves values (if any) for the BillingAddinWakeup and
      BillingAddinRunTime variables which would be found in 
      Notes.Ini.  If either of these variables do not exist then
      the default values are used.
					
   2) Call the AddInLogMessageText and AddInSetStatusText API
      functions to indicate on the Domino Server console that the
	   this add-in program is initializing.
	 
   3) Initialize and open the billing message queue with the 
      MQCreate and MQOpen HCL C API for Notes/Domino functions.  
	  This is necessaryto retrieve the billing information from the 
	  message queue.
	    
   4) Open the billing sample database, or creates it if it does 
      not exist.
 
   5) Loop until a quit message has been posted to the message
      queue (via "tell billses quit"), doing the following.
	    
      A) Check to see if the add-in is in an idle state.  If it
         is then call the AddInSetStatusText API function to
         indicate to the server that the add-in is processing.
			
      B) In a loop, call the HCL C API for Notes/Domino function 
	     MQGet to empty the billing message queue one message at a time.  
		 Then call the BillingMessageToDB function and write the
         billing information to the billing database - only if
         it is a session, database, or NoteCreate (user extended)
         billing record. Continue looping until either the message
         queue is empty or there is an error retrieving a message
         from the queue.      
		
      C) Check the elapsed running time of the addin with the
         HCL C API for Notes/Domino function AddinSecondsHaveElapsed.  
		 The wakeup time is specified by the Notes.Ini variable
         "BillingAddinWakeup".  If the number of seconds
         specified has been reached.  Call MQGet again to 
         retrieve the next message in the queue (if any).  If the
         queue is not empty and there was no error then get the
         current time with the HCL C API for Notes/Domino function
         OSGetCurrentTIMEDATE.  Then Increment the time with the
         HCL C API for Notes/Domino function TimeDateIncrement by 
		 the value specified in "BillingAddinRunTime".
    
      D) In a loop, write the billing message to the billing db,
         get the current time/date with OSCurrentTIMEDATE,
         compare the time with the one set in step C above.  If
         the BillingAddinRunTime has been reached then break
         from the loop - even if there are messages to be
         processed.  Otherwise, continue retrieving messages
         from the queue until i) the AddinRunTime is reached,
         ii) the message queue is empty, or iii) there is an error
         retrieving messages from the queue.

   6) When done processing, free up any memory allocated for
      the message queue (if any), post a termination message
      to the server with the API function AddInLogMessageText, 
      close the message queue with the API function MQClose, 
      close the Domino billing db with the API function NSFDbClose.

	
Input parameters:
   hResourceModule - An instance handle to this add-in task.  This is 
                     necessary due to the fact that there can and may be
                     more than one instance of this add-in program running
                     on the same Domino Server.
   argc - The count of command line arguments passed into this program
   argv - An array of the arguments passed into this program
    
Output Parameters:
   None.
	    
***************************************************************************/               

STATUS LNPUBLIC AddInMain (HMODULE hResourceModule, int argc, char *argv[])
{
   STATUS   error;
   MQHANDLE bMsgQueue=NULL;
   DWORD    Wakeup=0, Runtime=0, BillingClass=0;
   DHANDLE    hDB;
   char     *Message=0;
   WORD     msgsize=MAXWORD;
   BOOL     fBillingDBOpen=FALSE;

/* Default is to wake up every minute and run for 10 seconds. */

   if (! (Wakeup = OSGetEnvironmentLong("BillingAddinWakeup")) )
       Wakeup = 60;

   if (! (Runtime = OSGetEnvironmentLong("BillingAddinRuntime")) )
       Runtime = 10;

   if (Runtime > Wakeup)
   {
       AddInLogMessageText(MSG_ADDIN_TERMINATING, NULL);
       return (NOERROR);
   }

/* Log that the billing task has started. */

   AddInLogMessageText(MSG_ADDIN_STARTED, NULL);

/* Set the task into "Initializing" state */

   AddInSetStatusText(MSG_ADDIN_INITIALIZING, 0L);

/* Initialize the Message Queue */

   if (error = MQCreate(BILL_QUEUE_NAME, MAXWORD, 0))
   {
       if (error != ERR_DUPLICATE_MQ)
       {
           AddInLogErrorText(MSG_ADDIN_ERROR, error, "MQCreate");
           return (NOERROR);
       }
   }
 
   if (error = MQOpen (BILL_QUEUE_NAME, 0, &bMsgQueue))
   {
       AddInLogErrorText(MSG_ADDIN_ERROR, error,"MQOpen");
       return (NOERROR);
   }

/* Allocate enough memory to hold the session struct returned in message */

   if( (Message = (char *)malloc( msgsize)) == NULL)
   {
       AddInLogErrorText(MSG_ADDIN_ERROR_NOMEMORY, 0L,"Malloc()");
       return(NOERROR);
   }

/* Create/Open the billing sample database */

   if (error = BillingDBOpen (&hDB))
   {
       AddInLogErrorText (MSG_ADDIN_ERROR_DBOPEN, error, NULL); 
       goto done;
   }
   else
      fBillingDBOpen = TRUE;

/* Run until Message Queue is not in QUIT state */

   while (!MQIsQuitPending(bMsgQueue))
   {
       WORD len;

       AddInSetStatusText(MSG_ADDIN_IDLE, 0L);
		
       /* Yield processor to other tasks and reset schedules. 
          Quit if server terminates. */

       if (AddInIdle())
       {
		
           /* Empty the queue. */
	   
           AddInSetStatusText(MSG_ADDIN_PROCESSBILLING, 0L);
           while (TRUE)
           {
               error=MQGet(bMsgQueue, Message, msgsize, 0, 0, &len);
    
               /* Queue is empty.  Stop processing. */

               if (error) 
               {
                   if (error != ERR_MQ_EMPTY)
                       AddInLogErrorText(MSG_ADDIN_ERROR, error,"MQGet Quit");
               }
		
               if ((len <= 0) || (error))
                   break;
    
               if (error = BillingMessageToDB ((BILLMSG*) Message, hDB, len))
                   AddInLogErrorText (MSG_ADDIN_ERROR_WRITEDB, error, NULL);
           }  
           
           goto done;
       }
	
   /* Read the Billing message queue every X (default: 60) seconds. */

       if (AddInSecondsHaveElapsed(Wakeup))
       {

           TIMEDATE Now, DueTime;

           AddInSetStatusText(MSG_ADDIN_PROCESSBILLING, 0L);

           /* Get a billing message */
      
           error=MQGet(bMsgQueue, Message, msgsize, 0, 0, &len);

           if (error) 
           {
               if (error != ERR_MQ_EMPTY)
                   AddInLogErrorText(MSG_ADDIN_ERROR, error,"MQGet");
           } 

     /* Got a message, bill it (and the rest) to the database. */
	 
           if ((len > 0) && (error==NOERROR) )
           {              
               OSCurrentTIMEDATE(&DueTime);
               TimeDateIncrement(&DueTime, Runtime*100);

               while (TRUE)
               {
                   if (error = BillingMessageToDB ((BILLMSG*) Message, hDB, len))
                       AddInLogErrorText (MSG_ADDIN_ERROR_WRITEDB, error, NULL);

                   OSCurrentTIMEDATE(&Now);
			
                   /* Time is up.  Stop processsing. */

                   if (TimeDateCompare(&Now, &DueTime) >= 0)
                   {
                       OSCurrentTIMEDATE(&DueTime);
                       TimeDateIncrement(&DueTime, Runtime*100);
                       break;
                   }

                   error=MQGet(bMsgQueue, Message, msgsize, 0, 0, &len);
	    
                   if (error)
                   {
                        if (error != ERR_MQ_EMPTY)
                            AddInLogErrorText(MSG_ADDIN_ERROR, error,"MQGet");
                   }

                   /* Queue is empty.  Stop processing. */

                   if ((len == 0) || (error))
                       break;
      
               }  /* end while true- mqget */

           } /* end len > 0 */
        
       }  /* end minutes elapsed*/
      
   } /* end while !quit */


done:
   if (Message)
       free (Message);

   AddInLogMessageText(MSG_ADDIN_TERMINATING, NULL);

   MQClose(bMsgQueue, 0);

   if (fBillingDBOpen)
   {
       if (error = NSFDbClose (hDB))
           AddInLogErrorText (MSG_ADDIN_ERROR, error, "closing sessbill.nsf");
   }

   return(NOERROR);
}


/*****************************************************************************

Function:  BillingDBOpen

Description: Opens the billing sample database (sessbill.nsf). If its does not
             exist then this function will call BillingCreateDB to create it.

Input parameters: 
   None
    
Output Parameters:
   RethDb - Address of handle to the Domino billing sample DB if successful
    
******************************************************************************/               
    
STATUS LNPUBLIC BillingDBOpen (DHANDLE *RethDb)
{
   DHANDLE hDb;
   STATUS error=NOERROR;

   if (error = NSFDbOpen(BillingPathName, &hDb))
   {
       if (error == ERR_NOEXIST)
           error = BillingCreateDB (BillingPathName, &hDb);
   }

   if (!error)
       *RethDb = hDb;

   return (error);
}


/*****************************************************************************

Function:       BillingCreateDB

Description: Creates the Domino billing DB based on the Notes billing template
             specified by "BillingTemplateName". Specifically, this function
             1) opens the billing template db, 2) opens the target billing
             db, 3) copies all of the non-data notes (views, forms, etc.)
             from the template db to the new db, 4) copies all of the data
             notes from the template db to the new db, 5) copies the database
             design and Access Control List (ACL).

             Note that when a new billing database is created, it contains a
             unique replica ID.  This is done so that billing databases on
             other servers in the same domain will not replicate with each
             other.
		
Input Parameters:
   BillingPathName - Contains the fully qualified path of the database to be
                     created.
						
Output Parameters:
   RethDb - Contains the address of the handle of the new database created
            by this function (if successful).
											
******************************************************************************/               

STATUS LNPUBLIC BillingCreateDB (char *BillingPathName, DHANDLE *RethDb)
{
   STATUS     error;
   DHANDLE      hTemplateDb = NULLHANDLE;
   DHANDLE      hBillingDb = NULLHANDLE;
   TIMEDATE   Time;
   char       NewInfo[NSF_INFO_SIZE];
   char       TemplateInfo[NSF_INFO_SIZE];
   char       Class[NSF_INFO_SIZE];
   NOTEHANDLE hNote;
   
   if (error = NSFDbOpen(BillingTemplateName, &hTemplateDb))
       return (error);

   error = NSFDbCreate(BillingPathName, DBCLASS_NOTEFILE, TRUE);

   if (!error)
       error = NSFDbOpen(BillingPathName, &hBillingDb);

   if (!error)
   {

   /* Copy notes from template file to new file */
      
       TimeConstant(TIMEDATE_WILDCARD, &Time);
       error = NSFDbCopyExtended(hTemplateDb, hBillingDb, Time, NOTE_CLASS_ALLNONDATA, DBCOPY_REPLICA|DBCOPY_SUBCLASS_TEMPLATE, NULL);
       if (error)
       {
           if (ERR(error) == ERR_NO_MODIFIED_NOTES)
               error = NOERROR;
           else
           {
               NSFDbClose(hBillingDb);
               goto done;
           }
       }

   /* Copy notes from template file to new file */

       TimeConstant(TIMEDATE_WILDCARD, &Time);
       error = NSFDbCopy(hTemplateDb, hBillingDb, Time, NOTE_CLASS_DOCUMENT);
       if (error)
       {
           if (ERR(error) == ERR_NO_MODIFIED_NOTES)
               error = NOERROR;
           else
           {
               NSFDbClose(hBillingDb);
               goto done;
           }
       }

       if (NSFDbInfoGet(hTemplateDb, TemplateInfo))
           TemplateInfo[0] = 0;

   /* Update the notefile info with that from the template including
      automatically inheriting design changes. */

       strcpy(NewInfo, TemplateInfo);
       NSFDbInfoModify(NewInfo, INFOPARSE_DESIGN_CLASS, NULL);
       NSFDbInfoParse(NewInfo, INFOPARSE_CLASS, Class, sizeof(Class)-1);
       NSFDbInfoModify(NewInfo, INFOPARSE_DESIGN_CLASS, Class);
       NSFDbInfoModify(NewInfo, INFOPARSE_CLASS, NULL);

       if (!error)
           error = NSFDbInfoSet(hBillingDb, NewInfo);

   /* Update the icon note's title with that from the NTF */

       if (!error)
       {
          if (!NSFNoteOpen(hBillingDb, NOTE_ID_SPECIAL+NOTE_CLASS_ICON, 0, &hNote))
          {
              NSFItemSetText(hNote, FIELD_TITLE, NewInfo, MAXWORD);
              NSFNoteUpdate(hNote, 0);
              NSFNoteClose(hNote);
          }
       }

       if (!error)
           error = NSFDbCopyTemplateACL( hTemplateDb, hBillingDb, 
                                         NULL, ACL_LEVEL_EDITOR );
   
       if (!error)
           *RethDb = hBillingDb;
       else
           NSFDbClose(hBillingDb);
   }

done:

   NSFDbClose(hTemplateDb);
   return (error);

}


/*****************************************************************************

Function:       BillingMessageToDB

Description: Outputs the most recent session and database billing records
             that were taken from the billing message queue to the billing 
             database. This function keys on the struct type of the billing
             message. If the struct is of type BILL_SESSIONREC, BILL_DBREC,
             or BILL_NOTECREATEREC the header information contained in the billing
             message struct is extracted and added to the billing note using 
             NSFItemAppend. Next NSFItemSetText is called to set the note to 
             indicate the the record is a Session or Database record.  Then all
             of the fields in the record structures are processed and appended 
             to the billing note with NSFItemAppend. Lastly, NSFNoteUpdate is 
             called to write the Note to the billing database and the note is
             closed with NSFNoteClose.

Input Parameters:
   Message - The a pointer to the billing message taken from the billing
             message queue

   hDB - Handle to the billing database

   len - Length of the billing message
						
Output Parameters:
   None.
													
******************************************************************************/               

STATUS LNPUBLIC BillingMessageToDB (BILLMSG *Message, DHANDLE hDb, DWORD len)
{
   DHANDLE hNote;
   NUMBER FloatTemp, DWordTemp;
   STATUS error;
   long   tmp;

   /* Create a new note to hold the billing record */
    
   if (error = NSFNoteCreate(hDb, &hNote))
      goto Done;

   /* First write the billing record header (shared) info to the note */
    
   /* Header:  Add Message class */

   FloatTemp = (NUMBER) Message->Class;
   if (error = NSFItemAppend(hNote, ITEM_SUMMARY, 
                             BILL_ITEM_HEADER_CLASS, 
                             sizeof(BILL_ITEM_HEADER_CLASS) - 1, 	    
                             TYPE_NUMBER,                                                                        	    
                             &FloatTemp, (DWORD) sizeof(FloatTemp)))                                             	    
       goto Done;
	    
   /* Header:  Add Message StructType */

   FloatTemp = (NUMBER) Message->StructType;
   if (error = NSFItemAppend(hNote, ITEM_SUMMARY, 
                             BILL_ITEM_HEADER_STRUCTTYPE, 
                             sizeof(BILL_ITEM_HEADER_STRUCTTYPE) - 1, 
                             TYPE_NUMBER,
                             &FloatTemp, (DWORD) sizeof(FloatTemp)))
       goto Done;

   /* Header:  Add timestamp */

   if (error = NSFItemAppend(hNote, ITEM_SUMMARY, 
                             BILL_ITEM_HEADER_TIMESTAMP, 
                             sizeof(BILL_ITEM_HEADER_TIMESTAMP) - 1, 
                             TYPE_TIME, 
                             &Message->TimeStamp, (DWORD) sizeof(TIMEDATE)))
       goto Done;

   /* Header:  Add Servername */

   if (error = NSFItemAppend(hNote, ITEM_SUMMARY, 
                             BILL_ITEM_HEADER_SERVERNAME, 
                             sizeof(BILL_ITEM_HEADER_SERVERNAME) - 1, 
                             TYPE_TEXT,
                             Message->ServerName, (DWORD) strlen(Message->ServerName)))
       goto Done;

   /* Then write the billing message structtype specific info to the note */

   switch (Message->StructType)
   {

       /* Standard Session billing */

       case BILL_SESSIONREC:
		  
           /* Set the billing record type to type Session */

           if (error = NSFItemSetText(hNote, FIELD_FORM, BILLING_SESSION_FORM,
                                      (WORD) strlen(BILLING_SESSION_FORM)))              
               goto Done;

           /* Session:  Add SessionID*/

           memmove( &tmp, &Message->rec.sess.SessionID, sizeof( long));
	
           FloatTemp = (NUMBER) tmp;
           if (error = NSFItemAppend(hNote, ITEM_SUMMARY, 
                                     BILL_ITEM_SESSION_SESSIONID, 
                                     sizeof(BILL_ITEM_SESSION_SESSIONID) - 1, 
                                     TYPE_NUMBER,
                                     &FloatTemp, (DWORD) sizeof(FloatTemp)))
               goto Done;

           /* Session:  Add BytesIn */

           FloatTemp = (NUMBER) Message->rec.sess.BytesIn;
           if (error = NSFItemAppend(hNote, ITEM_SUMMARY, 
                                     BILL_ITEM_SESSION_BYTESIN, 
                                     sizeof(BILL_ITEM_SESSION_BYTESIN) - 1, 
                                     TYPE_NUMBER,
                                     &FloatTemp, (DWORD) sizeof(FloatTemp)))
               goto Done;

           /* Session:  Add BytesOut */

           FloatTemp = (NUMBER) Message->rec.sess.BytesOut;
           if (error = NSFItemAppend(hNote, ITEM_SUMMARY, 
                                     BILL_ITEM_SESSION_BYTESOUT, 
                                     sizeof(BILL_ITEM_SESSION_BYTESOUT) - 1, 
                                     TYPE_NUMBER,
                                     &FloatTemp, (DWORD) sizeof(FloatTemp)))
               goto Done;
	
           /* Session:  Username */

           if (error = NSFItemAppend(hNote, ITEM_SUMMARY, 
                                     BILL_ITEM_SESSION_USERNAME, 
                                     sizeof(BILL_ITEM_SESSION_USERNAME) - 1, 
                                     TYPE_TEXT,
                                     Message->rec.sess.Username, 
                                     (DWORD) strlen(Message->rec.sess.Username)))
               goto Done;
  
           /* Session:  Add Action*/

           FloatTemp = (NUMBER) Message->rec.sess.Action;
           if (error = NSFItemAppend(hNote, ITEM_SUMMARY, 
                                     BILL_ITEM_SESSION_ACTION, 
                                     sizeof(BILL_ITEM_SESSION_ACTION) - 1, 
                                     TYPE_NUMBER,
                                      &FloatTemp, (DWORD) sizeof(FloatTemp)))
               goto Done;

           /* Session:  Network Address of client */

           if (error = NSFItemAppend(hNote, ITEM_SUMMARY, 
                                     BILL_ITEM_SESSION_NETADR,
                                     sizeof(BILL_ITEM_SESSION_NETADR) - 1, 
                                     TYPE_TEXT,
                                     Message->rec.sess.NetAdr, 
                                     (DWORD) strlen(Message->rec.sess.NetAdr)))
               goto Done;

               break;


       /* Standard Database billing */

       case BILL_DBREC:

           /* Set the billing record type to type DB */

           if (error = NSFItemSetText(hNote, FIELD_FORM, BILLING_DB_FORM, 
                                      (WORD) strlen(BILLING_DB_FORM)))           
               goto Done;

           /* Database:  Add SessionID*/

           memmove (&tmp, &Message->rec.db.SessionID, sizeof (long));
   
           FloatTemp = (NUMBER) tmp;
           if (error = NSFItemAppend(hNote, ITEM_SUMMARY, 
                                     BILL_ITEM_DB_SESSIONID, 
                                     sizeof(BILL_ITEM_DB_SESSIONID) - 1, 
                                     TYPE_NUMBER,
                                     &FloatTemp, (DWORD) sizeof(FloatTemp)))
               goto Done;

           /* Database:  Add ReplicaID */

           if (error = NSFItemAppend(hNote, ITEM_SUMMARY, 
                                     BILL_ITEM_DB_REPLICAID, 
                                     sizeof(BILL_ITEM_DB_REPLICAID) - 1, 
                                     TYPE_TIME,
                                     &Message->rec.db.ReplicaID, (DWORD) sizeof(TIMEDATE)))
               goto Done;

           /* Database:  Username */

           if (error = NSFItemAppend(hNote, ITEM_SUMMARY, 
                                     BILL_ITEM_DB_USERNAME, 
                                     sizeof(BILL_ITEM_DB_USERNAME) - 1, 
                                     TYPE_TEXT,
                                     Message->rec.db.Username, 
                                     (DWORD) strlen(Message->rec.sess.Username)))
               goto Done;

           /* Database:  Add DBOpenTime */
           /* Convert from Centiseconds to Seconds, and Round up */

           tmp = (Message->rec.db.DBOpenTime + 50) / 100;
           FloatTemp = (NUMBER) tmp;

           if (error = NSFItemAppend(hNote, ITEM_SUMMARY, 
                                   BILL_ITEM_DB_OPENTIME, 
                                   sizeof(BILL_ITEM_DB_OPENTIME) - 1, 
                                   TYPE_NUMBER,
                                   &FloatTemp, (DWORD) sizeof(FloatTemp)))
               goto Done;

           /* Database:  Add Action*/
           FloatTemp = (NUMBER) Message->rec.db.Action;
           if (error = NSFItemAppend(hNote, ITEM_SUMMARY, 
                                     BILL_ITEM_DB_ACTION, 
                                     sizeof(BILL_ITEM_DB_ACTION) - 1, 
                                     TYPE_NUMBER,
                                     &FloatTemp, (DWORD) sizeof(FloatTemp)))
               goto Done;

           break;
      

       /* Note Create (extended) billing */

       case BILL_NOTECREATEREC:

           /* Set the billing record type to type  NOTECREATE */

           if (error = NSFItemSetText(hNote, FIELD_FORM, BILLING_NOTECREATE_FORM, 
                                      (WORD) strlen(BILLING_NOTECREATE_FORM)))              
               goto Done;

           /* Note Create:  Add NoteUser*/

           if (error = NSFItemAppend(hNote, ITEM_SUMMARY, 
                                     BILL_ITEM_NOTECREATE_USER, 
                                     sizeof(BILL_ITEM_NOTECREATE_USER) - 1, 
                                     TYPE_TEXT,
                                     Message->rec.notecreate.Username,
                                     (DWORD) strlen(Message->rec.notecreate.Username)))
               goto Done;

           /* Note Create:  Add NOTEID */
           DWordTemp = (NUMBER) Message->rec.notecreate.dbNoteID;
           if (error = NSFItemAppend(hNote, ITEM_SUMMARY, 
                                     BILL_ITEM_NOTECREATE_NOTEID, 
                                     sizeof(BILL_ITEM_NOTECREATE_NOTEID) - 1, 
                                     TYPE_NUMBER,
                                     &DWordTemp, (DWORD) sizeof(DWordTemp)))
               goto Done;


           /* Note Create:  Add ReplicaID */

           if (error = NSFItemAppend(hNote, ITEM_SUMMARY, 
                                     BILL_ITEM_NOTECREATE_REPLICAID,
                                     sizeof(BILL_ITEM_NOTECREATE_REPLICAID) - 1, 
                                     TYPE_TIME,
                                     &Message->rec.notecreate.ReplicaID, (DWORD) sizeof(TIMEDATE)))
               goto Done;

           break;
	 

       default:

       /* No processing for "other" billing record messages */

        goto Done; 
        break;

   }  /* end switch */


/*  Update the note */

   error = NSFNoteUpdate(hNote, UPDATE_FORCE|UPDATE_NOREVISION);   /* Suppress ERR_CONFLICT */

/* and close the note when done */

Done:
   if (hNote != NULLHANDLE)
       NSFNoteClose(hNote);
   return (error);

}
