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
//		nf_obj.cpp
//
//	Implementation of CImageObject
//

	// Standard C headers
#include <stdio.h>

	// Windows headers
#include <windows.h>
#include <ole2.h>
#include <ole2ver.h>

	// Domino and Notes header files
#include <global.h>
#include <olenotes.h>

	// Application headers
#include "notesflo.h"
#include "ln_buff.hpp"
#include "ln_flow.hpp"
#include "nf_image.hpp"
#include "nf_app.hpp"
#include "nf_doc.hpp"
#include "oleobj.hpp"
#include "oleps.hpp"
#include "oledo.hpp"
#include "oleipo.hpp"
#include "oleipao.hpp"
#include "oleec.hpp"
#include "nf_hatch.hpp"
#include "nf_obj.hpp"

//
//		Class management
//

	// Constructor
CImageObject::CImageObject (
	CImageDocument FAR *lpDoc,
	CImageApplication FAR *lpApp
) 
	: m_lpDoc (lpDoc),
	m_lpApp (lpApp),
	m_nRefCount (0),
	m_fInPlaceActive (FALSE),
	m_fInPlaceVisible (FALSE),
	m_fUIActive (FALSE),
        m_hSharedMenu ((HMENU) NULL),
	m_hOleMenuDesc ((HOLEMENU) NULL),
	m_dwRegisterTableID (0),
	m_hParentWindow ((HWND) NULL),
	m_lpOleClientSite ((LPOLECLIENTSITE) NULL),
	m_lpOleAdviseHolder ((LPOLEADVISEHOLDER) NULL),
	m_lpDataAdviseHolder ((LPDATAADVISEHOLDER) NULL),
	m_lpOleInPlaceFrame ((LPOLEINPLACEFRAME) NULL),
	m_lpOleInPlaceUIWindow ((LPOLEINPLACEUIWINDOW) NULL),
	m_lpOleInPlaceSite ((LPOLEINPLACESITE) NULL)
{
		// Note:  The objects for the application, document, and
		// hatch windows are created at window creation time, which
		// occurs before CClassFactory creates this OLE object!
	m_lpApp->AddRef ();

	m_lpHatch = m_lpApp->GetHatchWindowPtr ();
	m_lpHatch->AddRef ();

	m_OleObject.SetObjectPtr (this);
	m_PersistStorage.SetObjectPtr (this);
	m_DataObject.SetObjectPtr (this);
	m_OleInPlaceObject.SetObjectPtr (this);
	m_OleInPlaceActiveObject.SetObjectPtr (this);
	m_ExternalConnection.SetObjectPtr  (this);

	m_PosRect.top = 0;
	m_PosRect.left = 0;
	m_PosRect.bottom = 81;	// Default size is 81 x 81 pixels
	m_PosRect.right = 81;

	m_FrameInfo.cb = sizeof (OLEINPLACEFRAMEINFO);
	m_FrameInfo.fMDIApp = FALSE;
	m_FrameInfo.hwndFrame = (HWND) NULL;
	m_FrameInfo.haccel = (HACCEL) NULL;
	m_FrameInfo.cAccelEntries = 0;
}

	// Destructor
CImageObject::~CImageObject (
	void
) {
		// Clean up any remaining references
	if (((CHatchWindow FAR *) NULL) != m_lpHatch)
	{
		m_lpHatch->Release ();
		m_lpHatch = (CHatchWindow FAR *) NULL;
	}

	if (((CImageApplication FAR *) NULL) != m_lpApp)
	{
		m_lpApp->Release ();
		m_lpApp = (CImageApplication FAR *) NULL;
	}
}

//
//		IUnknown methods
//

	// QueryInterface
