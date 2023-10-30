/****************************************************************************
 *
 * Copyright HCL Technologies 1996, 2023.
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

    PROGRAM:    history

    FILE:       history.c

    PURPOSE:    Collect response documents into a History field in main docs.

    DESCRIPTION:

        This reads through a view of a service request tracking database,
        finds all the response documents to each main document, and
        copies all the data from the response documents into a rich text
        field in the corresponding main document.

        This puts the data from the response documents into a field
        called "History" in the corresponding main document. The
        History field in the main document is a rich text field. The
        service request form in the database defines the History field
        as a comuted (non-editable) rich text field.
        
        This program reads through any view of the database finding
        the main documents (service requests) and all responses to each
        request. It processes all the request documents by formatting the 
        text of the response notes as a list of entries in chronological
        order in the history field of the main document. Then it deletes
        the response documents.

        History is a complete windows application with a window, a menu,
        and two dialog boxes. When you launch History, a blank window is
        displayed. The windows has a "File" menu. The File menu has 3
        choices: Collect responses into History, Select Database to
        process, or Quit.

        When it first comes up, History looks for a database with file name
        "history.nsf" by default. You may selecting a different database
        using the "Select Database to Process" choice on the File menu.

        If you select "Collect responses into History" from the File menu,
        the program opens the CollectHistory dialog box. The CollectHistory
        dialog displays the name of the currently selected target database
        and prompts the user to select one of 3 choices: Collect responses
        into History, Select a different database, or cancel.

        The "Collect responses into History" button causes History to put
        the data from all the response documents into a field called
        "History" in the corresponding main documents.
        
        This program reads through any view of the database finding the main
        documents (service requests) and all responses to each request. It
        will read any view, but the view should show response hierarchy in
        order for the program to have any effect.

        The History field in the main document is a rich text field. The
        service request form in the database defines the History field as
        a computed (non-editable) rich text field. History formats the
        history field as a list of entries in chronological order. Each
        entry contains all the information from one response note. Then it
        deletes the response documents.

****************************************************************************/

/* Windows and C include files */
#include <stdlib.h>
#include <memory.h>
#include <windows.h>

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <nsfdb.h>
#include <nif.h>
#include <nsfdata.h>
#include <nsferr.h>
#include <nsfnote.h>
#include <nsfsearc.h>
#include <ostime.h>
#include <osmisc.h>
#include <osfile.h>
#include <osmem.h>
#include <stdnames.h>
#include <misc.h>
#include <easycd.h>
#include <editdflt.h>
#include <colorid.h>

/* Local INCLUDE FILES */

#include "history.h"
#include "histerr.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

extern int      __argc;
extern char **  __argv;

/* global variables */
HCURSOR            hSaveCursor;
HCURSOR            hHourGlass;

static DHANDLE     hInst;
static DHANDLE     hModule;
static HWND        hwndMain;                        /* Handle to main window */

STATUS             status;                          /* Status returned from HCL C API for Notes/Domino function calls. */

char               szMessageBoxTitle[TITLE_LEN+1];  /* "History Sample Program" */
char               szErrStr[ERR_STR_LEN+1];         /* formatted error messages */

static char        szDBFileNameString[MAXPATH];     /* target DB filename */
static char        szServerNameString[MAXPATH];     /* server of target DB */
static char        szDBFullPathString[2*MAXPATH];   /* full path to target DB */
static char        szDBTitleString[NSF_INFO_SIZE];  /* target DB title */
static DBHANDLE    hNotesDB = NULLHANDLE;           /* target DB handle */
static NOTEID      MainNoteID;                      /* ID of main document */
static NOTEID      nidSelectedView;                 /* ID of view to process */

/*************************************************************************

    FUNCTION: WinMain (DHANDLE, DHANDLE, LPSTR, int)
    
*************************************************************************/

int WINAPI WinMain (
        HINSTANCE     hInstance,
        HINSTANCE     hPrevInstance,
        LPSTR         lpCmdLine,
        int           nCmdShow )
{
    WORD    wLen;
    MSG     msg;      /* message.         */

    hModule = GetModuleHandle((LPSTR) "history");

    wLen = LoadString(hInstance,HISTORY_NAME,szMessageBoxTitle,TITLE_LEN);

    /* Initialize Domino and Notes. This also inits the global variable 'status' */
    if (status = NotesInitExtended(__argc, __argv))
    {
        /* Unable to Initialize Domino and Notes */
        MessageBox (GetFocus(), BuildErrStr(ERR_HISTORY_CANTINIT),
                    szMessageBoxTitle, MB_OK);
        return (FALSE);
    }

    if (!hPrevInstance)              /* Other instances of app running? */
    if (!InitApplication(hInstance)) /* Initialize shared things */
        return (FALSE);              /* Exits if unable to initialize  */

    /* Perform initializations that apply to a specific instance */

    if (!InitInstance(hInstance, nCmdShow))
        return (FALSE);

    /* Acquire and dispatch messages until WM_QUIT message is received. */

    while (GetMessage(&msg,              /* message structure                  */
                   (HWND) NULL,          /* handle of window receiving the msg */
                   (WORD) NULL,          /* lowest message to examine          */
                   (WORD) NULL))         /* highest message to examine         */
    {
        TranslateMessage(&msg); /* Translates virtual key codes.   */
        DispatchMessage(&msg);  /* Dispatches message to window.   */
    }

    NotesTerm();                /* Terminate the Domino and Notes runtime system. */

    return (msg.wParam);        /* Returns the value from PostQuitMessage */
}


/************************************************************************

    FUNCTION: InitApplication(DHANDLE)

*************************************************************************/

BOOL InitApplication(hInstance)
DHANDLE hInstance;               /* current instance         */
{
    WNDCLASS  wc;

    /*
     *  Fill in window class structure with parameters
     *  that describe the main window.
     */

    wc.style = (UINT) NULL;             /* Class style(s).        */
    wc.lpfnWndProc = MainWndProc;       /* Function for getting messages
                                            for windows of this class. */
    wc.cbClsExtra = 0;                  /* No per-class extra data.    */
    wc.cbWndExtra = 0;                  /* No per-window extra data.   */
    wc.hInstance = hInstance;           /* App. that owns the class.   */
    wc.hIcon = LoadIcon(hInstance, "Histicon");
    wc.hCursor = LoadCursor((DHANDLE) NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH); 
    wc.lpszMenuName =  "historyMenu";   /* Menu name in .RC file. */
    wc.lpszClassName = "historyClass"; /* Name used in CreateWindow call. */

    /* Register the window class. Return failure code if unsuccessfull. */

    return(RegisterClass(&wc));

}


/*************************************************************************

    FUNCTION:  InitInstance(DHANDLE, int)

    PURPOSE:  Saves instance handle and creates main window
        
*************************************************************************/

BOOL InitInstance(hInstance, nCmdShow)
    DHANDLE  hInstance;      /* Current instance identifier.       */
    int     nCmdShow;       /* Param for first ShowWindow() call. */
{
    HWND    hWnd;           /* Main window handle.                */
    WORD    wLen;
    STATUS  error;

    hHourGlass = LoadCursor((HINSTANCE)0, IDC_WAIT);

    /* Save the instance handle in static variable, which will be       */
    /* used in many subsequence calls from this application to Windows. */

    hInst = hInstance;

    /* Create a main window for this application instance.  */

    hWnd = CreateWindow(
                 "historyClass",                  /* See RegisterClass() call.    */
                  szMessageBoxTitle,              /* Text for window title bar.   */
                  WS_OVERLAPPEDWINDOW,            /* Window style.                */
                  CW_USEDEFAULT,                  /* Default horizontal position. */
                  CW_USEDEFAULT,                  /* Default vertical position.   */
                  CW_USEDEFAULT,                  /* Default width.               */
                  CW_USEDEFAULT,                  /* Default height.              */
                  (HWND) NULL,                    /* No parent.                   */
                  (HMENU) NULL,                   /* Use the window class menu.   */
                  hInstance,                      /* Owner of window.             */
                  (LONG) NULL                     /* Pointer not needed.          */
                  );

    /* If window could not be created, return "failure" */

    if (!hWnd)
        return (FALSE);

    hwndMain = hWnd;

    /* Make the window visible and update its client area. */

    ShowWindow(hWnd, nCmdShow);  /* Show the window          */
    UpdateWindow(hWnd);          /* Sends WM_PAINT message   */

   /*
      More instance-specific initialization. Load the default databse
      file name, "history", into global variable szDBFileNameString[].
      Load the default database server name ("") into global variable
      szServerNameString[]. Call OpenNotesDB to build the full net path
      to the database, and open it, and store the handle to the open DB
      in the global variable hNotesDB. Initialize the database title
      string stored in szDBTitleString[] to blank. InitCollHistDlg()
      will get the database title and save it in szDBTitleString[].
    */

    wLen = LoadString( hInstance, DEFAULT_HISTORY_DBFILENAME,
                       szDBFileNameString, MAXPATH );

    wLen = LoadString( hInstance, DEFAULT_HISTORY_DBSERVER,
                       szServerNameString, MAXPATH );

    error = OpenNotesDB( szServerNameString, szDBFileNameString,
                            &hNotesDB, TRUE );

    if (error)
    {  /* Unable to open the default database. Set global database handle
          to NULLHANDLE, disable the FILE_COLLECT menu item, and return.
          This allows user to select a different target database.
        */
        hNotesDB = NULLHANDLE;
        EnableMenuItem( GetMenu( hWnd ), FILE_COLLECT, MF_GRAYED );
    }

    szDBTitleString[0] = '\0';

    return (TRUE);               /* Returns value from PostQuitMessage */

}

/*************************************************************************

    FUNCTION: MainWndProc(HWND, UINT, WPARAM, LPARAM)

    PURPOSE:  Processes messages

    MESSAGES:

        WM_COMMAND    - application menu
            FILE_COLLECT - Collect responses into History...
            FILE_SELECT_DB - Select database to process...
            FILE_QUIT - Quit
        WM_DESTROY    - destroy window

    COMMENTS:

    To process the FILE_COLLECT message, call MakeProcInstance()
    to get the current instance address of the CollectHistoryDlgProc()
    function. Then call DialogBox, which displays the dialog box according
    to the information in the resource file and turns control over to
    CollectHistoryDlgProc(). The DialogBox function returns one of three
    values depending on what button the user clicks. Free the instance
    address then handle each case.

    Process the FILE_SELECT_DB message by calling DialogBox to display
    the "Select a Notes Database" modal dialog box in a manner similar to
    the above.

*************************************************************************/

