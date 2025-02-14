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
//		nf_hatch.cpp
//
//	Implementation of the CHatchWindow class.
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
#include "nf_hatch.hpp"
#include "oleobj.hpp"
#include "oleps.hpp"
#include "oledo.hpp"
#include "oleipo.hpp"
#include "oleipao.hpp"
#include "oleec.hpp"
#include "nf_obj.hpp"

//
//		Class management
//

	// Ye olde constructor
CHatchWindow::CHatchWindow (
	void
) {
	m_hHatchWnd = (HWND) NULL;
	m_BorderWidth = CHatchWindow::DefaultWidth;

	m_BorderRect.left = 0;
	m_BorderRect.top = 0;
	m_BorderRect.right = 0;
	m_BorderRect.bottom = 0;
	
	m_isTracking = FALSE;
	m_TrackMode = 0;

	m_TrackRect.left = 0;
	m_TrackRect.top = 0;
	m_TrackRect.right = 0;
	m_TrackRect.bottom = 0;

	m_ParentRect.left = 0;
	m_ParentRect.top = 0;
	m_ParentRect.right = 0;
	m_ParentRect.bottom = 0;
	
	m_TrackOffset.x = 0;
	m_TrackOffset.y = 0;

	m_hTrackBrush = (HBRUSH) NULL;
	m_hTrackPen = (HPEN) NULL;
	m_hTrackCursor = (HCURSOR) NULL;
	m_hSizeNESW = (HCURSOR) NULL;
	m_hSizeNWSE = (HCURSOR) NULL;
	m_hSizeNS = (HCURSOR) NULL;
	m_hSizeWE = (HCURSOR) NULL;
	m_hArrow = (HCURSOR) NULL;

	m_nRefCount = 0;
}

CHatchWindow::~CHatchWindow (
	void
) {
	;
}

//
//		Utility function to compute hit code
//

UINT CHatchWindow::GetHatchHitCode (
	LONG		x,
	LONG		y
) const {
	UINT		answer = 0;

	if (x <= (m_BorderRect.left + m_BorderWidth))
		answer |= CHatchWindow::BorderWest;
	else if (x >= (m_BorderRect.right - m_BorderWidth))
		answer |= CHatchWindow::BorderEast;

	if (y <= (m_BorderRect.top + m_BorderWidth))
		answer |= CHatchWindow::BorderNorth;
	else if (y >= (m_BorderRect.bottom - m_BorderWidth))
		answer |= CHatchWindow::BorderSouth;

	return (answer);
}

//
//		Windows message handlers
//

long CHatchWindow::HandleHatchCreate (
	HWND		hWnd,
	UINT		message
) {

#ifndef WIN32
		// In the 16-bit Windows world, get the current
		// border width from Windows
	m_BorderWidth = (LONG) ::GetProfileInt (
		"windows",
		"oleinplaceborderwidth",
		CHatchWindow::DefaultWidth);
#endif

		// Load the four sizing cursors
	m_hArrow = ::LoadCursor ((HINSTANCE) NULL, IDC_ARROW);
	m_hSizeNESW = ::LoadCursor ((HINSTANCE) NULL, IDC_SIZENESW);
	m_hSizeNWSE = ::LoadCursor ((HINSTANCE) NULL, IDC_SIZENWSE);
	m_hSizeNS = ::LoadCursor ((HINSTANCE) NULL, IDC_SIZENS);
	m_hSizeWE = ::LoadCursor ((HINSTANCE) NULL, IDC_SIZEWE);

	return (0);
}

long CHatchWindow::HandleHatchDestroy (
	HWND		hWnd,
	UINT		message
) {
		// If we get WM_DESTROY, the window is a goner!
	if (((HWND) NULL) != m_hHatchWnd)
		m_hHatchWnd = (HWND) NULL;

	return (0);
}

long CHatchWindow::HandleHatchPaint (
	HWND		hWnd,
	UINT		message
) {
	HDC			hDc;
	PAINTSTRUCT	ps;
        
	hDc = ::BeginPaint (hWnd, &ps);

	this->DrawShading (hDc);
	this->DrawHandles (hDc);

	::EndPaint (hWnd, &ps);

	return (0);
}

