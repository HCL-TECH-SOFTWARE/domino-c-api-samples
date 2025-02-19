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
*		oledo.cpp
*
*	Implementation of CDataObject
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
#include "ln_buff.hpp"
#include "ln_flow.hpp"
#include "nf_doc.hpp"
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

	// QueryInterface
STDMETHODIMP CDataObject::QueryInterface (
	REFIID			riid,
	LPVOID FAR *	ppvObj
) {
		// Let the containing CImageObject handle this
	return (m_lpObj->QueryInterface (riid, ppvObj));
}

	// AddRef
STDMETHODIMP_ (ULONG) CDataObject::AddRef (
	void
) {
		// Increment the local counter
	++m_nRefCount;

		// A reference to *this is a reference to the containing CImageObject!
	return (m_lpObj->AddRef ());
}

	// Release
STDMETHODIMP_ (ULONG) CDataObject::Release (
	void
) {
	--m_nRefCount;

		// A reference to *this is a reference to the containing CImageObject!
	return (m_lpObj->Release ());
}

//
//		IDataObject methods
//

	// Construct a representation for the caller
STDMETHODIMP CDataObject::GetData (
	LPFORMATETC	pFormatEtc,
	LPSTGMEDIUM	pStgMedium
) {
	SCODE		sc = DATA_E_FORMATETC;

		// Did they ask for a format we can supply?

		// Was it a request for a Metafile?
	if ((CF_METAFILEPICT == pFormatEtc->cfFormat)
		&& (DVASPECT_CONTENT == pFormatEtc->dwAspect)
		&& (((DVTARGETDEVICE *) NULL) == pFormatEtc->ptd)
		&& ((-1) == pFormatEtc->lindex)
		&& (TYMED_MFPICT & pFormatEtc->tymed))
	{
		DHANDLE	hmfPict = m_lpObj->GetMetafilePict ();
		pStgMedium->tymed = TYMED_MFPICT;
		pStgMedium->hGlobal = (HGLOBAL)hmfPict;
		pStgMedium->pUnkForRelease = NULL;
		sc = S_OK;
	}
	else
	{
// NotesFlow code
		CImageDocument FAR *lpDoc = m_lpObj->GetDocumentPtr ();

			// Do we have a valid document pointer?
		if (((CImageDocument FAR *) NULL) == lpDoc)
			return (ResultFromScode (E_FAIL));		// Nope!!

			// Keep the document object around while we use it
		lpDoc->AddRef ();

			// Might it be NoteshNote?
		if ((lpDoc->GetNoteshNoteClipFormat() == pFormatEtc->cfFormat)
			&& (DVASPECT_CONTENT == pFormatEtc->dwAspect)
			&& (((DVTARGETDEVICE *) NULL) == pFormatEtc->ptd)
			&& ((-1) == pFormatEtc->lindex)
			&& (TYMED_HGLOBAL & pFormatEtc->tymed))
		{
				// Update the media type in the FORMATETC structure
			pFormatEtc->tymed = TYMED_HGLOBAL;

				// Fill in the STGMEDIUM structure
			sc = lpDoc->ProcessNoteshNote (pStgMedium);
		}
			// Might it be NotesDocAction?
		else if ((lpDoc->GetNotesDocActionClipFormat() == pFormatEtc->cfFormat)
			&& (DVASPECT_CONTENT == pFormatEtc->dwAspect)
			&& (((DVTARGETDEVICE *) NULL) == pFormatEtc->ptd)
			&& ((-1) == pFormatEtc->lindex)
			&& (TYMED_HGLOBAL & pFormatEtc->tymed))
		{
				// Update the media type in the FORMATETC structure
			pFormatEtc->tymed = TYMED_HGLOBAL;

				// Fill in the STGMEDIUM structure
			sc = lpDoc->ProcessNotesDocAction (pStgMedium);
		}

		// We're done with the document object
	lpDoc->Release ();

// End NotesFlow code

	}

	return (ResultFromScode (sc));
}

	// Get data in caller's location - not supported