LRESULT CALLBACK MainWndProc (
        HWND        hWnd,
        UINT        message,
        WPARAM      wParam,
        LPARAM      lParam )
{
    FARPROC lpProcCollectHistory;   /* func ptr to CollectHistoryDlgProc */
    FARPROC lpProcSelectDB;         /* func ptr to SelectDatabaseDlgProc */
    int     nResult;                /* return code from DialogBox */
    DWORD   dwResponsesProcessed;   /* total no. response notes processed*/
    STATUS  error;                  /* returned from CollectHistory func */
    WORD    idItem;                 /* Command item ID */

    switch (message)
    { 
        case WM_COMMAND:       /* message: command from app.  menu */
#ifdef _WIN32
            idItem = LOWORD(wParam);
#else
            idItem = wParam;
#endif
            switch (idItem)
            {
                case FILE_COLLECT:

                    lpProcCollectHistory = MakeProcInstance(
                                            CollectHistoryDlgProc, hInst );
                   /*
                    * The CollectHistory dialog proc displays the name of a
                    * target database and prompts the user to select one of
                    * 3 choices: Collect responses into History, Select a
                    * different database, or cancel.
                    */

                    nResult = DialogBox( hInst,   /* current instance */
                                   "HISTORYDLG",  /* resource to use  */
                                   hWnd,          /* parent handle    */
                                   (DLGPROC)lpProcCollectHistory ); /* instance address */
                               
                    FreeProcInstance( lpProcCollectHistory );

                    switch( nResult )
                    {
                        case COLLECT_HIST_OK:

                           /*
                            * The user clicked "Collect responses into
                            * History", so call the CollectHistory()
                            * function to open the selected view,
                            * navigate to each main document in the view,
                            * and find all the responses to each main
                            * document.
                            */
                            hSaveCursor = SetCursor( hHourGlass );

                            error = CollectHistory( &dwResponsesProcessed );

                            SetCursor( hSaveCursor );

                            if (error)
                            {
                                /* Failure! */
                                MessageBox( GetFocus (),
                                BuildErrStr(ERR_HISTORY_FAILURE),
                                szMessageBoxTitle, MB_ICONHAND | MB_OK );
                            }
                            else if (dwResponsesProcessed == (DWORD)0)
                            {
                                /* no response documents were found. */
                                MessageBox( GetFocus (),
                                BuildErrStr(ERR_HISTORY_NORESPONSE),
                                szMessageBoxTitle, MB_OK );
                            }
                            else
                            {
                                /* Success! */
                                MessageBox( GetFocus (),
                                BuildErrStr(ERR_HISTORY_SUCCESS),
                                szMessageBoxTitle, MB_OK );
                            }
                            break;

                        case COLLECT_HIST_SEL_DIFF_DB:

                            goto LABELSelectDatabase;
                            break;

                        case COLLECT_HIST_CANCEL:
                        default:
                            break;
                    }
                    break;

                case FILE_SELECT_DB:
                LABELSelectDatabase:

                    lpProcSelectDB = MakeProcInstance(
                                            SelectDatabaseDlgProc, hInst );

                    DialogBox( hInst, "SELECTDBDLG",  /* resource to use  */
                               hWnd,                  /* parent handle */
                               (DLGPROC)lpProcSelectDB );      /* instance address */
                               
                    FreeProcInstance( lpProcSelectDB );

                    break;

                case FILE_QUIT:
                
                   /*
                    * User chose to quit the program.  If the database
                    * was left open, close it.  Then send the Windows
                    * message to close the application window.
                    */
             
                    if (hNotesDB != NULLHANDLE)
                        NSFDbClose (hNotesDB);

                    hNotesDB = NULLHANDLE;
                    SendMessage (hWnd, WM_CLOSE, 0, 0L);
                    break;
            } /* end of switch (wParam) */
            break; /* end of case WM_COMMAND */

       case WM_DESTROY:            /* message: window being destroyed */
            if (hNotesDB != NULLHANDLE)
                NSFDbClose( hNotesDB );
            PostQuitMessage(0);
            break;

       default:                    /* Passes it on if unproccessed    */
            return (DefWindowProc(hWnd, message, wParam, lParam));
    } 
    return 0;
} 

/************************************************************************

    FUNCTION: CollectHistoryDlgProc (HWND, WORD, WPARAM, LONG)

    PURPOSE:  Processes messages for "Collect History" dialog box

    MESSAGES:

    WM_INITDIALOG - initialize dialog box
    WM_COMMAND    - Input received:
            IDC_OK - "Collect responses into History" button clicked
            IDC_SELECT_DB - "Select a different database" button clicked
            IDC_CANCEL - "Cancel" button clicked.

    COMMENTS:

***********************************************************************/

BOOL CALLBACK CollectHistoryDlgProc (
        HWND        hDlg,
        UINT        message,
        WPARAM      wParam,
        LPARAM      lParam )
{
    DWORD   dwIndex;
    DWORD   dwData;
    WORD    idItem;

    switch (message)
    {
        case WM_INITDIALOG:         /* message: initialize dialog box */
            return( InitCollHistDlg(hDlg) );

        case WM_COMMAND:            /* message: received a command */
#ifdef _WIN32
            idItem = LOWORD(wParam);
#else
            idItem = wParam;
#endif
            switch (idItem)
            {
                case IDC_OK:        /* "Collect responses into History" */

                   /* Get the note ID of the selected view and store
                      it in the global variable nidSelectedView. The
                      view note IDs are stored in the combo box item
                      data associated with each item in the list.
                    */
                    dwIndex = SendDlgItemMessage( hDlg,
                                IDC_VIEW_TITLE, CB_GETCURSEL, 0, 0);

                    if (dwIndex == CB_ERR)
                    {
                        /* Unable to get ID of selected View */
                        MessageBox( GetFocus (),
                                BuildErrStr(ERR_HISTORY_GETSELVIEWID),
                                szMessageBoxTitle, MB_ICONHAND | MB_OK );
                        EndDialog( hDlg, COLLECT_HIST_CANCEL );
                        return FALSE;
                    }

                    dwData = SendDlgItemMessage( hDlg,
                                    IDC_VIEW_TITLE, CB_GETITEMDATA,
                                    (WPARAM) dwIndex, (LPARAM)0 );

                    nidSelectedView = LOWORD(dwData);

                    EndDialog( hDlg, COLLECT_HIST_OK );
                    return TRUE;

                case IDC_SELECT_DB: /* "Select a different database" */

                   /* Return a code to the caller of DialogBox that
                      indicates the user wants to select a different
                      Domino database.
                    */
                    EndDialog( hDlg, COLLECT_HIST_SEL_DIFF_DB );
                    return (TRUE);

                case IDC_CANCEL:    /* "Cancel" button clicked.*/
                case IDCANCEL:      /* System menu close command. */

                    EndDialog( hDlg, COLLECT_HIST_CANCEL );
                    return ( TRUE );

                default:
                     break;

            }       /* end of switch (wParam) */
            break;  /* end of case WM_COMMAND */

    }   /* end of switch (message) */

    return (FALSE);                 /* Didn't process a message.  */
}

/************************************************************************

    FUNCTION: SelectDatabaseDlgProc (HWND, WORD, WPARAM, LONG)

    PURPOSE:  Processes messages for "Select Notes Database" dialog box

    MESSAGES:

    WM_INITDIALOG - initialize dialog box
    WM_COMMAND    - Input received:
            IDOK - "OK" button clicked
            IDCANCEL - "Cancel" button clicked.

    COMMENTS:
        Use the SelectDbDlg dialog box to prompt the user to enter
        the name of a HCL Domino Server and the name of a Domino database
        file. When the user clicks OK, construct the full network
        path to the database and open the database to obtain the
        handle. Store the file name, server name, and database handle
        in global variables. If unable to open the database specified
        by the server and the file name, stay in this dialog box
        until the user specifies a good one or clicks Cancel.

***********************************************************************/

BOOL CALLBACK SelectDatabaseDlgProc (
        HWND        hDlg,
        UINT        message,
        WPARAM      wParam,
        LPARAM      lParam )
{
    char        szUserDBFileName[MAXPATH];
    char        szUserServerName[MAXPATH];
    DBHANDLE    hUserDB;
    WORD        idItem;

    switch (message)
    {
        case WM_INITDIALOG:         /* message: initialize dialog box */
            return( InitSelDbDlg(hDlg) );

        case WM_COMMAND:            /* message: received a command */
#ifdef _WIN32
            idItem = LOWORD(wParam);
#else
            idItem = wParam;
#endif
            switch (idItem)
            {
                case IDOK:
                    /* User clicked OK. Get the server name and file name
                       that the user entered in the edit controls. Use these
                       to try to open the specified database. If successfull,
                       copy the strings and Db handle to global variables.
                     */

                    GetDlgItemText( hDlg, IDC_SERVER,
                                    szUserServerName,
                                    MAXPATH-1);
                    GetDlgItemText( hDlg, IDC_FILE,
                                    szUserDBFileName,
                                    MAXPATH-1);

                    status = OpenNotesDB( szUserServerName,
                                          szUserDBFileName,
                                          &hUserDB, FALSE );

                    if (status)
                    {  /* Unable to open the specified database. OpenNotesDB
                          has already printed a message explaining why.
                          Return TRUE but do not call EndDialog. This way,
                          the user remains in this dialog box until success
                          or user clicks the Cancel button.
                       */
                        return (TRUE);
                    }
                       /* else we successfully opened the specified Domino DB.
                          If the global variable hNotesDB is non-NULL, close
                          that database. Save the new open notes db handle in
                          the global variable hNotesDB and copy the new file
                          and server names to the global variables. Clear the
                          database title stored it in the global variable
                          szDBTitleString[] to force InitCollHistDlg to
                          refresh this with the new DB. Enable the FILE_COLLECT
                          menu item. Call EndDialog to exit the dialog box.
                       */

                    if (hNotesDB != NULLHANDLE)
                        NSFDbClose( hNotesDB );
                    hNotesDB = hUserDB;
                    lstrcpy( szDBFileNameString, szUserDBFileName );
                    lstrcpy( szServerNameString, szUserServerName );

                    szDBTitleString[0] = '\0';

                    EnableMenuItem( GetMenu( hwndMain ), FILE_COLLECT,
                                    MF_ENABLED );

                    EndDialog( hDlg, TRUE );

                    return (TRUE);

                case IDCANCEL:      /* System menu close command. */
                    EndDialog( hDlg, TRUE );
                    return ( TRUE );
                default:
                    return FALSE;
            }
        default:
            break;
        }
    return FALSE ;
}

/************************************************************************

    FUNCTION: InitCollHistDlg

    PURPOSE:  Initialize the "Response Collector" dialog box.

***********************************************************************/

