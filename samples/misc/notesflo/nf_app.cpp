/*
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
*/
//
//		nf_app.cpp
//
//	Implementation of the CImageApplication class.
//	Also contains the application window support functions:
//
//

	// C header files
#include <stdio.h>

	// Windows header files
#include <windows.h>
#include <ole2.h>
#include <ole2ver.h>

	// Domino and Notes header files
#include <global.h>
#include <olenotes.h>

	// Force the single instance of GUID_NOTESFLOW into
	// this module (it's declared in notesflo.h).
#include "initguid.h"

	// Application header files
#include "app_rsc.h"
#include "notesflo.h"
#include "olecf.hpp"
#include "nf_hatch.hpp"
#include "ln_buff.hpp"
#include "ln_flow.hpp"
#include "nf_app.hpp"
#include "nf_doc.hpp"
#include "nf_image.hpp"

//
//		CImageApplication::CImageApplication
//
//	Constructor for the class.
//

CImageApplication::CImageApplication (
	BOOL		fStartedByOle
) {
	m_hInst = (HINSTANCE) NULL;		// Save the instance handle
	m_hAppWnd = (HWND) NULL;
	m_lpDoc = (CImageDocument FAR *) NULL;
	m_lpHatch = (CHatchWindow FAR *) NULL;
	m_nRefCount = 0L;
	m_fStartedByOLE = fStartedByOle;
	m_dwClassRegistration = 0;
	m_hMainMenu = (HMENU) NULL;
	m_hColorMenu = (HMENU) NULL;
	m_hHelpMenu = (HMENU) NULL;
}

//
//		CImageApplication::~CImageApplication
//
//	Destructor.
//

CImageApplication::~CImageApplication (
	void
) {
		// Let go of the hatch window
	if (((CHatchWindow FAR *) NULL) != m_lpHatch)
	{
		m_lpHatch->Release ();
		m_lpHatch = (CHatchWindow FAR *) NULL;
	}

		// Don't leave any references to the Document object!
	if (((CImageDocument FAR *) NULL) != m_lpDoc)
	{
		m_lpDoc->Release ();
		m_lpDoc = (CImageDocument FAR *) NULL;
	}
}

//
//		CImageApplication::WindowInit
//
//	Create the window.  The window class must be registered
//	(::RegisterWindow()) in order for this function to
//	succeed.
//
//	Note that calling ::CreateWindow() will call into the
//	application window procedure, which in turn calls
//	CImageApplication::HandleCreate().
//

BOOL CImageApplication::WindowInit (
	DHANDLE		hInstance,
	int			nCmdShow,
	CClassFactory FAR * FAR *lplpClassFactory,
	CImageDocument FAR *lpDoc,
	CHatchWindow FAR *lpHatch
) {
		// Save references to document and hatch windows
	m_lpDoc = lpDoc;
	m_lpDoc->AddRef ();
	
	m_lpHatch = lpHatch;
	m_lpHatch->AddRef ();

		// Make sure this pointer is NULL if something goes wrong
	*lplpClassFactory = (CClassFactory FAR *) NULL;

        m_hInst = (HINSTANCE)hInstance;            // Save the instance handle
	m_hAppWnd = ::CreateWindow (
        pAppWindowClass,
        pAppTitle,
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        (HWND) NULL,
        (HMENU) NULL,
        (HINSTANCE)hInstance,
        (LPVOID)NULL);

	if (((HWND) NULL) == m_hAppWnd)
		return (FALSE);

		// Were we started by OLE?
	if (m_fStartedByOLE)
	{
			// Yes:  Create and register the class factory
		*lplpClassFactory = new CClassFactory (this);
		if (((CClassFactory FAR *) NULL) == *lplpClassFactory)
			return (FALSE);

			// Hang on to the class factory until it's registered
		(*lplpClassFactory)->AddRef ();

			// Register the class factory
		::CoRegisterClassObject (
			GUID_NOTESFLOW,
			(IUnknown FAR *) *lplpClassFactory,
			CLSCTX_LOCAL_SERVER,
			REGCLS_SINGLEUSE,
			&m_dwClassRegistration);

			// Release our pointer
		(*lplpClassFactory)->Release ();
	}
	else
	{
		if (((CImageDocument FAR *) NULL) == m_lpDoc)
			return (FALSE);

			// No:  Show the application window
		this->ShowAppWindow (nCmdShow);
	}

		// Fetch the menu handles, should we need them for
		// in-place activation
	m_hMainMenu = ::GetMenu (m_hAppWnd);
	m_hColorMenu = ::GetSubMenu (m_hMainMenu, 1);
	m_hHelpMenu = ::GetSubMenu (m_hMainMenu, 2);

	return (TRUE);
}

