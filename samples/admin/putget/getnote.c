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

This is the main function for the Domino and Notes GETNOTE utility.

This function extracts documents from a Domino database and converts
them to simple structured text.

The parameters are as follows:

    server (input): The Domino server where the input database resides. A null
    string here indicates the local machine (no server).

    database (input): The filename of the input Domino database.

    ascii_formula (input): The selection formula that determines which
    documents in the Domino database are retrieved.

    text_path (input): The pathname of the file that will hold the output.

    log_path (input): The pathname of the log file that will hold any warning and
    error messages.

    control_string (input): A character string containing various flags
    that control how GETNOTE operates.

    function return (output): An error code that reports success or failure
    to the calling routine.
*/


/* C include files */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <nsfsearc.h>
#include <osfile.h>
#include <osmem.h>
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

STATUS LNPUBLIC note_action ( 	/* called for every document */
                              VOID *,
                              SEARCH_MATCH *,
                              ITEM_TABLE *);

int far print_field (NOTEHANDLE, BLOCKID);  /* called for every field */

STATUS LNPUBLIC EnumProc(
                         VOID *,
                         SEARCH_MATCH *,
                         ITEM_TABLE *);

#if defined(LINUX)
STATUS removeSpecialChar(char* inputStr);
#endif

/* This is the entry point for GETNOTE. */

int LNPUBLIC getnote (
int        argc,
char    ** argv,
char far * server,
char far * database,
char far * ascii_formula,
char far * text_path,
char far * log_path,
char far * control_string)
{

/* Local data */

    char		full_netpath[MAXPATH+1]; /* complete db pathname */
    DBHANDLE	db_handle;	/* handle of open database */
    FORMULAHANDLE	compiled_formula;  /* compiled selection formula */
    STATUS		formula_error;	/* error from formula compile */
    WORD		dc;		/* a word we Don't Care about */
    STATUS		api_error;	/* return status from API calls */
    int		error;		/* internal error code */

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

    if (error = open_textfile (text_path, FILE_MODE_WRITE))
    {
        log_message ("ERROR", "Unable to open the text file.");
        close_logfile ();
        return (ERR_GETNOTE_FAILED);
    }

/* Initialize Domino and Notes. (We cannot get the text for the error code, since
Domino and Notes could not start.) */

    if (api_error = NotesInitExtended(argc, argv))
    {
        log_message ("ERROR", "Unable to initialize Notes.");
        close_logfile ();
        close_textfile ();
        return (ERR_GETNOTE_FAILED);
    }

/* Compile the selection formula. */

    api_error = NSFFormulaCompile (
                                   NULL,			/* name of formula (none) */
                                   (WORD) 0,			/* length of name */
                                   ascii_formula,		/* the ASCII formula */
                                   (WORD) strlen(ascii_formula),	/* length of ASCII formula */
                                   &compiled_formula,	/* handle of compiled formula */
                                   &dc,			/* compiled formula length */
                                   &formula_error,		/* error code from compile */
                                   &dc, &dc, &dc, &dc);	/* compile error info (don't care) */

/* If there was an error in the formula, get the text associated with
the COMPILATION error code. If there was some other kind of error
(unlikely) get the error message for the regular error code. */

    if (ERR(api_error) == ERR_FORMULA_COMPILATION)
    {
        log_api_message ("ERROR", formula_error);
        NotesTerm ();
        close_logfile ();
        close_textfile ();
        return (ERR_GETNOTE_FAILED);
    }

    if (api_error)
    {
        log_api_message ("ERROR", api_error);
        NotesTerm ();
        close_logfile ();
        close_textfile ();
        return (ERR_GETNOTE_FAILED);
    }

/* Construct the network path name of the database. */

    if (api_error = OSPathNetConstruct(NULL, server, database,
                                       full_netpath))
    {
        log_api_message ("ERROR", api_error);
        NotesTerm ();
        close_logfile ();
        close_textfile ();
        return (ERR_GETNOTE_FAILED);
    }

/* Open the database. */

    if (api_error = NSFDbOpen (full_netpath, &db_handle))
    {
        log_api_message ("ERROR", api_error);
        NotesTerm ();
        close_logfile ();
        close_textfile ();
        return (ERR_GETNOTE_FAILED);
    }

/* Call NSFSearch to search all the documents in the database for ones
that match the selection formula. For each document that matches, call
an action routine that extracts each field. */

    if (api_error = NSFSearch (
                               db_handle,		/* database handle */
                               compiled_formula,	/* selection formula */
                               NULL,			/* title of view in selection formula */
                               0,			/* search flags */
                               NOTE_CLASS_DOCUMENT,	/* note class to find */
                               NULL,			/* starting date (unused) */
                               note_action,            /* action routine for notes found */
                               &db_handle,		/* parameter to action routine */
                               NULL))			/* returned ending date (unused) */
    {
        log_message ("ERROR", "Problem during NSFSearch.");
        OSMemFree (compiled_formula);
        NSFDbClose (db_handle);
        NotesTerm ();
        close_logfile ();
        close_textfile ();
        return (ERR_GETNOTE_FAILED);
    }

    error = NSFSearch (
                       db_handle,      
                       compiled_formula, 
                       NULL,           
                       0,              
                       NOTE_CLASS_DOCUMENT,
                       NULL,         
                       EnumProc, 
                       &db_handle, 
                       NULL);
    if (error != NOERROR)
    {
        log_api_message("ERROR", error);
        log_message("ERROR", "NSFsearch failed during document deletion.");
        NSFDbClose (db_handle);
    }

/* Free the memory allocated by NSFFormulaCompile. */

    OSMemFree (compiled_formula);

/* Close the database. */

    if (api_error = NSFDbClose (db_handle))
    {
        log_api_message ("ERROR", api_error);
        NotesTerm ();
        close_logfile ();
        close_textfile ();
        return (ERR_GETNOTE_FAILED);
    }

/* Detach from Domino and Notes. */

    NotesTerm();

/* Close the log file. */

    close_logfile ();

/* Close the text file. */

    close_textfile ();

/* End of function. */

    return (ERR_NO_ERROR);
}