BOOL LNPUBLIC InitCollHistDlg( HWND hCollHistDlg )
{
    char        szBuffer[NSF_INFO_SIZE];

    if (szDBTitleString[0] == '\0')
    {
        if (status = NSFDbInfoGet( hNotesDB, szBuffer ))
        {
            /* Unable to get title of Domino database */
            MessageBox( GetFocus (), BuildErrStr(ERR_HISTORY_NBFDBINFO),
                       szMessageBoxTitle, MB_OK );
            szDBTitleString[0] = '\0';
        }
        else
        {
            NSFDbInfoParse( szBuffer, INFOPARSE_TITLE,
                            szDBTitleString,
                            NSF_INFO_SIZE - 1);
        }
    }

    /* Initialize the IDC_DB_TITLE control in the dialog box with the
       database title stored in szDBTitleString.
     */
    SetDlgItemText( hCollHistDlg,
                    IDC_DB_TITLE, /* ID of Dlg item "Database Title" */
                    (LPCSTR) szDBTitleString );

   /* Clear any items that may already be in the view titles combo box.
      Then find out what views are available in this database and populate
      the combo box with the titles of all the views in the database.
    */
    SendDlgItemMessage( hCollHistDlg,
                        IDC_VIEW_TITLE,
                        CB_RESETCONTENT,
                        0, 0 );

    if (InitViewTitleList( hCollHistDlg ) != NOERROR)
        return FALSE;

    return TRUE;
}


/************************************************************************

    FUNCTION: InitViewTitleList

    PURPOSE:  Initialize the IDC_VIEW_TITLE combo box with the titles
              and view note IDs of all the views available in the DB.

    COMMENTS:

    Get the note IDs of all the views in the database. For each view, get
    the view title. Populate the IDC_VIEW_TITLE combo box with the view
    titles. Store the note ID of each view in the lower oder word of the
    doubleword value corresponding item in the combo box.

*************************************************************************/

STATUS LNPUBLIC InitViewTitleList( HWND hCollHistDlg )
{
    FARPROC     lpProcInsertOne;
    LPCSTR      lpszRoutineName = "InsertOneViewItem";
    WORD        wNumViewItems;

    /* Create a proc instance for the action routine InsertOneViewItem(). */
    lpProcInsertOne = MakeProcInstance( (FARPROC) InsertOneViewItem, hInst );

    if (lpProcInsertOne == (FARPROC)NULL)
    {
        /* Unable to make procedure instance for InsertOneViewItem.*/
        MessageBox( GetFocus (), BuildErrStr(ERR_HISTORY_MAKEPROC),
                   szMessageBoxTitle, MB_ICONHAND | MB_OK );
        return (ERR_HISTORY_MAKEPROC);
    }

    /* Search the DB for all view notes. */

    status = NSFSearch( hNotesDB,
                        NULLHANDLE,
                        NULL,
                        SEARCH_SUMMARY,     /* get summary buffer with each match */
                        NOTE_CLASS_VIEW,    /* find all views */
                        NULL,               /* starting date (unused) */
                                            /* call for each note found */
                        (NSFSEARCHPROC)lpProcInsertOne,
                        (void*)(&hCollHistDlg),/* argument to InsertOneViewItem */
                        NULL);              /* returned ending date (unused) */

    FreeProcInstance( lpProcInsertOne );

    if (status)
    {
        /* Unable to search for views in database.*/
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_SEARCH),
                    szMessageBoxTitle, MB_ICONHAND | MB_OK );
        return( ERR(status) );
    }

    wNumViewItems = (WORD) SendDlgItemMessage( hCollHistDlg,
                                               IDC_VIEW_TITLE,
                                               CB_GETCOUNT, 0, 0);

    if (wNumViewItems > 0)
    {
        SendDlgItemMessage( hCollHistDlg,
                            IDC_VIEW_TITLE,
                            CB_SETCURSEL,
                            (WPARAM)0, (LPARAM)0 );
    }

    return NOERROR;
}

/************************************************************************

    FUNCTION: InsertOneViewItem

    PURPOSE:  Action routine - insert one item in the View selection
              combo box.

    COMMENTS:
        This is the action routine called by NSFSearch when searching
        for all views in the database. This routine gets the title of
        the view and the view Note ID. It adds the title as a new entry
        in the IDC_VIEW_TITLE combo box of the "Response Collector"
        dialog box. Then it stores the View note ID in the lower order
        word of a DWORD and sets this DWORD into the item data associated
        with the new combo box item.

*************************************************************************/

STATUS LNCALLBACK InsertOneViewItem( void * phCollHistDlg,
                             SEARCH_MATCH far *pSearchMatch,
                             ITEM_TABLE * summary_buffer)
{
    SEARCH_MATCH SearchMatch;
    HWND         hCollHistDlg;
    NOTEID       nidViewID;
    char         szViewTitle[DESIGN_VIEW_MAX];
    DWORD        dwIndex;

    memcpy( (char*)&SearchMatch, (char*)pSearchMatch, sizeof(SEARCH_MATCH) );

    if (!(SearchMatch.SERetFlags & SE_FMATCH))
        return (NOERROR);

    hCollHistDlg = *((HWND*)phCollHistDlg);

    nidViewID = SearchMatch.ID.NoteID;

    /* Obtain the view title (a text item) from the summary buffer */
    if (FALSE == NSFGetSummaryValue(
                            summary_buffer,
                            VIEW_TITLE_ITEM,    /* "$TITLE" */
                            szViewTitle,
                            DESIGN_VIEW_MAX ))
    {
        /* Unable to get title of View note */
        MessageBox( GetFocus (), BuildErrStr(ERR_HISTORY_VIEWTITLE),
                   szMessageBoxTitle, MB_ICONHAND | MB_OK );
        return NOERROR;
    }

    dwIndex = SendDlgItemMessage( hCollHistDlg,
                                  IDC_VIEW_TITLE,
                                  CB_ADDSTRING,
                                  0, (LPARAM)((LPSTR)szViewTitle) );

    if (dwIndex == CB_ERR)
    {
        /* Unable to add View title to list.*/
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_ADDVIEW),
                    szMessageBoxTitle, MB_ICONHAND | MB_OK );
    }
    else if (dwIndex == CB_ERRSPACE)
    {
        /* insufficient space available to add View title to list.*/
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_ADDVIEWSPACE),
                    szMessageBoxTitle, MB_ICONHAND | MB_OK );
    }
    else /* dwIndex is the zero-based index of the new entry in the list */
    {
        /* Store the View note ID in the lower order word of item data. */
        SendDlgItemMessage( hCollHistDlg,
                            IDC_VIEW_TITLE,
                            CB_SETITEMDATA,
                            (WPARAM)dwIndex,
                            MAKELPARAM((WORD)nidViewID, 0) );
    }
    return NOERROR;
}

/************************************************************************

    FUNCTION:   InitSelDbDlg

    PURPOSE:    Initialize the "Select a Notes Database" dialog box.

    COMMENTS:
        Initialize the IDC_SERVER control with the server name, and the
        IDC_FILE control with the database file name.

************************************************************************/

BOOL LNPUBLIC InitSelDbDlg( HWND hSelDBDlg )
{
   /* Initialize the IDC_SERVER control in the dialog box with the
      server name stored in global variable szServerNameString.
    */
    SetDlgItemText( hSelDBDlg,
                    IDC_SERVER, /* ID of Dlg item "Database Title" */
                    (LPCSTR) szServerNameString );

   /* Initialize the IDC_FILE control in the dialog box with the
       database file name stored in global variable szDBFileNameString.
    */
    SetDlgItemText( hSelDBDlg,
                    IDC_FILE, /* ID of Dlg item "Database Title" */
                    (LPCSTR) szDBFileNameString );

    return( TRUE );
}



/************************************************************************
                                                                        
     FUNCTION:  OpenNotesDB()                                           
                                                                        
     PURPOSE:   Open the Domino Db (nsf file) on the specified server.

     INPUTS:
        lpszServer - null-terminated server name string. Use "" to
                     to open a local database (resides on local system.)

        lpszNsfName - null-terminated Domino database file name.

        fIsDefault  - flag that controlls what error message to display
                      if unable to open database.

     OUTPUT:    *phDb gets the handle to the open database

*************************************************************************/

STATUS LNPUBLIC OpenNotesDB( LPSTR lpszServer, LPSTR lpszNsfName,
                                DBHANDLE * phDb, BOOL fIsDefault )
{
    char    fullpath_name[MAXPATH+100];

    if (lstrlen( lpszServer ) != 0) /* remote server specified */
    {
        status = OSPathNetConstruct( NULL, lpszServer, lpszNsfName,
                                    fullpath_name );
        if (status != NOERROR)
        {
            /* unable to construct network path to database.*/
            MessageBox( GetFocus (), BuildErrStr(ERR_HISTORY_DBPATH),
                       szMessageBoxTitle, MB_ICONHAND | MB_OK );
            return( ERR(status) );
        }
    }
    else                                /* DB is LOCAL: just use nsf name */
        lstrcpy( fullpath_name, lpszNsfName );

    /* Check user input in case DB full path name too long */

    if (lstrlen( fullpath_name ) > MAXPATH)
    {
        /* Database full path name is too long.*/
        MessageBox( GetFocus (), BuildErrStr(ERR_HISTORY_PATHTOOLONG),
                       szMessageBoxTitle, MB_ICONHAND | MB_OK );
        return ERR_HISTORY_PATHTOOLONG;
    }

    if (status = NSFDbOpen( fullpath_name, phDb ))
    {
        if (fIsDefault)
        {
            /* Unable to open Default Domino database */
            MessageBox( GetFocus (), BuildErrStr(ERR_HISTORY_CANTOPENDEFDB),
                       szMessageBoxTitle, MB_OK );
        }
        else
        {
            /* Unable to open Domino database.*/
            MessageBox( GetFocus (), BuildErrStr(ERR_HISTORY_CANTOPENDB),
                       szMessageBoxTitle, MB_OK );
        }
        return( ERR(status) );
    }
    /* successfully opened database. Store the full net path in global.*/
    lstrcpy( szDBFullPathString, fullpath_name );

    return NOERROR;
}


/****************************************************************************

    FUNCTION: BuildErrStr (STATUS error)

    PURPOSE:  Build an error string based on a HISTORY error code,
              <error> and a Domino and Notes error code <status>.

    COMMENTS:

        Loads the string specified by the string ID <error> into the global
        variable szErrStr. Then, if the global variable <status> is
        non-zero, this appends the error string associated with <status>
        to the string stored in szErrStr. This returns szErrStr.

        The <error> code input to this function must contain the resource
        ID of a string in the HISTORY string table (see HISTORY.RC). The
        global variable <status> must contain a HCL C API for Domino and 
        Notes error code.

****************************************************************************/

LPCSTR LNPUBLIC BuildErrStr (STATUS error)
{
    WORD    wLen;

    OSLoadString( hModule, error, szErrStr, ERR_STR_LEN );
    if (status)
    {
        lstrcat( szErrStr, " - \n" );
        wLen = lstrlen( szErrStr );
        OSLoadString ( NULLHANDLE, ERR(status), szErrStr+wLen,
                                                (WORD) (ERR_STR_LEN-wLen) );
    }
    return (szErrStr);
}

