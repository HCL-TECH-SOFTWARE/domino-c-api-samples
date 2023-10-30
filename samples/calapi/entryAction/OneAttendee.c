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

    PROGRAM:    OneAttendee

    FILE:       OneAttendee.c

    SYNTAX:     OneAttendee serverName mailfile user1 user2

    PURPOSE:    demonstrates accept an invation of an repeating meeting, 
    		then decline one of them.

    DESCRIPTION:

        This program processes the following logic:

        .  Call GetAndAcceptNewInvitationsFromYesterday to receive all 
        invations from yesterday.After this, you will see the entry in the calendar.        
           
        .  Call TestCalEntryActionDeclineExt to decline one instance of
        	the invation.

        .  Call TestCalReadEntryExt to read the instance you have just declined.
		if everything is OK, you will get an error 0x404(Entry not found in index), which we expected.
		
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
#include "calapi.h"
#include "textlist.h"
#include "names.h"
#include "kfm.h"
#include "icalSample.h"
#include "../../printLog.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#if defined(OS400)
#pragma convert(850)
#endif

// Global variables
FILE        *InFile=NULL;

STATUS GetDBHdl(char* szPath, DHANDLE* hDB)
{
    STATUS error = NOERROR;
    char fullPath[MAXPATH] = "";

    OSPathNetConstruct(NULL, NULL, szPath, fullPath);
    error = NSFDbOpen(fullPath, hDB);

    if (error != NOERROR)
        PRINTERROR(error,"NSFDbOpen");

    return error;
}

STATUS TestCalReadEntryExt(DHANDLE hDB, char* pszICalenderUID, char* pszRecurrenceID)
{
    STATUS   error = NOERROR;

    MEMHANDLE hCalData = NULL;
    DWORD dwFlags = CAL_READ_INCLUDE_X_LOTUS;
    char* pszCalData = NULL;

    error = CalReadEntry(hDB, pszICalenderUID, pszRecurrenceID, &hCalData, NULL, dwFlags, NULL);
    if(error != NOERROR)
    {
        PRINTERROR(error,"CalReadEntry");
        return error;
    }

    pszCalData = (char*)OSMemoryLock(hCalData);

    PRINTLOG("Calender Data Returnd\r\n%s\r\n", pszCalData);

    OSMemoryUnlock(hCalData);
    OSMemoryFree(hCalData);

    return error;
}

STATUS TestCalEntryActionDeclineExt(DHANDLE hDB, char* pszICalenderUID, char* pszRecurrenceID)
{
    STATUS   error = NOERROR;

    DWORD dwAction = CAL_PROCESS_DECLINE;
    DWORD dwRange = 0;
    PEXT_CALACTION_DATA pExtActionInfo = NULL;

    PRINTLOG("Calender UID : %s\n", pszICalenderUID);
    PRINTLOG("Deleting Recurrence ID : %s\n", pszRecurrenceID);

    error = CalEntryAction(hDB, pszICalenderUID, pszRecurrenceID, dwAction, dwRange, "decline", pExtActionInfo, 0, NULL);

    if (error != NOERROR)
    {
        PRINTERROR(error,"CalEntryAction");
        return error;
    }

    return error;
}


STATUS TestCalNoticeActionAccept(DHANDLE hDB, NOTEID noteID)
{
    STATUS   error = NOERROR;

    DWORD dwAction = CAL_PROCESS_ACCEPT;

    PEXT_CALACTION_DATA pExtActionInfo = NULL;

    error = CalNoticeAction(hDB, noteID, dwAction, NULL, NULL, 0, NULL);

    if (error != NOERROR)
    {
        PRINTERROR(error,"CalNoticeAction");
        return error;
    }

    return error;
}

/*
Note: we doesn't specify start/end time, so will start one day before current time.
*/
STATUS GetAndAcceptNewInvitationsFromYesterday(DHANDLE hDB)
{
    STATUS   error = NOERROR;

    WORD wNumInvites = 0;
    MEMHANDLE hRetNOTEIDs = NULL;
    MEMHANDLE hRetUNIDs = NULL;

    NOTEID* pNoteID = NULL;


    error = CalGetNewInvitations(hDB, NULL, NULL, NULL, NULL, &wNumInvites, &hRetNOTEIDs, &hRetUNIDs, NULL, 0, NULL);

    if (error != NOERROR)
    {
        PRINTERROR(error,"CalGetNewInvitations");
        return error;
    }

    if(hRetNOTEIDs && hRetUNIDs)
    {

        pNoteID = (NOTEID*)OSMemoryLock(hRetNOTEIDs);

        while(wNumInvites)
        {
            PRINTLOG("CalGetNewInvitations returned NoteID: 0x%x\n", *pNoteID);
            error = TestCalNoticeActionAccept(hDB, *pNoteID);
            pNoteID++;
            wNumInvites--;
        }

        OSMemoryUnlock(hRetNOTEIDs);
        OSMemoryFree(hRetNOTEIDs);
    }

    return error;
}

int main(int argc, char *argv[])
{
    STATUS error = NOERROR;

    char pname[MAXPATH] = "";         /* buffer to store the input path to database */

    DHANDLE hDB = 0;

    char pStartTime[MAXALPHATIMEDATE] = {0};
    char pEndTime[MAXALPHATIMEDATE] = {0};
    char pRecurrenceID[MAXALPHATIMEDATE] = {0};
    char pOrganizer[MAXPATH] = {0};
    char pAttendee[MAXPATH] = {0};
    char UID[MAXPATH] =  {0};

    if(argc < 5)
    {
        PRINTLOG("usage: OneAttendee server1/test mail\\attendee.nsf 'chair test/test' 'attendee test/test'\r\n");
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

    /*Get Organizer and Attendee Name*/

    memcpy(pOrganizer, argv[3], sizeof(pOrganizer));
    memcpy(pAttendee, argv[4], sizeof(pAttendee));


    /*
    we should run as attendee's id from now on. say "n sf/test" here.
    */
    error = GetDBHdl(pname, &hDB);
    if(error != NOERROR)
    {
        PRINTLOG("GetDBHdl error!");
        goto ERROR_EXIT;
    }

    GetStartTime(pStartTime, pEndTime, pRecurrenceID);
    InFile = fopen("EntryAction_UID.tmp","r");
    fscanf(InFile,"%s", &UID);
    fclose(InFile); 
    remove("EntryAction_UID.tmp");
    /*
    if chair created entry correctly, we should see the invation and accept it!
    Note: for a recurrence meeting, attendee will receive only one invitation.
    */
    error = GetAndAcceptNewInvitationsFromYesterday(hDB);
    if(error != NOERROR)
    {
        PRINTLOG("GetAndAcceptNewInvitationsFromYesterday error!");
        goto ERROR_EXIT;
    }

    PRINTLOG("Meeting Invitation came from %s to %s\n", pOrganizer, pAttendee);

    /*
    decline meeting at this day.
    */
    error = TestCalEntryActionDeclineExt(hDB, UID, pRecurrenceID);
    if(error != NOERROR)
    {
        PRINTLOG("TestCalEntryActionDeclineExt error!");
        goto ERROR_EXIT;
    }

    //should not found
    error = TestCalReadEntryExt(hDB, UID, pRecurrenceID);
    if(ERR(error) == 0x404)
    {
        PRINTLOG("Instance was deleted successfully so we can not find it now!\n");
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

