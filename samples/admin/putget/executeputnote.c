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

This is the main function for the Domino and Notes PUTNOTE utility. It is platform
independent, and should be called by a platform-dependent front end.

This function reads a simple structured text file and translates it
into Domino and Notes documents.

The parameters are as follows:

    text_path (input): The pathname of the file that holds the structured
    text.

    server (input): The HCL Domino Server where the output database resides. A
    null string here indicates the local machine (no server).

    database (input): The filename of the output Domino database.

    log_path (input): The pathname of the log file that will hold any
    warning and error messages.

    control_string (input): A character string containing various flags
    that control how PUTNOTE operates.

    function return (output): An error code that reports success or failure
    to the calling routine.
*/


/* C include files */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <ctype.h>

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <nsfsearc.h>
#include <osfile.h>
#include <osmisc.h>
#include <names.h>
#include <nsferr.h>
#include "../../printLog.h"

/* Local include files */

#include "constant.h"
#include "fields.h"
#include "file_io.h"
#include "misc.h"

/* Local function prototypes */

int put_document (DBHANDLE);
int put_field (NOTEHANDLE);
int get_field_name (char *);
int get_field_type (char *);
int get_field_contents (char *, int);
int find_next_field (void);

#if defined(LINUX)
STATUS removeSpecialChar(char* inputStr);
#endif

/* Entry point for the PUTNOTE function. */

int LNPUBLIC putnote (
int        argc,
char    ** argv,
char far * server,
char far * database,
char far * text_path,
char far * log_path,
char far * control_string)
{

/* Local data */

    char          full_netpath[MAXPATH+1]; /* complete db pathname */
    DBHANDLE      db_handle;	/* handle of open database */
    STATUS        api_error;	/* return status from API calls */
    int           error;		/* internal error codes */

/* Open the log file. (On an error, we must return a special error code
to tell the caller what happened. We cannot write the problem to the
log file, since we couldn't open it!!) */

    if (error = open_logfile (log_path))
        return (ERR_CANNOT_OPEN_LOG);

/* Read the information in the control string and set the appropriate
variables. (If there is an error in the control string, it should be
reported to the log file from within this function call.) */

    if (error = set_control_info (control_string))
    {
        close_logfile ();
        return (ERR_GETNOTE_FAILED);
    }

/* Open the text file. */

    if (error = open_textfile (text_path, FILE_MODE_READ))
    {
        log_message ("ERROR", "Unable to open the text file.");
        close_logfile ();
        return (ERR_PUTNOTE_FAILED);
    }

/* Initialize Domino and Notes. (We cannot get the text of any error return, since
Domino and Notes could not start.) */

    if (api_error = NotesInitExtended(argc, argv))
    {
        log_message ("ERROR", "Unable to initialize Notes.\n");
        return (ERR_PUTNOTE_FAILED);
    }

/* Construct the network path name of the database. */

    if (api_error = OSPathNetConstruct(NULL, server, database, full_netpath))
    {
        log_api_message ("ERROR", api_error);
        NotesTerm();
        close_logfile ();
        close_textfile ();
        return (ERR_PUTNOTE_FAILED);
    }

/* Open the database. */

    if (api_error = NSFDbOpen (full_netpath, &db_handle))
    {
        log_api_message ("ERROR", api_error);
        NotesTerm();
        close_logfile ();
        close_textfile ();
        return (ERR_PUTNOTE_FAILED);
    }

/* Start a loop that processes each record in the text file. */

    while (TRUE)
    {

/* Get the next record in the text file and put it into the Domino
database as a document. */

        error = put_document (db_handle);

        if (error == ERR_END_OF_FILE) break;  /* all done */

        if (error)	/* this is a real error, already reported */
        {
            NSFDbClose (db_handle);
            NotesTerm();
            close_logfile ();
            close_textfile ();
            return (ERR_PUTNOTE_FAILED);
        }

/* End of loop that is processing each input record. */

    }

/* Close the database. */

    if (api_error = NSFDbClose (db_handle))
    {
        log_api_message ("ERROR", api_error);
        NotesTerm();
        close_logfile ();
        close_textfile ();
        return (ERR_PUTNOTE_FAILED);
    }

/* Detach from Domino and Notes. */

    NotesTerm ();

/* Close the log file. */

    close_logfile ();

/* Close the text file. */

    close_textfile ();

/* End of function. */

    PRINTLOG("\nProgram completed successfully.\n");
    return (ERR_NO_ERROR);
}





