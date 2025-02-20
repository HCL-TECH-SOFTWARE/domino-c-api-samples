/*
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
//		ln_buff.cpp - Lotus C API for Domino and Notes interface class implementation
//
//	These functions provide buffered access to the Lotus C API for Domino and Notes.
//

	// C header files
#include <stdlib.h>

	// Windows header files
#include <windows.h>

	// Domino and Notes header files
#include <global.h>
#include <oserr.h>
#include <osmisc.h>
#include <nsferr.h>
#include <nsfdata.h>

	// Project header files
#include "ln_buff.hpp"

//
//		Module-specific definitions
//

#define ENV_BUF_SIZE		4096

//
//		Utility functions
//

	// This chunk of code was repeated, so it was made
	// into a private subroutine.
void CBufferedNotesAPI::ClearPointers (
	void
) {
	m_lpfnInit = (LPNOTESINITEXTENDED) NULL;
	m_lpfnTerm = (LPNOTESTERM) NULL;
	m_lpfnNoteCopy = (LPNSFNOTECOPY) NULL;
	m_lpfnItemInfo = (LPNSFITEMINFO) NULL;
	m_lpfnGetText = (LPNSFITEMGETTEXT) NULL;
	m_lpfnSetText = (LPNSFITEMSETTEXT) NULL;
	m_lpfnTranslate = (LPOSTRANSLATE) NULL;
	m_lpfnLockObject = (LPOSLOCKOBJECT) NULL;
	m_lpfnUnlockObject = (LPOSUNLOCKOBJECT) NULL;
	m_lpfnFreeObject = (LPOSMEMFREE) NULL;
}

//
//		Class management
//

	// Constructor
CBufferedNotesAPI::CBufferedNotesAPI (
	void
) {
	m_hNotesDLL = (HMODULE) NULL;
	this->ClearPointers ();
}

	// Destructor
CBufferedNotesAPI::~CBufferedNotesAPI (
	void
) {
	if (((HMODULE) NULL) != m_hNotesDLL)
	{
			// Make sure we unload the buffer DLL!
		this->FreeNotesAPI ();
		m_hNotesDLL = (HMODULE) NULL;
	}
}

//
//		Buffer DLL management
//

	// Load the Lotus C API for Domino and Notes buffer DLL
STATUS CBufferedNotesAPI::LoadNotesAPI (
	void
) {
	LONG		answer;
	long		pathLen = MAX_PATH;
	char		pathBuffer [MAX_PATH] = "CLSID\\";
	char *		pClassId = pathBuffer + strlen (pathBuffer);

		// Get the Domino and Notes class ID from the registry
	answer = RegQueryValue (HKEY_CLASSES_ROOT, "Notes.NotesSession\\CLSID",
		pClassId, &pathLen);
	if (ERROR_SUCCESS != answer)
	{
		return (ERR_LIBLOAD2);
	}

		// Build the key for the Domino and Notes executable pathname
#ifdef _WIN32
	strcat (pathBuffer, "\\LocalServer32");
#else
	strcat (pathBuffer, "\\LocalServer");
#endif

	pathLen = MAX_PATH;

		// Get the Domino and Notes path from the registry
	answer = RegQueryValue (HKEY_CLASSES_ROOT, pathBuffer, pathBuffer, &pathLen);
	if (ERROR_SUCCESS != answer)
	{
		return (ERR_LIBLOAD2);
	}

		// Search backwards for the \ before the executable name
	while ((pathLen >= 0) && ('\\' != pathBuffer[pathLen]))
		pathLen--;
	pathBuffer[pathLen+1] = '\0';

		// Add to the PATH environment variable
	if (pathLen > 0)
	{
		char		envBuffer [ENV_BUF_SIZE];
		char *		pDest;

			// Copy new path
		strcpy (envBuffer, pathBuffer);
		strcat (envBuffer, ";");

			// Point to end of string
		pDest = envBuffer + strlen (envBuffer);

		if (0 != ::GetEnvironmentVariable ("PATH", pDest, ENV_BUF_SIZE))
		{
			::SetEnvironmentVariable ("PATH", envBuffer);
		}
	}

		// Try to load the DLL
#ifdef _WIN32
	m_hNotesDLL = ::LoadLibrary ("nnotes.dll");
#else
	m_hNotesDLL = ::LoadLibrary ("_notes.dll");
#endif
	if (((HMODULE) NULL) == m_hNotesDLL)
		return (ERR_LIBLOAD2);

		// Load the function pointers
	m_lpfnInit = (LPNOTESINITEXTENDED) GetProcAddress (
		(HINSTANCE)m_hNotesDLL, "NotesInitExtended");
	m_lpfnTerm = (LPNOTESTERM) GetProcAddress (
		(HINSTANCE)m_hNotesDLL, "NotesTerm");
	m_lpfnNoteCopy = (LPNSFNOTECOPY) GetProcAddress (
		(HINSTANCE)m_hNotesDLL, "NSFNoteCopy");
	m_lpfnItemInfo = (LPNSFITEMINFO) GetProcAddress (
		(HINSTANCE)m_hNotesDLL, "NSFItemInfo");
	m_lpfnGetText = (LPNSFITEMGETTEXT) GetProcAddress (
		(HINSTANCE)m_hNotesDLL, "NSFItemGetText");
	m_lpfnSetText = (LPNSFITEMSETTEXT) GetProcAddress (
		(HINSTANCE)m_hNotesDLL, "NSFItemSetText");
	m_lpfnTranslate = (LPOSTRANSLATE) GetProcAddress (
		(HINSTANCE)m_hNotesDLL, "OSTranslate");
	m_lpfnLockObject = (LPOSLOCKOBJECT) GetProcAddress (
		(HINSTANCE)m_hNotesDLL, "OSLockObject");
	m_lpfnUnlockObject = (LPOSUNLOCKOBJECT) GetProcAddress (
		(HINSTANCE)m_hNotesDLL, "OSUnlockObject");
	m_lpfnFreeObject = (LPOSMEMFREE) GetProcAddress (
		(HINSTANCE)m_hNotesDLL, "OSMemFree");

	return (NOERROR);
}

	// Free the buffer DLL
STATUS CBufferedNotesAPI::FreeNotesAPI (
	void
) {
		// Did the DLL load?
	if (((HMODULE) NULL) == m_hNotesDLL)
		return (ERR_LIBLOAD2);

		// Free the DLL
	::FreeLibrary ((HINSTANCE)m_hNotesDLL);
	m_hNotesDLL = (HMODULE) NULL;

		// Clear the function pointers
	this->ClearPointers ();
	
	return (NOERROR);
}

//
//		Wrappers for Lotus C API for Domino and Notes routines
//

	// Call NotesInitExtended()
STATUS CBufferedNotesAPI::BufferedNotesInitExtended (
	int		argc,
	char FAR * FAR *argv
) {
	if ((((HMODULE) NULL) == m_hNotesDLL) || (((LPNOTESINITEXTENDED) NULL) == m_lpfnInit))
		return (ERR_LIBLOAD2);

	return ((*m_lpfnInit) (argc, argv));
}

	// Call NotesTerm()
STATUS CBufferedNotesAPI::BufferedNotesTerm (
	void
) {
	if ((((HMODULE) NULL) == m_hNotesDLL) || (((LPNOTESTERM) NULL) == m_lpfnTerm))
		return (ERR_LIBLOAD2);

	(*m_lpfnTerm) ();

	return (NOERROR);
}
		
	// Lock a Domino and Notes memory object
STATUS CBufferedNotesAPI::BufferedNoteCopy (
	NOTEHANDLE		sourceNoteHandle,
	NOTEHANDLE FAR *lpDestNoteHandle
) {
	if ((((HMODULE) NULL) == m_hNotesDLL) || (((LPNSFNOTECOPY) NULL) == m_lpfnNoteCopy))
		return (ERR_LIBLOAD2);

	return ((*m_lpfnNoteCopy) (sourceNoteHandle, lpDestNoteHandle));
}

	// Read a TYPE_TEXT item from a note
STATUS CBufferedNotesAPI::BufferedReadTextItem (
	NOTEHANDLE	hNote,
	char FAR *	pItemName,
	WORD		bufferLength,
	char FAR *	pBuffer,
	WORD FAR *	pLength
) {
	WORD		answerLength;
	char		itemNameBuffer [MAX_PATH];
	char		dataBuffer [MAX_PATH];

	if ((((HMODULE) NULL) == m_hNotesDLL)
		|| (((LPOSTRANSLATE) NULL) == m_lpfnTranslate)
		|| (((LPNSFITEMGETTEXT) NULL) == m_lpfnGetText)
		|| (((LPNSFITEMINFO) NULL) == m_lpfnItemInfo))
	{
		return (ERR_LIBLOAD2);
	}

		// Translate the item name to LMBCS
	answerLength = (*m_lpfnTranslate) (
		OS_TRANSLATE_NATIVE_TO_LMBCS,
		pItemName,
		(WORD) strlen (pItemName),
		itemNameBuffer,
		MAX_PATH);

		// Is that item present in the note?
	if (NOERROR != ((*m_lpfnItemInfo) (
		hNote,
		itemNameBuffer,
		answerLength,
		NULL,
		NULL,
		NULL,
		NULL)))
	{
		*pBuffer = '\0';
		*pLength = 0;
		return (ERR_ITEM_NOT_FOUND);
	}

		// Fetch the data
	answerLength = (*m_lpfnGetText) (
		hNote,
		itemNameBuffer,
		dataBuffer,
		MAX_PATH);

		// Translate the data to native format
	*pLength = (*m_lpfnTranslate) (
		OS_TRANSLATE_LMBCS_TO_NATIVE,
		dataBuffer,
		answerLength,
		pBuffer,
		bufferLength);

	return (NOERROR);
}

	// Write a TYPE_TEXT item to a note
	// If the item doesn't exist, it will be added.
	// If the item exists, it will be replaced.		*/
