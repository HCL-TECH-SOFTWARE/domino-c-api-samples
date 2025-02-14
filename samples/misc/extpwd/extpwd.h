/****************************************************************************
 *
 * Copyright HCL Technologies 1996, 2024.
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
 *      extpwd.h - Header file for Extension Manager password program
 */

#ifndef _EXTPWD_H_
#define _EXTPWD_H_

/*
 *      Project platform-independence macros
 *
 *      Typical usage:
 *           To declare an imported function:
 *           DLL_IMPORT_PREFIX <type> DLL_IMPORT_INFIX <name> ( <parameters> );
 *
 *           To declare an exported function:
 *           DLL_EXPORT_PREFIX <type> DLL_EXPORT_INFIX <name> ( <parameters> );
 */

#if defined(NT)
#define DLL_IMPORT_PREFIX       __declspec (dllimport)
#define DLL_EXPORT_PREFIX       __declspec (dllexport)
#define DLL_IMPORT_INFIX
#define DLL_EXPORT_INFIX
#else
#define DLL_IMPORT_PREFIX
#define DLL_EXPORT_PREFIX
#define DLL_IMPORT_INFIX
#define DLL_EXPORT_INFIX
#endif

/*
 *      Data type declarations
 */

typedef struct {
        DWORD           MaxPwdLen;
        char far *      FileName;
        char far *      OwnerName;
        DWORD far *     retLength;
        char far *      retPassword;
} EXT_DLG_DATA;

/*
 *       Constants and suchlike
 */

#define DLG_ANSWER_OK           0
#define DLG_ANSWER_CANCEL       1
#define DLG_ANSWER_ERROR        2

/*
 *       Function Declarations
 */

/*
 *       Master extension manager routines
 */

/* Domino and Notes Extension Manager entry - must have ordinal 1 */
DLL_EXPORT_PREFIX STATUS LNPUBLIC DLL_EXPORT_INFIX MainEntryPoint (void);

/* Deregister extensions */
DLL_EXPORT_PREFIX STATUS LNPUBLIC DLL_EXPORT_INFIX ExtClear (void);

/* Platform-dependent routine to fetch dialog text */
int LNPUBLIC ExtGetPasswordText (
        DWORD           MaxPwdLen,
        char far *      FileName,
        char far *      OwnerName,
        DWORD far *     retLength,
        char far *      retPassword
);

/* Master callback handler */
STATUS LNCALLBACK ExtHandler (EMRECORD far *pRecord);

/* Message box routine */
STATUS LNPUBLIC ExtDisplayMessageBox (
        char far *pMessage,
        char far *pBoxTitle
);

#endif