/************************************************************************

    FUNCTION: CollectHistory

    PURPOSE:  Collect responses into history field of main documents
              for all documents in the selected view of the database.

    COMMENTS:
        This function opens the selected view, navigates to each
        main document in the view, and finds all the responses to
        each main document. It processes all response documents by
        calling ProcessResponses(), which collects the information
        into the "History" field of the main document. This loops
        through the view until it reaches the end of the view.

        If a single main document has more than about 32,000 response
        documents, this function will not process all the responses.
        This limitation arises because this function does not check for
        the SIGNAL_MORE_TO_DO bit in the SignalFlags parameter returned
        from NIFReadEntries.

*************************************************************************/

STATUS LNPUBLIC  CollectHistory( DWORD * pdwResponsesProcessed )
{
    STATUS                error;
    HCOLLECTION           hCollection;            /* handle to open collection */
    BOOL                  fFirstTime = TRUE;      /* used to detect empty view */
    COLLECTIONPOSITION    coll_pos;               /* collection position */
    COLLECTIONPOSITION    SaveCollPos;            /* save coll_pos for comparison */
    DHANDLE               hRetBuff;               /* return buffer from ReadEntries */
    DWORD                 dwNotesFound;           /* number of notes read */
    int                   i;
    BOOL                  fCollPosTumblerChanged; /* used when comparing coll_pos */

    /* initialize return value to 0 */
    *pdwResponsesProcessed = (DWORD)0;

                                        /* open view note (collection) */
    status = NIFOpenCollection( hNotesDB, hNotesDB, nidSelectedView,
                                0, NULLHANDLE, &hCollection,
                                NULLHANDLE, NULL, NULLHANDLE,
                                NULLHANDLE );
    if (status)
    {   /* Unable to open selected database view.*/
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_OPENVIEW),
                       szMessageBoxTitle, MB_ICONHAND | MB_OK );
        return( ERR(status) );
    }

    coll_pos.Level = 0;             /* set coll_pos to first doc in view */
    coll_pos.MinLevel = 0;
    coll_pos.MaxLevel = 0;
    coll_pos.Tumbler[0] = (DWORD)1;

    while(TRUE)                         /* loop over all main docs in view */
    {
        status = NIFReadEntries( hCollection,
                                &coll_pos,          /* where the match begins */
                                NAVIGATE_CURRENT,   /* order to skip entries */
                                0,                  /* no. of entries to skip */
                                NAVIGATE_NEXT_PEER, /* navigate at this level */
                                1,                  /* just get one note id */
                                READ_MASK_NOTEID,   /* get the note ID */
                                &hRetBuff,          /* Note ID return buffer */
                                NULL,               /* length of return buffer */
                                NULL,               /* no. entries skipped */
                                &dwNotesFound,      /* entries read */
                                NULL);              /* signal warning */
        if (status)
        {
            /* Unable to read next entry in view.*/
            MessageBox( GetFocus (), BuildErrStr(ERR_HISTORY_READNEXT),
                       szMessageBoxTitle, MB_ICONHAND | MB_OK );
            if (hRetBuff != NULLHANDLE)
            {
                OSMemFree( hRetBuff );
            }
            NIFCloseCollection( hCollection );
            return( ERR(status) );
        }
        if ( (dwNotesFound == (DWORD)0) && (fFirstTime == TRUE) )
        {
            /* No documents found in selected view.*/
            MessageBox( GetFocus (), BuildErrStr(ERR_HISTORY_EMPTYVIEW),
                       szMessageBoxTitle, MB_OK );
            if (hRetBuff != NULLHANDLE)
            {
                OSMemFree( hRetBuff );
            }
            NIFCloseCollection( hCollection );
            return NOERROR;
        }
        fFirstTime = FALSE;

        /* If note is a category note, skip to next main document.
           If note is a main document, this function does nothing.
         */
        if (status = SkipToMainDocument( &hRetBuff, &dwNotesFound,
                                        &hCollection, &coll_pos ))
        {
            OSMemFree( hRetBuff );
            NIFCloseCollection( hCollection );
            return( ERR(status) );
        }
                                        /* process the main doc */
        error = ProcessMainDocument( hRetBuff, dwNotesFound );
        OSMemFree( hRetBuff );
        if (error)
        {
            NIFCloseCollection( hCollection );
            return( ERR(error) );
        }
                                        /* read all response docs */
        status = NIFReadEntries( hCollection,
                                &coll_pos,          /* where the match begins */
                                NAVIGATE_CHILD,     /* skip main document */
                                1,                  /* skip 1 to responses */
                                NAVIGATE_NEXT_PEER, /* navigate at this level */
                                0xFFFFFFFF,         /* return ALL responses */
                                READ_MASK_NOTEID,   /* get the note IDs */
                                &hRetBuff,          /* Note ID return buffer */
                                NULL,               /* length of return buffer */
                                NULL,               /* no. entries skipped */
                                &dwNotesFound,      /* entries read */
                                NULL);              /* signal warning */
        if (status)
        {
            /* Unable to read response documents.*/
            MessageBox( GetFocus (), BuildErrStr(ERR_HISTORY_READRESP),
                       szMessageBoxTitle, MB_ICONHAND | MB_OK );
            if (hRetBuff != NULLHANDLE)
            {
                OSMemFree( hRetBuff );
            }
            NIFCloseCollection( hCollection );
            return( ERR(status) );
        }
                                        /* process response docs */
        error = ProcessResponses( hRetBuff, dwNotesFound );
        OSMemFree( hRetBuff );
        if (error)
        {
            NIFCloseCollection( hCollection );
            return( ERR(error) );
        }

        (*pdwResponsesProcessed) += dwNotesFound;

        /*
         * Skip to next main document in view. If no more documents, break 
         * out of loop. To detect that there are no more main docs, save 
         * the current position, then skip 1 to next main document
         * but read 0 entries. Then compare new position to saved value. If 
         * position didn't change, have reached end of view. Else go back 
         * to top of loop to process next main document. 
         */
        
        SaveCollPos = coll_pos ;
         
        status = NIFReadEntries( hCollection, &coll_pos,
                                NAVIGATE_NEXT_MAIN, 1,
                                0, 0, 0, NULL, NULL, NULL, NULL, NULL );
        if (status)
        {
            /* Unable to skip from response documents in view. */
            MessageBox( GetFocus (), BuildErrStr(ERR_HISTORY_SKIPRESP),
                       szMessageBoxTitle, MB_ICONHAND | MB_OK );
            NIFCloseCollection( hCollection );
            return( ERR(status) );
        }
                                        /* done when no change in coll pos */
        for(i = 0, fCollPosTumblerChanged = FALSE;
            i <= (int)min( SaveCollPos.Level, coll_pos.Level );
            i++)
        {
            if (SaveCollPos.Tumbler[i] != coll_pos.Tumbler[i])
            {
                fCollPosTumblerChanged = TRUE;
            }
        }
        if (!fCollPosTumblerChanged && (SaveCollPos.Level == coll_pos.Level))
            break;                      /* coll pos unchanged: done! */
        
    }                                   /* end of loop over main docs */

    NIFCloseCollection( hCollection );
    return NOERROR;
}


/*************************************************************************

   FUNCTION:  SkipToMainDocument()

   PURPOSE:   If the note ID in the specified buffer identifies a
              category note, then skip to the next non-category note,
              adjusting collectionposition, buff_handle, and notes_found.

   RETURNS:   status

*************************************************************************/

STATUS LNPUBLIC   SkipToMainDocument( DHANDLE *phRetBuff,
                                        DWORD * pdwNotesFound,
                                        HCOLLECTION *phCollection,
                                        COLLECTIONPOSITION *pCollPos )
{
    while (IsCategory( *phRetBuff, *pdwNotesFound ))
    {
        /* Is a category note. Calling NIFReadEntries to skip 1  */
        OSMemFree( *phRetBuff );
        status = NIFReadEntries( *phCollection,
                                pCollPos,           /* where the match begins */
                                NAVIGATE_NEXT,      /* order to skip entries */
                                1,                  /* no. of entries to skip */
                                NAVIGATE_NEXT_PEER, /* navigate at this level */
                                1,                  /* just get one note id */
                                READ_MASK_NOTEID,   /* get the note ID */
                                phRetBuff,          /* Note ID return buffer */
                                NULL,               /* length of return buffer */
                                NULL,               /* no. entries skipped */
                                pdwNotesFound,      /* entries read */
                                NULL );             /* signal warning */
        if (status)
        {
            /* Unable to skip category note to main document. */
            MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_SKIPCAT),
                    szMessageBoxTitle, MB_ICONHAND | MB_OK );
            if (*phRetBuff != NULLHANDLE)
            {
                OSMemFree( *phRetBuff );
            }
            return( ERR(status) ) ;
        }
    }
    return NOERROR;
}

/************************************************************************

   FUNCTION:  IsCategory()

   RETURNS:   TRUE if Note ID in buffer specifies a category note,
              FALSE otherwise.

*************************************************************************/

BOOL LNPUBLIC IsCategory( DHANDLE BuffHdl, DWORD dwNotesFound )
{
    NOTEID *pNoteIDData = NULL;
    BOOL    fRetval = FALSE;

    pNoteIDData = (NOTEID *)OSLockObject( BuffHdl );

    if (NOTEID_CATEGORY & *pNoteIDData)
    {
        fRetval = TRUE;
    }
    OSUnlockObject( BuffHdl );
    return( fRetval );
}

/****************************************************************************

   FUNCTION:  ProcessMainDocument()

   PURPOSE:   Just store the Note ID of main document in global variable
              MainNoteID for use when processing response documents to it.

*****************************************************************************/

STATUS LNPUBLIC ProcessMainDocument( DHANDLE hBuff, DWORD dwNotesFound )
{
    char   *pBuffer = NULL;
      
    if (dwNotesFound <= (DWORD)0)        /* need to handle empty buffers */
        return NOERROR;
   
    pBuffer = OSLockObject( hBuff );

    MainNoteID = *((NOTEID *)pBuffer);

    OSUnlockObject( hBuff );

    return NOERROR;
}

/****************************************************************************

   FUNCTION:  ProcessResponses()

   PURPOSE:   Process all responses to one main document.

   INPUTS:    hBuff - a buffer full of response Note IDs.
              dwNotesFound - the number of response note IDs

   RETURNS:   status

*****************************************************************************/

