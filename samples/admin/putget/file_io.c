/*
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
* DESCRIPTION

This source file contains platform-DEPENDENT file I/O functions used by
the GETNOTE and PUTNOTE programs.

*/

/* C include files */

#include <stdio.h>
#include <string.h>

/* Notes API include files */

#include <global.h>
#include <osmisc.h>

/* Local include files */

#include "constant.h"


/* File-scope data */

static FILE *logfile;	/* handle to the log file */
static FILE *textfile;	/* handle to the structured text file */

static long unsigned int byte_count; /* byte count in text file */
static long unsigned int line_count; /* line count in text file */



/* Open the text file. */

int open_textfile (char * pathname, int mode)

{

/* Local data */

    char     cmode[5];	/* C buffered I/O mode */

/* See what mode (read / write) we will open the file in. */

    if (mode == FILE_MODE_READ)
        strcpy (cmode, "r");
    else if (mode == FILE_MODE_WRITE)
        strcpy (cmode, "w");
    else
        return (ERR_INTERNAL_ERROR);

/* Try to open the text file and check for errors. */

    textfile = fopen (pathname, cmode);

    if (textfile == NULL)
        return (ERR_IO_ERROR);

/* Initialize counters for overall number of bytes and lines processed
in the text file. These values will be used in error reporting. */

    byte_count = 1;
    line_count = 1;

/* End of function */

    return (ERR_NO_ERROR);
}



/* Close the text file. (This function currently does not test for or
return any errors, although it could.) */

    void close_textfile (void)

    {
        fclose (textfile);
        return;
    }




/* Open the log file. */

    int open_logfile (char * pathname)

    {
        logfile = fopen (pathname, "w");

    if (logfile == NULL)
        return (ERR_IO_ERROR);

    return (ERR_NO_ERROR);
    }




/* Close the log file. (This function currently does not test for or return
any errors, although it could.) */

    void close_logfile (void)

    {
        fclose (logfile);
        return;
    }





/* Print a string to the text file. (This function currently does
not test for or return any errors, although it could.) */

    void write_text_string (char * string)

    {

/* Local data */

        char	c;
        unsigned int	i;


/* Put the string to the text file. */

        fputs (string, textfile);

/***************************************************
The next section of code could slow down performance.
****************************************************/

/* Update the overall byte and line counters. */

        for (i=0; ;i++)
        {
             c = *(string+i);
            if (c == '\0') break;
                byte_count++;
            if (c == LF) line_count++;
        }

/* End of function */

        return;
    }




/* Print a single character to the text file. (This function currently does
not test for or return any errors, although it could.) */

    void write_text_char (char c)

    {
/* Write the character */

        putc (c, textfile);

/* Keep track of overall number of bytes and lines written. */

        byte_count++;
        if (c==LF) line_count++;

/* End of function */

        return;
    }





/* Read one character from the text file. */

    int read_text_char (char * c)

    {

/* Try to read one character from the text file. */

        *c = (char) getc (textfile);

/* Check for hard error and normal end of file. */

        if (ferror(textfile))
        {
            return (ERR_IO_ERROR);
        }

        if (feof(textfile))
        {
            return (ERR_END_OF_FILE);
        }

/* Keep track of overall number of bytes and lines read from text file. */

        byte_count++;
        if (*c==LF) line_count++;

/* End of function */

        return (ERR_NO_ERROR);
    }





/* Put one character back into the text file. (This function currently does
not test for or return any errors, although it could.)*/

    void unread_text_char (char c)

    {

/* Put the character back into the input buffer. */

        ungetc (c, textfile);

/* Subtract one from the overall byte count of the text file. If the
character is a CR, subtract one from the line count also. */

        byte_count--;
        if (c == LF) line_count--;

/* End of function. */

        return;
    }





/* Print a message to the log file. (This function currently does
not test for or return any errors, although it could.) */

    void log_message (char * severity, char * message)

    {
        fprintf (logfile,
                 "\n%s: %s \nNear line number %ld, byte number %ld.\n",
                 severity, message, line_count, byte_count);

        return;
    }





/* Get the text associated with a Notes API error code and print it
to the log file. As we print it, prepend the severity string passed in
argument 1. (This function currently does not test for or return any
errors, although it could.)*/


    void log_api_message (char * severity, STATUS api_code)

    {

/* Local data */

        char		message[MAX_API_ERR];
        WORD		text_len;

/* Get the message for this API error code from the resource string
table. */

        text_len = OSLoadString (
                                 NULLHANDLE,
                                 ERR(api_code),
                                 message,
                                 sizeof(message));

/* Print the error message. */

        fprintf (logfile,
                 "\n%s: %s \nErrorcode : 0x%04x,Near line number %ld, byte number %ld.\n",
                 severity, message, api_code, line_count, byte_count);

/* End of function. */

        return;

    }
