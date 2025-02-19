/****************************************************************************** 
 *
 * Copyright HCL Technologies 1996, 2025.
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

Module: BILLSES.h

Description: Header file for BILLSES.C.  Contains #defines for error
	     messages, and integer and string constants. 
		
******************************************************************************/


#ifndef BILLSES_DEFS
#define BILLSES_DEFS

/* Message & Error Definitions */

/* String #0 must always be the add-in application's task name, and
   string #1 must have its version number. */

#define MSG_ADDIN_NAME              "Session Billing Sample"
#define MSG_ADDIN_VERSION           "Version 1.0"

/*      Define messages */

#define MSG_ADDIN_STARTED           "Billing Sample started"
#define MSG_ADDIN_INITIALIZING      "Initializing Billing Sample"
#define MSG_ADDIN_TERMINATING       "Billing Sample shutdown"
#define MSG_ADDIN_IDLE              "Idle"
#define MSG_ADDIN_PROCESSBILLING    "Process Billing"
#define MSG_ADDIN_ERROR             "%s Billing Sample error"
#define MSG_ADDIN_ERROR_DBOPEN      "Unable to create SessBill.nsf"
#define MSG_ADDIN_ERROR_WRITEDB     "Error writing billing record to SessBill.nsf."
#define MSG_ADDIN_ERROR_NOMEMORY    "%s Not enough memory to allocate for billing record."


/* Database Item Name Definitions */

/* Header for all billing record struct types */
#define BILL_ITEM_HEADER_CLASS      "Class"
#define BILL_ITEM_HEADER_STRUCTTYPE "StructType"
#define BILL_ITEM_HEADER_TIMESTAMP  "TimeStamp"
#define BILL_ITEM_HEADER_SERVERNAME "ServerName"

/* Session Form */
#define BILLING_SESSION_FORM        "Session"
#define BILL_ITEM_SESSION_SESSIONID "SessionID"
#define BILL_ITEM_SESSION_BYTESIN   "BytesIn"
#define BILL_ITEM_SESSION_BYTESOUT  "BytesOut"
#define BILL_ITEM_SESSION_USERNAME  "Username"
#define BILL_ITEM_SESSION_ACTION    "Action"
#define BILL_ITEM_SESSION_NETADR    "NetAdr"

/* Database Form */
#define BILLING_DB_FORM             "Database"
#define BILL_ITEM_DB_SESSIONID      "SessionID"
#define BILL_ITEM_DB_REPLICAID      "ReplicaID"
#define BILL_ITEM_DB_USERNAME       "Username"
#define BILL_ITEM_DB_OPENTIME       "OpenTime"
#define BILL_ITEM_DB_ACTION         "Action"


/* Note Create Form */
#define BILLING_NOTECREATE_FORM        "NoteCreate"
#define BILL_ITEM_NOTECREATE_USER      "NoteUser"
#define BILL_ITEM_NOTECREATE_NOTEID    "NOTEID"
#define BILL_ITEM_NOTECREATE_REPLICAID "DBID"

#endif

