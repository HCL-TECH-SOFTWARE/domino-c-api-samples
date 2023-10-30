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

    PROGRAM:      schedule

    FILE:         schedule.c

    SYNTAX:       schedule <command> <srvr> <mail file> <start_time> <end_time>

    PURPOSE:      demonstrates calendar and scheduling functionality.

    DESCRIPTION:  provides commands to add an appointment, or an invitation,
                  or delete the scheduled event in a User's
                  mail database and display a User's busy and free times.

                  <command>     add-i add-a del  busy
                  <srvr>        the server the User's mail database resides in.
                                (ex. zathras ) (enter "" for local)
                  <mail file>   the user's mail file name (ex. mail\jdoe.nsf)
                  <start_time>  beginning time to add, delete, or view busy/free
                                time (ex. "09/16/04 09:30 am").
                  <end_time>    end time (ex. "09/16/04 02:30 pm")

 ***************************************************************************/

/*
 * Windows header file. Include only if compiling for Windows.
 */

#if(defined(NT))
#include <windows.h>
#endif

/* C header files     */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* HCL C API for Notes/Domino Header files.  */


#include <global.h>
#include <nsfdata.h>
#include <kfm.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <nsferr.h>
#include <editods.h>
#include <stdnames.h>
#include <miscerr.h>
#include <osmem.h>
#include <oserr.h>
#include <osfile.h>
#include <schedule.h>
#include <schgtw.h>
#include <textlist.h>

#include "../../printLog.h"
#include <osmisc.h>


/* Sample's Header file.  */
#include "sched.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif


/************************************************************************

    FUNCTION:    Main

*************************************************************************/

int main(int argc, char *argv[])
{
    STATUS  sError = NOERROR;

    /*
     *  Initialize Domino and Notes.
     */

	if (sError = NotesInitExtended (argc, argv))
	{
	    PRINTLOG("\n Unable to initialize Notes.\n");
	    return (1);
	}

    /*
     *  Process Command line arguments...
     */

    time_start = startTime;
    time_end  = endTime;


    /* get command, server name, mail file name, start and end times from
       the command line */
    ProcessArgs(argc, argv, szCmd, szServerName, szMailFileName,
                time_start, time_end);

    szFullMailFileName[0]='\0';

    strcpy(szFullMailFileName, szMailFileName);


   /*
    * Get full distinguished user name
    */
    SECKFMUserInfo(KFM_ui_GetUserInfo, AUser, NULL);
    PRINTLOG("User name in full distinguished format is : %s\n",AUser);


   /*
    * Get full pathname
    */
    sError = OSPathNetConstruct(NULL, szServerName, szFullMailFileName,
                                szFullName);

    if (sError)
    {
      PRINTERROR (sError,"OSPathNetConstruct");
      NotesTerm();
      return (1);
    }

   /*
    * Call Schedule routine
    */

    if (strcmp(szCmd,"add-a")==0)
    {
        EventType=APPT;
        sError = ScheduleTask(ADD);
    }
    else if (strcmp(szCmd,"add-i")==0)
    {
        EventType=INVITATION;
        sError = ScheduleTask(ADD);
    }
    else if (strcmp(szCmd, "del")==0)
      sError = ScheduleTask(DEL);

    else if (strcmp(szCmd, "busy")==0)
      sError = ScheduleTask(BUSY);

    else
    {
        PRINTLOG ("Invalid Schedule Command %s.\n",szCmd);
        PRINTERROR (sError,"OSPathNetConstruct");
        NotesTerm();
        return (1);
    }

    if (!sError)
    {
        PRINTLOG("\n\nProgram completed successfully.\n");
    }	
    else
    {
        PRINTERROR (sError,"OSPathNetConstruct");
    }


    NotesTerm();
    return (0);

}

/************************************************************************

    FUNCTION:     ScheduleTask

    PURPOSE:      This routine will add a schedule, delete a schedule or
                  display busy and free time depending on the command
                  argument.

    DESCRIPTION:  Command ADD    - adds an appointment or a meeting
                                       invitation
                  Command DEL    - deletes a scheduled event
                  Command BUSY   - displays busy and free time.

*************************************************************************/

