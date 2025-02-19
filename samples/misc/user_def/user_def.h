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
 */

/* Local include file used by the USER_DEF sample program in the Lotus
C API for Domino and Notes kit */


/* Max size of user-defined field (in this program). In general,
user-defined fields can be up to 15K. 

For large fields you should use dynamic allocation for the field buffer,
rather than static allocation as this program does. */

#define MAX_USERDEF_FIELD 200


/* Max size of user-defined type descriptor string. The size must fit
into one byte, so 255 is the absolute maximum. */

#define MAX_USER_DESCRIP 255


/* Descriptive string for the data type created by this program. This
string distinguishes various user-defined types from each other. */

#define MY_DATA_TYPE "Chuck's Data Type"


/* Size of the data portion of the user-defined field. This size can be up to
(MAX_USERDEF_FIELD - (MAX_USER_DESCRIP + 1)). */

#define MY_DATA_LEN  100


/* Name of the Domino and Notes field that will hold the user-defined data type. */

#define MY_FIELD_NAME "ChucksData"


/* Max size (in this program) of the ASCII selection formula we use with
NSFSearch */

#define MAX_ASCII_FORMULA 100


/* Max size of user-defined path to the database file. */

#define STRING_LENGTH 256

/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:     argc, argv - directly from the command line

    OUTPUTS:    db_filename get data from the command line or from what
                the user types at a prompt

*************************************************************************/

void  LNPUBLIC  ProcessArgs (int argc, char *argv[], char *db_filename);
