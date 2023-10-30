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
This source file contains low-level routines that read and write fields
in Domino and Notes documents. These functions are used by the GETNOTE 
and PUTNOTE programs.
*/

/* C include files */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <textlist.h>
#include <misc.h>
#include <miscerr.h>
#include <ods.h>
#include <editods.h>
#include <editdflt.h>
#include <fontid.h>
#include <osmem.h>
#include <colorid.h>

/* Local include files */

#include "constant.h"
#include "file_io.h"
#include "misc.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif


/* This function extracts a text field from a note. It handles the fact
that Domino and Notes uses embedded null characters to indicated new lines. */

WORD get_text_field (BLOCKID value_block,
                     WORD item_len,
                     char * output_buffer,
                     WORD max_output)
{

/* Local data */

    VOID    *item_ptr;
    WORD    i;
    char    c;
    WORD    output_len;


/* Convert the BLOCKID of the field value into a memory pointer. */

    item_ptr = OSLockBlock (VOID, value_block);

/* Reset the output length to zero. */

    output_len = 0;

/* Start a loop that will copy the text field to the output buffer. Replace
any null byte with a linefeed. (We skip over the two-byte datatype at the
start of the field.) */

    for (i = sizeof(USHORT); i < item_len; i++)
    {

/* Get one character. */

        c = * ((char *) item_ptr + i);

/* Write the character to the output buffer. */

        if (c != '\0')
            * (output_buffer + output_len) = c;
        else
            * (output_buffer + output_len) = LF;

/* Keep track of output length. */

        output_len++;

/* Check to see if we have exceeded the maximum output length. (We need
to save one extra byte to write the final null character.) */

        if (output_len >= max_output-1) break;

/* End of loop that is extracting the text field. */

    }

/* Add a null byte to the end of the output, so it will be a regular
C character string. */

    * (output_buffer + output_len) = '\0';

/* Unlock the memory block holding this field. */

    OSUnlockBlock (value_block);

/* End of function */

    return (output_len);
}





/* This function puts a text field into a note. It converts line feeds
characters into nulls as it does so, since Domino and Notes represents line feeds
with nulls. */

int put_text_field (NOTEHANDLE note_handle,
                    char * field_name,
                    char * field_text)
{

/* Local data */

    WORD    i;
    char    c;
    WORD    field_len;
    WORD    field_name_len;
    char    *output_buffer;
    STATUS  api_error;


/* Find the length of the field and the length of the name. */

    field_len = strlen (field_text);
    field_name_len = strlen (field_name);

/* Allocate a buffer that will hold the output. */

    output_buffer = (char *) malloc (field_len);

/* Start a loop that will copy the text field to the output buffer. Replace
any line feed with a null. */

    for (i = 0; i < field_len; i++)
    {

/* Get one character. */

        c = * (field_text + i);

/* Write the character to the output buffer. */

        if (c != LF)
            * (output_buffer + i) = c;
        else
            * (output_buffer + i) = '\0';

/* End of loop that is converting the text field. */

    }

/* Write this field to the note. */

    if (api_error = NSFItemAppend(note_handle, ITEM_SUMMARY,
                                  field_name, field_name_len,
                                  TYPE_TEXT,
                                  output_buffer, field_len))

    {
        log_api_message ("ERROR", api_error);
        return (ERR_API_ERROR);
    }

/* Free the buffer we created. */

    free (output_buffer);

/* End of function */

    return (ERR_NO_ERROR);
}




/* This function puts a text list field into a note. It strips out all
carriage returns and line feeds as it does so. CR and LF do not make sense
in any kind of list field, since the Domino and Notes form specifies the separator
between list elements. */

int put_textlist_field (NOTEHANDLE note_handle,
                        char * field_name,
                        char * field_text)