STATUS LNPUBLIC ScheduleTask(int command)
{

    STATUS                      sError = NOERROR;
    char                        *theData;
    char                        *theSchedList;
    TIMEDATE_PAIR               theApptInterval;
    TIMEDATE_PAIR               pInterval;
    VOID                        *list_ptr;            /* pointer to same list */
    WORD                        list_size;            /* total size of list   */
                                                      /* structure */
    DWORD                       dwReserved = 0;
    SCHEDULE                    *pSchedule;
    DHANDLE                       rethRange;
    UNID                        theApptUnid;
    HCNTNR                      rethCntnr = NULLHANDLE; /* container handle */
    DHANDLE                       list_handle;         /* handle to Domino and Notes list */
                                                     /* structure */
    DHANDLE                       rethSchedList;
    HCNTNROBJ                   hSchedObj;
    HCNTNROBJ                   hMoreObj = NULLCNTNROBJ;
    DWORD                       retdwSize;
    DWORD                       SchrqstLocalOrRemote;

    /* ADD the appointment or the meeting invitation to the User's schedule. */

    if (command == ADD)
    {
      sError = AddSchedule();
      return (sError);
    }

    /* Create an empty text list data structure. */

    sError = ListAllocate (0, 0, FALSE, &list_handle, &list_ptr, &list_size);

    if (sError)
      goto Done;



    /* Add the user we're looking for */
    sError = ListAddEntry(list_handle, FALSE, &list_size, 1, AUser,
                          (WORD) strlen(AUser));

    if (sError)
      goto Done;


    /* convert start time string to TIMEDATE format */
    sError = ConvertTextToTIMEDATE(NULL,
                                   NULL,
                                   &time_start,
                                   (WORD) strlen(startTime),
                                   &theApptInterval.Lower);
    if (sError)
      goto Done;

    /* convert end time string to TIMEDATE format */
    sError = ConvertTextToTIMEDATE(NULL,
                                   NULL,
                                   &time_end,
                                   (WORD) strlen(endTime),
                                   &theApptInterval.Upper);
    if (sError)
      goto Done;

    OSUnlockObject(list_handle);

    list_ptr = OSLockObject(list_handle);

    /* Retrieve the user's schedule container */
    if (szServerName==NULL)
                SchrqstLocalOrRemote=SCHRQST_LOCAL;        /* local mail  */
    else
                SchrqstLocalOrRemote=SCHRQST_FORCEREMOTE;  /* server mail */

    sError = SchRetrieve(&theApptUnid, NULL,
                         SCHRQST_EACHPERSON|SchrqstLocalOrRemote,
                         &theApptInterval, (LIST *)list_ptr, &rethCntnr,
                         NULL, NULL, NULL);

    if (sError)
      goto Done;

    /* Get the first schedule */
    sError = SchContainer_GetFirstSchedule(rethCntnr, &hSchedObj, &pSchedule);

    if (sError)
      goto Done;

    /* display BUSY free time information */
    if (command == BUSY)
      PRINTLOG("\n\nThis is the first free time available for the user: %s\n",
                   AUser);

    /* Get the first free time available */
    if (sError = Schedule_ExtractFreeTimeRange(rethCntnr, hSchedObj,
                   &theApptUnid, TRUE, 60, &pInterval,
                   (DWORD *) &retdwSize, &rethRange))
      goto Done;

    theData = (char *)OSLockObject(rethRange);

    /* display BUSY free time information */
    if (command == BUSY)
      /* Output that free time information */
      DumpTimeRangeItem( theData);

    OSUnlockObject(rethRange);

    /* Extract the busy times */
    sError = Schedule_ExtractBusyTimeRange(rethCntnr, hSchedObj,
                &theApptUnid, &pInterval, &retdwSize,
                &rethRange, &hMoreObj);

    if (sError)
      goto Done;

    theData = (char *)OSLockObject(rethRange);

    /* display BUSY free time information */
    if (command == BUSY)
      PRINTLOG("\nThese are the busy times for the user: %s\n", AUser);

    /* display BUSY free time information */
    if (command == BUSY)
    {
      /* Output the busy times to the console */
      DumpTimeRangeItem( theData);
      OSUnlockObject(rethRange);
      goto Done;
    }

    /* Attempt to find schedule time */
    sError = FindSchedTime(theData);

    OSUnlockObject(rethRange);

    if (sError)
      goto Done;

    /* Extract a schedule list */
    sError = Schedule_ExtractSchedList(rethCntnr, hSchedObj,
               &pInterval,  &retdwSize, &rethSchedList,
               &hMoreObj);

    if (sError)
      goto Done;

    theSchedList = (char *)OSLockObject(rethSchedList);

    /* Attempt to find schedule entry in list */
    sError = FindSchedEntry(theSchedList);

    OSUnlockObject(rethSchedList);

    /* Schedule_Free(rethCntnr,hSchedObj); */
    SchContainer_Free(rethCntnr);

  Done:
    OSUnlockObject(list_handle);
    return (sError);

}


