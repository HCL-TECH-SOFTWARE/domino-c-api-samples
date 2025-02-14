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
 */

#define STRING_LENGTH 256
#define SCHEDULEITEMS 30  /* number of schedule Items to add to a note */
#define EVENTS        2

/* structure for a schedule item */
typedef struct Schedule_Item
{
  char * ItemName;
  char * ItemValue;
  WORD ItemType;
} SCHEDULE_ITEM;

/* constants defining Schedule Item names */
#define ALARM               0
#define ALARMDESCRIPTION    1 
#define ALARMOFFSET         2  
#define APPOINTMENTTYPE     3
#define APPTUNID            4
#define BODY                5
#define BOOKFREETIME        6
#define BUSYNAME            7 
#define BUSYPRIORITY        8 
#define CALENDARDATETIME    9
#define CHAIR              10
#define COPYTO             11 
#define ENDDATE            12
#define ENDDATETIME        13
#define ENDTIME            14
#define EXCLUDEFROMVIEW    15
#define EXPANDGROUPS       16
#define FORM               17
#define FROM               18
#define NOPURGE            19         
#define ORGTABLE           20
#define PRINCIPAL          21
#define PUBLICACCESS       22
#define SENDTO             23
#define SEQUENCENUM        24
#define STARTDATE          25
#define STARTDATETIME      26
#define STARTTIME          27
#define SUBJECT            28
#define VIEWICON           29


/* constants defining Schedule commands */
#define ADD                 0
#define DEL                 1
#define BUSY                2

#define APPT                0
#define INVITATION          1


int EventType; /* APPT, INVITATION, etc */

/* table containing each SCHEDULE_ITEM values */

SCHEDULE_ITEM ScheduleItemTable[EVENTS][SCHEDULEITEMS]={

/*  Items for Appointment  */

{"$Alarm","1.000000",TYPE_NUMBER,
"$AlarmDescription","Ring Me!",TYPE_TEXT,
"$AlarmOffset","-5.000000",TYPE_NUMBER,
"AppointmentType","0",TYPE_TEXT,
"apptUNID","apptUNID",TYPE_TEXT,
"Body","This test appointment is scheduled thru Notes C API.",TYPE_COMPOSITE,
"BookFreeTime","",TYPE_TEXT,
"$BusyName","$BusyName",TYPE_TEXT,
"$BusyPriority","1",TYPE_TEXT,
"CalendarDateTime","CalendarDateTime",TYPE_TIME,
"Chair","Chair",TYPE_TEXT,
"CopyTo","",TYPE_TEXT,
"EndDate","EndDate",TYPE_TIME,
"EndDateTime","EndDateTime",TYPE_TIME,
"EndTime","EndTime",TYPE_TIME,
"ExcludeFromView","D",TYPE_TEXT,
"$ExpandGroups","3",TYPE_TEXT,
"Form","Appointment",TYPE_TEXT,
"From","From",TYPE_TEXT,
"$NoPurge","$NoPurge",TYPE_TIME,
"ORGTABLE","C0",TYPE_TEXT,
"Principal","Principal",TYPE_TEXT,
"$PublicAccess","1",TYPE_TEXT,
"SendTo","",TYPE_TEXT,
"SEQUENCENUM","1.000000",TYPE_NUMBER,
"StartDate","StartDate",TYPE_TIME,
"StartDateTime","StartDateTime",TYPE_TIME,
"StartTime","StartTime",TYPE_TIME,
"Subject","This is a Test Appointment",TYPE_TEXT,
"_ViewIcon","160.000000",TYPE_NUMBER},

/*  Items for Invitation  */

{"$Alarm","1.000000",TYPE_NUMBER,
"$AlarmDescription","Ring Me",TYPE_TEXT,
"$AlarmOffset","-5.000000",TYPE_NUMBER,
"AppointmentType","3",TYPE_TEXT,
"apptUNID","apptUNID",TYPE_TEXT,
"Body","Meeting agenda will soon be distributed",TYPE_COMPOSITE,
"BookFreeTime","",TYPE_TEXT,
"$BusyName","$BusyName",TYPE_TEXT,
"$BusyPriority","1",TYPE_TEXT,
"CalendarDateTime","CalendarDateTime",TYPE_TIME,
"Chair","Chair",TYPE_TEXT,
"CopyTo","An Optional Person",TYPE_TEXT,
"EndDate","EndDate",TYPE_TIME,
"EndDateTime","EndDateTime",TYPE_TIME,
"EndTime","EndTime",TYPE_TIME,
"ExcludeFromView","D",TYPE_TEXT,
"$ExpandGroups","3",TYPE_TEXT,
"Form","Appointment",TYPE_TEXT,
"From","From",TYPE_TEXT,
"$NoPurge","$NoPurge",TYPE_TIME,
"ORGTABLE","C0",TYPE_TEXT,
"Principal","Principal",TYPE_TEXT,
"$PublicAccess","1",TYPE_TEXT,
"SendTo","A Primary Person",TYPE_TEXT,
"SEQUENCENUM","1.000000",TYPE_NUMBER,
"StartDate","StartDate",TYPE_TIME,
"StartDateTime","StartDateTime",TYPE_TIME,
"StartTime","StartTime",TYPE_TIME,
"Subject","This is a Test Meeting",TYPE_TEXT,
"_ViewIcon","158.000000",TYPE_NUMBER}
};


/* define global variables */
char   szServerName[STRING_LENGTH];
char   szFullName[STRING_LENGTH];
char   szFullMailFileName[STRING_LENGTH];
char   szMailFileName[STRING_LENGTH];
char   *time_start;
char   *time_end;
char   startTime[STRING_LENGTH];
char   endTime[STRING_LENGTH];
char   AUser[STRING_LENGTH];
char   szCmd[STRING_LENGTH];
char   szTemp[MAXALPHATIMEDATE+1];
char   szTimedate[MAXALPHATIMEDATE+1];
char   szUpperTD[MAXALPHATIMEDATE+1];
 
/* define functions */

void  LNPUBLIC  ProcessArgs (int argc, char *argv[], char *cmd,
                             char *server, char *mail_file, 
                             char *start_time, char *end_time);
STATUS LNPUBLIC  ScheduleTask(int command);
STATUS LNPUBLIC  AddSchedule(void);
STATUS LNPUBLIC  FindSchedTime( char * pData);
STATUS LNPUBLIC  FindSchedEntry( char *pSchedList);
STATUS LNPUBLIC  DeleteSchedEntry( UNID *pUnid);
void   LNPUBLIC  DumpTimeRangeItem( char * pData);
void   LNPUBLIC  GetTDString( TIMEDATE * ptdModified, char * szTimedate );
BOOL   LNPUBLIC  CreateCDRecords(char *pData, WORD *pwDataLen, 
                             char * wstring);
BOOL   CDPutPabdef(WORD para_num, BYTE **c_buffer_ptr);
BOOL   CDPutPabref(WORD para_num, BYTE **c_buffer_ptr);
BOOL   CDPutText(char *text_buf, WORD text_size, BYTE **cd_buffer_ptr);