long CHatchWindow::HandleHatchEscapeKey (
	HWND		hWnd,
	UINT		message,
	WPARAM		wParam,
	LPARAM		lParam,
	HWND		hParent
) {
	if (m_isTracking)
	{
			// Tracking - just terminate the tracking operation
		this->DrawTrackRect (hWnd);		// Erase tracking rectangle
		
		::ReleaseCapture ();			// Let go of mouse
		
		::SetCursor (m_hArrow);			// Restore the cursor

		::DeleteObject (m_hTrackPen);	// Release the pen

		this->m_isTracking = FALSE;		// No longer tracking
		
		return (0);
	}
	else
	{
			// Not tracking - escape means exit the server
		return (::SendMessage (
			hParent,
			message,
			wParam,
			lParam));
	}
}

long CHatchWindow::HandleHatchMouseMove (
	HWND		hWnd,
	UINT		message,
	LONG		x,
	LONG		y,
	WPARAM		flags
) {
	HCURSOR		hNewCursor = m_hArrow;

	if (m_isTracking)
	{
		this->DrawTrackRect (hWnd);		// Erase tracking rectangle

		this->ComputeTrackRect (x, y);	// Figure out size of new tracking rectangle

		this->DrawTrackRect (hWnd);		// Redraw tracking rectangle

		hNewCursor = m_hTrackCursor;	// Keep same cursor while tracking
	}
	else
	{
		UINT		hitCode = this->GetHatchHitCode (x, y);

			// Select appropriate cursor depending on location
		switch (hitCode)
		{
			case CHatchWindow::BorderNE:
			case CHatchWindow::BorderSW:
				hNewCursor = m_hSizeNESW;
				break;

			case CHatchWindow::BorderNW:
			case CHatchWindow::BorderSE:
				hNewCursor = m_hSizeNWSE;
				break;

			case CHatchWindow::BorderNorth:
			case CHatchWindow::BorderSouth:
				hNewCursor = m_hSizeNS;
				break;

			case CHatchWindow::BorderWest:
			case CHatchWindow::BorderEast:
				hNewCursor = m_hSizeWE;
				break;
		}
	}

	::SetCursor (hNewCursor);

	return (0);
}

long CHatchWindow::HandleHatchLeftDown (
	HWND		hWnd,
	UINT		message,
	LONG		x,
	LONG		y,
	WPARAM		flags
) {
		// Figure out which borders are movable
	m_TrackMode = this->GetHatchHitCode (x, y);

		// Select appropriate cursor depending on tracking mode
	switch (m_TrackMode)
	{
		case CHatchWindow::BorderNE:
		case CHatchWindow::BorderSW:
			m_hTrackCursor = m_hSizeNESW;
			break;

		case CHatchWindow::BorderNW:
		case CHatchWindow::BorderSE:
			m_hTrackCursor = m_hSizeNWSE;
			break;

		case CHatchWindow::BorderNorth:
		case CHatchWindow::BorderSouth:
			m_hTrackCursor = m_hSizeNS;
			break;

		case CHatchWindow::BorderWest:
		case CHatchWindow::BorderEast:
			m_hTrackCursor = m_hSizeWE;
			break;
	}

		// Store the tracking rectangle in screen coordinates
	::GetWindowRect (hWnd, &m_TrackRect);
	m_TrackOffset.x = m_TrackRect.left;
	m_TrackOffset.y = m_TrackRect.top;
	::GetWindowRect (::GetParent (hWnd), &m_ParentRect);

        m_hTrackBrush = (HBRUSH)::GetStockObject (NULL_BRUSH);
        m_hTrackPen = ::CreatePen (PS_INSIDEFRAME, m_BorderWidth, RGB (100, 100, 100));

	this->DrawTrackRect (hWnd);			// Draw the tracking rectangle

	::SetCapture (hWnd);				// Take control of the mouse cursor
	::SetCursor (m_hTrackCursor);

	m_isTracking = TRUE;

	return (0);
}

