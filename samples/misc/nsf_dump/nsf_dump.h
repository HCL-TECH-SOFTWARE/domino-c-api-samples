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
 *

    PROGRAM:   nsf_dump

    MODULE:    nsf_dump.h

    DESCRIPTION:
	Constant definitions and function prototypes for API program
	nsf_dump.

****************************************************************************/
#include <colorods.h>   /*
			 * need to include here, so UNIX won't complain on DumpColorValue()
			 * function prototype
			 */

#include <editods.h>   /*
			 * need to include here, so UNIX won't complain on DumpLengthValue()
			 * function prototype
			 */
					
/*
 *              Data Types
 */

	/* Table entries for code-to-string conversion.         */
	/* Used by GetLiteralString(). Each entry contains:     */
	/*      1)  Domino and Notes constant                   */
	/*      2)  Pointer to null-terminated output string    */
	/* Table entries are stored in a DWORD, so any          */
	/* size API constant can use the same table.  The       */
	/* last entry must have a constant value of             */
	/* LITERAL_TABLE_END (0xFFFFFFFF), and is usually       */
	/* given a string like "Unknown".                       */
typedef struct
{
	DWORD           dwValue;
	char *          pName;
} LITERAL_NAME_ENTRY;

	/* Internal handle for a CD file */
typedef FILE * CDFILEHANDLE;

#define LITERAL_TABLE_END       (0xFFFFFFFF)

	/* Table at the start of a CD Storage file */
typedef struct {
	WORD    segmentCount;
	WORD    segmentLengths[1];
} NSF_DUMP_CD_FILE_SEGMENT_TABLE;

/* Global Variables */
extern  NOTEHANDLE  hNote;              /* handle to currently open note */
extern  FILE       *dumpfile;
extern  DWORD       dwItem;
extern  DWORD       dwCDRecord;
extern  DWORD           dwActionRecord;
extern  DWORD           dwNavigateRecord;
extern  LITERAL_NAME_ENTRY *TypeStringTablePtr;         /* Type name strings */
extern  LITERAL_NAME_ENTRY *ClassStringTablePtr;        /* Class name strings */
extern  BOOL ACLDigestItemFlag;
/* constants */
#define LINELENGTH  68
#define DEFAULT_FORMULA "@ALL"
#define MAX_FORMULA_LEN 256
/* NUM_NOTE_CLASSES dimensions both NoteClassTable and ObjectClassTable */
#define NUM_NOTE_CLASSES    13

/* functions in nsf_dump.c */
STATUS  LNPUBLIC  ProcessArgs( int, char**, FORMULAHANDLE* );
void    LNPUBLIC  PrintUsage( void );
STATUS  LNPUBLIC  DumpDBHeaderData( DBHANDLE, char * );
STATUS  LNPUBLIC  ProcessFormula( char*, FORMULAHANDLE* );
STATUS  LNPUBLIC  DumpOneNote( void far*,SEARCH_MATCH far*,ITEM_TABLE far* );
STATUS  LNPUBLIC  DumpDelStub( DBHANDLE, DWORD *);
STATUS  LNPUBLIC  GetNoteClassAndDumpCount( DBHANDLE, NOTEID, char *, WORD, DWORD* );
STATUS  LNPUBLIC  DumpNoteHeaderData( NOTEHANDLE );
#if defined (PROFILE)
STATUS  LNPUBLIC  DumpProfile( DBHANDLE, void far*, char *, WORD, char *, WORD, NOTEID );
#endif
void    LNPUBLIC  PrintOid( OID* );
void    LNPUBLIC  PrintNoteClass( WORD );
void    LNPUBLIC  PrintNotePrivileges( WORD );
void    LNPUBLIC  PrintNoteFlags( WORD );
STATUS  LNPUBLIC  DumpOneItem( WORD, WORD, char far *, WORD, void far *, DWORD, void far * );
void    LNPUBLIC  PrintText( char far*, DWORD );
void    LNPUBLIC  NSFDDUMPDumpMemory( char * pData, WORD wSize );
void    LNPUBLIC  WordCount( char * pText, WORD wTextLen );
void    LNPUBLIC  PrintUNID( char * pIndent, UNID *pUnid );
void    LNPUBLIC  PrintNOTELINK( char * pIndent, NOTELINK *pLink);
void    LNPUBLIC  PrintFormula( char far *pFormula, WORD formulaLen );
void    LNPUBLIC  PrintScriptSource( char far *pScript, DWORD scriptLen );
void    LNPUBLIC  PrintScriptObject( char far *pScript, DWORD scriptLen );
STATUS  LNPUBLIC  PrintCompositeBuffer (char FAR *pBuffer, DWORD ItemValueLength,
			ActionRoutinePtr ActionRoutine, void FAR *pContext);
