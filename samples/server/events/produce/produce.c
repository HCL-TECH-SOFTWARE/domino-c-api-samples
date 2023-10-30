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

    PROGRAM:    produce

    FILE:       produce.c

    PURPOSE:    Server add-in task that generates user-defined events.

    DESCRIPTION:
        This is an example of an event producer.  Every minute, this program
        adds an event of type EVT_MISC and severity SEV_NORMAL to the
        event queue called "TEST_EVENT_QUEUE".

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

#define EVENT_BUF_SIZE  128

/*
 *  OS and C include files
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <globerr.h>
#ifdef UNIX
#include <pthread.h>
#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>
#include <sys/types.h> /* for pid_t */
#else
#include <Windows.h>
#endif

/*
 *  HCL C API for Notes/Domino include files
 */

#include <global.h>
#include <nsfdata.h>
#include <addin.h>
#include <event.h>
#include <osmisc.h>
#include <ostime.h>
#include <misc.h>
#include <osmem.h>
#include <eventerr.h>



/*
 *  Local include files 
 */
 


#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif



BOOL receiveConsumermesg()
{

	FILE*           outFile = NULL;

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
	BYTE far*       pBuf;
	EVENT_DATA far* pEventData;
	BYTE            DataBuf[64];                        /* A temp buf to hold event data.       */
	BYTE            MessageBuf[128];                    /* A buffer in which to build log msgs. */

	/*
         * Create the event queue, and specify that we are interested in events
         * of type EVT_MISCand of severity SEV_NORMAL.
         */
	

	//if (sError = EventQueueAlloc(QueueName))
	//{
	//	 
	//	AddInLogMessageText("EventQueueAlloc failed", NOERROR);
	//	//			fflush(stdout);
	//	return FALSE;
	//}

	if (sError = EventRegisterEventRequest(EVT_MISC,
		SEV_NORMAL,
		QueueName,
		InputDestName))
	{
		AddInLogMessageText("EventRegisterEventRequest failed", NOERROR);
		return FALSE;
	}

	sError = EventQueueGet(QueueName, &hEventData);

	if (sError == ERR_EVTQUEUE_EMPTY)
	{
		AddInLogMessageText("EventRegisterEventRequest failed", NOERROR);
		
		return FALSE;
	}
	//OSPreemptOccasionally();
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


		pBuf = (BYTE*)OSLockObject(hEventData);
		pEventData = (EVENT_DATA far*)pBuf;
		memmove(DataBuf,
			&pEventData->EventSpecificData,
			pEventData->EventDataLength);
		DataBuf[pEventData->EventDataLength] = '\0';

		sprintf((char*)MessageBuf,
			"CONSUME consumed an event at %s",
			(char*)DataBuf);
		AddInLogMessageText((char*)MessageBuf, NOERROR);
		fflush(stdout);
		//printf("%s", (char *)MessageBuf);
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

		//AddInSetStatusText(string15);
		//AddInLogMessageText(string16, NOERROR);
		fflush(stdout);

		return TRUE;
	}
}


/************************************************************************

    FUNCTION:   AddInMain

    PURPOSE:    Standard entry for all HCL Domino Server add-in tasks

*************************************************************************/