int put_document (DBHANDLE db_handle)

/* Extract one record from the text file (the program's input) and
transform it to one Domino and Notes document (the program's output). */

{

/* Local data */

    NOTEHANDLE note_handle;
    STATUS	api_error;	/* HCL C API for Notes/Domino error */
    int	error;		/* internal error codes */
    int	field_count;	/* how many fields we created */


/* Create a new document in this database. */

    if (api_error = NSFNoteCreate (db_handle, &note_handle))
    {
        log_api_message ("ERROR", api_error);
        return (ERR_API_ERROR);
    }

/* Reset the field count. */

    field_count = 0;

/* Start a loop that reads each field in the text file and creates a
field in the Domino and Notes document. */

    while (TRUE)
    {

/* Advance the input pointer to the start of the next field. Check for
normal conditions as well as errors in the result. */

        error = find_next_field();

        if ((error == ERR_END_OF_FILE) && (field_count > 0))
        {
            log_message ("ERROR", "Unexpected end of file within record.");
            NSFNoteClose (note_handle);
            return (ERR_BAD_TEXTFILE);
        }

        if (error == ERR_END_OF_FILE)  /* normal end of input */
        {
            NSFNoteClose (note_handle);
            return (ERR_END_OF_FILE);
        }

        if (error == ERR_END_OF_RECORD) break;   /* normal end of record */

        if (error)
        {
            NSFNoteClose (note_handle);
            return (error);
        }

/* Read the field in the text file and use it to create a field in the
Domino and Notes document. */

        error = put_field (note_handle);

        if (error == ERR_UNSUPPORTED_TYPE) continue;

        if (error)
        {
            NSFNoteClose (note_handle);
            return (error);
        }

/* Update the count of fields in this document. */

        field_count++;

/* End of loop that is processing each field. */

    }

/* If we found some fields in this record, add the resulting document
to the Domino database. If we found no fields, issue a warning. */

    if (field_count > 0)
        if (api_error = NSFNoteUpdate (note_handle, 0))
        {
            NSFNoteClose (note_handle);
            log_api_message ("ERROR", api_error);
            return (ERR_API_ERROR);
        }
        else;
    else
        log_message ("WARNING", "An input record contains no fields.");

/* Close the note. (Remove its structure from memory.) */

    if (api_error = NSFNoteClose (note_handle))
    {
        log_api_message ("ERROR", api_error);
        return (ERR_API_ERROR);
    }

/* End of function. */

    return (ERR_NO_ERROR);
}





int find_next_field ()

/* Parse the input stream and advance the input pointer to the start of the
next field (if there is one) within the current record. If there is no next
field within the current record, return the reason why.

This function enforces the rule that items in the input must be separated
by one or more newlines. In other words, each field and end-of-record
marker must start on a new line.

This function assumes that the input stream is positioned at one of the
following locations:

    At the very beginning of the file

    Immediately after a complete record (after an end-of-record marker)

    Immediately after a complete field (after the end-of-field marker that
    terminates the field contents)
*/

{

/* Local data */

    static BOOL beginning_of_file = TRUE; /* beginning of input ? */
    char	c;		/* one character of input */
    int	newline_count;	/* number of newlines found */
    int	error;		/* internal error codes */


/* Reset new line counter. */

    newline_count = 0;

/* If we are at the very beginning of the input, treat this as if we
have seen a newline. */

    if (beginning_of_file)
    {
        newline_count++;
        beginning_of_file = FALSE;
    }

/* Start a loop that will look for the start of the next field. */

    while (TRUE)
    {

/* Get the next character in the input. */

        error = read_text_char (&c);

/* If we had an I/O error, we cannot proceed. */

        if (error == ERR_IO_ERROR)
        {
            log_message ("ERROR", "I/O error while looking for start of field.");
            return (error);
        }

/* If we are at the end of the input, this is OK. We have finished reading
the input stream. */

        if (error == ERR_END_OF_FILE) return (error);

/* If we got the end-of-record marker and have seen some newlines, this
is OK. We have finished with this record. */

        if ((c == get_record_separator()) && (newline_count != 0))
            return (ERR_END_OF_RECORD);

/* If we got a carriage return or line feed, count this and swallow it. */

        if ((c == CR) || (c == LF))
        {
            newline_count++;
            continue;
        }

/* When we get here, we are reading real input (not an end-of-record or
CR, or LF).

If we have seen at least one new line, then we have found the start of a
new field. Push the input character back into the stream and exit.

If we have not seen a new line, this is an error. (This means the next
field does not start at the beginning of a line. ) */

        if (newline_count > 0)
        {
            unread_text_char (c);
            break;
        }
        else
        {
            log_message ("ERROR", "Extra characters after valid line of input.");
            return (ERR_BAD_TEXTFILE);
        }

/* End of loop that is looking for the start of a field. */

    }

/* End of function */

    return (ERR_NO_ERROR);

}






