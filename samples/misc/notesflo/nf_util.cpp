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
//		nf_util.cpp
//
//	Handy utility routines
//

	// Standard C headers
#include <stdarg.h>
#include <stdio.h>

	// Standard C++ headers
#include <fstream>
using namespace std;

	// Windows headers
#include <windows.h>

	// Domino and Notes headers
#include <global.h>
#include <olenotes.h>

	// Project headers
#include "notesflo.h"

//
//		ParseCommandLine ()
//
//	Parse the command line, checking for the special OLE flags
//

void ParseCommandLine (
	LPSTR		lpCmdLine,
	BOOL FAR *	lpRegServer,
	BOOL FAR *	lpUnregServer,
	BOOL FAR *	lpEmbedding,
	BOOL FAR *	lpMakeReg
) {
	char	buffer [256];
	int		index;

		// Pre-set the flags to FALSE
	*lpRegServer = FALSE;
	*lpUnregServer = FALSE;
	*lpEmbedding = FALSE;
	*lpMakeReg = FALSE;

		// Check every argument on the command line
	while ('\0' != *lpCmdLine)
	{
			// Skip any leading whitespace
		while (isspace (*lpCmdLine))
			lpCmdLine++;

			// Is this a switch ('-' or '/'?)
		if (('-' == *lpCmdLine) || ('/' == *lpCmdLine))
		{
			lpCmdLine++;		// Skip switch character

				// Copy the string to the buffer
			for (
				index = 0;
				('\0' != *lpCmdLine) && (!isspace (*lpCmdLine)) && (index < 255);
				index++
			) {
				buffer[index] = *(lpCmdLine++);
			}
			
			buffer [index] = '\0';

				// Is it "Embedding"?
			if (0 == ::lstrcmpi (buffer, "Embedding"))
				*lpEmbedding = TRUE;

				// Is it "RegServer"?
			if (0 == ::lstrcmpi (buffer, "RegServer"))
				*lpRegServer = TRUE;

				// Is it "UnregServer"?
			if (0 == ::lstrcmpi (buffer, "UnregServer"))
				*lpUnregServer = TRUE;

				// How about "MakeReg"?
			if (0 == ::lstrcmpi (buffer, "MakeReg"))
				*lpMakeReg = TRUE;
		}
		else
		{
				// Not a switch - ignore it
			while (('\0' != *lpCmdLine) && (!isspace (*lpCmdLine)))
				lpCmdLine++;
		}
	}
}

//
//		Registry key management
//

typedef struct
{
	char FAR *	lpKeyTemplate;
	char FAR *	lpValueTemplate;
} REGKEY;
typedef REGKEY FAR *LPREGKEY;

LPTSTR	lpRegClassID = APP_CLASS_ID_STRING;
LPTSTR	lpRegAppTitle = "NotesFlow Sample 1.0";

REGKEY RegistryTable [] =
{
		// Program name entries
	{"NotesFlow",										lpRegAppTitle},
	{"NotesFlow\\CLSID",								lpRegClassID},
	{"NotesFlow\\Insertable",							""},
	{"NotesFlow\\protocol\\StdFileEditing\\server",		"%s"},		// <path>
	{"NotesFlow\\protocol\\StdFileEditing\\verb\\0",	"&Edit"},
	{"NotesFlow\\protocol\\StdFileEditing\\verb\\1",	"&Open"},

// NotesFlow code

		// NotesFlow OLE 1 data formats
		// Note that there are 2 SetDataFormats and 2 RequestDataFormats!
		// Implicit string concatenation is used to build the registry entries.
	{"NotesFlow\\protocol\\StdFileEditing\\SetDataFormats",
														NOTES_DOCINFO_CLIP_FORMAT ","
														NOTES_DOCACTION_DONE_CLIP_FORMAT},
	{"NotesFlow\\protocol\\StdFileEditing\\RequestDataFormats",
														NOTES_HNOTE_CLIP_FORMAT ","
														NOTES_DOCACTION_CLIP_FORMAT },
// End NotesFlow code

		// Class ID entries
	{"CLSID\\%s",										lpRegAppTitle},
	{"CLSID\\%s\\ProgID",								"NotesFlow"},
	{"CLSID\\%s\\Insertable",							""},
	{"CLSID\\%s\\LocalServer32",						"%s"},		// <path>
	{"CLSID\\%s\\InprocHandler32",						"ole32.dll"},
	{"CLSID\\%s\\MiscStatus",							"32"},
	{"CLSID\\%s\\DefaultIcon",							"%s,0"},	// <path>,0
	{"CLSID\\%s\\AuxUserType\\2",						"NotesFlow"},
	{"CLSID\\%s\\AuxUserType\\3",						"NotesFlow Server 1.0"},
	{"CLSID\\%s\\Verb\\0",								"&Edit,0,2"},
	{"CLSID\\%s\\Verb\\1",								"&Open,0,2"},
	{"CLSID\\%s\\DataFormats\\GetSet\\0",				"3,1,32,1"},
		// <CF_METAFILEPICT, DVASPECT_CONTENT, TYMED_MFPICT, DATADIR_GET>

		// End marker
	{NULL, NULL}
};

	// Register the server
