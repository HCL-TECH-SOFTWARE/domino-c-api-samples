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

    PROGRAM:    msg_q    

    FILE:       msg_q.c

    PURPOSE:    HCL Domino Server message queue sample

    SYNTAX:     (at the HCL Domino Server console) > load msg_q

    DESCRIPTION:
        This is a sample HCL Domino Server add-in task. First, it sets the 
        default status line to be displayed by the HCL Domino Server "show 
        tasks" command. Then it enters a loop where it writes a
        message to the HCL Domino Server log file.

        The time delay between log messages and the message text may
        be modified from the server console, while the program is
        running, by use of the server console "TELL" command.  The
        commands understood by this program are:

                TELL MSG_Q QUIT
                    Terminate the add-in.
     
                TELL MSG_Q TIME int
                    Change the time delay between log messages to "int".
        
                TELL MSG_Q MSG string
                    Change the message placed in the log file to "string".

                TELL MSG_Q RESET
                    Restore original time delay and message.
                                       
                TELL MSG_Q WAIT
                    Stop writing messages to the log, but do not end the add-in.
        
                TELL MSG_Q RUN
                    Resume writing messages to the log.

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

/*
 *                Application definitions
 */

/* Message queue name */
char MsgQueueName [] = TASK_QUEUE_PREFIX "MSG_Q";
/* Note the implicit string concatenation!! */

/*
 * Function prototypes
 */

STATUS OurJob (void);
STATUS ResetJobControls (void);
STATUS ProcessMessage (
        char        *pMsg,
        WORD        msgLen
);

/* Command operation functions */
STATUS CmdError (
        char        *pMsg,
        WORD        msgLen
);

STATUS CmdTime (
        char        *pMsg,
        WORD        msgLen
);

STATUS CmdMsg (
        char        *pMsg,
        WORD        msgLen
);

STATUS CmdReset (
        char        *pMsg,
        WORD        msgLen
);

STATUS CmdWait (
        char        *pMsg,
        WORD        msgLen
);

STATUS CmdRun (
        char        *pMsg,
        WORD        msgLen
);


/*
 * Job control information
 */

#define MAX_MESSAGE                 (256)
#define DEFAULT_PERIOD              (20)
char        DefaultMessage [] = "Running Message Queue sample job";

DWORD       JobPeriod = 20;           /* Repeat job after this many seconds */
char        JobMessage [MAX_MESSAGE] = "";
BOOL        JobEnabled = TRUE;        /* Run the job or not? */

/*
 * Command table
 */

/* Pointer to command function */
typedef STATUS (*PCMD_FN) (
        char        *pMsg,
        WORD        msgLen
);
                                                                          
/* Command table entries */
typedef struct {
        char        *pCmdName;
        PCMD_FN        pCmdFn;
} CMD_ENTRY;

/* Table of commands */
/* "TELL MSG_Q QUIT" is handled by the server. The */
/* state of the message queue is set to "QUIT". */
CMD_ENTRY CmdTable [] =
{
        {"TIME",        CmdTime},
        {"MSG",         CmdMsg},
        {"RESET",       CmdReset},
        {"WAIT",        CmdWait},
        {"RUN",         CmdRun},

/* This entry marks the end of the table! */
        {NULL,          CmdError}        
};

/* Message strings */
char        MessageCmdError [] = "MSG_Q: Unknown command: ";

/************************************************************************

    FUNCTION:   AddInMain

    PURPOSE:    Main entry point for all HCL Domino Server add-in tasks 

    ALGORITHM:
         Get the default status line.  Delete it. Create a new status line
         with the task name "Message Queue Sample".  Set the default
         status line to this new status line. Set the status string
         to "Initializing".
         
         Create the message queue "MSG_Q".
         
         Enters a loop until the HCL Domino Server terminates or 
         an error occurs.
         Within this loop:

            Check for a control message in the message queue.  If a
            message has arrived, determine which command (if any) was
            entered and take the appropriate action.
                        
            Once every JobInterval seconds, set status to "Doing periodic
            job" and write an entry to the server log.

*************************************************************************/
 
