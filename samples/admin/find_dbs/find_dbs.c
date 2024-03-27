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

    PROGRAM:    find_dbs

    FILE:       find_dbs.c

    SYNTAX:     find_dbs  <server>  <directory>

    PURPOSE:    finds all the Domino databases contained in a directory.

    DESCRIPTION:
       This finds all the Domino databases in the specified directory on
       the specified server.  By specifying "" as the server this works
       for local databases.  This also lists any subdirectories it finds.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* OS and C include files */

#include <stdio.h>
#include <string.h>

/* HCL C API for Notes/Domino include files */


#include <global.h>
#include <nsfdb.h>
#include <nsfsearc.h>
#include <osfile.h>
#include <names.h>
#include <nsferr.h>
#include <textlist.h>
#include <ods.h>
#include <odstypes.h>
#if defined(OS400)
#include <misc.h>
#endif

#if defined(CAPI_TESTING) 
#include "printlog.h" 
#else
 #define PRINTLOG printf 
 #define PRINTERROR(api_error, api_name) {\
 char    szErrorText[256] = { 0 };\
 OSLoadString(NULLHANDLE, ERR(api_error), szErrorText, sizeof(szErrorText));\
 fprintf(stderr, "[ERROR]:%s:%d:%s - %s", __FILE__,__LINE__,api_name,szErrorText); }
#endif 

#ifndef OSMISC_DEFS
#include <osmisc.h>
#endif

/* Function prototypes */

STATUS LNPUBLIC file_action (void *, SEARCH_MATCH *, ITEM_TABLE *);
STATUS LNPUBLIC print_file_summary (ITEM_TABLE *);
VOID print_usage (void);
void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                               char *server, char *directory);

#define  STRING_LENGTH  256


#ifdef OS400
#define FLOAT NUMBER
#endif

#ifdef LINUX
#define FLOAT float
#endif

/************************************************************************

    FUNCTION:   Main

    PURPOSE:    Main HCL C API for Notes/Domino routine

*************************************************************************/

/* Program declaration */
int main(int argc, char *argv[])
{

/* Local data declarations */

    char        server[STRING_LENGTH];      /* name of HCL Domino Server */
    char        directory[STRING_LENGTH];   /* directory path */
    char        full_netpath[MAXPATH] = ""; /* full network path */
    DBHANDLE    dir_handle;                 /* handle for directory */
#ifdef OS400
    STATUS      error;            /* return status from API calls */
    error = NOERROR;
#else
    STATUS      error = NOERROR;            /* return status from API calls */
#endif


/* Initialize strings to point to "" */
    strcpy(server, full_netpath);
    strcpy (directory, full_netpath);
	
/* Get the command line parameters that the user entered. */
    ProcessArgs(argc, argv, server, directory);
	
    if (strcmp(directory, "\"\""))
        strcpy(full_netpath, directory);

    error = NotesInitExtended (argc, argv);

    if (error)
    {
        fprintf (stderr, "\nError initializing Notes.\n");
        return (1);
    }

    if (strcmp (server, "\"\""))
    {

/* Compose the full network pathname to the directory. */

        if (error = OSPathNetConstruct(NULL,
                                       server,
                                       directory,
                                       full_netpath))
        {
            PRINTERROR (error,"OSPathNetConstruct");
            NotesTerm();
            return (1);
        }

    }

/* Open the directory. */

    if (error = NSFDbOpen (full_netpath, &dir_handle))
    {
        PRINTERROR (error,"NSFDbOpen");
        NotesTerm();
        return (1);
    }

/* Call NSFSearch to find files in the directory. For each file found,
call an action routine. */

    if (error = NSFSearch (
                           dir_handle,        /* directory handle */
                           NULLHANDLE,        /* selection formula */
                           NULL,              /* title of view in formula */
                           SEARCH_FILETYPE +  /* search for files */
                           SEARCH_SUMMARY,    /* return a summary buffer */
                           FILE_DBANY +       /* find any .NS? file */
                           FILE_DIRS +        /* find subdirectories */
                           FILE_NOUPDIRS,     /* don't find the ".." dir */
                           NULL,              /* starting date */
                           file_action,       /* call for each file found */
                           NULL,              /* argument to action routine */
                           NULL))             /* returned ending date (unused) */

    {
        PRINTERROR (error,"NSFSearch");
        NSFDbClose (dir_handle);
        NotesTerm();
        return (1);
    }

/* Close the directory. */

    if (error = NSFDbClose (dir_handle))
    {
        PRINTERROR (error,"NSFDbClose");
        NotesTerm();
        return (1);
    }

/* Terminate Domino and Notes. */

    NotesTerm();

/* End of main routine. */
    PRINTLOG("\nProgram completed successfully.\n");

/* End of intro program. */

    return (0);
}

