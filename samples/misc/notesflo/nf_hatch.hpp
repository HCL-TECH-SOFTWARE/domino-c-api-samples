/****************************************************************************
 *
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
//		nf_hatch.hpp
//
//	Definition of CHatchWindow class
//

#ifndef _NF_HATCH_HPP_
#define _NF_HATCH_HPP_		1

//
//		Constants and macros
//

	// Raster operation codes
#define ROP_3_DPa	(0x00A000C9L)

class CImageApplication;

//
//		CHatchWindow class
//

class CHatchWindow : public IUnknown
{
	private:
		HWND		m_hHatchWnd;		// Window handle
		LONG		m_BorderWidth;		// Width of the border
		RECT		m_BorderRect;		// Perimeter of border
		BOOL		m_isTracking;		// Are we tracking the mouse?
		UINT		m_TrackMode;		// Current tracking mode
		RECT		m_TrackRect;		// Current tracking rectangle in screen coordinates
		RECT		m_ParentRect;		// Screen location of parent window
		POINT		m_TrackOffset;		// Offset between window and screen coordinates
		HBRUSH		m_hTrackBrush;		// Brush for tracking rectangle
		HPEN		m_hTrackPen;		// Pen for tracking rectangle
		HCURSOR		m_hTrackCursor;		// Cursor to use when tracking
		HCURSOR		m_hSizeNESW;		// Sizing cursor handles
		HCURSOR		m_hSizeNWSE;
		HCURSOR		m_hSizeNS;
		HCURSOR		m_hSizeWE;
		HCURSOR		m_hArrow;

		LONG		m_nRefCount;		// Reference counter

	public:
		enum {
			DefaultWidth = 4,			// Default border width
			MinimumSize = 9,			// Smallest area allowed
			BorderWest = 0x01,			// Encoding bits for hit
			BorderEast = 0x02,			// detection and tracking
			BorderNorth = 0x04,
			BorderSouth = 0x08,
			BorderNW = BorderNorth | BorderWest,
			BorderNE = BorderNorth | BorderEast,
			BorderSW = BorderSouth | BorderWest,
			BorderSE = BorderSouth | BorderEast
		};
		
		//
		//		Class management
		//

		CHatchWindow (void);
		~CHatchWindow (void);

		//
		//		Member data functions
		//

		HWND GetHatchWindowHandle (void) const
			{ return (m_hHatchWnd); }
		LONG GetHatchBorderWidth (void) const
			{ return (m_BorderWidth); }
		UINT GetHatchHitCode (LONG x, LONG y) const;

		//
		//		Windows message handlers
		//

		long HandleHatchCreate (HWND hWnd, UINT message);
		long HandleHatchDestroy (HWND hWnd, UINT message);
		long HandleHatchPaint (HWND hWnd, UINT message);
		long HandleHatchEscapeKey (HWND hWnd, UINT message,
			WPARAM wParam, LPARAM lParam, HWND hParent);
		long HandleHatchMouseMove (HWND hWnd, UINT message, LONG x, LONG y, WPARAM flags);
		long HandleHatchLeftDown (HWND hWnd, UINT message, LONG x, LONG y, WPARAM flags);
		long HandleHatchLeftUp (HWND hWnd, UINT message, LONG x, LONG y, WPARAM flags);

		//
		//		Hatch window controls
		//

		void WindowInit (HWND hParent, HINSTANCE hInst);
		void ShowHatchWindow (void);
		void HideHatchWindow (void);
		void SetHatchWindowSize (LPCRECT lpPosRect, LPCRECT lpClipRect, LPSIZE lpOffset);
	
		//
		//		OLE interfaces
		//

			// IUnknown methods
		STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR *ppvObj);
		STDMETHODIMP_ (ULONG) AddRef (void);
		STDMETHODIMP_ (ULONG) Release (void);

	protected:
		//
		//		Internal utility functions
		//

		void DrawHandles (HDC hDc);
		void DrawShading (HDC hDc);
		void DrawTrackRect (HWND hWnd);
		void ComputeTrackRect (LONG x, LONG y);
};

#endif
