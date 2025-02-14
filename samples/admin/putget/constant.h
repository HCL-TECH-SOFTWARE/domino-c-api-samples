/****************************************************************************
 *
 * Copyright HCL Technologies 1996, 2025.
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

   DESCRIPTION

This include file contains global constants for the GETNOTE and PUTNOTE
programs.
*/


/* Platform-specific definitions */

#ifdef W
   #define STRICMP _stricmp
#endif


/* String length maximums */

#define MAX_PARAM 100		/* max length of command line parameter */
#define MAX_API_ERR 200		/* max length of API error message */
#define MAX_TYPE_STRING 20	/* max length of ASCII data type string */
#define MAX_FIELD_NAME 50	/* max length of field name */
#define MAX_LOG_MESSAGE 200	/* max length of output to log file */

/* Data limits */

#define MAX_TEXT_FIELD 6000	/* max length of text field (in ASCII form) */
#define MAX_NUMBER_FIELD 50	/* max length of number field (in ASCII form) */
#define MAX_TIME_FIELD MAXALPHATIMEDATE + 1
                                /* max length of time field (in ASCII form) */
#define MAX_TEXTLIST_FIELD 6000	/* max length of text list field (in ASCII form) */
#define MAX_NUMBERLIST_FIELD 2000  /* max length of number list field (in ASCII form) */
#define MAX_TIMELIST_FIELD 2000	 /* max length of time list field (in ASCII form) */
#define MAX_RICHTEXT_FIELD 32000  /* max length of rich text field (in ASCII form) */
#define MAX_USERID_FIELD 100	/* max length of user ID (author) field (in ASCII form) */
#define MAX_UNKNOWN_FIELD 1000	/* max length of unknown type field (in ASCII form) */

/* Default program parameters */

#define DEFAULT_GETNOTE_DATABASE "GETNOTE.NSF"	/* GETNOTE input database */
#define DEFAULT_PUTNOTE_DATABASE "PUTNOTE.NSF"	/* PUTNOTE output database */
#define DEFAULT_GETNOTE_FORMULA "@All"	   /* GETNOTE selection formula */
#define DEFAULT_GETNOTE_TEXTFILE "GETNOTE.TXT"	/* GETNOTE text file */
#define DEFAULT_PUTNOTE_TEXTFILE "PUTNOTE.TXT"	/* PUTNOTE text file */
#define DEFAULT_GETNOTE_LOG "GETNOTE.LOG"	/* GETNOTE log file */
#define DEFAULT_PUTNOTE_LOG "PUTNOTE.LOG"	/* PUTNOTE log file */

#define DEFAULT_RECORD_SEPARATOR '*'	   /* separates records (documents) in text file */
#define DEFAULT_FIELD_SEPARATOR '^'	   /* separates parts of a field in the text file */
#define DEFAULT_LIST_SEPARATOR '|'	   /* separates parts of a list in the text file */

/* Positions of the items in the control strings that are passed
to PUTNOTE and GETNOTE */

#define RECORD_SEPARATOR_CONTROL 0
#define FIELD_SEPARATOR_CONTROL 1
#define LIST_SEPARATOR_CONTROL 2

/* Internal file I/O mode indicators */

#define FILE_MODE_READ 1
#define FILE_MODE_WRITE 2

/* Miscellaneous constants */

#define CONTROL_STRING_LEN 10	/* len of control parameter in main calling sequence */

#define LF '\xA'		/* line feed */
#define CR '\xD'		/* carriage return */



/* Internal PUTNOTE/GETNOTE error codes */

#define ERR_NO_ERROR 0

#define ERR_END_OF_FILE 1
#define ERR_END_OF_RECORD 2
#define ERR_BAD_TEXTFILE 3
#define ERR_IO_ERROR 4
#define ERR_UNSUPPORTED_TYPE 5
#define ERR_PUTNOTE_FAILED 6
#define ERR_GETNOTE_FAILED 7
#define ERR_CANNOT_OPEN_LOG 8

#define ERR_API_ERROR 90

#define ERR_INTERNAL_ERROR 99


/* These error codes are additions to the Notes API error code set. The API
allows for application-defined error values to be added in this way. The
first error code added must be PKG_ADDIN+2, since PKG_ADDIN+0 and
PKG_ADDIN+1 are reserved. */

#define ACTION_ROUTINE_FAILED	PKG_ADDIN+2
