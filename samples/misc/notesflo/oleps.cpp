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
*		oleps.cpp
*
*	Implementation of CPersistStorage
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
#include "nf_store.hpp"
#include "oleobj.hpp"
#include "oleps.hpp"
#include "oledo.hpp"
#include "oleipo.hpp"
#include "oleipao.hpp"
#include "oleec.hpp"
#include "nf_obj.hpp"

//
//		Constructor & destructor
//

CPersistStorage::CPersistStorage (
	void
) : m_lpObj ((CImageObject FAR *) NULL),
	m_nRefCount (0),
	m_State (STATE_UNINITIALIZED),
	m_lpImageStore ((CImageStore FAR *) NULL)
{
	;
}

CPersistStorage::~CPersistStorage (
	void
) {
	if (((CImageStore FAR *) NULL) != m_lpImageStore)
	{
		m_lpImageStore->Release ();
		m_lpImageStore = NULL;
	}
}


//
//		IUnknown methods
//

	// QueryInterface
STDMETHODIMP CPersistStorage::QueryInterface (
	REFIID			riid,
	LPVOID FAR *	ppvObj
) {
		// Let the containing CImageObject handle this
	return (m_lpObj->QueryInterface (riid, ppvObj));
}

	// AddRef
STDMETHODIMP_ (ULONG) CPersistStorage::AddRef (
	void
) {
		// Increment the local counter
	++m_nRefCount;

		// A reference to *this is a reference to the containing CImageObject!
	return (m_lpObj->AddRef ());
}

	// Release
STDMETHODIMP_ (ULONG) CPersistStorage::Release (
	void
) {
	--m_nRefCount;

		// A reference to *this is a reference to the containing CImageObject!
	return (m_lpObj->Release ());
}

//
//		IPersist methods
//

	// Get the object's class ID
STDMETHODIMP CPersistStorage::GetClassID (
	LPCLSID	lpClassID
) {
		// Have we been initialized yet?
    if (STATE_UNINITIALIZED == m_State)
        return (ResultFromScode (E_UNEXPECTED));

	m_lpObj->GetClassID (lpClassID);

	return (ResultFromScode (S_OK));
}

//
//		IPersistStorage methods
//

	// Has the object been modified?
STDMETHODIMP CPersistStorage::IsDirty (
	void
) {
		// Have we been initialized yet?
    if (STATE_UNINITIALIZED == m_State)
        return (ResultFromScode (E_UNEXPECTED));

		// If there IS an image object . . .
	if (((CImageObject FAR *) NULL) != m_lpObj)
	{
			// . . . ask it if we're dirty!
		if (m_lpObj->IsDirty ())
		{
			return (ResultFromScode (S_OK));
		}
	}

	return (ResultFromScode (S_FALSE));
}

	// Initialize a new storage
STDMETHODIMP CPersistStorage::InitNew (
	LPSTORAGE	lpStg
) {
	HRESULT		answer;

		// Have we already been initialized?
    if (STATE_UNINITIALIZED != m_State)
        return (ResultFromScode (CO_E_ALREADYINITIALIZED));

		// Is the input valid?
	if (((LPSTORAGE) NULL) == lpStg)
		return (ResultFromScode (E_POINTER));

		// If there's already an Image Store, discard it
	if (((CImageStore FAR *) NULL) != m_lpImageStore)
	{
		m_lpImageStore->Release ();
		m_lpImageStore = (CImageStore FAR *) NULL;
	}

		// Create a new Image Store
	m_lpImageStore = new CImageStore;
	if (((CImageStore FAR *) NULL) == m_lpImageStore)
		return (ResultFromScode (E_OUTOFMEMORY));
	
	m_lpImageStore->AddRef ();

		// Initialize the Image Store
	answer = m_lpImageStore->InitStore (lpStg, m_lpObj);
	if (SUCCEEDED (answer))
	{
		m_State = CPersistStorage::STATE_SCRIBBLE;
	}

		// Are we all set?
	if ((FAILED (answer)) && (((CImageStore FAR *) NULL) != m_lpImageStore))
	{
			// No - discard the image store
		m_lpImageStore->Release ();
		m_lpImageStore = (CImageStore FAR *) NULL;
	}

	return (answer);
}

	// Load the object from the storage
STDMETHODIMP CPersistStorage::Load (
	LPSTORAGE	lpStg
) {
	HRESULT		answer;

		// Have we already been initialized?
    if (STATE_UNINITIALIZED != m_State)
        return (ResultFromScode (CO_E_ALREADYINITIALIZED));

		// Is the input valid?
	if (((LPSTORAGE) NULL) == lpStg)
		return (ResultFromScode (E_POINTER));

		// If there's already an Image Store, discard it
	if (((CImageStore FAR *) NULL) != m_lpImageStore)
	{
		m_lpImageStore->Release ();
		m_lpImageStore = (CImageStore FAR *) NULL;
	}

		// Create a new Image Store
	m_lpImageStore = new CImageStore;
	if (((CImageStore FAR *) NULL) == m_lpImageStore)
		return (ResultFromScode (E_OUTOFMEMORY));
	
	m_lpImageStore->AddRef ();

		// Connect the new IStorage to the Image Store
	answer = m_lpImageStore->OpenStore (lpStg, m_lpObj);
	if (SUCCEEDED (answer))
	{
			// Load the image data
		answer = m_lpImageStore->LoadData (m_lpObj);
		if (SUCCEEDED (answer))
		{
			m_State = CPersistStorage::STATE_SCRIBBLE;
		}
	}

		// Are we all set?
	if ((FAILED (answer)) && (((CImageStore FAR *) NULL) != m_lpImageStore))
	{
			// No - discard the image store
		m_lpImageStore->Release ();
		m_lpImageStore = (CImageStore FAR *) NULL;
	}

	return (answer);
}

	// Save the object to the storage
