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
*		oleobj.cpp
*
*	Implementation of COleObject
*/

	// Windows header files
#include <windows.h>
#include <ole2.h>
#include <ole2ver.h>

	// Domino and Notes header files
#include <global.h>
#include <olenotes.h>

	// Application header files
#include "notesflo.h"
#include "oleobj.hpp"
#include "oleps.hpp"
#include "oledo.hpp"
#include "oleipo.hpp"
#include "oleipao.hpp"
#include "oleec.hpp"
#include "nf_obj.hpp"

//
//		IUnknown methods
//

STDMETHODIMP COleObject::QueryInterface (
	REFIID			riid,
	LPVOID FAR *	ppvObj
) {
		// Let the containing CImageObject handle this
	return (m_lpObj->QueryInterface (riid, ppvObj));
}

STDMETHODIMP_ (ULONG) COleObject::AddRef (
	void
) {
		// Increment the local counter
	++m_nRefCount;

		// A reference to *this is a reference to the containing CImageObject!
	return (m_lpObj->AddRef ());
}

STDMETHODIMP_ (ULONG) COleObject::Release (
	void
) {
	--m_nRefCount;

		// A reference to *this is a reference to the containing CImageObject!
	return (m_lpObj->Release ());
}

//
//		IOleObject methods
//

	// Store the client site in the CImageObject object
STDMETHODIMP COleObject::SetClientSite (
	LPOLECLIENTSITE lpClientSite
) {
		// Store the new client site pointer in the CImageObject
	return (m_lpObj->SetOleClientSite (lpClientSite));
}

	// Get the client site pointer if the caller needs it again!
STDMETHODIMP COleObject::GetClientSite (
	LPOLECLIENTSITE FAR *ppClientSite
) {
	if (((LPOLECLIENTSITE FAR *) NULL) == ppClientSite)
		return (ResultFromScode (E_INVALIDARG));

	*ppClientSite = m_lpObj->GetOleClientSite ();

	return (ResultFromScode (S_OK));
}

	// Set strings for Window titles - nothing needed in this server!
STDMETHODIMP COleObject::SetHostNames (
	LPCOLESTR	lpszContainerApp,
	LPCOLESTR	lpszContainerObj
) {
	return (ResultFromScode (S_OK));
}

	// The container wants us to close
STDMETHODIMP COleObject::Close (
	DWORD		dwSaveOption
) {
		// Tell the CImageObject to close everything
	return (m_lpObj->Close (dwSaveOption));
}

	// Set the object's moniker
STDMETHODIMP COleObject::SetMoniker (
	DWORD		dwWhichMoniker,
	LPMONIKER	lpMoniker
) {
	LPMONIKER	lpClientMoniker;
	LPOLECLIENTSITE	lpClientSite = m_lpObj->GetOleClientSite ();

		// Quit if there's no OLE client site
	if (((LPOLECLIENTSITE) NULL) == lpClientSite)
		return (ResultFromScode (E_FAIL));

		// Get the moniker from the client
	if (FAILED (lpClientSite->GetMoniker (
		OLEGETMONIKER_ONLYIFTHERE,
		OLEWHICHMK_OBJFULL,
		&lpClientMoniker)))
	{
		return (ResultFromScode (E_FAIL));
	}

		// If there's an advise holder, tell it we're being renamed
	LPOLEADVISEHOLDER	lpAdviseHolder = m_lpObj->GetOleAdviseHolder ();
	if (((LPOLEADVISEHOLDER) NULL) != lpAdviseHolder)
		lpAdviseHolder->SendOnRename (lpClientMoniker);

	LPRUNNINGOBJECTTABLE	lpRot;

	if (SUCCEEDED (::GetRunningObjectTable (0, &lpRot)))
	{
		int			refCount;
		DWORD		RegisterCookie = m_lpObj->GetRegisterTableID ();

		if (0 != RegisterCookie)
			lpRot->Revoke (RegisterCookie);

		lpRot->Register (0, m_lpObj, lpClientMoniker, &RegisterCookie);

		m_lpObj->SetRegisterTableID (RegisterCookie);

		refCount = lpRot->Release ();
	}

	return (ResultFromScode (S_OK));
}

	// Hand the current moniker to the caller
