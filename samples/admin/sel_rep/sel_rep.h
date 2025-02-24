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

 * SEL_REP.H
 */

#ifndef __SEL_REP_H
#define __SEL_REP_H

#define TEXTLIST_BUFFER_LENGTH  1024
#define CLASS_STRING_LENGTH     10

/*
 *  Define the classes of notes that the sample program is interested
 *  in replicating.
 */
 
#define MY_NOTE_CLASSES (NOTE_CLASS_DOCUMENT | \
                         NOTE_CLASS_INFO | \
                         NOTE_CLASS_FORM | \
                         NOTE_CLASS_VIEW | \
                         NOTE_CLASS_ICON | \
                         NOTE_CLASS_DESIGN | \
                         NOTE_CLASS_ACL | \
                         NOTE_CLASS_HELP_INDEX | \
                         NOTE_CLASS_HELP | \
                         NOTE_CLASS_FILTER | \
                         NOTE_CLASS_FIELD | \
                         0)


#endif

