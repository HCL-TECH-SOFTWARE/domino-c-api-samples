//*********************************************************************
/*
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
*		selcolor.cpp
*
*	Color selection routine and associated components.
*
*	FUNCTIONS:
*
*	SelectColor()			Routine to manage the ChooseColor
*								common dialog
*	SelectColorHookProc()	Hook procedure to customize the
*								common dialog
*
*********************************************************************/

	// Windows headers
#include <windows.h>            // Required for all Windows apps.

	// Application headers
#include "notesflo.h"

//
//		Module data
//

	// Default dialog box title
char DefaultColorDialogTitle [] = "Color";

	// Current dialog box title
char *pColorDialogTitle = DefaultColorDialogTitle;

	// Custom color array for ChooseColor
COLORREF	CustomColors [16] =
{
	RGB (  0,   0,   0),
	RGB (127,   0,   0),
	RGB (255,   0,   0),
	RGB (  0, 127,   0),
	RGB (  0, 255,   0),
	RGB (  0,   0, 127),
	RGB (  0,   0, 255),
	RGB (127, 127,   0),
	RGB (127,   0, 127),
	RGB (  0, 127, 127),
	RGB (127, 127, 127),
	RGB (127, 255, 127),
	RGB (255, 255,   0),
	RGB (255,   0, 255),
	RGB (  0, 255, 255),
	RGB (255, 255, 255)
};

//*********************************************************************
//
//	FUNCTION: SelectColor (char *, HWND, COLORREF *)
//
//	PURPOSE:  Wrap the Windows common color dialog routine
//
//*********************************************************************

BOOL SelectColor (
	char *			pTitle,
	HWND			hOwner,
	COLORREF *		pColor
) {
	CHOOSECOLOR		cc;
	DWORD			dialogFlags;
	LPCCHOOKPROC	lpfnHook;

	if (((char *) NULL) != pTitle)
	{
		pColorDialogTitle = pTitle;
		dialogFlags = CC_RGBINIT | CC_FULLOPEN | CC_ENABLEHOOK;
		lpfnHook = (LPCCHOOKPROC)SelectColorHookProc;
	}
	else
	{
		pColorDialogTitle = DefaultColorDialogTitle;
		dialogFlags = CC_RGBINIT | CC_FULLOPEN;
		lpfnHook = (LPCCHOOKPROC) NULL;
	}

		// Initialize the dialog data structure
	cc.lStructSize    = sizeof (CHOOSECOLOR);
	cc.hwndOwner      = hOwner;
	cc.hInstance      = NULL;
	cc.rgbResult      = *pColor;
	cc.lpCustColors   = CustomColors;
	cc.Flags          = dialogFlags;
	cc.lCustData      = 0L;
	cc.lpfnHook       = lpfnHook;
	cc.lpTemplateName = NULL;

		// Use the common dialog box to select a color
	if (ChooseColor (&cc))
	{
		*pColor = cc.rgbResult;
		return (TRUE);
	}
	else
	{
		return (FALSE);
	}

}

//*********************************************************************
//
//	FUNCTION: SelectColorHookProc (HWND, UINT, WPARAM, LPARAM)
//
//	PURPOSE:  Customize the common color dialog
//
//*********************************************************************

UINT CALLBACK SelectColorHookProc (
	HWND	hWnd,
	UINT	message,
	WPARAM	wParam,
	LPARAM	lParam
) {
		// Just change the dialog title on WM_INITDIALOG
	switch (message)
	{
		case WM_INITDIALOG:
			return (SetWindowText (hWnd, pColorDialogTitle));
	}

	return (FALSE);
}