{

/* Local data */

    WORD    i;
    char    c;
    DHANDLE  list_handle;    /* handle to Domino and Notes list structure */
    VOID    *list_ptr;      /* pointer to same list */
    WORD    list_size;      /* total size of list structure */
    WORD    input_len;      /* total length of input */
    WORD    field_name_len; /* length of the field's name */
    WORD    element_len;    /* length of one element */
    char    *element_buffer;  /* buffer to hold one element */
    WORD    element_number;  /* position of one element in list */
    STATUS  api_error;


/* Create an empty text list data structure. */

    if (api_error = ListAllocate (0, 0, FALSE,
                                  &list_handle, &list_ptr, &list_size))
    {
        log_api_message ("ERROR", api_error);
        return (ERR_API_ERROR);
    }

/* Unlock the block of memory allocated by ListAllocate. 
ListAddEntry, below, may move this block of memory, so we will
need to refresh this pointer before calling NSFItemAllocate anyway.
For security, set this pointer to NULL to ensure that no one uses 
it before it gets refreshed. */

    OSUnlock (list_handle);
    list_ptr = NULL;

/* Find the overall length of the field (as input to this function) and
the length of the field name. */

    input_len = strlen (field_text);
    field_name_len = strlen (field_name);

/* Allocate a buffer that will hold one list element. (Any element in
the list cannot be longer than the whole list.) */

    element_buffer = (char *) malloc (input_len);

/* Start a loop that will copy each list element to the buffer and add it
to the field. Throw out all CR and LF, since these make no sense in a
list field. (The Domino and Notes form will add its own element separators at the
UI.) */

    element_len = 0;
    element_number = 0;

    for (i = 0; i < input_len; i++)
    {

/* Get one character. */

        c = * (field_text + i);

/* If the character is the separator between elements... 1) Write the item
we have already found to the field. 2) Reset the element length (to be used
for the next element). 3) Get the next character in the list. */

        if (c == get_list_separator())
        {
            if (api_error = ListAddEntry (list_handle,
                                          FALSE,
                                          &list_size,
                                          element_number,
                                          element_buffer,
                                          element_len))

            {
                log_api_message ("ERROR", api_error);
                OSMemFree (list_handle);
                free (element_buffer);
                return (ERR_API_ERROR);
            }

            element_number++;
            element_len = 0;
            continue;
        }

/* If the character is a CR or LF, throw it away. */

        if (c==CR || c==LF) continue;

/* We have a real character that is part of one list element. Save it and
keep track of the length of this element. */

        * (element_buffer + element_len++) = c;

/* End of loop that is scanning the input string. */

    }

/* Write the last element to the field. */

    if (api_error = ListAddEntry (list_handle,
                                  FALSE,
                                  &list_size,
                                  element_number,
                                  element_buffer,
                                  element_len))

    {
        log_api_message ("ERROR", api_error);
        OSMemFree (list_handle);
        free (element_buffer);
        return (ERR_API_ERROR);
    }

/* Add the completed field to the note. */

    list_ptr = OSLockObject (list_handle);

    if (api_error = NSFItemAppend(note_handle, ITEM_SUMMARY,
                                  field_name, field_name_len,
                                  TYPE_TEXT_LIST,
                                  list_ptr, list_size))

    {
        log_api_message ("ERROR", api_error);
        OSUnlock (list_handle);
        OSMemFree (list_handle);
        free (element_buffer);
        return (ERR_API_ERROR);
    }

/* Unlock and free the buffer that was holding the entire field. */

    OSUnlock (list_handle);
    OSMemFree (list_handle);

/* Free the buffer that was holding individual elements. */

    free (element_buffer);

/* End of function */

   return (ERR_NO_ERROR);
}





/* This function puts a number field into a note. */

int put_number_field (NOTEHANDLE note_handle,
                      char * field_name,
                      char * field_text)
{

/* Local data */

    WORD    field_name_len;
    NUMBER  binary_number;
    STATUS  api_error;


/* Get the length of the field's name. */

    field_name_len = strlen (field_name);

/* Convert the number to binary form. If the ASCII string is ill-formed,
the atof function returns a zero. We use this as the field in this case. */

    binary_number = atof (field_text);

/* Write this field to the note. */

    if (api_error = NSFItemAppend(note_handle, ITEM_SUMMARY,
                                  field_name, field_name_len,
                                  TYPE_NUMBER,
                                  &binary_number, sizeof(binary_number)))

    {
        log_api_message ("ERROR", api_error);
        return (ERR_API_ERROR);
    }

/* End of function */

    return (ERR_NO_ERROR);
}