STATUS LNPUBLIC note_action(VOID *db_handle,
                            SEARCH_MATCH far *pSearchMatch,
                            ITEM_TABLE *summary_info)

/* This is the routine that is called by NSFSearch for each note
that matches the selection criteria. It finds every field in the
note. */
{

/* Local data declarations. */

    SEARCH_MATCH SearchMatch;
    NOTEHANDLE  note_handle;
    BLOCKID item_block;	/* block ID of a field */
    BLOCKID prev_block;	/* block ID of previous field */
    STATUS	api_error;
    int	error;		/* internal error code */

    memcpy( (char*)&SearchMatch, (char*)pSearchMatch, sizeof(SEARCH_MATCH) );

/* Skip this note if it does not really match the search criteria (it is
now deleted or modified).  This is not necessary for full searches,
but is shown here in case a starting date was used in the search. */

    if (!(SearchMatch.SERetFlags & SE_FMATCH))
        return (NOERROR);

/* Open the note. */

    if (api_error = NSFNoteOpen (
                    *(DBHANDLE*)db_handle,	/* database handle */
                    SearchMatch.ID.NoteID,	/* note ID */
                    0,			/* open flags */
                    &note_handle))		/* note handle (return) */
		
    {
        log_api_message ("ERROR", api_error);
        return (api_error);
    }

/* Get the block ID of the first field in the note. */

    api_error = NSFItemInfo (
                             note_handle,
                             NULL, 0,
                             &item_block,
                             NULL, NULL, NULL);

    if (ERR(api_error) == ERR_ITEM_NOT_FOUND) /* nothing to do */
        goto no_fields;

    if (api_error)
    {
        log_api_message ("ERROR", api_error);
        NSFNoteClose (note_handle);
        return (api_error);
    }

/* Start a loop that will retrieve each field in the note. */

    while (TRUE)
    {

/* Print the name, datatype and contents of this field. */

        if (error = print_field (note_handle, item_block))
        {
            NSFNoteClose (note_handle);
            return (ACTION_ROUTINE_FAILED);
        }

/* Find the block ID of the next field in this note. */

        prev_block = item_block;

        api_error = NSFItemInfoNext (
                                     note_handle,
                                     prev_block,
                                     NULL, 0,
                                     &item_block,
                                     NULL, NULL, NULL);

        if (ERR(api_error) == ERR_ITEM_NOT_FOUND) break; /* normal loop exit */

        if (api_error)
        {
            log_api_message ("ERROR", api_error);
            NSFNoteClose (note_handle);
            return (api_error);
        }

/* End of field loop. */

    }

no_fields: ;

/* Close the note. */

    if (api_error = NSFNoteClose (note_handle))
    {
        log_api_message ("ERROR", api_error);
        return (api_error);
    }

/* Write the character that indicates the end of a document (record). */

    write_text_char (get_record_separator());

/* Put a new line at the end of this record. */

    write_text_string ("\n");

/* End of note action routine. */

    return (NOERROR);

}






int far print_field (NOTEHANDLE note_handle, BLOCKID item_block)

/* This function finds and prints the name, type, and contents of one
field in a Domino and Notes document. */

