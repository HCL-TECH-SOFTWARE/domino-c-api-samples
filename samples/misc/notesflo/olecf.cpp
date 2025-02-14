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
*
*
*		olecf.cpp
*
*	Implementation of CClassFactory
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
#include "nf_app.hpp"
#include "nf_doc.hpp"
#include "olecf.hpp"

//
//		Class management
//

	// Constructor
CClassFactory::CClassFactory (
	CImageApplication FAR *	lpApp
) {
	m_lpApp = lpApp;
	m_nRefCount = 0;
}

	// Destructor
CClassFactory::~CClassFactory (
	void
) {
	;
}

//
//		IUnknown methods
//

	// QueryInterface
STDMETHODIMP CClassFactory::QueryInterface (
	REFIID			riid,
	LPVOID FAR *	ppvObj
) {
	SCODE			sc = S_OK;

		// Note that the class factory object is independent of any
		// instance of the actual object, so this interface returns
		// "this" when asked for the IUnknown interface.
	if ((IID_IUnknown == riid) || (IID_IClassFactory == riid))
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
STDMETHODIMP_ (ULONG) CClassFactory::AddRef (
	void
) {
		// Increment and return the reference count
	return (++m_nRefCount);
}

	// Release
STDMETHODIMP_ (ULONG) CClassFactory::Release (
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

//
//		IClassFactory methods
//

	// Create an instance of the NotesFlow OLE object
STDMETHODIMP CClassFactory::CreateInstance (
	LPUNKNOWN		pUnkOuter,
	REFIID			riid,
	LPVOID FAR *	ppvObject
) {
	SCODE			sc = S_OK;

		// Clear *ppvObject in case something goes wrong
	*ppvObject = (LPVOID) NULL;

	if (((LPUNKNOWN) NULL) != pUnkOuter)
		sc = CLASS_E_NOAGGREGATION;
	else
	{
			// Get the pointer to the document
		CImageDocument FAR *	lpDoc = m_lpApp->GetDocumentPtr ();

			// If there's a pointer there, call CreateObject ()
		if (((CImageDocument FAR *) NULL) != lpDoc)
			return (lpDoc->CreateObject (riid, ppvObject));
		else
			sc = CLASS_E_CLASSNOTAVAILABLE;
	}

	return (ResultFromScode (sc));
}

	// If the argument is TRUE, lock the server in memory.  If
	// the argument is FALSE, unlock the server.
STDMETHODIMP CClassFactory::LockServer (
	BOOL	fLock
) {
	::CoLockObjectExternal (m_lpApp, fLock, TRUE);

	return (ResultFromScode (S_OK));
}
