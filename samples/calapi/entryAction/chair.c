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

    PROGRAM:    chair

    FILE:       chair.c

    SYNTAX:     chair serverName mailfile user1 user2

    PURPOSE:    demonstrates create of an iCalendar entry.

    DESCRIPTION:

        This program processes the following logic:

        .  Call TestCalCreateEntry to create an iCalendar entry in the
           mailbox.After this, you will see the entry in the calendar.
           if RRULE included, the entry repates.         

***************************************************************************/

#include "global.h"
#include "oserr.h"
#include "osfile.h"
#include "idtable.h"
#include "miscerr.h"
#include "nsf.h"
#include "osenv.h"
#include "osmem.h"
#include "ostime.h"
#include "osmisc.h"
#include "intl.h"
#include "calapi.h"
#include "textlist.h"
#include "names.h"
#include "kfm.h"
#include "icalSample.h"
#include "../../printLog.h"

#include <stdlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ICAL_UID 	//Leaving it blank, if we don't provide the value, Notes will create one for you
#define OUTFILE		"EntryAction_UID.tmp"

#if defined(OS400)
#pragma convert(850)
#endif

// Global variables
FILE        *outFile=NULL;

STATUS GetDBHdl(char* szPath, DHANDLE* hDB)
{
    STATUS error = NOERROR;
    char fullPath[MAXPATH] = "";
    
    OSPathNetConstruct(NULL, NULL, szPath, fullPath);
    error = NSFDbOpen(fullPath, hDB);

    if (error != NOERROR)
    {
        PRINTERROR(error, "NSFDbOpen");
    }
 
    return error;
}

STATUS TestCalCreateEntry(DHANDLE hDB, char* pszICalender, char* pOrganizer, char* pAttendee)
{
    STATUS	error = NOERROR;

    DWORD dwFlags = 0L;
    MEMHANDLE hRetUID = NULL;
    char* pszRetUID = NULL;
    
    error = CalCreateEntry(hDB, pszICalender, dwFlags, &hRetUID, NULL);
    if(error != NOERROR)
    {
        PRINTERROR(error,"CalCreateEntry");
        return error;
    }

    pszRetUID = (char*)OSMemoryLock(hRetUID);
   /* Print the note content to an open file */
    outFile = fopen( OUTFILE, "w" );
    fprintf (outFile, "%s\n", pszRetUID);
    fclose(outFile);
    
    PRINTLOG("UID Returnd %s\n", pszRetUID);
    PRINTLOG("Mail was sent to %s succesfully from %s\n", pAttendee, pOrganizer);

    OSMemoryUnlock(hRetUID);
    OSMemoryFree(hRetUID);
    return error;
}
    

int main(int argc, char *argv[])
{
    /*
    example from rfc5545

    "DTSTART:%s"  start at the next day, will be created by GetStartTime
    "RRULE:FREQ=DAILY;COUNT=10"  repeating 10 days. 
    "ORGANIZER:mailto:TestDomino/HCLPNP"  "test2 admin/HCLPNP" will be the chair of the meeting,change as you need
    "ATTENDEE;RSVP=TRUE:mailto:TestDomino/HCLPNP"   "test3 admin/HCLPNP" will be the attendee. If don't have RSVP=TRUE setted, will be a broadcast, can't be acceptted.
    */
    char pszICalendartTemp[] = "BEGIN:VCALENDAR" CRLF
                               "VERSION:2.0" CRLF
                               "PRODID:-//hacksw/handcal//NONSGML v1.0//EN" CRLF
                               "BEGIN:VEVENT" CRLF
                               "UID:" ICAL_UID CRLF
                               "DTSTAMP:20130820T172345Z" CRLF
                               "DTSTART:%s" CRLF
                               "DTEND:%s" CRLF
                               "CATEGORIES:MEETING" CRLF
                               "CLASS:PRIVATE" CRLF
                               "RRULE:FREQ=DAILY;COUNT=10" CRLF
                               "SUMMARY:Hot Summer Party" CRLF
                               "ORGANIZER:mailto:%s" CRLF
                               "ATTENDEE;RSVP=TRUE:mailto:%s" CRLF
                               "END:VEVENT" CRLF
                               "END:VCALENDAR" CRLF;

    char pszICalendar[1024] = {0};
    
    STATUS error = NOERROR;

    char pname[MAXPATH] = "";         /* buffer to store the input path to database */
    DHANDLE hDB = 0;

    
    char pStartTime[MAXALPHATIMEDATE] = {0};
    char pEndTime[MAXALPHATIMEDATE] = {0};
    char pRecurrenceID[MAXALPHATIMEDATE] = {0};
    char pOrganizer[MAXPATH] = {0};
    char pAttendee[MAXPATH] = {0};
    char psRetUID[MAXALPHATIMEDATE] = {0};
    
    if(argc < 5)
    {
        PRINTLOG("usage: chair server1/test mail\\chair.nsf 'chair test/test' 'attendee test/test' \r\n");
        return 0;
    }
    error = NotesInitExtended (argc, argv);

    if (strcmp(argv[1], ""))
    {
        if (error = OSPathNetConstruct( NULL, argv[1], argv[2], pname))
        {
            PRINTERROR (error,"OSPathNetConstruct");
            NotesTerm();
            return (1);
        }
    }
    else
    {
        memcpy(pname, argv[2], sizeof(pname));
    }


    /* Get organizer mail id and attendee mail id */
    memcpy(pOrganizer, argv[3], sizeof(pOrganizer));
    memcpy(pAttendee, argv[4], sizeof(pAttendee));
    
    /*
    get start time of the first instance of the meeting
    */
    GetStartTime(pStartTime, pEndTime, pRecurrenceID);
    sprintf(pszICalendar, pszICalendartTemp, pStartTime, pEndTime, pOrganizer, pAttendee);
    
    /*
    we should run as chair's id from now on. say "n if/test" here.
    */
    error = GetDBHdl(pname, &hDB);
    if(error != NOERROR)
    {
    PRINTLOG("GetDBHdl error!");
        goto ERROR_EXIT;
    }
    
    /*
    if create success, you will see the entry in Calendar.
    Note: when creating a meeting, Notes will send notice to attendees too.
          so make sure attendee name and server are all right.
    */
    error = TestCalCreateEntry(hDB, pszICalendar, pOrganizer, pAttendee);
    if(error != NOERROR)
    {
        PRINTLOG("TestCalCreateEntry error!");
        goto ERROR_EXIT;
    }

ERROR_EXIT:
    if(hDB)
    {
        NSFDbClose(hDB);
    }

    NotesTerm();

    return error;
}