STDMETHODIMP CImageObject::QueryInterface (
	REFIID			riid,
	LPVOID FAR *	ppvObj
) {
	SCODE			sc = S_OK;

	if (IID_IUnknown == riid)
		*ppvObj = (LPVOID) this;
	else if (IID_IOleObject == riid)
		*ppvObj = (LPVOID) &m_OleObject;
	else if (IID_IDataObject == riid)
		*ppvObj = (LPVOID) &m_DataObject;
	else if ((IID_IPersistStorage == riid) || (IID_IPersist == riid))
		*ppvObj = (LPVOID) &m_PersistStorage;
	else if (IID_IOleInPlaceObject == riid)
		*ppvObj = (LPVOID) &m_OleInPlaceObject;
	else if (IID_IOleInPlaceActiveObject == riid)
		*ppvObj = (LPVOID) &m_OleInPlaceActiveObject;
	else if (IID_IExternalConnection == riid)
		*ppvObj = (LPVOID) &m_ExternalConnection;
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
STDMETHODIMP_ (ULONG) CImageObject::AddRef (
	void
) {
		// Increment and return the reference count
	return (++m_nRefCount);
}

	// Release
STDMETHODIMP_ (ULONG) CImageObject::Release (
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

	// Switch to a new OleClientSite object
HRESULT CImageObject::SetOleClientSite (
	LPOLECLIENTSITE	lpClientSite
) {
		// If we already have a client site, release it
	if (((LPOLECLIENTSITE) NULL) != m_lpOleClientSite)
	{
		m_lpOleClientSite->Release ();
		m_lpOleClientSite = (LPOLECLIENTSITE) NULL;
	}

		// Store the pointer to the new site interface
	m_lpOleClientSite = lpClientSite;

		// If there's something there, we need to AddRef()!!
	if (((LPOLECLIENTSITE) NULL) != m_lpOleClientSite)
	{
		m_lpOleClientSite->AddRef ();
	}

	return (ResultFromScode (S_OK));
}

	// Store the new DataAdviseHolder object pointer
HRESULT CImageObject::SetDataAdviseHolderPtr (
	LPDATAADVISEHOLDER	lpDataAdviseHolder
) {
		// If we already have an advise holder, release it
	if (((LPDATAADVISEHOLDER) NULL) != m_lpDataAdviseHolder)
	{
		m_lpDataAdviseHolder->Release ();
		m_lpDataAdviseHolder = (LPDATAADVISEHOLDER) NULL;
	}

		// Store the pointer to the new site interface
	m_lpDataAdviseHolder = lpDataAdviseHolder;

		// If there's something there, we need to AddRef()!!
	if (((LPDATAADVISEHOLDER) NULL) != m_lpDataAdviseHolder)
	{
		m_lpDataAdviseHolder->AddRef ();
	}

	return (ResultFromScode (S_OK));
}

	// Obtain the clipboard format for this object
UINT CImageObject::GetObjectClipboardFormat (
	void
) const {
	if (((CImageDocument FAR *) NULL) == m_lpDoc)
		return (0);
	else
		return (m_lpDoc->GetDocumentClipboardFormat ());
}

	// Get the OLE user type string
void CImageObject::GetUserType (
	LPOLESTR *	lplpTypeBuf
) {
	CLSID		classID;

	this->GetClassID (&classID);

		// Try to get the official user type from the registry
	if (FAILED(::OleRegGetUserType (classID, USERCLASSTYPE_FULL, lplpTypeBuf)))
	{

		*lplpTypeBuf = (LPOLESTR) ::CoTaskMemAlloc (MAX_PATH * sizeof (OLECHAR));
		if (((LPOLESTR) NULL) != (*lplpTypeBuf))
			wcscpy ((*lplpTypeBuf), lpAppOleTitle);
	}
}

	// Get the window handle for the application window
HWND CImageObject::GetAppWindowHandle (
	void
) const {
	if (((CImageApplication FAR *) NULL) == m_lpApp)
		return ((HWND) NULL);
	else
		return (m_lpApp->GetAppWindowHandle ());
}

	// Get the window handle for the document window
HWND CImageObject::GetDocWindowHandle (
	void
) const {
	if (((CImageDocument FAR *) NULL) == m_lpDoc)
		return ((HWND) NULL);
	else
		return (m_lpDoc->GetDocWindowHandle ());
}

	// Get the current image size
void CImageObject::GetSize (
	LPSIZE		lpSize
) const {
	lpSize->cx = m_lpDoc->GetXSize ();
	lpSize->cy = m_lpDoc->GetYSize ();
}

	// Store a new size for the image
void CImageObject::SetSize (
	LPSIZE		lpNewSize
) {
	m_lpDoc->UpdateSizes (lpNewSize->cx, lpNewSize->cy);
}

	// Set up an Advise connection
HRESULT CImageObject::CreateAdvise (
	LPADVISESINK	lpAdvSink,
	LPDWORD			lpdwConnection
) {
		// If we don't have an OleAdviseHolder yet, make one
	if (((LPOLEADVISEHOLDER) NULL) == m_lpOleAdviseHolder)
		::CreateOleAdviseHolder (&m_lpOleAdviseHolder);

		// Now have the OleAdviseHolder to the REAL work!
	return (m_lpOleAdviseHolder->Advise (lpAdvSink, lpdwConnection));
}

	// We've been told we're done with the advise connection
HRESULT CImageObject::Unadvise (
	DWORD		dwConnection
) {
	if (((LPOLEADVISEHOLDER) NULL) != m_lpOleAdviseHolder)
		return (m_lpOleAdviseHolder->Unadvise (dwConnection));
	else
		return (ResultFromScode (E_FAIL));
}

	// Return an enumerator for outstanding advise connections
HRESULT CImageObject::EnumAdvise (
	LPENUMSTATDATA FAR *lplpEnumAdvise
) {
		// Make sure the answer is NULL if something goes wrong
	*lplpEnumAdvise = (LPENUMSTATDATA) NULL;

		// Let the OleAdviseHolder do it for us
	return (m_lpOleAdviseHolder->EnumAdvise (lplpEnumAdvise));
}

	// Get the class ID for this server
void CImageObject::GetClassID (
	CLSID FAR *	lpClsId
) {
		// Just hand back our GUID
	*lpClsId = GUID_NOTESFLOW;
}

	// Tell the document to close
HRESULT CImageObject::Close (
	DWORD		dwSaveOption
) {
	HRESULT		answer;
	HRESULT		answer2;
	BOOL		fSaveDoc = FALSE;

		// Check the save conditions
	if (OLECLOSE_SAVEIFDIRTY == dwSaveOption)
	{
		if (m_lpDoc->IsDirty ())
			fSaveDoc = TRUE;
	}
	else if (OLECLOSE_PROMPTSAVE == dwSaveOption)
	{
		int		answer;

		answer = ::MessageBox (
			m_lpDoc->GetDocWindowHandle (),
			"Do you want to save this image?",
			"Closing NotesFlow",
			MB_YESNOCANCEL | MB_ICONQUESTION);

		if (IDCANCEL == answer)
			return (OLE_E_PROMPTSAVECANCELLED);
		else if (IDYES == answer)
			fSaveDoc = TRUE;
	}
	else
		fSaveDoc = FALSE;

		// Increment the reference count so we stay alive until we're done
	this->AddRef ();

		// If the object is in-place active, deactivate
	if (m_fInPlaceActive)
	{
		this->InPlaceDeactivate ();
	}
	
		// Unregister from the Running Object Table
	if (0 != m_dwRegisterTableID)
	{
		LPRUNNINGOBJECTTABLE	lpRot;

		if (NOERROR == ::GetRunningObjectTable (0, &lpRot))
		{
			lpRot->Revoke (m_dwRegisterTableID);
			lpRot->Release ();
			m_dwRegisterTableID = 0L;
		}
	}

		// If there's a client site, ask it to save the object
	if (((LPOLECLIENTSITE) NULL) != m_lpOleClientSite)
	{
			// If we're supposed to save . . .
		if (fSaveDoc)
		{
				// . . . ask for a save operation!
			m_lpOleClientSite->SaveObject ();
		}

			// We're done with the client site window
		m_lpOleClientSite->OnShowWindow (FALSE);
	}

		// Send an OnDataChange for containers wanting ADVF_DATAONSTOP notification
	if (((LPDATAADVISEHOLDER) NULL) != m_lpDataAdviseHolder)
	{
		m_lpDataAdviseHolder->SendOnDataChange (
			GetDataObjectPtr (),
			0,
			ADVF_DATAONSTOP);
		m_lpDataAdviseHolder->Release ();
		m_lpDataAdviseHolder = (LPDATAADVISEHOLDER) NULL;
	}

		// Tell the container we're closing
	if (((LPOLEADVISEHOLDER) NULL) != m_lpOleAdviseHolder)
	{
		m_lpOleAdviseHolder->SendOnClose ();
		m_lpOleAdviseHolder->Release ();
		m_lpOleAdviseHolder = (LPOLEADVISEHOLDER) NULL;
	}

		// Release the client site
	if (((LPOLECLIENTSITE) NULL) != m_lpOleClientSite)
	{
		m_lpOleClientSite->Release ();
		m_lpOleClientSite = (LPOLECLIENTSITE) NULL;
	}

	answer = m_lpDoc->CloseOleObject ();

		// Disconnect the object in the event of a severe
		// OLE crisis
	answer2 = ::CoDisconnectObject ((LPUNKNOWN) this, 0);
	if (SUCCEEDED (answer))
		answer = answer2;

		// Tell the application to quit now
	::PostMessage (
		m_lpApp->GetAppWindowHandle (),
		WM_CLOSE,
		0,
		0L);

		// We're done - let the image object go away
	this->Release ();

	return (answer);
}

	// Create a buffer with the output metafile in it
DHANDLE CImageObject::GetMetafilePict (
	void
) {
		// Allocate the structure
	DHANDLE			hMFP;
	hMFP = (DHANDLE)::GlobalAlloc (GMEM_SHARE | GHND, sizeof (METAFILEPICT));
	if (((DHANDLE) NULL) == hMFP)
		return (hMFP);

		// Lock
	LPMETAFILEPICT	lpMFP = (LPMETAFILEPICT) ::GlobalLock ((HGLOBAL)hMFP);

		// Set mapping mode
	lpMFP->mm = MM_ANISOTROPIC;

		// Set dimensions (this being a clipboard
		// format, the dimensions are in HIMETRIC)
	lpMFP->xExt = ::XFormWidthPixelToHimetric (m_lpDoc->GetXSize ());
	lpMFP->yExt = ::XFormHeightPixelToHimetric (m_lpDoc->GetYSize ());

		// Create an in-memory metafile
	HDC hDc = ::CreateMetaFile (NULL);

		// Set origin and extent
	::SetWindowOrgEx (hDc, 0, 0, (LPPOINT) NULL);
	::SetWindowExtEx (hDc, m_lpDoc->GetXSize (),
		m_lpDoc->GetYSize (), (LPSIZE) NULL);

		// Create a rectangle corresponding to the metafile dimensions
	RECT	drawRect;

	drawRect.left = 0;
	drawRect.top = 0;
	drawRect.right = m_lpDoc->GetXSize ();
	drawRect.bottom = m_lpDoc->GetYSize ();

		// Stuff the platform-specific graphics info
		// into a CDrawEnvironment object.
		// When drawing into a metafile, the invalid rectangle is the
		// same as the client rectangle.
	CDrawEnvironment drawEnv (hDc, &drawRect, &drawRect, TRUE);

		// Draw into the image area
	m_lpDoc->DrawImage (drawEnv);

		// Close, unlock, and return
	lpMFP->hMF = ::CloseMetaFile (hDc);

	::GlobalUnlock ((HGLOBAL)hMFP);

	return (hMFP);
}

	// Connect to the UI
void CImageObject::AddFrameLevelUI (
	void
) {
		// Display the combined menu
	if (((LPOLEINPLACEFRAME) NULL) != m_lpOleInPlaceFrame)
		m_lpOleInPlaceFrame->SetMenu (
			m_hSharedMenu,
			m_hOleMenuDesc,
			m_lpDoc->GetDocWindowHandle ());

		// Set the parents for the hatch and document windows
	::SetParent (
		m_lpHatch->GetHatchWindowHandle (),
		m_hParentWindow);
	::SetParent (
		m_lpDoc->GetDocWindowHandle (),
		m_lpHatch->GetHatchWindowHandle ());

		// Set the border space;  we don't need room for a toolbar
	if (((LPOLEINPLACEFRAME) NULL) != m_lpOleInPlaceFrame)
		m_lpOleInPlaceFrame->SetBorderSpace ((LPCBORDERWIDTHS) NULL);
	if (((LPOLEINPLACEUIWINDOW) NULL) != m_lpOleInPlaceUIWindow)
		m_lpOleInPlaceUIWindow->SetBorderSpace ((LPCBORDERWIDTHS) NULL);
}

	// Close down the UI
void CImageObject::DeactivateUI (
	void
) {
		// If the UI is running . . .
	if ((m_fUIActive) && (((LPOLEINPLACEFRAME) NULL) != m_lpOleInPlaceFrame))
	{
			// . . . deactivate it!
		m_fUIActive = FALSE;

			// Restore the container's menu
		m_lpOleInPlaceFrame->SetMenu (NULL, NULL, NULL);

			// Set the parents for the hatch and document windows
		::SetParent (
			m_lpDoc->GetDocWindowHandle (),
			m_lpApp->GetAppWindowHandle ());
		::SetParent (
			m_lpHatch->GetHatchWindowHandle (),
			m_lpDoc->GetDocWindowHandle ());

			// If we're in an MDI container, call SetActiveObject ()
		if (((LPOLEINPLACEUIWINDOW) NULL) != m_lpOleInPlaceUIWindow)
			m_lpOleInPlaceUIWindow->SetActiveObject (
				(LPOLEINPLACEACTIVEOBJECT) NULL, (LPCOLESTR) NULL);

		m_lpOleInPlaceFrame->SetActiveObject (
			(LPOLEINPLACEACTIVEOBJECT) NULL, (LPCOLESTR) NULL);
			
			// Tell the container we're closing our UI
		if (((LPOLEINPLACESITE) NULL) != m_lpOleInPlaceSite)
		{
			m_lpOleInPlaceSite->OnUIDeactivate (FALSE);
		}
	}
}

	// Do the In Place activation!
BOOL CImageObject::DoInPlaceActivate (
	LONG		iVerb
) {
	RECT		posRect;
	RECT		clipRect;

		// If we're not currently In Place Active . . .
	if (!m_fInPlaceActive)
	{
			// Ask for the OleInPlaceSite interface
		if (FAILED (m_lpOleClientSite->QueryInterface (
			IID_IOleInPlaceSite, ((LPVOID FAR *) (&m_lpOleInPlaceSite)))))
		{
			return (FALSE);
		}

			// Make sure we have a site!
		if (((LPOLEINPLACESITE) NULL) == m_lpOleInPlaceSite)
			return (FALSE);

			// Can that interface support in-place activation?
		if (S_OK != m_lpOleInPlaceSite->CanInPlaceActivate ())
		{
				// No - clean up
			m_lpOleInPlaceSite->Release ();
			m_lpOleInPlaceSite = (LPOLEINPLACESITE) NULL;

			return (FALSE);
		}

			// We have a site - tell it we're activating
		m_lpOleInPlaceSite->OnInPlaceActivate ();
		m_fInPlaceActive = TRUE;
	}

		// If we're not currently In Place Visible . . .
	if (!m_fInPlaceVisible)
	{
		m_fInPlaceVisible = TRUE;

			// Get the window handle of the container
		m_lpOleInPlaceSite->GetWindow (&m_hParentWindow);

			// Get the window context
		m_lpOleInPlaceSite->GetWindowContext (
			&m_lpOleInPlaceFrame,
			&m_lpOleInPlaceUIWindow,
			&posRect,
			&clipRect,
			&m_FrameInfo);

			// Show the hatch window
		m_lpHatch->ShowHatchWindow ();

			// Set the parents for the hatch and document windows
		::SetParent (
			m_lpHatch->GetHatchWindowHandle (),
			m_hParentWindow);
		::SetParent (
			m_lpDoc->GetDocWindowHandle (),
			m_lpHatch->GetHatchWindowHandle ());

			// Tell the client site to show the object
		m_lpOleClientSite->ShowObject ();

			// Compute the display size we've been given
		RECT	actualArea;
		::IntersectRect (&actualArea, &posRect, &clipRect);
		m_PosRect = posRect;

			// Adjust the hatch window size
		SIZE	docOffset;
		m_lpHatch->SetHatchWindowSize (&actualArea, &clipRect, &docOffset);

			// Figure out where the document window goes,
			// relative to the hatch window
		::OffsetRect (&actualArea, docOffset.cx, docOffset.cy);

			// Position the document window
		m_lpDoc->SetDocWindow (&actualArea);

		this->AssembleMenus ();
	}

		// If the UI is not active . . .
	if (!m_fUIActive)
	{
		m_fUIActive = TRUE;

			// Tell the in-place site we are activating
		if (((LPOLEINPLACESITE) NULL) != m_lpOleInPlaceSite)
			m_lpOleInPlaceSite->OnUIActivate ();

			// Set the focus to our document window
		::SetFocus (m_lpDoc->GetDocWindowHandle ());

			// Give the frame the interface pointer to our COleInPlaceActiveObject
		if (((LPOLEINPLACEFRAME) NULL) != m_lpOleInPlaceFrame)
			m_lpOleInPlaceFrame->SetActiveObject (
				&m_OleInPlaceActiveObject,
				lpAppOleTitle);

			// If there's a document object, give it the interface too!
		if (((LPOLEINPLACEUIWINDOW) NULL) != m_lpOleInPlaceUIWindow)
			m_lpOleInPlaceUIWindow->SetActiveObject (
				&m_OleInPlaceActiveObject,
				lpAppOleTitle);

			// Add us to the frame's user interface
		this->AddFrameLevelUI ();
	}

	return (TRUE);
}

	// Hide ourselves while in-place active
void CImageObject::DoInPlaceHide (
	void
) {
		// If we're not visible, there's nothing to hide!
	if (!m_fInPlaceVisible)
		return;

	m_fInPlaceVisible = FALSE;

		// Set the parents for the hatch and document windows
	::SetParent (
		m_lpDoc->GetDocWindowHandle (),
		m_lpApp->GetAppWindowHandle ());
	::SetParent (
		m_lpHatch->GetHatchWindowHandle (),
		m_lpDoc->GetDocWindowHandle ());

		// Take apart the combined menus
	this->DisassembleMenus ();

		// Release the in-place frame
	if (((LPOLEINPLACEFRAME) NULL) != m_lpOleInPlaceFrame)
	{
		m_lpOleInPlaceFrame->Release ();
		m_lpOleInPlaceFrame = (LPOLEINPLACEFRAME) NULL;
	}

		// Release the UI window
	if (((LPOLEINPLACEUIWINDOW) NULL) != m_lpOleInPlaceUIWindow)
	{
		m_lpOleInPlaceUIWindow->Release ();
		m_lpOleInPlaceUIWindow = (LPOLEINPLACEUIWINDOW) NULL;
	}

}

	// Pass the new image offsets to the document
void CImageObject::SetImageOffsets (
	int			xOff,
	int			yOff
) {
	m_lpDoc->SetOffsets (xOff, yOff);
}

	// Store the object's position
void CImageObject::SetPosition (
	LPCRECT		lpPosRect
) {
	m_PosRect = *lpPosRect;
}

	// Tell the document to hide the application window
void CImageObject::HideAppWindow (
	void
) {
	if (((CImageApplication FAR *) NULL) != m_lpApp)
		m_lpApp->HideAppWindow ();
}

	// Deactivate through the OleInPlaceObject
HRESULT CImageObject::InPlaceDeactivate (
	void
) {
		// If we're not In Place Active, return NOERROR
	if (!this->IsInPlaceActive ())
		return (NOERROR);

		// Clear the In Place Active flag
	this->SetInPlaceActive (FALSE);

		// Deactivate the UI
	this->DeactivateUI ();
	this->DoInPlaceHide ();

		// Tell the container we're deactivating
	if (((LPOLEINPLACESITE) NULL) != m_lpOleInPlaceSite)
	{
		m_lpOleInPlaceSite->OnInPlaceDeactivate ();
		m_lpOleInPlaceSite->Release ();
		m_lpOleInPlaceSite = (LPOLEINPLACESITE) NULL;
	}

	return (ResultFromScode (S_OK));
}

	// Open the object into a separate window
BOOL CImageObject::OpenForEdit (
	LPOLECLIENTSITE lpSite
) {
	if (((LPOLECLIENTSITE) NULL) != m_lpOleClientSite)
	{
		m_lpOleClientSite->ShowObject ();
		m_lpOleClientSite->OnShowWindow (TRUE);
	}

	m_lpDoc->ShowDocWindow ();
	m_lpHatch->HideHatchWindow ();
	m_lpApp->ShowAppWindow ();
	::SetFocus (m_lpApp->GetAppWindowHandle ());

	return (TRUE);
}

	// Send a data change to the advise holder and update the running object table
void CImageObject::SendOnDataChange (
	void
) {
	if (((LPDATAADVISEHOLDER) NULL) != m_lpDataAdviseHolder)
		m_lpDataAdviseHolder->SendOnDataChange (
			&m_DataObject, 0, 0);

	LPRUNNINGOBJECTTABLE	lpRot;

	::GetRunningObjectTable (0, &lpRot);

	if ((0 != m_dwRegisterTableID) && (((LPRUNNINGOBJECTTABLE) NULL) != lpRot))
	{
		FILETIME	ft;

		CoFileTimeNow (&ft);

		lpRot->NoteChangeTime (m_dwRegisterTableID, &ft);
		lpRot->Release ();
	}
}

	// Assemble the combined application menus
void CImageObject::AssembleMenus (
	void
) {
		// Clean up anything left around
	if (((HMENU) NULL) != m_hSharedMenu)
	{
		::DestroyMenu (m_hSharedMenu);
		m_hSharedMenu = (HMENU) NULL;
	}

		// If there is, indeed, a container . . .
	if (((LPOLEINPLACEFRAME) NULL) != m_lpOleInPlaceFrame)
	{
			// Create the menu resource
		m_hSharedMenu = ::CreateMenu ();

			// Let the container build its' menus
		if (SUCCEEDED (m_lpOleInPlaceFrame->InsertMenus (
			m_hSharedMenu, &m_oleMenuGroups)))
		{
				// Skip the file group
			LONG	menuPos = m_oleMenuGroups.width[0];
			HMENU	hActionMenu = (HMENU) NULL;

				// Nothing to add to the edit group
			m_oleMenuGroups.width[1] = 0;

				// Skip the container group
			menuPos += m_oleMenuGroups.width[2];

				// Add our Color menu to the object group
			::InsertMenu (
				m_hSharedMenu,
				menuPos++,
				MF_BYPOSITION | MF_POPUP | MF_STRING,
				(UINT) m_lpApp->GetColorMenuHandle (),
				"&Color");

			m_oleMenuGroups.width[3] = 1;

// NotesFlow code

				// Do we have an Action menu?
				// (CreateActionMenu() returns NULL if there are no actions)
			if (((HMENU) NULL) != (hActionMenu = m_lpDoc->CreateActionMenu ()))
			{
					// Yes - add the Action menu to the object group
				::InsertMenu (
					m_hSharedMenu,
					menuPos++,
					MF_BYPOSITION | MF_POPUP | MF_STRING,
					(UINT) hActionMenu,
					"&Actions");

				m_oleMenuGroups.width[3]++;
			}

// End NotesFlow code

				// Skip the window group
			menuPos += m_oleMenuGroups.width[4];

				// Add our Help menu to the help group
			::InsertMenu (
				m_hSharedMenu,
				menuPos++,
				MF_BYPOSITION | MF_POPUP | MF_STRING,
				(UINT) m_lpApp->GetHelpMenuHandle (),
				"&Help");

			m_oleMenuGroups.width[5] = 1;
		}
		else
		{
			::DestroyMenu (m_hSharedMenu);
			m_hSharedMenu = (HMENU) NULL;
		}

			// Tell OLE to create the menu descriptor
		m_hOleMenuDesc = ::OleCreateMenuDescriptor (
			m_hSharedMenu, &m_oleMenuGroups);
	}
}

	// Disassemble the combined application menus
void CImageObject::DisassembleMenus (
	void
) {
	int			index;
	int			curMenu = 0;

		// Destroy any OLE menu descriptor
	::OleDestroyMenuDescriptor (m_hOleMenuDesc);

		// Is there a shared menu?
	if (((HMENU) NULL) != m_hSharedMenu)
	{
			// Remove our menus

			// Get the total number of menus
		for (index = 0; index < 6; index++)
			curMenu += m_oleMenuGroups.width [index];

			// Remove entries in the help group
		for (
			;
			(m_oleMenuGroups.width [5] > 0) && (curMenu > 0);
			m_oleMenuGroups.width [5]--)
		{
			::RemoveMenu (m_hSharedMenu, curMenu, MF_BYPOSITION);
			curMenu--;
		}

			// Skip entries in the window group
		curMenu -= m_oleMenuGroups.width [4];

			// Remove entries in the object group
		for (
			;
			(m_oleMenuGroups.width [3] > 0) && (curMenu > 0);
			m_oleMenuGroups.width [3]--)
		{
			::RemoveMenu (m_hSharedMenu, curMenu, MF_BYPOSITION);
			curMenu--;
		}

			// Skip entries in the container group
		curMenu -= m_oleMenuGroups.width [2];

			// Remove entries in the edit group
		for (
			;
			(m_oleMenuGroups.width [1] > 0) && (curMenu > 0);
			m_oleMenuGroups.width [1]--)
		{
			::RemoveMenu (m_hSharedMenu, curMenu, MF_BYPOSITION);
			curMenu--;
		}

			// We'll let the container worry about the file group!

			// Have the container remove its' menus
		m_lpOleInPlaceFrame->RemoveMenus (m_hSharedMenu);

// NotesFlow code

			// Delete any Actions menu
		m_lpDoc->DeleteActionMenu ();

// End NotesFlow code

			// We're done with the menu
		::DestroyMenu (m_hSharedMenu);

		m_hSharedMenu = (HMENU) NULL;
	}
}

	// Set new size for the hatch window
void CImageObject::SetHatchWindowSize (
	LPCRECT		lpPosRect,
	LPCRECT		lpClipRect,
	LPSIZE		lpOffset
) {
	if (((CHatchWindow FAR *) NULL) != m_lpHatch)
		m_lpHatch->SetHatchWindowSize (lpPosRect, lpClipRect, lpOffset);
}

	// Tell the container we've been resized
void CImageObject::ResizeObject (
	LONG		newWidth,
	LONG		newHeight
) {
		// Is there an OleInPlaceSite?
	if (((LPOLEINPLACESITE) NULL) != m_lpOleInPlaceSite)
	{
		RECT		newRect;

			// Yup - tell the container we want a new size
		newRect.left = m_PosRect.left;
		newRect.top = m_PosRect.top;
		newRect.right = m_PosRect.left + newWidth;
		newRect.bottom = m_PosRect.top + newHeight;

		m_lpOleInPlaceSite->OnPosRectChange (&newRect);
	}
}

	// Is the data up-to-date?
BOOL CImageObject::IsDirty (
	void
) {
	if (((CImageDocument FAR *) NULL) == m_lpDoc)
		return (FALSE);
	else
		return (m_lpDoc->IsDirty ());
}
