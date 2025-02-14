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
//		nf_app.hpp
//
//	Declaration of the CImageApplication class.
//

#ifndef _NF_APP_HPP_
#define _NF_APP_HPP_		1

class CClassFactory;
class CImageDocument;
class CHatchWindow;

class CImageApplication : public IUnknown
{
	private:
		HINSTANCE	m_hInst;				// Application instance handle
		HWND		m_hAppWnd;				// Application window handle

		CImageDocument FAR *	m_lpDoc;	// Pointer to document object
		CHatchWindow FAR *		m_lpHatch;	// Pointer to hatch window object

		ULONG		m_nRefCount;			// OLE reference count
		BOOL		m_fStartedByOLE;		// Were we started by OLE?
		DWORD		m_dwClassRegistration;	// Cookie we get when we register

		HMENU		m_hMainMenu;			// Handle for main menu
		HMENU		m_hColorMenu;			// Handle for color menu
		HMENU		m_hHelpMenu;			// Handle for help menu

	public:
		//
		//	Object management
		//

		CImageApplication (BOOL fStartedByOle = FALSE);
		~CImageApplication (void);			// Destructor

		//
		//	Application window management
		//

		BOOL WindowInit (
			DHANDLE		hInstance,
			int			nCmdShow,
			CClassFactory FAR * FAR *lplpClassFactory,
			CImageDocument FAR *lpDoc,
			CHatchWindow FAR *lpHatch);

		//
		//	Windows message handlers
		//

			// Application window handlers
		LRESULT HandleClose (HWND hWnd, UINT message);
		LRESULT HandleCommand (HWND hWnd, UINT message,
			UINT wID, UINT wNotifyCode, HWND hwndCtl);
		LRESULT HandleCreate (HWND hWnd, UINT message,
			LPCREATESTRUCT lpCs);
		LRESULT HandleSize (HWND hWnd, UINT message, WORD sizeType,
			LONG width, LONG height);

		LRESULT HandleEscapeKey (HWND hWnd, UINT message, TCHAR key);

		//
		//	Utility functions
		//
		
		void ShowAppWindow (int nCmdShow = SW_SHOWNORMAL);
		void HideAppWindow (void);
		BOOL IsInPlaceActive (void);

		//
		//	Attribute functions
		//

		HWND GetAppWindowHandle (void) const
			{ return (m_hAppWnd); }
		HINSTANCE GetInstanceHandle (void) const
			{ return (m_hInst); }
		CImageDocument FAR *GetDocumentPtr (void) const
			{ return (m_lpDoc); }
		CHatchWindow FAR *GetHatchWindowPtr (void) const
			{ return (m_lpHatch); }
		BOOL IsStartedByOLE (void) const
			{ return (m_fStartedByOLE); }
		LPOLEINPLACEFRAME GetOleInPlaceFramePtr (void) const;
		LPOLEINPLACEFRAMEINFO GetOleInPlaceFrameInfoPtr (void) const;
		HWND GetMainMenuHandle (void) const
                        { return ((HWND)m_hMainMenu); }
		HWND GetColorMenuHandle (void) const
                        { return ((HWND)m_hColorMenu); }
		HWND GetHelpMenuHandle (void) const
                        { return ((HWND)m_hHelpMenu); }        

		//
		//	OLE support
		//

			// IUnknown methods
		STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR *ppvObj);
		STDMETHODIMP_ (ULONG) AddRef (void);
		STDMETHODIMP_ (ULONG) Release (void);

};

#endif
