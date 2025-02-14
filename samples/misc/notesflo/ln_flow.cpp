/*
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
//		ln_flow.cpp - Implementation of NotesFlow controls class
//

	// Standard C header files
#include <stdlib.h>
#include <stdio.h>

	// Windows header files
#include <windows.h>

	// Domino and Notes header files
#include <global.h>
#include <olenotes.h>
#include <oserr.h>

	// Project header files
#include "app_rsc.h"
#include "ln_buff.hpp"
#include "ln_flow.hpp"
#include "nf_doc.hpp"

// NotesFlow code

//
//		Module Data
//

	// Domino and Notes field names
char NotesFieldTitle	[] = "Title";
char NotesFieldWidth	[] = "ImageWidth";
char NotesFieldHeight	[] = "ImageHeight";
char NotesFieldFgRed	[] = "ForegroundRed";
char NotesFieldFgGreen	[] = "ForegroundGreen";
char NotesFieldFgBlue	[] = "ForegroundBlue";
char NotesFieldBgRed	[] = "BackgroundRed";
char NotesFieldBgGreen	[] = "BackgroundGreen";
char NotesFieldBgBlue	[] = "BackgroundBlue";

//
//		CNotesFlowAction Class
//

CNotesFlowAction::CNotesFlowAction (
	void
) {
	m_ActionID = 0;
	m_MenuCode = 0;
	m_NameLength = 0;
	m_Name[0] = '\0';
	m_Flags = 0;
}

CNotesFlowAction::~CNotesFlowAction (
	void
) {
	;
}

BOOL CNotesFlowAction::InitializeAction (
	CBufferedNotesAPI &		bufAPI,
	DOC_ACTION_INFO FAR *	lpActionRecord,
	LONG					menuCode
) {
	m_ActionID = lpActionRecord->ActionID;
	m_MenuCode = menuCode;
	m_NameLength = lpActionRecord->NameLength;
	
		// Hafta convert the LMBCS action name to native!!
	bufAPI.BufferedTranslateText (
		CBufferedNotesAPI::TRANSLATE_LMBCS_TO_NATIVE,
		lpActionRecord->NameLength,
		lpActionRecord->Name,
		(ACTION_NAME_SIZE - 1),
		m_Name,
		&m_NameLength);

		// Make sure the name is NUL terminated
	m_Name [m_NameLength] = '\0';

	m_Flags = lpActionRecord->Flags;

	return (TRUE);
}

//
//		CNotesFlowActionSet Class 
//

CNotesFlowActionSet::CNotesFlowActionSet (
	void
) {
	m_ActionCount = 0;
	m_pActionList = (CNotesFlowAction *) NULL;
	m_NextMenuCode = IDM_NOTESFLOW_START;
	m_hActionMenu = (HMENU) NULL;
}

CNotesFlowActionSet::~CNotesFlowActionSet (
	void
) {
	if (((CNotesFlowAction *) NULL) != m_pActionList)
		delete [] m_pActionList;

	if (((HMENU) NULL) != m_hActionMenu)
		::DestroyMenu (m_hActionMenu);
}

BOOL CNotesFlowActionSet::InitializeActionSet (
	CBufferedNotesAPI &bufAPI,
	WORD actionCount,
	DOC_ACTION_INFO FAR *lpActionRecords
) {
	int			index;

		// Release any previous record list
	if (((CNotesFlowAction *) NULL) != m_pActionList)
		delete [] m_pActionList;

		// Allocate space for the action list
	m_pActionList = new CNotesFlowAction [actionCount];
	if (((CNotesFlowAction *) NULL) == m_pActionList)
	{
		m_ActionCount = 0;
		return (FALSE);
	}

		// Initialize the actions
	m_ActionCount = actionCount;
	for (index = 0; (index < actionCount) && (m_NextMenuCode < 65536); index++)
	{
		if (!(m_pActionList[index]).InitializeAction (
			bufAPI,
			&(lpActionRecords [index]),
			m_NextMenuCode++))
		{
			this->ClearActionSet ();
			return (FALSE);
		}
	}

	return (TRUE);
}

BOOL CNotesFlowActionSet::ClearActionSet (
	void
) {
	m_ActionCount = 0;
	delete [] m_pActionList;
	m_pActionList = (CNotesFlowAction *) NULL;
	m_NextMenuCode = IDM_NOTESFLOW_START;
	if (((HMENU) NULL) != m_hActionMenu)
	{
		::DestroyMenu (m_hActionMenu);
		m_hActionMenu = (HMENU) NULL;
	}

	return (TRUE);
}

	// If there are actions to display, create the Actions menu.
	// Returns NULL if there are no actions available.
HMENU CNotesFlowActionSet::CreateActionMenu (
	void
) {
	int			index;

	if (0 != m_ActionCount)
	{
			// Create the popup menu
		m_hActionMenu = ::CreatePopupMenu ();
		if (((HMENU) NULL) == m_hActionMenu)
			return ((HMENU) NULL);

			// Add a menu entry for each displayed action
		for (index = 0; index < m_ActionCount; index++)
		{
				// Is the ACTION_FLAG_HIDE flag set?
			if (0 == (ACTION_FLAG_HIDE & (m_pActionList[index]).GetActionFlags ()))
			{
					// Nope - add the action to the menu
				::AppendMenu (
					m_hActionMenu,
					MF_STRING,
					(m_pActionList[index]).GetMenuCode (),
					(m_pActionList[index]).GetNamePointer ());
			}
		}

		return (m_hActionMenu);
	}
	else
		return ((HMENU) NULL);
}

	// Delete the Actions menu
void CNotesFlowActionSet::DeleteActionMenu (
	void
) {
	if (((HMENU) NULL) != m_hActionMenu)
	{
		::DestroyMenu (m_hActionMenu);
		m_hActionMenu = (HMENU) NULL;
	}
}

	// Look up the Action ID for a menu selection
	// *lpActionID is unchanged if no matching action is found
BOOL CNotesFlowActionSet::LookupActionID (
	LONG		menuCode,
	int FAR *	lpActionID
) {
	int			index;

	for (index = 0; index < m_ActionCount; index++)
	{
		if (menuCode == (m_pActionList[index]).GetMenuCode ())
		{
			*lpActionID = (m_pActionList[index]).GetActionID ();
			return (TRUE);
		}
	}

	return (FALSE);
}

//
//		CNotesFlowManager Class
//

	// Constructor
CNotesFlowManager::CNotesFlowManager (
	void
) {
		// Blank out the clipboard formats
	m_cfNotesDocInfo = 0;
	m_cfNoteshNote = 0;
	m_cfNotesDocAction = 0;
	m_cfNotesDocActionDone = 0;

		// Clear the document controls
	m_IsNoteActive = FALSE;
	m_noteOpenMode = 0;
	m_noteFlags = 0;
	m_noteUserAccess = 0;
	m_hDocWindow = (HWND) NULL;
	m_hCopyOfNote = (DHANDLE) NULL;
	m_hFXNote = (DHANDLE) NULL;

	m_IsDataUpdatePending = FALSE;
	m_IsActionPending = FALSE;
	m_PendingActionID = 0;
	m_PendingActionFlags = 0;
	m_IsActionInProgress = FALSE;
}

	// Destructor
CNotesFlowManager::~CNotesFlowManager (
	void
) {
	if (((DHANDLE) NULL) != m_hCopyOfNote)
	{
			// Free the memory for our copy of the note
		m_BufferDLL.BufferedFreeObject (m_hCopyOfNote);
		m_hCopyOfNote = (DHANDLE) NULL;
	}

	if (m_BufferDLL.IsLibraryLoaded())
	{
		m_BufferDLL.BufferedNotesTerm ();
		m_BufferDLL.FreeNotesAPI ();
	}
}

	// Load the NotesFlow clipboard formats
void CNotesFlowManager::InitializeClipboardFormats (
	void
) {
	m_cfNotesDocInfo = ::RegisterClipboardFormat (NOTES_DOCINFO_CLIP_FORMAT);
	m_cfNoteshNote = ::RegisterClipboardFormat (NOTES_HNOTE_CLIP_FORMAT);
	m_cfNotesDocAction = ::RegisterClipboardFormat (NOTES_DOCACTION_CLIP_FORMAT);
	m_cfNotesDocActionDone = ::RegisterClipboardFormat (NOTES_DOCACTION_DONE_CLIP_FORMAT);
}

	// Process the NotesDocInfo request
HRESULT CNotesFlowManager::ProcessNotesDocInfo (
	CImageDocument FAR *lpDoc,
	LPSTGMEDIUM	pStgMedium
) {
	HRESULT		hResult = ResultFromScode (S_OK);
	NOTES_DOC_INFO_MSG far *lpDocInfoMessage;

		// If there's already an active note, free the resources
	if ((m_IsNoteActive) && (((DHANDLE) NULL) != m_hCopyOfNote))
	{
			// Free the copy we made of the original note
		m_BufferDLL.BufferedFreeObject (m_hCopyOfNote);
		m_hCopyOfNote = (DHANDLE) NULL;
		m_noteActions.ClearActionSet ();
	}

		// Lock the memory
	lpDocInfoMessage = (NOTES_DOC_INFO_MSG far *) ::GlobalLock (pStgMedium->hGlobal);
	if (((NOTES_DOC_INFO_MSG far *) NULL) == lpDocInfoMessage)
		return (ResultFromScode (E_INVALIDARG));

		// Is this a version we understand?
	if (NOTES_DOC_INFO_VERSION1 != lpDocInfoMessage->Version)
	{
		hResult = ResultFromScode (E_INVALIDARG);		// Nope!
	}
		// Has the buffer DLL been loaded?
	else if (!m_BufferDLL.IsLibraryLoaded())
	{
		if (!(this->LoadAndInitializeNotes ()))
			hResult = ResultFromScode (E_FAIL);
	}

	if (SUCCEEDED (hResult))
	{
		STATUS		status;

			// Copy info from the message
		m_noteOpenMode = lpDocInfoMessage->DocOpenMode;
		m_noteFlags = lpDocInfoMessage->DocFlags;
		m_noteUserAccess = lpDocInfoMessage->UserAccess;
		m_hDocWindow = lpDocInfoMessage->hDocWnd;
		m_hFXNote = (DHANDLE) lpDocInfoMessage->hFXNote;

			// Make an in-memory copy of the note for our use
		status = m_BufferDLL.BufferedNoteCopy (
			lpDocInfoMessage->hNote,
			&m_hCopyOfNote);
		if (NOERROR != status)
		{
			if (ERR_MEMORY == status)
				hResult = ResultFromScode (E_OUTOFMEMORY);
			else
				hResult = ResultFromScode (E_FAIL);
		}
		else
		{
			hResult = ResultFromScode (S_OK);
		}
	}

	if (SUCCEEDED (hResult))
	{
			// If there are any actions, initialize those
		if ((0 != lpDocInfoMessage->cNumDocActions)
			&& (((DHANDLE) NULL) != lpDocInfoMessage->hDocActions))
		{
			DOC_ACTION_INFO FAR *lpActionRecords;

				// OSLockObject() on the action array
			lpActionRecords = (DOC_ACTION_INFO FAR *) m_BufferDLL.BufferedLockObject (
				lpDocInfoMessage->hDocActions);
			if (((DOC_ACTION_INFO FAR *) NULL) == lpActionRecords)
				hResult = ResultFromScode (E_FAIL);
			else
			{
					// Copy data out of action array
				if (m_noteActions.InitializeActionSet (
					m_BufferDLL,
					lpDocInfoMessage->cNumDocActions,
					lpActionRecords))
				{
					hResult = ResultFromScode (S_OK);
				}
				else
				{
					hResult = ResultFromScode (E_FAIL);
				}

					// Unlock the action array
				m_BufferDLL.BufferedUnlockObject (lpDocInfoMessage->hDocActions);
			}
		}

			// If there's an action array, free it
		if (((DHANDLE) NULL) != lpDocInfoMessage->hDocActions)
		{
			m_BufferDLL.BufferedFreeObject (lpDocInfoMessage->hDocActions);
		}

			// If all is well . . .
		if (SUCCEEDED (hResult))
		{
			m_IsNoteActive = TRUE;		// we now have an active note

				// If there's any field data in the note,
				// use that instead of the defaults.
			this->FetchDataFromNote (lpDoc, lpDocInfoMessage->hNote);
		}
	}

		// Unlock the storage medium, and we're done!
	::GlobalUnlock (pStgMedium->hGlobal);

	return (hResult);
}

	// Process a NotesDocActionDone message
HRESULT CNotesFlowManager::ProcessNotesDocActionDone (
	CImageDocument FAR *lpDoc,
	LPSTGMEDIUM pStgMedium
) {
	HRESULT		hResult = ResultFromScode (S_OK);
	NOTES_DOCACTION_DONE_MSG far *lpActionDoneMessage;

		// If there's no active note, there's nothing to do
	if (!m_IsNoteActive)
		return (ResultFromScode (E_FAIL));

		// Lock the memory
	lpActionDoneMessage =
		(NOTES_DOCACTION_DONE_MSG far *) ::GlobalLock (pStgMedium->hGlobal);
	if (((NOTES_DOCACTION_DONE_MSG far *) NULL) == lpActionDoneMessage)
		return (ResultFromScode (E_INVALIDARG));

	if (m_IsActionInProgress)
	{
		if (lpActionDoneMessage->ActionID == m_PendingActionID)
		{
				// Need to update status display
		}

		this->ClearActionInProgress ();
	}

		// Unlock the storage medium, and we're done!
	::GlobalUnlock (pStgMedium->hGlobal);

	return (hResult);
}

	// Process a NoteshNote request
HRESULT CNotesFlowManager::ProcessNoteshNote (
	CImageDocument FAR *lpDoc,
	LPSTGMEDIUM pStgMedium
) {
	HRESULT		hResult = ResultFromScode (S_OK);
	NOTES_HNOTE_MSG far *lphNoteMessage;
	STATUS		status;

		// If there's no active note, there's nothing to do
	if (!m_IsNoteActive)
		return (ResultFromScode (E_FAIL));

		// Is this in response to an update from this server?
	if (!m_IsDataUpdatePending)
	{
			// Nope!

			// Clear the STGMEDIUM structure
		pStgMedium->hGlobal = (HGLOBAL) NULL;
		pStgMedium->tymed = TYMED_NULL;
		pStgMedium->pUnkForRelease = (IUnknown *) NULL;

			// Return the "Unexpected" error
		return (ResultFromScode (E_UNEXPECTED));
	}

		// Clear the flag
	this->ClearDataUpdatePending ();

		// Allocate the message block (which will be released by Domino and Notes)
	pStgMedium->hGlobal = ::GlobalAlloc (
		GMEM_SHARE | GMEM_MOVEABLE,
		sizeof (NOTES_HNOTE_MSG));
	if (((HGLOBAL) NULL) == pStgMedium->hGlobal)
		return (ResultFromScode (E_OUTOFMEMORY));

		// Fill in the rest of the STGMEDIUM structure
	pStgMedium->tymed = TYMED_HGLOBAL;
	pStgMedium->pUnkForRelease = (IUnknown *) NULL;

		// Lock the memory
	lphNoteMessage =
		(NOTES_HNOTE_MSG far *) ::GlobalLock (pStgMedium->hGlobal);
	if (((NOTES_HNOTE_MSG far *) NULL) == lphNoteMessage)
	{
			// Haven't a clue what's wrong, so return!
		::GlobalFree (pStgMedium->hGlobal);
		return (ResultFromScode (E_OUTOFMEMORY));
	}

		// Make a copy of the note
	status = m_BufferDLL.BufferedNoteCopy (
		m_hCopyOfNote,
		&(lphNoteMessage->hNote));
	if (NOERROR != status)
	{
		if (ERR_MEMORY == status)
			hResult = ResultFromScode (E_OUTOFMEMORY);
		else
			hResult = ResultFromScode (E_FAIL);
	}
	else
	{
			// Store the new fields into the new note
		if (this->StoreDataIntoNote (lpDoc, lphNoteMessage->hNote))
			hResult = ResultFromScode (S_OK);
		else
			hResult = ResultFromScode (E_FAIL);
	}

		// Unlock the storage medium, and we're done!
	::GlobalUnlock (pStgMedium->hGlobal);

	return (hResult);
}

	// Process a NotesDocAction request
HRESULT CNotesFlowManager::ProcessNotesDocAction (
	CImageDocument FAR *lpDoc,
	LPSTGMEDIUM pStgMedium
) {
	HRESULT		hResult = ResultFromScode (S_OK);
	NOTES_EXECUTE_DOCACTION_MSG far *lpDocActionMessage;
	int			index;

		// If there's no active note, there's nothing to do
	if (!m_IsNoteActive)
		return (ResultFromScode (E_FAIL));

		// Is this in response to an update from this server?
	if (!m_IsActionPending)
	{
			// Nope!

			// Clear the STGMEDIUM structure
		pStgMedium->hGlobal = (HGLOBAL) NULL;
		pStgMedium->tymed = TYMED_NULL;
		pStgMedium->pUnkForRelease = (IUnknown *) NULL;

			// Return the "Unexpected" error
		return (ResultFromScode (E_UNEXPECTED));
	}

		// Clear the flag
	this->ClearActionPending ();

		// Allocate the message block (which will be released by Domino and Notes)
	pStgMedium->hGlobal = ::GlobalAlloc (
		GMEM_SHARE | GMEM_MOVEABLE,
		sizeof (NOTES_EXECUTE_DOCACTION_MSG));
	if (((HGLOBAL) NULL) == pStgMedium->hGlobal)
		return (ResultFromScode (E_OUTOFMEMORY));

		// Fill in the rest of the STGMEDIUM structure
	pStgMedium->tymed = TYMED_HGLOBAL;
	pStgMedium->pUnkForRelease = (IUnknown *) NULL;

		// Lock the memory
	lpDocActionMessage =
		(NOTES_EXECUTE_DOCACTION_MSG far *) ::GlobalLock (pStgMedium->hGlobal);
	if (((NOTES_EXECUTE_DOCACTION_MSG far *) NULL) == lpDocActionMessage)
	{
			// Haven't a clue what's wrong, so return!
		::GlobalFree (pStgMedium->hGlobal);
		return (ResultFromScode (E_OUTOFMEMORY));
	}

	lpDocActionMessage->ActionID = m_PendingActionID;
	lpDocActionMessage->Flags = m_PendingActionFlags;
	for (index = 0; index < 3; index++)
		lpDocActionMessage->Unused[index] = 0;

		// Set the ActionInProgress flag
	this->SetActionInProgress ();

		// Need to set the status display

		// Unlock the storage medium, and we're done!
	::GlobalUnlock (pStgMedium->hGlobal);

	return (hResult);
}

	// Load the Domino and Notes DLL and call the buffered NotesInit() function
BOOL CNotesFlowManager::LoadAndInitializeNotes (
	void
) {
	STATUS		status;

	status = m_BufferDLL.LoadNotesAPI ();
	if (NOERROR == status)
	{
		status = m_BufferDLL.BufferedNotesInitExtended (0, NULL);
	}

	if (NOERROR == status)
		return (TRUE);
	else
		return (FALSE);
}

	// Read the field values from Domino and Notes
BOOL CNotesFlowManager::FetchDataFromNote (
	CImageDocument FAR *lpDoc,
	DHANDLE			hNote
) {
	int			colorRed;
	int			colorGreen;
	int			colorBlue;
	COLORREF	fgColor;
	COLORREF	bgColor;
	int			imageWidth;
	int			imageHeight;

		// If we don't get any color info from Domino and Notes,
		// keep the current color.
	fgColor = lpDoc->GetForegroundColor ();
	bgColor = lpDoc->GetBackgroundColor ();

		// Fetch Domino and Notes fields & convert data
		// Note:  the hNote field in the DocInfoMessage is only valid
		//		during the SetData call, so it's not stored in the
		//		Domino and Notes control object.

		// Get the foreground red color value
	if (!(this->FetchValueFromNotes (hNote, NotesFieldFgRed, &colorRed)))
	{
		colorRed = GetRValue (fgColor);
	}

		// Get the foreground green color value
	if (!(this->FetchValueFromNotes (hNote, NotesFieldFgGreen, &colorGreen)))
	{
		colorGreen = GetGValue (fgColor);
	}

		// Get the foreground blue color value
	if (!(this->FetchValueFromNotes (hNote, NotesFieldFgBlue, &colorBlue)))
	{
		colorBlue = GetBValue (fgColor);
	}

		// Combine the components to create the new foreground color
	fgColor = RGB (colorRed, colorGreen, colorBlue);

	lpDoc->SetForegroundColor (fgColor);

		// Get the background red color value
	if (!(this->FetchValueFromNotes (hNote, NotesFieldBgRed, &colorRed)))
	{
		colorRed = GetRValue (bgColor);
	}

		// Get the background green color value
	if (!(this->FetchValueFromNotes (hNote, NotesFieldBgGreen, &colorGreen)))
	{
		colorGreen = GetGValue (bgColor);
	}

		// Get the background blue color value
	if (!(this->FetchValueFromNotes (hNote, NotesFieldBgBlue, &colorBlue)))
	{
		colorBlue = GetBValue (bgColor);
	}

		// Combine the components to create the new background color
	bgColor = RGB (colorRed, colorGreen, colorBlue);

	lpDoc->SetBackgroundColor (bgColor);

		// Get the image width
	if (!(this->FetchValueFromNotes (hNote, NotesFieldWidth, &imageWidth)))
	{
		imageWidth = lpDoc->GetXSize ();
	}

		// Get the image height
	if (!(this->FetchValueFromNotes (hNote, NotesFieldHeight, &imageHeight)))
	{
		imageHeight = lpDoc->GetYSize ();
	}

	lpDoc->SetSizes (imageWidth, imageHeight);

	return (TRUE);
}

	// Update the Domino and Notes field values
BOOL CNotesFlowManager::StoreDataIntoNote (
	CImageDocument FAR *lpDoc,
	DHANDLE			hNote
) {
	COLORREF		fgColor = lpDoc->GetForegroundColor ();
	COLORREF		bgColor = lpDoc->GetBackgroundColor ();

		// Store the components of the foreground color
	if (!(this->StoreValueIntoNotes (
		hNote,
		NotesFieldFgRed,
		GetRValue (fgColor))))
	{
		return (FALSE);
	}

	if (!(this->StoreValueIntoNotes (
		hNote,
		NotesFieldFgGreen,
		GetGValue (fgColor))))
	{
		return (FALSE);
	}

	if (!(this->StoreValueIntoNotes (
		hNote,
		NotesFieldFgBlue,
		GetBValue (fgColor))))
	{
		return (FALSE);
	}

		// Store the components of the background color
	if (!(this->StoreValueIntoNotes (
		hNote,
		NotesFieldBgRed,
		GetRValue (bgColor))))
	{
		return (FALSE);
	}

	if (!(this->StoreValueIntoNotes (
		hNote,
		NotesFieldBgGreen,
		GetGValue (bgColor))))
	{
		return (FALSE);
	}

	if (!(this->StoreValueIntoNotes (
		hNote,
		NotesFieldBgBlue,
		GetBValue (bgColor))))
	{
		return (FALSE);
	}

		// Store the image size
	if (!(this->StoreValueIntoNotes (
		hNote,
		NotesFieldWidth,
		lpDoc->GetXSize ())))
	{
		return (FALSE);
	}

	if (!(this->StoreValueIntoNotes (
		hNote,
		NotesFieldHeight,
		lpDoc->GetYSize ())))
	{
		return (FALSE);
	}

	return (TRUE);
}

	//	Read a field from Domino and Notes and convert to an integer
BOOL CNotesFlowManager::FetchValueFromNotes (
	NOTEHANDLE	hNote,
	char FAR *	lpItemName,
	int FAR *	lpAnswer
) {
	STATUS		status;
	char		fieldBuffer [MAX_PATH];
	WORD		fieldLength;

	fieldBuffer [0] = '\0';
	fieldLength = 0;
	status = m_BufferDLL.BufferedReadTextItem (
		hNote,
		lpItemName,
		MAX_PATH,
		fieldBuffer,
		&fieldLength);
	if (NOERROR == status)
	{
		fieldBuffer[fieldLength] = '\0';
		*lpAnswer = atoi (fieldBuffer);

		return (TRUE);
	}
	else
		return (FALSE);
}

	// Convert integer to text and append to note
BOOL CNotesFlowManager::StoreValueIntoNotes (
	NOTEHANDLE	hNote,
	char FAR *	lpItemName,
	int			newValue
) {
	STATUS		status;
	char		fieldBuffer [MAX_PATH];

	sprintf (fieldBuffer, "%d", newValue);

	status = m_BufferDLL.BufferedWriteTextItem (
		hNote,
		lpItemName,
		fieldBuffer,
		(WORD) strlen (fieldBuffer));
	if (NOERROR == status)
		return (TRUE);
	else
		return (FALSE);
}

// End NotesFlow code
