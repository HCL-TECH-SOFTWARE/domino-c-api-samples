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

    PROGRAM:    add_res
                
    FILE:       add_res.c      

    SYNTAX:     add_res

    PURPOSE:    Example HCL Domino Server add-in task for Windows & OS/2

    DESCRIPTION:
        This code demonstrates a HCL Domino Server add-in task for Windows
        and OS/2.  The task performs some operations periodically,
        sets the status of the task, and writes to the server log file.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* OS and C include files */

#include <stdio.h>
#include <string.h>
#include <stdbool.h> 

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <addin.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <ostime.h>
#include <osmisc.h>

/* Local include files */

#include "add_res.h"


/* Local definitions */

#define DB_NAME "addintst"   /* database we will write to */

/* Function prototypes */

STATUS OurJob (char *);

/************************************************************************

    FUNCTION:   AddInMain 

    PURPOSE:    Standard entry for all HCL Domino Server add-in tasks

*************************************************************************/
 
STATUS LNPUBLIC  AddInMain (HMODULE hModule, int argc, char *argv[])
{
    STATUS       error;               /* return code from API calls */
    DHANDLE      hOldStatusLine;      /* handle to initial default status line*/
    DHANDLE      hStatusLineDesc;     /* handle to new default status line */
    HMODULE      hMod;                /* add-in task's module handle */
    int          count = 0;
    bool         Automation = false;  /* Automation flag set to false */

    char         string1[] = "Sample Add-in";
    char         string2[] = "Initializing";
    char         string3[] = "Add-in Test: Initialization complete.";
    char         string4[] = "Idle";     /* 'Idle' replaces 'Initializing' */
    char         string5[] = "Doing daily job";
    char         string6[] = "Add-in Test: Daily job complete.";
    char         string7[] = "Doing two-minute job";
    char         string8[] = "Add-in Test: Two-minute job complete.";
    char         string9[] = "Doing 20-second job";
    char         string10[] = "Add-in Test: 20-second job complete.";
    char         string11[] = "Terminating";
    char         string12[] = "Add-in Test: Termination complete.";

    if (argc > 1)
    {
        int rslt = strcmp(argv[1], "auto"); /*check for automation */
        if (rslt == 0)
        {
            Automation = true;
        }
    }

    AddInQueryDefaults (&hMod, &hOldStatusLine);

    AddInDeleteStatusLine (hOldStatusLine);
    hStatusLineDesc = AddInCreateStatusLine(string1);

    AddInSetDefaults (hMod, hStatusLineDesc);

    AddInSetStatusText(string2);
    
/* Initialization is complete */

    AddInLogMessageText(string3, NOERROR);
    AddInSetStatusText(string4);     /* 'Idle' replaces 'Initializing' */	
/* Initialize this task. */

/* Begin the main loop.  We give up control to the server at the start of 
each iteration, and get control back when the server says it is OK to 
proceed.  The server returns TRUE when it is time to shut down this task.
*/

    while (!AddInIdle())
    {

/* Give up control to other tasks. */

        OSPreemptOccasionally();

/* Do the operations that we do each day. */

        if (AddInDayHasElapsed())
        {
            AddInSetStatusText(string5);
            AddInLogMessageText(string6, NOERROR);
            AddInSetStatusText(string4);
        }
 
/* Do the operations that we do every other minute. */

        else if (AddInMinutesHaveElapsed(2))
        {
            AddInSetStatusText(string7);
            AddInLogMessageText(string8, NOERROR);
            AddInSetStatusText(string4);
        }
 
/* Do the operations that we do every 20 seconds. */

        else if (AddInSecondsHaveElapsed(20))
        {
            AddInSetStatusText(string9);
            if (error = OurJob(DB_NAME))  /* our local job */
                return (ERR(error));
            AddInLogMessageText(string10, NOERROR);
            AddInSetStatusText(string4);
            if (Automation)
            {
                count++;
                if (count == 3)
                    break;
            }
        }
 
/* End of main task loop. */

    }

/* We get here when the server notifies us that it is time to terminate.  
   This is usually when the user has entered "quit" to the server console. 
   Clean up anything we have been doing.  */

    AddInSetStatusText(string11);

    AddInLogMessageText(string12, NOERROR);
  
/* End of add-in task.  We must "return" here rather than "exit". */

    return (NOERROR);

}

/************************************************************************

    FUNCTION:   OurJob 

    PURPOSE:    Local function called by the main task loop.

*************************************************************************/

STATUS OurJob (char *db_name)
{

/* Local data declarations. */

    DBHANDLE    db_handle;    /* database handle */
    NOTEHANDLE  note_handle;  /* note handle */
    TIMEDATE    timedate;     /* contents of a time/date field */
    STATUS      error;        /* return code from API calls */

/* Open the database. */

    if (error = NSFDbOpen (db_name, &db_handle))
        return (ERR(error));

/* Create a new data note. */

    if (error = NSFNoteCreate (db_handle, &note_handle))
    {
        NSFDbClose (db_handle);
        return (ERR(error));
    }

/* Write the form name to the note. */

    if (error = NSFItemSetText (note_handle, "Form",
                "AddInForm", MAXWORD))
    {
        NSFNoteClose (note_handle);
        NSFDbClose (db_handle);
        return (ERR(error));
    }

/* Write a text field to the note. */

    if (error = NSFItemSetText (note_handle, "AddIn_Text",
                "This is a note written by the add_res task.",
                MAXWORD))
    {
        NSFNoteClose (note_handle);
        NSFDbClose (db_handle);
        return (ERR(error));
    }

/* Write the current time into the note. */

    OSCurrentTIMEDATE(&timedate);
    if (error = NSFItemSetTime (note_handle, "AddIn_Time", &timedate))
    {
        NSFNoteClose (note_handle);
        NSFDbClose (db_handle);
        return (ERR(error));
    }

/* Add the note to the database. */

    if (error = NSFNoteUpdate (note_handle, 0))
    {
        NSFNoteClose (note_handle);
        NSFDbClose (db_handle);
        return (ERR(error));
    }

/* Deallocate the new note from memory. */

    if (error = NSFNoteClose (note_handle))
    {
        NSFDbClose (db_handle);
        return (ERR(error));
    }

/* Close the database. */

    if (error = NSFDbClose (db_handle))
        return (ERR(error));

/* End of function. */

    return (NOERROR);
}