STDMETHODIMP CPersistStorage::Save (
	LPSTORAGE	lpStg,
	BOOL		fSameAsLoad
) {
	HRESULT		answer;

		// Are we in the "Scribble" state?
	if (CPersistStorage::STATE_SCRIBBLE != m_State)
		return (ResultFromScode (E_UNEXPECTED));

		// lpStg can only be NULL if fSameAsLoad is TRUE
	if ((!fSameAsLoad) && (((LPSTORAGE) NULL) == lpStg))
		return (ResultFromScode (E_FAIL));
		
		// Make sure we have an Image Store ready for this save
	if (((CImageStore FAR *) NULL) == m_lpImageStore)
		return (ResultFromScode (E_FAIL));

		// Incremental or full save?
	if (fSameAsLoad)
	{
			// Incremental save
		answer = m_lpImageStore->IncrementalSave (m_lpObj);
	}
	else
	{
		CImageStore FAR *	lpTempStore;

			// Full save

			// Create a temporary image store
		lpTempStore = new CImageStore;
		if (((CImageStore FAR *) NULL) == lpTempStore)
			return (ResultFromScode (E_OUTOFMEMORY));

		lpTempStore->AddRef ();

			// Initialized this new storage
		answer = lpTempStore->InitStore (lpStg, m_lpObj);
		if (SUCCEEDED (answer))
		{
				// Save everything
			answer = lpTempStore->FullSave (m_lpObj);
		}

			// We're done with the temporary storage
		lpTempStore->Release ();
	}

	if (SUCCEEDED (answer))
		m_State = CPersistStorage::STATE_NO_SCRIBBLE;

	return (answer);
}

	// Save has been completed - leave "Hands Off" state
STDMETHODIMP CPersistStorage::SaveCompleted (
	LPSTORAGE	lpNewStg
) {
	HRESULT		answer;

		// Is this being called at the appropriate time?
	if ((CPersistStorage::STATE_NO_SCRIBBLE != m_State)
		&& (CPersistStorage::STATE_HANDS_OFF != m_State))
	{
		return (ResultFromScode (E_UNEXPECTED));
	}

		// Is the storage pointer valid?
	if ((CPersistStorage::STATE_HANDS_OFF == m_State)
		&& (((LPSTORAGE) NULL) == lpNewStg))
	{
		return (ResultFromScode (E_INVALIDARG));
	}

		// Did the storage change during the save?
	if ((((LPSTORAGE) NULL) == lpNewStg)
		&& (((CImageStore FAR *) NULL) != m_lpImageStore))
	{
			// No - re-open the current Image Store
		lpNewStg = m_lpImageStore->GetStoragePtr ();
		if (((LPSTORAGE) NULL) == lpNewStg)
			return (ResultFromScode (E_INVALIDARG));

			// Hang on to that storage while we re-open
		lpNewStg->AddRef ();

			// Re-open the save IStorage
		answer = m_lpImageStore->OpenStore (lpNewStg, m_lpObj);

		lpNewStg->Release ();
	}
	else
	{
		CImageStore FAR *	lpNewStore;

			// Yes - create a new Image Store
		lpNewStore = new CImageStore;
		if (((CImageStore FAR *) NULL) == lpNewStore)
			return (ResultFromScode (E_OUTOFMEMORY));

		lpNewStore->AddRef ();

			// Hand the new IStorage to the new Image Store
		answer = lpNewStore->OpenStore (lpNewStg, m_lpObj);
		if (SUCCEEDED (answer))
		{
				// Did we have an Image Store open?
			if (((CImageStore FAR *) NULL) != m_lpImageStore)
			{
					// Yes - release it
				m_lpImageStore->Release ();
				m_lpImageStore = (CImageStore FAR *) NULL;
			}

				// Hang on to the new Image Store
			m_lpImageStore = lpNewStore;
		}
	}

		// Perform OLE On Save notification
	if (SUCCEEDED (answer))
	{
		(m_lpObj->GetOleAdviseHolder ())->SendOnSave ();
		m_State = CPersistStorage::STATE_SCRIBBLE;
	}

	return (answer);
}

	// Enter "Hands Off" state
STDMETHODIMP CPersistStorage::HandsOffStorage (
	void
) {
		// Did we get this at an appropriate time?
	if ((CPersistStorage::STATE_UNINITIALIZED == m_State)
		|| (CPersistStorage::STATE_HANDS_OFF == m_State))
		return (ResultFromScode (E_UNEXPECTED));

		// Just release the Image Store!
	if (((CImageStore FAR *) NULL) != m_lpImageStore)
	{
		m_lpImageStore->Release ();
		m_lpImageStore = (CImageStore FAR *) NULL;
	}

	m_State = CPersistStorage::STATE_SCRIBBLE;

	return (ResultFromScode (S_OK));
}
