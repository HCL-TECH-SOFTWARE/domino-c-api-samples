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
//		oleobj.hpp
//
//	Declaration of:	COleObject
//	Inherits from:	IOleObject
//

#ifndef _OLEOBJ_HPP_
#define _OLEOBJ_HPP_		1

class CImageObject;

class COleObject : public IOleObject
{
	private:
		int			m_nRefCount;		// Reference count
		CImageObject FAR *	m_lpObj;	// Pointer to containing object
		BOOL		m_fOpen;

	public:
		COleObject (void)
			: m_nRefCount (0), m_lpObj ((CImageObject FAR *) NULL),
				m_fOpen (FALSE)
			{ ; }
		~COleObject (void) { ; }

			// IUnknown methods
		STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR *ppvObj);
		STDMETHODIMP_ (ULONG) AddRef (void);
		STDMETHODIMP_ (ULONG) Release (void);

			// IOleObject methods
		STDMETHODIMP SetClientSite (LPOLECLIENTSITE lpClientSite);
		STDMETHODIMP GetClientSite (LPOLECLIENTSITE FAR *ppClientSite);
		STDMETHODIMP SetHostNames (LPCOLESTR lpszContainerApp,
			LPCOLESTR lpszContainerObj);
		STDMETHODIMP Close (DWORD dwSaveOption);
		STDMETHODIMP SetMoniker (DWORD dwWhichMoniker, LPMONIKER lpMoniker);
		STDMETHODIMP GetMoniker (DWORD dwAssign, DWORD dwWhichMoniker,
			LPMONIKER FAR *ppMoniker);
		STDMETHODIMP InitFromData (LPDATAOBJECT lpDataObject,
			BOOL fCreation, DWORD dwReserved);
		STDMETHODIMP GetClipboardData (DWORD dwReserved,
			LPDATAOBJECT FAR *ppDataObject);
		STDMETHODIMP DoVerb (LONG iVerb, LPMSG lpMsg, LPOLECLIENTSITE lpActiveSite,
			LONG lindex, HWND hwndParent, LPCRECT lpPosRect);	
		STDMETHODIMP EnumVerbs (LPENUMOLEVERB FAR *ppEnumOleVerb);
		STDMETHODIMP Update (void);
		STDMETHODIMP IsUpToDate (void);
		STDMETHODIMP GetUserClassID (CLSID FAR *lpClsId);
		STDMETHODIMP GetUserType (DWORD dwFormOfType, LPOLESTR FAR *ppszUserType);
		STDMETHODIMP SetExtent (DWORD dwDrawAspect, LPSIZEL lpSizel);
		STDMETHODIMP GetExtent (DWORD dwDrawAspect, LPSIZEL lpSizel);
		STDMETHODIMP Advise (LPADVISESINK lpAdvSink, LPDWORD lpdwConnection);
		STDMETHODIMP Unadvise (DWORD dwConnection);
		STDMETHODIMP EnumAdvise (LPENUMSTATDATA FAR *lplpEnumAdvise);
		STDMETHODIMP GetMiscStatus (DWORD dwAspect, LPDWORD lpStatus);
		STDMETHODIMP SetColorScheme (LPLOGPALETTE lpLogPal);

			// Support functions
		void SetObjectPtr (CImageObject FAR *lpObj) { m_lpObj = lpObj; }
};

#endif
