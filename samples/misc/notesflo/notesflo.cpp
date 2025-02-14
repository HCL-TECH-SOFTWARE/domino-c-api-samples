/*********************************************************************
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
*
*   PROGRAM:	NotesFlo.cpp
*
*	PURPOSE:	NotesFlow demonstration
*
*	NOTES:		This program can run either stand-alone or as an OLE
*				server.  As an OLE server, this program supports
*				self-registration - no .reg file is required.  In
*				addition, this program can generate a registration
*				file automatically if that's needed - run the program
*				with the single command-line argument -MakeReg, and
*				the appropriate file, either win16.reg or win32.reg,
*				will be created.
*
*********************************************************************/

	// Standard C headers
#include <stdio.h>

	// Windows headers
#include <windows.h>            // Required for all Windows apps.
#include <ole2.h>
#include <ole2ver.h>

	// Domino and Notes header files
#include <global.h>
#include <olenotes.h>

	// Application header files
#include "notesflo.h"
#include "app_rsc.h"
#include "nf_image.hpp"
#include "olecf.hpp"
#include "ln_buff.hpp"
#include "ln_flow.hpp"
#include "nf_app.hpp"
#include "nf_doc.hpp"
#include "nf_hatch.hpp"

//
//		Global data
//

	// The one and only application instance
CImageApplication FAR *	lpAppObject = (CImageApplication FAR *) NULL;

	// And the document window object
CImageDocument FAR *	lpDocObject = (CImageDocument FAR *) NULL;

	// Hatch window object instance
CHatchWindow FAR *		lpHatchWindow = (CHatchWindow FAR *) NULL;

	// OLE class factory
CClassFactory FAR *		lpClassFactory = (CClassFactory FAR *) NULL;

	// Windows module handle
HINSTANCE	hInst;

	// Label and ID strings
char *	pAppName		= "NotesFlow";
char *	pAppTitle		= "Sierpinski Carpet";
char *	pAppWindowClass	= "NotesFlowAppClass";
char *	pDocWindowClass	= "NotesFlowDocClass";
char *	pHatchWindowClass	= "NotesFlowHatchClass";
char *	pszCarpet = pAppTitle;
LPWSTR	lpAppOleTitle	= OLESTR ("NotesFlow Demo");

//*********************************************************************
//
//	FUNCTION:	WinMain
//
//	PURPOSE:	Updates registry, calls initialization function,
//				processes message loop.
//
//*********************************************************************

int WINAPI WinMain(
	HINSTANCE	hInstance,
	HINSTANCE	hPrevInstance,
	LPSTR		lpCmdLine,
	int			nCmdShow
) {
	MSG			msg;
	BOOL		fInitSucceeded;
	BOOL		fRegServer;
	BOOL		fUnregServer;
	BOOL		fEmbedding;
	BOOL		fMakeReg;
	int			answer;

		// Set Win16 message queue to the recommended size for OLE
	::SetMessageQueue (96);

		// Figure out whether or not we were started by OLE
	::ParseCommandLine (
		lpCmdLine,
		&fRegServer,
		&fUnregServer,
		&fEmbedding,
		&fMakeReg);

	//
	// Deal with special start cases
	//

		// Is this a request to create the .reg file (-MakeReg)?
	if (fMakeReg)
		return (::RegistryMakeFile (hInstance, lpCmdLine));

	// Are we being asked to un-register (-UnregServer)?
	if (fUnregServer)
		return (::RegistryUnregServer (hInstance, lpCmdLine));

		// Are we being asked to register (-RegServer)?
	if (fRegServer)
		return (::RegistryRegServer (hInstance, lpCmdLine));

		// None of the above - verify the registry entries
	answer = ::RegistryVerify (hInstance, lpCmdLine);
	if (0 != answer)
		return (answer);

	//
	//		Neither -RegServer nor -UnregServer - initialize!
	//

	hInst = hInstance;

		// If there are no previous instances, perform
		// initializations that apply to all instances
    if (!hPrevInstance)
		if (!::InitApplication(hInstance, fEmbedding))
		    return (0);

		// Perform initializations that apply to a specific instance
    if (!(fInitSucceeded = ::InitInstance(hInstance, nCmdShow, fEmbedding)))
        return (0);

		// That good old Windows message loop
    while (::GetMessage(&msg, (HWND) NULL, (WORD) NULL, (WORD) NULL))
    {
			// Are we in-place active?
		if ((((CImageApplication FAR *) NULL) != lpAppObject)
			&& (lpAppObject->IsInPlaceActive ()))
		{
				// Yes - is this a key message?
			if ((msg.message >= WM_KEYFIRST) && (msg.message <= WM_KEYLAST))
			{
					// Yep - have OLE check for accelerator
				if (S_OK == ::OleTranslateAccelerator (
						lpAppObject->GetOleInPlaceFramePtr (),
						lpAppObject->GetOleInPlaceFrameInfoPtr (),
						&msg))
					continue;		// Skip if the keystroke was consumed
			}
		}

		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
    }

		// Clean up
	::TermInstance (fInitSucceeded);

	return (msg.wParam);
}


