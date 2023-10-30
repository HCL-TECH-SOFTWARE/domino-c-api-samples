/************************************************************************
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
 
   PROGRAM:    alarm

   FILE:       alarm.c

   PURPOSE:    Demonstrate the Alarm APIs functionality.

   SYNTAX:     alarm
   
   COMMENTS:
          This program demonstrates the Alarm APIs functionality by
          creating and opening a message queue for Alarm messages,
          registering the client with the Alarms Daemon and waiting
          for an Alarm Message that has been previously set by the UI.
          Once the message is received the alarm information is displayed
          and the client deregisters with the Alarm Daemon.

**************************************************************************/

/* OS and C include files */

#include <stdio.h>
#include <string.h>
/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <nsfdb.h>
#include <nsfdata.h>
#include <mq.h>
#include <alarm.h>
#include <misc.h>
#include <miscerr.h>
#include <ods.h>
#include <osmem.h>
#include <osmisc.h>
#include "../../printLog.h"


#define MAX_MESSAGE  (256)
#define NOTESCLIENT  "NOTESCLIENT"

/* Message queue name */
char MsgQueueName [] = MQPREFIX_FOR_CLIENT "NOTESCLIENT";
/* Note the implicit string concatenation!! */


/* Function declarations */
STATUS ProcessAlarmMessage (char *, WORD); 
void LNPUBLIC  GetTDString( TIMEDATE *, char * );


int main(int argc, char * argv[])
{
    STATUS      error = NOERROR;             /* error code from API calls */
    MQHANDLE    hQueue;                      /* Handle to message queue */
    char        MsgBuffer [MAX_MESSAGE + 1]; /* Buffer for messages */
    WORD        MsgLen;                      /* Size of message */
    BOOL        bNO_ALARM_MSG = TRUE;
  
    /* initialize with Domino and Notes */
    if (error = NotesInitExtended (argc, argv))
    {
        PRINTLOG("\n Unable to initialize Notes.\n");
        return (1);
    }

    /* Create and open the message queue.  */
    if (error = MQOpen (MsgQueueName, MQ_OPEN_CREATE, &hQueue))
       goto Done1;
      

    /* Register the client with the Alarms Daemon */
    if (error = Alarm_RegisterInterest (NOTESCLIENT, 0))
        goto Done2;

    /* Tell the Alarms Daemon to refresh it's list of alarms */
    if (error = Alarm_RefreshAlarms (NOTESCLIENT))
        goto Done3;

    /* Main loop to process Alarm message.  We'll wait for 5 minutes for a
    message and if we don't get one, we'll time out */

    while (bNO_ALARM_MSG)
    {
        PRINTLOG("Waiting for message from the Alarms Daemon...\n");
        error = MQGet (hQueue, MsgBuffer, MAX_MESSAGE,
                       MQ_WAIT_FOR_MSG, (300 * 1000), &MsgLen);

        MsgBuffer[MsgLen] = '\0';

        if (NOERROR == error)
        {
            PRINTLOG("Received Alarm Message!\n\n");
            error = ProcessAlarmMessage (MsgBuffer, MsgLen);
            bNO_ALARM_MSG = FALSE; 
        }
        else if (ERR_MQ_TIMEOUT == error)
        {
            PRINTLOG("Received Timeout...\n");
            bNO_ALARM_MSG = FALSE; 
        }
    }   /* End of main task loop. */
 
Done3:
    /* deregister with the Alarm Daemon */
    Alarm_DeregisterInterest (NOTESCLIENT, 0);
    goto Done2;

Done2:
    /* close the message queue */
    MQClose (hQueue, 0);
    goto Done1;

Done1:
    /* End of subroutine. */
    if (error)
    {
        if (error!=999)
            PRINTERROR (error,"MQOpen");  
    }
    else
    {
        PRINTLOG("\nProgram completed successfully.\n");
    }

    NotesTerm();
    return (error); 

}


/************************************************************************

    FUNCTION:   ProcessAlarmMessage

    PURPOSE:    Process and display the Alarm Message information.
    
    NOTE:       

*************************************************************************/

STATUS ProcessAlarmMessage (char *pMsg, WORD msgLen) 
{
    char        szEventTime[MAXALPHATIMEDATE + 1];
    char        szAlarmTime[MAXALPHATIMEDATE + 1];
    ALARM_MSG   pAlarmMsg;
    char far    *pData;
    ALARM_DATA  pAlarmData;
    STATUS      error = NOERROR;

    
    memcpy((ALARM_MSG *)&pAlarmMsg, (ALARM_MSG *)pMsg, sizeof(ALARM_MSG));
       
    /* see what type of alarm message type we have */
    switch (pAlarmMsg.Type)
    {
        case ALARM_MSG_PENDING_ALARMS:
            PRINTLOG("Alarm Message Type:%s\n","ALARM_MSG_PENDING_ALARMS");
            break;
        case ALARM_MSG_NEW_ALARM:
            PRINTLOG("Alarm Message Type:%s\n","ALARM_MSG_NEW_ALARM");
            break;
        case ALARM_MSG_IS_TERMINATING:
            PRINTLOG("Alarm Message Type:%s\n","ALARM_MSG_IS_TERMINATING");
            break;
        default:
            PRINTLOG("Invalid Alarm Message Type!\n");
            return(999);
            break;
    }

    /* print out the data */
    PRINTLOG("Number of Alarms:%d\n",pAlarmMsg.wNumOfAlarms);
    PRINTLOG("Number of Alarms:%lx\n",pAlarmMsg.Flags);

    /* get a pointer to the ALARMS_DATA structure */
    pData = (char *)OSLockObject(pAlarmMsg.hAlarmsData);
    memcpy((ALARM_DATA *)&pAlarmData, (ALARM_DATA *)pData,
            sizeof(ALARM_DATA));

    /* convert timedate info to string */
    GetTDString(&(pAlarmData.tmEventTime), szEventTime);
    GetTDString(&(pAlarmData.tmAlarmTime), szAlarmTime);

    /* print out the data */
    PRINTLOG("Event Time:%s\n",szEventTime);
    PRINTLOG("Alarm Time:%s\n",szAlarmTime);

    /* Tell Alarm Daemon to process Alarm... */
    error = Alarm_ProcessAlarm(NOTESCLIENT, 0, pAlarmData.EventUNID, 0);

    /* clean up... */
    OSUnlockObject(pAlarmMsg.hAlarmsData);
    OSMemFree(pAlarmMsg.hAlarmsData);

    return(error);
}


/************************************************************************

   FUNCTION:   GetTDString

*************************************************************************/

void LNPUBLIC  GetTDString( TIMEDATE * ptdModified, char * szTimedate )
{
    WORD            wLen;

    ConvertTIMEDATEToText( NULL, NULL, 
                           ptdModified, 
                           szTimedate, 
                           MAXALPHATIMEDATE,
             &wLen );
    szTimedate[wLen] = '\0';
    return;
}
