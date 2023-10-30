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

This source file contains various routines used by the GETNOTE and
PUTNOTE programs.

*/

/* C include files */

#include <string.h>

/* Notes API include files */

#include <global.h>
#include <nsfnote.h>

/* Local include files */

#include "constant.h"

#ifdef LINIX
#define STRICMP strcasecmp
#endif

/* File-scope data */

static char record_separator;	/* separator between documents (records) */
static char field_separator;	/* separator between parts of a field */
static char list_separator;	/* separator between elements in a list */


/* Convert an ASCII string containing the name of a data type into a
Notes binary data type code. */

void datatype_ascii2bin (char * ascii_datatype, USHORT * p_datatype)

{
#ifdef LINUX
    if (strcasecmp(ascii_datatype, "TEXT") == 0)
#else
    if (STRICMP (ascii_datatype, "TEXT") == 0)
#endif 
    {
        *p_datatype = TYPE_TEXT;
        return;
    }

#ifdef LINUX
    if (strcasecmp(ascii_datatype, "TEXT_LIST") == 0)
#else
    if (STRICMP(ascii_datatype, "TEXT_LIST") == 0)
#endif 
    {
        *p_datatype = TYPE_TEXT_LIST;
        return;
    }

#ifdef LINUX
    if (strcasecmp(ascii_datatype, "NUMBER") == 0)
#else
    if (STRICMP(ascii_datatype, "NUMBER") == 0)
#endif 
    {
        *p_datatype = TYPE_NUMBER;
        return;
    }

#ifdef LINUX
    if (strcasecmp(ascii_datatype, "NUMBER_LIST") == 0)
#else
    if (STRICMP(ascii_datatype, "NUMBER_LIST") == 0)
#endif 
    {
        *p_datatype = TYPE_NUMBER_RANGE;
        return;
    }

#ifdef LINUX
    if (strcasecmp(ascii_datatype, "TIME") == 0)
#else
    if (STRICMP(ascii_datatype, "TIME") == 0)
#endif 
    {
        *p_datatype = TYPE_TIME;
        return;
    }

#ifdef LINUX
    if (strcasecmp(ascii_datatype, "TIME_LIST") == 0)
#else
    if (STRICMP(ascii_datatype, "TIME_LIST") == 0)
#endif 
    {
        *p_datatype = TYPE_TIME_RANGE;
        return;
    }

#ifdef LINUX
    if (strcasecmp(ascii_datatype, "RICH_TEXT") == 0)
#else
    if (STRICMP(ascii_datatype, "RICH_TEXT") == 0)
#endif 
    {
        *p_datatype = TYPE_COMPOSITE;
        return;
    }

#ifdef LINUX
    if (strcasecmp(ascii_datatype, "USER_ID") == 0)
#else
    if (STRICMP(ascii_datatype, "USER_ID") == 0)
#endif 
    {
        *p_datatype = TYPE_USERID;
        return;
    }

    *p_datatype = TYPE_INVALID_OR_UNKNOWN;
    return;
}





/* Convert a Notes data type (in binary format) into an ASCII string
that names the data type. */

void datatype_bin2ascii (USHORT datatype, char * ascii_datatype)

{
    switch (datatype)
    {
        case TYPE_TEXT:
            strcpy (ascii_datatype, "TEXT");
            break;
        case TYPE_TEXT_LIST:
            strcpy (ascii_datatype, "TEXT_LIST");
            break;
        case TYPE_NUMBER:
            strcpy (ascii_datatype, "NUMBER");
            break;
        case TYPE_NUMBER_RANGE:
            strcpy (ascii_datatype, "NUMBER_LIST");
            break;
        case TYPE_TIME:
            strcpy (ascii_datatype, "TIME");
            break;
        case TYPE_TIME_RANGE:
            strcpy (ascii_datatype, "TIME_LIST");
            break;
        case TYPE_COMPOSITE:
            strcpy (ascii_datatype, "RICH_TEXT");
            break;
        case TYPE_USERID:
            strcpy (ascii_datatype, "USER_ID");
            break;
        default:
            strcpy (ascii_datatype, "UNKNOWN");
            break;
    }

    return;
}


/* Read and remember the information in the PUTNOTE or GETNOTE control
parameter. Currently, this function does not check the control string
for errors, but it could. */

int set_control_info (char * control_string)

{
    record_separator = control_string[RECORD_SEPARATOR_CONTROL];
    field_separator = control_string[FIELD_SEPARATOR_CONTROL];
    list_separator = control_string[LIST_SEPARATOR_CONTROL];

    return (ERR_NO_ERROR);
}



/* Return the record separator character. */

char get_record_separator (void)

{
    return (record_separator);
}



/* Return the field separator character. */

char get_field_separator (void)

{
    return (field_separator);
}



/* Return the list separator character. */

char get_list_separator (void)

{
    return (list_separator);
}