char *  LNPUBLIC  GetLiteralString (LITERAL_NAME_ENTRY *pTable, DWORD dwValue);
void    LNPUBLIC  PrintCanonicalValue (char *pValue, WORD wType, WORD wValueLen);
void    LNPUBLIC  PrintOleGuid (void *pOleGuid);
void    LNPUBLIC  PrintNotesActions (char FAR * pBuffer, DWORD BufferLen);
void    LNPUBLIC  DumpLengthValue( LENGTH_VALUE  );

/* functions in dumpitem.c */
void    LNPUBLIC  GetFieldFlagsString( WORD, char **);
void    LNPUBLIC  GetTDString( TIMEDATE *, char * );
void    LNPUBLIC  DumpTextlistItem( char *, WORD );
void    LNPUBLIC  DumpCompoundTextItem( char *, WORD, char *, WORD );
void    LNPUBLIC  DumpNumberItem( char *, WORD );
void    LNPUBLIC  DumpNumberRangeItem( char *, WORD );
void    LNPUBLIC  DumpTimeItem( char *, WORD );
void    LNPUBLIC  DumpTimeRangeItem( char *, WORD );
void    LNPUBLIC  DumpUseridItem( char *, WORD );
void    LNPUBLIC  DumpObjectItem( char *, WORD, char *, WORD );
void    LNPUBLIC  DumpNotelinkListItem( char *, WORD );
void    LNPUBLIC  DumpUserDataItem( char *, WORD );
	/* New data types in Release 4.0 */
void    LNPUBLIC  DumpAgentQueryItem ( char * Name, WORD NameLength,
	char *  pData, WORD length );
void    LNPUBLIC  DumpActionItem ( char * Name, WORD NameLength,
	char *  pData, WORD length );
void    LNPUBLIC  DumpViewmapDatasetItem ( char * Name, WORD NameLength,
	char *  pData, WORD length );
void    LNPUBLIC  DumpViewmapLayoutItem ( char * Name, WORD NameLength,
	char *  pData, WORD length );
void    LNPUBLIC  DumpLotusScriptObjectItem ( char * Name, WORD NameLength,
	char *  pData, WORD length );
void    LNPUBLIC  DumpHTMLItem ( char * Name, WORD NameLength,
	char *  pData, WORD  length );

