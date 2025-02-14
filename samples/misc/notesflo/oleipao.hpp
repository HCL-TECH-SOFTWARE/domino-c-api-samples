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
//		oleipao.hpp
//
//	Declaration of:	COleInPlaceActiveObject
//	Inherits from:	IOleInPlaceActiveObject
//

#ifndef _OLEIPAO_HPP_
#define _OLEIPAO_HPP_		1

class CImageObject;

class COleInPlaceActiveObject : public IOleInPlaceActiveObject
{
	private:
		CImageObject FAR *m_lpObj;		// Containing object
		int			m_nRefCount;		// Reference count

	public:
		COleInPlaceActiveObject (void)
			: m_lpObj ((CImageObject FAR *) NULL), m_nRefCount (0)
		{
			;
		}

		~COleInPlaceActiveObject (void) { ; }

			// IUnknown methods
		STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR *ppvObj);
		STDMETHODIMP_(ULONG) AddRef (void);
		STDMETHODIMP_(ULONG) Release (void);

			// IOleWindow methods
		STDMETHODIMP GetWindow (HWND FAR *lphWnd);
		STDMETHODIMP ContextSensitiveHelp (BOOL fEnterMode);

			// IOleInPlaceActiveObject methods
		STDMETHODIMP TranslateAccelerator (LPMSG lpMsg);
		STDMETHODIMP OnFrameWindowActivate (BOOL fActivate);
		STDMETHODIMP OnDocWindowActivate (BOOL fActivate);
		STDMETHODIMP ResizeBorder (LPCRECT prcBorder,
			LPOLEINPLACEUIWINDOW pUIWindow, BOOL fFrameWindow);
		STDMETHODIMP EnableModeless (BOOL fEnable);

			// Support functions
		void SetObjectPtr (CImageObject FAR *lpObj) { m_lpObj = lpObj; }
};

#endif