STDMETHODIMP CDataObject::GetDataHere (
	LPFORMATETC	pFormatEtc,
	LPSTGMEDIUM	pStgMedium
) {
	return (ResultFromScode (DATA_E_FORMATETC));
}

STDMETHODIMP CDataObject::QueryGetData (
	LPFORMATETC pFormatEtc
) {
	SCODE		sc = DATA_E_FORMATETC;

		// Did they ask for a format we can supply?
	if ((-1) != pFormatEtc->lindex)
	{
		sc = DV_E_LINDEX;			// Bad lindex
	}
	else if (DVASPECT_CONTENT != pFormatEtc->dwAspect)
	{
		sc = DV_E_DVASPECT;			// Bad dwAspect
	}
	else if ((CF_METAFILEPICT == pFormatEtc->cfFormat)
		&& (TYMED_MFPICT & pFormatEtc->tymed))
	{
		sc = S_OK;					// Metafiles are just fine!
	}
	else
	{
// NotesFlow code
		CImageDocument FAR *lpDoc = m_lpObj->GetDocumentPtr ();

			// Do we have a valid document pointer?
		if (((CImageDocument FAR *) NULL) == lpDoc)
		{
			sc = E_FAIL;			// NotesFlow isn't available
		}
		else
		{
				// Keep the document object around while we use it
			lpDoc->AddRef ();

			if ((lpDoc->GetNoteshNoteClipFormat() == pFormatEtc->cfFormat)
				&& (TYMED_HGLOBAL & pFormatEtc->tymed))
			{
				sc = S_OK;				// Sure, we can supply a NoteshNote request
			}
			else if ((lpDoc->GetNotesDocActionClipFormat() == pFormatEtc->cfFormat)
				&& (TYMED_HGLOBAL & pFormatEtc->tymed))
			{
				sc = S_OK;				// Sure, we can supply a NotesDocAction request
			}

				// We're done with the document object
			lpDoc->Release ();
		}

// End NotesFlow code
	}

	return (ResultFromScode (sc));
}

STDMETHODIMP CDataObject::GetCanonicalFormatEtc (
	LPFORMATETC	pFormatEtc,
	LPFORMATETC	pFormatEtcOut
) {
	HRESULT		hResult;

		// Validate arguments
	if (((LPFORMATETC) NULL) == pFormatEtcOut)
		return (ResultFromScode (E_INVALIDARG));

		// Make SURE that pointer gets set to NULL!
	pFormatEtcOut->ptd = NULL;

	if (((LPFORMATETC) NULL) == pFormatEtc)
		return (ResultFromScode (E_INVALIDARG));

		// Is the requested format supported?
	hResult = QueryGetData (pFormatEtc);
	if (NOERROR != hResult)
		return (hResult);

		// This application is insensitive to target device.
	*pFormatEtcOut = *pFormatEtc;
	if (NULL == pFormatEtc->ptd)
		return (ResultFromScode (DATA_S_SAMEFORMATETC));
	else
	{
		pFormatEtcOut->ptd = NULL;
		return (NOERROR);
	}
}

	// Set data in this object
	// Implemented for NotesFlow support