STATUS CBufferedNotesAPI::BufferedWriteTextItem (
	NOTEHANDLE	hNote,
	char FAR *	pItemName,
	char FAR *	pData,
	WORD		dataLength
) {
	WORD		encodedDataLength;
	char		itemNameBuffer [MAX_PATH];
	char		dataBuffer [MAX_PATH];

	if ((((HMODULE) NULL) == m_hNotesDLL)
		|| (((LPOSTRANSLATE) NULL) == m_lpfnTranslate)
		|| (((LPNSFITEMSETTEXT) NULL) == m_lpfnSetText))
	{
		return (ERR_LIBLOAD2);
	}

		/* Translate the item name to LMBCS */
	(*m_lpfnTranslate) (
		OS_TRANSLATE_NATIVE_TO_LMBCS,
		pItemName,
		(WORD) strlen (pItemName),
		itemNameBuffer,
		MAX_PATH);

		/* Translate the data to LMBCS */
	encodedDataLength = (*m_lpfnTranslate) (
		OS_TRANSLATE_NATIVE_TO_LMBCS,
		pData,
		dataLength,
		dataBuffer,
		MAX_PATH);

		/* Call Domino and Notes */
	return ((*m_lpfnSetText) (
		hNote,
		itemNameBuffer,
		dataBuffer,
		encodedDataLength));
}

	// Translate text to or from LMBCS
