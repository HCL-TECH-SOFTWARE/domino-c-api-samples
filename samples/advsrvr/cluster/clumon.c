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

    PROGRAM:    CLUMON.EXE
    
    FILE:       CLUMON.C

    PURPOSE:    Advanced Server Cluster C API Sample program for Windows that
                illustrates a "Cluster Monitor" application for managing Cluster
                server information and their databases.  
                        
************************************************************************/

/* Standard Library Include Files */
#include <windows.h>
#include <stdlib.h>
#include <string.h>

/* HCL C API for Notes/Domino INCLUDE FILES */
#include <global.h>
#include <nsfdb.h>
#include <ns.h>
#include <osmem.h>
#include <osmisc.h>
#include <osfile.h>
#include <dname.h>
#include <textlist.h>
#include <srverr.h>

/* LOCAL INCLUDE FILES */
#include "clumon.h"
#include "clfunc.h"
#include "resource.h"

/*  Additions for V4 */
extern int      __argc;
extern char **  __argv;

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/* Function prototypes */
/* Windows routines */
int PASCAL          WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
LRESULT FAR PASCAL  MainWndProc(HWND, UINT, WPARAM, LPARAM);
BOOL                InitApplication(DHANDLE);
BOOL                InitInstance(DHANDLE, int);

/* Dialog box message handler routines */
BOOL FAR PASCAL GetClusterInfo(HWND, WORD, WPARAM, LPARAM);
BOOL FAR PASCAL GetDBOptions(HWND, WORD, WPARAM, LPARAM);
BOOL FAR PASCAL ManageDBOptions(HWND, WORD, WPARAM, LPARAM);
BOOL FAR PASCAL RestrictServer(HWND, WORD, WPARAM, LPARAM);
BOOL FAR PASCAL SetThreshold(HWND, WORD, WPARAM, LPARAM);
BOOL FAR PASCAL CreateCopyDB(HWND, WORD, WPARAM, LPARAM);

/* Dialog box control service routines */
void dspDBOptions(HWND, char *, char *);
void updateDBOptions(HWND, char *, char *);
void dspClusterInfo(HWND, char *, DWORD);
void dspServerRestrict(HWND, char *);
void setServerRestrict(HWND, char *);
void shutdownServer(HWND, char *);
void dspThresholdInfo(HWND, char *);
void setThresholdInfo(HWND, char *);
void createcopyDB(HWND, char *, char *, char *, char *, BOOL);

/* static values */
static DHANDLE     hInst;                       /* current instance */
static DHANDLE     hModule;                     /* module handle */
static HWND        hWnd;                        /* Main window handle */

static char  szDBServerNameString[LINEOTEXT]; /* String to hold Name of Server */
static char  szDBServerNamePrev[LINEOTEXT];   /* "Previous" Server String */
static char  szDBNameString[LINEOTEXT];       /* String to hold Name of Database */
static char  szDBNamePrev[LINEOTEXT];         /* "Previous" Database String */
static char  szWindowTitle[LINEOTEXT];        /* Window Title */