STATUS LNPUBLIC ProcessResponses( DHANDLE hBuff, DWORD dwNotesFound )
{
    NOTEHANDLE    hMainDoc;               /* main doc handle */
    char         *pBuffer = NULL;
    NOTEID       *pNoteIDData = NULL;
    int           i;
    STATUS        error;
    DHANDLE       hCompound;              /* handle to Compound Text context */
    COMPOUNDSTYLE Style;
    DWORD         dwStyleID;

    /* An empty buffer means there are no responses to this main document */
    if (dwNotesFound <= (DWORD)0)
    {
        return NOERROR;
    }
    /* Open main document. */
    if (status = NSFNoteOpen( hNotesDB, MainNoteID, 0, &hMainDoc ))
    {
        /* Unable to open main document in view */
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_OPENMAIN),
                    szMessageBoxTitle, MB_ICONHAND | MB_OK );
        return( ERR(status) );
    }

    /* Create a stand-alone Composite Text context. */
    if (status = CompoundTextCreate( NULLHANDLE, NULL, &hCompound ))
    {
        /* Unable to create compound text context */
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_CONTEXT),
                    szMessageBoxTitle, MB_ICONHAND | MB_OK );
        NSFNoteClose( hMainDoc );
        return( ERR(status) );
    }
    /* Initialize a standard paragraph styles: left justified, etc. */
    CompoundTextInitStyle( &Style );

    /* Indent the paragraphs by 0.3 inches */
    Style.LeftMargin = (ONEINCH/3);

    if (status = CompoundTextDefineStyle( hCompound, "", &Style,
                                            &dwStyleID ))
    {
        /* Unable to define compound text style */
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_STYLE),
                    szMessageBoxTitle, MB_ICONHAND | MB_OK );
        CompoundTextDiscard( hCompound );
        NSFNoteClose( hMainDoc );
        return( ERR(status) );
    }

   /* Assimilate the existing history field from the main document
      into the new compound text context.
    */
    if (status = CompoundTextAssimilateItem (
                        hCompound,      /* context to assimilate into */
                        hMainDoc,       /* main doc to get item from */
                        HISTORY_ITEM_NAME, /* name of item: "History" */
                        0L))
    {
        /* Unable to assimilate history field from main document */
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_OLDHISTORY),
                    szMessageBoxTitle, MB_ICONHAND | MB_OK );
        CompoundTextDiscard( hCompound );
        NSFNoteClose( hMainDoc );
        return( ERR(status) );
    }

   /* Now loop over each Note ID in the buffer. The buffer contains
      the Note IDs of all the responses to this main document. For
      each response, assimilate that response note into the compound
      text context.
    */

    pBuffer = OSLockObject( hBuff );

    pNoteIDData = (NOTEID *) pBuffer;

    for( i=0, error = NOERROR;
        (i<(int)dwNotesFound) && (error == NOERROR);
        i++)
    {
        error = ProcessOneResponse( *(pNoteIDData+i), hCompound, dwStyleID );
    }
    OSUnlockObject( hBuff );

   /* If error, clean up and return. ProcessOneResponse has already
      displayed an error message.
    */
    if (error)
    {
        CompoundTextDiscard( hCompound );
        NSFNoteClose( hMainDoc );
        return( ERR(error) );
    }

   /* The compound text context now contains the assimilated history
      of the main document and all responses to it. Now delete the
      History field that was initially in the main document, and
      replace it with the assimilated compound text.
    */
    if (error = ReplaceHistory( hMainDoc, hCompound ))
    {
        CompoundTextDiscard( hCompound );
        NSFNoteClose( hMainDoc );
        return( ERR(error) );
    }

    status = NSFNoteUpdate( hMainDoc, 0 );
    NSFNoteClose( hMainDoc );

    if (status)
    {
        /* Unable to update main document with history. */
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_UPDATE),
                szMessageBoxTitle, MB_ICONHAND | MB_OK );
        return( ERR(status) );
    }
    /* We successfully updated the main document. Since we are now certain
      that the information has been safely assimilated, delete all the
      response documents to the current main document.
    */
    pBuffer = OSLockObject( hBuff );

    pNoteIDData = (NOTEID *) pBuffer;

    for (i=0; (i<(int)dwNotesFound) && (status == NOERROR); i++)
    {
        if (status = NSFNoteDelete( hNotesDB, *(pNoteIDData+i), 0 ))
        {
            /* unable to delete response note after assimilation */
            MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_DELETE),
                szMessageBoxTitle, MB_ICONHAND | MB_OK );
        }
    }
    OSUnlockObject( hBuff );
    return( ERR(status) );
}

/****************************************************************************

   FUNCTION:  ProcessOneResponse

   PURPOSE:   Process one Note. Called once for each response note in
              the buffer.

   RETURNS:   status

   COMMENTS:
        ProcessOneResponse() process just one response note. It append
        "Response Dated: " followed by the contents of Date field from the
        response note to the compound text context. Then it appends "Author:"
        followed by the author of the response note, and so similarly for
        the Subject filed. Since the "comments" field of the response
        document is a rich text field, ProcessOneResponse() can call
        CompoundTextAssimilateItem() to assimilate it from the response
        document into the compound text context.

*****************************************************************************/

STATUS LNPUBLIC ProcessOneResponse( NOTEID nidResp,
                                      DHANDLE hCompound,
                                      DWORD dwStyleID )
{
    NOTEHANDLE  hRespNote;
    FONTID      fidBoldBlue;
    FONTID      fidBlack;
    char        szAuthorStaticText[STATIC_TEXT_MAX_LEN];
    DWORD       dwAuthorStaticTextLen;
    char        szDateStaticText[STATIC_TEXT_MAX_LEN];
    char        dwDateStaticTextLen;
    char        szTopicStaticText[STATIC_TEXT_MAX_LEN];
    char        dwTopicStaticTextLen;
    char        szCommentsStaticText[STATIC_TEXT_MAX_LEN];
    char        dwCommentsStaticTextLen;
    BLOCKID     bhAuthorItem, bhTopicItem;
    WORD        wAuthorType, wTopicType;
    BLOCKID     bhAuthorValue, bhTopicValue;
    DWORD       dwAuthorValueLen, dwTopicValueLen;
    DHANDLE     hAuthorText, hTopicText;
    DWORD       dwAuthorTextLen, dwTopicTextLen;
    char        szDateText[MAXALPHATIMEDATE];
    WORD        wDateTextLen;

    if (status = NSFNoteOpen( hNotesDB, nidResp, 0, &hRespNote ))
    {
        /* Unable to open response document */
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_RESPONSE),
                szMessageBoxTitle, MB_ICONHAND | MB_OK );
        return ( ERR(status) );
    }

    fidBoldBlue = DEFAULT_BOLD_FONT_ID;
    FontSetColor(fidBoldBlue,NOTES_COLOR_BLUE);

    fidBlack = DEFAULT_FONT_ID;

   /* Load some strings from the string table into local variables.
      Append these strings to the rich text buffer in place of static
      text in the response docs.
    */

    dwDateStaticTextLen = LoadString( hInst,
                        HISTORY_RESP_DATE_STAT_TXT,
                        szDateStaticText, STATIC_TEXT_MAX_LEN );

    dwAuthorStaticTextLen = LoadString( hInst,
                        HISTORY_RESP_AUTHOR_STAT_TXT,
                        szAuthorStaticText, STATIC_TEXT_MAX_LEN );

    dwTopicStaticTextLen = LoadString( hInst,
                        HISTORY_RESP_TOPIC_STAT_TXT,
                        szTopicStaticText, STATIC_TEXT_MAX_LEN );

    dwCommentsStaticTextLen = LoadString( hInst,
                        HISTORY_RESP_COMMENTS_STAT_TXT,
                        szCommentsStaticText, STATIC_TEXT_MAX_LEN );

   /* Append "Response Dated: " followed by the contents of Date field */
    if (status = CompoundTextAddTextExt( hCompound, dwStyleID, fidBoldBlue,
                         szDateStaticText,
                         dwDateStaticTextLen,
                         "\n",
                         COMP_PRESERVE_LINES, NULL ))
    {
        /* Unable to add Author static text */
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_AUTHSTAT),
                szMessageBoxTitle, MB_ICONHAND | MB_OK );
        NSFNoteClose( hRespNote );
        return ( ERR(status) );
    }

    wDateTextLen = NSFItemConvertToText( hRespNote,
                                DATE_ITEM_NAME, /* "Date" */
                                szDateText,
                                MAXALPHATIMEDATE,
                                0 );

    if (status = CompoundTextAddTextExt( hCompound, dwStyleID, fidBlack,
                            szDateText,
                            (DWORD)wDateTextLen,
                            "\n",
                            COMP_PRESERVE_LINES,
                            NULL ))
    {
        /* Unable to add Date text */
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_DATEITEM),
                szMessageBoxTitle, MB_ICONHAND | MB_OK );
        NSFNoteClose( hRespNote );
        return ( ERR(status) );
    }

   /* Append the static text "Author: " followed by the contents of
      the From field.
    */

    if (status = CompoundTextAddTextExt( hCompound, dwStyleID, fidBoldBlue,
                         szAuthorStaticText,
                         dwAuthorStaticTextLen,
                         "\n",
                         COMP_PRESERVE_LINES, NULL ))
    {
        /* Unable to add Author static text */
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_AUTHSTAT),
                szMessageBoxTitle, MB_ICONHAND | MB_OK );
        NSFNoteClose( hRespNote );
        return ( ERR(status) );
    }

    if (status = NSFItemInfo( hRespNote, AUTHOR_ITEM_NAME, /* "From" */
                             (WORD) lstrlen(AUTHOR_ITEM_NAME),
                             &bhAuthorItem,
                             &wAuthorType,
                             &bhAuthorValue,
                             &dwAuthorValueLen ))
    {
        /* Unable to get From item */
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_FROMITEMINFO),
                szMessageBoxTitle, MB_ICONHAND | MB_OK );
        NSFNoteClose( hRespNote );
        return ( ERR(status) );
    }

    if (status = ConvertItemToText( bhAuthorValue,
                            dwAuthorValueLen, "", MAXWORD,
                            &hAuthorText,
                            &dwAuthorTextLen,
                            FALSE ))
    {
        /* Unable to convert author (From item) to text */
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_FROMITEMTEXT),
                szMessageBoxTitle, MB_ICONHAND | MB_OK );
        NSFNoteClose( hRespNote );
        return ( ERR(status) );
    }

    status = CompoundTextAddTextExt( hCompound, dwStyleID, fidBlack,
                            OSLockObject( hAuthorText ),
                            dwAuthorTextLen,
                            "\n",
                            COMP_PRESERVE_LINES,
                            NULL );

    OSUnlockObject( hAuthorText );
    OSMemFree( hAuthorText );

    if (status)
    {
        /* Unable to add Author item text */
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_AUTHITEM),
                szMessageBoxTitle, MB_ICONHAND | MB_OK );
        NSFNoteClose( hRespNote );
        return ( ERR(status) );
    }

    /* Append "Topic: " followed by the contents of the Subject field */
    if (status = CompoundTextAddTextExt( hCompound, dwStyleID, fidBoldBlue,
                         szTopicStaticText,
                         dwTopicStaticTextLen,
                         "\n",
                         COMP_PRESERVE_LINES, NULL ))
    {
        /* Unable to add Topic static text */
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_TOPICSTAT),
                szMessageBoxTitle, MB_ICONHAND | MB_OK );
        NSFNoteClose( hRespNote );
        return ( ERR(status) );
    }

    if (status = NSFItemInfo( hRespNote, TOPIC_ITEM_NAME, /* "Subject" */
                             (WORD) lstrlen(TOPIC_ITEM_NAME),
                             &bhTopicItem,
                             &wTopicType,
                             &bhTopicValue,
                             &dwTopicValueLen ))
    {
        /* Unable to get Topic item */
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_TOPICITEMINFO),
                szMessageBoxTitle, MB_ICONHAND | MB_OK );
        NSFNoteClose( hRespNote );
        return ( ERR(status) );
    }

    if (status = ConvertItemToText( bhTopicValue,
                            dwTopicValueLen, "", MAXWORD,
                            &hTopicText,
                            &dwTopicTextLen,
                            FALSE ))
    {
        /* Unable to convert Topic (Subject item) to text */
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_TOPICITEMTEXT),
                szMessageBoxTitle, MB_ICONHAND | MB_OK );
        NSFNoteClose( hRespNote );
        return ( ERR(status) );
    }

    status = CompoundTextAddTextExt( hCompound, dwStyleID, fidBlack,
                            OSLockObject( hTopicText ),
                            dwTopicTextLen,
                            "\n",
                            COMP_PRESERVE_LINES,
                            NULL );

    OSUnlockObject( hTopicText );
    OSMemFree( hTopicText );

    /* Append "comments: " before the contents of the body field */
    if (status = CompoundTextAddTextExt( hCompound, dwStyleID, fidBoldBlue,
                         szCommentsStaticText,
                         dwCommentsStaticTextLen,
                         "\n",
                         COMP_PRESERVE_LINES, NULL ))
    {
        /* Unable to add Comments static text */
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_COMMNTSTAT),
                szMessageBoxTitle, MB_ICONHAND | MB_OK );
        NSFNoteClose( hRespNote );
        return ( ERR(status) );
    }

    /* The "comments" field from the response document is a rich text
       field. Call CompoundTextAssimilateItem to assimilate the comments
       field from the response document into the compound text context
       that will become the History field of the main document.

       CompoundTextAssimilateItem() preserves rich text attributes
       such as paragraph styles, fonts, and colors. It does not, however,
       preserve Document Links if they consist of a DocLink reference list
       ($Links item) and a CDLINK2 record. To overcome this limitation,
       call ExportLinks() to convert any CDLINK2-type Doc Links to
       CDLINKEXPORT2 records before calling CompoundTextAssimilateItem().
       If the response note specified by hRespNote contains no Doc Links
       of the CDLINK2 type, then the function ExportLinks() does nothing.
    */

    ExportLinks( hRespNote, COMMENTS_ITEM_NAME );

    if (status = CompoundTextAssimilateItem (
                        hCompound,      /* context to assimilate into */
                        hRespNote,      /* Open note to get item from */
                        COMMENTS_ITEM_NAME, /* name of item: "Body" */
                        0L ))
    {
        /* Unable to assimilate body from response document */
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_ASSIMRESP),
                szMessageBoxTitle, MB_ICONHAND | MB_OK );
    }

    NSFNoteClose( hRespNote );
    return( ERR(status) );
}