long CHatchWindow::HandleHatchLeftUp (
	HWND		hWnd,
	UINT		message,
	LONG		x,
	LONG		y,
	WPARAM		flags
) {
	LONG		newWidth;
	LONG		newHeight;

		// Ignore this message if we're not tracking
	if (m_isTracking)
	{
		this->DrawTrackRect (hWnd);		// Erase tracking rectangle

		this->ComputeTrackRect (x, y);	// Figure out size of new tracking rectangle

		::ReleaseCapture ();			// Let go of mouse
		
		::SetCursor (m_hArrow);			// Restore the cursor

		::DeleteObject (m_hTrackPen);	// Release the pen

		this->m_isTracking = FALSE;		// No longer tracking

		newWidth = m_TrackRect.right - m_TrackRect.left - (2 * m_BorderWidth);
		newHeight = m_TrackRect.bottom - m_TrackRect.top - (2 * m_BorderWidth);

			// We have a new window size - send self a message to that effect
		::SendMessage (hWnd, WM_USER_HATCH_RESIZE, SIZE_RESTORED,
			((newHeight & 0xFFFF) << 16)
				| (newWidth & 0xFFFF));
	}

	return (0);
}

//
//		Hatch window controls
//

void CHatchWindow::WindowInit (
	HWND		hParent,
	HINSTANCE	hInst
) {
	if ((((HWND) NULL) == hParent) || (((HINSTANCE) NULL) == hInst))
		return;

	m_hHatchWnd = CreateWindow (
		pHatchWindowClass,
		"Hatch Window",
		WS_CHILDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
		0,
		0,
		0,
		0,
		hParent,
		(HMENU) NULL,
		hInst,
		0);
}

	// Display the window
void CHatchWindow::ShowHatchWindow (
	void
) {
	::ShowWindow (m_hHatchWnd, SW_SHOW);
}

	// Hide the window
void CHatchWindow::HideHatchWindow (
	void
) {
	::ShowWindow (m_hHatchWnd, SW_HIDE);
}

	// Set the new boundaries
void CHatchWindow::SetHatchWindowSize (
	LPCRECT		lpPosRect,
	LPCRECT		lpClipRect,
	LPSIZE		lpOffset
) {
	RECT		VisibleRect;
	SIZE		curOffset;

	if (!::IsWindow (m_hHatchWnd))
		return;

		// Adjust the hatch rectangle to include the border
	m_BorderRect = *lpPosRect;
	::InflateRect (
		&m_BorderRect,
		(int) m_BorderWidth,
		(int) m_BorderWidth);

		// Stay inside the clip rect
	::IntersectRect (&VisibleRect, &m_BorderRect, lpClipRect);

	::MoveWindow (
		m_hHatchWnd,
		VisibleRect.left,
		VisibleRect.top,
		VisibleRect.right - VisibleRect.left,
		VisibleRect.bottom - VisibleRect.top,
		TRUE);
	::InvalidateRect (m_hHatchWnd, NULL, TRUE);

		// Compute the window offset
	curOffset.cx = m_BorderRect.left
		- VisibleRect.left - m_BorderRect.left;
	curOffset.cy = m_BorderRect.top
		- VisibleRect.top - m_BorderRect.top;
	::OffsetRect (&m_BorderRect, curOffset.cx, curOffset.cy);

		// Tell the caller where to put the document window
	lpOffset->cx = curOffset.cx;
	lpOffset->cy = curOffset.cy;
}
	
//
//		OLE interfaces
//

