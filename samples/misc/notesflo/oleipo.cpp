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
*		oleipo.cpp
*
*	Implementation of COleInPlaceObject
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
STDMETHODIMP COleInPlaceObject::QueryInterface (
	REFIID			riid,
	LPVOID FAR *	ppvObj
) {
		// Let the containing CImageObject handle this
	return (m_lpObj->QueryInterface (riid, ppvObj));
}

	// AddRef
STDMETHODIMP_ (ULONG) COleInPlaceObject::AddRef (
	void
) {
		// Increment the local counter
	++m_nRefCount;

		// A reference to *this is a reference to the containing CImageObject!
	return (m_lpObj->AddRef ());
}

	// Release
STDMETHODIMP_ (ULONG) COleInPlaceObject::Release (
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
STDMETHODIMP COleInPlaceObject::GetWindow (
	HWND FAR *	lphWnd
) {
	*lphWnd = m_lpObj->GetDocWindowHandle ();

	return (ResultFromScode (S_OK));
}

	// Activate context sensitive help - not implemented
STDMETHODIMP COleInPlaceObject::ContextSensitiveHelp (
	BOOL		fEnterMode
) {
	return (ResultFromScode (E_NOTIMPL));
}

//
//		IOleInPlaceObject methods
//

STDMETHODIMP COleInPlaceObject::InPlaceDeactivate (
	void
) {
	return (m_lpObj->InPlaceDeactivate ());
}

STDMETHODIMP COleInPlaceObject::UIDeactivate (
	void
) {
	m_lpObj->DeactivateUI ();

	return (ResultFromScode (S_OK));
}

	// Called when container's clipping region or object position changes
STDMETHODIMP COleInPlaceObject::SetObjectRects (
	LPCRECT	lpPosRect,
	LPCRECT	lpClipRect
) {
	RECT	resultRect;
	SIZE	offsets;

		// Get the intersection of the position and clipping rectangles
	::IntersectRect (&resultRect, lpPosRect, lpClipRect);

		// Set the new offsets
	m_lpObj->SetImageOffsets (
		abs (resultRect.left - lpPosRect->left),
		abs (resultRect.top - lpPosRect->top)
	);

		// Adjust the hatch window size
	m_lpObj->SetHatchWindowSize (lpPosRect, lpClipRect, &offsets);

		// Offset the result rectangle
	::OffsetRect (&resultRect, offsets.cx, offsets.cy);

		// Set the object's position rectangle
	m_lpObj->SetPosition (lpPosRect);

		// Move the document window
	::MoveWindow (
		m_lpObj->GetDocWindowHandle (),
		resultRect.left,
		resultRect.top,
		resultRect.right - resultRect.left,
		resultRect.bottom - resultRect.top,
		TRUE
	);

	return (ResultFromScode (S_OK));
}

	// Undo the last edit made in this object
	// Since we don't support Undo, return an error
STDMETHODIMP COleInPlaceObject::ReactivateAndUndo (
	void
) {
	return (ResultFromScode (INPLACE_E_NOTUNDOABLE));
}
