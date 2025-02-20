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
 *
 */
//
//		ln_buff.hpp - Header file for Lotus C API for Domino and Notes
//      buffer function class
//
//	These functions explicitly load the Lotus C API for Domino and Notes 
//  DLLs and bind to the API functions while the application is running.  
//  There are 3 reasons for isolating the Lotus C API for Domino and Notes 
//  from the application in this way:
//
//	1)	This prevents the Lotus C API for Domino and Notes DLL from 
//      automatically beingloaded when the application is loaded. Domino 
//      and Notes does not have to be on the PATH, and the application 
//      can run even if Domino and Notes is not installed on the system 
//      or is not currently available.
//
//	2)	The names of the Lotus C API for Domino and Notes functions are 
//      "isolated" from the application's names when the application is linked.
//
//	3)	NotesInit() and NotesInitExtended() should not be called
//		when an OLE application is initializing - NotesInit() returns
//		control to the Windows kernel, and OLE 1 thinks the
//		load of the application has failed and quits.  The wrapper
//		function for NotesInitExtended() should be called when the
//		first GetData message is received.
//

#ifndef _LN_BUFF_HPP_
#define _LN_BUFF_HPP_		1

// NotesFlow code

//
//		Pointer types for the DLL functions
//

extern "C" {

		// Pointer to NotesInitExtended()
	typedef STATUS (LNPUBLIC *LPNOTESINITEXTENDED) (
		int		argc,
		char FAR * FAR *argv
	);

		// Pointer to NotesTerm()
	typedef void (LNPUBLIC *LPNOTESTERM) (
		void
	);

		// Pointer to NSFNoteCopy()
	typedef STATUS (LNPUBLIC *LPNSFNOTECOPY) (
		NOTEHANDLE	sourceNoteHandle,
		NOTEHANDLE FAR *lpDestNoteHandle
	);

		// Pointer to NSFItemInfo()
	typedef BOOL (LNPUBLIC *LPNSFITEMINFO) (
		NOTEHANDLE		hNote,
		char FAR *		pItemName,
		WORD			itemNameLength,
		BLOCKID far *	retbhItem,
		WORD far *		retDataType,
		BLOCKID far *	retbhValue,
		DWORD far *		retValueLength
	);

		// Pointer to NSFItemGetText()
	typedef WORD (LNPUBLIC *LPNSFITEMGETTEXT) (
		NOTEHANDLE	hNote,
		char FAR *	pItemName,
		char FAR *	pRetBuffer,
		WORD		bufferLength
	);

		// Pointer to NSFItemSetText()
	typedef STATUS (LNPUBLIC *LPNSFITEMSETTEXT) (
		NOTEHANDLE	hNote,
		char FAR *	pItemName,
		char FAR *	pData,
		WORD		dataLength
	);

		// Pointer to OSTranslate()
	typedef WORD (LNPUBLIC *LPOSTRANSLATE) (
		WORD		mode,
		char FAR *	lpInput,
		WORD		inputLength,
		char FAR *	lpOutput,
		WORD		outputLength
	);

		// Pointer to OSLockObject()
	typedef void FAR * (LNPUBLIC *LPOSLOCKOBJECT) (
		DHANDLE		hObject
	);

		// Pointer to OSUnlockObject()
	typedef BOOL (LNPUBLIC *LPOSUNLOCKOBJECT) (
		DHANDLE		hObject
	);

		// Pointer to OSMemFree()
	typedef STATUS (LNPUBLIC *LPOSMEMFREE) (
		DHANDLE		hObject
	);
};

class CBufferedNotesAPI {

