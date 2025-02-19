/****************************************************************************
 *
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
//		nf_doc.hpp
//
//	Declaration of the CImageDocument class.
//

#ifndef _NF_DOC_HPP_
#define _NF_DOC_HPP_		1

class CImageApplication;
class CImageObject;
class CImagePattern;
class CImageStore;
class CDrawEnvironment;
class CHatchWindow;
class CNotesFlowManager;

class CImageDocument : public IUnknown
{
	private:
		HWND		m_hDocWnd;					// Document window handle

		CImageApplication FAR *	m_lpApp;		// Pointer to application object
		CImageObject FAR *		m_lpObj;		// Pointer to OLE management object
		CImagePattern FAR *		m_lpPattern;	// Pointer to pattern object

		ULONG		m_nRefCount;				// OLE reference count
		BOOL		m_fIsClosing;				// Already in Close processing

		SIZE		m_CurSize;					// Current display area size
		SIZE		m_CurOffset;				// Offsets in display area

		UINT		m_cfCarpet;					// Sierpinski Carpet clipboard format

// NotesFlow code
		CNotesFlowManager	m_NotesManager;	// Control info for NotesFlow
// End NotesFlow code

	public:
		//
		//	Object management
		//

		CImageDocument (CImageApplication FAR *lpApp);
		~CImageDocument (void);			// Destructor

		//
		//	Document window management
		//

		BOOL WindowInit (HWND hAppWnd, HINSTANCE hInst);
		BOOL InvalidateWindow (const LPRECT lpRect, BOOL fErase);
		void SetDocWindow (LPRECT lpRect);

		//
		//	Windows message handlers
		//

		long DrawImage (CDrawEnvironment &DrawEnv);
		long HandlePaint (HWND hWnd, UINT message);
		long HandleEscapeKey (HWND hWnd, UINT message, TCHAR key);
		long HandleHatchResize (HWND hWnd, UINT message, WORD sizeType,
			LONG width, LONG height);

		//
		//	Utility functions
		//
		
		void ShowDocWindow (int nCmdShow = SW_SHOWNORMAL);
		void HideDocWindow (void);
		long ResizeDocWindow (RECT &newArea);

		LPOLEINPLACEFRAME GetOleInPlaceFramePtr (void) const;
		LPOLEINPLACEFRAMEINFO GetOleInPlaceFrameInfoPtr (void) const;

		BOOL IsInPlaceActive (void);

		//
		//	Attribute functions
		//

		HWND GetDocWindowHandle (void) const
			{ return (m_hDocWnd); }
		CImageApplication FAR *GetApplicationPtr (void) const
			{ return (m_lpApp); }
		CImagePattern FAR *GetPatternPtr (void) const
			{ return (m_lpPattern); }

		COLORREF GetForegroundColor (void) const;
		COLORREF GetBackgroundColor (void) const;
		
			// The Set...() functions perform low-level
			// field updates.  Calling these functions
			// does not generate any OLE data change
			// notifications.
		void SetForegroundColor (COLORREF color);
		void SetBackgroundColor (COLORREF color);
		void SetOffsets (int xOffset, int yOffset);
		void SetSizes (int xSize, int ySize);

		int GetXOffset (void) const { return (m_CurOffset.cx); }
		int GetYOffset (void) const { return (m_CurOffset.cy); }
		int GetXSize (void) const { return (m_CurSize.cx); }
		int GetYSize (void) const { return (m_CurSize.cy); }

		UINT GetDocumentClipboardFormat (void) const { return (m_cfCarpet); }

		//
		//	OLE support
		//

			// IUnknown methods
		STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR *ppvObj);
		STDMETHODIMP_ (ULONG) AddRef (void);
		STDMETHODIMP_ (ULONG) Release (void);

			// OLE object management
		HRESULT CreateObject (REFIID riid, LPVOID FAR *ppvObject);
		HRESULT Close (void);
		HRESULT CloseOleObject (void);
		void DataChanged (void);
		BOOL IsDirty (void);

// NotesFlow code
		//
		//		NotesFlow support
		//

		UINT GetNotesDocInfoClipFormat (void) const;
		UINT GetNoteshNoteClipFormat (void) const;
		UINT GetNotesDocActionClipFormat (void) const;
		UINT GetNotesDocActionDoneClipFormat (void) const;

			// SetData routines
		HRESULT ProcessNotesDocInfo (LPSTGMEDIUM pStgMedium);
		HRESULT ProcessNotesDocActionDone (LPSTGMEDIUM pStgMedium);

			// GetData routines
		HRESULT ProcessNoteshNote (LPSTGMEDIUM pStgMedium);
		HRESULT ProcessNotesDocAction (LPSTGMEDIUM pStgMedium);

			// Change data and notify container
			// Calling these routines sets the DataUpdatePending
			// flag in the NotesFlow Manager class and generates
			// a DataChanged notification to Notes.
		void UpdateForegroundColor (COLORREF newColor);
		void UpdateBackgroundColor (COLORREF newColor);
		void UpdateSizes (LONG newWidth, LONG newHeight);

			// NotesFlow Action menu support
		HMENU CreateActionMenu (void);
		HMENU GetActionMenuHandle (void);
		void DeleteActionMenu (void);
		LRESULT CheckForNotesFlowAction (LONG menuCode);

// End NotesFlow code

};

#endif
