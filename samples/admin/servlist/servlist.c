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

    PROGRAM:    servlist

    FILE:       servlist.c

    PURPOSE:    Shows how to call NSGetServerList to get the list of
                HCL Domino Servers visible on all LAN ports.

    DESCRIPTION:
        This program gets a list of available HCL Domino Servers,
        effectively duplicating the list of servers presented when the
        File/Database/Add dialog box is opened.

        This calls the function NSGetServerList, which returns the list of
        servers. NSGetServerList takes two parameters: the name of the
        LAN port on which to search for servernames, and a handle to use
        to return the list.  This program specifies NULL as the port name
        to get a list of servers available on all LAN ports.  If a
        server is known on more than one port, its name will appear in the
        list only once.

        This then parses the buffer returned by NSGetServerList, reading the
        server names one at a time.  When all server names have been read,
        a listbox containing all of the names is displayed.

****************************************************************************/

#include <windows.h>            /* required for all Windows apps */

/* Standard Library Include Files */

#include <stdlib.h>
#include <string.h>

/* DOMINO AND NOTES INCLUDE FILES */

#include <global.h>
#include <osmisc.h>
#include <nsfdb.h>
#include <names.h>
#include <osenv.h>
#include <osfile.h>
#include <osmem.h>
#include <nif.h>
#include <ns.h>
#include <neterr.h>

/*
 *  LOCAL INCLUDE FILES
 */

#include "servlist.h"

extern int      __argc;
extern char **  __argv;

/*
 *  Local Defines
 */

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

#define LINEOTEXT   256

/*
 *  static values
 */

static DHANDLE     hInst, hModule;             /* current instance,
                                                module handle.    */

/*************************************************************************

 FUNCTION: WinMain(HINSTANCE, HINSTANCE, LPSTR, int)
    
 PURPOSE: calls initialization function, processes message loop

*************************************************************************/

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
           LPSTR lpCmdLine, int nCmdShow)

{
    MSG     msg;         /* message.         */
    STATUS  nError;      /* Status of HCL C API for Notes/Domino call. */

    hModule = GetModuleHandle((LPSTR) "ServList");
    hInst = hInstance;

    nError = NotesInitExtended(__argc, __argv);

    /*
     *  If the call to NotesInitExtended fails, display an error message
     *  and exit the program.
     */

    if (nError != NOERROR)
    {
        MessageBox (GetFocus(),"Unable to Initialize Notes!","SERVLIST", MB_OK);
        return (FALSE);
    }

    if (!hPrevInstance)              /* Other instances of app running? */
    if (!InitApplication(hInstance)) /* Initialize shared things */
        return (FALSE);                /* Exits if unable to initialize  */

  /*
   *  Perform initializations that apply to a specific instance
   */

    if (!InitInstance(hInstance, nCmdShow))
        return (FALSE);

  /*
   *  Acquire and dispatch messages until WM_QUIT message is received.
   */

  while (GetMessage(&msg,       /* message structure                  */
                   (HWND) NULL,          /* handle of window receiving the msg */
                   (WORD) NULL,          /* lowest message to examine          */
                   (WORD) NULL))         /* highest message to examine         */
    {
        TranslateMessage(&msg);     /* Translates virtual key codes.   */
        DispatchMessage(&msg);      /* Dispatches message to window.   */
    }

    NotesTerm();                  /* Terminate the Domino and Notes runtime system. */

    return (msg.wParam);          /* Returns the value from PostQuitMessage */
}


/************************************************************************

 FUNCTION: InitApplication(DHANDLE)

 PURPOSE: Initializes window data and registers window class

*************************************************************************/