/* functions in dumpcd.c */
STATUS  LNPUBLIC  DumpOneCDRecord( char*, WORD, DWORD, void* );
void    LNPUBLIC  DumpCDParagraph( char *, DWORD );
void    LNPUBLIC  DumpCDPabdefinition( char *, DWORD );
void    LNPUBLIC  DumpCDPabreference( char *, DWORD );
void    LNPUBLIC  DumpCDText( char *, DWORD, WORD );
void    LNPUBLIC  DumpCDHeader( char *, DWORD );
void    LNPUBLIC  DumpCDLinkexport2( char *, DWORD );
void    LNPUBLIC  DumpCDBitmapheader( char *, DWORD );
void    LNPUBLIC  DumpCDBitmapsegment( char *, DWORD );
void    LNPUBLIC  DumpCDColortable( char *, DWORD );
void    LNPUBLIC  DumpCDGraphic( char *, DWORD );
void    LNPUBLIC  DumpCDPmmetaseg( char *, DWORD );
void    LNPUBLIC  DumpCDWinmetaseg( char *, DWORD );
void    LNPUBLIC  DumpCDMacmetaseg( char *, DWORD );
void    LNPUBLIC  DumpCDCgmmeta( char *, DWORD );
void    LNPUBLIC  DumpCDPmmetaheader( char *, DWORD );
void    LNPUBLIC  DumpCDWinmetaheader( char *, DWORD );
void    LNPUBLIC  DumpCDMacmetaheader( char *, DWORD );
void    LNPUBLIC  DumpCDTablebegin( char *, DWORD, WORD );
void    LNPUBLIC  DumpCDTablecell( char *, DWORD, WORD );
void    LNPUBLIC  DumpCDTableend( char *, DWORD, WORD );
void    LNPUBLIC  DumpCDStylename( char *, DWORD );
void    LNPUBLIC  DumpCDField( char *, DWORD, WORD );
void    LNPUBLIC  DumpCDDocument( char *, DWORD );
void    LNPUBLIC  DumpCDMetafile( char *, DWORD );
void    LNPUBLIC  DumpCDBitmap( char *, DWORD );
void    LNPUBLIC  DumpCDFonttable( char *, DWORD );
void    LNPUBLIC  DumpCDLink( char *, DWORD );
void    LNPUBLIC  DumpCDLinkexport( char *, DWORD );
void    LNPUBLIC  DumpCDKeyword( char *, DWORD );
void    LNPUBLIC  DumpCDLink2( char *, DWORD );
void    LNPUBLIC  DumpCDCgm( char *, DWORD );
void    LNPUBLIC  DumpCDTiff( char *, DWORD );
void    LNPUBLIC  DumpCDPatterntable( char *, DWORD );
void    LNPUBLIC  DumpCDDdebegin( char *, DWORD );
void    LNPUBLIC  DumpCDDdeend( char *, DWORD );
void    LNPUBLIC  DumpCDOlebegin( char *, DWORD );
void    LNPUBLIC  DumpCDOleend( char *, DWORD );
void    LNPUBLIC  DumpCDHotspotbegin( char *, DWORD, WORD );
void    LNPUBLIC  DumpCDHotspotend( char *, DWORD, WORD );
void    LNPUBLIC  DumpCDButton( char *, DWORD );
void    LNPUBLIC  DumpFontIDFields( FONTIDFIELDS * );
void    LNPUBLIC  DumpColorValue( COLOR_VALUE  );

	/* New in Release 4.0 */