STATUS LNPUBLIC AddInMain(HMODULE hModule, int argc, char* argv[])
{
	char szQueueName[] = "TEST_EVENT_QUEUE";
	char szDBName[] = "EVENT.NSF";

	TIMEDATE EventTimeDate;
	BYTE     EventBuffer[EVENT_BUF_SIZE];
	WORD     wLen;
	STATUS   sError;

	/*
	 *  Initialize this task.
	 */

	char string1[] = "Initializing";
	char string2[] = "Event Producer Test: Initialization complete.";
	char string3[] = "Idle";
	char string4[] = "PRODUCE Test: Produced an event!";
	char string5[] = "Terminating";
	char string6[] = "PRODUCE Test: Termination complete.";
	char string7[] = "before thread";
	char string8[] = "after thread";
	char string9[] = "system call execution failed";
	char string11[] = "Initializing";
	char string12[] = "CONSUME Test: Initialization complete.";
	char string13[] = "Idle";
	char string14[] = "CONSUME consumed an event at ";
	char string16[] = "CONSUME Test: Termination complete.";

	AddInSetStatusText(string1);
	AddInLogMessageText(string2, NOERROR);
	//intf("%s", string2);
	AddInSetStatusText(string3);


	/*
	 *  Start of code that gets executed each minute.
	 */

#ifdef UNIX
	{
			
	AddInSetStatusText(string11);
	AddInLogMessageText(string12, NOERROR);
	//	printf("%s",string2);
	AddInSetStatusText(string13);

	EventQueueAlloc(szQueueName);
	EventRegisterEventRequest(EVT_MISC,
				SEV_NORMAL,
				szQueueName,
				szDBName);

		/*	if (sError = EventQueueAlloc(szQueueName))
				return (ERR(sError));

			if (sError = EventRegisterEventRequest(EVT_MISC,
				SEV_NORMAL,
				szQueueName,
				szDBName))
				return (ERR(sError));*/

			

	int count = 0;
	while (!AddInIdle())
	{
		if (AddInSecondsHaveElapsed(10))
		{
			/*
			 *  If a minute has passed, get the current time and date.  Then
			 *  generate an event of type EVT_MISC and severity SEV_NORMAL.
			 *  Pass a string containing the time and date as event-specific
			 *  data.
			 */

			OSCurrentTIMEDATE(&EventTimeDate);
			ConvertTIMEDATEToText(NULL,
						NULL,
						&EventTimeDate,
						(char*)EventBuffer,
						sizeof(EventBuffer) - 1,
						&wLen);

			sError = EventQueuePut(szQueueName,
						NULL,
						EVT_MISC,
						SEV_NORMAL,
						&EventTimeDate,
						FMT_TEXT,
						wLen,
						(BYTE far*) EventBuffer);

			AddInLogMessageText(string4, NOERROR);
			sleep(1);
			receiveConsumermesg();
			count++;
			if (count > 4)
				{

					sleep(2); //2 seconds

					break;
				}
			}
		}

		sError = EventDeregisterEventRequest(EVT_MISC, SEV_NORMAL, szQueueName);
		EventQueueFree(szQueueName);
		/*
		 *  We get here when the server notifies us that it is time to terminate.
		 *  This is usually when the user has entered "quit" to the server console.
		 * Clean up anything we have been doing.
		 */
		AddInSetStatusText(string5);
		AddInLogMessageText(string16, NOERROR);
		AddInSetStatusText(string5);
		AddInLogMessageText(string6, NOERROR);


		/*
		 *  End of add-in task.  We must "return" here rather than "exit".ADDIN_MSG_FMT
		 */

		return (NOERROR);
	}

#else
	{
		char                szPath[] = "consume.exe";

		PROCESS_INFORMATION pif;          //Gives info on the thread and..
					          //..process for the new process
		STARTUPINFO         si;           //Defines how to start the program

		ZeroMemory(&si, sizeof(si));      //Zero the STARTUPINFO struct
		si.cb = sizeof(si);               //Must set size of structure

		BOOL bRet = CreateProcess(
				szPath, //Path to executable file
				NULL,   //Command string - not needed here
				NULL,   //Process handle not inherited
				NULL,   //Thread handle not inherited
				FALSE,  //No inheritance of handles
				0,      //No special flags
				NULL,   //Same environment block as this prog
				NULL,   //Current directory - no separate path
				&si,    //Pointer to STARTUPINFO
				&pif);   //Pointer to PROCESS_INFORMATION

		if (bRet == FALSE)
		{
			MessageBox(HWND_DESKTOP, "Unable to start program", "", MB_OK);
			return 1;
		}
		CloseHandle(pif.hProcess);   //Close handle to process
		CloseHandle(pif.hThread);    //Close handle to thread


		int count = 0;
		while (!AddInIdle())
		{
			if (AddInSecondsHaveElapsed(10))
			{
				/*
				 *  If a minute has passed, get the current time and date.  Then
				 *  generate an event of type EVT_MISC and severity SEV_NORMAL.
				 *  Pass a string containing the time and date as event-specific
				 *  data.
				 */

				OSCurrentTIMEDATE(&EventTimeDate);
				ConvertTIMEDATEToText(NULL,
						NULL,
						&EventTimeDate,
						(char*)EventBuffer,
						sizeof(EventBuffer) - 1,
						&wLen);

				sError = EventQueuePut(szQueueName,
						NULL,
						EVT_MISC,
						SEV_NORMAL,
						&EventTimeDate,
						FMT_TEXT,
						wLen,
						(BYTE far*) EventBuffer);
				count++;
				AddInLogMessageText(string4, NOERROR);
				if (count > 4)
				{

					Sleep(2000);//2000 ms

					break;
				}
			}
		}

		/*
		 *  We get here when the server notifies us that it is time to terminate.
		 *  This is usually when the user has entered "quit" to the server console.
		 * Clean up anything we have been doing.
		 */

		AddInSetStatusText(string5);
		AddInLogMessageText(string6, NOERROR);


		/*
		 *  End of add-in task.  We must "return" here rather than "exit".ADDIN_MSG_FMT
		 */

		return (NOERROR);
	}
#endif
}