/************************************************************************

    FUNCTION:   ReplaceHistory

    PURPOSE:    Delete the History field that was initially in the main
                document, and replace it with the assimilated compound
                text.
    
    COMMENTS:
        First delete the HISTORY item from the main document specified
        by hMainDoc. Then close the compound text context specified by
        hCompound. This hCompound specifies the stand - alone context
        that contains the merged history rich text field.  If this is
        small enough, we get a buffer back from CompoundTextClose. We
        can append this buffer to hMainDoc using NSFItemAppendByBLOCKID.
        Else, if the merged rich text field is greater than about 64K,
        CompoundTextClose will write it to a CD file, and we get back
        the file name.  In this case, create a new item compound text
        context (as opposed to a stand alone context) and assimilate the
        file into the new context. Closing this new context appends the
        new rich text field to the document.

*************************************************************************/

STATUS LNPUBLIC  ReplaceHistory( NOTEHANDLE  hMainDoc,
                                    DHANDLE  hCompound )
{
    DHANDLE       hReturnCD;
    DWORD       dwReturnCDLen;
    char        achReturnCDFileName[MAXPATH];
    BLOCKID     bidItem, bidValue;
    DHANDLE       hNewHistory;

    if (status = NSFItemDelete( hMainDoc,
                               HISTORY_ITEM_NAME,
                               (WORD) HIS_ITEM_NAME_LEN ))
    {
        /* Unable to delete old history from main doc */
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_DELETEHIST),
                szMessageBoxTitle, MB_ICONHAND | MB_OK );
        return( ERR(status) );
    }

    if (status = CompoundTextClose( hCompound,  /* context to close */
                                   &hReturnCD, /* if it fits in memory */
                                   &dwReturnCDLen,
                                   achReturnCDFileName,
                                   MAXPATH-1 ))
    {
        /* Unable to close compound text context.*/
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_CLOSECOMP),
                szMessageBoxTitle, MB_ICONHAND | MB_OK );
        return( ERR(status) );
    }

    if (hReturnCD != NULLHANDLE)      /* the CD fit in one memory segment */
    {
        bidValue.pool = hReturnCD;
        bidValue.block = NULLBLOCK;
        bidItem.pool = 0;
        bidItem.block = NULLBLOCK;
        
        if (status = NSFItemAppendByBLOCKID (
                            hMainDoc, 
                            0, 
                            HISTORY_ITEM_NAME,
                            (WORD) HIS_ITEM_NAME_LEN,
                            bidValue, 
                            dwReturnCDLen, 
                            &bidItem ))
        {
            /* Unable to append new history item to main doc */
            MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_APPENDHIST),
                szMessageBoxTitle, MB_ICONHAND | MB_OK );
            OSMemFree( hReturnCD );
            return( ERR(status) );
        }
       /* If NSFItemAppendByBLOCKID was successfull, do not free the
          hReturnCD buffer because it belongs to the note now.
        */

        return( ERR(status) );
    }

   /*
      Else, assimilated rich text is in the named file. Create an item
      compound text context associated with the Hisory field of the
      open note. Then assimilate the data from the CD file.
    */

    if (status = CompoundTextCreate (hMainDoc,
                                    HISTORY_ITEM_NAME,
                                    &hNewHistory))
    {
        /* Unable to create new history compound text context */
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_NEWCONTXT),
                szMessageBoxTitle, MB_ICONHAND | MB_OK );
        return( ERR(status) );
    }

    if (status = CompoundTextAssimilateFile (
                                hNewHistory,
                                achReturnCDFileName,
                                0L))
    {
        /* Unable to assimilate old history file into CD context */
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_ASSIMFILE),
                szMessageBoxTitle, MB_ICONHAND | MB_OK );
        CompoundTextDiscard( hNewHistory );
        return( ERR(status) );
    }

    if (status = CompoundTextClose( hNewHistory,
                                    NULL, NULL, NULL, 0))
    {
        /* Unable to close New compound text context */
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_CLOSENEW),
                szMessageBoxTitle, MB_ICONHAND | MB_OK );
        CompoundTextDiscard( hNewHistory );
        return( ERR(status) );
    }

    return( ERR(status) );
}

/****************************************************************************

   FUNCTION:  ExportLinks

   PURPOSE:   Convert "weak" (CDLINK2) doc links to "strong"
              (CDLINKEXPORT2) doc links.

   INPUTS:
        NOTEHANDLE hRespNote - handle to open note to process
        char * szBodyFieldName - name of the rich text field to process

   COMMENTS:

    This function searches for a Doc Link Reference List item (an item
    named $Links) in the response note. If it finds one, it processes the
    rich text field by converting any CDLINK2 records it finds to
    CDLINKEXPORT2 records using the information in the $Links item.
    Here's why:
        
    Domino and Notes creates two different types of Doc Links: "weak" 
    and "strong".

    A "weak" Doc Link consist of a CDLINK2 record and a Doc Link Reference
    list.  A "strong" Doc Link consist of a CDLINKEXPORT2 record. I call
    the first type "weak" because it only functions in the context of the
    note that also contains the Doc Link Reference List. I call the second
    type "strong" because it may be copied from one document to another
    and will still work. When a Domino and Notes user creates a Doc Link 
    from the Domino and Notes editor, Notes creates a "weak" Doc Link.  
    This function converts all the CDLINK2 ("weak") Doc Links in the named 
    rich text field to CDLINKEXPORT2 ("strong") Doc Links.

    Since CompoundTextAssimilateItem() only assimilates the CD records
    from the source to the destination, it effectively breaks "weak"
    Doc Links.  Therefore, HISTORY calls this function to convert "weak"
    links to "strong" links before calling CompoundTextAssimilateItem().

    This function searches for a Doc Link Reference List item (an item
    named $Links) in the open document specified by hRespNote. If it finds
    one, it processes the rich text field named by szFieldName. This
    converts any CDLINK2 record it finds in the rich text field to a
    CDLINKEXPORT2 record using the information in the $Links item.

    Internally, Domino and Notes converts CDLINK2 doc links to CDLINKEXPORT2 
    doc links when exporting rich text data to the clipboard.  Domino and 
    Notes performs this conversion because the corresponding Doc Link 
    Reference List ($Links) item is not part of the rich text field and so 
    is not exported to the clipboard. A CDLINKEXPORT2 record contains the
    NOTELINK information that a CDLINK2 structure does not have.
    Therefore, a CDLINKEXPORT2 will function properly after the rich text
    data has been pasted into another document or database where the
    original Doc Link Reference List ($Links) item is not available.

*****************************************************************************/

