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

    PROGRAM:    entrymanip

    FILE:       entrymanip.c

    SYNTAX:     entrymanip  <mailbox filename>

    PURPOSE:    Demonstrates create,read,update,delete of an iCalendar
                entry.

    DESCRIPTION:

        This program processes the following logic:

        .  Call TestCalCreateEntry to create an iCalendar entry in the
           mailbox.After this, you will see the entry in the calendar.
           if RRULE included, the entry repates.         
           
        .  Call TestCalUpdateEntry to update the entry with some 
           comments, ie, delayed for some reason.

        .  Call TestCalReadEntry to read the entry.
           After read, you will see some information of the entry.
           The flag may have impact on the output, see calapi.h for 
           more.

        .  Call TestCalEntryActionDelete to delete the entry.
           We call CalEntryAction to delete the entry.
           Actually, more can be done with this function. see calapi.h
           for more.

***************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

#include "global.h"
#include "oserr.h"
#include "osfile.h"

/*#include "csinterface.hpp" */
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
/*#include "timelist.h"*/
#include "../../printLog.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CRLF "\n"
#define ICAL_UID

#define OUTFILE       "entryManip_outfile.tmp"

FILE        *gOutFP=NULL;
FILE        *gInFP=NULL;

#if defined(LINUX)

STATUS removeSpecialChar(char* inputStr)
{
    const int slen = strlen(inputStr);
    char tmpStr[slen+1];
    char *sztmpStr = tmpStr;
    char *szbase = inputStr;
    int len = 0;
    memset(tmpStr, '\0', sizeof(tmpStr));

    while (inputStr != NULL && len < slen )
    {
        if (*inputStr != '\r' ) 
        {
            *sztmpStr = *inputStr;
            sztmpStr++;
        }
        inputStr++;
        len++;
    }
    tmpStr[len]='\0';
    strcpy(szbase, tmpStr);
    return NOERROR;
}
#endif

STATUS GetDBHdl(char* pszPath, DBHANDLE* phDB)
{
    STATUS error = NOERROR;
    char fullPath[MAXPATH] = {0};
    char szInternetPath[MAXPATH] = {0}; /* To store the TCP host name */
    char szHostName[MAXPATH] = {0}; /* To store the TCP host name */
    char szDomainName[MAXPATH] = {0}; /* To store the TCP host name */

    if (error=OSPathNetConstruct(NULL, NULL, pszPath, fullPath))
    {
        PRINTERROR(error,"OSPathNetConstruct");
        return error;
    }

    error = NSFDbOpen(fullPath, phDB);

    if (error != NOERROR)
    {
        PRINTERROR(error,"NSFDbOpen");
        return error;
    }

    /* Check whether a database is remote or local */
    if (NSFDbIsRemote(*phDB))
    {
        /* To get the TCP host name of the given DB */
        error = NSFDbGetTcpHostName(*phDB, szHostName, sizeof(szHostName)-1, 
                                    szDomainName, sizeof(szDomainName)-1, 
                                    szInternetPath, sizeof(szInternetPath)-1);
        if (!error)
        {
           PRINTLOG("\nGiven DB [%s] is on a remote server. Remote server details:  TCP name: [%s], Host name: [%s]," \
                  " Domain name: [%s].\n", pszPath, szInternetPath, szHostName, szDomainName);
        }
        else
        {
           PRINTERROR(error,"NSFDbGetTcpHostName");
        }

    } 

    /* Check whether the database has full access using NSFDbHasFullAccess */
    if (NSFDbHasFullAccess(*phDB))
    {
        PRINTLOG("\nDB [%s] has full access.\n", fullPath);
    }
    else
    {
       PRINTLOG("\nDB [%s] doesn't have full access.\n", fullPath);
    }

    return error;
}

STATUS TestCalCreateEntry(DHANDLE hDB, char* pszICalender)
{
    STATUS   error = NOERROR;

    DWORD dwFlags = 0;
    MEMHANDLE hRetUID = NULL;
    char* pszRetUID = NULL;
    error = CalCreateEntry(hDB, pszICalender, dwFlags, &hRetUID, NULL);
    if(error != NOERROR)
    {
        PRINTERROR(error,"CalCreateEntry");
        return error;
    }

    pszRetUID = (char*)OSMemoryLock(hRetUID);
    gOutFP = fopen( OUTFILE, "w" );
    fprintf (gOutFP, "%s\n", pszRetUID);
    fclose(gOutFP); 
    	
    PRINTLOG("UID returned %s \n", pszRetUID);

    OSMemoryUnlock(hRetUID);
    OSMemoryFree(hRetUID);

    return error;
}

STATUS TestCalUpdateEntry(DHANDLE hDB, char* pszICalender)
{
    STATUS   error = NOERROR;

    DWORD dwFlags = 0;

    error = CalUpdateEntry(hDB, pszICalender, NULL, NULL, "Delayed for some reason...", dwFlags, NULL);
    if(error != NOERROR)
    {
        PRINTERROR(error,"CalUpdateEntry");
        return error;
    }

    PRINTLOG("CalUpdateEntry success!\n");

    return error;
}