STATUS LNPUBLIC AddInMain (
        HMODULE                hModule,
        int                        argc,
        char *                argv []
) {
    
    /* Local data. */

    STATUS                error;                       /* return code from API calls */
    DHANDLE               hOldStatusLine;              /* handle to initial default status line*/
    DHANDLE               hStatusLineDesc;             /* handle to new default status line */
    HMODULE               hMod;                        /* add-in task's module handle */
    MQHANDLE              hQueue;                      /* Handle to message queue */
    char                  MsgBuffer [MAX_MESSAGE + 1]; /* Buffer for messages */
    WORD                  MsgLen;                      /* Size of message */

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
   
    hStatusLineDesc = AddInCreateStatusLine("Message Queue Add-in");

    AddInSetDefaults (hMod, hStatusLineDesc);

    AddInSetStatusText("Initializing");
    
    /*
       Create and open the message queue.
    */

    error = MQOpen (MsgQueueName, MQ_OPEN_CREATE, &hQueue);
    if (NOERROR != error)
           return (ERR(error));

    /* Initialize job controls */
    
    ResetJobControls ();
                    
    /* Initialization is complete */

    AddInLogMessageText("MSG_Q: Initialization complete.", NOERROR);
    AddInSetStatusText("Idle");     /* 'Idle' replaces 'Initializing' */

    /* Begin the main loop.  We give up control to the server at the start of 
      each iteration, and get control back when the server says it is OK to 
      proceed.  When AddInIdle returns TRUE, it is time to shut down this 
      task.
    */
 
    while (NOERROR == error)
    {
                error = MQGet (hQueue, MsgBuffer, MAX_MESSAGE,
                        MQ_WAIT_FOR_MSG, (JobPeriod * 1000), &MsgLen);

                MsgBuffer[MsgLen] = '\0';

                if (NOERROR == error)
                {
                        error = ProcessMessage (MsgBuffer, MsgLen);
                }
                else if (ERR_MQ_TIMEOUT == error)
                {
                        error = NOERROR;

                /* Run the periodic job */
                if (JobEnabled)
                {        
                    AddInSetStatusText("Doing periodic job");
                          error = OurJob ();                /* our local job */
                    AddInSetStatusText("Idle");
                }
                }
   
    }   /* End of main task loop. */
   
   /* We get here when the server notifies us that it is time to terminate.  
      This is usually when the user has entered "quit" to the server console. 
      Clean up anything we have been doing.  
    */
    AddInSetStatusText("Terminating");

    error = MQClose (hQueue, 0);
        
    AddInLogMessageText("MSG_Q: Termination complete.", NOERROR);
    
    /* End of add-in task.  We must "return" here rather than "exit". */
    
    return (ERR(error));
}

/************************************************************************

    FUNCTION:   OurJob

    PURPOSE:    Local function called by the main task loop.

*************************************************************************/

STATUS OurJob (void)
{

   AddInLogMessageText(JobMessage, NOERROR);

   /* End of function. */

   return (NOERROR);
}

/************************************************************************

    FUNCTION:   ResetJobControls

    PURPOSE:    Restore initial state of job controls.

*************************************************************************/

STATUS ResetJobControls (void)
{
        JobPeriod = DEFAULT_PERIOD;
        strcpy (JobMessage, DefaultMessage);
        JobEnabled = TRUE;

        return (NOERROR);
}

/************************************************************************

    FUNCTION:   ProcessMessage

    PURPOSE:    Figure out which command we got (if any).
    
    NOTE:       This function modifies the message buffer pointed
                to by the pMsg parameter!

*************************************************************************/