/************************************************************************

    FUNCTION:   file_action

    PURPOSE:    routine called by NSFSearch for each file found

    INPUTS:
      The first argument to this function is the optional argument
      that we supplied when we called NSFSearch.

      The second argument is supplied by NSFSearch.    It is
      a structure of information about the object that was found.

      The third argument is also supplied by NSFSearch and is
      the summary buffer for this object.

*************************************************************************/

STATUS LNPUBLIC file_action
                            (void *unused,
                             SEARCH_MATCH far *pSearchMatch,
                             ITEM_TABLE *summary_info)
{
    STATUS        error;
    SEARCH_MATCH SearchMatch;

    memcpy( (char*)&SearchMatch, (char*)pSearchMatch, sizeof(SEARCH_MATCH) );

    /* Skip this object if it does not really match the search criteria (it
      is now deleted or modified).  This is not necessary for full searches,
      but is shown here in case a starting date was used in the search. */

    if (!(SearchMatch.SERetFlags & SE_FMATCH))
        return (NOERROR);

/* Call a local function to print the file information in the
summary buffer. */

    if (error = print_file_summary (summary_info))
        return(error);

/* End of subroutine. */

    return (NOERROR);

}

/************************************************************************

    FUNCTION:   print_file_summary

    PURPOSE:    This function prints the items in a file summary buffer.

    DESCRIPTION:
         The information in a summary buffer is structured as follows:

            header (total length of summary + number of items in summary)
            length of name of item #1
            length of item #1 (including data type)
            length of name of item #2
            length of item #2 (including data type)
            length of name of item #3
            length of item #3 (including data type)
            ...
            name of item #1
            data type of item #1 + value of item #1
            name of item #2
            data type of item #2 + value of item #2
            name of item #3
            data type of item #3 + value of item #3
            ....

*************************************************************************/