//
//		CImageApplication::HandleClose
//
//	WM_CLOSE message handler.
//

LRESULT CImageApplication::HandleClose (
	HWND	hWnd,
	UINT	message
) {
		// Close the document (and document window)
	if (((CImageDocument FAR *) NULL) != m_lpDoc)
	{
		m_lpDoc->Close ();

		m_lpDoc->Release ();
		m_lpDoc = (CImageDocument FAR *) NULL;
	}

		// Dispose of the hatch window
	if (((CHatchWindow FAR *) NULL) != m_lpHatch)
	{
		m_lpHatch->Release ();
		m_lpHatch = (CHatchWindow FAR *) NULL;
	}

		// Hide the application's window
	this->HideAppWindow ();

		// If we were started by OLE, unregister the class factory
	if (m_fStartedByOLE)
		::CoRevokeClassObject (m_dwClassRegistration);

	return (WIN_MSG_HANDLED);
}

//
//		CImageApplication::HandleCommand
//
//	WM_COMMAND message handler.
//

LRESULT CImageApplication::HandleCommand (
	HWND	hWnd,
	UINT	message,
	UINT	wID,
	UINT	wNotifyCode,
	HWND	hwndCtl
) {
	switch (wID)
	{
		case IDM_ABOUT:
			{
					// Display the "About..." dialog box
#if defined (WIN32)
					// MakeProcInstance isn't a function in Win32!
				FARPROC lpProcAbout = MakeProcInstance (
					(FARPROC) AboutDlgProc, m_hInst);
#else
				FARPROC lpProcAbout = ::MakeProcInstance (
					(FARPROC) AboutDlgProc, m_hInst);
#endif

				::DialogBox (
					m_hInst,
					"AboutBox",
					m_hAppWnd,
                                        (DLGPROC)lpProcAbout);

#if defined (WIN32)
					// FreeProcInstance isn't a function in Win32
				FreeProcInstance (lpProcAbout);
#else
				::FreeProcInstance (lpProcAbout);
#endif
			}
			return (WIN_MSG_HANDLED);

		case IDM_EXIT:
			::SendMessage (hWnd, WM_SYSCOMMAND, SC_CLOSE, 0L);
			return (WIN_MSG_HANDLED);

		case IDM_FGCOLOR:
			if (((CImageDocument FAR *) NULL) != m_lpDoc)
			{
				COLORREF	fgColor = m_lpDoc->GetForegroundColor ();

					// Allow the user to select a color
				if (::SelectColor ("Select Foreground Color", hWnd, &fgColor))
				{
						// Set the drawing color in the pattern
					m_lpDoc->UpdateForegroundColor (fgColor);

						// Force redrawing
					m_lpDoc->InvalidateWindow (NULL, TRUE);
				}
			}
			return (WIN_MSG_HANDLED);

		case IDM_BGCOLOR:
			if (((CImageDocument FAR *) NULL) != m_lpDoc)
			{
				COLORREF	bgColor = m_lpDoc->GetBackgroundColor ();

					// Allow the user to select a color
				if (::SelectColor ("Select Background Color", hWnd, &bgColor))
				{
						// Set the drawing color in the pattern
					m_lpDoc->UpdateBackgroundColor (bgColor);

						// Force redrawing
					m_lpDoc->InvalidateWindow (NULL, TRUE);
				}
			}
			return (WIN_MSG_HANDLED);

		default:
			return (m_lpDoc->CheckForNotesFlowAction (wID));
	}

	return (WIN_MSG_NOT_HANDLED);
}

//
//		CImageApplication::HandleCreate
//
//	Create the document window.
//