void LNPUBLIC ExportLinks( NOTEHANDLE hRespNote, char * szBodyFieldName )
{
    BLOCKID     bidLinksItem;
    WORD        wLinksType;
    BLOCKID     bidLinksValue;
    DWORD       dwLinksValueLen;
    BYTE       *pLinksValue;
    LIST        List;
    BLOCKID     bidCurrentBodyItem;
    BLOCKID     bidPreviousBodyItem;
    WORD        wBodyType;
    BLOCKID     bidBodyValue;
    DWORD       dwBodyValueLength;
    STATUS      error = NOERROR;
    BLOCKID     pBodyItemsToDelete[MAX_BODY_ITEMS_TO_DELETE];
    WORD        wBodyItemsToDelete;
    BODYVALUE   pBodyValuesToAppend[MAX_BODY_VALUES_TO_APPEND];
    WORD        wBodyValuesToAppend, i;

   /* First get the $Links item. If there is no $Links item in the
      response document, we conclude there are no CDLINK2 doc links
      either.
    */

    status = NSFItemInfo(hRespNote,
                        ITEM_NAME_LINK,    /* "$Links" */
                        (WORD) lstrlen(ITEM_NAME_LINK),
                        &bidLinksItem,
                        &wLinksType,
                        &bidLinksValue,
                        &dwLinksValueLen );

    if (status == ERR_ITEM_NOT_FOUND)
    {   /* Document does not contain any Doc Links to convert - OK. */
        status = NOERROR;
        return;
    }

    if (status)     /* some other error occurred - Not OK. */
    {
        /* Unable to get Doc Link Reference List information */
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_LINKSINFO),
                szMessageBoxTitle, MB_OK );
        return;
    }

   /* Extract the relevant data from the $Links field. This requires
      locking bidLinksValue. Be sure to unlock it before returning.
      The value of the $Links field consists of a LIST structure
      followed by an array of NOTELINK structures. Get the number of
      NOTELINKs in the array from the ListEntries member of the LIST
      structure.
    */

    pLinksValue = OSLockBlock( BYTE, bidLinksValue );
    ((WORD*)pLinksValue)++;             /* step over data type word*/

    /* pLinksValue now points to a LIST data structure. Get it. */
    List = *((LIST*)pLinksValue);

    if (List.ListEntries == 0)          /* no CDLINK2 Doc Links. OK. */
    {
        OSUnlockBlock( bidLinksValue );
        return;
    }

    ((LIST*)pLinksValue)++;     /* Step over the LIST structure. */

   /* pLinksValue now points to the first NOTELINK structure in the
      array of NOTELINK structures.
    */

   /* Process the Body field (a rich text field). Most rich text fields
      larger than about 40K consist of multiple TYPE_COMPOSITE items,
      where all the items have the same field name. Loop over all the
      Body items in the note by calling NSFItemInfo and NSFItemInfoNext.
      Process each item by calling the function ProcessOneBodyItem().
      ProcessOneBodyItem() makes a copy of the item, puts the original
      in the array pBodyItemsToDelete[], and puts the new item in the
      array pBodyValuesToAppend[]. Repeat this loop until all the body
      items in the note have been processed.
    */

    wBodyItemsToDelete = 0;
    wBodyValuesToAppend = 0;

    status = NSFItemInfo(   hRespNote,
                            szBodyFieldName,
                            (WORD) lstrlen(szBodyFieldName),
                            &bidCurrentBodyItem,
                            &wBodyType,
                            &bidBodyValue,
                            &dwBodyValueLength );
    if (status)
    {
        /* unable to get body of response document */
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_BODYINFO),
                szMessageBoxTitle, MB_ICONHAND | MB_OK );
        OSUnlockBlock( bidLinksValue );
        return;
    }

   /* Call ProcessOneBodyItem() to process the first rich text item from
      the body of the response document.
    */
    if (error = ProcessOneBodyItem( hRespNote,
                                    (NOTELINK *)pLinksValue,
                                    List.ListEntries,
                                    bidCurrentBodyItem,
                                    bidBodyValue,
                                    dwBodyValueLength,
                                    pBodyItemsToDelete,
                                    &wBodyItemsToDelete,
                                    pBodyValuesToAppend,
                                    &wBodyValuesToAppend ))
    {
        OSUnlockBlock( bidLinksValue );
        return;
    }

   /* Loop over all remaining body items in this rich text field. */

    while( TRUE )
    {
        bidPreviousBodyItem = bidCurrentBodyItem;
        status = NSFItemInfoNext(   hRespNote,
                                    bidPreviousBodyItem,
                                    szBodyFieldName,
                                    (WORD) lstrlen(szBodyFieldName),
                                    &bidCurrentBodyItem,
                                    &wBodyType,
                                    &bidBodyValue,
                                    &dwBodyValueLength );

        if (status == ERR_ITEM_NOT_FOUND)
        {                               /* no more Body items to process */
            status = NOERROR;           /* we expect this - not an error */
            break;                      /* break loop over body items */
        }

        if (status)     /* some other error occurred - Not OK. */
        {
            /* unable to get body of response document */
            MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_BODYINFO),
                szMessageBoxTitle, MB_ICONHAND | MB_OK );
            break;
        }

        if (error = ProcessOneBodyItem( hRespNote,
                                    (NOTELINK *)pLinksValue,
                                    List.ListEntries,
                                    bidCurrentBodyItem,
                                    bidBodyValue,
                                    dwBodyValueLength,
                                    pBodyItemsToDelete,
                                    &wBodyItemsToDelete,
                                    pBodyValuesToAppend,
                                    &wBodyValuesToAppend ))
        {
            break;
        }
    }   /* end of loop over body items. */

    OSUnlockBlock( bidLinksValue );

    if (status) /* free all allocated body value blocks before returning. */
    {
        for (i = 0; i < wBodyValuesToAppend; i++)
        {
            OSMemFree( pBodyValuesToAppend[i].bid.pool );
        }
        return;
    }

    /* Delete all the body items identified in pBodyItemsToDelete[] */

    for (i = 0; i < wBodyItemsToDelete; i++)
    {
        if (status = NSFItemDeleteByBLOCKID( hRespNote,
                            pBodyItemsToDelete[i] ))
        {
            /* Unable to delete old body from response document*/
            MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_DELOLDBODY),
                    szMessageBoxTitle, MB_OK );
            break;
        }
    }

    if (status)
    {
        for (i = 0; i < wBodyValuesToAppend; i++)
        {
            OSMemFree( pBodyValuesToAppend[i].bid.pool );
        }
        return;
    }

    /* Append all new body items identified in pBodyValuesToAppend[] */

    for (i = 0; i < wBodyValuesToAppend; i++)
    {
        if (status = NSFItemAppendByBLOCKID( hRespNote, 0,
                            szBodyFieldName,
                            (WORD) lstrlen(szBodyFieldName),
                            pBodyValuesToAppend[i].bid,
                            pBodyValuesToAppend[i].dwLen,
                            NULL ))
        {
            /* Unable to append new body item to response note */
            MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_APPENDBODY),
                    szMessageBoxTitle, MB_OK );
            break;
        }
    }

    if (status)
    {
        for ( ; i < wBodyValuesToAppend; i++ )
        {
            OSMemFree( pBodyValuesToAppend[i].bid.pool );
        }
    }

    return;
}


/****************************************************************************

   FUNCTION:  ProcessOneBodyItem

   PURPOSE:   Process one Body item by creating a new body item and
              copying each CD record from the original item to the new
              item, converting all CDLINK2 document links to CDLINKEXPORT2
              document links.

   INPUTS:
        NOTEHANDLE  hRespNote - handle to open note to process
        NOTELINK   *pNoteLinks - array of NOTELINK structures
        WORD        wNumNoteLinks - number of NOTELINKs in array
        BLOCKID     bidBodyItem - item blockid of item to process
        BLOCKID     bidBodyValue - value blockid of item to process
        DWORD       dwBodyValueLength - length of the item value
        BLOCKID    *pBodyItemsToDelete - array of BLOCKIDs of items to delete
        WORD       *pwBodyItemsToDelete - number of BLOCKIDs in delete array
        BODYVALUE  *pBodyValuesToAppend - array of BODYVALUEs to add
        WORD       *pwBodyValuesToAppend - number of BODYVALUEs in add array

   COMMENTS:

    This function creates a copy of the specified body item. It copies
    every CD record from the original body item to the new copy, but
    converts any CDLINK2 doc links in the original to CDLINKEXPORT2 doc
    links in the copy. This adds the item block ID of the original to
    the array pBodyItemsToDelete[] and adds the block ID of the new body
    item to the array pBodyItemsToAppend[].

    This calls EnumCompositeBuffer to process every CD record in the
    specified body item. EnumCompositeBuffer calls the action routine,
    ProcessOneCDRecord, on every CD record in the item.

*****************************************************************************/

