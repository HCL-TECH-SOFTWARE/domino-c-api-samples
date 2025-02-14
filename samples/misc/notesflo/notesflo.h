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
//
//		notesflo.h
//
//	Application header file.
//

#ifndef _NOTESFLO_H_
#define _NOTESFLO_H_	1

//
//		Constants and suchlike
//

#define	WIN_MSG_HANDLED			(0)
#define	WIN_MSG_NOT_HANDLED		(1)

#define DEFAULT_FOREGROUND_COLOR	RGB (255, 255, 255)
#define DEFAULT_BACKGROUND_COLOR	RGB (128,128, 255)

	// Specialty Windows messages
#define WM_USER_HATCH_RESIZE	(WM_USER)		// Resize of hatch window

//
//		GUID for NotesFlow sample
//
#define APP_CLASS_ID_STRING "{D5D6F660-4706-11d0-9F75-00805F98C02F}"
DEFINE_GUID(GUID_NOTESFLOW, 0xd5d6f660, 0x4706,
	0x11d0, 0x9f, 0x75, 0x0, 0x80, 0x5f, 0x98, 0xc0, 0x2f);

//
//		Global data
//

extern char *pAppName;
extern char *pAppTitle;
extern char *pAppWindowClass;
extern char *pDocWindowClass;
extern char *pHatchWindowClass;
extern char *pszCarpet;
extern LPWSTR lpAppOleTitle;

//
//		Function declarations
//
//	Functions in notesflo.cpp:
//

	// Initialize the application
BOOL InitApplication (
	HINSTANCE	hInstance,
	BOOL		fStartedByOle
);

	// Initialize the instance
BOOL InitInstance (
    HINSTANCE	hInstance,
    int			nCmdShow,
	BOOL		fStartedByOle
);

	// Clean up when the instance is done
BOOL TermInstance (
	BOOL		fInitSucceeded
);

	// The application window procedure
LRESULT CALLBACK AppWndProc (
	HWND	hWnd,
	UINT	message,
    WPARAM	wParam,
	LPARAM	lParam
);

	// The document child window procedure
LRESULT CALLBACK DocWndProc (
	HWND	hWnd,
	UINT	message,
    WPARAM	wParam,
	LPARAM	lParam
);

	// The hatch window procedure
LRESULT CALLBACK HatchWndProc (
	HWND	hWnd,
	UINT	message,
    WPARAM	wParam,
	LPARAM	lParam
);

	// The "About..." dialog box procedure
BOOL CALLBACK AboutDlgProc (
	HWND	hDlg,
	UINT	message,
    WPARAM	wParam,
	LPARAM	lParam
);

//
//	Functions in nf_util.cpp
//

	// Scan the command line for switches
void ParseCommandLine (
	LPSTR		lpCmdLine,
	BOOL FAR *	lpRegServer,
	BOOL FAR *	lpUnregServer,
	BOOL FAR *	lpEmbedding,
	BOOL FAR *	lpMakeReg
);

	// Register the server
int RegistryRegServer (
	HINSTANCE	hInstance,
	LPSTR		lpCmdLine
);

	// Unregister the server
int RegistryUnregServer (
	HINSTANCE	hInstance,
	LPSTR		lpCmdLine
);

	// Verify the registry entries
int RegistryVerify (
	HINSTANCE	hInstance,
	LPSTR		lpCmdLine
);

	// Create the .reg file
int RegistryMakeFile (
	HINSTANCE	hInstance,
	LPSTR		lpCmdLine
);

	// Recursively delete registry keys
LONG RegistryRecursiveDeleteKey (
	HKEY		hParentKey,
	LPSTR		lpKeyName
);

	// Coordinate scale transformations
int XFormWidthHimetricToPixel (
	int		width
);

int XFormHeightHimetricToPixel (
	int		height
);

int XFormWidthPixelToHimetric (
	int		width
);

int XFormHeightPixelToHimetric (
	int		height
);

//
//	Functions in selcolor.cpp:
//

	// Select the color
BOOL SelectColor (
	char *		pTitle,
	HWND		hOwner,
	COLORREF *	pColor
);

	// Hook function to customize the color dialog */
UINT CALLBACK SelectColorHookProc (
	HWND	hWnd,
	UINT	message,
	WPARAM	wParam,
	LPARAM	lParam
);

#endif