	private:
		HMODULE				m_hNotesDLL;		// Handle for the Lotus C API for Domino and Notes DLL
		LPNOTESINITEXTENDED	m_lpfnInit;			// Pointer to initialization function
		LPNOTESTERM			m_lpfnTerm;			// Pointer to termination function
		LPNSFNOTECOPY		m_lpfnNoteCopy;		// Pointer to note copy function
		LPNSFITEMINFO		m_lpfnItemInfo;		// Pointer to ItemInfo function
		LPNSFITEMGETTEXT	m_lpfnGetText;		// Pointer to read data function
		LPNSFITEMSETTEXT	m_lpfnSetText;		// Pointer to write data function
		LPOSTRANSLATE		m_lpfnTranslate;	// Pointer to text translation function
		LPOSLOCKOBJECT		m_lpfnLockObject;	// Pointer to lock object function
		LPOSUNLOCKOBJECT	m_lpfnUnlockObject;	// Pointer to unlock object function
		LPOSMEMFREE			m_lpfnFreeObject;	// Pointer to free object function

			// Prevent copying and assigning this class
		CBufferedNotesAPI (const CBufferedNotesAPI &);
		CBufferedNotesAPI & operator= (const CBufferedNotesAPI &);

	public:
		//
		//		Class management
		//

			// Constructor
		CBufferedNotesAPI (void);

			// Destructor
		~CBufferedNotesAPI (void);

		//
		//		Information functions
		//

		inline BOOL IsLibraryLoaded (void) const
			{ return ((((HMODULE) NULL) != m_hNotesDLL) ? TRUE : FALSE); }

		//
		//		Buffer DLL management
		//

			// Load the Lotus C API for Domino and Notes buffer DLL
		STATUS LoadNotesAPI (
			void
		);

			// Free the buffer DLL
		STATUS FreeNotesAPI (
			void
		);

		//
		//		Wrappers for Lotus C API for Domino and Notes routines
		//

			// Call NotesInitExtended()
		STATUS BufferedNotesInitExtended (
			int		argc,
			char FAR * FAR *argv
		);

			// Call NotesTerm()
		STATUS BufferedNotesTerm (
			void
		);

			// Call NSFNoteCopy()
		STATUS CBufferedNotesAPI::BufferedNoteCopy (
			NOTEHANDLE		sourceNoteHandle,
			NOTEHANDLE FAR *lpDestNoteHandle
		);

			// Read a TYPE_TEXT item from a note
		STATUS BufferedReadTextItem (
			NOTEHANDLE	hNote,
			char FAR *	pItemName,
			WORD		bufferLength,
			char FAR *	pBuffer,
			WORD FAR *	pLength
		);

			// Write a TYPE_TEXT item to a note
			// If the item doesn't exist, it will be added.
			// If the item exists, it will be replaced.
		STATUS BufferedWriteTextItem (
			NOTEHANDLE	hNote,
			char FAR *	pItemName,
			char FAR *	pData,
			WORD		dataLength
		);

			// Translation codes for BufferedTranslateText()
			// (Stolen directly from osmisc.h)
		enum {
			TRANSLATE_NATIVE_TO_LMBCS =	0,	/* Translate platform-specific to LMBCS */
			TRANSLATE_LMBCS_TO_NATIVE =	1,	/* Translate LMBCS to platform-specific */
			TRANSLATE_LOWER_TO_UPPER  = 3,	/* current int'l case table */
			TRANSLATE_UPPER_TO_LOWER  = 4,	/* current int'l case table */
			TRANSLATE_UNACCENT        = 5  	/* int'l unaccenting table */
		};

			// Translate text to or from LMBCS
		STATUS BufferedTranslateText (
			WORD		mode,
			WORD		inputLength,
			char FAR *	lpInput,
			WORD		outputLength,
			char FAR *	lpOutput,
			WORD FAR *	lpActualLength
		);
		
			// Lock a Domino and Notes memory object
		void FAR * BufferedLockObject (
			DHANDLE		hObject
		);

			// Unlock a Domino and Notes memory object
		STATUS BufferedUnlockObject (
			DHANDLE		hObject
		);

			// Free a Domino and Notes memory object
		STATUS BufferedFreeObject (
			DHANDLE		hObject
		);

	private:
			// Utility functions
		void ClearPointers (void);
};

// End NotesFlow code

#endif
