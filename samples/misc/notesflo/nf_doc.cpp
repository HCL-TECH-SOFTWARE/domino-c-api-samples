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
//		nf_doc.cpp
//
//	Implementation of the CImageDocument class.
//

	// Windows header files
#include <windows.h>
#include <ole2.h>
#include <ole2ver.h>

	// Domino and Notes header files
#include <global.h>
#include <olenotes.h>

	// Application header files
#include "notesflo.h"
#include "ln_buff.hpp"
#include "ln_flow.hpp"
#include "nf_app.hpp"
#include "nf_doc.hpp"
#include "nf_image.hpp"
#include "nf_obj.hpp"

//
//		CImageDocument::CImageDocument
//
//	Constructor for the class.
//

CImageDocument::CImageDocument (
	CImageApplication FAR *	lpApp
) {
	m_hDocWnd = (HWND) NULL;

	m_lpApp = lpApp;
	m_lpApp->AddRef ();

	m_lpObj = (CImageObject FAR *) NULL;
	m_nRefCount = 0L;
	m_fIsClosing = FALSE;
	m_CurOffset.cx = 0;
	m_CurOffset.cy = 0;
	m_CurSize.cx = 81;		// Default size is 81 x 81 pixels
	m_CurSize.cy = 81;
	m_cfCarpet = 0;

		// Create the pattern object
	m_lpPattern = new CImagePattern ();

	if (((CImagePattern FAR *) NULL) == m_lpPattern)
		throw ("Cannot generate default pattern");
}

//
//		CImageDocument::~CImageDocument
//
//	Destructor.
//

CImageDocument::~CImageDocument (
	void
) {
		// We're done with the pattern object
	delete m_lpPattern;

		// Release the Application object.
	if (((CImageApplication FAR *) NULL) != m_lpApp)
	{
		m_lpApp->Release ();
		m_lpApp = (CImageApplication FAR *) NULL;
	}
}

//
//		CImageDocument::WindowInit
//
//	Create and initialize the document window.  The document window
//	class must be registered (::RegisterWindow()) before this member
//	function can be called!
//

BOOL CImageDocument::WindowInit (
	HWND		hAppWnd,
	HINSTANCE	hInst
) {
	RECT		docRect;

		// Get the rectangle for the document window
	::GetClientRect (hAppWnd, &docRect);

		// Try creating the document window
	m_hDocWnd = ::CreateWindow (
        pDocWindowClass,
        (char *) NULL,
        WS_CHILD | WS_CLIPSIBLINGS,
        docRect.left,
        docRect.top,
        docRect.right,
        docRect.bottom,
        hAppWnd,
        (HMENU) NULL,
        hInst,
        (LPVOID) NULL);

	if (((HWND) NULL) == m_hDocWnd)
		return (FALSE);

		// Paint the window
	this->ShowDocWindow ();

	m_cfCarpet = ::RegisterClipboardFormat (pszCarpet);

// NotesFlow code
		// Get the clipboard format IDs for NotesFlow
	m_NotesManager.InitializeClipboardFormats ();
// End NotesFlow code

	return (TRUE);
}

//
//		CImageDocument::InvalidateWindow
//
//	Invalidate the document window
//

BOOL CImageDocument::InvalidateWindow (
	const LPRECT	lpRect,
	BOOL			fErase
) {
	return (::InvalidateRect (m_hDocWnd, lpRect, fErase));
}

//
//		CImageDocument::SetDocWindow
//
//	Move the document window to the given location and dimensions.
//

void CImageDocument::SetDocWindow (
	LPRECT		lpRect
) {
	::MoveWindow (
		m_hDocWnd,
		lpRect->left,
		lpRect->top,
		lpRect->right - lpRect->left,
		lpRect->bottom - lpRect->top,
		FALSE);
}

//
//		CImageDocument::DrawImage
//
//	Draw the image in the specifed drawing environment.
//

long CImageDocument::DrawImage (
	CDrawEnvironment &	DrawEnv
) {
	m_lpPattern->NormalizeRectangle (DrawEnv.GetClientRectPtr ());

		// Paint the background
	DrawEnv.SetColors (m_lpPattern->GetBackgroundColor (),
		m_lpPattern->GetBackgroundColor ());	// Yes, they are BOTH background!
	DrawEnv.FillArea (DrawEnv.GetInvalidRectPtr ());
	DrawEnv.RestoreColors ();

		// Draw the pattern
	DrawEnv.SetColors (m_lpPattern->GetBackgroundColor (),
		m_lpPattern->GetForegroundColor ());
	m_lpPattern->DrawSierpinskiCarpet (DrawEnv, DrawEnv.GetClientRectPtr ());
	DrawEnv.RestoreColors ();

	return (WIN_MSG_HANDLED);
}