/* This function puts a number list field into a note. It strips out all
carriage returns and line feeds as it does so. CR and LF do not make sense
in any kind of list field, since the Domino and Notes form specifies the separator
between list elements. */

int put_numberlist_field (NOTEHANDLE note_handle,
                          char * field_name,
                          char * field_text)
{

/* Local data */

    WORD    input_len;              /* total length of input */
    WORD    field_name_len;         /* length of field's name */
    WORD    element_len;            /* length of one element in list */
    WORD    element_count;          /* number of elements in list */
    char    ascii_number[MAX_NUMBER_FIELD];  /* one element in ASCII form */
    NUMBER  number;                 /* one element in binary form */
    RANGE   *range;                 /* header for number list */
    BYTE    *output_buffer;         /* Domino and Notes field that we create */
    WORD    output_len;             /* overall output length */
    WORD    i;                      /* a counter */
    char    c;                      /* one character from the input */
    STATUS  api_error;              /* return code from API calls */


/* Find the overall length of the field (as input to this function)
and the length of the field name. */

    input_len = strlen (field_text);
    field_name_len = strlen (field_name);

/* Allocate a buffer that will hold the completed number list in Domino and Notes
binary format. The header to the list will be a RANGE structure and
each element will fill one NUMBER structure. The number of numbers in the
list cannot be greater than the length of the list plus one. */

    output_buffer = (BYTE *) malloc (sizeof(RANGE) +
                    (sizeof(NUMBER) *
                    (input_len+1)));

/* Reset the counters we will use. */

    output_len = 0;
    element_len = 0;
    element_count = 0;

/* Advance the output length past the list header. We have not added this
yet to the list but will do so later. */

    output_len += sizeof (RANGE);

/* Start a loop that will extract each element from the list, convert it
to binary, and add it to the number list we are creating. Throw out all
CR and LF. */


    for (i = 0; i < input_len; i++)
    {

/* Get one character. */

        c = * (field_text + i);

/* If the character is the separator between elements...

1) Convert the element to binary, 2) Write the binary item to the list
structure, 3) Update the various counters. 4) Get the next character in
the input.

If the ASCII string for the number is ill-formed, the atof function returns
a zero. We use this as the output. */

        if (c == get_list_separator())
        {

            ascii_number[element_len] = '\0';
            number = atof (ascii_number);

            *((NUMBER *) (output_buffer + output_len)) = number;

            output_len += sizeof (NUMBER);
            element_count++;
            element_len = 0; /* for next element */

            continue;
        }

/* If the character is a CR or LF, throw it away. */

        if (c==CR || c==LF) continue;

/* We have a real character that is part of one list element. Save it and
keep track of the length of this element. */

        ascii_number[element_len++] = c;

/* End of loop that is scanning the input string. */

    }

/* Put the last element into the list -- just as we did above. */

    ascii_number[element_len] = '\0';
    number = atof (ascii_number);

    *((NUMBER *) (output_buffer + output_len)) = number;

    output_len += sizeof (NUMBER);
    element_count ++;

/* Add the header to the list (now that we know how many elements there
are). */

    range = (RANGE *) output_buffer;
    range->ListEntries = element_count;
    range->RangeEntries = 0;

/* Add the field to the note. */

    if (api_error = NSFItemAppend (note_handle, ITEM_SUMMARY,
                                   field_name, field_name_len,
                                   TYPE_NUMBER_RANGE,
                                   output_buffer, output_len))
    {
        log_api_message ("ERROR", api_error);
        free (output_buffer);
        return (ERR_API_ERROR);
    }

/* Free the buffer we created. */

    free (output_buffer);

/* End of function */

    return (ERR_NO_ERROR);
}









/* This function puts a time/date field into a note. */