STDMETHODIMP COleObject::GetMoniker (
	DWORD		dwAssign,
	DWORD		dwWhichMoniker,
	LPMONIKER FAR *ppMoniker
) {
	LPOLECLIENTSITE	lpClientSite = m_lpObj->GetOleClientSite ();

		// Make sure the result pointer is NULL!
	*ppMoniker = (LPMONIKER) NULL;

		// Quit if there's no OLE client site
	if (((LPOLECLIENTSITE) NULL) != lpClientSite)
	{
		return (lpClientSite->GetMoniker (
			OLEGETMONIKER_ONLYIFTHERE,
			OLEWHICHMK_OBJFULL,
			ppMoniker));
	}
	else
		return (ResultFromScode (E_FAIL));
}

	// Initialize from the passed IDataObject
	// Not implemented for this server
STDMETHODIMP COleObject::InitFromData (
	LPDATAOBJECT	lpDataObject,
	BOOL			fCreation,
	DWORD			dwReserved
) {
	return (ResultFromScode (S_FALSE));
}

	// Return an IDataObject that's equivalent to a clipboard entry
	// Not implemented for this server
STDMETHODIMP COleObject::GetClipboardData (
	DWORD		dwReserved,
	LPDATAOBJECT FAR *ppDataObject
) {
	*ppDataObject = (LPDATAOBJECT) NULL;
	return (ResultFromScode (E_NOTIMPL));
}

#define VERB_OPEN	1

	// Container's asking us to do something
STDMETHODIMP COleObject::DoVerb (
	LONG			iVerb,
	LPMSG			lpMsg,
	LPOLECLIENTSITE	lpActiveSite,
	LONG			lindex,
	HWND			hwndParent,
	LPCRECT			lpPosRect
) {
	switch (iVerb)
	{
		case OLEIVERB_SHOW:
		case OLEIVERB_PRIMARY:
			if (m_fOpen)
				::SetFocus (m_lpObj->GetAppWindowHandle ());
			else if (FALSE == m_lpObj->DoInPlaceActivate (iVerb))
				m_lpObj->OpenForEdit (lpActiveSite);
			break;

		case OLEIVERB_UIACTIVATE:
			if (m_fOpen)
				return (ResultFromScode (E_FAIL));

				// Activate!
			if (FALSE == m_lpObj->DoInPlaceActivate (iVerb))
				return (ResultFromScode (E_FAIL));
			break;

		case OLEIVERB_DISCARDUNDOSTATE:
				// Since we don't support an undo state,
				// don't bother with this!
			if (!(m_lpObj->IsInPlaceActive ()))
				return (ResultFromScode (OLE_E_NOT_INPLACEACTIVE));
			break;

		case OLEIVERB_HIDE:
				// If we're in-place active, do an in-place hide
			if (m_lpObj->IsInPlaceActive ())
			{
				m_lpObj->DeactivateUI ();
				m_lpObj->DoInPlaceHide ();
			}
			else
				m_lpObj->HideAppWindow ();
			break;

		case OLEIVERB_OPEN:
		case VERB_OPEN:
				// If we're already in-place active, deactivate
			if (m_lpObj->IsInPlaceActive ())
				m_lpObj->InPlaceDeactivate ();

				// Open into the new window
			m_fOpen = m_lpObj->OpenForEdit (lpActiveSite);
			break;

		default:
			if (iVerb < 0)
				return (ResultFromScode (E_FAIL));
	}

	return (ResultFromScode (S_OK));
}

	// Enumerate verbs associated with this object
	// For this server, just tell them to look it
	// up in the registry!