STATUS  LNPUBLIC  ProcessOneBodyItem(
                        NOTEHANDLE  hRespNote,
                        NOTELINK   *pNoteLinks,
                        WORD        wNumNoteLinks,
                        BLOCKID     bidBodyItem,
                        BLOCKID     bidBodyValue,
                        DWORD       dwBodyValueLength,
                        BLOCKID    *pBodyItemsToDelete,
                        WORD       *pwBodyItemsToDelete,
                        BODYVALUE  *pBodyValuesToAppend,
                        WORD       *pwBodyValuesToAppend )
{
    CONVERTCONTEXT      Context;
    FARPROC             lpProc;         /* ProcessOneCDRecord function address */
    DWORD               dwStructDiff;   /* difference between CDLINK2 and EXPORT2 */
    DWORD               dwGrowthFactor; /* rich text grows by appx this much */
    DWORD               dwNewBodyLen;
    DHANDLE             hNewBody, hNewBody2;
    BODYVALUE           bvalNewBody, bvalNewBody2;
    ActionRoutinePtr    actionroutine_ptr;

   /* Initialize the CONVERTCONTEXT data structure in preparation for
      calling EnumCompositeBuffer, below. Initialize pNoteLinks with
      the pointer to the first NOTELINK structure in the array.
      Initialize wNumNoteLinks with the number of links. Initalize
      fUseBody2 to FALSE. ProcessOneCDRecord() sets fUseBody2 to TRUE
      if it overflows Body1 and starts using Body2.
    */
    Context.pNoteLinks = pNoteLinks ;
    Context.wNumNoteLinks = wNumNoteLinks ;
    Context.fUseBody2 = FALSE;

   /* Allocate a block of memory to hold the new rich text item. This
      block of memory must be larger than the size of the original item
      because CDLINKEXPORT2 structures are bigger than CDLINK2 structures.
      If the original body item plus the additional size of the
      CDLINKEXPORT2 structures would exceed about 40K, then allocate an
      additional new body item to hold the overflow.
    */

    dwStructDiff = ODSLength(_CDLINKEXPORT2) - ODSLength(_CDLINK2) ;
    dwStructDiff += (dwStructDiff % 2); /* round up to whole words */
    dwGrowthFactor = dwStructDiff * wNumNoteLinks;
    dwNewBodyLen = dwBodyValueLength + dwGrowthFactor;

    if (dwNewBodyLen <= OVERFLOWSIZE)
    {
        Context.dwNewBodyLen = dwNewBodyLen;
        Context.dwNewBody2Len = 0;
    }
    else
    {
        Context.dwNewBodyLen = MAXONESEGSIZE;   /* appx. 64K */
        Context.dwNewBody2Len = OVERFLOWSIZE;   /* appx. 40K */
    }

    if (status = OSMemAlloc( 0, Context.dwNewBodyLen, &hNewBody ))
    {
        /* Unable to allocate memory for new body item.*/
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_MALLOC),
                szMessageBoxTitle, MB_ICONHAND | MB_OK );
        return( ERR_HISTORY_MALLOC );
    }

    if (Context.dwNewBody2Len != 0)
    {
        if (status = OSMemAlloc( 0, Context.dwNewBody2Len, &hNewBody2 ))
        {
            MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_MALLOC),
                szMessageBoxTitle, MB_ICONHAND | MB_OK );
            OSMemFree( hNewBody );
            return( ERR_HISTORY_MALLOC );
        }
    }

   /* Lock the block of memory that will contain the new body. This
      yields a pointer. Initialize both pNewBody and pNewBodyCurrPos
      members of the Context structure to this pointer. pNewBody will
      always point to the top of this buffer. ProcessOneCDRecord() will
      advance pNewBodyCurrPos as it appends CD records to the buffer.
      Write the data type word (TYPE_COMPOSITE) into the first word
      of the new body buffer and advance pNewBodyCurrPos by 1 word.
      Do the same for NewBody2, if necessary.
    */

    Context.pNewBody =
        Context.pNewBodyCurrPos = OSLock( BYTE, hNewBody );

    *((WORD*)(Context.pNewBodyCurrPos)) = TYPE_COMPOSITE;
    ((WORD*)(Context.pNewBodyCurrPos))++;

    if (Context.dwNewBody2Len != 0)
    {
        Context.pNewBody2 =
            Context.pNewBody2CurrPos = OSLock( BYTE, hNewBody2 );
        *((WORD*)(Context.pNewBody2CurrPos)) = TYPE_COMPOSITE;
        ((WORD*)(Context.pNewBody2CurrPos))++;
    }

    lpProc = MakeProcInstance( (FARPROC) ProcessOneCDRecord, hInst );

    actionroutine_ptr = (ActionRoutinePtr)lpProc;

    status = EnumCompositeBuffer(
                            bidBodyValue,
                            dwBodyValueLength,
                            actionroutine_ptr,
                            &Context );

    FreeProcInstance(lpProc);

    if (status)
    {
        /* Unable to process body of response document */
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_ENUMBODY),
                    szMessageBoxTitle, MB_ICONHAND | MB_OK );
        OSUnlock( hNewBody );
        OSMemFree( hNewBody );
        if (Context.dwNewBody2Len != 0)
        {
            OSUnlock( hNewBody2 );
            OSMemFree( hNewBody2 );
        }
        return( ERR_HISTORY_ENUMBODY );
    }

   /* Else, successfully copied all CD records from the original body
      item to the new body item, possibly overflowing into a second
      new body item. Add the item block id of the original body to the
      array pBodyItemsToDelete. Add a BODYVALUE structure containing the
      new body value and length to the array pBodyValuesToAppend.
    */

    if (*pwBodyItemsToDelete < MAX_BODY_ITEMS_TO_DELETE)
    {
        pBodyItemsToDelete[(*pwBodyItemsToDelete)++] = bidBodyItem;
    }
    else
    {
        /* Response document contains too many Body items */
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_TOOMANYRESBDY),
                szMessageBoxTitle, MB_OK );
        OSUnlock( hNewBody );
        OSMemFree( hNewBody );
        if (Context.dwNewBody2Len != 0)
        {
            OSUnlock( hNewBody2 );
            OSMemFree( hNewBody2 );
        }
        return( ERR_HISTORY_TOOMANYRESBDY );
    }

    if (*pwBodyValuesToAppend < MAX_BODY_VALUES_TO_APPEND)
    {
        bvalNewBody.bid.pool = hNewBody;
        bvalNewBody.bid.block = NULLBLOCK;
        bvalNewBody.dwLen = (Context.pNewBodyCurrPos - Context.pNewBody) ;
        pBodyValuesToAppend[(*pwBodyValuesToAppend)++] = bvalNewBody;
    }
    else
    {
        /* New Body of response document is too large */
        MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_NEWRESBDY),
                szMessageBoxTitle, MB_OK );
        OSUnlock( hNewBody );
        OSMemFree( hNewBody );
        if (Context.dwNewBody2Len != 0)
        {
            OSUnlock( hNewBody2 );
            OSMemFree( hNewBody2 );
        }
        return( ERR_HISTORY_NEWRESBDY );
    }
    if (Context.fUseBody2)
    {
        if (*pwBodyValuesToAppend < MAX_BODY_VALUES_TO_APPEND)
        {
            bvalNewBody2.bid.pool = hNewBody2;
            bvalNewBody2.bid.block = NULLBLOCK;
            bvalNewBody2.dwLen = (Context.pNewBody2CurrPos -
                                    Context.pNewBody2) ;
            pBodyValuesToAppend[(*pwBodyValuesToAppend)++] = bvalNewBody2;
        }
        else
        {
            /* New Body of response document is too large */
            MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_NEWRESBDY),
                szMessageBoxTitle, MB_OK );
            OSUnlock( hNewBody );
            OSMemFree( hNewBody );
            if (Context.dwNewBody2Len != 0)
            {
                OSUnlock( hNewBody2 );
                OSMemFree( hNewBody2 );
            }
            return( ERR_HISTORY_NEWRESBDY );
        }
    }

    OSUnlock( hNewBody );
    if (Context.dwNewBody2Len != 0)
    {
        OSUnlock( hNewBody2 );
    }

    return NOERROR;
}


/************************************************************************

    FUNCTION:   ProcessOneCDRecord

    PURPOSE:    Process one Compound Text (CD) Record from the Body
                of the response document. If the record is of type
                CDLINK2, convert it to CDLINKEXPORT2.

    COMMENTS:

        This is the action routine for EnumCompositeBuffer() called
        in ExportLinks(). EnumCompositeBuffer calls ProcessOneCDRecord
        once on every CD record. This copies each CD record -- except
        CDLINK2 records -- into the "new" body field. This converts any
        CDLINK2 records that it finds to records of type CDLINKEXPORT2.
        This appends the CDLINKEXPORT2 record to the "new" body field in
        place of the CDLINK2 record in the original item.

        A CDLINK2 Doc Link has two important parts: a CDLINK2 record
        which resides in the rich text field, and a DocLink Reference
        List. The DocLink Reference List resides in the $Links item.

        The CDLINK2 record in the rich text field consists of a CDLINK2
        structure followed by a display comment. The CDLINK2 structure
        contains a header and an index. This index identifies the
        corresponding NOTELINK entry in the DocLink Reference List. The
        CDLINK2 record requires the Doc Link Reference List (the $Links
        item) in order to resolve the link.

    INPUTS:      
         char      *RecordPtr       -   pointer to start of data
         WORD       RecordType      -   CD record type (SIG_CD_*)
         DWORD      RecordLength    -   length of CD record
         void far * pvoidContext    -   pointer to CONVERTCONTEXT

    The CONVERTCONTEXT structure contains all the data needed to convert
    the CD record specified by RecordPtr from a CDLINK2 to a CDLINKEXPORT2
    and append it to a "new body item" buffer. The CONVERTCONTEXT structure
    contains the following members:

    NOTELINK   *pNoteLinks;              first NOTELINK in an array of
                                         NOTELINKS from $Links
    WORD        wNumNoteLinks;           number of NOTELINKS in array
    BOOL        fUseBody2;               TRUE means overflow into Body2
    WORD        wNumLinksExported;       number of links processed
    BYTE       *pNewBody;                top of buffer holding new body
    BYTE       *pNewBodyCurrPos;         where to put next CD record
    DWORD       dwNewBodyLen;            allocated length of new body
    BYTE       *pNewBody2;               top of buffer holding overflow
    BYTE       *pNewBody2CurrPos;        where to put next overflow CD
    DWORD       dwNewBody2Len;           max length of overflow body

*************************************************************************/

STATUS  LNCALLBACK  ProcessOneCDRecord(
                                    char   *RecordPtr,
                                    WORD    RecordType, 
                                    DWORD   RecordLength, 
                                    void   *pvoidContext )
{
    CONVERTCONTEXT   *pContext;
    BYTE             *pDest;
    BYTE             *pSource;
    DWORD             dwCount;
    WORD              wDisplayCommentLen;
    CDLINK2          *pCDLINK2;
    BYTE             *pCDLINK2DispComm;
    WORD              wLinkID;
    CDLINKEXPORT2     pCDLExport2;
    BYTE             *pCurrCDLExport2;
    DHANDLE           hCDLRecord;

    pContext = (CONVERTCONTEXT *)pvoidContext;
    pDest = pContext->pNewBodyCurrPos;  /* assume no overflow yet */

    if (RecordType != SIG_CD_LINK2) /* record is not a CDLINK2 */
    {
        dwCount = RecordLength;
        pSource = RecordPtr;
    }
    else
    {  /*
          Create a record that consists of a CDLINKEXPORT2 followed
          by the same display comment found in the CDLINK2 record.
          Allocate a block of memory that will contain a CDLINKEXPORT2
          structure followed by the display comment.
        */
        wDisplayCommentLen = (WORD)RecordLength - ODSLength(_CDLINK2);
        dwCount = wDisplayCommentLen + ODSLength(_CDLINKEXPORT2);
        if (status = OSMemAlloc( 0, dwCount, &hCDLRecord ))
        {
            /* Unable to allocate memory for Doc Link record */
            MessageBox( GetFocus(), BuildErrStr(ERR_HISTORY_CDMALLOC),
                szMessageBoxTitle, MB_ICONHAND | MB_OK );
            return(ERR(status));
        }


        pCDLINK2 = (CDLINK2*)RecordPtr;
        wLinkID = pCDLINK2->LinkID;
        pCDLINK2DispComm = RecordPtr+ODSLength(_CDLINK2);

        memset(&pCDLExport2,'\0',ODSLength(_CDLINKEXPORT2));
        pCDLExport2.Header.Signature = SIG_CD_LINKEXPORT2;
        pCDLExport2.Header.Length = (WORD)dwCount;
        pCDLExport2.NoteLink = (pContext->pNoteLinks)[wLinkID];

        pSource = OSLock( BYTE, hCDLRecord );
        pCurrCDLExport2=pSource;
        ODSWriteMemory(&pCurrCDLExport2,_CDLINKEXPORT2,&pCDLExport2,1);
        memcpy( pCurrCDLExport2,pCDLINK2DispComm,wDisplayCommentLen);

    }

   /* pSource, pDest, and dwCount are ready for the memcpy. But first,
      check that a record of size dwCount bytes will fit in the first
      body (starting at pNewBodyCurrPos) without overflowing.
    */

    if (pContext->fUseBody2 ==  FALSE)  /* we did not previously overflow */
    {
        if ( (DWORD)((pDest+dwCount) - pContext->pNewBody) >
                            (pContext->dwNewBodyLen) ) /* overflow! */
        {
            pContext->fUseBody2 = TRUE;
            pDest = pContext->pNewBody2CurrPos;
        }
    }
    else /* we previously overflowed NewBody. Continue with NewBody2.*/
    {
        pDest = pContext->pNewBody2CurrPos;
    }

    memcpy( pDest, pSource, (size_t)dwCount );

    if (RecordType == SIG_CD_LINK2)
    {
        OSUnlockObject( hCDLRecord );
        OSMemFree( hCDLRecord );
    }

    /* now go back and update the "CurrPos" pointers in the Context */
    if (pContext->fUseBody2 == FALSE)
    {
        pContext->pNewBodyCurrPos += dwCount;
        if (dwCount % 2)
        {
            *(pContext->pNewBodyCurrPos) = 0;
            pContext->pNewBodyCurrPos++;
        }
    }
    else
    {
        pContext->pNewBody2CurrPos += dwCount;
        if (dwCount % 2)
        {
            *(pContext->pNewBody2CurrPos) = 0;
            pContext->pNewBody2CurrPos++;
        }
    }

    return NOERROR;
}