STDMETHODIMP CHatchWindow::QueryInterface (
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

STDMETHODIMP_ (ULONG) CHatchWindow::AddRef (
	void
) {
		// Increment and return the reference count
	return (++m_nRefCount);
}

STDMETHODIMP_ (ULONG) CHatchWindow::Release (
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
//		Internal utility functions
//

	// Draw the handles for the hatch window
void CHatchWindow::DrawHandles (
	HDC		hDc
) {
	HBRUSH	hBrush;
	HPEN	hPen;
	HBRUSH	hOldBrush;
	HPEN	hOldPen;
	int		OldBackgroundMode;
	int		dx, dy;			// Handle spacing

		// Set background mode
	OldBackgroundMode = ::SetBkMode (hDc, TRANSPARENT);

		// Set the brush and pen we need
        hBrush = (HBRUSH)::GetStockObject (BLACK_BRUSH);
        hOldBrush = (HBRUSH)::SelectObject (hDc, hBrush);
        hPen = (HPEN)::GetStockObject (BLACK_PEN);
        hOldPen = (HPEN)::SelectObject (hDc, hPen);

		// Draw the four corner handles
	::Rectangle (						// Top left
		hDc,
		m_BorderRect.left,
		m_BorderRect.top,
		m_BorderRect.left + m_BorderWidth,
		m_BorderRect.top + m_BorderWidth);
	::Rectangle (						// Bottom left
		hDc,
		m_BorderRect.left,
		m_BorderRect.bottom - m_BorderWidth,
		m_BorderRect.left + m_BorderWidth,
		m_BorderRect.bottom);
	::Rectangle (						// Top right
		hDc,
		m_BorderRect.right - m_BorderWidth,
		m_BorderRect.top,
		m_BorderRect.right,
		m_BorderRect.top + m_BorderWidth);
	::Rectangle (						// Bottom right
		hDc,
		m_BorderRect.right - m_BorderWidth,
		m_BorderRect.bottom - m_BorderWidth,
		m_BorderRect.right,
		m_BorderRect.bottom);

		// Compute the offsets for the side handles
	dx = (1 + m_BorderRect.right - m_BorderRect.left
		- m_BorderWidth) / 2;
	dy = (1 + m_BorderRect.bottom - m_BorderRect.top
		- m_BorderWidth) / 2;

		// Draw the four side handles
	::Rectangle (						// Top
		hDc,
		m_BorderRect.left + dx,
		m_BorderRect.top,
		m_BorderRect.left + dx + m_BorderWidth,
		m_BorderRect.top + m_BorderWidth);
	::Rectangle (						// Bottom
		hDc,
		m_BorderRect.left + dx,
		m_BorderRect.bottom - m_BorderWidth,
		m_BorderRect.left + dx + m_BorderWidth,
		m_BorderRect.bottom);
	::Rectangle (						// Left
		hDc,
		m_BorderRect.left,
		m_BorderRect.top + dy,
		m_BorderRect.left + m_BorderWidth,
		m_BorderRect.top + dy + m_BorderWidth);
	::Rectangle (						// Right
		hDc,
		m_BorderRect.right - m_BorderWidth,
		m_BorderRect.top + dy,
		m_BorderRect.right,
		m_BorderRect.top + dy + m_BorderWidth);

		// Figure out where the object border should be drawn
	RECT		outlineRect = m_BorderRect;

	outlineRect.left += m_BorderWidth - 1;
	outlineRect.top += m_BorderWidth - 1;
	outlineRect.right -= m_BorderWidth - 1;
	outlineRect.bottom -= m_BorderWidth - 1;

		// Draw the outline border
	::FrameRect (hDc, &outlineRect, hBrush);

		// Restore the original pen, brush, and background mode, and we're done
	::SelectObject (hDc, hOldPen);
	::SelectObject (hDc, hOldBrush);
	::SetBkMode (hDc, OldBackgroundMode);
}

	// Draw the hatch pattern
void CHatchWindow::DrawShading (
	HDC			hDc
) {
		// Build a pattern brush from a bitmap
	WORD		hatchPattern [] = {0x11, 0x22, 0x44, 0x88, 0x11, 0x22, 0x44, 0x88};
	HBITMAP		hBitMap = ::CreateBitmap (8, 8, 1, 1, hatchPattern);
	HBRUSH		hBrush = ::CreatePatternBrush (hBitMap);
        HBRUSH          hOldBrush = (HBRUSH)::SelectObject (hDc, hBrush);
	COLORREF	oldTextColor = ::SetTextColor (hDc, RGB (255, 255, 255));
	COLORREF	oldBackgroundColor = ::SetBkColor (hDc, RGB (0, 0, 0));

		// Draw the top edge
	::PatBlt (
		hDc,
		m_BorderRect.left,
		m_BorderRect.top,
		m_BorderRect.right - m_BorderRect.left,
		(int) m_BorderWidth,
		ROP_3_DPa);

		// Draw the left edge
	::PatBlt (
		hDc,
		m_BorderRect.left,
		m_BorderRect.top,
		(int) m_BorderWidth,
		m_BorderRect.bottom - m_BorderRect.top,
		ROP_3_DPa);
	
		// Draw the right edge
	::PatBlt (
		hDc,
		m_BorderRect.right - m_BorderWidth,
		m_BorderRect.top,
		(int) m_BorderWidth,
		m_BorderRect.bottom - m_BorderRect.top,
		ROP_3_DPa);
	
		// Draw the bottom edge
	::PatBlt (
		hDc,
		m_BorderRect.left,
		m_BorderRect.bottom - m_BorderWidth,
		m_BorderRect.right - m_BorderRect.left,
		(int) m_BorderWidth,
		ROP_3_DPa);

		// Restore the original colors and brush
	::SetTextColor (hDc, oldTextColor);
	::SetBkColor (hDc, oldBackgroundColor);
	::SelectObject (hDc, hOldBrush);
	::DeleteObject (hBrush);
	::DeleteObject (hBitMap);
}

	// Draw the tracking rectangle
void CHatchWindow::DrawTrackRect (
	HWND		hWnd
) {
	HDC			hDc = ::GetDC ((HWND) NULL);

	::SetROP2 (hDc, R2_XORPEN);
	::SelectObject (hDc, m_hTrackBrush);
	::SelectObject (hDc, m_hTrackPen);

		// Draw using parent window coordinates
	::Rectangle (
		hDc,
		m_TrackRect.left,
		m_TrackRect.top,
		m_TrackRect.right,
		m_TrackRect.bottom);

	::ReleaseDC (hWnd, hDc);
}

	// Figure out new size of tracking rectangle
void CHatchWindow::ComputeTrackRect (
	LONG		x,
	LONG		y
) {
		// Convert mouse position to screen coordinates
	x += m_TrackOffset.x;
	y += m_TrackOffset.y;

	if (0 != (CHatchWindow::BorderNorth & m_TrackMode))
	{
		register LONG maximum = m_TrackRect.bottom - m_BorderWidth
			- CHatchWindow::MinimumSize;

		if (y < m_ParentRect.top)
			m_TrackRect.top = m_ParentRect.top;
		else if (y > maximum)
			m_TrackRect.top = maximum;
		else
			m_TrackRect.top = y;
	}

	if (0 != (CHatchWindow::BorderSouth & m_TrackMode))
	{
		register LONG minimum = m_TrackRect.top + m_BorderWidth + CHatchWindow::MinimumSize;

		if (y > m_ParentRect.bottom)
			m_TrackRect.bottom = m_ParentRect.bottom;
		else if (y < minimum)
			m_TrackRect.bottom = minimum;
		else
			m_TrackRect.bottom = y;
	}

	if (0 != (CHatchWindow::BorderWest & m_TrackMode))
	{
		register LONG maximum = m_TrackRect.right - m_BorderWidth - CHatchWindow::MinimumSize;

		if (x < m_ParentRect.left)
			m_TrackRect.left = m_ParentRect.left;
		else if (x > maximum)
			m_TrackRect.left = maximum;
		else
			m_TrackRect.left = x;
	}

	if (0 != (CHatchWindow::BorderEast & m_TrackMode))
	{
		register LONG minimum = m_TrackRect.left + m_BorderWidth + CHatchWindow::MinimumSize;

		if (x > m_ParentRect.right)
			m_TrackRect.right = m_ParentRect.right;
		else if (x < minimum)
			m_TrackRect.right = minimum;
		else
			m_TrackRect.right = x;
	}
}
