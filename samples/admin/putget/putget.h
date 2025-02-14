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

This include file contains the function prototypes for the main
PUTNOTE and GETNOTE routines. These routines are platform independent. */

#include <global.h>

extern int LNPUBLIC putnote (int, char **, char far *, char far *, char far *, char far *, char far *);

extern int LNPUBLIC getnote (int, char **, char far *, char far *, char far *, char far *, char far *, char far *);