/*************************************************************************

    FUNCTION: WinMain(DHANDLE, DHANDLE, LPSTR, int)
    
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

    hModule = GetModuleHandle((LPSTR) "clumon");

    /* Init the Domino and Notes runtime system. */
    nError = NotesInitExtended(__argc, __argv);    /* V4 */

   /*
    *  If the call to NotesInit fails, display an error message
    *  and exit the program.
    */
    if (nError != NOERROR)
    {
        MessageBox (GetFocus(), "Unable to Initialize Notes!",
                    "Error!", MB_OK);
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

    return (msg.wParam);    /* Returns the value from PostQuitMessage */
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

BOOL InitApplication(DHANDLE hInstance)
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
    wc.hIcon   = LoadIcon (hInstance, MAKEINTRESOURCE(110));
    wc.hCursor = LoadCursor((DHANDLE) NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH); 
    wc.lpszMenuName =  "clumonMenu";    /* Menu name in .RC file. */
    wc.lpszClassName = "clumonClass";   /* Name used in CreateWindow call. */

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

BOOL InitInstance(DHANDLE hInstance, int nCmdShow)
{

    /* Save the instance handle in static variable, which will be       */
    /* used in many subsequence calls from this application to Windows. */
    hInst = hInstance;

    /* Create a main window for this application instance.  */
    hWnd = CreateWindow(
                        "clumonClass",                  /* See RegisterClass() call.    */
                        CLUMON_TITLE,					/* Text for window title bar.   */
                        WS_OVERLAPPEDWINDOW,            /* Window style.                */
                        CW_USEDEFAULT,                  /* Default horizontal position. */
                        CW_USEDEFAULT,                  /* Default vertical position.   */
                        CW_USEDEFAULT,                  /* Default width.               */
                        CW_USEDEFAULT,                  /* Default height.              */
                        (HWND) NULL,                    /* No parent.                   */
                        (HMENU) NULL,                   /* Use the window class menu.   */
                        hInstance,                      /* Owner of window.             */
                        (LPVOID) NULL                   /* Pointer not needed.          */
    );

    /* If window could not be created, return "failure" */
    if (!hWnd)
        return (FALSE);

    /*
     * Make the window visible; update its client area;
     */
    ShowWindow(hWnd, nCmdShow);  /* Show the window          */
    UpdateWindow(hWnd);          /* Sends WM_PAINT message   */

    /* Initialize static Server Name and Database */
    szDBServerNameString[0] = szDBNameString[0] = '\0';

    return (TRUE);               /* Returns value from PostQuitMessage */
}

/*************************************************************************

    FUNCTION: MainWndProc(HWND, UINT, WPARAM, LPARAM)

    PURPOSE:  Processes messages

    MESSAGES:

    WM_COMMAND    - application menu (GetDBName dialog box)
    WM_DESTROY    - destroy window

    COMMENTS:

    To process the Cluster Montitor menu item messages, call MakeProcInstance()
    to get the current instance address of the relevant functions.  Then
    call the relevant Dialog box to create the box, according to the information
    in the clumon.rc file, and turn control over to the dialog box message handler
    function.  When it returns, free the instance addresses.

*************************************************************************/

LRESULT FAR PASCAL MainWndProc(HWND hWnd, UINT message,
                            WPARAM wParam, LPARAM lParam)
{
    FARPROC lpProcGetClusterInfo;    /* pointer to the "GetClusterInfo" function */
    FARPROC lpProcGetDBOptions;      /* pointer to the "GetDBOptions" function */
    FARPROC lpProcManageDBOptions;   /* pointer to the "ManageDBOptions" function */
    FARPROC lpProcRestrictServer;    /* pointer to the "RestrictServer" function */
    FARPROC lpProcSetThreshold;      /* pointer to the "SetThreshold" function */
    FARPROC lpProcCreateCopyDB;      /* pointer to the "CreateCopyDB" function */

    int     iError;                  /* Ret code from DialogBox call */

    switch (message)
    { 
       
        /* Intepret Command messages from the app menu */
        case WM_COMMAND:      
            switch (wParam)
            {
                /* Process Information-Cluster menu */
                case INFO_CLUSTER:

                /*
                 *  The GetClusterInfo dialog proc prompts the user for the
                 *  name of the server to base the cluster information against.
                 *  The procedure stores the Server Name filename in the 
                 *  character string szDBServerNameString.
                 */
                lpProcGetClusterInfo = MakeProcInstance(GetClusterInfo, hInst);
                iError = DialogBox (hInst,                  /* current instance */
                                    "ClusterInfoDlg",       /* resource to use  */
                                    hWnd,                   /* parent handle    */
                                    (DLGPROC)lpProcGetClusterInfo);  /* instance address */
                FreeProcInstance(lpProcGetClusterInfo);
                return 0;
            
                             
                /* Process Information-Database Options menu */
                case INFO_DATABASE_OPTIONS:

                /*
                 *  The GetDBOptions dialog proc prompts the user for the
                 *  name of the server and database name to base the options against.
                 *  The procedure stores the Server Name and Database name in the 
                 *  character string szDBServerNameString and szDBNameString, resp.
                 */
                lpProcGetDBOptions = MakeProcInstance(GetDBOptions, hInst);
                iError = DialogBox (hInst,               /* current instance */
                                    "DBOptionsDlg",      /* resource to use  */
                                    hWnd,                /* parent handle    */
                                    (DLGPROC)lpProcGetDBOptions); /* instance address */
                FreeProcInstance(lpProcGetDBOptions);
                return 0;
            

                /*  Process Administration-Manage Database menu */
                case ADMIN_MANAGE_DATABASE:

                /*
                 *  The ManageDBOptions dialog proc prompts the user for the
                 *  name of the server and database name to base the options against.
                 *  The procedure stores the Server Name and Database name in the 
                 *  character string szDBServerNameString and szDBNameString, resp.
                 */
                lpProcManageDBOptions = MakeProcInstance(ManageDBOptions, hInst);
                iError = DialogBox (hInst,                  /* current instance */
                                    "DBAdminDlg",           /* resource to use  */
                                    hWnd,                   /* parent handle    */
                                    (DLGPROC)lpProcManageDBOptions); /* instance address */
                FreeProcInstance(lpProcManageDBOptions);
                return 0;


                /*  Process Administration-Server Restriction menu */
                case ADMIN_RESTRICT_SERVER:

                /*
                 *  The SetThreshold dialog proc prompts the user for the
                 *  name of the server to be restricted.
                 *  The procedure stores the Server Name in szDBServerNameString
                 */
                lpProcRestrictServer = MakeProcInstance(RestrictServer, hInst);
                iError = DialogBox (hInst,                  /* current instance */
                                    "SvrRestrictDlg",       /* resource to use  */
                                    hWnd,                   /* parent handle    */
                                    (DLGPROC)lpProcRestrictServer);  /* instance address */
                FreeProcInstance(lpProcRestrictServer);
                return 0;
            
                /*  Process Administration-Server Availability Threshold menu */
                case ADMIN_MODIFY_THRESHOLD:
               
                /*
                 *  The SetThreshold dialog proc prompts the user for the
                 *  name of the server whose threshold is to be modified.
                 *  The procedure stores the Server Name in szDBServerNameString
                 */
                lpProcSetThreshold = MakeProcInstance(SetThreshold, hInst);
                iError = DialogBox (hInst,                  /* current instance */
                                    "SvrThresholdDlg",       /* resource to use  */
                                    hWnd,                   /* parent handle    */
                                    (DLGPROC)lpProcSetThreshold);  /* instance address */
                FreeProcInstance(lpProcSetThreshold);
                return 0;

                /*  Process Administration-Create and Copy Database menu */
                case ADMIN_CREATECOPY_DB:

                /*
                 *  The CreateCopyDB dialog proc prompts the user for the  
                 *  name of the server and database name for both the source 
                 *  and destination datbases.
                 *  The procedure stores the source Server Name and Database
                 *  name in the character string szDBServerNameString and 
                 *  szDBNameString, resp.
                 */
                lpProcCreateCopyDB = MakeProcInstance(CreateCopyDB, hInst);
                iError = DialogBox (hInst,                  /* current instance */
                                    "DBCreateCopyDlg",       /* resource to use  */
                                    hWnd,                   /* parent handle    */
                                    (DLGPROC)lpProcCreateCopyDB);  /* instance address */
                FreeProcInstance(lpProcCreateCopyDB);
                return 0;

                /*  Process File-Exit menu */
                case FILE_EXIT:
                
                /* Send the Windows message to close the application window.*/
                SendMessage (hWnd, WM_CLOSE, 0, 0L); 
                break;

            }

        case WM_DESTROY:            /* Message: window being destroyed */
            PostQuitMessage(0);
            break;

        default:                    /* Pass on unproccessed messages */
            return (DefWindowProc(hWnd, message, wParam, lParam));
    } 
    return 0;
} 


/************************************************************************

    FUNCTION: GetClusterInfo(HWND, WORD, WPARAM, LPARAM)

    PURPOSE:  Processes messages for "ClusterInfoDlg" dialog box

    MESSAGES:

          WM_INITDIALOG - initialize dialog box
          WM_COMMAND    - input received

    COMMENTS:

          At initialization, the Servername Edit box is set to the 
          existing Servername and the cluster information for that 
          Server is retrieved and displayed.

          When the User clicks "Info", the Servername Edit box is read and
          the cluster information is retrieved and displayed.  The new 
          Servername is stored for future context.
          
          When the User clicks "Done", the dialog box is closed.

***********************************************************************/

BOOL FAR PASCAL GetClusterInfo (HWND hDlg, WORD message,
              WPARAM wParam, LPARAM lParam)
{
    STATUS  nError = NOERROR;
  
    switch (message)
    {
        case WM_INITDIALOG:             /* Message: initialize dialog box */

            /* If a servername is already configured, 
               then display its cluster information */
            if (szDBServerNameString[0] != '\0') 
            {
                SetDlgItemText(hDlg, SERVERNAME_EDIT, szDBServerNameString);

                /* Display the cluster name, load index, and clustermates
                   of the specified server */
                SetCursor (LoadCursor (NULL, IDC_WAIT));
                dspClusterInfo(hDlg, szDBServerNameString, NPN_CLU_SHOW_ALL);
                SetCursor (LoadCursor (NULL, IDC_ARROW));
            }
            return TRUE;

        case WM_COMMAND:            /* Message: received a command */
            switch (wParam)
            {
                case IDOK:          /* "Info" button */

                    /* Clear out "old" cluster information */
                    SetDlgItemText(hDlg, CLUSTERNAME_VALUE, (LPSTR)"");
                    SetDlgItemText(hDlg, AVAILABILITY_VALUE, (LPSTR)"");
                    SetDlgItemText(hDlg, CLUSTERMATES_COUNT, (LPSTR)"");
                    SendDlgItemMessage (hDlg, CLUSTERMATES_LIST, LB_RESETCONTENT, 0, 0L);

                    /* Save Previous Server name in case of error */
                    strcpy (szDBServerNamePrev, szDBServerNameString);                 
                           
                    /* Get Server Name */
                    GetDlgItemText(hDlg,
                                   SERVERNAME_EDIT,
                                   szDBServerNameString,
                                   LINEOTEXT-1);

                    /* If no Server name was entered, restore previous string
                       and display an appropriate error message. */
                    if (szDBServerNameString[0] == '\0')
                    {
                        strcpy (szDBServerNameString, szDBServerNamePrev);                 
                        MessageBox (GetFocus(),"No Server Name entered","Error!", MB_OK);
                        return TRUE;
                    }

                    /* Display the cluster name, load index, and clustermates
                       of the specified server. */
                    SetCursor (LoadCursor (NULL, IDC_WAIT));
                    dspClusterInfo(hDlg, szDBServerNameString, NPN_CLU_SHOW_ALL);
                    SetCursor (LoadCursor (NULL, IDC_ARROW));

                    return TRUE;
                        
                case IDCANCEL:      /* "Done" button */
                    EndDialog(hDlg, FALSE); 
                    return TRUE;
            }
            break;
    }
    return FALSE;                 /* Message not processed.  */
}

/************************************************************************

    FUNCTION: GetDBOptions(HWND, WORD, WPARAM, LPARAM)

    PURPOSE:  Processes messages for "DBOptionsDlg" dialog box

    MESSAGES:

          WM_INITDIALOG - initialize dialog box
          WM_COMMAND    - input received

    COMMENTS:

          At initialization, the Server and Database name Edit boxes
          are set to the existing Servername and Databasename and its 
          database option information is retrieved and displayed.

          When the User clicks "Current", the Server and Database name
          Edit boxes are read and the database option information is 
          retrieved and displayed.  The new Server and Database names
          are stored for future context.

          When the User clicks "Done", the dialog box is closed.

***********************************************************************/

BOOL FAR PASCAL GetDBOptions (HWND hDlg, WORD message,
              WPARAM wParam, LPARAM lParam)
{
    STATUS  nError = NOERROR;

    switch (message)
    {
        case WM_INITDIALOG:             /* Message: initialize dialog box */

            /* Set servername and database text Edit boxes to current value 
               If both are set, then fill-in the current Db Options*/
            if (szDBServerNameString[0] != '\0')
            { 
                SetDlgItemText(hDlg, SERVERNAME_EDIT, szDBServerNameString);
                if (szDBNameString[0] != '\0')
                {
                    SetDlgItemText(hDlg, DBNAME_EDIT, szDBNameString);
            
                    /* Get and display the Option Mask of the database */
                    SetCursor (LoadCursor (NULL, IDC_WAIT));
                    dspDBOptions (hDlg, szDBServerNameString, szDBNameString);
                    SetCursor (LoadCursor (NULL, IDC_ARROW));
                }
            }
            return TRUE;

        case WM_COMMAND:            /* Message: received a command */
            switch (wParam)
            {
                case IDOK:          /* "Current" button */
                    /* Save Previous Server and DB names in case of error */
                    strcpy (szDBServerNamePrev, szDBServerNameString);                 
                    strcpy (szDBNamePrev, szDBNameString);                 
                           
                   /* Get Server Name */
                    GetDlgItemText(hDlg,
                                   SERVERNAME_EDIT,
                                   szDBServerNameString,
                                   LINEOTEXT-1);
                    /* Get Database Name */
                    GetDlgItemText(hDlg,
                                   DBNAME_EDIT,
                                   szDBNameString,
                                   LINEOTEXT-1);

                    /* If no Server name was entered, restore previous string
                       and display an appropriate error message. */
                    if (szDBServerNameString[0] == '\0')
                    {
                        strcpy (szDBServerNameString, szDBServerNamePrev);                 
                        MessageBox (GetFocus(),"No Server Name entered","Error!", MB_OK);
                        return TRUE;
                    }

                    /* If no Database name was entered, restore previous string
                       and display an appropriate error message. */
                    if (szDBNameString[0] == '\0')
                    {
                        strcpy (szDBNameString, szDBNamePrev);                 
                        MessageBox (GetFocus(),"No Database Name entered","Error!", MB_OK);
                        return TRUE;
                    }

                    /* Get and display the Option Mask of the database */
                    SetCursor (LoadCursor (NULL, IDC_WAIT));
                    dspDBOptions (hDlg, szDBServerNameString, szDBNameString);
                    SetCursor (LoadCursor (NULL, IDC_ARROW));
                    return TRUE;

                        
                case IDCANCEL:                  /* "Done" button */
                    EndDialog(hDlg, FALSE);
                    return TRUE;
            }
            break;
    }
    return FALSE;       /* Message not processed */
}


/************************************************************************

    FUNCTION: ManageDBOptions(HWND, WORD, WPARAM, LPARAM)

    PURPOSE:  Processes messages for "DBAdminDlg" dialog box.
              
    MESSAGES:

          WM_INITDIALOG - initialize dialog box
          WM_COMMAND    - input received

    COMMENTS:

          At initialization, the Server and Database name Edit boxes
          are set to the existing Servername and Databasename and its 
          database option information is retrieved and displayed.

          When the User clicks "Current", the Server and Database name
          Edit boxes are read and the database option information is 
          retrieved and displayed.  The new Server and Database names
          are stored for future context.

          When the User clicks "Update", the Server and Database name
          Edit boxes are read along with the Mark Option Checkboxes, and
          the database option is marked appropriately.

          When the User clicks "Done", the dialog box is closed.

***********************************************************************/

BOOL FAR PASCAL ManageDBOptions (HWND hDlg, WORD message,
              WPARAM wParam, LPARAM lParam)
{
    STATUS  nError = NOERROR;

    switch (message)
    {
        case WM_INITDIALOG:             /* Message: initialize dialog box */

            /* Set servername and database text in Edit box to current value 
               If both are set, then fill-in the current Db Options*/
            if (szDBServerNameString[0] != '\0')
            { 
                SetDlgItemText(hDlg, SERVERNAME_EDIT, szDBServerNameString);
                if (szDBNameString[0] != '\0')
                {
                    SetDlgItemText(hDlg, DBNAME_EDIT, szDBNameString);

                    /* Get and display the Option Mask of the database */
                    SetCursor (LoadCursor (NULL, IDC_WAIT));
                    dspDBOptions (hDlg, szDBServerNameString, szDBNameString );
                    SetCursor (LoadCursor (NULL, IDC_ARROW));
                }
            }
            return TRUE;

        case WM_COMMAND:                /* Message: received a command */
            switch (wParam)
            {
                /* Ensure that the three Mark Options Checkboxes are exclusive by 
                   unchecking the appropriate Checkboxes when the user checks one */
                
                /* 1) MARK_IN_SERVICE mutually exclusive from all */
                case DBOPTION_INSERVICE:   
                    if ( IsDlgButtonChecked (hDlg, DBOPTION_INSERVICE) > 0 )
                    {
                        CheckDlgButton (hDlg, DBOPTION_OUTSERVICE, 0);
                        CheckDlgButton (hDlg, DBOPTION_DELETE, 0);
                    }
                    return TRUE;

                /* 2) MARK_OUT_OF_SERVICE exclusive from MARK_IN_SERVICE */
                case DBOPTION_OUTSERVICE:
                    if ( IsDlgButtonChecked (hDlg, DBOPTION_OUTSERVICE) > 0 )
                        CheckDlgButton (hDlg, DBOPTION_INSERVICE, 0);
                    return TRUE;

                /* 3) MARK_FOR_DELETE exclusive from MARK_IN_SERVICE */
                case DBOPTION_DELETE:
                    if ( IsDlgButtonChecked (hDlg, DBOPTION_DELETE) > 0 )
                        CheckDlgButton (hDlg, DBOPTION_INSERVICE, 0);
                    return TRUE;


                case IDOK:              /* "Update" button */ 
                    /* First, save Previous Server name in case of error */
                    strcpy (szDBServerNamePrev, szDBServerNameString);                 
                    strcpy (szDBNamePrev, szDBNameString);                 
                           
                    /* Get Server Name */
                    GetDlgItemText(hDlg,
                                   SERVERNAME_EDIT,
                                   szDBServerNameString,
                                   LINEOTEXT-1);
                    /* Get Database Name */
                    GetDlgItemText(hDlg,
                                   DBNAME_EDIT,
                                   szDBNameString,
                                   LINEOTEXT-1);
                    
                    /* If no Server name was entered, restore previous string
                       and display an appropriate error message. */
                    if (szDBServerNameString[0] == '\0')
                    {
                        strcpy (szDBServerNameString, szDBServerNamePrev);                 
                        MessageBox (GetFocus(),"No Server Name entered","Error!", MB_OK);
                        return TRUE;
                    }

                    /* If no Database name was entered, restore previous string
                       and display an appropriate error message. */
                    if (szDBNameString[0] == '\0')
                    {
                        strcpy (szDBNameString, szDBNamePrev);                 
                        MessageBox (GetFocus(),"No Database Name entered","Error!", MB_OK);
                        return TRUE;
                    }

                    /* Update the DB for the checked Mark values */
                    SetCursor (LoadCursor (NULL, IDC_WAIT));
                    updateDBOptions (hDlg, szDBServerNameString, szDBNameString );
                    SetCursor (LoadCursor (NULL, IDC_ARROW));
                    return TRUE;


                case IDCURRENT:             /* "Current" button */
                    /* First, save Previous Server name in case of error */
                    strcpy (szDBServerNamePrev, szDBServerNameString);                 
                    strcpy (szDBNamePrev, szDBNameString);                 
                           
                    /* Get Server Name */
                    GetDlgItemText(hDlg,
                                   SERVERNAME_EDIT,
                                   szDBServerNameString,
                                   LINEOTEXT-1);
                    /* Get Database Name */
                    GetDlgItemText(hDlg,
                                   DBNAME_EDIT,
                                   szDBNameString,
                                   LINEOTEXT-1);

                    /* If no Server name was entered, restore previous string
                       and display an appropriate error message. */
                    if (szDBServerNameString[0] == '\0')
                    {
                        strcpy (szDBServerNameString, szDBServerNamePrev);                 
                        MessageBox (GetFocus(),"No Server Name entered","Error!", MB_OK);
                        return TRUE;
                    }

                    /* If no Database name was entered, restore previous string
                       and display an appropriate error message. */
                    if (szDBNameString[0] == '\0')
                    {
                        strcpy (szDBNameString, szDBNamePrev);                 
                        MessageBox (GetFocus(),"No Database Name entered","Error!", MB_OK);
                        return TRUE;
                    }

                    /* Get and display the Option Mask of the database */
                    SetCursor (LoadCursor (NULL, IDC_WAIT));
                    dspDBOptions (hDlg, szDBServerNameString, szDBNameString );
                    SetCursor (LoadCursor (NULL, IDC_ARROW));
                    return TRUE;
                        
                case IDCANCEL:          /* "Done" button */
                    EndDialog(hDlg, FALSE); 
                    return TRUE;
            }
            break;
    }
    return FALSE;           /* Message not processed */
}


/************************************************************************

    FUNCTION: RestrictServer(HWND, WORD, WPARAM, LPARAM)

    PURPOSE:  Processes messages for "SvrRestrictDlg" dialog box

    MESSAGES:

          WM_INITDIALOG - initialize dialog box
          WM_COMMAND    - input received

    COMMENTS:

          At initialization, the Server name Edit box is set to the existing
          Servername and the current Restriction information is retrieved
          and displayed.
          
          When the User clicks "Update", the Server name Edit box and the 
          Server Restricted Checbox are read and the server is restricted or
          unrestricted as appropriate.  If successful, the servername is 
          stored for future context.
                     
          When the User clicks "Current", the Server name Edit box is read 
          and the Server Restricted checkbox is updated appropriately 
          (checked if restricted, unchecked if not).  If successful, the 
          servername is stored for future context.
          
          When the User clicks "Shutdown", the Server name Edit box is read
          and the server is shutdown (ie a "QUIT" Console command is sent).
          If successful, the servername is stored for future context.

          When the User clicks "Done", the dialog box is closed.

***********************************************************************/

BOOL FAR PASCAL RestrictServer (HWND hDlg, WORD message,
              WPARAM wParam, LPARAM lParam)
{
    STATUS  nError = NOERROR;
    
    switch (message)
    {
        case WM_INITDIALOG:             /* Message: initialize dialog box */

            /* If a servername is already configured, then display it along with 
               the current server restriction status */
            if (szDBServerNameString[0] != '\0') 
            {
                SetDlgItemText(hDlg, SERVERNAME_EDIT, szDBServerNameString);

                /* Get and display the Server Restriction status of the server */
                SetCursor (LoadCursor (NULL, IDC_WAIT));
                dspServerRestrict (hDlg, szDBServerNameString);
                SetCursor (LoadCursor (NULL, IDC_ARROW));
            }
            return TRUE;

        case WM_COMMAND:            /* Message: received a command */
             switch (wParam)
             {
                case IDOK:          /* "Update" button */
                    /* First, save Previous Server name in case of error */
                    strcpy (szDBServerNamePrev, szDBServerNameString);                 
                
                    /* Get Server Name */
                    GetDlgItemText(hDlg,
                                   SERVERNAME_EDIT,
                                   szDBServerNameString,
                                   LINEOTEXT-1);
    
                    /* If no Server name was entered, restore previous string
                       and display an appropriate error message. */
                    if (szDBServerNameString[0] == '\0')
                    {
                        strcpy (szDBServerNameString, szDBServerNamePrev);                 
                        MessageBox (GetFocus(),"No Server Name entered","Error!", MB_OK);
                        return TRUE;
                    }

                    /* Set Server Restriction as specified by the 
                     * RESTRICT_SVR checkbox 
                     */
                    SetCursor (LoadCursor (NULL, IDC_WAIT));
                    setServerRestrict (hDlg, szDBServerNameString);
                    SetCursor (LoadCursor (NULL, IDC_ARROW));
                    return TRUE;

                case IDCURRENT:     /* "Current" button */
                    /* First, save Previous Server name in case of error */
                    strcpy (szDBServerNamePrev, szDBServerNameString);                 
                
                    /* Get Server Name */
                    GetDlgItemText(hDlg,
                                   SERVERNAME_EDIT,
                                   szDBServerNameString,
                                   LINEOTEXT-1);
    
                    /* If no Server name was entered, restore previous string
                       and display an appropriate error message. */
                    if (szDBServerNameString[0] == '\0')
                    {
                        strcpy (szDBServerNameString, szDBServerNamePrev);                 
                        MessageBox (GetFocus(),"No Server Name entered","Error!", MB_OK);
                        return TRUE;
                    }

                    /* Get and display the Server Restriction status of the server */
                    SetCursor (LoadCursor (NULL, IDC_WAIT));
                    dspServerRestrict (hDlg, szDBServerNameString);
                    SetCursor (LoadCursor (NULL, IDC_ARROW));
                    return TRUE;
                         
                case IDSHUTDOWN:    /* "Shutdown" button */
                    /* First, save Previous Server name in case of error */
                    strcpy (szDBServerNamePrev, szDBServerNameString);                 
                
                    /* Get Server Name */
                    GetDlgItemText(hDlg,
                                   SERVERNAME_EDIT,
                                   szDBServerNameString,
                                   LINEOTEXT-1);
    
                    /* If no Server name was entered, restore previous string
                       and display an appropriate error message. */
                    if (szDBServerNameString[0] == '\0')
                    {
                        strcpy (szDBServerNameString, szDBServerNamePrev);                 
                        MessageBox (GetFocus(),"No Server Name entered","Error!", MB_OK);
                        return TRUE;
                    }

                    /* Shutdown Server */
                    SetCursor (LoadCursor (NULL, IDC_WAIT));
                    shutdownServer (hDlg, szDBServerNameString);
                    SetCursor (LoadCursor (NULL, IDC_ARROW));
                    return TRUE;
                                            
                case IDCANCEL:      /* "Done" button */
                    EndDialog(hDlg, FALSE); 
                    return TRUE;
            }
        break;
    }
    return FALSE;                 /* Message not processed.  */
}


/************************************************************************

    FUNCTION: SetThreshold(HWND, WORD, WPARAM, LPARAM)

    PURPOSE:  Processes messages for "SvrThresholdDlg" dialog box

    MESSAGES:

          WM_INITDIALOG - initialize dialog box
          WM_COMMAND    - input received

    COMMENTS:

          At initialization, the Server name Editbox is set to the existing
          Servername and the current server availability and threshold values
          are retrieved and displayed.
          
          When the User clicks "Update", the Servername Editbox and Availability
          Threshold Editbox are read, and the server is updated accordingly. If
          successful, the servername is stored for future context.

          When the User clicks "Current", the Server name Editbox is read 
          and the current server availability and threshold values
          are retrieved and displayed.  If successful, the servername is 
          stored for future context.

          When the User clicks "Done", the dialog box is closed.

***********************************************************************/

BOOL FAR PASCAL SetThreshold (HWND hDlg, WORD message,
              WPARAM wParam, LPARAM lParam)
{
    STATUS  nError = NOERROR;
  
    switch (message)
    {
        case WM_INITDIALOG:             /* Message: initialize dialog box */

            /* If a servername is already configured, then display it */
            if (szDBServerNameString[0] != '\0') 
            {
                SetDlgItemText(hDlg, SERVERNAME_EDIT, szDBServerNameString);

                /* Get and display the Server Avaialbility Information of the server */
                SetCursor (LoadCursor (NULL, IDC_WAIT));
                dspThresholdInfo (hDlg, szDBServerNameString);
                SetCursor (LoadCursor (NULL, IDC_ARROW));
            }
            return TRUE;

        case WM_COMMAND:            /* Message: received a command */
            switch (wParam)
            {
                case IDOK:           /* "Update" button */
                    /* First, save Previous Server name in case of error */
                    strcpy (szDBServerNamePrev, szDBServerNameString);                 

                    /* Get Server Name */
                    GetDlgItemText(hDlg,
                                   SERVERNAME_EDIT,
                                   szDBServerNameString,
                                   LINEOTEXT-1);
    
                    /* If no Server name was entered, restore previous string
                       and display an appropriate error message. */
                    if (szDBServerNameString[0] == '\0')
                    {
                        strcpy (szDBServerNameString, szDBServerNamePrev);                 
                        MessageBox (GetFocus(),"No Server Name entered","Error!", MB_OK);
                        return TRUE;
                    }

                    /* Set Server Availability Threshold, as specified */
                    SetCursor (LoadCursor (NULL, IDC_WAIT));
                    setThresholdInfo (hDlg, szDBServerNameString);
                    SetCursor (LoadCursor (NULL, IDC_ARROW));
                    return TRUE;

                case IDCURRENT:     /* "Current" button */
                    /* First, save Previous Server name in case of error */
                    strcpy (szDBServerNamePrev, szDBServerNameString);                 

                    /* Get Server Name */
                    GetDlgItemText(hDlg,
                                   SERVERNAME_EDIT,
                                   szDBServerNameString,
                                   LINEOTEXT-1);
    
                    /* If no Server name was entered, restore previous string
                       and display an appropriate error message. */
                    if (szDBServerNameString[0] == '\0')
                    {
                        strcpy (szDBServerNameString, szDBServerNamePrev);                 
                        MessageBox (GetFocus(),"No Server Name entered","Error!", MB_OK);
                        return TRUE;
                    }

                    /* Get and display the Server Availability of the server */
                    SetCursor (LoadCursor (NULL, IDC_WAIT));
                    dspThresholdInfo (hDlg, szDBServerNameString);
                    SetCursor (LoadCursor (NULL, IDC_ARROW));
                    return TRUE;

                case IDCANCEL:      /* "Done" button */
                    EndDialog(hDlg, FALSE); 
                    return TRUE;
            }
            break;
    }
    return FALSE;                 /* Message not processed.  */
}


/************************************************************************

    FUNCTION: CreateCopyDB(HWND, WORD, WPARAM, LPARAM)

    PURPOSE:  Processes messages for "DBCreateCopyDlg" dialog box

    MESSAGES:

          WM_INITDIALOG - initialize dialog box
          WM_COMMAND    - input received

    COMMENTS:

          At initialization, the Server and Database name Edit boxes for
          the source DB are set to the existing Servername and Databasename,
          the Databasename Editbox for the copy DB is set to the existing
          Databasename, and the Replica Copy Flag is checkeed.   No 
          information is programatically retrieved or set.

          When the User clicks "Copy", the Server and Database name
          Edit boxes for both the source and copy databases are read
          along with the copy flag Checkbox.  The database is created and
          copied as specified.  If successful, the source Servername and 
          Databasenames are stored for future context.
          
          When the User clicks "Done", the dialog box is closed.

***********************************************************************/

BOOL FAR PASCAL CreateCopyDB (HWND hDlg, WORD message,
                              WPARAM wParam, LPARAM lParam)
{
    STATUS  nError = NOERROR;
    char    szDBCopyServer[LINEOTEXT];  /* Copy database server name */
    char    szDBCopyFile[LINEOTEXT];    /* Copy database file name */
    BOOL    bReplFlag;                  /* TRUE for repl copy, FALSE for new */

    switch (message)
    {
        case WM_INITDIALOG:             /* Message: initialize dialog box */

            /* Set servername and database text Edit boxes for the DB source
             * file to the current values.
             */
            if (szDBServerNameString[0] != '\0')
            { 
                SetDlgItemText(hDlg, SERVERNAME_EDIT, szDBServerNameString);
                if (szDBNameString[0] != '\0')
                {
                    SetDlgItemText(hDlg, DBNAME_EDIT, szDBNameString);

                    /* Default the DB copy file name */
                    SetDlgItemText(hDlg, DBDEST_EDIT, szDBNameString);
                }
            }

            /* and default to the Replica copy flag */
            CheckDlgButton (hDlg, DBCOPY_REPL, 1);
            CheckDlgButton (hDlg, DBCOPY_NEW, 0);
            return TRUE;

        case WM_COMMAND:            /* Message: received a command */
            switch (wParam)
            {

                /* Ensure that the two copy flag Checkboxes are exclusive */
                case DBCOPY_REPL:   
                    if ( IsDlgButtonChecked (hDlg, DBCOPY_REPL) > 0 )
                        CheckDlgButton (hDlg, DBCOPY_NEW, 0);
                    else
                        CheckDlgButton (hDlg, DBCOPY_NEW, 1);
                    return TRUE;

                case DBCOPY_NEW:
                    if ( IsDlgButtonChecked (hDlg, DBCOPY_NEW) > 0 )
                        CheckDlgButton (hDlg, DBCOPY_REPL, 0);
                    else
                        CheckDlgButton (hDlg, DBCOPY_NEW, 1);
                    return TRUE;

                case IDOK:          /* "Copy" button */
                    /* Save Previous Server and DB names in case of error */
                    strcpy (szDBServerNamePrev, szDBServerNameString);                 
                    strcpy (szDBNamePrev, szDBNameString);                 
                           
                   /* Get the source Server Name */
                    GetDlgItemText(hDlg,
                                   SERVERNAME_EDIT,
                                   szDBServerNameString,
                                   LINEOTEXT-1);
                    /* Get the source Database Name */
                    GetDlgItemText(hDlg,
                                   DBNAME_EDIT,
                                   szDBNameString,
                                   LINEOTEXT-1);

                   /* Get the copy Server Name */
                    GetDlgItemText(hDlg,
                                   SERVERDEST_EDIT,
                                   szDBCopyServer,
                                   LINEOTEXT-1);

                    /* Get the copy Database Name */
                    GetDlgItemText(hDlg,
                                   DBDEST_EDIT,
                                   szDBCopyFile,
                                   LINEOTEXT-1);

                    /* If no Server name was entered, restore previous string
                       and display an appropriate error message. */
                    if (szDBServerNameString[0] == '\0')
                    {
                        strcpy (szDBServerNameString, szDBServerNamePrev);                 
                        MessageBox (GetFocus(),"No Source Server Name entered",
                                    "Error!", MB_OK);
                        return TRUE;
                    }

                    /* If no Database name was entered, restore previous string
                       and display an appropriate error message. */
                    if (szDBNameString[0] == '\0')
                    {
                        strcpy (szDBNameString, szDBNamePrev);                 
                        MessageBox (GetFocus(),"No Source Database Name entered",
                                    "Error!", MB_OK);
                        return TRUE;
                    }

                    /* Likewise for the Copy Server and Database names */
                    if (szDBCopyServer[0] == '\0')
                    {
                        strcpy (szDBServerNameString, szDBServerNamePrev);                 
                        MessageBox (GetFocus(),"No Copy Server Name entered",
                                    "Error!", MB_OK);
                        return TRUE;
                    }

                    if (szDBCopyFile[0] == '\0')
                    {
                        strcpy (szDBNameString, szDBNamePrev);                 
                        MessageBox (GetFocus(),"No Copy Database Name entered",
                                    "Error!", MB_OK);
                        return TRUE;
                    }

                    /* Else, get and set copy flag */
                    if ( IsDlgButtonChecked (hDlg, DBCOPY_REPL) > 0 )
                        bReplFlag = TRUE;
                    else
                        bReplFlag = FALSE;

                    /* and copy the DB */
                    SetCursor (LoadCursor (NULL, IDC_WAIT));
                    createcopyDB (hDlg, szDBServerNameString, szDBNameString,
                                  szDBCopyServer, szDBCopyFile, bReplFlag);
                    SetCursor (LoadCursor (NULL, IDC_ARROW));
                    return TRUE;
                        
                case IDCANCEL:      /* "Done" button */
                    EndDialog(hDlg, FALSE); 
                    return TRUE;
            }
            break;
    }
    return FALSE;                 /* Message not processed.  */
}


/************************************************************************
 
    FUNCTION: dspClusterInfo(HWND, char *, DWORD)
  
    PURPOSE:  Retrieves and displays the Advanced Server Cluster
              information for the "ClusterInfoDlg" dialog box
              The GetServerCluster, GetServerLoad, and 
              GetServerClusterMates routines (clfunc.c) are called 
              to perform the relevant HCL C API for Notes/Domino 
              functions.
  
    COMMENTS:
  
          This routine passes the input server name to the
          GetServerCluster, GetServerLoad, and GetServerClusterMates
          routines to retrieve the Cluster information.  This 
          information is displayed as appropriate to the relevant
          controls of the "ClusterInfoDlg" dialog box.
  
          Although designed to retrieve each piece separately, this
          routine currently is called by the GetClusterInfo() 
          function passing a NPN_CLU_SHOW_ALL flag.  This enables 
          displaying all the three server cluster information elements. 
  
          Any errors are processed and displayed to the user as appropriate.
            
 ************************************************************************/

void dspClusterInfo(HWND hDlg, char *szServerName, DWORD dwClusterFlags)
{
    STATUS nError;
    char szErrorString[LINEOTEXT];      /* Error Message String */
    char szClusterName[MAXUSERNAME];    /* Name of Cluster */
    char szClusterInfo[LINEOTEXT];      /* Desired Cluster info string */

    DWORD dwLoadIndex = 0;
    DHANDLE hClusterList = NULLHANDLE;	 /* list of clustermates */
    void FAR *lpClusterList = NULL;		 /* locked-down cluster list pointer */
      
    WORD wNumListEntries = 0;
    WORD wBufferLen = 0;
    char *pBuffer;
    int i;

    /* If the Show_Server's_Cluster menu item was selected */
    if (dwClusterFlags & NPN_CLU_SHOW_CLUNAME )
    {
        nError = GetServerCluster( (LPSTR)szServerName, (LPSTR)szClusterName );
       
        /* return if error, restoring the previous settings */
        if (nError != NOERROR)
        {
            strcpy (szServerName, szDBServerNamePrev); 
            goto Cleanup;
        }
        else
        {
            /* No errors ->  Change Window Title Context */
            wsprintf ((LPSTR)szWindowTitle, "%s - <%s>", 
                       CLUMON_TITLE, (LPSTR)szServerName);
            SetWindowText (hWnd, (LPSTR)szWindowTitle);
    
            /* and Update Cluster Name dialog item */
            strcpy (szClusterInfo, szClusterName);
            SetDlgItemText(hDlg, CLUSTERNAME_VALUE, (LPSTR)szClusterInfo);
        }
    }

    /* If the Show_Server's_Load menu item was selected */
    if ( dwClusterFlags & NPN_CLU_SHOW_LOAD )
    {
        nError = GetServerLoad( (LPSTR)szServerName, &dwLoadIndex);
      
        if (nError)
            goto Cleanup;
	
        /* Update Availability Index dialog item */
        wsprintf ((LPSTR)szClusterInfo, "%lu", dwLoadIndex);
        SetDlgItemText(hDlg, AVAILABILITY_VALUE, (LPSTR)szClusterInfo);
    }
    
    /* If the Show_Server's_ClusterMates menu item was selected, get the
     * Cluster mates.  To ensure the latest information, the Cluster mate
     * list will be looked up via server NameLookup using by specifiying
     * the CLUSTER_LOOKUP_NOCACHE flag.
     */
    if ( dwClusterFlags & NPN_CLU_SHOW_CLUMATES )
    {

        /* Call the routine to get the cluster list of the specified server.  
         * If hClusterList != NULLHANDLE. then it must be freed in this 
         * block of code.
         */
        nError = GetServerClusterMates( (LPSTR)szServerName, 
                                        (DWORD)CLUSTER_LOOKUP_NOCACHE,
                                        &hClusterList);
    
        /* If the server is in a restricted or unavailable state then we can
         * still continue because it will still return the cluster info to us.
         * If it's another error then quit but make sure that the list is freed
        */
        if (nError)
        {
            if ( !(( ERR(nError) == ERR_SERVER_UNAVAILABLE) || 
                 ( ERR(nError) == ERR_SERVER_RESTRICTED)) )
                goto Cleanup;
        }
        nError = NOERROR;
        lpClusterList = OSLock( void, hClusterList);
        wNumListEntries = ListGetNumEntries( lpClusterList, FALSE);
        
        /* Display the member count */
        wsprintf ((LPSTR)szClusterInfo, "%u", wNumListEntries);
        SetDlgItemText(hDlg, CLUSTERMATES_COUNT, (LPSTR)szClusterInfo);

        /* Get the members from the list */
        for (i = 0; i < (int) wNumListEntries; i++)
        {
            nError = ListGetText( lpClusterList, FALSE, (WORD) i, 
                     FAR &pBuffer, &wBufferLen );
            if (!nError)
            {
                /* Update the Cluster Mates dialog list box */
                memcpy(szClusterInfo, pBuffer, wBufferLen);
                szClusterInfo[wBufferLen] = '\0';
                SendDlgItemMessage (hDlg, CLUSTERMATES_LIST, LB_ADDSTRING, 
                                    0, (LPARAM) ((LPSTR)szClusterInfo));
            }
            else
            {
                OSUnlock( hClusterList);
                goto Cleanup;
            }
        } /* end of FOR */
        
        /* No longer need this memory - Freeing done in "Cleanup:" below */
        OSUnlock( hClusterList);
    }  

Cleanup:
   if (hClusterList != NULLHANDLE)        
       OSMemFree( hClusterList);

   /* Result Processing */
   switch (nError)
   {
        case NPNERR_NOT_CLUSTER_MEMBER:
            wsprintf ((LPSTR)szErrorString, 
                      "Server %s is not a member of any cluster!\n",
                      (LPSTR)szDBServerNameString);
            MessageBox (GetFocus(), (LPSTR)szErrorString, "Error!", MB_OK);
            break;
   
        case NOERROR:
            break;
   
        default:  /* HCL C API for Notes/Domino Error */
            OSLoadString(hModule, ERR(nError),
                         (LPSTR)szErrorString, LINEOTEXT-1);
            MessageBox (GetFocus(), (LPSTR)szErrorString, "Notes Error!", MB_OK);
            break;
    }                 
   return;
}   


/************************************************************************

    FUNCTION: dspDBOptions(HWND, char *, char *)

    PURPOSE:  Retrieves and displays the Database Mark Options for the
              "DBOptionsDlg" and "DBAdminDlg" dialog boxes.  The
              GetDBMarks routine (clfunc.c) is called to perform the
              relevant HCL C API for Notes/Domino functions.
              
    COMMENTS:

          This routine passes the input Server and Database names to the
          GetDBMarks routine which opens the database and returns its Option
          flag mask. The Option flag mask is interpreted and the routine 
          updates the dialog by checking the appropriate option Checkboxes
          in the dialog.

          Note that clusetered server failover is supported by the called
          GetDBMarks routine.  If failover occurs, a message box is
          displayed to inform the user.

          Any errors are processed and displayed to the user as appropriate.

***********************************************************************/

void dspDBOptions (HWND hDlg, char *szServerName, char *szDBName )
{
    DWORD   dwOptionMask;
    STATUS  nError;
    char    szErrorString[LINEOTEXT];   /* Error Message String */
    BOOL    bFailover;                  /* Failover flag */

    /* Call routine to get the Mark options */
    nError = GetDBMarks ((LPSTR)szServerName, (LPSTR)szDBName, 
                            &dwOptionMask, &bFailover);
    
    /* Return if error getting options, restoring previous settings */
    if (nError != NOERROR)
    {
        strcpy (szServerName, szDBServerNamePrev); 
        strcpy (szDBName, szDBNamePrev);
        goto Cleanup;
    }

    /* Report clustered server failover and update server/DB name Edit boxes */
    if (bFailover)
    {
        wsprintf ((szErrorString), 
                  "Clustered Database Server Failover Occurred\nOpened DB: %s!!%s",
                  (LPSTR)szServerName, (LPSTR)szDBName);
        MessageBox (GetFocus(), szErrorString, "Information!", MB_OK);
        SetDlgItemText(hDlg, SERVERNAME_EDIT, szServerName);
        SetDlgItemText(hDlg, DBNAME_EDIT, szDBName);
    }

    /* Display Checkbox Marks for In Service, 
       Out of Service, and Delete to the Dialog box */
    if (dwOptionMask & DBOPTION_OUT_OF_SERVICE)        
    {
        CheckDlgButton (hDlg, DBOPTION_OUTSERVICE, 1);
        CheckDlgButton (hDlg, DBOPTION_INSERVICE, 0);
    }
    else
    {
        CheckDlgButton (hDlg, DBOPTION_OUTSERVICE, 0);
        CheckDlgButton (hDlg, DBOPTION_INSERVICE, 1);
    }
    
    if (dwOptionMask & DBOPTION_MARKED_FOR_DELETE)
        CheckDlgButton (hDlg, DBOPTION_DELETE, 1);
    else
        CheckDlgButton (hDlg, DBOPTION_DELETE, 0);

Cleanup:
    /* Result Processing */
    if ( nError != NOERROR)
    {
        OSLoadString(hModule, ERR(nError),
                     (LPSTR)szErrorString, LINEOTEXT-1);
        MessageBox (GetFocus(), szErrorString, "Notes Error!", MB_OK);
    }

    /* No errors ->  Change Window Title Context */
    else 
    {
        wsprintf ((LPSTR)szWindowTitle, "%s - <%s!!%s>", 
                   CLUMON_TITLE, (LPSTR)szServerName, (LPSTR)szDBName);
        SetWindowText (hWnd, (LPSTR)szWindowTitle);
    }
    return;
}

/************************************************************************

    FUNCTION: updateDBOptions(HWND, char *, char *)

    PURPOSE:  Updates the Database Options Checked by the User in the 
              "DBAdminDlg" dialog box.  The SetDBMarks routine (clfunc.c)
              is called to peform the relevant HCL C API for Notes/Domino
              functions.

    COMMENTS:

          This routine reads in the database option marks checked
          by the user, and passes them along with the input server
          and database names to the SetDBMarks routine to mark the
          database as appropriate.

          If the user has Checked to mark a database for delete, a
          YES-NO warning message box is displayed.  A status message
          box is displayed after the database has been marked.
          
          Any errors are processed and displayed to the user as appropriate.
            
***********************************************************************/

void updateDBOptions (HWND hDlg, char *szServerName, char *szDBName )
{
    STATUS  nError;
    WORD    wMarkFlag = 0;
    char    szErrorString[LINEOTEXT];        /* Error Message String */

    /* Read desired Marks;  Error if both Mark In and Mark Out of Service */
    if ( IsDlgButtonChecked (hDlg, DBOPTION_INSERVICE) > 0 )
        wMarkFlag |= MARK_IN_SERVICE;

    if ( IsDlgButtonChecked (hDlg, DBOPTION_OUTSERVICE) > 0 )
        wMarkFlag |= MARK_OUT_SERVICE;

    if ( IsDlgButtonChecked (hDlg, DBOPTION_DELETE) > 0 )
    {
        strcpy (szErrorString,
                "Marking a database for deletion can not be undone.\nDo you wish to continue?");
     
        if ( MessageBox (GetFocus(), (LPSTR)szErrorString, 
                            "Warning!", MB_YESNO) == IDYES)
            wMarkFlag |= MARK_DELETE;
        else
        { 
            nError = NPNERR_CANCELLED;
            goto Cleanup;
        }
    }

    /* Call routine to set the database marks based on the mark flag */
    nError = SetDBMarks ((LPSTR)szServerName, (LPSTR)szDBName, wMarkFlag);

    /* Return if there was an error, restoring previous settings */    
    if (nError != NOERROR)
    {
        strcpy (szServerName, szDBServerNamePrev); 
        strcpy (szDBName, szDBNamePrev);
        goto Cleanup;
    }

Cleanup:
    /* Result Processing */
    switch (nError)
    {
        case NPNERR_CANCELLED:
            MessageBox (GetFocus(), "Database Mark Cancelled", 
                        "Information!", MB_OK);
            dspDBOptions (hDlg, szServerName, szDBName);
            break;

        case NOERROR:
            MessageBox (GetFocus(), "Database Mark Successful", 
                        "Information!", MB_OK);
            wsprintf ((LPSTR)szWindowTitle, "%s - <%s!!%s>", 
                        CLUMON_TITLE, (LPSTR)szServerName, (LPSTR)szDBName);
            SetWindowText (hWnd, (LPSTR)szWindowTitle);
            break;

        default:
            OSLoadString(hModule, ERR(nError),
                         (LPSTR)szErrorString, LINEOTEXT-1);
            MessageBox (GetFocus(), (LPSTR)szErrorString, "Notes Error!", MB_OK);
            dspDBOptions (hDlg, szServerName, szDBName);
            break;
    }
    return;
}


/************************************************************************

    FUNCTION: dspServerRestrict(HWND, char *)

    PURPOSE:  Retrieves and displays the Server Restriction state for the
              "SvrRestrictDlg" dialog box.   The RemoteCommand routine 
              (clfunc.c) is called to perform the relevant HCL C API for 
              Domino and Notes functions.  Specifically, this routine 
              programmatically uses Domino and Notes remote console commands 
              to retrieve server restriction status. The referenced commands 
              are defined in clumon.h.
              
    COMMENTS:

          This routine passes the input Servername parameter along with 
          the proper console command to the RemoteCommand routine, which
          retrieves the current server restriction status. If successful,
          the returned status (console response) is interpreted and the 
          Server Restricted Checkbox is updated as appropriate.

          Any errors are processed and displayed to the user as appropriate.
            
***********************************************************************/

void dspServerRestrict (HWND hDlg, char *szServerName)
{
    STATUS  nError;
    char    szErrorString[LINEOTEXT];           /* Error Message String */
    char    szServerResponse[LINEOTEXT];        /* response buffer */
    WORD    wResponseLen = strlen(RESTRICTED);  /* and length      */

    /* Call routine to get the Mark options */
    nError = RemoteCommand ((LPSTR)szServerName, GET_RESTRICT, 
                            (LPSTR)szServerResponse, &wResponseLen);
    
    /* Return if error with remote command, restoring previous settings */
    if (nError != NOERROR)
    {
        strcpy (szServerName, szDBServerNamePrev); 
        goto Cleanup;
    }

    /* and check Server Restricted checkbox if SERVER_RESTRICT=1 
     * response is returned
     */
    if (!lstrcmp(szServerResponse, RESTRICTED))        
        CheckDlgButton (hDlg, RESTRICT_SVR, 1);
    else
        CheckDlgButton (hDlg, RESTRICT_SVR, 0);

Cleanup:
    /* Result Processing */
    if ( nError != NOERROR)
    {
        OSLoadString(hModule, ERR(nError),
                     (LPSTR)szErrorString, LINEOTEXT-1);
        MessageBox (GetFocus(), (LPSTR)szErrorString, "Notes Error!", MB_OK);
    }

    /* No errors ->  Change Window Title Context */
    else
    {
        wsprintf ((LPSTR)szWindowTitle, "%s - <%s>", 
                  CLUMON_TITLE, (LPSTR)szServerName);
        SetWindowText (hWnd, (LPSTR)szWindowTitle);
    }
    return;
}

/************************************************************************

    FUNCTION: setServerRestrict(HWND, char *)

    PURPOSE:  Sets and displays the Server Restriction state for the
              "SvrRestrictDlg" dialog box.  The RemoteCommand routine 
              (clfunc.c) is called to perform the relevant HCL C API for 
              Domino and Notes functions.  Specifically, this routine 
              programmatically uses Domino and Notes remote console commands 
              to set/unset the server restriction configuration. The 
              referenced commands are defined in clumon.h.

    COMMENTS:

          This routine reads the Server Restricted Checkbox, builds the 
          appropriate console command, and passes the input Servername 
          parameter amd console command to the RemoteCommand routine, which
          sets the current server restriction status as desired.  If successful,
          a message box is displayed and the Server Restricted Checkbox is
          updated to the current setting.

          Any errors are processed and displayed to the user as appropriate.
            
***********************************************************************/

void setServerRestrict (HWND hDlg, char *szServerName)
{
    STATUS  nError;
    char    szErrorString[LINEOTEXT];       /* Error Message String */
    char    szCommand[LINEOTEXT];           /* remote command */
    char    szServerResponse[LINEOTEXT];    /* response buffer */
    WORD    wResponseLen = 0;               /* and length      */
    BOOL    bRestrictFlag;                  /* TRUE=restrict, FALSE=unrestrict */

    /* Determine whether server is being restricted or unrestricted, and
     * build up the appropriate console command 
     */
    if ( IsDlgButtonChecked (hDlg, RESTRICT_SVR) > 0 )
    {
        wsprintf ((LPSTR)szCommand, "%s%d", RESTRICT_SERVER, 1);
        bRestrictFlag = TRUE;
    }
    else
    {
        wsprintf ((LPSTR)szCommand, "%s", RESTRICT_SERVER);
        bRestrictFlag = FALSE;
    }

    /* Call routine to set the Server restriction */
    nError = RemoteCommand ((LPSTR)szServerName, (LPSTR)szCommand, 
                            (LPSTR)szServerResponse, &wResponseLen);
    
    /* Return if error with remote command, restoring previous settings */
    if (nError != NOERROR)
    {
        strcpy (szServerName, szDBServerNamePrev); 
        goto Cleanup;
    }

Cleanup:
    /* Result Processing */
    if ( nError != NOERROR)
    {
        OSLoadString(hModule, ERR(nError),
                     (LPSTR)szErrorString, LINEOTEXT-1);
        MessageBox (GetFocus(), (LPSTR)szErrorString, "Notes Error!", MB_OK);
        dspServerRestrict (hDlg, szServerName);
    }
    else
    {
        if ( bRestrictFlag )
            MessageBox (GetFocus(), "Server Restricted", 
                        "Information!", MB_OK);
        else
            MessageBox (GetFocus(), "Server Unrestricted", 
                        "Information!", MB_OK);

        wsprintf ((LPSTR)szWindowTitle, "%s - <%s>", 
                  CLUMON_TITLE, (LPSTR)szServerName);
        SetWindowText (hWnd, (LPSTR)szWindowTitle);
    }
    return;
}

/************************************************************************

    FUNCTION: shutdownServer(HWND, char *)

    PURPOSE:  Shuts down (quits) the server specified by the  
              "SvrRestrictDlg" dialog box.  The RemoteCommand routine 
              (clfunc.c) is called to perform the relevant HCL C API for 
              Domino and Notes functions.  Specifically, this routine 
              programmatically uses Domino and Notes remote console commands 
              to shutdown the server. The referenced commands are defined 
              in clumon.h.
              
    COMMENTS:

          This routine first displays a YES-NO messagebox to the user,
          warning them and asking them if they wish to continue.  If yes,
          the the input Servername parameter along with the proper console
          command to the RemoteCommand routine, which shuts down the server. 
          
          If successful, a message box is displayed.   Note, that since the
          server is being shutdown, "error" message boxes may be displayed
          by the remote console command (e.g. Server unavailable).  These
          messages can be expected, but are not interpreted by this routine.
                 
          Any errors are processed and displayed to the user as appropriate.
            
***********************************************************************/

void shutdownServer (HWND hDlg, char *szServerName)
{
    STATUS  nError;
    char    szErrorString[LINEOTEXT];
    char    szServerResponse[LINEOTEXT];    /* response buffer */
    WORD    wResponseLen = 0;               /* and length      */

    /* First warn the user */
    strcpy (szErrorString,
        "A server can not be restarted from a remote console.\nDo you wish to continue with the shutdown?");
     
    if ( MessageBox (GetFocus(), (LPSTR)szErrorString, 
                            "Warning!", MB_YESNO) == IDNO)
    {
        nError = NPNERR_CANCELLED;
        goto Cleanup;
    }

    /* Call routine to shutdown the server */
    nError = RemoteCommand ((LPSTR)szServerName, SHUTDOWN_SERVER, 
                            (LPSTR)szServerResponse, &wResponseLen);
    
    /* Return if error with remote command, restoring previous settings */
    if (nError != NOERROR)
    {
        strcpy (szServerName, szDBServerNamePrev); 
        goto Cleanup;
    }

Cleanup:
    /* Result Processing */
    switch ( nError )
    {
        case NPNERR_CANCELLED:
            MessageBox (GetFocus(), "Server Shutdown Cancelled", 
                        "Information!", MB_OK);
            dspServerRestrict (hDlg, szServerName);
            break;

        case NOERROR:
            MessageBox (GetFocus(), "Server Shutdown",
                        "Information!", MB_OK);
            wsprintf ((LPSTR)szWindowTitle, "%s - <%s>", 
                        CLUMON_TITLE, (LPSTR)szServerName);
            SetWindowText (hWnd, (LPSTR)szWindowTitle);
            break;

        default:
            OSLoadString(hModule, ERR(nError),
                         (LPSTR)szErrorString, LINEOTEXT-1);
            MessageBox (GetFocus(), (LPSTR)szErrorString, "Notes Error!", MB_OK);
            dspServerRestrict (hDlg, szServerName);
            break;
    }
    return;
}

/************************************************************************

    FUNCTION: dspThresholdInfo(HWND, char *)

    PURPOSE:  Retrieves and displays the Server availability and threshold
              information for the "SvrThresholdDlg" dialog box.   The 
              GetServerLoad and RemoteCommand routines (clfunc.c) are called
              to perform the relevant Notes API functions.  RemoteCommand
              programmatically uses Domino and Notes remote console commands 
              to get the server availability threshold information. The referenced
              commands are defined in clumon.h.
              
    COMMENTS:

          This routine passes the input Servername parameter to the 
          GetServerLoad to retrieve the current availability (load)
          index.  If successful, the Availability Static Text field is 
          updated to display this value.

          Next, the input Servername parameter is passed with the proper 
          console command to the RemoteCommand routine, which retrieves 
          the current server availability threshold configuration. If 
          successful, the returned status (console response) is interpreted
          and the Availability Threshold Editbox is updated to display the 
          value.

          Any errors are processed and displayed to the user as appropriate.
            
***********************************************************************/

void dspThresholdInfo (HWND hDlg, char *szServerName)
{
    STATUS  nError;
    char    szErrorString[LINEOTEXT];           /* Error Message String */
    DWORD   dwAvailability;                     /* server availability value*/
    char    szInfo[LINEOTEXT];                  /* info string*/
    char    szServerResponse[LINEOTEXT];        /* response buffer */
    WORD    wResponseLen = LINEOTEXT;           /* and length      */
    int     i;

    /* First retrieve and display the server availability (if noerror) */
    nError = GetServerLoad ((LPSTR)szServerName, &dwAvailability);
    if (nError != NOERROR)
    {
        strcpy (szServerName, szDBServerNamePrev); 
        goto Cleanup;
    }
    wsprintf ((LPSTR)szInfo, "%lu", dwAvailability);
    SetDlgItemText(hDlg, AVAILABILITY_VALUE, (LPSTR) szInfo);

    /* Then call routine to get the current Threshold configuration */
    nError = RemoteCommand ((LPSTR)szServerName, GET_THRESHOLD, 
                            (LPSTR)szServerResponse, &wResponseLen);
    
    /* Return if error with remote command, restoring previous settings */
    if (nError != NOERROR)
    {
        strcpy (szServerName, szDBServerNamePrev); 
        goto Cleanup;
    }

    /* Parse out Threshold setting from response string */
    /* first strip out CR/LF termination of Notes.ini string */
    i = strlen(szServerResponse) - 2;
    szServerResponse[i] = '\0';

    /* if never configured, the threshold is zero */
    if (!strcmp(szServerResponse, CON_ERROR))
        wsprintf((LPSTR)szInfo, "%d", 0); 
   
    /* else read in the value assigned to the config variable */
    else
    {
        i = strlen(THRESHOLD); 
        strcpy(szInfo, "");
        while (szServerResponse[i] != '\0')  
            wsprintf((LPSTR)szInfo, "%s%c", szInfo, szServerResponse[i++]);
    }

  	/* and display to the dialog box */
    SetDlgItemText(hDlg, THRESHOLD_EDIT, (LPSTR)szInfo);

Cleanup:
    /* Result Processing */
    if ( nError != NOERROR)
    {
        OSLoadString(hModule, ERR(nError),
                    (LPSTR)szErrorString, LINEOTEXT-1);
        MessageBox (GetFocus(), (LPSTR)szErrorString, "Notes Error!", MB_OK);
    }

    /* No errors ->  Change Window Title Context */
    else
    {
        wsprintf ((LPSTR)szWindowTitle, "%s - <%s>", 
                  CLUMON_TITLE, (LPSTR)szServerName);
        SetWindowText (hWnd, (LPSTR)szWindowTitle);
    }
    return;
}

/************************************************************************

    FUNCTION: setThresholdInfo(HWND, char *)

    PURPOSE:  Sets and displays the Server availability and threshold
              information for the "SvrThresholdDlg" dialog box.  The
              RemoteCommand routine (clfunc.c) is called to perform the
              relevant HCL C API for Notes/Domino functions.  
              Specifically, this routine programmatically uses Domino and 
              Notes remote console commands to set the server availability 
              threshold configuration. The referenced commands are defined 
              in clumon.h.

    COMMENTS:

          This routine reads the Availability Threshold Editbox, builds the 
          appropriate console command, and passes the input Servername 
          parameter amd console command to the RemoteCommand routine, which
          sets the current server availability threshold as desired.
          If successful, a message box is displayed.

          Any errors are processed and displayed to the user as appropriate.
            
***********************************************************************/

void setThresholdInfo (HWND hDlg, char *szServerName)
{
    STATUS  nError;
    char    szErrorString[LINEOTEXT];       /* Error Message String */
    char    szCommand[LINEOTEXT];           /* remote command */
    char    szServerResponse[LINEOTEXT];    /* response buffer */
    WORD    wResponseLen = 0;               /* and length      */
    int     iThreshold = -1;                /* specified Threshold value */

    /* Read in specified threshold from the THRESHOLD_EDIT editbox */
    iThreshold = (int) GetDlgItemInt(hDlg, THRESHOLD_EDIT, NULL, TRUE);           
    
    /* Check for input errors -> Value must be between 0 and 100 */
    if ( (iThreshold < 0) || (iThreshold > 100) ) 
    {
        nError = NPNERR_BAD_INPUT;
        goto Cleanup;
    }
    
    /* No errors, build console command string and send command */
    wsprintf ((LPSTR)szCommand, "%s%d", SET_THRESHOLD, iThreshold);
    nError = RemoteCommand ((LPSTR)szServerName, (LPSTR)szCommand, 
                            (LPSTR)szServerResponse, &wResponseLen);
    
    /* Return if error with remote command, restoring previous settings */
    if (nError != NOERROR)
        strcpy (szServerName, szDBServerNamePrev); 
        goto Cleanup;

Cleanup:
    /* Result Processing */
    switch (nError)
    {
        case NPNERR_BAD_INPUT:
            strcpy (szErrorString,
                    "Invalid Threshold Value Entered\nValid Range: 0 - 100"); 
            MessageBox (GetFocus(), (LPSTR)szErrorString, "Error!", MB_OK);
            dspThresholdInfo (hDlg, szServerName);
            break;

        case NOERROR:
            MessageBox (GetFocus(), "Server Threshold Updated", 
                        "Information!", MB_OK);
            wsprintf ((LPSTR)szWindowTitle, "%s - <%s>", 
                        CLUMON_TITLE, (LPSTR)szServerName);
            SetWindowText (hWnd, (LPSTR)szWindowTitle);
            break;

        default:
            OSLoadString(hModule, ERR(nError),
                            (LPSTR)szErrorString, LINEOTEXT-1);
            MessageBox (GetFocus(), (LPSTR)szErrorString, "Notes Error!", MB_OK);
            dspThresholdInfo (hDlg, szServerName);
            break;
    }
    return;
}

/************************************************************************

    FUNCTION: createcopyDB(HWND, char *, char *, chare *, char *, BOOL)

    PURPOSE:  Creates and Copies a database file as specified in the 
              "DBCreateCopyDlg" dialog box.   The CreateAndCopyDB
              routine (clfunc.c) is called to perform the relevant
              HCL C API for Notes/Domino functions.

    COMMENTS:

          This routine passes the input source and copy Server and 
          Database names along with the Replica copy flag value to the
          CreateAndCopyDB routine, which creates and copies the
          whole database (with ACL) as desired.
          
          If successful, a status message is displayed. 

          Any errors are processed and displayed to the user as appropriate.
            
***********************************************************************/

void createcopyDB (HWND hDlg, char *szServerName, char *szDBName, 
                   char *szCopyServer, char *szCopyDB, BOOL bReplFlag )
{
    STATUS  nError;
    char    szErrorString[LINEOTEXT];       /* Error Message String */
    DWORD   dwCopyFlag;
    
    /* Determine and set Replication Flag */
    if (bReplFlag)
        dwCopyFlag = DBCOPY_REPLICA;
    else
        dwCopyFlag = (DWORD)0;
          
    /* Pass the input parameters to the CreateCopy routine */
    nError = CreateAndCopyDB ((LPSTR)szServerName, 
                              (LPSTR)szDBName,
                              (LPSTR)szCopyServer, 
                              (LPSTR)szCopyDB,
                              dwCopyFlag);

    /* Result Processing */
    if ( nError != NOERROR)
    {
        OSLoadString(hModule, ERR(nError),
                     (LPSTR)szErrorString, LINEOTEXT-1);
        MessageBox (GetFocus(), (LPSTR)szErrorString, "Notes Error!", MB_OK);
    }

    /* No errors ->  Change Window Title Context */
    else
    {
        MessageBox (GetFocus(), "Database Copy Successful", 
                    "Information!", MB_OK);
        wsprintf ((LPSTR)szWindowTitle, "%s - <%s!!%s>", 
                  CLUMON_TITLE, (LPSTR)szCopyServer, (LPSTR)szCopyDB);
        SetWindowText (hWnd, (LPSTR)szWindowTitle);
    }
    return;
}