int put_field (NOTEHANDLE note_handle)

/* Extract one field from the text file and transform it to one field in
the Domino and Notes document.

This function assumes that the input stream is positioned exactly at the
start of a field. */

{

/* Local data */

    char	field_name[MAX_FIELD_NAME+1];	/* field name */
    char	ascii_datatype[MAX_TYPE_STRING+1]; /* field type in ASCII */
    USHORT	datatype;	/* field type in binary */
    char	*field_contents; /* the ASCII contents of a field */
    char	message[MAX_LOG_MESSAGE]; /* a message for the log file */
    int	field_len;	/* length of above field buffer */
    int	error;		/* internal error code */


/* Get the name of this field from the text file. */

    if (error = get_field_name (field_name))
        return (error);

/* Get the type of the field from the text file. This type indicator
is an ASCII string. */

    if (error = get_field_type (ascii_datatype))
        return (error);

/* Convert the ASCII type string into a Domino and Notes binary data type. */

    datatype_ascii2bin (ascii_datatype, &datatype);

/* Find the length of the buffer that will hold the contents of the
field. */

    switch (datatype)
    {
        case TYPE_TEXT:
            field_len = MAX_TEXT_FIELD;
            break;
        case TYPE_NUMBER:
            field_len = MAX_NUMBER_FIELD;
            break;
        case TYPE_TIME:
            field_len = MAX_TIME_FIELD;
            break;
        case TYPE_TEXT_LIST:
            field_len = MAX_TEXTLIST_FIELD;
            break;
        case TYPE_NUMBER_RANGE:
            field_len = MAX_NUMBERLIST_FIELD;
            break;
        case TYPE_TIME_RANGE:
            field_len = MAX_TIMELIST_FIELD;
            break;
        case TYPE_COMPOSITE:
            field_len = MAX_RICHTEXT_FIELD;
            break;
        case TYPE_USERID:
            field_len = MAX_USERID_FIELD;
            break;
        case TYPE_INVALID_OR_UNKNOWN:
            field_len = MAX_UNKNOWN_FIELD;
            break;
        default:
            sprintf (message, "Internal error, unrecognized data type for field %s.", field_name);
            log_message ("ERROR", message);
            return (ERR_INTERNAL_ERROR);
            break;
    }

/* Allocate a buffer to hold the ASCII contents of the field.  Allow an
extra byte in the buffer for a terminating null. */

    field_contents = (char *) malloc (field_len + 1);

/* Get the field contents from the text file. */

    if (error = get_field_contents (field_contents, field_len))
    {
        free (field_contents);
        return (error);
    }

/* Put this field into the Domino and Notes document. */

    switch (datatype)
    {
        case TYPE_TEXT:
            error = put_text_field (note_handle, field_name, field_contents);
            break;
        case TYPE_TEXT_LIST:
            error = put_textlist_field (note_handle, field_name, field_contents);
            break;
        case TYPE_NUMBER:
            error = put_number_field (note_handle, field_name, field_contents);
            break;
        case TYPE_NUMBER_RANGE:
            error = put_numberlist_field (note_handle, field_name, field_contents);
            break;
        case TYPE_TIME:
            error = put_time_field (note_handle, field_name, field_contents);
            break;
        case TYPE_TIME_RANGE:
            error = put_timelist_field (note_handle, field_name, field_contents);
            break;
        case TYPE_COMPOSITE:
            error = put_richtext_field (note_handle, field_name, field_contents);
            break;
        case TYPE_USERID:
            error = put_userid_field (note_handle, field_name, field_contents);
            break;
        case TYPE_INVALID_OR_UNKNOWN:
            sprintf (message, "Unsupported data type for field %s, not written to output.", field_name);
            log_message ("WARNING", message);
            error = ERR_UNSUPPORTED_TYPE;
            break;
        default:
            sprintf (message, "Internal error, unrecognized data type for field %s.", field_name);
            log_message ("ERROR", message);
            error = ERR_INTERNAL_ERROR;
            break;
    }

    if (error)
    {
        free (field_contents);
        return (error);
    }

/* Free the buffer we used to hold the field. */

    free (field_contents);

/* End of function */

    return (ERR_NO_ERROR);

}