/************************************************************************

    FUNCTION:     AddSchedule

    PURPOSE:      Shows how to add an appointment or a meeting
                      invitation to a User's schedule.

    DESCRIPTION:  Adds an appointment to a User's schedule in the User's mail
                  database.  The Items included in the ScheduleItemTable
                  are appended to a newly created note of
                  class NOTE_CLASS_DOCUMENT to add an event.

*************************************************************************/

STATUS LNPUBLIC AddSchedule()
{
    NOTEHANDLE    hNote;
    STATUS        sError = NOERROR;
    DBHANDLE      hDB;
    int           i;
    char          *pData;
    char          *stopstring;
    TIMEDATE      theStartTime;
    TIMEDATE      theEndTime;
    WORD          wDataLen;
    DWORD         dwDataLen;
    DHANDLE         hData;
    WORD          ScheduleFlags;
    WORD          ScheduleType;
    WORD          ClassDoc = NOTE_CLASS_DOCUMENT;
    double        ViewIconNumber;
    char far      uidText[MAXALPHATIMEDATE];
    ORIGINATORID  oid;

    sError = ConvertTextToTIMEDATE(NULL,
                                         NULL,
                                         &time_start,
                                         (WORD) strlen(startTime),
                                         &theStartTime);
    if (sError)
        goto Exit0;


    sError = ConvertTextToTIMEDATE(NULL,
                                         NULL,
                                         &time_end,
                                         (WORD) strlen(endTime),
                                         &theEndTime);
    if (sError)
        goto Exit0;

    /*
     * Open database
     */

    sError = NSFDbOpen(szFullName, &hDB);

    if (sError)
      goto Exit0;

    /*
     * Now create a note in database
     */

    sError = NSFNoteCreate(hDB, &hNote);

    if (sError)
      goto Exit1;

    /*
     * Set the NOTE_CLASS to NOTE_CLASS_DOCUMENT.
     */

    NSFNoteSetInfo( hNote, _NOTE_CLASS, &ClassDoc );

   /*
    *  First, allocate a data buffer and get a pointer to it.
    */

    sError = OSMemAlloc(0, MAXONESEGSIZE, &hData);

    if (sError)
      goto Exit2;

    pData = (char*)OSLockObject(hData);

   /*
    *  Loop through number of items, switch on Items in Table, and add
    *  them to the Note.  Skip those items with ItemName="NULL", those
    *  items should not be used for this EventType.
    */

    for (i=0;i<SCHEDULEITEMS; i++)
    {
       /* Skip the item not belong to this Event type */
       if (strcmp(ScheduleItemTable[EventType][i].ItemName,"NULL"))
       {
          ScheduleType = ScheduleItemTable[EventType][i].ItemType;
          switch(i)
          {
            case CALENDARDATETIME:
            case STARTDATE:
            case STARTDATETIME:
            case STARTTIME:
            {
               ScheduleFlags = ITEM_SUMMARY;

               /* copy data to buffer */
               wDataLen = sizeof(TIMEDATE);
               memset( pData, 0, (size_t)wDataLen );
               memcpy((TIMEDATE *)pData, &theStartTime, sizeof(TIMEDATE));
               dwDataLen = (DWORD)wDataLen;

               break;
            }

            case NOPURGE:
            case ENDDATE:
            case ENDDATETIME:
            case ENDTIME:
            {

               ScheduleFlags = ITEM_SUMMARY;

               /* copy data to buffer */
               wDataLen = sizeof(TIMEDATE);
               memset( pData, 0, (size_t)wDataLen );
               memcpy((TIMEDATE *)pData, &theEndTime, sizeof(TIMEDATE));
               dwDataLen = (DWORD)wDataLen;

               break;
            }

            case BUSYNAME:
            case CHAIR:
            case PRINCIPAL:
            {
               ScheduleFlags = ITEM_SUMMARY;

               wDataLen = strlen(AUser);
               memset( pData, 0, (size_t)wDataLen );

               memcpy((char *)pData, AUser, wDataLen);
               dwDataLen = (DWORD)wDataLen;

               break;
            }

            case ALARMDESCRIPTION:
            case APPOINTMENTTYPE:
            case BOOKFREETIME:
            case BUSYPRIORITY:
            case COPYTO:
            case EXCLUDEFROMVIEW:
            case EXPANDGROUPS:
            case FORM:
            case ORGTABLE:
            case PUBLICACCESS:
            case SENDTO:
            case SUBJECT:
            {
               ScheduleFlags = ITEM_SUMMARY;

               wDataLen = strlen(ScheduleItemTable[EventType][i].ItemValue);
               memset( pData, 0, (size_t)wDataLen );

               memcpy((char *)pData, ScheduleItemTable[EventType][i].ItemValue,
                              wDataLen);
               dwDataLen = (DWORD)wDataLen;

               break;
            }

            case FROM:
            {
               ScheduleFlags = ITEM_SUMMARY|ITEM_READWRITERS|ITEM_NAMES;

               wDataLen = strlen(AUser);
               memset( pData, 0, (size_t)wDataLen );
               memcpy((char *)pData, AUser, wDataLen);
               dwDataLen = (DWORD)wDataLen;

               break;
            }
            case ALARM:
            case ALARMOFFSET:
            case SEQUENCENUM:
            case VIEWICON:
            {
               /* convert string to double */
               ViewIconNumber = strtod(ScheduleItemTable[EventType][i].ItemValue, &stopstring);
               ScheduleFlags = ITEM_SUMMARY;
               wDataLen = sizeof(double);
               memset( pData, 0, (size_t)wDataLen );
               memcpy((char *)pData, &ViewIconNumber, wDataLen);
               dwDataLen = (DWORD)wDataLen;

               break;
            }

            case APPTUNID:
            {
               NSFNoteGetInfo(hNote, _NOTE_OID, &oid);
               sprintf(uidText,"%.8lx%.8lx%.8lx%.8lx",
                                oid.File.Innards[1],
                                oid.File.Innards[0],
                                oid.Note.Innards[1],
                                oid.Note.Innards[0]);
               wDataLen=32;
               memset( pData, 0, (size_t)wDataLen );
               memcpy((char *)pData, uidText, wDataLen);
               dwDataLen = (DWORD)wDataLen;

               break;
            }

            case BODY:
            {
               ScheduleFlags = ITEM_SUMMARY;
               if (!(CreateCDRecords(pData,&wDataLen,
                                   ScheduleItemTable[EventType][i].ItemValue)))
                  goto Exit3;
               dwDataLen = (DWORD)wDataLen;
               break;
            }

            default:
               break;
         }  /* switch */

        /* append the Schedule Item to the Note. */
        if ( ScheduleType == TYPE_TEXT_LIST)
            sError = NSFItemAppendTextList(hNote,
                             ScheduleItemTable[EventType][i].ItemName,
                             pData,
                             (WORD) dwDataLen,
                             FALSE);
        else
            sError = NSFItemAppend(hNote,
                             ScheduleFlags,
                             ScheduleItemTable[EventType][i].ItemName,
                             (WORD) strlen(ScheduleItemTable[EventType][i].ItemName),
                             ScheduleType,
                             (void far *)pData,
                             dwDataLen);

       if (sError)
         goto Exit3;
       }  /* if ItemName != NULL */

    } /* end of for */

    sError = NSFNoteUpdate( hNote, 0 );
    PRINTLOG("\n Scheduled meeting for %s between %s to %s ", szMailFileName,startTime,endTime);
Exit3:

    OSUnlockObject (hData);
    OSMemFree (hData);

Exit2:

    NSFNoteClose( hNote );

Exit1:

    NSFDbClose( hDB );

Exit0:

    return (sError);
	
}