STATUS LNPUBLIC print_file_summary (ITEM_TABLE *summary)
{

#define     MAX_ITEMS           30
#define     MAX_TEXT_LEN        1000
#define     DATATYPE_SIZE       sizeof(USHORT)
#define     NAME_LENGTH_SIZE    sizeof(USHORT)
#define     ITEM_LENGTH_SIZE    sizeof(USHORT)
#define     NUMERIC_SIZE        sizeof(FLOAT)
#define     TIME_SIZE           ODSLength(_TIMEDATE)


/* Local variables */

    BYTE       *summary_position;       /* current position in summary */
    ITEM_TABLE item_table;              /* header at start of summary */
    USHORT     item_count;              /* number of items in summary */
    USHORT     name_length[MAX_ITEMS];  /* length of each item name */
    USHORT     item_length[MAX_ITEMS];  /* length of each item */
    char       item_name[MAX_TEXT_LEN]; /* name of a summary item */
    USHORT     datatype;                /* type of item */
    char       item_text[MAX_TEXT_LEN]; /* text of a summary item */
    FLOAT      numeric_item;            /* a numeric item */
    TIMEDATE   time_item;               /* a time/date item */
    WORD       time_string_len;         /* length of ASCII time/date */
    STATUS     error;                   /* return code from API calls */
    USHORT     i;                       /* a counter */


/* Print a blank line to start this display. */

    PRINTLOG ("\n");

/* Get the header at the beginning of the summary buffer. */

    item_table = *summary;

/* Keep track of where we are in the buffer. */

    summary_position = (BYTE *) summary;

/* Get the number of items in the summary. */

    item_count = item_table.Items;

/* Check boundary of the array */

    if (item_count > MAX_ITEMS)
    {
        PRINTLOG("ERROR: Number of items has exceeded boundary of defined array.\n");
        return (0);
    }

/* Advance the buffer pointer over the header. */

    summary_position += ODSLength(_ITEM_TABLE);

/* Get the length of each item in the summary. */

    for (i=0; i < item_count; i++)
    {
        memcpy (&name_length[i], summary_position, NAME_LENGTH_SIZE);
        summary_position += NAME_LENGTH_SIZE;
        memcpy (&item_length[i], summary_position, ITEM_LENGTH_SIZE);
        summary_position += ITEM_LENGTH_SIZE;
    }

/* Start a loop that extracts each item in the summary. */

    for (i=0; i < item_count; i++)
    {

/* Print the name of the item. */

        memcpy (item_name, summary_position, name_length[i]);
        item_name[name_length[i]] = '\0';
        summary_position += name_length[i];

        PRINTLOG ("%s:  ", item_name);

/* Get the data type of this item. */

        memcpy (&datatype, summary_position, DATATYPE_SIZE);
        summary_position += DATATYPE_SIZE;

        /* Extract the item from the summary and put it in readable
           form. The way in which we extract the item depends on its type.
           This program handles TEXT, NUMBER, and TIME. */

        switch (datatype)
        {

            /* Extract a text item from the summary. */

            case TYPE_TEXT:
                memcpy (item_text,
                        summary_position,
                        item_length[i] - DATATYPE_SIZE);
                item_text[item_length[i] - DATATYPE_SIZE] = '\0';
                break;

/* Extract a number item from the summary. */

            case TYPE_NUMBER:
                memcpy (&numeric_item, summary_position, NUMERIC_SIZE);
                sprintf (item_text, "%g", numeric_item);
                break;

/* Extract a time/date item from the summary. */

            case TYPE_TIME:

                memcpy (&time_item, summary_position, TIME_SIZE);

                if (error = ConvertTIMEDATEToText (NULL,
                                                   NULL,
                                                   &time_item,
                                                   item_text,
                                                   MAXALPHATIMEDATE,
                                                   &time_string_len))
                    return (ERR(error));

                    item_text[time_string_len] = '\0';

                    break;

            case TYPE_TEXT_LIST:
            {
                LIST      *pList;
                WORD      list_entry;
                char      *Buffer;
                WORD      text_size;

                memset (item_text,'\0', item_length[i] - DATATYPE_SIZE + 1);
                pList = (LIST *)summary_position;
                for (list_entry = 0; list_entry < pList->ListEntries; list_entry++)
                {
                    ListGetText(pList, FALSE, list_entry, &Buffer, &text_size);
                    strncat (item_text, Buffer, text_size);
                }
            }
            break;

/* If the summary item is not one of the data types this program
handles. */

            default:
                strcpy (item_text, "(Data type not handled)");
                break;
        }

/* Print the item. */

        PRINTLOG ("%s\n", item_text);

/* Advance to next item in the summary. */

        summary_position += (item_length[i] - DATATYPE_SIZE);

/* End of loop that is extracting each item in the summary. */

    }

/* End of function */

    return (NOERROR);

}

/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:     argc, argv - directly from the command line

    OUTPUTS:    server -     The server name to use
							
				directory  -     The directory to look in

*************************************************************************/

void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                               char *server, char *directory)
{
    if (argc != 3)
    {
        printf("\n");
        printf("Enter server name: ");
        fflush (stdout);
        gets(server);
        printf("\n");
        printf ("Enter directory name:  ");
        fflush (stdout);
        gets(directory);

    }
    else
    {
        strcpy(server, argv[1]);
        strcpy(directory, argv[2]);
    } /* end if */
} /* ProcessArgs */