STDMETHODIMP COleObject::EnumVerbs (
	LPENUMOLEVERB FAR *ppEnumOleVerb
) {
	return (ResultFromScode (OLE_S_USEREG));
}

	// Make sure the data is up-to-date
STDMETHODIMP COleObject::Update (
	void
) {
	m_lpObj->SendOnDataChange ();
	return (ResultFromScode (S_OK));
}

	// Tell the caller if we're up to date
	// This object is always up to date!
STDMETHODIMP COleObject::IsUpToDate (
	void
) {
	return (ResultFromScode (S_OK));
}

	// Hand our class ID back to the container
STDMETHODIMP COleObject::GetUserClassID (
	CLSID FAR *	lpClsId
) {
	m_lpObj->GetClassID (lpClsId);
	return (ResultFromScode (S_OK));
}

	// Get a presentable ID for this server
	// Actually, tell the caller to look it up
	// in the registry.
STDMETHODIMP COleObject::GetUserType (
	DWORD			dwFormOfType,
	LPOLESTR FAR *	ppszUserType
) {
	return (ResultFromScode (OLE_S_USEREG));
}

	// The application is telling us how much room is available
STDMETHODIMP COleObject::SetExtent (
	DWORD	dwDrawAspect,
	LPSIZEL	lpSizel
) {
		// We're only supporting Content at the present time
	if (DVASPECT_CONTENT != dwDrawAspect)
		return (ResultFromScode (E_FAIL));

	SIZEL	newSize;

		// Convert the dimensions from Himetric to pixels
	newSize.cx = ::XFormWidthHimetricToPixel ((int) lpSizel->cx);
	newSize.cy = ::XFormHeightHimetricToPixel ((int) lpSizel->cy);

		// Store the new size
	m_lpObj->SetSize (&newSize);

	return (ResultFromScode (S_OK));
}

	// Tell the container how much room we need to display
STDMETHODIMP COleObject::GetExtent (
	DWORD	dwDrawAspect,
	LPSIZEL	lpSizel
) {
		// We're only supporting Content at the present time
	if (DVASPECT_CONTENT != dwDrawAspect)
		return (ResultFromScode (E_FAIL));

		// Get the current size (stored in pixels)
	SIZE	curSize;

	m_lpObj->GetSize (&curSize);

		// Convert to standard scale
	lpSizel->cx = (LONG) ::XFormWidthPixelToHimetric (curSize.cx);
	lpSizel->cy = (LONG) ::XFormHeightPixelToHimetric (curSize.cy);

	return (ResultFromScode (S_OK));
}

	// The caller wants an Advise connection
STDMETHODIMP COleObject::Advise (
	LPADVISESINK	lpAdvSink,
	LPDWORD			lpdwConnection
) {
	return (m_lpObj->CreateAdvise (lpAdvSink, lpdwConnection));
}

	// The caller's done with the connection
STDMETHODIMP COleObject::Unadvise (
	DWORD		dwConnection
) {
	return (m_lpObj->Unadvise (dwConnection));
}

	// Return an enumerator for outstanding advise connections
STDMETHODIMP COleObject::EnumAdvise (
	LPENUMSTATDATA FAR *lplpEnumAdvise
) {
		// Make sure the answer is NULL if something goes wrong
	*lplpEnumAdvise = (LPENUMSTATDATA) NULL;

		// Tell the object to create the enumerator
	return (m_lpObj->EnumAdvise (lplpEnumAdvise));
}

	// Return status information for this server
	// Just tell the caller to look in the registry
STDMETHODIMP COleObject::GetMiscStatus (
	DWORD		dwAspect,
	LPDWORD		lpStatus
) {
	*lpStatus = 0L;
	
	return (ResultFromScode (OLE_S_USEREG));
}

	// Set the color palette
	// Not used in this server
STDMETHODIMP COleObject::SetColorScheme (
	LPLOGPALETTE	lpLogPal
) {
	return (ResultFromScode (S_OK));
}