BOOL LNPUBLIC CreateCDRecords(char *pData, WORD *wDataLen, char * wstring)
{
   BYTE * rt_field;
   BYTE * buff_ptr;
   WORD   tmpLen;

   tmpLen =  ODSLength(_CDPABDEFINITION) +
             ODSLength(_CDPABREFERENCE)  +
             ODSLength(_CDTEXT)          +
             strlen(wstring)+strlen(wstring)%2;

   *wDataLen=tmpLen;

   rt_field=(BYTE *) malloc( tmpLen);
   if (rt_field == (BYTE *) NULL)
   {
        PRINTLOG("Error: unable to allocate %d bytes memory\n",tmpLen);
        return(FALSE);
   }

   buff_ptr=rt_field;

   /* Create PABDEF */
   if (!(CDPutPabdef(1,&buff_ptr)))
   {
        free(rt_field);
        return(FALSE);
   }

   /* Create PABREF */
   if (!(CDPutPabref(1,&buff_ptr)))
   {
        free(rt_field);
        return(FALSE);
   }

   /* Create Text   */
   if (!(CDPutText(wstring,(WORD) strlen(wstring),&buff_ptr)))
   {
        free(rt_field);
        return(FALSE);
   }

   memcpy(pData,rt_field,tmpLen);

   free(rt_field);

   return(TRUE);
}

