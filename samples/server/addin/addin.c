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

    PROGRAM:    addin    

    FILE:       addin.c

    PURPOSE:    sample HCL Domino Server add-in task

    SYNTAX:     (at the HCL Domino Server console) > load addin

    DESCRIPTION:
        This is a sample HCL Domino Server add-in task. First, it sets the 
        default status line to be displayed by the HCL Domino Server "show 
        tasks" command. Then it enters a loop where it performs certain 
        operations periodically. For each operation, it logs a message in
        the HCL Domino Server log file.

        This add-in task is designed for portability to platforms such as 
        Unix. This sample differs from OS/2 and Windows 
        add-in samples in that it does not use a string table. A string 
        table is a resource that requires a resource compiler, normally 
        only available under OS/2 and Windows.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

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

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/* Local definitions */

char DB_NAME[] = "addintst";  /* database we will write to */


/* Function prototypes */

STATUS OurJob (char *);

/************************************************************************

    FUNCTION:   AddInMain

    PURPOSE:    Main entry point for all HCL Domino Server add-in tasks 

    ALGORITHM:
         Get the default status line.  Delete it. Create a new status line
         with the task name "Sample Addin".  Set the default status line
         to this new status line. Set the status string to "Initializing".
         
         Enters a loop until the HCL Domino Server terminates or an 
         error occurs.

         Within this loop:

            Once a day, set status to "Doing daily job" and write a
            message to the log file. 
            
            Once every 2 minutes, sets status to "Doing two-minute job" 
            and write a message to the log.

            Once every 20 seconds, set status to "Doing 20-second job". 
            Call OurJob to opens a test database, creates a new note, 
            write some data, then update and closes the database. Then 
            write a message to the log.



*************************************************************************/
 
STATUS LNPUBLIC  AddInMain (HMODULE hModule, int argc, char *argv[])
{
    
    /* Local data. */

    STATUS       error;           /* return code from API calls */
    DHANDLE      hOldStatusLine;  /* handle to initial default status line*/
    DHANDLE      hStatusLineDesc; /* handle to new default status line */
    HMODULE      hMod;            /* add-in task's module handle */
    int          count = 0;
    bool         Automation = false;     /* Automation flag set to false */
	
    char         string1[] = "Sample Addin";
    char         string2[] = "Initializing";
    char         string3[] = "Addin Test: Initialization complete.";
    char         string4[] = "Idle";     /* 'Idle' replaces 'Initializing' */
    char         string5[] = "Doing daily job";
    char         string6[] = "Addin Test: Daily job complete.";
    char         string7[] = "Doing two-minute job";
    char         string8[] = "Addin Test: Two-minute job complete.";
    char         string9[] = "Doing 20-second job";
    char         string10[] = "Addin Test: 20-second job complete.";
    char         string11[] = "Terminating";
    char         string12[] = "Addin Test: Termination complete.";

    if (argc > 1)
    {
	int rslt = strcmp(argv[1], "auto"); /*check for automation */
		
        if (rslt == 0)
        {
			Automation = true;
        }		
    }
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
   
    hStatusLineDesc = AddInCreateStatusLine(string1);

    AddInSetDefaults (hMod, hStatusLineDesc);

    AddInSetStatusText(string2);
    
    /* Initialization is complete */

    AddInLogMessageText(string3, NOERROR);
    AddInSetStatusText(string4);     /* 'Idle' replaces 'Initializing' */

   /* Begin the main loop.  We give up control to the server at the start of 
      each iteration, and get control back when the server says it is OK to 
      proceed.  When AddInIdle returns TRUE, it is time to shut down this 
      task.
    */
 
    while (!AddInIdle())
    {
        
    /* For non-preemptive operating systems, give up control to 
       other tasks. OSPreemptOccasionally will do nothing in 
       preemptive operating systems.
     */

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
   
    }   /* End of main task loop. */
   
   /* We get here when the server notifies us that it is time to terminate.  
      This is usually when the user has entered "quit" to the server console. 
      Clean up anything we have been doing.  
    */
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

    DBHANDLE   db_handle;       /* database handle */
    NOTEHANDLE note_handle;     /* note handle */
    TIMEDATE   timedate;        /* contents of a time/date field */
    STATUS     error;           /* return code from API calls */

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

    if (error = NSFItemSetText (note_handle, "Form", "AddInForm", MAXWORD))
    {
        NSFNoteClose (note_handle);
        NSFDbClose (db_handle);
        return (ERR(error));
    }
    
   /* Write a text field to the note. */

    if (error = NSFItemSetText (note_handle, "AddIn_Text",
                               "This is a note written by the add-in task.",
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
