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
*		oleec.cpp
*
*	Implementation of CExternalConnection
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

	// QueryInterface
STDMETHODIMP CExternalConnection::QueryInterface (
	REFIID			riid,
	LPVOID FAR *	ppvObj
) {
		// Let the containing CImageObject handle this
	return (m_lpObj->QueryInterface (riid, ppvObj));
}

	// AddRef
STDMETHODIMP_ (ULONG) CExternalConnection::AddRef (
	void
) {
		// Increment the local counter
	++m_nRefCount;

		// A reference to *this is a reference to the containing CImageObject!
	return (m_lpObj->AddRef ());
}

	// Release
STDMETHODIMP_ (ULONG) CExternalConnection::Release (
	void
) {
	--m_nRefCount;

		// A reference to *this is a reference to the containing CImageObject!
	return (m_lpObj->Release ());
}

//
//		IExternalConnection methods
//

	// Add an external connection
STDMETHODIMP_(DWORD) CExternalConnection::AddConnection (
	DWORD	ExtConn,
	DWORD	Reserved
) {
	if (EXTCONN_STRONG & ExtConn)
		return (++m_dwStongCount);
	else
		return (0);
}

	// Release an external connection
STDMETHODIMP_(DWORD) CExternalConnection::ReleaseConnection (
	DWORD	ExtConn,
	DWORD	Reserved,
	BOOL	fLastReleaseCloses
) {
	if (EXTCONN_STRONG & ExtConn)
	{
		DWORD	dwCurCount = --m_dwStongCount;

			// On the last release, close the COleObject instance
		if ((0 == m_dwStongCount) && fLastReleaseCloses)
			(m_lpObj->GetOleObjectPtr ())->Close (OLECLOSE_SAVEIFDIRTY);

		return (dwCurCount);
	}
	else
		return (0);
}