//
//		CImageDocument::HandlePaint
//
//	WM_PAINT message handler.
//

long CImageDocument::HandlePaint (
	HWND	hWnd,
	UINT	message
) {
	HDC			hDc;
	PAINTSTRUCT	ps;
	RECT		clientArea;

	hDc = ::BeginPaint (hWnd, &ps);

		// Set the mapping mode to be used by the image
	::SetMapMode (hDc, MM_ISOTROPIC);
	::SetWindowOrgEx (
		hDc,
		this->GetXOffset (),
		this->GetYOffset (),
		NULL);
	::SetWindowExtEx (
		hDc,
		this->GetXSize (),
		this->GetYSize (),
		NULL);
	::SetViewportExtEx (
		hDc,
		this->GetXSize (),
		this->GetYSize (),
		NULL);

		// Figure out drawing area for Image algorithm
	::GetClientRect (hWnd, &clientArea);

		// Stuff the platform-specific graphics info
		// into a CDrawEnvironment object.
	CDrawEnvironment drawEnv (hDc, &clientArea, &ps.rcPaint, FALSE);

		// Draw into the image area
	this->DrawImage (drawEnv);

	::EndPaint (hWnd, &ps);
	
	return (WIN_MSG_HANDLED);
}

//
//		CImageDocument::HandleEscapeKey
//
//	Process the Escape key to deactivate when in-place active.
//

long CImageDocument::HandleEscapeKey (
	HWND	hWnd,
	UINT	message,
	TCHAR	key
) {
		// Deactivate the object
	if (((CImageObject FAR *) NULL) != m_lpObj)
		m_lpObj->InPlaceDeactivate ();

	return (0);
}

//
//		CImageDocument::HandleHatchResize
//
//	We've been re-sized via the OLE hatch window.  Update the
//	size info and tell OLE and Domino and Notes we've been changed.
//

long CImageDocument::HandleHatchResize (
	HWND		hWnd,
	UINT		message,
	WORD		sizeType,
	LONG		width,
	LONG		height
) {
	m_lpObj->ResizeObject (width, height);

	this->UpdateSizes (width, height);

	return (0);
}

//
//		CImageDocument::ShowDocWindow
//
//	Display the document window.
//

void CImageDocument::ShowDocWindow (
	int nCmdShow
) {
	::ShowWindow (m_hDocWnd, nCmdShow);
	::UpdateWindow (m_hDocWnd);
}

//
//		CImageDocument::HideDocWindow
//
//	Hide the document window.
//

void CImageDocument::HideDocWindow (
	void
) {
	::ShowWindow (m_hDocWnd, SW_HIDE);
}

//
//		CImageDocument::ResizeDocWindow
//
//	Move the document window.
//

long CImageDocument::ResizeDocWindow (
	RECT &		newArea
) {
		// Change our window
	::MoveWindow (m_hDocWnd, newArea.left, newArea.top,
		newArea.right, newArea.bottom, TRUE);

	return (WIN_MSG_HANDLED);
}

//
//		CImageDocument::GetOleInPlaceFramePtr
//
//	Get the pointer to the OLE in-place frame from the OLE object
//

LPOLEINPLACEFRAME CImageDocument::GetOleInPlaceFramePtr (
	void
) const {
	if (((CImageObject FAR *) NULL) == m_lpObj)
		return ((LPOLEINPLACEFRAME) NULL);
	else
		return (m_lpObj->GetOleInPlaceFramePtr ());
}

//
//		CImageDocument::GetOleInPlaceFrameInfoPtr
//
//	Get the pointer to the OLE in-place frame info from the document object
//

LPOLEINPLACEFRAMEINFO CImageDocument::GetOleInPlaceFrameInfoPtr (
	void
) const {
	if (((CImageObject FAR *) NULL) == m_lpObj)
		return ((LPOLEINPLACEFRAMEINFO) NULL);
	else
		return (m_lpObj->GetOleInPlaceFrameInfoPtr ());
}

//
//		CImageDocument::IsInPlaceActive
//
//	Return TRUE if we're In-Place Active.
//

BOOL CImageDocument::IsInPlaceActive (
	void
) {
	if (((CImageObject FAR *) NULL) != m_lpObj)
		return (m_lpObj->IsInPlaceActive ());
	else
		return (FALSE);
}

//
//		CImageDocument::GetForegroundColor
//
//	Fetch the foreground color from the pattern object.
//

COLORREF CImageDocument::GetForegroundColor (
	void
) const {
	return (m_lpPattern->GetForegroundColor ());
}