//*********************************************************************
//
//	FUNCTION:	InitApplication(DHANDLE)
//
//	PURPOSE:	Initializes window data and registers window class
//
//	NOTE:		The pattern must be initialized before this function
//				is called.
//
//*********************************************************************

BOOL InitApplication (
	HINSTANCE	hInstance,
	BOOL		fStartedByOle
) {
    WNDCLASS  wc;

	//
    // Fill in window class structure with parameters
    // that describe the main window.
	//

    wc.style = 0;
    wc.lpfnWndProc = AppWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = ::LoadIcon(hInstance, pAppName);
    wc.hCursor = ::LoadCursor((HINSTANCE) NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) ::GetStockObject (WHITE_BRUSH);
    wc.lpszMenuName = pAppName;
    wc.lpszClassName = pAppWindowClass;

	    // Register the window class and return success/failure code.
    if (!::RegisterClass(&wc))
		return (FALSE);

	//
    // Fill in window class structure with parameters
    // that describe the document window.
	//

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = DocWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = (HICON) NULL;
    wc.hCursor = ::LoadCursor((HINSTANCE) NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH) ::GetStockObject (WHITE_BRUSH);
    wc.lpszMenuName = NULL;
    wc.lpszClassName = pDocWindowClass;

	    // Register the window class and return success/failure code.
    if (!::RegisterClass(&wc))
		return (FALSE);

	//
    // Fill in window class structure with parameters
    // that describe the hatch window.
	//

    wc.style = CS_BYTEALIGNWINDOW;
    wc.lpfnWndProc = HatchWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = (HICON) NULL;
    wc.hCursor = (HCURSOR) NULL;		// We'll be setting sizing cursors
    wc.hbrBackground = (HBRUSH) (COLOR_WINDOW + 1);
		// Use the default system color for the background
    wc.lpszMenuName = NULL;
    wc.lpszClassName = pHatchWindowClass;

	    // Register the window class and return success/failure code.
    if (!::RegisterClass(&wc))
		return (FALSE);

	return (TRUE);
}

//*********************************************************************
//
//	FUNCTION:	InitInstance(DHANDLE, int)
//
//	PURPOSE:	Creates the application object and the application window.
//
//*********************************************************************

BOOL InitInstance (
    HINSTANCE	hInstance,
    int			nCmdShow,
    BOOL		fStartedByOle
) {
		// Check the OLE version number
	DWORD		dwOleVersion = ::OleBuildVersion ();

	if ((HIWORD (dwOleVersion) != rmm) || (LOWORD (dwOleVersion) < rup))
		return (FALSE);

		// Initialize the OLE libraries
	if (FAILED (::OleInitialize (NULL)))
		return (FALSE);

	try
	{
			// Create the window management objects
		lpAppObject = new CImageApplication (fStartedByOle);
		if (((CImageApplication FAR *) NULL) == lpAppObject)
			throw ("Cannot create Application object - insufficient memory");
		lpAppObject->AddRef ();

		lpDocObject = new CImageDocument (lpAppObject);
		if (((CImageDocument FAR *) NULL) == lpDocObject)
			throw ("Cannot create Document object - insufficient memory");
		lpDocObject->AddRef ();

		lpHatchWindow = new CHatchWindow ();
		if (((CHatchWindow FAR *) NULL) == lpHatchWindow)
			throw ("Cannot create hatch window - insufficient memory");
		lpHatchWindow->AddRef ();

		lpAppObject->WindowInit ((DHANDLE)hInstance, nCmdShow, &lpClassFactory,
			lpDocObject, lpHatchWindow);
	}
	catch (const char *pText)
	{
		::MessageBox (::GetFocus (), pText, NULL, MB_OK | MB_ICONSTOP);

		return (FALSE);
	}

	return (TRUE);
}