STDMETHODIMP CDataObject::SetData (
	LPFORMATETC		pFormatEtc,
	LPSTGMEDIUM		pMedium,
	BOOL			fRelease
) {
	HRESULT			result = ResultFromScode (DATA_E_FORMATETC);
// NotesFlow code
	CImageDocument FAR *lpDoc = m_lpObj->GetDocumentPtr ();

	if (((CImageDocument FAR *) NULL) == lpDoc)
		return (ResultFromScode (E_FAIL));

		// Keep the document object around while we use it
	lpDoc->AddRef ();

		// Is the format NotesDocInfo?
	if ((lpDoc->GetNotesDocInfoClipFormat() == pFormatEtc->cfFormat)
		&& (DVASPECT_CONTENT == pFormatEtc->dwAspect)
		&& (TYMED_HGLOBAL & pFormatEtc->tymed))
	{
			// We can only deal with TYMED_HGLOBAL
		if (0 == (TYMED_HGLOBAL & pMedium->tymed))
			result = ResultFromScode (DV_E_TYMED);
		else
		{
				// Update program info from Domino and Notes data
			result = lpDoc->ProcessNotesDocInfo (pMedium);
		}

				// Do we need to free the memory?
		if (fRelease)
		{
			ReleaseStgMedium (pMedium);
		}
	}
		// Might it be NotesDocActionDone?
	else if ((lpDoc->GetNotesDocActionDoneClipFormat() == pFormatEtc->cfFormat)
		&& (DVASPECT_CONTENT == pFormatEtc->dwAspect)
		&& (TYMED_HGLOBAL & pFormatEtc->tymed))
	{
			// We can only deal with TYMED_HGLOBAL
		if (0 == (TYMED_HGLOBAL & pMedium->tymed))
			result = ResultFromScode (DV_E_TYMED);
		else
		{
				// Update program info from Domino and Notes data
			result = lpDoc->ProcessNotesDocActionDone (pMedium);
		}

			// Do we need to free the memory?
		if (fRelease)
		{
			ReleaseStgMedium (pMedium);
		}
	}

		// We're done with the document object
	lpDoc->Release ();

// End NotesFlow code

	return (result);
}

	// Enumerate formats supported by this object
	// For now, just tell the caller to use the registry info!
STDMETHODIMP CDataObject::EnumFormatEtc (
	DWORD					dwDirection,
	LPENUMFORMATETC FAR *	ppEnumFormatEtc
) {
	*ppEnumFormatEtc = NULL;

	return (ResultFromScode (OLE_S_USEREG));
}

	// Container is telling us it wants notification of data changes
STDMETHODIMP CDataObject::DAdvise (
	FORMATETC FAR *	pFormatEtc,
	DWORD			advf,
	LPADVISESINK	pAdvSink,
	DWORD FAR *		pdwConnection
) {
	LPDATAADVISEHOLDER	lpHolder = m_lpObj->GetDataAdviseHolderPtr ();
	HRESULT				hResult;

		// If no DataAdviseHolder has been created, create one!
	if (((LPDATAADVISEHOLDER) NULL) == lpHolder)
	{
		hResult = ::CreateDataAdviseHolder (&lpHolder);
		if (FAILED (hResult))
			return (hResult);

		m_lpObj->SetDataAdviseHolderPtr (lpHolder);
	}

		// Set up the advise connection
	return (lpHolder->Advise (this, pFormatEtc, advf, pAdvSink, pdwConnection));
}

	// Disconnect the advise connection
STDMETHODIMP CDataObject::DUnadvise (
	DWORD	dwConnection
) {
	LPDATAADVISEHOLDER	lpHolder = m_lpObj->GetDataAdviseHolderPtr ();

		// Only makes sense if a DataAdviseHolder exists
	if (((LPDATAADVISEHOLDER) NULL) == lpHolder)
		return (ResultFromScode (E_INVALIDARG));
	else
	{
			// Remove the advise connection
		return (lpHolder->Unadvise (dwConnection));
	}
}

	
STDMETHODIMP CDataObject::EnumDAdvise (
	LPENUMSTATDATA FAR *	ppEnumAdvise
) {
	LPDATAADVISEHOLDER	lpHolder = m_lpObj->GetDataAdviseHolderPtr ();

		// Ensure that this pointer is NULL on error
	*ppEnumAdvise = (LPENUMSTATDATA) NULL;

		// If there's a DataAdviseHolder . . .
	if (((LPDATAADVISEHOLDER) NULL) == lpHolder)
		return (ResultFromScode (E_INVALIDARG));
	else
	{
			// . . . let IT do the enumeration!
		return (lpHolder->EnumAdvise (ppEnumAdvise));
	}
}