int RegistryRegServer (
	HINSTANCE	hInstance,
	LPSTR		lpCmdLine
) {
	char		pathBuf [MAX_PATH] = "";
	LPREGKEY	lpCurEntry = RegistryTable;

		// We need the full pathname
	::GetModuleFileName (hInstance, pathBuf, MAX_PATH);

		// Create a key for each entry in the table
	while (NULL != lpCurEntry->lpKeyTemplate)
	{
			// Format the key and value buffers
		char	keyBuffer [MAX_PATH] = "";
		char	valueBuffer [MAX_PATH] = "";

			// Put the class ID into the key string where needed
		::wsprintf (keyBuffer, lpCurEntry->lpKeyTemplate, lpRegClassID);

			// Put the path into the value string where needed
		::wsprintf (valueBuffer, lpCurEntry->lpValueTemplate, pathBuf);

			// Write the key to the registry
		::RegSetValue (HKEY_CLASSES_ROOT, keyBuffer, REG_SZ, valueBuffer,
			::strlen (valueBuffer));

		lpCurEntry++;				// On to the next table entry
	}

	return (0);
}

	// Unregister the server
int RegistryUnregServer (
	HINSTANCE	hInstance,
	LPSTR		lpCmdLine
) {
	LONG		answer;
	char		keyBuf [MAX_PATH] = "CLSID\\";

		// Create the "CLSID\<class ID>" key
	strcat (keyBuf, lpRegClassID);

		// Recursively delete the two key hierarchies
	answer = ::RegistryRecursiveDeleteKey (HKEY_CLASSES_ROOT, "NotesFlow");
	answer = ::RegistryRecursiveDeleteKey (HKEY_CLASSES_ROOT, keyBuf);

	return (0);
}

	// Verify the registry entries
int RegistryVerify (
	HINSTANCE	hInstance,
	LPSTR		lpCmdLine
) {
		// Let's just cheat and re-write the whole thing
	return (::RegistryRegServer (hInstance, lpCmdLine));
}

	// Create the .reg file
int RegistryMakeFile (
	HINSTANCE	hInstance,
	LPSTR		lpCmdLine
) {
	ofstream	outStream;
	char		pathBuf [MAX_PATH] = "";
	LPREGKEY	lpCurEntry = RegistryTable;

		// Open the output file
	outStream.open ("notesflo.reg", ios::out);

		// We need the full pathname
	::GetModuleFileName (hInstance, pathBuf, MAX_PATH);

		// Write the file prefix
	outStream << "REGEDIT" << endl << endl;

		// Write the key for each entry in the table
	while (NULL != lpCurEntry->lpKeyTemplate)
	{
			// Format the key and value buffers
		char	keyBuffer [MAX_PATH] = "";
		char	valueBuffer [MAX_PATH] = "";

			// Put the class ID into the key string where needed
		::wsprintf (keyBuffer, lpCurEntry->lpKeyTemplate, lpRegClassID);

			// Put the path into the value string where needed
		::wsprintf (valueBuffer, lpCurEntry->lpValueTemplate, pathBuf);

			// Write the key
		outStream << "HKEY_CLASSES_ROOT\\" << keyBuffer << " = " << valueBuffer << endl;

		lpCurEntry++;				// On to the next table entry
	}

		// That oughta do it!
	outStream.close ();

	return (0);
}

	// Recursively delete registry keys
	// As the name implies, this function is RECURSIVE
LONG RegistryRecursiveDeleteKey (
	HKEY		hParentKey,
	LPSTR		lpKeyName
) {
	LONG		answer;
	HKEY		hCurKey;

	answer = ::RegOpenKeyEx (
		hParentKey,
		lpKeyName,
		0,
		KEY_ALL_ACCESS,
		&hCurKey);
	if (ERROR_SUCCESS == answer)
	{
		char		keyBuf [MAX_PATH] = "";
		DWORD		keyLen;
		FILETIME	lastWrite;

		while (ERROR_SUCCESS == answer)
		{
			keyLen = MAX_PATH;
			answer = ::RegEnumKeyEx (
				hCurKey,
				0,
				keyBuf,
				&keyLen,
				NULL,
				NULL,
				NULL,
				&lastWrite);
			if (ERROR_SUCCESS == answer)
				answer = ::RegistryRecursiveDeleteKey (hCurKey, keyBuf);
		}
		::RegCloseKey (hCurKey);
	}

	return (::RegDeleteKey (hParentKey, lpKeyName));
}

//
//		Coordinate scale transformations
//

#define HIMETRIC_UNITS_PER_INCH		(2540)

	// Common code to get the necessary conversion factor from the device context
int GetPixelsPerLogicalInch (
	int		index
) {
		// Get the system's default device context
	HDC		hDc = ::GetDC (NULL);

	int		PixelsPerLogicalInch = ::GetDeviceCaps (hDc, index);

	::ReleaseDC (NULL, hDc);

	return (PixelsPerLogicalInch);
}

int XFormWidthHimetricToPixel (
	int		width
) {
	return ((int) MulDiv (
		::GetPixelsPerLogicalInch (LOGPIXELSX),
		width,
		HIMETRIC_UNITS_PER_INCH));
}

int XFormHeightHimetricToPixel (
	int		height
) {
	return ((int) MulDiv (
		::GetPixelsPerLogicalInch (LOGPIXELSY),
		height,
		HIMETRIC_UNITS_PER_INCH));
}

int XFormWidthPixelToHimetric (
	int		width
) {
	return ((int) MulDiv (
		HIMETRIC_UNITS_PER_INCH,
		width,
		::GetPixelsPerLogicalInch (LOGPIXELSX)));
}

int XFormHeightPixelToHimetric (
	int		height
) {
	return ((int) MulDiv (
		HIMETRIC_UNITS_PER_INCH,
		height,
		::GetPixelsPerLogicalInch (LOGPIXELSY)));
}