int put_time_field (NOTEHANDLE note_handle,
                    char * field_name,
                    char * field_text)
{

/* Local data */

    WORD        input_len;
    WORD        field_name_len;
    TIMEDATE    binary_time;
    STATUS      api_error;
    char far *  pfield_text = (char far *)field_text;


/* Find the length of the field text (as input to this function) and
the length of the field's name. */

    input_len = strlen (field_text);
    field_name_len = strlen (field_name);

/* Convert the time/date to binary form. If the time string is bad, load
zeroes into the binary time so we have something to output. */

    if (api_error = ConvertTextToTIMEDATE(NULL,
                                          NULL,
                                          &pfield_text,
                                          input_len,
                                          &binary_time))
    {
        log_api_message ("WARNING", api_error);
        TimeDateClear (&binary_time);
    }

/* Write this field to the note. */

    if (api_error = NSFItemAppend(note_handle, ITEM_SUMMARY,
                                  field_name, field_name_len,
                                  TYPE_TIME,
                                  &binary_time, sizeof(binary_time)))

    {
        log_api_message ("ERROR", api_error);
        return (ERR_API_ERROR);
    }

/* End of function */

    return (ERR_NO_ERROR);
}






/* This function puts a time list field into a note. It strips out all
carriage returns and line feeds as it does so. CR and LF do not make sense
in any kind of list field, since the Domino and Notes form specifies the separator
between list elements. */

int put_timelist_field (NOTEHANDLE note_handle,
                        char * field_name,
                        char * field_text)
{

/* Local data */

    WORD     input_len;              /* total length of input */
    WORD     field_name_len;         /* length field name */
    WORD     element_len;            /* length of one element in list */
    WORD     element_count;          /* number of elements in list */
    char     ascii_time[MAX_TIME_FIELD]; /* one element in ASCII form */
    char far *ptime_ptr;              /* pointer to ASCII time string */
    TIMEDATE binary_time;            /* one element in binary form */
    RANGE    *range;                 /* header for time list */
    BYTE     *output_buffer;         /* Domino and Notes field that we create */
    WORD     output_len;             /* overall output length */
    WORD     i;                      /* a counter */
    char     c;                      /* one character from the input */
    STATUS   api_error;              /* return code from API calls */


/* Find the overall length of the field (as input to this function) and
the length of the field name. */

    input_len = strlen (field_text);
    field_name_len = strlen (field_name);

/* Allocate a buffer that will hold the completed time list in Domino and Notes
binary format. The header to the list will be a RANGE structure and
each element will fill one TIMEDATE structure. The number of timedates
in the list cannot be greater than the length of the list plus one. */

    output_buffer = (BYTE *) malloc (sizeof(RANGE) +
                    (sizeof(TIMEDATE) *
                    (input_len+1)));

/* Reset the counters we will use. */

    output_len = 0;
    element_len = 0;
    element_count = 0;

/* Advance the output length past the list header. We have not added this
yet to the list but will do so later. */

    output_len += sizeof (RANGE);

/* Start a loop that will extract each element from the list, convert it
to binary, and add it to the time list we are creating. Throw out all
CR and LF. */

    for (i = 0; i < input_len; i++)
    {

/* Get one character. */

        c = * (field_text + i);

/* If the character is the separator between elements...

1) Convert the element to binary, 2) Write the binary item to the list
structure, 3) Update the various counters. 4) Get the next character in
the input.

If the time string is bad, load zeroes into the binary time so we
have something to output. */

        if (c == get_list_separator())
        {

            ascii_time[element_len] = '\0';
            ptime_ptr = (char far *) ascii_time;

            if (api_error = ConvertTextToTIMEDATE(NULL,
                                                  NULL,
                                                  &ptime_ptr,
                                                  element_len,
                                                  &binary_time))

            {
                log_api_message ("WARNING", api_error);
                TimeDateClear (&binary_time);
            }

            memcpy (output_buffer+output_len, &binary_time, sizeof(TIMEDATE));

            output_len += sizeof (TIMEDATE);
            element_count++;
            element_len = 0; /* for next element */

            continue;
        }

/* If the character is a CR or LF, throw it away. */

        if (c==CR || c==LF) continue;

/* We have a real character that is part of one list element. Save it and
keep track of the length of this element. */

        ascii_time[element_len++] = c;

/* End of loop that is scanning the input string. */

    }

/* Put the last element into the list -- just as we did above. */

    ascii_time[element_len] = '\0';
    ptime_ptr = (char far *) ascii_time;

    if (api_error = ConvertTextToTIMEDATE(NULL,
                                          NULL,
                                          &ptime_ptr,
                                          element_len,
                                          &binary_time))

    {
        log_api_message ("WARNING", api_error);
        TimeDateClear (&binary_time);
    }

    memcpy (output_buffer+output_len, &binary_time, sizeof(TIMEDATE));

    output_len += sizeof (TIMEDATE);
    element_count++;

/* Add the header to the list (now that we know how many elements there
are). */

    range = (RANGE *) output_buffer;
    range->ListEntries = element_count;
    range->RangeEntries = 0;

/* Add the field to the note. */

    if (api_error = NSFItemAppend (note_handle, ITEM_SUMMARY,
                                   field_name, field_name_len,
                                   TYPE_TIME_RANGE,
                                   output_buffer, output_len))
    {
        log_api_message ("ERROR", api_error);
        free (output_buffer);
        return (ERR_API_ERROR);
    }

/* Free the buffer we created. */

    free (output_buffer);

/* End of function */

    return (ERR_NO_ERROR);
}

