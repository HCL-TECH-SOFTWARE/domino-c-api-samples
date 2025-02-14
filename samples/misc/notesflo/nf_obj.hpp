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
//		nf_obj.hpp
//
//	Declaration of CImageObject class.  CImageObject is the
//	interface between the application classes and the OLE
//	class objects.
//

#ifndef _NF_OBJ_HPP_
#define _NF_OBJ_HPP_		1

#ifndef _OLEOBJ_HPP_
#include "oleobj.hpp"
#endif

#ifndef _OLEPS_HPP_
#include "oleps.hpp"
#endif

#ifndef _OLEOD_HPP_
#include "oledo.hpp"
#endif

#ifndef _OLEIPO_HPP_
#include "oleipo.hpp"
#endif

#ifndef _OLEIPAO_HPP_
#include "oleipao.hpp"
#endif

#ifndef _OLEEC_HPP_
#include "oleec.hpp"
#endif

	// Class references
class CImageApplication;
class CImageDocument;
class CHatchWindow;

class CImageObject : public IUnknown
{
	private:
		CImageApplication FAR *m_lpApp;		// Pointer to application object
		CImageDocument FAR *m_lpDoc;		// Pointer to document object
		CHatchWindow FAR *m_lpHatch;		// Pointer to hatch window object

		int			m_nRefCount;			// Master OLE reference count
		BOOL		m_fInPlaceActive;		// Is the object In Place Active?
		BOOL		m_fInPlaceVisible;		// Is the object visible?
		BOOL		m_fUIActive;			// Is the user interface active?
		HMENU		m_hSharedMenu;			// Handle to shared menu
		HOLEMENU	m_hOleMenuDesc;			// Handle to OLE menu descriptor
		OLEMENUGROUPWIDTHS m_oleMenuGroups;	// Group widths for the shared OLE menu
		RECT		m_PosRect;				// Current position
		OLEINPLACEFRAMEINFO	m_FrameInfo;
		DWORD		m_dwRegisterTableID;	// Entry in Running Object Table
		HWND		m_hParentWindow;		// The window we're contained in

		//
		//		OLE Interfaces we use
		//

		LPOLECLIENTSITE			m_lpOleClientSite;
		LPOLEADVISEHOLDER		m_lpOleAdviseHolder;
		LPDATAADVISEHOLDER		m_lpDataAdviseHolder;
		LPOLEINPLACEFRAME		m_lpOleInPlaceFrame;
		LPOLEINPLACEUIWINDOW	m_lpOleInPlaceUIWindow;
		LPOLEINPLACESITE		m_lpOleInPlaceSite;

		//
		//		OLE interfaces we implement
		//

		COleObject				m_OleObject;
		CPersistStorage			m_PersistStorage;
		CDataObject				m_DataObject;
		COleInPlaceObject		m_OleInPlaceObject;
		COleInPlaceActiveObject	m_OleInPlaceActiveObject;
		CExternalConnection		m_ExternalConnection;

	public:
			// Class management
		CImageObject (CImageDocument FAR *lpDoc, CImageApplication FAR *lpApp);
		~CImageObject (void);

			// IUnknown methods
		STDMETHODIMP QueryInterface (REFIID riid, LPVOID FAR *ppvObj);
		STDMETHODIMP_ (ULONG) AddRef (void);
		STDMETHODIMP_ (ULONG) Release (void);

			//	Member Management
		HRESULT SetOleClientSite (LPOLECLIENTSITE lpClientSite);
		LPOLECLIENTSITE GetOleClientSite (void) const
			{ return (m_lpOleClientSite); }
		LPOLEADVISEHOLDER GetOleAdviseHolder (void) const
			{ return (m_lpOleAdviseHolder); }
		HRESULT SetDataAdviseHolderPtr (LPDATAADVISEHOLDER lpDataAdviseHolder);
		LPDATAADVISEHOLDER GetDataAdviseHolderPtr (void) const
			{ return (m_lpDataAdviseHolder); }
		LPOLEINPLACEFRAME GetOleInPlaceFramePtr (void) const
			{ return (m_lpOleInPlaceFrame); }
		LPOLEINPLACEUIWINDOW GetOleInPlaceUIWindowPtr (void) const
			{ return (m_lpOleInPlaceUIWindow); }
		LPOLEINPLACESITE GetOleInPlaceSitePtr (void) const
			{ return (m_lpOleInPlaceSite); }
		LPOLEINPLACEFRAMEINFO GetOleInPlaceFrameInfoPtr (void)
			{ return (&m_FrameInfo); }
		LPOLEOBJECT GetOleObjectPtr (void)
			{ return (&m_OleObject); }
		LPDATAOBJECT GetDataObjectPtr (void)
			{ return (&m_DataObject); }
		UINT GetObjectClipboardFormat (void) const;
		void GetUserType (LPOLESTR *lplpTypeBuf);

		DWORD GetRegisterTableID (void) const
			{ return (m_dwRegisterTableID); }
		void SetRegisterTableID (DWORD RegisterTableID)
			{ m_dwRegisterTableID = RegisterTableID; }

		HWND GetAppWindowHandle (void) const;
		HWND GetDocWindowHandle (void) const;
		CImageDocument FAR *GetDocumentPtr (void) const
			{ return (m_lpDoc); }
		void GetSize (LPSIZE lpSize) const;
		void SetSize (LPSIZE lpNewSize);

		void SetInPlaceActive (BOOL fActive)
			{ m_fInPlaceActive = fActive; }
		BOOL IsInPlaceActive (void) const
			{ return (m_fInPlaceActive); }
		BOOL IsInPlaceVisible (void) const
			{ return (m_fInPlaceVisible); }
		BOOL IsUIActive (void) const
			{ return (m_fUIActive); }

			// OLE Support functions
		HRESULT CreateAdvise (LPADVISESINK lpAdvSink, LPDWORD lpdwConnection);
		HRESULT EnumAdvise (LPENUMSTATDATA FAR *lplpEnumAdvise);
		HRESULT Unadvise (DWORD dwConnection);

			// General object operations
		void GetClassID (CLSID FAR *lpClsId);
		HRESULT Close (DWORD dwSaveOption);
		DHANDLE GetMetafilePict (void);
		void AddFrameLevelUI (void);
		void DeactivateUI (void);
		BOOL DoInPlaceActivate (LONG iVerb);
		void DoInPlaceHide (void);
		void SetImageOffsets (int xOff, int yOff);
		void SetPosition (LPCRECT lpPosRect);
		void HideAppWindow (void);
		HRESULT InPlaceDeactivate (void);
		BOOL OpenForEdit (LPOLECLIENTSITE lpSite);
		void SendOnDataChange (void);
		void AssembleMenus (void);
		void DisassembleMenus (void);
		void SetHatchWindowSize (LPCRECT lpPosRect, LPCRECT lpClipRect, LPSIZE lpOffset);
		void ResizeObject (LONG newWidth, LONG newHeight);
		BOOL IsDirty (void);
};

#endif
