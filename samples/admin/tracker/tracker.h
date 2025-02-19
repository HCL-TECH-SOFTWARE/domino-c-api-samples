/****************************************************************************
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

    PROGRAM:   tracker

    FILE:      tracker.h

    PURPOSE:   Symbolic constants and function prototypes

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* NOTES.INI variables Tracker looks for. */
#define  TRACKER_DBNAME_VARIABLE "TRACKER_TARGET"
#define  TRACKER_TRASHCAN_VARIABLE  "TRACKER_TRASH"

/* if Tracker doesn't find the above variables NOTES.INI, it 
   uses the following defaults.
 */
#define  TRACKER_DEFAULT_DBNAME "tracker"
#define  TRACKER_DEFAULT_TRASHCAN "trashcan"

/*  Field names used in the tracker database */
#define TRACKER_ITEM_SERVER             "Server"         /* text */
#define TRACKER_ITEM_STARTTIME          "StartTime"      /* time/date */
#define TRACKER_ITEM_FINISHTIME         "FinishTime"     /* time/date */
#define TRACKER_ITEM_BODY               "Body"           /* rich text */
#define TRACKER_ITEM_REQ_NUMBER         "ReqNumber"      /* number */

/* Field values that depend on design of the tracker database */
#define TRACKER_FORM    "Tracker Log"
#define REQUEST_FORM    "1 Assistance Request"

/* max text length of a form name */
#define MAX_FORM_NAME   128

/* max length of notes error code text  */
#define MAX_NOTES_ERROR 128

/* maximum text size of one entry in the Tracker Log */
#define MAX_TRACKERLOG  (256+MAXUSERNAME)

/* maximum number of tracker instances */
#define MAX_TRACKER_INSTANCES 99

/* Tracker keeps a detailed trace of everything it does in the ASCII
   file "tracker.xxx".  This error trace file records every time Domino and Notes
   calls a tracker function, regardless of what database or note is being
   accessed. Use this error trace to debug tracker. Programs like tracker  
   may log information to a trace file or similar output because they can
   not simply printf to the standard out. Tracker interprets any nonzero 
   status codes returned from Lotus C API for Domino and Notes functions 
   using OSLoadString and prints the error message to the "tracker.xxx" file.  
   A new file gets written each time Tracker gets initialized.
 */

#if defined(OS400)
#define TRACE_FILENAME "tracker"
#else
#define TRACE_FILENAME "c:\\tracker"
#endif
#define TRACKER_BLURB   \
"This trace file is created by Notes API sample program \"tracker\".\n\
Tracker runs automatically if the notes.ini variable NSF_HOOKS=TRACKER.\n"


/* Tracker provides a unique sequential request number feature to the
   service request application. When a user saves a newly-composed service
   request document, Tracker sets the ReqNumber field with a unique integer
   that is one greater than the last number used. Tracker implements this
   feature using the "reqnum.dat" file. The "reqnum.dat" file is a binary
   file that contains the NUMBER (a double) to use as the next service
   request number. Each time a user updates a service request document,
   Domino and Notes calls TrackerNoteUpdate. TrackerNoteUpdate detects whether the
   document is new and being saved for the first time. If so, it opens and
   reads the "reqnum.dat" file, sets the ReqNumber field in the document
   to the number from the file, increments the number, and updates the file
   with the incremented number. Tracker opens the file with share mode
   _SH_DENYRW to enforce serial access to the file in case multiple
   instances of Tracker are active. Note that this scheme only works in the
   limited context of a single Domino server. Extending this scheme to wider
   scope would require some kind of network-wide synchronization. To create
   the reqnum.dat file for the first time, use reqnum, included with
   tracker.
 */
/**
* For i5/OS make these relative to the current directory - which will normally 
* be the data directory.  We could also call OSGetDataDirectory and ensure they 
* go there.  This allows for better seperation of partitioned servers.
**/

#if defined(OS400)
#define REQNUM_FILENAME "reqnum.dat"
#define INSTANCE_FILENAME "instance.dat"
#else
#define REQNUM_FILENAME "c:\\reqnum.dat"
#define INSTANCE_FILENAME "c:\\instance.dat"
#endif

/* Tracker's error codes. */
                              /* Unable to get "ReqNumber" item from note */
#define ERR_TRACKER_REQNUMITEM   (PKG_ADDIN + 1)
                              /* Trouble accessing "reqnum.dat" file */
#define ERR_TRACKER_REQNUMFILE   (PKG_ADDIN + 2)
                              /* Trouble accessing "instance.dat" file */
#define ERR_TRACKER_INSTANCEFILE (PKG_ADDIN + 3)
                              /* maximum instances exceeded */
#define ERR_TRACKER_MAX_INSTANCES (PKG_ADDIN + 4)

/* function prototypes  */
STATUS LNPUBLIC MainEntryPoint(DBHOOKVEC*);
STATUS LNPUBLIC TrackerTerm(DBHOOKVEC*);
STATUS LNPUBLIC TrackerNoteOpen(DBHOOKVEC*, char*,
            LIST*, DBHANDLE, NOTEID, NOTEHANDLE, WORD);
STATUS LNPUBLIC TrackerNoteUpdate(DBHOOKVEC*, char*,
            LIST*, DBHANDLE, NOTEID, NOTEHANDLE, WORD*);
STATUS LNPUBLIC TrackerDbStampNotes(DBHOOKVEC*, char*,
            LIST*, DBHANDLE, HANDLE, char*, WORD, void*, WORD);
STATUS LNPUBLIC StartLog(DBHANDLE);
VOID   LNPUBLIC FinishLog (void);
VOID   LNPUBLIC AddLogEntry(char*, char*);
BOOL   LNPUBLIC IsTargetDB(DBHANDLE);
BOOL   LNPUBLIC NoteIsServiceRequest(NOTEHANDLE);
VOID   LNPUBLIC PrintToTraceFile(char*, char*, 
            LIST*, DBHANDLE, NOTEID, NOTEHANDLE, WORD);
VOID   LNPUBLIC PrintListToTraceFile(LIST*, char*);
VOID   LNPUBLIC PrintDBPathToTraceFile(DBHANDLE, char*);
STATUS LNPUBLIC GetReqNumItem(NOTEHANDLE, NUMBER*, char*);
STATUS LNPUBLIC GetReqNumber(NUMBER *);
STATUS LNPUBLIC CopyNoteToTrashcan(DBHANDLE, NOTEID);
STATUS LNPUBLIC RenderDataAsText(void*, WORD, char**);
VOID   LNPUBLIC TraceNotesError(STATUS, char*);
STATUS LNPUBLIC GetInstanceNumber(int*);