void    LNPUBLIC  DumpCDBar( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDExtfield( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDPabhide( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDPabformref( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDActionbar( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDAction( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDDocautolaunch( char * RecordPtr, DWORD RecordLength );
	/* New in Release 4.5 */
void    LNPUBLIC  DumpCDOleObjInfo( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDLayout( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDLayoutEnd( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDLayoutText( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDLayoutField( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDLayoutGraphic( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDLayoutButton( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDTextEffect( char * RecordPtr, DWORD RecordLength, WORD RecordType );
void    LNPUBLIC  DumpCDTransparentTable( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDStorageLink( char * RecordPtr, DWORD RecordLength );
	/* New in Release 4.6 */
void    LNPUBLIC  DumpActiveObject ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpActiveObjectParameter ( int index, char ** ppRecord );
void    LNPUBLIC  DumpActiveObjectLink ( int index, char ** ppRecord );
void    LNPUBLIC  DumpCDAnchor ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDAltText ( char * RecordPtr, DWORD RecordLength, WORD RecordType );
void    LNPUBLIC  DumpCDAlternateBegin ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDAlternateEnd ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDHRule ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDHtmlBegin ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDHtmlEnd ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDHtmlFormula ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDHtmlHeader ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDHtmlSegment ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDLSObject ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDOleRTMarker ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpHSOleRichText ( char * RecordPtr, DWORD RecordLength );
	/* New in Release 5.0 */
void    LNPUBLIC  DumpCDColor ( char * RecordPtr, DWORD RecordLength, WORD RecordType);
void    LNPUBLIC  DumpCDBeginRecord ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDEndRecord (char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDVerticalAlign ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDFloat ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDTimerInfo ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDTableRowHeight ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDTableLabel ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDRegionBegin ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDRegionEnd ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDTransition ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDEmbeddedOutline ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDEmbeddedView ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDCellBackgroundData ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDTarget ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDMapElement ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDAreaElement ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDResource ( char * RecordPtr, DWORD RecordLength, WORD RecordType );
void    LNPUBLIC  DumpCDEmbeddedCtl ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDEvent ( char * RecordPtr, DWORD RecordLength, WORD RecordType );
void    LNPUBLIC  DumpCDIDName ( char * RecordPtr, DWORD RecordLength, WORD RecordType );
void    LNPUBLIC  DumpCDPreTableBegin ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDBorderInfo ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDEmbeddedSchedCtl ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDExt2Field ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDActionBarExt ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDImageSegment ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDImageHeader ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDBlobPart ( char * RecordPtr, DWORD RecordLength, WORD RecordType ); /* modified in Notes/Domino 6 */
void    LNPUBLIC  DumpCDPlaceHolder ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDLinkColors ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDCaption ( char * RecordPtr, DWORD RecordLength );
	/* New in Release 5.0.1 */
void	LNPUBLIC  DumpCDFrame ( char * RecordPtr, DWORD RecordLength );
void	LNPUBLIC  DumpCDFrameSet ( char * RecordPtr, DWORD RecordLength );
void	LNPUBLIC  DumpCDFrameSetHeader ( char * RecordPtr, DWORD RecordLength );
	/* New in Notes/Domino 6 */
void	LNPUBLIC  DumpCDActionExt ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDEmbeddedSchedCtlExtra ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDInline ( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDEmbeddedCalCtl ( char * RecordPtr, DWORD RecordLength );
void	LNPUBLIC  DumpCDPositioning ( char * RecordPtr, DWORD RecordLength );
void	LNPUBLIC  DumpCDLayer ( char * RecordPtr, DWORD RecordLength );
void	LNPUBLIC  DumpCDBoxSize ( char * RecordPtr, DWORD RecordLength );
void	LNPUBLIC  DumpCDBackgroundProperties ( char * RecordPtr, DWORD RecordLength );
void	LNPUBLIC  DumpCDEventEntry ( char * RecordPtr, DWORD RecordLength );
void	LNPUBLIC  DumpCDFileHeader ( char * RecordPtr, DWORD RecordLength );
void	LNPUBLIC  DumpCDFileSegment ( char * RecordPtr, DWORD RecordLength );
void	LNPUBLIC  DumpCDDECSField ( char * RecordPtr, DWORD RecordLength );
void	LNPUBLIC  DumpCDTextPropertiesTable ( char * RecordPtr, DWORD RecordLength );
void	LNPUBLIC  DumpCDSpanRecord ( char * RecordPtr, DWORD RecordLength, WORD RecordType );
void	LNPUBLIC  DumpCDEmbeddedEditCtl ( char * RecordPtr, DWORD RecordLength );
void	LNPUBLIC  DumpCDTextProperty ( char * RecordPtr, DWORD RecordLength );
void	LNPUBLIC  DumpCDDataFlags ( char * RecordPtr, DWORD RecordLength );
void	LNPUBLIC  DumpCDTablecellDataFlags ( char * RecordPtr, DWORD RecordLength );
void	LNPUBLIC  DumpCDFieldHint ( char * RecordPtr, DWORD RecordLength );
void	LNPUBLIC  DumpCDLargeParagraph ( char * RecordPtr, DWORD RecordLength );
void	LNPUBLIC  DumpCDIgnore ( char * RecordPtr, DWORD RecordLength );
void	LNPUBLIC  DumpCDEmbeddedContactList ( char * RecordPtr, DWORD RecordLength );
void	LNPUBLIC  DumpCDTableDataExtension ( char * RecordPtr, DWORD RecordLength );
void	LNPUBLIC  DumpCDEmbedExtraInfo ( char * RecordPtr, DWORD RecordLength );
/* Functions in agents.c (New in Release 4.0) */
STATUS  LNPUBLIC  DumpOneAgentQueryRecord( char*, WORD, DWORD, void* );
void    LNPUBLIC  DumpQueryHeader( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpQueryTextterm( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpQueryByfield( char * RecordPtr, DWORD RecordLength, WORD RecordType );
void    LNPUBLIC  DumpQueryFormula( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpQueryByform( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpQueryByfolder( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpQueryUsesform( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpQueryTopic( char * RecordPtr, DWORD RecordLength );
STATUS  LNPUBLIC  DumpOneActionRecord( char*, WORD, DWORD, void* );
void    LNPUBLIC  DumpActionHeader( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpActionModifyfield( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpActionReply( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpActionFormula( char * RecordPtr, DWORD RecordLength, WORD RecordType );
void    LNPUBLIC  DumpActionLotusscript( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpActionSendmail( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpActionDbcopy( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpActionDelete( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpActionByform( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpActionReadMarks( char * RecordPtr, DWORD RecordLength, WORD RecordType );
void    LNPUBLIC  DumpActionFolder( char * RecordPtr, DWORD RecordLength, WORD RecordType );
void    LNPUBLIC  DumpActionNewsletter( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpActionRunagent( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpActionSenddocument( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpActionJavaAgent( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpAgentInfoItem ( char * Name, WORD NameLength,
	char *  pData, WORD length );

	/* Functions in navigate.c (New in Release 4.0) */
STATUS  LNPUBLIC  DumpOneViewmapDatasetRecord( char*, WORD, DWORD, void* );
void    LNPUBLIC  DumpCDVmdataset( char * RecordPtr, DWORD RecordLength );
STATUS  LNPUBLIC  DumpOneViewmapLayoutRecord( char*, WORD, DWORD, void* );
void    LNPUBLIC  DumpCDVmheader( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDVmbitmap( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDVmrect( char * RecordPtr, DWORD RecordLength, WORD RecordType );
void    LNPUBLIC  DumpCDVmpolygon( char * RecordPtr, DWORD RecordLength, WORD RecordType );
void    LNPUBLIC  DumpCDVmpolyline( char * RecordPtr, DWORD RecordLength );
void    LNPUBLIC  DumpCDVmregion( char * RecordPtr, DWORD RecordLength, WORD RecordType );
void    LNPUBLIC  DumpCDVmaction( char * RecordPtr, DWORD RecordLength, WORD RecordType );
void    LNPUBLIC  DumpCDVmtextbox( char * RecordPtr, DWORD RecordLength );

/* functions in cdfile.c */
STATUS LNPUBLIC DumpCDFile( char * );
STATUS LNPUBLIC DumpCDStorageFile (char *);
STATUS LNPUBLIC NSFDUMPOpenFile( char *, CDFILEHANDLE * );
VOID   LNPUBLIC NSFDUMPCloseFile(CDFILEHANDLE);
STATUS LNPUBLIC NSFDUMPEnumCDFile(CDFILEHANDLE, DWORD dwFileSize, DWORD dwFileOffset,
	ActionRoutinePtr, /*   STATUS (LNCALLBACKPTR)(char far *, WORD, DWORD, void far *), */
	char *);
STATUS LNPUBLIC NSFDUMPGetFileSize( CDFILEHANDLE, DWORD * );
STATUS LNPUBLIC NSFDUMPReadFromFile( CDFILEHANDLE, char far *, unsigned long );
STATUS LNPUBLIC NSFDUMPSetFilePosition( CDFILEHANDLE, DWORD );
WORD NSFDUMPByteSwap (WORD input);
STATUS LNPUBLIC NSFDUMPReadSegmentTable (CDFILEHANDLE hCDFile,
	NSF_DUMP_CD_FILE_SEGMENT_TABLE ** ppTable, DWORD *pOffset);

/* Functions in mime.c (New in Release 5.0.7) */
void LNPUBLIC DumpMimePart( char *, DWORD );
void LNPUBLIC DumpRFC822Item( char *, DWORD );
void LNPUBLIC PrintItemString (char *, WORD);

#ifdef OS390
#include <_Ascii_a.h> /* NOTE: must be the LAST file included */
#endif

