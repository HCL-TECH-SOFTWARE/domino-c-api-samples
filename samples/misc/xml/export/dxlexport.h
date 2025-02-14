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

    PROGRAM:   dxlexport

    MODULE:    dxlexport.h

    DESCRIPTION:
	Constant definitions and function prototypes for API program dxlexport.

****************************************************************************/

/* What will be exported */
#define	EXP_ACL				1
#define EXP_DATABASE		2

/* Structure contianing information needed */
struct ExportOptions
{
	int		whatToExport;			/* What we are exporting in this session */
	char	*fileDatabase;			/* NSF Database to export */
	char	*noteSpec;				/* Design Name to export */
	NOTEID	noteID;					/* A specific NoteID to export */
	WORD	noteClassMask;			/* Note class mask if exporting notes based on note class */
	char	*docDTD;				/* FileName of user supplied DTD */
	BOOL	forceNoteFormat;		/* If specifying -n option then set appropriately */
	BOOL	richTextOption;			/* If specifying -r option then set appropriately */
	FILE	*pOutputFile;			/* Pointer to the export output file specified */
	char	*serverName;			/* Server Name if supplied */
	DWORD	totalBytesWritten;		/* Total exported bytes written */
}expOptions;

/* Error Definitions */
#define ERR_INVALID_ARGS	1

/* Local function prototypes */
void    LNPUBLIC  PrintUsage();
void PrintAPIError (STATUS);
STATUS LNPUBLIC  ProcessArgs (int argc, char *argv[], struct ExportOptions *expOptions);
STATUS LNPUBLIC ExportData( struct ExportOptions *expOptions, DBHANDLE hDB );
STATUS LNPUBLIC ExportSetofNotes( struct ExportOptions *expOptions, DBHANDLE hDB );
STATUS LNPUBLIC SetExportOptions(DXLEXPORTHANDLE hDXLExport, struct ExportOptions *expOptions);
void LNCALLBACK DXLExportStreamFunc(const BYTE *bytes, DWORD length, void far *pExAction );
void LNCALLBACK CheckDXLErrorLog( DXLEXPORTHANDLE hDXLExport);
STATUS LNPUBLIC AddIDUnique (void far * phNoteIDTable, SEARCH_MATCH far *pSearchMatch, ITEM_TABLE far *summary_info);
