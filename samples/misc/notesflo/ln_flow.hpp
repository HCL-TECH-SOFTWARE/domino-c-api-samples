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
//		ln_flow.hpp - NotesFlow controls class definition
//

#ifndef _LN_FLOW_HPP_
#define _LN_FLOW_HPP_		1

// NotesFlow code

class CImageDocument;

	// Holds 1 NotesFlow action record
class CNotesFlowAction
{
	private:
		int			m_ActionID;			// Domino and Notes magic cookie identifying this action
		LONG		m_MenuCode;			// Windows menu code
		WORD		m_NameLength;		// Length of the name
		char		m_Name[ACTION_NAME_SIZE];
										// Action name in native character set
		DWORD		m_Flags;			// Action flags

	public:
		CNotesFlowAction (void);
		~CNotesFlowAction (void);

		BOOL InitializeAction (CBufferedNotesAPI &bufAPI,
			DOC_ACTION_INFO FAR *lpActionRecord, LONG menuCode);

		inline int GetActionID (void) const { return (m_ActionID); }
		inline LONG GetMenuCode (void) const { return (m_MenuCode); }
		inline WORD GetNameLength (void) const { return (m_NameLength); }
		inline char FAR * GetNamePointer (void) { return (m_Name); }
		inline DWORD GetActionFlags (void) const { return (m_Flags); }
};

class CNotesFlowActionSet {
	private:
		int			m_ActionCount;
		CNotesFlowAction *	m_pActionList;
		LONG		m_NextMenuCode;
		HMENU		m_hActionMenu;

	public:
		CNotesFlowActionSet (void);
		~CNotesFlowActionSet (void);

		BOOL InitializeActionSet (CBufferedNotesAPI &bufAPI,
			WORD actionCount, DOC_ACTION_INFO FAR *lpActionRecords);
		BOOL ClearActionSet (void);

			// NotesFlow Actions menu support
		HMENU CreateActionMenu (void);
		inline HMENU GetActionMenuHandle (void) const { return (m_hActionMenu); }
		void DeleteActionMenu (void);
		BOOL LookupActionID (LONG menuCode, int FAR *lpActionID);
};

class CNotesFlowManager {
	protected:
			// NotesFlow clipboard formats
		UINT		m_cfNotesDocInfo;
		UINT		m_cfNoteshNote;
		UINT		m_cfNotesDocAction;
		UINT		m_cfNotesDocActionDone;

			// Buffered DLL controls
		CBufferedNotesAPI	m_BufferDLL;

			// Domino and Notes document controls
		BOOL		m_IsNoteActive;
		WORD		m_noteOpenMode;
		WORD		m_noteFlags;
		WORD		m_noteUserAccess;
		HWND		m_hDocWindow;
		DHANDLE		m_hCopyOfNote;
		DHANDLE		m_hFXNote;

			// NotesFlow actions
		CNotesFlowActionSet	m_noteActions;

			// NotesFlow state flags
		BOOL		m_IsDataUpdatePending;
		BOOL		m_IsActionPending;
		int			m_PendingActionID;
		DWORD		m_PendingActionFlags;
		BOOL		m_IsActionInProgress;

	public:
		CNotesFlowManager (void);
		~CNotesFlowManager (void);

		//
		//		Get/set member functions
		//

			// Clipboard format functions
		void InitializeClipboardFormats (void);
		inline UINT GetNotesDocInfoClipFormat (void) const
			{ return (m_cfNotesDocInfo); }
		inline UINT GetNoteshNoteClipFormat (void) const
			{ return (m_cfNoteshNote); }
		inline UINT GetNotesDocActionClipFormat (void) const
			{ return (m_cfNotesDocAction); }
		inline UINT GetNotesDocActionDoneClipFormat (void) const
			{ return (m_cfNotesDocActionDone); }

			// Is there an active note?
		inline BOOL IsNoteActive (void) const
			{ return (m_IsNoteActive); }

			// Domino and Notes Field Exchange control access
		inline WORD GetNoteOpenMode (void) const
			{ return (m_noteOpenMode); }
		inline WORD GetNoteFlags (void) const
			{ return (m_noteFlags); }
		inline WORD GetNoteUserAccess (void) const
			{ return (m_noteUserAccess); }
		inline HWND GetNotesDocWindow (void) const
			{ return (m_hDocWindow); }
		inline DHANDLE GetNoteHandle (void) const
			{ return (m_hCopyOfNote); }
		inline DHANDLE GetFXNoteHandle (void) const
			{ return (m_hFXNote); }

			// NotesFlow state requests
		inline BOOL IsDataUpdatePending (void) const
			{ return (m_IsDataUpdatePending); }
		inline void SetDataUpdatePending (void)
			{ m_IsDataUpdatePending = TRUE; }
		inline void ClearDataUpdatePending (void)
			{ m_IsDataUpdatePending = FALSE; }
		inline BOOL IsActionPending (void) const
			{ return (m_IsActionPending); }
		inline void SetActionPending (int ActionID, DWORD ActionFlags)
			{
				m_IsActionPending = TRUE;
				m_PendingActionID = ActionID;
				m_PendingActionFlags = ActionFlags;
			}
		inline void ClearActionPending (void)
			{ m_IsActionPending = FALSE; }
		inline BOOL IsActionInProgress (void) const
			{ return (m_IsActionInProgress); }
		inline void SetActionInProgress (void)
			{ m_IsActionInProgress = TRUE; }
		inline void ClearActionInProgress (void)
			{ m_IsActionInProgress = FALSE; }

			// Get the pointer to the API buffer class
		inline CBufferedNotesAPI FAR * GetBufferedAPIPtr (void)
			{ return (&m_BufferDLL); }

			// Get the pointer to the NotesFlow action set class
		inline CNotesFlowActionSet FAR * GetActionSetPtr (void)
			{ return (&m_noteActions); }

			// SetData routines
		HRESULT ProcessNotesDocInfo (CImageDocument FAR *lpDoc,
			LPSTGMEDIUM pStgMedium);
		HRESULT ProcessNotesDocActionDone (CImageDocument FAR *lpDoc,
			LPSTGMEDIUM pStgMedium);

			// GetData routines
		HRESULT ProcessNoteshNote (CImageDocument FAR *lpDoc,
			LPSTGMEDIUM pStgMedium);
		HRESULT ProcessNotesDocAction (CImageDocument FAR *lpDoc,
			LPSTGMEDIUM pStgMedium);

	protected:
			// Common code to load and initialize the Domino and Notes DLL
		BOOL LoadAndInitializeNotes (void);

			// Data load and store routines
		BOOL FetchDataFromNote (CImageDocument FAR *lpDoc, DHANDLE hNote);
		BOOL StoreDataIntoNote (CImageDocument FAR *lpDoc, DHANDLE hNote);

			// Common code for integer field values
		BOOL FetchValueFromNotes (DHANDLE hNote,
			char FAR *lpItemName, int FAR *lpAnswer);
		BOOL StoreValueIntoNotes (DHANDLE hNote,
			char FAR *lpItemName, int newValue);
};

// End NotesFlow code

#endif
