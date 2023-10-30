/*************************************************************************
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

    PROGRAM:    INTROWIN

    PURPOSE:    Windows version of HCL C API for Notes/Domino sample 
                introductory program that opens a Domino database and
                displays the database title.


    FUNCTIONS:

    WinMain() -           Call initialization function, process
                          message loop.
    InitApplication() -   Init window data and register window.
    InitInstance() -      Save instance handle, create main window.
    MainWndProc() -       Process messages.
    GetDBName() -         Process messages for "GetDBName" dialog box.

    COMMENTS:

        Windows can have several copies of your application running at the
        same time.  The variable hInst keeps track of which instance this
        application is so that processing will be to the correct window.

************************************************************************/

#include <windows.h>            /* required for all Windows apps */

/* Standard Library Include Files */

#include <stdlib.h>
#include <string.h>

/* DOMINO AND NOTES INCLUDE FILES */

#include <global.h>
#include <osmisc.h>
#include <nsfdb.h>

/* LOCAL INCLUDE FILES */

#include "introwin.h"

extern int      __argc;
extern char **  __argv;

/*  Local Defines */

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

#define LINEOTEXT   256

//LRESULT FAR PASCAL MainWndProc (HWND hWnd, UINT message,
//                                WPARAM wParam, LPARAM lParam);
/* static values */

static HANDLE     hInst, hModule;             /* current instance,
                                                module handle.    */
static DBHANDLE    hNotesDB = NULLHANDLE;
static char    szDBFileNameString[LINEOTEXT]; /* String to hold filename
                                                     of database.  */
static char    szErrorString[LINEOTEXT];      /* String to be used for
                                                     error messages. */
static char    szBuffer[LINEOTEXT];           /* Info buffer. */

static char    szDBTitleString[NSF_INFO_SIZE];    /* String to hold title
                                                     of database.   */
/*************************************************************************

    FUNCTION: WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
    
    PURPOSE: calls initialization function, processes message loop

    COMMENTS:

    Windows recognizes this function by name as the initial entry point 
    for the program.  This function calls the application initialization
    routine, if no other instance of the program is running, and always 
    calls the instance initialization routine.  It then executes a message
    retrieval and dispatch loop that is the top-level control structure
    for the remainder of execution.  The loop is terminated when a WM_QUIT
    message is received, at which time this function exits the application 
    instance by returning the value passed by PostQuitMessage().

    If this function must abort before entering the message loop, it 
    returns the conventional value NULL.  

*************************************************************************/

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, int nCmdShow)

{
    MSG     msg;         /* message.         */
    STATUS  nError;      /* Status of HCL C API for Notes/Domino call. */

    hModule = GetModuleHandle((LPSTR) "introwin");
    hInst = hInstance;

    /* Init the Domino and Notes runtime system. */
    nError = NotesInitExtended(__argc, __argv);

    /*
     *  If the call to NotesInit fails, display an error message
     *  and exit the program.
     */

    if (nError != NOERROR)
    {
        MessageBox (GetFocus(), "Unable to Initialize Notes!",
                    "introwin", MB_OK);
        return (FALSE);
    }

    if (!hPrevInstance)              /* Other instances of app running? */
    if (!InitApplication(hInstance)) /* Initialize shared things */
        return (FALSE);              /* Exits if unable to initialize  */

    /* Perform initializations that apply to a specific instance */

    if (!InitInstance(hInstance, nCmdShow))
        return (FALSE);

    /* Acquire and dispatch messages until WM_QUIT message is received. */

    while (GetMessage(&msg,     /* message structure                  */
                      (HWND) NULL,          /* handle of window receiving the msg */
                      (WORD) NULL,          /* lowest message to examine          */
                      (WORD) NULL))         /* highest message to examine         */
    {
        TranslateMessage(&msg);     /* Translates virtual key codes.   */
        DispatchMessage(&msg);      /* Dispatches message to window.   */
    }

    NotesTerm();                /* Terminate the Domino and Notes runtime system. */


    return (SIZET_TO_DWORD(msg.wParam));    /* Returns the value from PostQuitMessage */
}