int get_field_name (char * field_name)

/* Parse the input stream and extract the name of the current field.

This function assumes that the input stream is positioned exactly at the
start of the name. */

{

/* Local data */

    char	c;	    /* one character of input */
    int	name_len;   /* current length of field name */
    int	error;	    /* internal error codes */


/* Clear the field name. */

    strcpy (field_name, "");
    name_len = 0;

/* Start a loop that will get the field name. We check for bad input
as we go. */

    while (TRUE)
    {

/* Get the next character in the input. */

        error = read_text_char (&c);

/* If we had an I/O error, we cannot proceed. */

        if (error == ERR_IO_ERROR)
        {
            log_message ("ERROR", "I/O error reading field name.");
            return (error);
        }

/* If we are at the end of the input, this is an error. */

        if (error == ERR_END_OF_FILE)
        {
            log_message ("ERROR", "End-of-file in field name.");
            return (ERR_BAD_TEXTFILE);
        }

/* If we got the end-of-record marker, this is an error. */

        if (c == get_record_separator())
        {
            log_message ("ERROR", "End-of-record in field name.");
            return (ERR_BAD_TEXTFILE);
        }

/* If we got the end-of-field marker and there is no name, this is an
error. */

        if ((c == get_field_separator()) && (name_len == 0))
        {
            log_message ("ERROR", "Zero length field name.");
            return (ERR_BAD_TEXTFILE);
        }

/* If we got the end-of-field marker and there is a name, then we are
done. */

        if ((c == get_field_separator()) && (name_len != 0))
        {
            break;
        }

/* If we got a control character or a whitespace character, this is an
error. */

        if (iscntrl(c) || isspace(c))
        {
            log_message ("ERROR", "Illegal character in field name.");
            return (ERR_BAD_TEXTFILE);
        }

/* See if we will exceed the maximum length for a field name. */

        if (name_len >= MAX_FIELD_NAME)
        {
            log_message ("ERROR", "Field name too long.");
            return (ERR_BAD_TEXTFILE);
        }

/* Add this character to the field name string. */

        field_name[name_len] = c;
        name_len++;

/* End of loop that is getting the field name. */

    }

/* Put a terminating null on the field name. */

    field_name[name_len] = '\0';

/* End of function */

    return (ERR_NO_ERROR);

}






int get_field_type (char * ascii_datatype)

/* Parse the input stream and extract the name of the current field data
type.

This function assumes that the input stream is positioned exactly at the
start of the type string. */

{

/* Local data */

    char c;		    /* one character of input */
    int  type_len;	    /* current length of field type */
    int  error;	    /* internal error code */

/* Clear the field type. */

    strcpy (ascii_datatype, "");
    type_len = 0;

/* Start a loop that will get the field type. We check for bad input
as we go. */

    while (TRUE)
    {

/* Get the next character in the input. */

        error = read_text_char (&c);

/* If we had an I/O error, we cannot proceed. */

        if (error == ERR_IO_ERROR)
        {
            log_message ("ERROR", "I/O error reading field type.");
            return (error);
        }

/* If we are at the end of the input, this is an error. */

        if (error == ERR_END_OF_FILE)
        {
            log_message ("ERROR", "End-of-file in field type.");
            return (ERR_BAD_TEXTFILE);
        }

/* If we got the end-of-record marker, this is an error. */

        if (c == get_record_separator())
        {
            log_message ("ERROR", "End-of-record in field type.");
            return (ERR_BAD_TEXTFILE);
        }

/* If we got the end-of-field marker and there is no type, this is an
error. */

        if ((c == get_field_separator()) && (type_len == 0))
        {
            log_message ("ERROR", "Zero length field type.");
            return (ERR_BAD_TEXTFILE);
        }

/* If we got the end-of-field marker and there is a type, then we are
done. */

        if ((c == get_field_separator()) && (type_len != 0))
        {
            break;
        }

/* If we got a control character or a whitespace character, this is an
error. */

        if (iscntrl(c) || isspace(c))
        {
            log_message ("ERROR", "Illegal character in type string.");
            return (ERR_BAD_TEXTFILE);
        }

/* See if we will exceed the maximum length for a type string. */

        if (type_len >= MAX_TYPE_STRING)
        {
            log_message ("ERROR", "Field type too long.");
            return (ERR_BAD_TEXTFILE);
        }

/* Add this character to the field type string. */

        ascii_datatype[type_len] = c;
        type_len++;

/* End of loop that is getting the field type. */

    }

/* Put a terminating null on the field type. */

    ascii_datatype[type_len] = '\0';

/* End of function */

    return (ERR_NO_ERROR);

}