LRESULT CImageApplication::HandleCreate (
	HWND			hWnd,
	UINT			message,
	LPCREATESTRUCT	lpCs
) {
		// Initialize the document and hatch windows
	m_lpDoc->WindowInit (hWnd, this->GetInstanceHandle ());

	m_lpHatch->WindowInit (hWnd, this->GetInstanceHandle ());

	m_lpHatch->HideHatchWindow ();

	return (WIN_MSG_HANDLED);
}

//
//		CImageApplication::HandleSize
//
//	Process the Escape key to deactivate when in-place active.
//

LRESULT CImageApplication::HandleSize (
	HWND		hWnd,
	UINT		message,
	WORD		sizeType,
	LONG		width,
	LONG		height
) {
	RECT		clientArea;

	::GetClientRect (hWnd, &clientArea);
	m_lpDoc->ResizeDocWindow (clientArea);

	return (WIN_MSG_HANDLED);
}

//
//		CImageApplication::HandleEscapeKey
//
//	Process the Escape key to deactivate when in-place active.
//

LRESULT CImageApplication::HandleEscapeKey (
	HWND	hWnd,
	UINT	message,
	TCHAR	key
) {
	if (((CImageDocument FAR *) NULL) == m_lpDoc)
		return (0);
	else
		return (m_lpDoc->HandleEscapeKey (hWnd, message, key));
}

//
//		CImageApplication::ShowAppWindow
//
//	Display the application window.
//

void CImageApplication::ShowAppWindow (
	int nCmdShow
) {
	::CoLockObjectExternal (this, TRUE, FALSE);
	::ShowWindow (m_hAppWnd, nCmdShow);
	::UpdateWindow (m_hAppWnd);
}

//
//		CImageApplication::HideAppWindow
//
//	Hide the application window.
//

void CImageApplication::HideAppWindow (
	void
) {
	::CoLockObjectExternal (this, FALSE, TRUE);
	::ShowWindow (m_hAppWnd, SW_HIDE);
}

//
//		CImageApplication::IsInPlaceActive
//
//	Return TRUE if we're In-Place Active.
//

BOOL CImageApplication::IsInPlaceActive (
	void
) {
	if (((CImageDocument FAR *) NULL) != m_lpDoc)
		return (m_lpDoc->IsInPlaceActive ());
	else
		return (FALSE);
}

//
//		CImageApplication::GetOleInPlaceFramePtr
//
//	Get the pointer to the OLE in-place frame from the document object
//

LPOLEINPLACEFRAME CImageApplication::GetOleInPlaceFramePtr (
	void
) const {
	if (((CImageDocument FAR *) NULL) == m_lpDoc)
		return ((LPOLEINPLACEFRAME) NULL);
	else
		return (m_lpDoc->GetOleInPlaceFramePtr ());
}

//
//		CImageApplication::GetOleInPlaceFrameInfoPtr
//
//	Get the pointer to the OLE in-place frame info from the document object
//

LPOLEINPLACEFRAMEINFO CImageApplication::GetOleInPlaceFrameInfoPtr (
	void
) const {
	if (((CImageDocument FAR *) NULL) == m_lpDoc)
		return ((LPOLEINPLACEFRAMEINFO) NULL);
	else
		return (m_lpDoc->GetOleInPlaceFrameInfoPtr ());
}

//
//		IUnknown methods
//

	// QueryInterface
STDMETHODIMP CImageApplication::QueryInterface (
	REFIID			riid,
	LPVOID FAR *	ppvObj
) {
	SCODE			sc = S_OK;

	if (IID_IUnknown == riid)
		*ppvObj = (LPVOID) this;
	else
	{
		*ppvObj = (LPVOID) NULL;
		sc = E_NOINTERFACE;
	}

	if (((LPVOID) NULL) != *ppvObj)
		((LPUNKNOWN) *ppvObj)->AddRef ();

	return (ResultFromScode (sc));
}

	// AddRef
STDMETHODIMP_ (ULONG) CImageApplication::AddRef (
	void
) {
		// Increment and return the reference count
	return (++m_nRefCount);
}

	// Release
STDMETHODIMP_ (ULONG) CImageApplication::Release (
	void
) {
		// Decrement the reference count, and delete Self
		// if the count goes to 0.
	if (0 == (--m_nRefCount))
	{
		delete this;
		return (0);
	}

	return (m_nRefCount);
}
