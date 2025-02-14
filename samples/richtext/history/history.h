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

     HISTORY.H

***************************************************************************/

#define FILE_COLLECT                WM_USER+100
#define FILE_SELECT_DB              WM_USER+101
#define FILE_QUIT                   WM_USER+110

#define IDC_OK                      102
#define IDC_SELECT_DB               103
#define IDC_CANCEL                  104
#define IDC_DB_TITLE                105
#define IDC_VIEW_TITLE              107

#define IDC_SERVER                  113
#define IDC_FILE                    114

#define TITLE_LEN                   30
#define ERR_STR_LEN                 256
#define STATIC_TEXT_MAX_LEN         20
#define OVERFLOWSIZE       ((DWORD)1024*40)   /* appx. 40K */
#define MAX_BODY_ITEMS_TO_DELETE   125
#define MAX_BODY_VALUES_TO_APPEND  (2*MAX_BODY_ITEMS_TO_DELETE)

/* The following values are returned from the Collect History dialog
   box to the caller of DialogBox function.
*/
#define COLLECT_HIST_OK             1
#define COLLECT_HIST_SEL_DIFF_DB    2
#define COLLECT_HIST_CANCEL         3

/*  The name of the history field in the main document is "History".
    This is a TYPE_COMPOSITE field into which responses are collected.
*/

#define HISTORY_ITEM_NAME   "History"
#define HIS_ITEM_NAME_LEN    lstrlen(HISTORY_ITEM_NAME)

/*  The following values define the names and data types of items
    found in the response documents.  History formats these fields
    into the TYPE_COMPOSITE history field of the main documents.
    These names and types depend on the design of the target database.
*/

#define AUTHOR_ITEM_NAME    "From"
#define DATE_ITEM_NAME      "Date"
#define TOPIC_ITEM_NAME     "Subject"
#define COMMENTS_ITEM_NAME  "Body"

#define HISTORY_DEFAULT_VIEW_TITLE  "1. Date Received (Main View)"

/*  Convert Context data structure. The function ProcessOneBodyItem()
    passes the address of this structure as the optional fourth parameter
    to ProcessOneCDRecord(), the action routine of EnumCompositeBuffer().
*/
typedef struct _ConvertContext
{
    NOTELINK   *pNoteLinks;             /* address of first NOTELINK in an
                                           array of NOTELINKS from $Links */
    WORD        wNumNoteLinks;          /* number of NOTELINKS in array */
    BOOL        fUseBody2;              /* TRUE means overflow into Body2 */
    BYTE       *pNewBody;               /* top of buffer holding new body */
    BYTE       *pNewBodyCurrPos;        /* where to put next CD record */
    DWORD       dwNewBodyLen;           /* allocated length of new body */
    BYTE       *pNewBody2;              /* top of buffer holding overflow */
    BYTE       *pNewBody2CurrPos;       /* where to put next overflow CD */
    DWORD       dwNewBody2Len;          /* max length of overflow body */
}   CONVERTCONTEXT;

/* Body Value data structure. This structure contains the information
   NSFItemAppendByBLOCKID needs to append the new body items to the
   response notes. The function ExportLinks() defines an array of structures
   of type BODYVALUE. Used to pass information between ExportLinks() and
   ProcessOneBodyItem().
*/
typedef struct _NewBodyValue
{
    BLOCKID bid;
    DWORD   dwLen;
}   BODYVALUE;

/* Function prototypes */
int         WINAPI          WinMain( HINSTANCE, HINSTANCE, LPSTR, int );
BOOL        InitApplication( HANDLE );
BOOL        InitInstance( HANDLE, int );
LRESULT     CALLBACK        MainWndProc( HWND, UINT, WPARAM, LPARAM );
BOOL        CALLBACK        CollectHistoryDlgProc( HWND, UINT, WPARAM, LPARAM );
BOOL        CALLBACK        SelectDatabaseDlgProc( HWND, UINT, WPARAM, LPARAM );
BOOL        LNPUBLIC        InitCollHistDlg( HWND );
STATUS      LNPUBLIC        InitViewTitleList( HWND );
STATUS      LNCALLBACK      InsertOneViewItem( void*, SEARCH_MATCH*, ITEM_TABLE* );
BOOL        LNPUBLIC        InitSelDbDlg( HWND );
STATUS      LNPUBLIC        OpenNotesDB( LPSTR, LPSTR, DBHANDLE*, BOOL );
LPCSTR      LNPUBLIC        BuildErrStr( STATUS );
STATUS      LNPUBLIC        CollectHistory( DWORD* );
STATUS      LNPUBLIC        SkipToMainDocument( HANDLE*, DWORD*, HCOLLECTION*,
                            COLLECTIONPOSITION* );
BOOL        LNPUBLIC        IsCategory( HANDLE, DWORD );
STATUS      LNPUBLIC        ProcessMainDocument( HANDLE, DWORD );
STATUS      LNPUBLIC        ProcessResponses( HANDLE, DWORD );
STATUS      LNPUBLIC        ProcessOneResponse( NOTEID, HANDLE, DWORD );
STATUS      LNPUBLIC        ReplaceHistory( NOTEHANDLE, HANDLE );
void        LNPUBLIC        ExportLinks( NOTEHANDLE, char* );
STATUS      LNPUBLIC        ProcessOneBodyItem( NOTEHANDLE, NOTELINK*, WORD,
                            BLOCKID, BLOCKID, DWORD, BLOCKID*, WORD *,
                            BODYVALUE*, WORD* );
STATUS      LNCALLBACK      ProcessOneCDRecord( char*, WORD, DWORD, void* );