/* This function puts a rich text field into a note. It converts line
feeds characters into nulls as it does so, since Domino and Notes represents line
feeds with nulls. */

int put_richtext_field (NOTEHANDLE note_handle,
                        char * field_name,
                        char * field_text)
{

/* Local data */

    WORD                input_len;      /* length of field in ASCII form */
    WORD                field_name_len; /* length of the field's name */

    char                *rt_text;       /* string that goes into field */
    WORD                rt_text_len;    /* length of above string */

    CDPABDEFINITION     def;           /* rich-text paragraph style */
    CDPARAGRAPH         para;          /* rich-text paragraph header */
    CDPABREFERENCE      ref;           /* rich-text style reference */
    CDTEXT              text;          /* rich-text text header */
    FONTIDFIELDS        *font;         /* font definitions in text header */
    DWORD               rt_size;       /* size of rich-text field */
    WORD                wBuffLen;      /* required CD buffer length */

    BYTE                *rt_field;      /* pointer to entire field */
    BYTE                *buff_ptr;      /* moving pointer within field */

    STATUS              api_error;      /* error code from API calls */

    WORD                i;              /* a counter */
    char                c;              /* one character */


/* Get the length of the field length (as input to this function) and
the length of the field's name. */

    input_len = strlen (field_text);
    field_name_len = strlen (field_name);

/* Create a buffer to hold the actual text string that will become a part
of the rich text field. This string will be no longer than the string passed
into this function. */

    rt_text = (char *) malloc (input_len);

/* Copy the input text string into this buffer. As we do so, remove all CRs,
replace all LFs with NULL, and count the length of the result. */

    rt_text_len = 0;

    for (i = 0; i < input_len; i++)
    {

        c = * (field_text + i);

        if (c == CR)
            continue;
        else if (c == LF)
            * (rt_text + rt_text_len++) = '\0';
        else
            * (rt_text + rt_text_len++) = c;

    }

/* Determine the overall size of the rich text field. */

    wBuffLen = ODSLength (_CDPABDEFINITION) +
               ODSLength (_CDPARAGRAPH) +
               ODSLength (_CDPABREFERENCE) +
               ODSLength (_CDTEXT) +
               rt_text_len;

/* Allocate a buffer to hold the field. */

    rt_field = (BYTE *) malloc (wBuffLen);

/* Keep a pointer to our current position in the buffer. */

     buff_ptr = rt_field;

/* Put a CDPABDEFINITION structure in the field. We use all defaults. */

    memset(&def, 0, sizeof(CDPABDEFINITION));

    def.Header.Signature = SIG_CD_PABDEFINITION;
    def.Header.Length = sizeof(CDPABDEFINITION);
    def.PABID = 1;
    def.JustifyMode = DEFAULT_JUSTIFICATION;
    def.LineSpacing =        DEFAULT_LINE_SPACING;
    def.ParagraphSpacingBefore = DEFAULT_ABOVE_PAR_SPACING;
    def.ParagraphSpacingAfter = DEFAULT_BELOW_PAR_SPACING;
    def.LeftMargin = DEFAULT_LEFT_MARGIN;
    def.RightMargin = DEFAULT_RIGHT_MARGIN;
    def.FirstLineLeftMargin = DEFAULT_FIRST_LEFT_MARGIN;
    def.Tabs = DEFAULT_TABS;
    def.Tab[0] = DEFAULT_TAB_INTERVAL;
    def.Flags = 0;
    def.TabTypes = TAB_DEFAULT;
    def.Flags2 = 0;

/* Advance the buffer pointer, so we know where to put the next object in 
the buffer. */

    ODSWriteMemory(&buff_ptr, _CDPABDEFINITION, &def, 1);

/* Put a paragraph header in the field, and advance the pointer. */

    para.Header.Signature = SIG_CD_PARAGRAPH;
    para.Header.Length = (BYTE)ODSLength(_CDPARAGRAPH);

	ODSWriteMemory(&buff_ptr, _CDPARAGRAPH, &para, 1);

/* Put a paragraph reference block in the field, and advance the pointer.
Specify that the paragraph uses definition block #1 (above). */

    ref.Header.Signature = SIG_CD_PABREFERENCE;
    ref.Header.Length = (BYTE)ODSLength(_CDPABREFERENCE);
    ref.PABID = 1;
    
    ODSWriteMemory(&buff_ptr, _CDPABREFERENCE, &ref, 1);

/* Add a text header to the field, and advance the pointer. */

    text.Header.Signature = SIG_CD_TEXT;
    text.Header.Length = ODSLength(_CDTEXT) + rt_text_len;
    
/* Fill in the font information for this run of text. The font information
is stored in the text header that we already created. */

    font = (FONTIDFIELDS *) &(text.FontID);

    font->Face = FONT_FACE_SWISS;
    font->Attrib = 0;
    font->Color = NOTES_COLOR_BLACK;
    font->PointSize = 10;

    ODSWriteMemory(&buff_ptr, _CDTEXT, &text, 1);

/* Copy the text into the field. */

    memcpy ((char *)buff_ptr, rt_text, rt_text_len);
    buff_ptr += rt_text_len;

    rt_size = (DWORD)(buff_ptr - rt_field);

/* Add the rich-text field to the note. */

    if (api_error = NSFItemAppend (note_handle, 0,
                                   field_name, field_name_len,
                                   TYPE_COMPOSITE,
                                   rt_field, rt_size))
    {
        log_api_message ("ERROR", api_error);
        free (rt_text);
        free (rt_field);
        return (ERR_API_ERROR);
    }

/* Free the buffers we created. */

    free (rt_text);
    free (rt_field);

/* End of function */

    return (ERR_NO_ERROR);
}




