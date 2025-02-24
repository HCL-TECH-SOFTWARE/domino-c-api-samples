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
 * DESCRIPTION

This include file contains prototypes for low-level functions that
read or write specific types of Domino and Notes fields.

These functions are used in the GETNOTE and PUTNOTE programs.
*/


/* Function Prototypes */

extern WORD get_text_field (BLOCKID, WORD, char *, WORD);

extern int put_text_field (NOTEHANDLE, char *, char *);

extern int put_textlist_field (NOTEHANDLE, char *, char *);

extern int put_number_field (NOTEHANDLE, char *, char *);

extern int put_numberlist_field (NOTEHANDLE, char *, char *);

extern int put_time_field (NOTEHANDLE, char *, char *);

extern int put_timelist_field (NOTEHANDLE, char *, char *);

extern int put_richtext_field (NOTEHANDLE, char *, char *);

extern int put_userid_field (NOTEHANDLE, char *, char *);
