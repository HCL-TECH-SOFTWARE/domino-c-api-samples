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
*/
//
//		nf_store.cpp
//
//	Implementation of CImageStore.
//

	// Windows header files
#include <windows.h>
#include <ole2.h>
#include <ole2ver.h>

	// Domino and Notes header files
#include <global.h>
#include <olenotes.h>

	// Project header files
#include "notesflo.h"
#include "ln_buff.hpp"
#include "ln_flow.hpp"
#include "nf_obj.hpp"
#include "nf_doc.hpp"
#include "nf_store.hpp"

LPWSTR lpColorStreamName = OLESTR ("Colors");

//
//		Class management
//

	// Construct an empty image store
CImageStore::CImageStore (
	void
) {
	m_lpStorage = (LPSTORAGE) NULL;
	m_lpStream = (LPSTREAM) NULL;
	m_nRefCount = 0;
}

CImageStore::~CImageStore (
	void
) {
		// Make sure we've released anything we've been using!
	this->ReleaseStore ();
}

//
//		IUnknown methods
//

STDMETHODIMP CImageStore::QueryInterface (
	REFIID		riid,
	LPVOID FAR *ppvObj
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

STDMETHODIMP_ (ULONG) CImageStore::AddRef (
	void
) {
		// Increment and return the reference count
	return (++m_nRefCount);
}

STDMETHODIMP_ (ULONG) CImageStore::Release (
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

	// Fully initialize a new storage
HRESULT CImageStore::InitStore (
	LPSTORAGE	lpStg,
	CImageObject FAR *lpObj
) {
	HRESULT		answer;

		// Release anything that might be open
	this->ReleaseStore ();

	if (((LPSTORAGE) NULL) == lpStg)
	{
		answer = ResultFromScode (E_OUTOFMEMORY);
	}
	else
	{
			// Create the stream we need
		answer = lpStg->CreateStream (
			lpColorStreamName,
			STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_CREATE,
			(DWORD) 0,
			(DWORD) 0,
			&m_lpStream);
		if (SUCCEEDED (answer))
		{
			LPOLESTR	lpUserType;

				// Fetch the User Type
			lpObj->GetUserType (&lpUserType);

				// Write the identifying data
			answer = ::WriteFmtUserTypeStg (
				lpStg,
				lpObj->GetObjectClipboardFormat (),
				lpUserType);

				// Free the memory used for the User Type string
			::CoTaskMemFree (lpUserType);

			if (SUCCEEDED (answer))
			{
				ULARGE_INTEGER		uliSize;

				uliSize.LowPart = 2 * sizeof (COLORREF);
				uliSize.HighPart = 0;

					// Pre-allocate storage space
				answer = m_lpStream->SetSize (uliSize);
				if (SUCCEEDED (answer))
				{
						// All's well - hang on to this storage
					m_lpStorage = lpStg;
					m_lpStorage->AddRef ();
				}
			}
			
			if (FAILED (answer))
			{
					// Can't preallocate space - release the stream
				m_lpStream->Release ();
				m_lpStream = (LPSTREAM) NULL;
			}
		}
	}

	return (answer);
}

	// Open an existing storage
HRESULT CImageStore::OpenStore (
	LPSTORAGE	lpStg,
	CImageObject FAR *lpObj
) {
	HRESULT		answer;

		// Release anything that might be open
	this->ReleaseStore ();

		// Is the argument valid?
	if (((LPSTORAGE) NULL) == lpStg)
	{
		answer = ResultFromScode (E_OUTOFMEMORY);
	}
	else
	{
			// Try to open the stream
		answer = lpStg->OpenStream (
			lpColorStreamName,
			(void *) NULL,
			STGM_READWRITE | STGM_SHARE_EXCLUSIVE,
			(DWORD) 0,
			&m_lpStream);
		if (SUCCEEDED (answer))
		{
			m_lpStorage = lpStg;
			m_lpStorage->AddRef ();
		}
	}

	return (answer);
}

	// Read data for the object
HRESULT CImageStore::LoadData (
	CImageObject FAR *	lpObj
) {
	HRESULT				answer;
	CImageDocument FAR *lpDoc;
	COLORREF			curColor;
	LARGE_INTEGER		pos;

	if (((CImageObject FAR *) NULL) == lpObj)
		return (ResultFromScode (E_FAIL));

	lpDoc = lpObj->GetDocumentPtr ();
	if ((((CImageDocument FAR *) NULL) == lpDoc) || (((LPSTREAM) NULL) == m_lpStream))
		return (ResultFromScode (E_FAIL));

		// Seek to the start of the stream
	pos.LowPart = 0L;
	pos.HighPart = 0L;
	answer = m_lpStream->Seek (pos, STREAM_SEEK_SET,
		(ULARGE_INTEGER *) NULL);
	if (FAILED (answer))
		return (answer);

		// Read the colors
	curColor = DEFAULT_FOREGROUND_COLOR;		// Just in case!
	answer = m_lpStream->Read (&curColor,
		sizeof (COLORREF), (ULONG *) NULL);
	if (FAILED (answer))
		return (answer);

	lpDoc->SetForegroundColor (curColor);

	curColor = DEFAULT_BACKGROUND_COLOR;		// Just in case!
	answer = m_lpStream->Read (&curColor,
		sizeof (COLORREF), (ULONG *) NULL);
	if (FAILED (answer))
		return (answer);

	lpDoc->SetBackgroundColor (curColor);

	return (ResultFromScode (S_OK));
}

	// Perform an incremental save of the data
HRESULT CImageStore::IncrementalSave (
	CImageObject FAR *lpObj
) {
		// For this object, same as doing a FullSave!
	return (this->FullSave (lpObj));
}

	// Save all the data for the object
HRESULT CImageStore::FullSave (
	CImageObject FAR *lpObj
) {
	HRESULT				answer;
	CImageDocument FAR *lpDoc;
	COLORREF			curColor;
	LARGE_INTEGER		pos;

	if (((CImageObject FAR *) NULL) == lpObj)
		return (ResultFromScode (E_FAIL));

	lpDoc = lpObj->GetDocumentPtr ();
	if ((((CImageDocument FAR *) NULL) == lpDoc) || (((LPSTREAM) NULL) == m_lpStream))
		return (ResultFromScode (E_FAIL));

		// Seek to the start of the stream
	pos.LowPart = 0L;
	pos.HighPart = 0L;
	answer = m_lpStream->Seek (pos, STREAM_SEEK_SET,
		(ULARGE_INTEGER *) NULL);
	if (FAILED (answer))
		return (answer);

		// Save the colors
	curColor = lpDoc->GetForegroundColor ();
	answer = m_lpStream->Write (&curColor,
		sizeof (COLORREF), (ULONG *) NULL);
	if (FAILED (answer))
		return (answer);

	curColor = lpDoc->GetBackgroundColor ();
	answer = m_lpStream->Write (&curColor,
		sizeof (COLORREF), (ULONG *) NULL);
	if (FAILED (answer))
		return (answer);

	return (ResultFromScode (S_OK));
}

	// Common code to release current storages and streams
void CImageStore::ReleaseStore (
	void
) {
		// Release pointers to stream and storage objects
	if (((LPSTREAM) NULL) != m_lpStream)
	{
		m_lpStream->Release ();
		m_lpStream = (LPSTREAM) NULL;
	}

	if (((LPSTORAGE) NULL) != m_lpStorage)
	{
		m_lpStorage->Release ();
		m_lpStorage = (LPSTORAGE) NULL;
	}
}