/************************************************************************

    FUNCTION:     FindSchedTime

    DESCRIPTION:  Searches the busy time information for a specific
                  scheduled event and returns 0 if found.

*************************************************************************/

STATUS LNPUBLIC FindSchedTime( char * pData)
{
    RANGE               Range;
    TIMEDATE_PAIR       TimeDatePair;
    USHORT              usRangeEntry;
    char                szTimedate[MAXALPHATIMEDATE+1];
    char                szUpperTD[MAXALPHATIMEDATE+1];
    char                szAMPM[MAXALPHATIMEDATE+1];
    STATUS              sError=NOERROR;

    /* add additional zeros to startTime string (from 00:00 AMPM to
       00:00:00 AMPM) */

    /* copy AM or PM time into string */
    strcpy(szAMPM,&startTime[strlen(startTime)-2]);

    /* make sure AMPM is in upper case */
    szAMPM[strlen(szAMPM)-1]=toupper(szAMPM[strlen(szAMPM)-1]);
    szAMPM[strlen(szAMPM)-2]=toupper(szAMPM[strlen(szAMPM)-2]);

    /* copy startTime string */
    strcpy(szTemp,startTime);

    /* trunc string up to 00:00 (from 00:00 AMPM) and add another
       colon : (00:00:) */
    szTemp[strlen(szTemp)-1]='\0';
    szTemp[strlen(szTemp)-1]='\0';
    szTemp[strlen(szTemp)-1]=':';

    /* add additional zeros (now we have 00:00:00 ) */
    strcat(szTemp,"00 ");

    /* add AMPM (now we have 00:00:00 AMPM) */
    strcat(szTemp,szAMPM);

    /* Get the RANGE from the location specified by pData */
    memcpy( (char*)&Range, pData, sizeof(RANGE) );

    /* Advance pData to point to the first TIMEDATE in the array */
    pData += sizeof(RANGE);

    for (usRangeEntry = 0; usRangeEntry < Range.RangeEntries; usRangeEntry++)
    {
      memcpy( (char*)&TimeDatePair, pData, sizeof(TIMEDATE_PAIR) );
      pData += sizeof(TIMEDATE_PAIR);
      GetTDString( &TimeDatePair.Lower, szTimedate );
      if (strcmp(szTimedate,szTemp)==0)
        return(NOERROR);

      GetTDString( &TimeDatePair.Upper, szUpperTD );
    }

    return(ERR_SCHED_CANTFINDSCHEDULE);
}

/************************************************************************

    FUNCTION:   GetTDString

*************************************************************************/

void LNPUBLIC GetTDString( TIMEDATE * ptdModified, char * szTimedate )
{
    WORD  wLen;

    ConvertTIMEDATEToText( NULL, NULL,
                           ptdModified,
                           szTimedate,
                           MAXALPHATIMEDATE,
                           &wLen );

    szTimedate[wLen] = '\0';
    return;
}


/************************************************************************

    FUNCTION:     FindSchedEntry

    DESCRIPTION:  Searches the schedule list for a specific scheduled
                  entry.  If the entry is found the scheduled entry is
                  deleted.

*************************************************************************/

