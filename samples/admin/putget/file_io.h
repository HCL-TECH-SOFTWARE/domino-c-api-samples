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

This header file contains declarations for the platform-dependent
file I/O functions.

*/

extern int open_textfile (char *, int);
extern void close_textfile (void);

extern int open_logfile (char *);
extern void close_logfile (void);

extern void write_text_string (char *);
extern void write_text_char (char);

extern int read_text_char (char *);
extern void unread_text_char (char);

extern void log_message (char *, char *);
extern void log_api_message (char *, STATUS);
