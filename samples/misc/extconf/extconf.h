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
 *              extconf.h - Header file for Extension Manager program
 */


/*
 *    Project platform-independence macros
 *
 * Typical usage:
 *    To declare an imported function:
 *       DLL_IMPORT_PREFIX <type> DLL_IMPORT_INFIX <name> ( <parameters> );
 *
 *    To declare an exported function:
 *       DLL_EXPORT_PREFIX <type> DLL_EXPORT_INFIX <name> ( <parameters> );
 */

#if defined(NT)
#define DLL_IMPORT_PREFIX  __declspec (dllimport)
#define DLL_EXPORT_PREFIX  __declspec (dllexport)
#define DLL_IMPORT_INFIX   
#define DLL_EXPORT_INFIX
#elif defined(DOSW16)
#define DLL_IMPORT_PREFIX
#define DLL_EXPORT_PREFIX
#define DLL_IMPORT_INFIX
#define DLL_EXPORT_INFIX   __export
#else
#define DLL_IMPORT_PREFIX
#define DLL_EXPORT_PREFIX
#define DLL_IMPORT_INFIX
#define DLL_EXPORT_INFIX
#endif

/* Domino and Notes Extension Manager entry - must have ordinal 1 */
STATUS LNPUBLIC DLL_EXPORT_INFIX MainEntryPoint (void);

/* Deregister extensions */
STATUS LNPUBLIC DLL_EXPORT_INFIX ExtClear (void);


