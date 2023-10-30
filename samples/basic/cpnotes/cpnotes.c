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
    PROGRAM:    cpnotes
                
    FILE:       cpnotes.c

    SYNTAX:     cpnotes  <input db>  <output db>  <output title>
                          or
                cpnotes

    PURPOSE:    Create a non-replica copy of a database and select documents
                to copy based on their type and modification time/date.

    DESCRIPTION:
        This program shows how to selectively copy notes from one database
        to another. The selections are based on the kinds of note and their 
        modification times. This shows much of the API's time/date handling.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* C and OS include files */

#include <stdio.h>
#include <string.h>

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <misc.h>
#include <ostime.h>
#include <nsferr.h>
#include <osmisc.h>
#include "../../printLog.h"

void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                             char *input_path, 
                             char *output_path, 
                             char *output_title); 
                         
#define  STRING_LENGTH  256

/************************************************************************

    FUNCTION:   main 

*************************************************************************/

int main(int argc, char *argv[])
{
    char        input_path[STRING_LENGTH];      /* pathname of input database */
    char        output_path[STRING_LENGTH];     /* pathname of output database */
    char        output_title [NSF_INFO_SIZE];   /* title of output database */
    char        output_db_info[NSF_INFO_SIZE];  /* database info buffer */
    DBHANDLE    input_handle;                   /* handle of input database */
    DBHANDLE    output_handle;                  /* handle of output database */
    TIMEDATE    start_time;                     /* cutoff time in Domino and Notes binary form */
    char        timetext[MAXALPHATIMEDATE+1];   /* time/date in ASCII */
    char        far *text_pointer;              /* pointer to timetext */
    STATUS        error = NOERROR;              /* return status from API calls */

    ProcessArgs (argc, argv, input_path, output_path, output_title);

    if (error = NotesInitExtended (argc, argv))
    {
        PRINTLOG("\n Unable to initialize Notes.\n");
        return (1);
    }

/* Open the input database. */

    if (error = NSFDbOpen (input_path, &input_handle))
    {
        PRINTERROR (error,"NSFDbOpen");  
        NotesTerm();
        return (1);
    }

    PRINTLOG("\nOpened \"%s\" as the input database", input_path); 

/* Create the output database, open it and set its title. */

    if (error = NSFDbCreate (output_path, (USHORT) DBCLASS_NOTEFILE, TRUE))
    {
        NSFDbClose (input_handle);
        PRINTERROR (error,"NSFDbCreate");  
        NotesTerm();
        return (1);
    }

    if (error = NSFDbOpen (output_path, &output_handle))
    {
        NSFDbClose (input_handle);
        PRINTERROR (error,"NSFDbOpen");  
        NotesTerm();
        return (1);
    }
  
    PRINTLOG("\nCreated \"%s\" as the output database\n", output_path); 

/* Set a binary time/date structure to "the beginning of time". */

    start_time.Innards[0] = ALLDAY;
    start_time.Innards[1] = ANYDAY;

/* Copy all the non-data notes, except the help document, from the input 
database to the output database. */

    PRINTLOG("\nDatabase copying..."); 

    if (error = NSFDbCopy (input_handle,
                           output_handle,
                           start_time,
                           NOTE_CLASS_ALL & ~NOTE_CLASS_DOCUMENT & ~NOTE_CLASS_HELP))
    {
        NSFDbClose (input_handle);
        NSFDbClose (output_handle);
        PRINTERROR (error,"NSFDbCopy");  
        NotesTerm();
        return (1);
    }

/* Get the current time and date. This is a void function - nothing is
   returned. */

    OSCurrentTIMEDATE (&start_time);

/* Subtract one month from the current time/date. This is a BOOL function 
that returns TRUE when it fails. On failure, we print a message and 
return NOERROR to the caller because we don't have a predefined error 
code for this error. */

    if (TimeDateAdjust(&start_time, 0, 0, 0, 0, -1, 0))
    {
        NSFDbClose (input_handle);
        NSFDbClose (output_handle);
        PRINTLOG ("\nProblem adjusting time/date.\n");
        NotesTerm();
        return (0);
    }

/* Copy only the data notes modified within the last month from 
the input to the output. The call may return a status code indicating
that there are no notes after the start time.  This is not really
an error, so we don't want to return on this condition.*/

    PRINTLOG("\nDatabase copying..."); 

    error = NSFDbCopy (input_handle,
                       output_handle,
                       start_time,
                       NOTE_CLASS_DOCUMENT);

    if (!(ERR(error) == ERR_NO_MODIFIED_NOTES || error == NOERROR))
    {
        NSFDbClose (input_handle);
        NSFDbClose (output_handle);
        PRINTERROR (error,"NSFDbCopy");  
        NotesTerm();
        return (1);
    }

/* Set an ASCII time/date string to the beginning of 1996. Note: The 
required syntax for the time/date string is controlled by the target
computer's operating system. Often the format is 01/01/96. However,
this format is normally customizable and may be set for your locale.
Modify this string to conform to the format used on your system.*/

    #define NEWYEARS_1996   "01/01/96 12:01 AM"

    strcpy (timetext, NEWYEARS_1996);

/* Get a pointer to the time/date string. We need this for the next
call. */

    text_pointer = timetext;

/* Put the time/date text into Domino and Notes binary form. */

    if (error = ConvertTextToTIMEDATE(NULL,
                                      NULL,
                                      &text_pointer,
                                      (WORD) strlen(timetext),
                                      &start_time))
    {
        NSFDbClose (input_handle);
        NSFDbClose (output_handle);
        PRINTERROR (error,"ConvertTextToTIMEDATE");  
        NotesTerm();
        return (1);
    }

/* Copy the help document only if it was modified since 1/1/96. */

    PRINTLOG("\nDatabase copying..."); 

    error = NSFDbCopy (input_handle,
                       output_handle,
                       start_time,
                       NOTE_CLASS_HELP);

    if (!(ERR(error) == ERR_NO_MODIFIED_NOTES || error == NOERROR))
    {
        NSFDbClose (input_handle);
        NSFDbClose (output_handle);
        PRINTERROR (error,"NSFDbCopy");  
        NotesTerm();
        return (1);
    } 

/* Now we can change the title of the output database
   by following these steps:
 
   - Get the info buffer of the database (NSFDbInfoGet);
   - Modify the title information in the buffer (NSFDbInfoModify);
   - Write the modified info buffer into the database (NSFDbInfoSet);
*/

 
/* Clear out the database information buffer */
    output_db_info[0] = '\0';
 
/* Get the output database information buffer. */
  
    if (error = NSFDbInfoGet (output_handle, output_db_info))
    {
        NSFDbClose (input_handle);
        NSFDbClose (output_handle);
        PRINTERROR (error,"NSFDbInfoGet");  
        NotesTerm();
        return (1);
    }

/* Add the database title to the database information buffer */

    NSFDbInfoModify (output_db_info, INFOPARSE_TITLE, output_title);

/* Set the database information buffer. */

    if (error = NSFDbInfoSet (output_handle, output_db_info))
    {
        NSFDbClose (input_handle);
        NSFDbClose (output_handle);
        PRINTERROR (error,"NSFDbInfoSet");  
        NotesTerm();
        return (1);
    }

    PRINTLOG("\nSet the title of \"%s\" to \"%s\"", output_path, output_title);

/* Close the databases. */

    if (error = NSFDbClose (input_handle))
    {
        NSFDbClose (output_handle);
        PRINTERROR (error,"NSFDbClose");  
        NotesTerm();
        return (1);
    }

    if (error = NSFDbClose (output_handle))
    {
        PRINTERROR (error,"NSFDbClose");  
        NotesTerm();
        return (1);
    }

    PRINTLOG("\nDone.\n"); 

/* End of subroutine. */
    PRINTLOG("\nProgram completed successfully.\n");
    NotesTerm();
    return (0);
}


/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:     argc, argv - directly from the command line
   
    OUTPUTS:    input_path, output_path, output_title get data
                from the command line or from what the user types
                at a prompt 

*************************************************************************/

void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                             char *input_path, 
                             char *output_path, 
                             char *output_title)
{ 
    if (argv[1] == NULL)
    {

        printf("Enter name of input database: ");      
        fflush (stdout);
        gets(input_path);
           
        printf("\n");
        printf("Enter name of output database: ");      
        fflush(stdout);
        gets(output_path);

        printf("\n");
        printf("Enter title of output database: ");      
        fflush(stdout);
        gets(output_title);
        
    }  
    else
    {
        strcpy(input_path, argv[1]);
        strcpy(output_path, argv[2]);
        strcpy(output_title, argv[3]);      
     } /* end if */
} /* ProcessArgs */