BOOL InitApplication(hInstance)
DHANDLE hInstance;               /* current instance         */
{
    WNDCLASS  wc;

    /*
     *  Fill in window class structure with parameters
     *  that describe the main window.
     */

    wc.style = (WORD) NULL;             /* Class style(s).        */

    wc.cbClsExtra = 0;                  /* No per-class extra data.    */
    wc.cbWndExtra = 0;                  /* No per-window extra data.   */
    wc.hInstance = hInstance;           /* App. that owns the class.   */
    wc.hIcon = LoadIcon((DHANDLE) NULL, IDI_APPLICATION);
    wc.hCursor = LoadCursor((DHANDLE) NULL, IDC_ARROW);
    wc.hbrBackground = GetStockObject(WHITE_BRUSH); 
    wc.lpszMenuName =  "ServListMenu";  /* Menu name in .RC file. */
    wc.lpszClassName = "ServListClass"; /* Name used in CreateWindow call. */

    wc.lpfnWndProc = MainWndProc;       /* Function for getting messages
                                         for windows of this class. */

    /*
     *  Register the window class and return success/failure code.
     */

    return (RegisterClass(&wc));

}


/*************************************************************************

 FUNCTION:  InitInstance(DHANDLE, int)

 PURPOSE:  Saves instance handle and creates main window

*************************************************************************/

