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

This include file contains the function prototype for miscellaneous functions in
the PUTNOTE/GETNOTE package.

*/

extern void datatype_bin2ascii (USHORT, char *);

extern void datatype_ascii2bin (char *, USHORT *);

extern int set_control_info (char *);

extern char get_record_separator (void);

extern char get_field_separator (void);

extern char get_list_separator (void);