/************************************************************************

    FUNCTION: InitApplication(DHANDLE)

    PURPOSE: Initializes window data and registers window class

    COMMENTS:

    This function is called at initialization time only if no other 
    instances of the application are running.  This function performs 
    initialization tasks that can be done once for any number of running 
    instances.  

    In this case, we initialize a window class by filling out a data 
    structure of type WNDCLASS and calling the Windows RegisterClass() 
    function.  Since all instances of this application use the same window 
    class, we only need to do this when the first instance is initialized.  


*************************************************************************/

BOOL InitApplication(hInstance)
HANDLE hInstance;               /* current instance         */
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
    wc.hIcon = LoadIcon((DHANDLE) NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor((DHANDLE) NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH); 
    wc.lpszMenuName =  "introwinMenu";   /* Menu name in .RC file. */
    wc.lpszClassName = "introwinClass"; /* Name used in CreateWindow call. */

    /* Register the window class and return success/failure code. */

    return (RegisterClass(&wc));

}


/*************************************************************************

    FUNCTION:  InitInstance(DHANDLE, int)

    PURPOSE:  Saves instance handle and creates main window

    COMMENTS:

        This function is called at initialization time for every instance
        of this application.  This function performs initialization tasks
        that cannot be shared by multiple instances.  

        In this case, we save the instance handle in a static variable and
        create and display the main program window.  
        
*************************************************************************/

BOOL InitInstance(hInstance, nCmdShow)
    HANDLE  hInstance;      /* Current instance identifier.       */
    int     nCmdShow;       /* Param for first ShowWindow() call. */
{
    HWND    hWnd;           /* Main window handle.                */

    /* Save the instance handle in static variable, which will be       */
    /* used in many subsequence calls from this application to Windows. */

    hInst = hInstance;

    /* Create a main window for this application instance.  */

    hWnd = CreateWindow(
                        "introwinClass",                /* See RegisterClass() call.    */
                        "Intro Notes API program",      /* Text for window title bar.   */
                        WS_OVERLAPPEDWINDOW,            /* Window style.                */
                        CW_USEDEFAULT,                  /* Default horizontal position. */
                        CW_USEDEFAULT,                  /* Default vertical position.   */
                        CW_USEDEFAULT,                  /* Default width.               */
                        CW_USEDEFAULT,                  /* Default height.              */
                        (HWND) NULL,                    /* No parent.                   */
                        (HMENU) NULL,                   /* Use the window class menu.   */
                        hInstance,                      /* Owner of window.             */
                        (LPVOID) NULL                     /* Pointer not needed.          */
    );

    /* If window could not be created, return "failure" */

    if (!hWnd)
        return (FALSE);

    /*
     * Make the window visible; update its client area;
     *   Return "success"
     */

    ShowWindow(hWnd, nCmdShow);  /* Show the window          */
    UpdateWindow(hWnd);          /* Sends WM_PAINT message   */
    return (TRUE);               /* Returns value from PostQuitMessage */

}

/*************************************************************************

    FUNCTION: MainWndProc(HWND, UINT, WPARAM, LPARAM)

    PURPOSE:  Processes messages

    MESSAGES:

    WM_COMMAND    - application menu (GetDBName dialog box)
    WM_DESTROY    - destroy window

    COMMENTS:

    To process the FILE_GET_DB_TITLE message, call MakeProcInstance()
    to get the current instance address of the GetDBName() function.  Then
    call Dialog box which will create the box according to the information
    in the introwin.rc file and turn control over to the GetDBName()
    function.  When it returns, free the instance address.

*************************************************************************/

long FAR PASCAL MainWndProc(HWND hWnd, UINT message,
                            WPARAM wParam, LPARAM lParam)