STATUS LNPUBLIC FindSchedEntry( char *pSchedList)
{
    int i;
    int num;
    SCHED_LIST SchedList;
    SCHED_ENTRY SchedEntry;
    STATUS          sError=0;

    /* Get the SCHED_LIST from the location specified by pSchedList */
    memcpy( (char*)&SchedList, pSchedList, sizeof(SCHED_LIST) );

    pSchedList += sizeof(SCHED_LIST);

    num = SchedList.NumEntries;

    for (i=0;i<num;i++)
    {
      memcpy((char *)&SchedEntry, pSchedList, sizeof(SCHED_ENTRY) );

      GetTDString( &(SchedEntry.Unid).Note, szTimedate );
      GetTDString( &(SchedEntry).Interval.Lower, szTimedate );
      GetTDString( &(SchedEntry).Interval.Upper, szUpperTD );

      if (strcmp(szTimedate,szTemp)==0)
        sError = DeleteSchedEntry(&(SchedEntry.Unid));

      pSchedList += sizeof(SCHED_ENTRY);
    }

    return(sError);
}

/************************************************************************

    FUNCTION:   DeleteSchedEntry

*************************************************************************/
STATUS LNPUBLIC DeleteSchedEntry( UNID *pUnid)
{
    DHANDLE           hDB;
    NOTEID          retNoteId;
    OID             retOID;
    TIMEDATE        retModTime;
    WORD            retClass;
    STATUS          sError;

    /* open Database */

    sError = NSFDbOpen(szFullName, &hDB);

    if (sError)
      return(sError);

    /* Open the Note by UNID */

    sError = NSFDbGetNoteInfoByUNID(hDB, pUnid, &retNoteId, &retOID,
             &retModTime, &retClass);

    if (sError)
      return(sError);

    /* Delete Note */

    sError = NSFNoteDelete(hDB, retNoteId, 0);

    if (sError)
      return(sError);

    /* Close Database */

    sError = NSFDbClose(hDB);

    return(sError);
}

/************************************************************************

    FUNCTION:   DumpTimeRangeItem

*************************************************************************/

void LNPUBLIC DumpTimeRangeItem( char * pData)
{
    RANGE           Range;
    TIMEDATE_PAIR   TimeDatePair;
    USHORT          usRangeEntry;
    char            szTimedate[MAXALPHATIMEDATE+1];
    char            szUpperTD[MAXALPHATIMEDATE+1];

    /* Get the RANGE from the location specified by pData */
    memcpy( (char*)&Range, pData, sizeof(RANGE) );

    /* Advance pData to point to the first TIMEDATE in the array */
    pData += sizeof(RANGE);

    for (usRangeEntry = 0; usRangeEntry < Range.RangeEntries; usRangeEntry++)
    {
      memcpy( (char*)&TimeDatePair, pData, sizeof(TIMEDATE_PAIR) );
      pData += sizeof(TIMEDATE_PAIR);
      GetTDString( &TimeDatePair.Lower, szTimedate );
      GetTDString( &TimeDatePair.Upper, szUpperTD );
      PRINTLOG("%s - %s\n", szTimedate, szUpperTD );
    }

    return;
}

/************************************************************************

    FUNCTION:       ProcessArgs

    INPUTS:         argc, argv - directly from the command line

    OUTPUTS:        cmd        - The Schedule command
                    server     - The server where the user's mail file is
                    mail_file  - The name of the user's mail file
                    start_time - start time obtained from command line or
                                 from prompt.
                    end_time   - end time obtained from command line or
                                 from prompt.

*************************************************************************/

void  LNPUBLIC  ProcessArgs (int argc, char *argv[], char *cmd,
                             char *server, char *mail_file,
                             char *start_time, char *end_time)
{
    if (argc != 6)
    {
      printf("Enter schedule command (add-a, add-i, del, busy): ");
      fflush (stdout);
      gets(cmd);
      printf("Enter the name of the server where the user's mail database is: ");
      fflush (stdout);
      gets(server);
      printf("Enter the name of the user's mail database: ");
      fflush (stdout);
      gets(mail_file);
      printf("Enter the start date and time of the schedule: ");
      fflush (stdout);
      gets(start_time);
      printf ("Enter the end date and time of the schedule:  ");
      fflush (stdout);
      gets(end_time);

    }
    else
    {
      strcpy(cmd, argv[1]);
      strcpy(server, argv[2]);
      strcpy(mail_file, argv[3]);
      strcpy(start_time, argv[4]);
      strcpy(end_time, argv[5]);
    } /* end if */

} /* ProcessArgs */