//
//		CImageDocument::GetBackgroundColor
//
//	Fetch the background color from the pattern object.
//

COLORREF CImageDocument::GetBackgroundColor (
	void
) const {
	return (m_lpPattern->GetBackgroundColor ());
}

//
//		CImageDocument::SetForegroundColor
//
//	Update the foreground color in the pattern object.
//

void CImageDocument::SetForegroundColor (
	COLORREF	color
) {
	m_lpPattern->SetForegroundColor (color);
}

//
//		CImageDocument::SetBackgroundColor
//
//	Update the background color in the pattern object.
//

void CImageDocument::SetBackgroundColor (
	COLORREF	color
) {
	m_lpPattern->SetBackgroundColor (color);
}

	// Store new image offsets
void CImageDocument::SetOffsets (
	int		xOffset,
	int		yOffset
) {
	m_CurOffset.cx = xOffset;
	m_CurOffset.cy = yOffset;
}

	// Store new image size
void CImageDocument::SetSizes (
	int		xSize,
	int		ySize
) {
	m_CurSize.cx = xSize;
	m_CurSize.cy = ySize;
}

//
//		IUnknown methods
//

	// QueryInterface
STDMETHODIMP CImageDocument::QueryInterface (
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
STDMETHODIMP_ (ULONG) CImageDocument::AddRef (
	void
) {
		// Increment and return the reference count
	return (++m_nRefCount);
}

	// Release
STDMETHODIMP_ (ULONG) CImageDocument::Release (
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

	// OLE object creation
HRESULT CImageDocument::CreateObject (
	REFIID			riid,
	LPVOID FAR *	ppvObject
) {
	m_lpObj = new CImageObject (this, m_lpApp);

	if (((CImageObject FAR *) NULL) != m_lpObj)
	{
			// The document has a pointer to the object
		m_lpObj->AddRef ();

			// Get the interface for the caller
		return (m_lpObj->QueryInterface (riid, ppvObject));
	}

	return (ResultFromScode (S_OK));
}

	// Tell the Image object to clean up
HRESULT CImageDocument::Close (
	void
) {
	HRESULT		answer = ResultFromScode (S_OK);

	if (m_fIsClosing)
		return (answer);

	m_fIsClosing = TRUE;

	if (((CImageObject FAR *) NULL) != m_lpObj)
	{
		answer = m_lpObj->Close (OLECLOSE_SAVEIFDIRTY);
	}

	return (answer);
}

	// We're being closed from the OLE object - clean up *this
HRESULT CImageDocument::CloseOleObject (
	void
) {
	if (m_fIsClosing)
		return (ResultFromScode (S_OK));

	m_fIsClosing = TRUE;

	if (((CImageObject FAR *) NULL) != m_lpObj)
	{
		m_lpObj->Release ();
		m_lpObj = (CImageObject FAR *) NULL;
	}

	return (ResultFromScode (S_OK));
}

	// Tell the container when the data changes
void CImageDocument::DataChanged (
	void
) {
	if (((CImageObject FAR *) NULL) != m_lpObj)
		m_lpObj->SendOnDataChange ();
}

	// Is the data up-to-date?
BOOL CImageDocument::IsDirty (
	void
) {
	return (FALSE);
}

// NotesFlow code

//
//		NotesFlow support
//

	// Fetch the NotesDocInfo clipboard format
UINT CImageDocument::GetNotesDocInfoClipFormat (
	void
) const {
	return (m_NotesManager.GetNotesDocInfoClipFormat ());
}

	// Fetch the NoteshNote clipboard format
UINT CImageDocument::GetNoteshNoteClipFormat (
	void
) const {
	return (m_NotesManager.GetNoteshNoteClipFormat ());
}

	// Fetch the NotesDocAction clipboard format
UINT CImageDocument::GetNotesDocActionClipFormat (
	void
) const {
	return (m_NotesManager.GetNotesDocActionClipFormat ());
}

	// Fetch the NotesDocActionDone clipboard format
UINT CImageDocument::GetNotesDocActionDoneClipFormat (
	void
) const {
	return (m_NotesManager.GetNotesDocActionDoneClipFormat ());
}

	// Handle the NotesDocInfo SetData() request
HRESULT CImageDocument::ProcessNotesDocInfo (
	LPSTGMEDIUM	pStgMedium
) {
	HRESULT		hResult;

		// The REAL work is done by the NotesFlow Manager class
	hResult = m_NotesManager.ProcessNotesDocInfo (this, pStgMedium);
	if (SUCCEEDED (hResult))
	{
			// Tell Domino and Notes the metafile needs to be redrawn
		this->DataChanged ();
	}

	return (hResult);
}

	// Handle the NotesDocActionDone SetData() request
HRESULT CImageDocument::ProcessNotesDocActionDone (
	LPSTGMEDIUM	pStgMedium
) {
		// The REAL work is done by the NotesFlow Manager class
	return (m_NotesManager.ProcessNotesDocActionDone (this, pStgMedium));
}

	// Handle the NoteshNote GetData() request
HRESULT CImageDocument::ProcessNoteshNote (
	LPSTGMEDIUM	pStgMedium
) {
		// The REAL work is done by the NotesFlow Manager class
	return (m_NotesManager.ProcessNoteshNote (this, pStgMedium));
}

	// Handle the NotesDocAction GetData() request
HRESULT CImageDocument::ProcessNotesDocAction (
	LPSTGMEDIUM	pStgMedium
) {
		// If there's no active note, there's nothing to do
	if (!m_NotesManager.IsNoteActive ())
		return (ResultFromScode (E_FAIL));

		// The REAL work is done by the NotesFlow Manager class
	return (m_NotesManager.ProcessNotesDocAction (this, pStgMedium));
}

	// Write new color information back to Domino and Notes
void CImageDocument::UpdateForegroundColor (
	COLORREF	newColor
) {
	this->SetForegroundColor (newColor);
	if (m_NotesManager.IsNoteActive ())
	{
		m_NotesManager.SetDataUpdatePending ();
		this->DataChanged ();
	}
}

	// Write new color information back to Domino and Notes
void CImageDocument::UpdateBackgroundColor (
	COLORREF	newColor
) {
	this->SetBackgroundColor (newColor);
	if (m_NotesManager.IsNoteActive ())
	{
		m_NotesManager.SetDataUpdatePending ();
		this->DataChanged ();
	}
}

	// Write new size information back to Domino and Notes
void CImageDocument::UpdateSizes (
	LONG		newWidth,
	LONG		newHeight
) {
	this->SetSizes (newWidth, newHeight);
	if (m_NotesManager.IsNoteActive ())
	{
		m_NotesManager.SetDataUpdatePending ();
		this->DataChanged ();
	}
}

//
//		NotesFlow Actions menu support
//

	// If there are actions to display, create the Actions menu.
	// Returns NULL if there are no actions available.
HMENU CImageDocument::CreateActionMenu (
	void
) {
		// Fetch a pointer to the action set
	CNotesFlowActionSet FAR * lpActionSet = m_NotesManager.GetActionSetPtr ();

	if (((CNotesFlowActionSet FAR *) NULL) == lpActionSet)
		return ((HMENU) NULL);
	else
		return (lpActionSet->CreateActionMenu ());
}

	// Return the handle of the Actions menu
HMENU CImageDocument::GetActionMenuHandle (
	void
) {
		// Fetch a pointer to the action set
	CNotesFlowActionSet FAR * lpActionSet = m_NotesManager.GetActionSetPtr ();

	if (((CNotesFlowActionSet FAR *) NULL) == lpActionSet)
		return ((HMENU) NULL);
	else
		return (lpActionSet->GetActionMenuHandle ());
}

	// Delete the Actions menu
void CImageDocument::DeleteActionMenu (
	void
) {
		// Fetch a pointer to the action set
	CNotesFlowActionSet FAR * lpActionSet = m_NotesManager.GetActionSetPtr ();

	if (((CNotesFlowActionSet FAR *) NULL) != lpActionSet)
		lpActionSet->DeleteActionMenu ();
}

	// Look up the Action ID for a menu selection
	// *lpActionID is unchanged if no matching action is found
LRESULT CImageDocument::CheckForNotesFlowAction (
	LONG		menuCode
) {
	int			actionID;			// ID code of NotesFlow action

		// If there's no NotesFlow object, there's nothing to do
	if (((CImageObject FAR *) NULL) == m_lpObj)
		return (WIN_MSG_NOT_HANDLED);

		// Fetch a pointer to the action set
	CNotesFlowActionSet FAR * lpActionSet = m_NotesManager.GetActionSetPtr ();

	if (((CNotesFlowActionSet FAR *) NULL) == lpActionSet)
		return (WIN_MSG_NOT_HANDLED);

	if ((lpActionSet->LookupActionID (menuCode, &actionID))
		&& (m_NotesManager.IsNoteActive ()))
	{
			// User selected an action - notify Domino and Notes!
		m_NotesManager.SetActionPending (actionID, 0);	// No action flags currently supported
		this->DataChanged ();

		return (WIN_MSG_HANDLED);
	}
	else
		return (WIN_MSG_NOT_HANDLED);
}

// End NotesFlow code