//*********************************************************************
//
//	FUNCTION:	TermInstance
//
//	PURPOSE:	Clean up when the instance is done
//
//*********************************************************************

BOOL TermInstance (
	BOOL		fInitSucceeded
) {
	if (fInitSucceeded)
	{
		::OleUninitialize ();
	}

	return (TRUE);
}

//*********************************************************************
//
//	FUNCTION: AppWndProc (HWND, UINT, WPARAM, LPARAM)
//
//	PURPOSE:  Processes messages for the application window
//
//*********************************************************************

LRESULT CALLBACK AppWndProc (
	HWND	hWnd,
	UINT	message,
	WPARAM	wParam,
	LPARAM	lParam
) {
    switch (message)
    {
		case WM_CLOSE:
		{
			LRESULT		answer;

			answer = lpAppObject->HandleClose (hWnd, message);

			::DestroyWindow (hWnd);

			return (answer);
		}

		case WM_COMMAND:
		{
#if defined (WIN32)
			WORD	wNotify = HIWORD (wParam);
			WORD	wID = LOWORD (wParam);
			HWND	hCtl = (HWND) lParam;
#else
			WORD	wNotify = HIWORD (lParam);
			WORD	wID = wParam;
			HWND	hCtl = (HWND) LOWORD (lParam);
#endif

			if (WIN_MSG_HANDLED == lpAppObject->HandleCommand (
				hWnd, message, wID, wNotify, hCtl))
			{
				return (WIN_MSG_HANDLED);
			}

			break;
		}

		case WM_CREATE:
			return (lpAppObject->HandleCreate (hWnd, message,
				(LPCREATESTRUCT) lParam));

		case WM_DESTROY:
				// Free the application object
			lpAppObject->Release ();
			lpAppObject = (CImageApplication FAR *) NULL;

			::PostQuitMessage(0);
			
			return (WIN_MSG_HANDLED);

		case WM_SIZE:
			return (lpAppObject->HandleSize (
				hWnd,
				message,
				(WORD) wParam,
				(LONG) ((WORD) LOWORD (lParam)),
				(LONG) ((WORD) HIWORD (lParam))));

			// Look for the Escape key if we're in-place active
		case WM_CHAR:
			if ((VK_ESCAPE == ((TCHAR) wParam)) && (lpAppObject->IsInPlaceActive()))
				return (lpAppObject->HandleEscapeKey (hWnd, message, ((TCHAR) wParam)));
			break;

		default:
			break;
    } 
    
	return (::DefWindowProc(hWnd, message, wParam, lParam));
}

//*********************************************************************
//
//	FUNCTION: DocWndProc (HWND, UINT, WPARAM, LPARAM)
//
//	PURPOSE:  Processes messages for the document window
//
//*********************************************************************

LRESULT CALLBACK DocWndProc (
	HWND	hWnd,
	UINT	message,
	WPARAM	wParam,
	LPARAM	lParam
) {
    switch (message)
    {
		case WM_PAINT:
		{
			LRESULT		answer;

			answer = lpDocObject->HandlePaint (hWnd, message);
			
				// If we were started by OLE . . .
			if (lpAppObject->IsStartedByOLE ())
			{
					// . . . tell the container data changed
				lpDocObject->DataChanged ();
			}

			return (answer);
		}

		case WM_COMMAND:
		{
#if defined (WIN32)
			WORD	wNotify = HIWORD (wParam);
			WORD	wID = LOWORD (wParam);
			HWND	hCtl = (HWND) lParam;
#else
			WORD	wNotify = HIWORD (lParam);
			WORD	wID = wParam;
			HWND	hCtl = (HWND) LOWORD (lParam);
#endif
				// Let the application object handle commands
			if (WIN_MSG_HANDLED == lpAppObject->HandleCommand (
				hWnd, message, wID, wNotify, hCtl))
			{
				return (WIN_MSG_HANDLED);
			}
		}
			break;

			// Look for the Escape key if we're in-place active
		case WM_CHAR:
			if ((VK_ESCAPE == ((TCHAR) wParam)) && (lpAppObject->IsInPlaceActive()))
				return (::SendMessage (
					lpAppObject->GetAppWindowHandle (),
					message,
					wParam,
					lParam));
			break;

			// The document is being resized via the OLE hatch window
		case WM_USER_HATCH_RESIZE:
			return (lpDocObject->HandleHatchResize (
				hWnd,
				message,
				(WORD) wParam,
				(LONG) ((WORD) LOWORD (lParam)),
				(LONG) ((WORD) HIWORD (lParam))));


		case WM_DESTROY:
			lpDocObject->Release ();		// Done with the document object
			lpDocObject = (CImageDocument FAR *) NULL;

			return (WIN_MSG_HANDLED);

		default:
			break;
    } 

	return (::DefWindowProc(hWnd, message, wParam, lParam));
}

