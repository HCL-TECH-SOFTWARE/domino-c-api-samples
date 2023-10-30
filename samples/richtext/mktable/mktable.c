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

    PROGRAM:    mktable

    FILE:       mktable.c

    SYNTAX:     mktable <database path>

    PURPOSE:    Shows how to create tables in a rich text field.

    DESCRIPTION:
        Create a document in the database, add a rich-text field
        called "Body" to the document, and add 3 tables to the field.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* C includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* HCL C API for Notes/Domino includes */

#include <global.h>
#include <globerr.h>
#include <nsf.h>
#include <nsferr.h>
#include <osmisc.h>
#include <oserr.h>
#include <osmem.h>
#include <pool.h>
#include <names.h>
#include <colorid.h>
#include <editods.h>
#include <ods.h>
#include <odserr.h>
#include <editdflt.h>
#include <osfile.h>
#include <textlist.h>
#include <stdnames.h>
#include "../../printLog.h"

#include "mktable.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                         char *db_filename);

void PrintAPIError (STATUS);

/************************************************************************

    FUNCTION:   Main

    PURPOSE:    HCL C API for Notes/Domino subroutine

*************************************************************************/

int main(int argc, char *argv[])
{
    STATUS                error = NOERROR;
    WORD                  cleanup_state = DO_NOTHING;
    char                  *db_name;
    char                  cell_str[LINEOTEXT], temp_buf[LINEOTEXT*2];
    DBHANDLE              db_handle;
    NOTEHANDLE            note_handle;
    DHANDLE               cd_buf_handle;
    WORD                  rows, columns;
    DHANDLE               textlist_hdl;
    void                  *textlist_ptr;
    WORD                  list_entries, list_entry, textlist_size;
    WORD                  countr, countc;
    BYTE                  border_style;
    BYTE                  *cd_buf_ptr;              /* current CD buffer pointer */
    DWORD                 cd_buf_cnt = 0L;          /* CD buffer size counter */
    CDPABDEFINITION       *anchor_pabdef_ptr;       /* first (non table) PABDEF */

    CDPABDEFINITION       post_conversion_pabdef_ptr;  /* First paragraph definition in machine-specific format */
    void                  *pre_conversion_pabdef_ptr;  /* pointer for the data format conversion memory allocation */
    int                   table_num;

    if (error = NotesInitExtended (argc, argv))
    {
        PRINTLOG("\n Unable to initialize Notes.\n");
        return (1);
    }

    db_name = (char *) malloc(LINEOTEXT);
    if (db_name == NULL)
    {
        PRINTLOG("Error: Out of memory.\n");
        NotesTerm();
        return (0);
    }

    /* Get the pathname of the database. */
    ProcessArgs(argc, argv, db_name);

    /* Open the database */
    if (error = NSFDbOpen(db_name, &db_handle))
        goto Exit;
    else
        cleanup_state |= CLOSE_DB;

    /* Through with database name. */
    free(db_name);

    /* Create a New Note, append default items */
    if (error = NSFNoteCreate(db_handle, &note_handle))
        goto Exit;
    else
        cleanup_state |= CLOSE_NOTE;

    if (error = NSFItemSetText(note_handle, FIELD_FORM,
                                   "TableForm",
                                   MAXWORD))
        goto Exit;

    if (error = NSFItemSetText(note_handle, "Description",
                                   "Tables created with Notes API",
                                   MAXWORD))
        goto Exit;

    /* Allocate a Domino and Notes Memory Object and manage it with a BYTE ptr */

    if (error = OSMemAlloc(0, MAXONESEGSIZE, &cd_buf_handle))
         goto Exit;
    else
        cleanup_state |= FREE_BUFFER;

    /* lock buffer, assign CD buffer reference and current */
    /* cd_buf_ptr = (BYTE *) OSLockObject(cd_buf_handle); */
    cd_buf_ptr = OSLock(BYTE, cd_buf_handle);
    if (cd_buf_ptr == NULL)
        goto Exit;
    else
        cleanup_state |= UNLOCK_BUFFER;

    memset(cd_buf_ptr, '\0', MAXONESEGSIZE);

    /* Set up anchor (non-table PABDEFINITION) */
    anchor_pabdef_ptr = (CDPABDEFINITION *) cd_buf_ptr;

    /* Create paragraph style 1 using defaults */
    cur_pab_id  = 1;
 
    if(!CDPutPabdef(cur_pab_id, &cd_buf_ptr, cd_buf_cnt,
                    &cd_buf_cnt, 0, 0, 0))
        goto Exit;

    if(!CDPutPara( &cd_buf_ptr, cd_buf_cnt,
                     &cd_buf_cnt))
       goto Exit;

    if(!CDPutPabref(cur_pab_id, &cd_buf_ptr, cd_buf_cnt,
                    &cd_buf_cnt))
        goto Exit;

    /* Simple way to build initial CDTEXT record */
    strcpy(temp_buf, REQS);
    if(!CDPutText(BLDBLTR12, temp_buf, (WORD) strlen(temp_buf), &cd_buf_ptr,
                  cd_buf_cnt, &cd_buf_cnt))
        goto Exit;

    /* In CDPutPabdef routine we use ODSWriteMemory  to convert the CDPABDEFINITION 
       structure from machine-specific format into the canonical Domino and Notes format. 
       This also means that if we'd like to reference the data from this 
       sructure (PABID, LeftMargin, RightMargin, etc.), we have to copy 
       the structure and convert the data back to machine-specific format by 
       using ODSReadMemory function. */

    /* Allocate memory for the copy of the CDPABDEFINITION structure */
    pre_conversion_pabdef_ptr = (void *) malloc(ODSLength(_CDPABDEFINITION));

    if (pre_conversion_pabdef_ptr == NULL)
    {
        PRINTLOG("Error: Out of memory.\n");
        NotesTerm();
        return (0);
    }

    /* Copy the CDPABDEFINITION structure */

   memcpy(pre_conversion_pabdef_ptr,anchor_pabdef_ptr, ODSLength(_CDPABDEFINITION));

    /* Convert the structure back to the machine-specific format */

   ODSReadMemory(&pre_conversion_pabdef_ptr, _CDPABDEFINITION, &post_conversion_pabdef_ptr,1);


    /**************************************************************/
    /* Build a 2x2,  10" wide no border fixed size Table          */
    /**************************************************************/

    list_entries = textlist_size = 0;
    if (error = ListAllocate(0, 0, FALSE, &textlist_hdl,
                              &textlist_ptr, &textlist_size))
        goto Exit;
    else
    {
        cleanup_state |= FREE_LIST;

        /* Unlock text list before passing handle to ListAddEntry */
        OSUnlock(textlist_hdl);
    }

    rows = 2; columns = 2;
    for (countr = 0, list_entry = 0; countr < rows; countr++)
    {
        for (countc = 0; countc < columns; countc++)
        {
            strcpy(cell_str, "\\*********** FIXED CELL ");
            sprintf(temp_buf,"%i",countr );
            strcat(cell_str,temp_buf);
            strcat(cell_str, ",");
            sprintf(temp_buf,"%i",countc );
            strcat(cell_str,temp_buf);
            strcat(cell_str, " ***********\\");

            if (error = ListAddEntry(textlist_hdl, FALSE,
                                     &textlist_size, list_entry++,
                                     cell_str, (WORD) strlen(cell_str)))
                goto Exit;
        }
    }

    /* Lock text list to refresh pointer before passing it to CDPutTable*/
    textlist_ptr = OSLockObject(textlist_hdl);
    cleanup_state |= UNLOCK_LIST;

    table_num = 1;
    border_style = TABLE_BORDER_NONE;
    if (!CDPutTable(table_num,
                    MIDDLE_REC,/* flags need for prefix/suffix paras*/
                    rows,        /* # of rows desired.              */
                    columns,     /* # of columns desired.           */
                    (ONEINCH*10),/* width of table in twips 1"=1440 */
                    border_style, /* type of border to use          */
                    (ONEINCH/8),  /* Horz & Vert intercell spacing  */
                    CDTABLE_AUTO_CELL_WIDTH | CDTABLE_V4_BORDERS,  /* fit to Window      */
                    textlist_ptr,               /* text for cells   */
                    PLNREDCR10,              /* pre-set font to use */
                    &post_conversion_pabdef_ptr,/* used to build table's para */
                    &cd_buf_ptr,  /* ptr to begin of Rich Text Buf   */
                    &cd_buf_cnt))/* current offset in to Buf        */
    {
        PRINTLOG("\nMKTABLE ERROR - Table and\\or Text too big or");
        PRINTLOG(" dimension out of range!\n");
        goto Exit;
    }
    else
    {
        /* Get rid of the textlist */
        cleanup_state &= ~UNLOCK_LIST;
        OSUnlockObject(textlist_hdl);
        cleanup_state &= ~FREE_LIST;
        OSMemFree(textlist_hdl);
    }

    /**************************************************************/
    /* Build a 3x3, 6" wide single  border fit to window Table    */
    /**************************************************************/
    list_entries = textlist_size = 0;
    if (error = ListAllocate(0, 0, FALSE, &textlist_hdl,
                              &textlist_ptr, &textlist_size))
        goto Exit;
    else
    {
        cleanup_state |= FREE_LIST;

        /* Unlock text list before passing handle to ListAddEntry */
        OSUnlock(textlist_hdl);
    }

    rows = 3; columns = 3;
    for (countr = 0, list_entry = 0; countr < rows; countr++)
    {
        for (countc = 0; countc < columns; countc++)
        {
            strcpy(cell_str, "FIT TO WINDOW CELL ");
            sprintf(temp_buf,"%i",countr );
            strcat(cell_str,temp_buf);
            strcat(cell_str, ",");
            sprintf(temp_buf,"%i",countc );
            strcat(cell_str,temp_buf);

            if (error = ListAddEntry(textlist_hdl, FALSE,
                                     &textlist_size, list_entry++,
                                     cell_str, (WORD) strlen(cell_str)))
                goto Exit;
        }
    }

    /* Lock text list to refresh pointer before passing it to CDPutTable*/
    textlist_ptr = OSLockObject(textlist_hdl);
    cleanup_state |= UNLOCK_LIST;

    table_num = 2;
    border_style = TABLE_BORDER_SINGLE;
    if (!CDPutTable(table_num,
                    MIDDLE_REC, rows, columns, (ONEINCH*6),
                    border_style, (ONEINCH/8),
                    CDTABLE_AUTO_CELL_WIDTH | CDTABLE_V4_BORDERS | CDTABLE_ALTERNATINGCOLS,
                    textlist_ptr, PLNREDCR10,
                    &post_conversion_pabdef_ptr, &cd_buf_ptr, &cd_buf_cnt))
    {
        PRINTLOG("\nMKTABLE ERROR - Table and\\or Text too big or");
        PRINTLOG(" dimension out of range!\n");
        goto Exit;
    }
    else
    {
        /* Get rid of the textlist */
        cleanup_state &= ~UNLOCK_LIST;
        OSUnlockObject(textlist_hdl);
        cleanup_state &= ~FREE_LIST;
        OSMemFree(textlist_hdl);
    }


    /**************************************************************/
    /* EMPTY   3x3, 6" wide double  border fit to window Table    */
    /**************************************************************/

    table_num = 3;
    border_style = TABLE_BORDER_DOUBLE;
    rows = 3; columns = 3;
    if (!CDPutTable(table_num, LAST_REC, rows, columns, (ONEINCH*6),
             border_style, (ONEINCH/16),
             CDTABLE_AUTO_CELL_WIDTH | CDTABLE_V4_BORDERS | CDTABLE_ALTERNATINGROWS,
             NULL, PLNREDCR10,
             &post_conversion_pabdef_ptr, &cd_buf_ptr, &cd_buf_cnt))
    {
        PRINTLOG("\nMKTABLE ERROR - Table and\\or Text too big or");
        PRINTLOG(" dimension out of range!\n");
        goto Exit;
    }

    /* Append the completed TYPE_COMPOSITE item */
    if (error = NSFItemAppend( note_handle, 0, MAIL_BODY_ITEM,
                               (WORD) strlen(MAIL_BODY_ITEM),
                               TYPE_COMPOSITE, (BYTE *) anchor_pabdef_ptr, 
                               (DWORD) cd_buf_cnt ))
        goto Exit;

    if (error = NSFNoteUpdate(note_handle, 0))
        goto Exit;

    if (error = NSFNoteClose(note_handle))
        goto Exit;
    else
        cleanup_state &= ~CLOSE_NOTE;

    OSUnlockObject(cd_buf_handle);
    cleanup_state &= ~UNLOCK_BUFFER;
    OSMemFree(cd_buf_handle);
    cleanup_state &= ~FREE_BUFFER;

    if (error = NSFDbClose(db_handle))
        goto Exit;
    else
        cleanup_state &= ~CLOSE_DB;

Exit: /* ERROR HANDLING */
    if (cleanup_state)
    {
        if (cleanup_state & UNLOCK_BUFFER)
            OSUnlockObject(cd_buf_handle);

        if (cleanup_state & FREE_BUFFER)
            OSMemFree(cd_buf_handle);

        if (cleanup_state & UNLOCK_LIST)
            OSUnlockObject(textlist_hdl);

        if (cleanup_state & FREE_LIST)
            OSMemFree(textlist_hdl);

        if (cleanup_state & CLOSE_NOTE)
            NSFNoteClose(note_handle);

        if (cleanup_state & CLOSE_DB)
            NSFDbClose(db_handle);

      /* Free the allocated memory. */

       free (pre_conversion_pabdef_ptr);
    }

    if (!error)
	  PRINTLOG ("\nProgram completed successfully.\n");

    NotesTerm();
    return (error);
}

/************************************************************************
 
    FUNCTION:   ProcessArgs
 
    INPUTS:     argc, argv - directly from the command line
 
    OUTPUTS:    db_filename get data from the command line or from what
                the user types at a prompt
 
*************************************************************************/
 
void  LNPUBLIC  ProcessArgs (int argc, char *argv[], char *db_filename)
{
    if (argc != 2)
    {
        printf("Enter name of database: ");
        fflush(stdout);
        gets(db_filename);
    }
    else
    {
         strcpy(db_filename, argv[1]);
    } /* end if */
} /* ProcessArgs */


/*************************************************************************

    FUNCTION:   PrintAPIError

    PURPOSE:    This function prints the HCL C API for Notes/Domino 
		error message associated with an error code.

**************************************************************************/

void PrintAPIError (STATUS api_error)

{
    STATUS  string_id = ERR(api_error);
    char    error_text[200];
    WORD    text_len;

    /* Get the message for this HCL C API for Notes/Domino error code
       from the resource string table. */

    text_len = OSLoadString (NULLHANDLE,
                             string_id,
                             error_text,
                             sizeof(error_text));

    /* Print it. */

    fprintf (stderr, "\n%s\n", error_text);
}
