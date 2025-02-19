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
//		oleps.hpp
//
//	Declaration of:	CPersistStorage
//	Inherits from:	IPersistStorage
//

#ifndef _OLEPS_HPP_
#define _OLEPS_HPP_		1

class CImageObject;
class CImageStore;

class CPersistStorage : public IPersistStorage
{
	public:
		enum {
			STATE_UNINITIALIZED = 0,
			STATE_SCRIBBLE = 1,
			STATE_NO_SCRIBBLE = 2,
			STATE_HANDS_OFF = 3
		};

	private:
		CImageObject FAR *m_lpObj;		// Containing object
		int			m_nRefCount;		// Reference count
		int			m_State;			// Current storage state
		CImageStore FAR *m_lpImageStore;	// Store object

	public:
		CPersistStorage (void);
		~CPersistStorage (void);

			// IUnknown methods
		STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR *ppvObj);
		STDMETHODIMP_(ULONG) AddRef (void);
		STDMETHODIMP_(ULONG) Release (void);

			// IPersist methods
		STDMETHODIMP GetClassID (LPCLSID lpClassID);

			// IPersistStorage methods
		STDMETHODIMP IsDirty (void);
		STDMETHODIMP InitNew (LPSTORAGE lpStg);
		STDMETHODIMP Load (LPSTORAGE lpStg);
		STDMETHODIMP Save (LPSTORAGE lpStg, BOOL fSameAsLoad);
		STDMETHODIMP SaveCompleted (LPSTORAGE lpNewStg);
		STDMETHODIMP HandsOffStorage (void);

			// Handy utility methods
		void SetObjectPtr (CImageObject FAR *lpObj) { m_lpObj = lpObj; }
		int GetState (void) const { return (m_State); }
		void SetState (int newState) { m_State = newState; }
};

#endif
