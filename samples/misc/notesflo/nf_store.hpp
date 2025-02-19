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
//		nf_store.hpp
//
//	Declaration of CImageStore class.
//	CImageStore provides a layer of isolation between the OLE persistent
//	storage interface and the messy details of exactly what data is stored
//	into which streams.  The member functions on this class correspond to
//	the underlying storage and stream management steps in the IPersistStorage
//	interface.
//

#ifndef _NF_STORE_HPP_
#define _NF_STORE_HPP_		1

class CPersistStorage;
class CImageObject;

class CImageStore : public IUnknown
{
	private:
		LPSTORAGE	m_lpStorage;	// Pointer to OLE IStorage object
		LPSTREAM	m_lpStream;		// Pointer to OLE IStream object
		ULONG		m_nRefCount;	// OLE reference count

	public:
			// Object management
		CImageStore (void);
		~CImageStore (void);

			// Attribute access
		LPSTORAGE GetStoragePtr (void) const { return (m_lpStorage); }

			// IUnknown methods
		STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR *ppvObj);
		STDMETHODIMP_(ULONG) AddRef (void);
		STDMETHODIMP_(ULONG) Release (void);

			// Fully initialize a new storage
			// (Initializes this class for Load and Save)
		HRESULT InitStore (LPSTORAGE lpStg, CImageObject FAR *lpObj);

			// Open an existing storage
			// (Initializes this class for Load and Save)
		HRESULT OpenStore (LPSTORAGE lpStg, CImageObject FAR *lpObj);

			// Read data for the object
		HRESULT LoadData (CImageObject FAR *lpObj);
	
			// Perform an incremental save of the data
		HRESULT IncrementalSave (CImageObject FAR *lpObj);

			// Save all data for the object
		HRESULT FullSave (CImageObject FAR *lpObj);

	protected:
			// Common code to release held storages and streams
		void ReleaseStore (void);
};

#endif
