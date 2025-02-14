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

#ifndef XEDIT_DEFS
#define XEDIT_DEFS

/*  XEDIT local defines */
#define LINEOTEXT   256

/* String table defines */
#define PKG_XEDIT                           PKG_ADDIN
#define ERR_XEDIT_CDTOOBIG                  (PKG_XEDIT+128)
#define ERR_XEDIT_BADDELIM                  (PKG_XEDIT+129)
#define ERR_XEDIT_BADLEN                    (PKG_XEDIT+130)
#define ERR_XEDIT_BADSTRIP                  (PKG_XEDIT+131)

/* Dialog box defines */
#define IDD_OK           1
#define IDD_CANCEL       2

#define IDD_XEDIT      100
#define IDD_PROMPT1    101
#define IDD_PROMPT2    102
#define IDD_PROMPT3    103
#define IDD_STRING1    104
#define IDD_STRING2    105
#define IDD_STRING3    106

/* OS-specific, file manipulation defines/macros */
#if defined (W) || defined (W32)
#define FMODE_WRITE                  "w+"
#define FMODE_APPEND                 "a+"
#define __FILE                       FILE
#define __OpenFile( _fname, _mode )  fopen( _fname, _mode )
#define __CloseFile( _stream )       fclose( _stream )
#define __WriteFile( _buf, _stream ) fwrite( _buf, strlen(_buf), 1, _stream )
#define __FileError( _stream )       ferror( _stream )
#endif

/* Prototypes for Domino and Notes IMP/EXP functions */
STATUS LNPUBLIC ExpAsciiText(
  EDITEXPORTDATA *EditorData,
  WORD Flags,
  HMODULE hModule,
  char *AltLibraryName,
  char *FileName);

STATUS MapError(
  int wDosCode);

void  CleanupTrouble(
  HMODULE mod_handle,
  STATUS error);

#if defined (W16) || defined (W32)
BOOL LNPUBLIC XEditDlgProc(
  HWND hDlg,
  UINT message,
  UINT wParam,
  LONG lParam);
#endif

#endif