{

/* Local data declarations. */

    BLOCKID value_block;	/* block ID of field contents */
    char	item_name[MAX_FIELD_NAME+1]; /* name of the field */
    WORD	item_name_len;	/* length of field name */
    WORD	item_type;	/* data type of field (in binary) */
    char	ascii_dt[MAX_TYPE_STRING+1]; /* data type in ASCII */
    DWORD	item_len;	/* length of field's value */
    WORD	item_flags;	/* field flags */
    char	*text_buffer;	/* buffer to receive ASCII text of field */
    WORD	buffer_len;	/* length of above buffer */
    WORD	text_len;	/* length of field in ASCII form */
    char	message[MAX_LOG_MESSAGE]; /* message for log file */


/* Get all the info about this field. */

    NSFItemQuery (
                  note_handle,
                  item_block,
                  item_name,
                  sizeof (item_name),
                  &item_name_len,
                  &item_flags,
                  &item_type,
                  &value_block,
                  &item_len);

/* Put a null at the end of the field name, so we can treat it like a
regular C string. */

    if (item_name_len > MAX_FIELD_NAME) item_name_len = MAX_FIELD_NAME;
        item_name[item_name_len] = '\0';

/* Convert the binary data type into ASCII. */

    datatype_bin2ascii (item_type, ascii_dt);

/* If this field has an unknown datatype, do not try to write out the
ASCII representation of the field. */
#ifdef LINUX
    if (strcasecmp(ascii_dt, "UNKNOWN") == 0)
#else
    if (STRICMP(ascii_dt, "UNKNOWN") == 0)
#endif 
    {
        sprintf (message, "Unsupported data type for field %s, not written to output.", item_name);
        log_message ("WARNING", message);
        return (ERR_NO_ERROR);
    }

/* Print the name of the item. */

    write_text_string (item_name);

/* Print the character that separates the name of the field from its
data type. */

    write_text_char (get_field_separator());

/* Print the ASCII data type. */

    write_text_string (ascii_dt);

/* Print the character that separates the data type of the field from
the field contents. */

    write_text_char (get_field_separator());

/* Figure out how much space we will allocate for the ASCII representation
of the field. */

    switch (item_type)
    {
        case TYPE_TEXT:
          if (item_len < MAX_TEXT_FIELD)
              buffer_len = (WORD) item_len;
          else
              buffer_len = MAX_TEXT_FIELD;
              break;
        case TYPE_NUMBER:
            buffer_len = MAX_NUMBER_FIELD;
            break;
        case TYPE_TIME:
            buffer_len = MAX_TIME_FIELD;
            break;
        case TYPE_TEXT_LIST:
          if (item_len < MAX_TEXTLIST_FIELD)
                buffer_len = (WORD) item_len;
          else
                buffer_len = MAX_TEXTLIST_FIELD;
                break;
        case TYPE_NUMBER_RANGE:
            buffer_len = MAX_NUMBERLIST_FIELD;
            break;
        case TYPE_TIME_RANGE:
            buffer_len = MAX_TIMELIST_FIELD;
            break;
        case TYPE_COMPOSITE:   /* field may grow as it is formatted */
          if ((item_len*2) < MAX_RICHTEXT_FIELD)
              buffer_len = (WORD) (item_len*2);
          else
              buffer_len = MAX_RICHTEXT_FIELD;
              break;
        case TYPE_USERID:
            buffer_len = MAX_USERID_FIELD;
            break;
        default:
            log_message ("ERROR", "Internal error, unrecognized data type.");
            return (ERR_INTERNAL_ERROR);
            break;
    }

/* Add one extra byte to the buffer length to hold the terminating null. */

    buffer_len++;

/* Allocate a buffer to hold the ASCII text. */

    text_buffer = (char *) malloc (buffer_len);

/* Get the contents of the field as a null-terminated ASCII string.
We must handle text fields separately, since they may have embedded
nulls in them. The nulls indicate newlines.*/

    if (item_type == TYPE_TEXT)
        text_len = get_text_field (
                                   value_block,
                                   (WORD) item_len,
                                   text_buffer,
                                   buffer_len);
    else
        text_len = NSFItemConvertValueToText (
                                              item_type,
                                              value_block,
                                              item_len,
                                              text_buffer,
                                              buffer_len,
                                              get_list_separator());

/* If the field is at or over the maximum length accepted, issue a
warning. */

    if (text_len >= buffer_len-1)
    {
        sprintf (message, "Field %s is near the maximum length; possibly truncated.", item_name);
        log_message ("WARNING", message);
    }

/* Print the field. */

    write_text_string (text_buffer);

#if defined (LINUX)
    if (!removeSpecialChar(text_buffer))
        PRINTLOG("%s\n", text_buffer);
#else
    PRINTLOG("%s\n", text_buffer);
#endif

/* Free the buffer that is holding the ASCII version of the field. */

    free (text_buffer);

/* Print the character that indicates the end of the field contents. */

    write_text_char (get_field_separator());

/* Put a new line at the end of the field. */

    write_text_string ("\n");

/* End of subroutine. */

    return (ERR_NO_ERROR);

}
/************************************************************************

    FUNCTION:   EnumProc

    PURPOSE:    Called by NSFSearch for each note that matches the 
                selection criteria

    INPUTS:

    RETURNS:   
        NOERROR or STATUS if an error condition is encountered.

    DISCUSSION:

 *************************************************************************/
STATUS  LNPUBLIC  EnumProc(VOID *phDB, SEARCH_MATCH far *pSearchMatch, ITEM_TABLE *summary_info)
{
    SEARCH_MATCH SearchMatch;
    STATUS       error;

    memcpy( (char*)&SearchMatch, (char*)pSearchMatch, sizeof(SEARCH_MATCH) );

    if (!(SearchMatch.SERetFlags & SE_FMATCH))
        return (NOERROR);
	
    fflush(stdout);

/* Delete Note */
    error = NSFNoteDelete(*(DBHANDLE far *)phDB, SearchMatch.ID.NoteID, 0);
    
    if (error)
        return(error);

/* End of subroutine. */

    return (NOERROR);
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
