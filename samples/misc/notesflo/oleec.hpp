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
//		oleec.hpp
//
//	Declaration of:	CExternalConnection
//	Inherits from:	IExternalConnection
//

#ifndef _OLEEC_HPP_
#define _OLEEC_HPP_		1

class CImageObject;

class CExternalConnection : public IExternalConnection
{
	private:
		CImageObject FAR *m_lpObj;		// Containing object
		int			m_nRefCount;		// Reference count
		DWORD		m_dwStongCount;		// Strong connection count

	public:
		CExternalConnection (void)
			: m_lpObj ((CImageObject FAR *) NULL), m_nRefCount (0), m_dwStongCount (0)
		{
			;
		}

		~CExternalConnection (void) { ; }

			// IUnknown methods
		STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR *ppvObj);
		STDMETHODIMP_(ULONG) AddRef (void);
		STDMETHODIMP_(ULONG) Release (void);

			// IExternalConnection methods
		STDMETHODIMP_(DWORD) AddConnection (DWORD ExtConn, DWORD Reserved);
		STDMETHODIMP_(DWORD) ReleaseConnection (DWORD ExtConn, DWORD Reserved,
			BOOL fLastReleaseCloses);

			// Support functions
		void SetObjectPtr (CImageObject FAR *lpObj) { m_lpObj = lpObj; }
};

#endif