STATUS TestCalReadEntry(DHANDLE hDB, char* pszICalenderUID)
{
    STATUS   error = NOERROR;

    MEMHANDLE hCalData = NULL;
    DWORD dwFlags = CAL_READ_INCLUDE_X_LOTUS;
    char* pszCalData = NULL;

    error = CalReadEntry(hDB, pszICalenderUID, NULL, &hCalData, NULL, dwFlags, NULL);
    if(error != NOERROR)
    {
        PRINTERROR(error,"CalReadEntry");
        return error;
    }

    pszCalData = (char*)OSMemoryLock(hCalData);

    #if defined (LINUX)
    {
        if (!removeSpecialChar(pszCalData)) 
            PRINTLOG( "Calender data returned:\n%s\n",pszCalData);
    }
    #else
        PRINTLOG("Calender data returned:\n%s\n", pszCalData);
    #endif

    OSMemoryUnlock(hCalData);
    OSMemoryFree(hCalData);

    return error;
}

STATUS TestCalEntryActionDelete(DHANDLE hDB, char* pszICalenderUID)
{
    STATUS   error = NOERROR;

    DWORD dwAction = CAL_PROCESS_DELETE;
    DWORD dwRange = 0;
    PEXT_CALACTION_DATA pExtActionInfo = NULL;

    error = CalEntryAction(hDB, pszICalenderUID, NULL, dwAction, dwRange, "Cancled", pExtActionInfo, 0, NULL);

    if (error != NOERROR)
    {
        PRINTERROR(error,"CalEntryAction");
        return error;
    }
    else
        PRINTLOG("\nCalActionDelete success!\n");

    return error;
}

int main(int argc, char *argv[])
{
    char UID[MAXPATH] =  {0};
    /* example from rfc5545 */
    /* start at 20121122T100000Z and end at 20121122T120000Z */
    char pszICalendar[] = "BEGIN:VCALENDAR" CRLF
                          "VERSION:2.0" CRLF
                          "PRODID:-//hacksw/handcal//NONSGML v1.0//EN" CRLF
                          "BEGIN:VEVENT" CRLF
                          "UID:" ICAL_UID CRLF
                          "DTSTAMP:20121122T172345Z" CRLF
                          "DTSTART:20121122T100000Z" CRLF
                          "DTEND:20121122T120000Z" CRLF
                          /*"RRULE:FREQ=DAILY;COUNT=10" CRLF*/
                          "SUMMARY:Bastille Day Party" CRLF
                          "END:VEVENT" CRLF
                          "END:VCALENDAR" CRLF;

    /* delayed */
    /* start at 20121122T180000Z and end at 20121122T210000Z */
    char pszICalendarDelayedTemp[] = "BEGIN:VCALENDAR" CRLF
                                     "VERSION:2.0" CRLF
                                     "PRODID:-//hacksw/handcal//NONSGML v1.0//EN" CRLF
                                     "BEGIN:VEVENT" CRLF
                                     "UID: %s" CRLF
                                     "DTSTAMP:20121122T172345Z" CRLF
                                     "DTSTART:20121122T180000Z" CRLF
                                     "DTEND:20121122T210000Z" CRLF
                                     "SUMMARY:(Delayed)Bastille Day Party" CRLF
                                     "END:VEVENT" CRLF
                                     "END:VCALENDAR" CRLF;

    char pszICalendarDelayed[1024] = {0};
    STATUS error = NOERROR;
    DBHANDLE hDB = NULLHANDLE;

    error = NotesInitExtended (argc, argv);
    if ( error != NOERROR )
    {
        PRINTLOG("\n Unable to initialize Notes. Error Code[0x%04x]\n", error);    
        goto ERROR_EXIT;
    }
    if(argc != 2)
    {
        PRINTLOG("Usage: %s mailfileName\n", argv[0]);
        NotesTerm();
        return 0;
    }
    error = GetDBHdl(argv[1], &hDB);
    if(error != NOERROR)
    {
        PRINTLOG("GetDBHdl error!");
        NotesTerm();
        return error;
    }

    /* if create success, you will see the entry in Calendar. */
    error = TestCalCreateEntry(hDB, pszICalendar);
    if(error != NOERROR)
    {
        PRINTLOG("TestCalCreateEntry error!");
        goto ERROR_EXIT;
    }

    gInFP = fopen(OUTFILE,"r");
    fscanf(gInFP,"%s", &UID);
    fclose(gInFP);

    /* To clean up the temp file created to store the UID after creation */

    remove(OUTFILE);
    sprintf(pszICalendarDelayed, pszICalendarDelayedTemp, UID);

    error = TestCalUpdateEntry(hDB, pszICalendarDelayed);
    if(error != NOERROR)
    {
        PRINTLOG("TestCalUpdateEntry error!");
        goto ERROR_EXIT;
    }

    error = TestCalReadEntry(hDB, UID);
    if(error != NOERROR)
    {
        PRINTLOG("TestCalUpdateEntry error!");
        goto ERROR_EXIT;
    }

    error = TestCalEntryActionDelete(hDB, UID);
    if(error != NOERROR)
    {
        PRINTLOG("TestCalEntryActionDelete error!");
        goto ERROR_EXIT;
    }
ERROR_EXIT:
    if(hDB)
    {
        NSFDbClose(hDB);
        NotesTerm();
    }

    return error;
}