//*********************************************************************
//
//	FUNCTION: HatchWndProc (HWND, UINT, WPARAM, LPARAM)
//
//	PURPOSE:  Processes messages for the hatch window
//
//*********************************************************************

LRESULT CALLBACK HatchWndProc (
	HWND	hWnd,
	UINT	message,
	WPARAM	wParam,
	LPARAM	lParam
) {
    switch (message)
    {
		case WM_CREATE:
			return (lpHatchWindow->HandleHatchCreate (hWnd, message));

		case WM_PAINT:
			return (lpHatchWindow->HandleHatchPaint (hWnd, message));

		case WM_SETFOCUS:
				// Make sure cursor is visible if the hatch window gets focus
			::ShowCursor (TRUE);
			return (WIN_MSG_HANDLED);

		case WM_KILLFOCUS:
				// Decrement ShowCursor count when we lose focus
			::ShowCursor (FALSE);
			return (WIN_MSG_HANDLED);

		case WM_LBUTTONDOWN:
				// Detect left mouse click - start tracking for new size
			return (lpHatchWindow->HandleHatchLeftDown (
				hWnd,
				message,
				(LONG) ((SHORT) LOWORD (lParam)),
				(LONG) ((SHORT) HIWORD (lParam)),
				wParam));

		case WM_LBUTTONUP:
				// Detect end of tracking
			return (lpHatchWindow->HandleHatchLeftUp (
				hWnd,
				message,
				(LONG) ((SHORT) LOWORD (lParam)),
				(LONG) ((SHORT) HIWORD (lParam)),
				wParam));

		case WM_MOUSEMOVE:
				// Follow mouse cursor when in the hatch window
			return (lpHatchWindow->HandleHatchMouseMove (
				hWnd,
				message,
				(LONG) ((SHORT) LOWORD (lParam)),
				(LONG) ((SHORT) HIWORD (lParam)),
				wParam));

		case WM_CHAR:
				// Look for the Escape key if we're in-place active
			if ((VK_ESCAPE == ((TCHAR) wParam)) && (lpAppObject->IsInPlaceActive()))
				return (lpHatchWindow->HandleHatchEscapeKey (
					hWnd,
					message,
					wParam,
					lParam,
					lpAppObject->GetAppWindowHandle ()));
			break;

		case WM_USER_HATCH_RESIZE:
				// Send WM_USER_HATCH_RESIZE to the Document window for processing
			return (::SendMessage (lpDocObject->GetDocWindowHandle (),
				message, wParam, lParam));

		case WM_DESTROY:
			LRESULT		answer;

			answer = lpHatchWindow->HandleHatchDestroy (hWnd, message);

			lpHatchWindow->Release ();			// Done with the hatch window
			lpHatchWindow = (CHatchWindow FAR *) NULL;

			return (answer);

		default:
			break;
    } 

	return (::DefWindowProc(hWnd, message, wParam, lParam));
}

//*********************************************************************
//
//	FUNCTION: AboutDlgProc (HWND, UINT, WPARAM, LPARAM)
//
//	PURPOSE:  Processes messages for the "About..." dialog box
//
//*********************************************************************

BOOL CALLBACK AboutDlgProc (
	HWND	hDlg,
	UINT	message,
	WPARAM	wParam,
	LPARAM	lParam
) {
    switch (message)
    {
		case WM_INITDIALOG:
			return (TRUE);

		case WM_COMMAND:
			{
#if defined (WIN32)
				WORD	wNotify = HIWORD (wParam);
				WORD	wID = LOWORD (wParam);
				HWND	hCtl = (HWND) lParam;
#else
				WORD	wNotify = HIWORD (lParam);
				WORD	wID = wParam;
				HWND	hCtl = (HWND) LOWORD (lParam);
#endif

				if ((IDOK == wID) || (IDCANCEL == wID))
				{
					::EndDialog (hDlg, TRUE);
					return (TRUE);
				}
			}
			break;
	}

    return (FALSE);
}
