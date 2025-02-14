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

#ifndef __READFORM_H
#define __READFORM_H

/****************************************************************************

    PROGRAM:    readform

    FILE:       readform.h

    PURPOSE:    symbolic constants, types, and function prototypes

****************************************************************************/

#define TEXT_BUFFER_LENGTH     1024 

/*
 * Prototype of routine to locate form and get form information.
 */
 
STATUS LNPUBLIC ReadForm(char *pFormName);

/*
 * Prototype of routine to processes the body field.
 */
 
STATUS LNPUBLIC ReadBody(BLOCKID ValueBlockID,
                           DWORD dwLength,
                           char far *pOutputBuffer);

 
/*
 * Prototype of action routine.
 */
STATUS LNCALLBACK FormFields (char far *RecordPtr,
                              WORD RecordType,
                              DWORD RecordLength,
                              void far *pCtx);

#endif