{
    FARPROC lpProcGetDBName;     /* pointer to the "GetDBName" function */
    STATUS  nError = NOERROR;
    int     iError;                     /* Ret code from DialogBox call */
    char    szTempString[LINEOTEXT];    /* Temp string used in building
                                           the database title string.  */
    switch (message)
    { 
        case WM_COMMAND:       /* message: command from app.  menu */
            switch (wParam)
            { 
                case FILE_GET_DB_TITLE:

                    lpProcGetDBName = MakeProcInstance(GetDBName, hInst);

                    /*
                     *  The GetDBName dialog proc prompts the user for the
                     *  filename of the database.  The procedure stores the
                     *  filename in the character string szDBFileNameString.
                     */

                    iError = DialogBox(hInst,             /* current instance */
                                       "GetDBNameDlg",    /* resource to use  */
                                       hWnd,              /* parent handle    */
                                       (DLGPROC)lpProcGetDBName);  /* instance address */
                               
                    FreeProcInstance(lpProcGetDBName);
            
                    if (iError != TRUE)          /* If user pressed CANCEL, */
                        goto RETURN;             /* don't open database. */

                    /*
                     *   If no filename was entered, display an
                     *   appropriate error message.
                     */
                 
                    if (szDBFileNameString[0] == (char) NULL)
                    {
                        MessageBox (GetFocus(),
                                    "No Filename entered",
                                    "ERROR", MB_OK);
                        goto RETURN;
                    }

                    /*
                     * Open the specified database.  Display
                     * error message if the open fails.
                     */
                 
                    nError = NSFDbOpen (szDBFileNameString,
                                &hNotesDB);     /* Open database */
                    if (nError != NOERROR)
                    { 
                        OSLoadString(hModule, ERR(nError),
                                     szErrorString, LINEOTEXT-1);
                        MessageBox (GetFocus(), szErrorString,
                            "Notes Error", MB_OK);
                        goto RETURN;
                    } 

                    /*
                     *  Get the database title.  If the API call
                     *  fails,  display an appropriate message
                     *  and close the database recently opened.
                     *
                     *  If the API call succeeds, display
                     *  the title of the database.
                     */
             
                    nError = NSFDbInfoGet (hNotesDB, szBuffer);
                    if (nError != NOERROR)    
                    {
                        OSLoadString(hModule, ERR(nError),
                                     szErrorString,LINEOTEXT-1);
                        MessageBox (GetFocus(), szErrorString,
                                    "Notes Error", MB_OK);
                        NSFDbClose (hNotesDB);
                        goto RETURN;
                    } 



                    NSFDbInfoParse (szBuffer,
                                    INFOPARSE_TITLE,
                                    szDBTitleString,
                                    NSF_INFO_SIZE - 1);



                    wsprintf(szTempString,
                             "Database title: %s",
                             (char far *) szDBTitleString);
                    MessageBox (GetFocus(), szTempString, 
                                "Database Title", MB_OK);

                    /* Clear strings for reuse. */

                    szDBFileNameString[0] = ((char) NULL);
                    szDBFileNameString[0] = ((char) NULL);
                    szTempString[0] = ((char) NULL);

                    /*
                     * Close the database.  Display error message if the
                     * close operation fails.
                     */
             
                    if (nError = NSFDbClose (hNotesDB))
                    {
                        OSLoadString(hModule, ERR(nError),
                                     szDBFileNameString,
                                     LINEOTEXT-1);
                        MessageBox (GetFocus(),
                                    szDBFileNameString,
                                    "Notes Error",
                                    MB_OK);
                    }
RETURN:             hNotesDB = NULLHANDLE;
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
            }

            break;

        case WM_DESTROY:            /* message: window being destroyed */
            PostQuitMessage(0);
            break;

        default:                    /* Passes it on if unproccessed    */
            return (DefWindowProc(hWnd, message, wParam, lParam));
    } 
    return 0;
} 

/************************************************************************

    FUNCTION: GetDBName(HWND, WORD, WPARAM, LONG)

    PURPOSE:  Processes messages for "GetDBName" dialog box

    MESSAGES:

    WM_INITDIALOG - initialize dialog box
    WM_COMMAND    - Input received

    COMMENTS:

          No initialization is needed for this particular dialog box,
          but TRUE must be returned to Windows.

          Wait for user to click on the "OK" button or the "CANCEL"
          button, then close the dialog box.

***********************************************************************/

BOOL FAR PASCAL GetDBName(HWND hDlg, WORD message,
              WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:         /* message: initialize dialog box */
            return (TRUE);

        case WM_COMMAND:            /* message: received a command */
            switch (wParam)
            {
                case IDOK:          /* "OK" box selected?         */
                    GetDlgItemText(hDlg,
                                   DBNAME_EDIT,
                                   szDBFileNameString,
                                   LINEOTEXT-1);
                        
                    EndDialog(hDlg, TRUE);  /* Exits the dialog box. */
                    return (TRUE);
                        
                case IDCANCEL:      /* System menu close command? */
                    EndDialog(hDlg, FALSE); /* Exits the dialog box.   */
                    return (TRUE);
            }
        break;
    }
    return (FALSE);                 /* Didn't process a message.  */
}
