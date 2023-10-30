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

    PROGRAM:    consume

    FILE:       consume.c

    PURPOSE:    HCL Domino Server add-in task that consumes user defined events.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/*
 *  OS and C include files
 */

#include <stdio.h>
#include <string.h>

/*
 *  HCL C API for Notes/Domino include files
 */

#include <global.h>
#include <addin.h>
#include <event.h>
#include <eventerr.h>
#include <osmisc.h>
#include <osmem.h>

/*
 *  Local include files 
 */

#include "consume.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif


/************************************************************************

    FUNCTION:   AddInMain

    PURPOSE:    Standard entry for all HCL Domino Server add-in tasks

    ALGORITHM:
    1.  Allocate a queue named "TEST_EVENT_QUEUE" as the event queue
        for this program.  Do this by calling EventQueueAlloc().

    2.  Call EventRegisterEventRequest() to specify the type and severity
        of events that will be consumed by this program.
        
    3.  Within the AddInIdle() loop, call EventQueueGet() to read and remove
        events of the appropriate type and severity from the queue. 
        A return value of ERR_EVTQUEUE_EMPTY indicates that there were no
        events to be processed.  A return value of NOERROR indicates
        there was an event processed. In this case, a handle to event
        specific data is also returned.

        Now, act on the event.  This program simply logs a message
        that an event has been consumed.

    4.  Before exiting the program, call EventDeregisterEventRequest() 
        to discontinue notification of particular events, and then
        deallocate the queue by calling EventQueueFree().

*************************************************************************/
 
STATUS LNPUBLIC  AddInMain (HMODULE hModule, int argc, char *argv[])
{

char            QueueName[] = "TEST_EVENT_QUEUE";   /*  Name of queue that will be created. */
char            InputDestName[] = "EVENT.NSF";      /*  A database name to associate with   */
                                                    /*  events of a certain type.           */

char            OutputDestName[20];                 /*  A buffer in which to read the name  */
                                                    /*  associated with a certain event.    */
                                                    /*  (Probably will be the same as       */
                                                    /*  InputDestName, but this is defined  */
                                                    /*  seperately for demonstration        */
                                                    /*  purposes).                          */

BOOL            bDestNameReturned;                  /*  TRUE if a name returned by call to  */
                                                    /*  EventGetDestName.                   */
                                    
DHANDLE         hEventData;                         /*  A handle to event specific data.    */
STATUS          sError;
BYTE far       *pBuf;
EVENT_DATA far *pEventData;
BYTE            DataBuf[64];                        /* A temp buf to hold event data.       */
BYTE            MessageBuf[128];                    /* A buffer in which to build log msgs. */

/*
 *  Initialize this task. 
 */
#if defined (OS400)
    AddInSetStatusText("Initializing"); 
    AddInLogMessageText("CONSUME Test: Initialization complete.", NOERROR);
    AddInSetStatusText("Idle"); 
#else
    AddInSetStatus(ADDIN_MSG_FMT, "Initializing");
    AddInLogMsg(ADDIN_MSG_FMT, "CONSUME Test: Initialization complete.");
    AddInSetStatus(ADDIN_MSG_FMT, "Idle");
#endif

/*
 *  Create the event queue, and specify that we are interested in events
 *  of type EVT_MISC and of severity SEV_NORMAL.
 */
    if (sError = EventQueueAlloc(QueueName))
        return (ERR(sError));
   
    if (sError = EventRegisterEventRequest(EVT_MISC,
                                           SEV_NORMAL,
                                           QueueName,
                                           InputDestName))
        return (ERR(sError));

/* 
 *  Start the AddInIdle() loop.
 */
    int count = 0;
    while (!AddInIdle())
    {

    /*
     *  Check to see if there is an event we're interested in the queue.
     *  If no events in the queue, don't do anything.
     */

        sError = EventQueueGet(QueueName, &hEventData);

        if (sError == ERR_EVTQUEUE_EMPTY) 
            continue;
		OSPreemptOccasionally();
        if (sError == NOERROR)
        {

        /*
         *  There is an event in the queue.  Lock the handle returned 
         *  to get the EVENT_DATA structure that is returned with the event.
         *  Copy the event-specific data (In this case, a string denoting
         *  the time the event occurred) into a local buffer, terminate the
         *  string with a NULL, and log an appropriate message.
         *
         *  Also, call EventGetDestName to get the user or database name 
         *  assigned to this event by the call to EventRegisterEventRequest.
         *  This program doesn't do anything with the name - the
         *  function is called for demonstration purposes only.
         */


            pBuf =(BYTE *) OSLockObject(hEventData);
            pEventData = (EVENT_DATA far *)pBuf;
            memmove(DataBuf, 
                    &pEventData->EventSpecificData,
                    pEventData->EventDataLength);
            DataBuf[pEventData->EventDataLength] = '\0';            

            sprintf((char *)MessageBuf,
                    "CONSUME consumed an event at %s",
                    (char *)DataBuf);

#if defined (OS400)
            AddInLogMessageText(MessageBuf, NOERROR);
#else
            AddInLogMsg(ADDIN_MSG_FMT,(char *) MessageBuf);
#endif

            bDestNameReturned = EventGetDestName(EVT_MISC,
                                                 SEV_NORMAL,
                                                 QueueName,
                                                 OutputDestName,
                                                 sizeof(OutputDestName));

        /*
         *  Here, the event consumer could do something with the name
         *  returned by EventGetDestName - If a database name was specified
         *  when the event was registered, the consumer could update the
         *  database appropriately.  If a user name was specified during 
         *  registration, the event consumer could send mail to notify
         *  the user that the event occurred.  Such processing is covered
         *  elsewhere, so it won't be repeated here.
         */

            OSUnlockObject(hEventData);
            OSMemFree(hEventData);
			count++;
			if (count > 4)
				break;
            
        }
    }
    
/*
 *  Destroy the event queue.
 */
     
    sError = EventDeregisterEventRequest(EVT_MISC, SEV_NORMAL, QueueName);    
    EventQueueFree(QueueName);

/*
 *  We get here when the server notifies us that it is time to terminate.  
 *  This is usually when the user has entered "quit" to the server console. 
 *  Clean up anything we have been doing.
 */

#if defined (OS400)
    AddInSetStatusText("Terminating"); 
    AddInLogMessageText("CONSUME Test: Termination complete.", NOERROR);
#else
    AddInSetStatus(ADDIN_MSG_FMT, "Terminating");
    AddInLogMsg(ADDIN_MSG_FMT, "CONSUME Test: Termination complete.");
#endif
  
/*
 *  End of add-in task.  We must "return" here rather than "exit". 
 */

    return (NOERROR);

}