STATUS CBufferedNotesAPI::BufferedTranslateText (
	WORD		mode,
	WORD		inputLength,
	char FAR *	lpInput,
	WORD		outputLength,
	char FAR *	lpOutput,
	WORD FAR *	lpActualLength
) {
	if ((((HMODULE) NULL) == m_hNotesDLL) || (((LPOSTRANSLATE) NULL) == m_lpfnTranslate))
		return (ERR_LIBLOAD2);

	*lpActualLength =  ((*m_lpfnTranslate) (mode, lpInput, inputLength,
		lpOutput, outputLength));
	return (NOERROR);
}
		
	// Lock a Domino and Notes memory object
void FAR * CBufferedNotesAPI::BufferedLockObject (
	DHANDLE		hObject
) {
	if ((((HMODULE) NULL) == m_hNotesDLL) || (((LPOSLOCKOBJECT) NULL) == m_lpfnLockObject))
		return ((void FAR *) NULL);

	return ((*m_lpfnLockObject) (hObject));
}

	// Unlock a Domino and Notes memory object
STATUS CBufferedNotesAPI::BufferedUnlockObject (
	DHANDLE		hObject
) {
	if ((((HMODULE) NULL) == m_hNotesDLL) || (((LPOSUNLOCKOBJECT) NULL) == m_lpfnUnlockObject))
		return (ERR_LIBLOAD2);

	(*m_lpfnUnlockObject) (hObject);
	return (NOERROR);
}

	// Free a Domino and Notes memory object
STATUS CBufferedNotesAPI::BufferedFreeObject (
	DHANDLE		hObject
) {
	if ((((HMODULE) NULL) == m_hNotesDLL) || (((LPOSMEMFREE) NULL) == m_lpfnFreeObject))
		return (ERR_LIBLOAD2);

	return ((*m_lpfnFreeObject) (hObject));
}