BOOL InitInstance(hInstance, nCmdShow)
DHANDLE  hInstance;        /* Current instance identifier.       */
int     nCmdShow;         /* Param for first ShowWindow() call. */
{
  HWND    hWnd;           /* Main window handle.                */

    /*
     *  Save the instance handle in static variable, which will be
     *  used in many subsequence calls from this application to Windows.
     */

    hInst = hInstance;

    /*
     *  Create a main window for this application instance.
     */

    hWnd = CreateWindow(
                        "ServListClass",                /* See RegisterClass() call.    */
                        "ServList API program",         /* Text for window title bar.   */
                        WS_OVERLAPPEDWINDOW,            /* Window style.                */
                        CW_USEDEFAULT,                  /* Default horizontal position. */
                        CW_USEDEFAULT,                  /* Default vertical position.   */
                        CW_USEDEFAULT,                  /* Default width.               */
                        CW_USEDEFAULT,                  /* Default height.              */
                        (HWND) NULL,                    /* No parent.                   */
                        (HMENU) NULL,                   /* Use the window class menu.   */
                        hInstance,                      /* Owner of window.             */
                        (LONG) NULL);                   /* Pointer not needed.          */

    /*
     *  If window could not be created, return "failure"
     */

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

 WM_COMMAND    - application menu (ServList dialog box)
 WM_DESTROY    - destroy window

 COMMENTS:

 To process the FILE_GET_SERVER_LIST message, call MakeProcInstance()
 to get the current instance address of the ServListDlg() function.
 Then call Dialog box which will create the box according to the
 information in the SERVLIST.RC file and turn control over to the
 ServListDlg() function.  When it returns, free the instance address.

*************************************************************************/


LRESULT FAR PASCAL MainWndProc(HWND hWnd, UINT message,
                               WPARAM wParam, LPARAM lParam)

{
    FARPROC lpDlgProc;     /* pointer to the "ServListDlg" function */

    switch (message)
    {      
        case WM_COMMAND:       /* message: command from app.  menu */

        switch (wParam)
        {
            case FILE_GET_SERVER_LIST:

                lpDlgProc = MakeProcInstance((FARPROC) ServListDlg,
                                          GetWindowWord(hWnd, GWW_HINSTANCE));
                DialogBox (hInst, "SERVERLIST", hWnd, (DLGPROC)lpDlgProc);
                FreeProcInstance (lpDlgProc);
                return (FALSE);
                break;

            case FILE_QUIT:
                
            /*
             * User chose to quit the program.  Send the Windows
             * message to close the application window.
             */
             
                SendMessage (hWnd, WM_CLOSE, 0, 0L); 
                break;
        } 

        case WM_DESTROY:            /* message: window being destroyed */

            PostQuitMessage(0);
            break;

        default:                    /* Passes it on if unproccessed    */

            return (DefWindowProc(hWnd, message, wParam, lParam));
    }
    return (TRUE);
}


/************************************************************************

 FUNCTION: ServListDlg(HWND, WORD, WPARAM, LPARAM)

 PURPOSE:  Processes messages for "ServList" dialog box

 MESSAGES:

 WM_INITDIALOG - initialize dialog box
 WM_COMMAND    - Input received

 COMMENTS: When the WM_INITDIALOG message is received, generate the
           list of servers and display it in a listbox.  The list is
           generated by calling the function NSGetServerList().
           Specifying NULL as the first parameter (the name of the
           network port on which to search) will return a list of
           servers found on all network ports.
           
***********************************************************************/


BOOL FAR PASCAL ServListDlg(HWND hDlg, WORD message,
                            WPARAM wParam, LPARAM lParam)
{
    STATUS    sError = NOERROR;       /* Error return from API routines. */
    char      ServerString[MAXPATH];  /* String to hold server names.   */
    LPSTR     szServerString = ServerString;
    USHORT    i;                             /* Loop counter. */
    DHANDLE     hServerList=NULLHANDLE; /* Handle returned by NSGetServerList */
    BYTE far *pServerList;            /* Pointer to start of Server List */
    WORD      wServerCount;           /* Number of servers in list. */
    WORD far *pwServerLength;         /* Index to array of servername lens */
    BYTE far *pServerName;               

    switch (message)
    {
        case WM_COMMAND:            /* message: received a command      */

            switch (wParam)
            {
                case IDOK:           /* "OK" box selected?               */
                    EndDialog(hDlg, TRUE);  /* Exits the dialog box.         */
                    return (TRUE);
            }            
            break;

        case WM_INITDIALOG:         /* message: initialize dialog box   */

        /*
         *  Get the list of available servers.  Setting the first parameter
         *  to NULL gets a list of known servers on all ports.
         */
         
        sError = NSGetServerList( (char far *) NULL, &hServerList);
                                          
        if (sError != NOERROR)
        {
            char String[256];
            OSLoadString(NULLHANDLE, ERR(sError), String, sizeof(String)-1);
            MessageBox (GetFocus(), String, "Notes Error", MB_OK);
            return (TRUE);
        }

        /*
         *  Lock the handle returned to get the list of servers.  The buffer
         *  returned is in the following format:
         *
         *  The first WORD specifies the number of servernames in the buffer.
         *  This is followed by a series of N WORDS (N being the number
         *  of servernames in the buffer), specifying the length of each
         *  servername (without any NULL terminator).  This is then
         *  followed by the packed list of servernames.
         */
       
        /*
         *  First, get a pointer to the start of the buffer, and then
         *  get the number of servernames in the list.
         */
         
        pServerList  = (BYTE far *)OSLockObject(hServerList);
        wServerCount = *(WORD *)pServerList;

        /*
         *  Now, get a pointer to the first member in the array of
         *  servername lengths.
         */
         
        pwServerLength = (WORD *)(pServerList + sizeof(WORD));

      /*
       *  Now get a pointer to the first servername in the packed list.
       */

        pServerName = (BYTE far *) pServerList + sizeof(wServerCount) +
                      ((wServerCount) * sizeof(WORD));

        /*
         *  Copy each servername to a local character buffer, add a null
         *  terminator, then add the server name to the listbox.
         */
         
        for (i=0; i<wServerCount; pServerName+=pwServerLength[i], i++)
        {
            memmove (szServerString, pServerName, pwServerLength[i]);
            szServerString[pwServerLength[i]] = '\0'; 
            SendDlgItemMessage(hDlg, SERVLIST_LISTBOX, LB_ADDSTRING,
                                  (WORD) NULL, (LONG)(LPSTR) szServerString);
        }

        /*
         *  Unlock and free the memory associated with the server list.
         */
         
        OSUnlockObject (hServerList);
        OSMemFree (hServerList);

    }
    
    return (FALSE);                 /* Didn't process a message.  */
}
