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

    PROGRAM:    msg_t    

    FILE:       msg_t.c

    PURPOSE:    HCL Domino Server message queue task sample

    SYNTAX:     (at the HCL Domino Server console) 
                > load msg_t <task name> <command string>

    DESCRIPTION:
        This is a sample HCL Domino Server add-in task that sends a message
        to another add-in task, specifically the add-in task MSG_Q.  
        The usage syntax for invoking this Message Queue task is 
        "load MSG_T <task name> <command string>".

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <stdnames.h>
#include <addin.h>
#include <mq.h>
#include <miscerr.h>
#include <ostime.h>
#include <osmisc.h>

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

#define MAX_MESSAGE         (256)
#define MAX_NAME            (128)

/************************************************************************

    FUNCTION:   AddInMain

    PURPOSE:    Main entry point for all HCL Domino Server add-in tasks 

    ALGORITHM:
         Get the default status line.  Delete it. Create a new status line
         with the task name "Message Queue Task Sample".  Set the default
         status line to this new status line. Set the status string
         to "Initializing".
         
         Process the command line arguments that should include a task name
         and command string, if not terminate.

         Open a Message Queue with the task name, put the command string
         in the Queue, close the Queue and terminate.
         
*************************************************************************/
 
STATUS LNPUBLIC AddInMain (
    HMODULE       hModule,
    int           argc,
    char *        argv []
) {
    
    /* Local data. */

    STATUS      error;                         /* return code from API calls */
    DHANDLE     hOldStatusLine;                /* handle to initial default status line*/
    DHANDLE     hStatusLineDesc;               /* handle to new default status line */
    HMODULE     hMod;                          /* add-in task's module handle */
    char        TaskName[MAX_NAME + 1];        /* command line task name */
    char        MsgTaskName[MAX_NAME + 1];     /* task name with prefix added */
    char        CommandString[MAX_NAME + 1];   /* command line string */
    MQHANDLE    hQueue;                        /* Handle to message queue */
    char        MsgBuffer [MAX_MESSAGE + 1];   /* Buffer for messages */
    WORD        MsgLen;                        /* Size of message */
    int         i;

   /* 
      Initialization.

      Set the task name and status string of this add-in task. The task
      name and status string appear on the status line at the HCL Domino
      Server in response to the 'show tasks' command. By default, the
      name in the status line is the name of the program (the value of 
      argv[0]). First get the handle to this default status line descriptor 
      and delete it. Then create a new status line and set the status to
      'Initializing'.
    */
 
    AddInQueryDefaults (&hMod, &hOldStatusLine);

    AddInDeleteStatusLine (hOldStatusLine);
   
    hStatusLineDesc = AddInCreateStatusLine("Message Queue Task Add-in");

    AddInSetDefaults (hMod, hStatusLineDesc);

    AddInSetStatusText("Initializing");

    /* Process command line arguments */
    if (argc < 3)
    {
      AddInLogMessageText("Usage: load msg_t <task name> <command string>", NOERROR);
      AddInLogMessageText("MSG_T: Terminating.", NOERROR);
      return (NOERROR);
        
    }
    else
    {
      CommandString[0]='\0';
      strcpy(TaskName, argv[1]); /* get task name from command line */
      for (i=2;i<argc;i++)
      {
        strcat(CommandString, argv[i]);  /* copy command string from command line */
        if (i < (argc-1))
          strcat(CommandString," ");     /* add space */
      }
    }
    
    /* set up Task Queue Name */
    strcpy(MsgTaskName, TASK_QUEUE_PREFIX);
    strcat(MsgTaskName, TaskName);

    /*
        Open the task's message queue.
    */
       
    error = MQOpen (MsgTaskName, 0, &hQueue);
    if (NOERROR != error)
           return (ERR(error));


    AddInLogMessageText("Sending Message: %s %s", NOERROR, TaskName, CommandString);

    /* 
        Put the task message in the queue.
    */

    strcpy(MsgBuffer, CommandString);
    MsgLen = strlen(MsgBuffer);

    error = MQPut (hQueue, 1, MsgBuffer, MsgLen, 0);
    if (NOERROR != error)
           return (ERR(error));

    /* 
        Close the queue.
    */

    error = MQClose (hQueue, 0);
    
    AddInSetStatusText("Terminating");
    
    AddInLogMessageText("MSG_T: Termination complete.", NOERROR);
    
   /* End of add-in task.  We must "return" here rather than "exit". */
    
    return (ERR(error));
}