STATUS ProcessMessage (
        char                *pMsg,
        WORD                msgLen
) {
        CMD_ENTRY           *pEntry;
        char                *pCmd;
        WORD                originalLen;

        originalLen = msgLen;
                        
        /* Skip any leading blanks */
        while ((msgLen > 0) && (isspace (*pMsg)))
        {
                msgLen--;
                pMsg++;
        }
        
        /* Split off the command */
        pCmd = pMsg;
        while ((msgLen > 0) && (!isspace (*pMsg)))
        {
                *pMsg = (char) toupper (*pMsg);
                msgLen--;
                pMsg++;
        }
        
        /* Null-terminate the command name */
        *(pMsg++) = '\0';
        if (msgLen > 0)
                msgLen--;
    
        /* Look up command in command table */
        for (pEntry = CmdTable; TRUE; pEntry++)
        {
                if (((char *) NULL) == pEntry->pCmdName)
                {
                        /* Log as a bad command */
                        *(pMsg - 1) = ' ';
                        return ((*(pEntry->pCmdFn)) (pCmd, originalLen));
                }
                else if (0 == strcmp (pEntry->pCmdName, pCmd))
                {
                        /* Call the appropriate command function */
                        return ((*(pEntry->pCmdFn)) (pMsg, msgLen));
                }
        }
        
        /* Should never get here! */
        return (ERR_MQSCAN_ABORT);
}

/************************************************************************

    FUNCTION:   CmdError

    PURPOSE:    Log a bad command

*************************************************************************/

STATUS CmdError (
        char        *pMsg,
        WORD        msgLen
) {
        char        errorBuf [MAX_MESSAGE];
        int         copyLen;
        int         prefixLen;

        /* Figure out how many bytes of the message to copy */
        prefixLen = strlen (MessageCmdError);
        if (MAX_MESSAGE > (prefixLen + msgLen + 1))
                copyLen = msgLen;
        else
                copyLen = MAX_MESSAGE - prefixLen - 1;

        strcpy (errorBuf, MessageCmdError);   /* Copy error message to buf */
        strncat (errorBuf, pMsg, copyLen);

        AddInLogMessageText(errorBuf, NOERROR);
        
        return (NOERROR);
}

/************************************************************************

    FUNCTION:   CmdTime

    PURPOSE:    Change the delay for running the job.

*************************************************************************/

STATUS CmdTime (
        char        *pMsg,
        WORD        msgLen
) {
    int         index;
    int         i, j=0;
        
    /* Skip any extra spaces */        
    for (index = 0; (index < msgLen) && (' ' == *pMsg); index++)
            pMsg++;
        
    /* check each char to make sure it's a valid digit */
    for (i=index; i < msgLen; i++)
    {
      if (!isdigit(pMsg[j]))
            return (ERR_TDO_CONV);
          j++;
    }
        
    JobPeriod = (DWORD) atol (pMsg);
       
    return (NOERROR);
}

/************************************************************************

    FUNCTION:   CmdMsg

    PURPOSE:    Change the log message for the job.

*************************************************************************/

STATUS CmdMsg (
        char        *pMsg,
        WORD        msgLen
) {
        int                index;

        /* Skip any extra spaces */        
        for (index = 0; (index < msgLen) && (' ' == *pMsg); index++)
                pMsg++;
        
        msgLen -= index;

        /* Copy the message */        
        for (index = 0; index < msgLen; index++)
                JobMessage [index] = *pMsg++;

        /* Add a NUL terminator */
        JobMessage [index] = '\0';
        
        return (NOERROR);        
}

/************************************************************************

    FUNCTION:   CmdReset

    PURPOSE:    Restore initial job settings.

*************************************************************************/

STATUS CmdReset (
        char        *pMsg,
        WORD        msgLen
) {
        return (ResetJobControls ());
}

/************************************************************************

    FUNCTION:   CmdWait

    PURPOSE:    Disable running the periodic job.

*************************************************************************/

STATUS CmdWait (
        char        *pMsg,
        WORD        msgLen
) {
        JobEnabled = FALSE;
        
        return (NOERROR);
}

/************************************************************************

    FUNCTION:   CmdRun

    PURPOSE:    Enable running the periodic job.

*************************************************************************/

STATUS CmdRun (
        char        *pMsg,
        WORD        msgLen
) {
        JobEnabled = TRUE;
        
        return (NOERROR);
}