int get_field_contents (char * field_contents, int max_field)

/* Parse the input stream and extract the contents of the current field.

This function assumes that the input stream is positioned exactly at the
start of the field contents. */

{

/* Local data */

    char	c;	    /* one character of input */
    int	field_len;  /* current length of field */
    int	error;	    /* internal error codes */

/* Clear the field. */

    strcpy (field_contents, "");
    field_len = 0;

/* Start a loop that will get the field. We check for bad input
as we go. */

    while (TRUE)
    {

/* Get the next character in the input. */

        error = read_text_char (&c);

/* If we had an I/O error, we cannot proceed. */

        if (error == ERR_IO_ERROR)
        {
            log_message ("ERROR", "I/O error reading field contents.");
            return (error);
        }

/* If we are at the end of the input, this is an error. */

        if (error == ERR_END_OF_FILE)
        {
            log_message ("ERROR", "End-of-file in field contents.");
            return (ERR_BAD_TEXTFILE);
        }

/* If we got the end-of-record marker, this is an error. */

        if (c == get_record_separator())
        {
            log_message ("ERROR", "End-of-record in field contents.");
            return (ERR_BAD_TEXTFILE);
        }

/* If we got the end-of-field marker and there is no field, this is
unexpected but OK. */

        if ((c == get_field_separator()) && (field_len == 0))
        {
            log_message ("WARNING", "Zero length field.");
            break;
        }

/* If we got the end-of-field marker and there is a field, then we are
done. */

        if ((c == get_field_separator()) && (field_len != 0))
        {
            break;
        }

/* We have a valid character of the field contents. See if it will exceed
the maximum field length. If so, swallow the rest of the contents.
This truncates the field. */

        if (field_len >= max_field)
        {
            log_message ("WARNING", "Field contents too long; truncated.");
            while (TRUE)
            {
                error = read_text_char (&c);
                if (error == ERR_IO_ERROR)
                {
                    log_message ("ERROR", "I/O error reading field contents.");
                    return (error);
                }
                if (error == ERR_END_OF_FILE)
                {
                    log_message ("ERROR", "End-of-file in field contents.");
                    return (ERR_BAD_TEXTFILE);
                }
                if (c == get_record_separator())
                {
                    log_message ("ERROR", "End-of-record in field contents.");
                    return (ERR_BAD_TEXTFILE);
                }
                if (c == get_field_separator())
                    break;	/* break from loop that is swallowing input */
                }
                break;	/* break from overall loop getting field */
            }

/* Add this character to the field. */

        field_contents[field_len] = c;
        field_len++;

/* End of loop that is getting the field. */

    }

/* Put a terminating null on the field contents. */

    field_contents[field_len] = '\0';

/* End of function */

#if defined (LINUX)
    if (!removeSpecialChar(field_contents))
        PRINTLOG("%s\n", field_contents);
#else
    PRINTLOG("%s\n", field_contents);
#endif
    return (ERR_NO_ERROR);

}
#if defined(LINUX)

STATUS removeSpecialChar(char* inputStr)
{
    const int slen = strlen(inputStr);
    char tmpStr[slen + 1];
    char *sztmpStr = tmpStr;
    char *szbase = inputStr;
    int len = 0;
    memset(tmpStr, '\0', sizeof(tmpStr));

    while (inputStr != NULL && len < slen)
    {
        if (*inputStr != '\r')
        {
            *sztmpStr = *inputStr;
            sztmpStr++;
        }
        inputStr++;
        len++;
    }
    tmpStr[len] = '\0';
    strcpy(szbase, tmpStr);
    return NOERROR;
}
#endif
