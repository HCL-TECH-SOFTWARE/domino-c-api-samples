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

    FILE:       XVIEW.H

    PROGRAM:    XVIEW

    PURPOSE:    program definitions

*************************************************************************/
 
#ifndef XVIEW_DEFS
#define XVIEW_DEFS

/* Local defines */
#define LINEOTEXT      256
#define MAX_ITEM_SIZE  256

/* String table defines */
#define PKG_XVIEW                          PKG_ADDIN
#define ERR_XVIEW_OPENFILE                 (PKG_XVIEW+128)
#define ERR_XVIEW_NOSELECTED               (PKG_XVIEW+129)
#define ERR_XVIEW_NULLDBHANDLE             (PKG_XVIEW+130)
#define ERR_XVIEW_NULLITEMNAME             (PKG_XVIEW+131)
#define ERR_XVIEW_NSFNOTEOPEN              (PKG_XVIEW+132)
#define ERR_XVIEW_NSFNOTECLOSE             (PKG_XVIEW+133)
#define ERR_XVIEW_FILEWRITE                (PKG_XVIEW+134)

/* Dialog box defines */
#define IDD_OK           1
#define IDD_CANCEL       2

#define IDD_XVIEW      100
#define IDD_PROMPT1    101
#define IDD_PROMPT2    102
#define IDD_PROMPT3    103
#define IDD_STRING1    104
#define IDD_STRING2    105
#define IDD_STRING3    106

/* OS-specific, file manipulation defines/macros */
#if defined (W32) || defined (OS2_2x)
#define FMODE_WRITE                  "w+"
#define FMODE_APPEND                 "a+"
#define __FILE                       FILE
#define __OpenFile( _fname, _mode )  fopen( _fname, _mode )
#define __CloseFile( _stream )       fclose( _stream )
#define __WriteFile( _buf, _stream ) fwrite( _buf, strlen(_buf), 1, _stream )
#define __FileError( _stream )       ferror( _stream )
#endif

#endif
