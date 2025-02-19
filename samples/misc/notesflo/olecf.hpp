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
//		olecf.hpp
//
//	Declaration of:	CClassFactory
//	Inherits from:	IClassFactory
//

#ifndef _OLECF_HPP_
#define _OLECF_HPP_		1

class CImageObject;
class CImageApplication;

class CClassFactory : public IClassFactory
{
	private:
		int			m_nRefCount;		// Reference count
		CImageApplication FAR *	m_lpApp;

	public:
		CClassFactory (CImageApplication FAR *lpApp);
		~CClassFactory (void);

			// IUnknown methods
		STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR *ppvObj);
		STDMETHODIMP_ (ULONG) AddRef (void);
		STDMETHODIMP_ (ULONG) Release (void);

			// IClassFactory methods
		STDMETHODIMP CreateInstance (LPUNKNOWN pUnkOuter,
			REFIID riid, LPVOID FAR *ppvObject);
		STDMETHODIMP LockServer (BOOL fLock);
};

#endif
