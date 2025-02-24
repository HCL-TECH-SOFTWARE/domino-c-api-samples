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
*		oleipao.cpp
*
*	Implementation of COleInPlaceActiveObject
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
STDMETHODIMP COleInPlaceActiveObject::QueryInterface (
	REFIID			riid,
	LPVOID FAR *	ppvObj
) {
		// Let the containing CImageObject handle this
	return (m_lpObj->QueryInterface (riid, ppvObj));
}

	// AddRef
STDMETHODIMP_ (ULONG) COleInPlaceActiveObject::AddRef (
	void
) {
		// Increment the local counter
	++m_nRefCount;

		// A reference to *this is a reference to the containing CImageObject!
	return (m_lpObj->AddRef ());
}

	// Release
STDMETHODIMP_ (ULONG) COleInPlaceActiveObject::Release (
	void
) {
	--m_nRefCount;

		// A reference to *this is a reference to the containing CImageObject!
	return (m_lpObj->Release ());
}

//
//		IOleWindow methods
//

	// Get the window handle for this OLE object
STDMETHODIMP COleInPlaceActiveObject::GetWindow (
	HWND FAR *	lphWnd
) {
	*lphWnd = m_lpObj->GetDocWindowHandle ();

	return (ResultFromScode (S_OK));
}

	// Activate context sensitive help - not implemented
STDMETHODIMP COleInPlaceActiveObject::ContextSensitiveHelp (
	BOOL		fEnterMode
) {
	return (ResultFromScode (E_NOTIMPL));
}

//
//		IOleInPlaceActiveObject methods
//

	// Translate accelerators for this DLL
	// Since we're an EXE, we just return S_FALSE!
STDMETHODIMP COleInPlaceActiveObject::TranslateAccelerator (
	LPMSG		lpMsg
) {
	return (ResultFromScode (S_FALSE));
}

	// Activate the frame window
STDMETHODIMP COleInPlaceActiveObject::OnFrameWindowActivate (
	BOOL		fActivate
) {
		// Nothing needs to be done!
	return (ResultFromScode (S_OK));
}

	// Activate the document window
STDMETHODIMP COleInPlaceActiveObject::OnDocWindowActivate (
	BOOL		fActivate
) {
	if (fActivate)
	{
		m_lpObj->AddFrameLevelUI ();
	}
		// There's nothing to clean up on deactivate!

	return (ResultFromScode (S_OK));
}

	// Called when the border changes size
STDMETHODIMP COleInPlaceActiveObject::ResizeBorder (
	LPCRECT		prcBorder,
	LPOLEINPLACEUIWINDOW pUIWindow,
	BOOL		fFrameWindow
) {
	(m_lpObj->GetOleInPlaceFramePtr ())->SetBorderSpace (NULL);

		// Is there a UI window?
        if (((HWND) NULL) != (HWND)m_lpObj->GetOleInPlaceUIWindowPtr ())
		(m_lpObj->GetOleInPlaceUIWindowPtr ())->SetBorderSpace (NULL);

	return (ResultFromScode (S_OK));
}

	// Enable or disable modeless dialogs.
	// This OLE object doesn't have any, so we can just return S_OK.
STDMETHODIMP COleInPlaceActiveObject::EnableModeless (
	BOOL		fEnable
) {
	return (ResultFromScode (S_OK));
}