/* This function puts a user id field into a note. The ASCII user name
is written out to the field, plus a dummy binary ID. */

int put_userid_field (NOTEHANDLE note_handle,
                      char * field_name,
                      char * field_text)
{

/* Local data */

    WORD    input_len;
    WORD    field_name_len;
    WORD    output_len;
    BYTE    *output_buffer;
    BYTE    *buff_ptr;
    LICENSEID id;
    STATUS  api_error;


/* Find the length of the field text (as input to this function) and
the length of the field's name. */

    input_len = strlen (field_text);
    field_name_len = strlen (field_name);

/* Find the length of the field that we will output. */

    output_len = input_len + sizeof (LICENSEID);

/* Allocate a buffer that will hold the output. Keep a pointer to our
current location in the output buffer. */

    output_buffer = (BYTE *) malloc (output_len);
    buff_ptr = output_buffer;

/* Copy the user name to the output buffer. */

    memcpy (buff_ptr, field_text, input_len);
    buff_ptr += input_len;

/* Create a dummy LICENSEID. */

    memset (&id, '\0', sizeof(LICENSEID));

/* Append the ID to the output buffer. */

    memcpy (buff_ptr, &id, sizeof(LICENSEID));

/* Write this field to the note. */

    if (api_error = NSFItemAppend(note_handle, ITEM_SUMMARY,
                                  field_name, field_name_len,
                                  TYPE_USERID,
                                  output_buffer, output_len))

    {
        log_api_message ("ERROR", api_error);
        return (ERR_API_ERROR);
    }

/* Free the buffer we created. */

    free (output_buffer);

/* End of function */

    return (ERR_NO_ERROR);
}

