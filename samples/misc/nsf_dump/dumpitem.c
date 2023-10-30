/****************************************************************************
 *
 * Copyright HCL Technologies 1996, 2023.
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
			
    FILE:       dumpitem.c

    PROGRAM:    nsf_dump

    DESCRIPTION:
      This file contains routines that print the contents of one item
      to the dumpfile. For many of the data types exposed by the API,
      (e.g. TYPE_NUMBER), this file contains a function to dump an item
      of that type (e.g. DumpNumberItem). These functions in are mostly
      called from procedure DumpOneItem() in file nsf_dump.c.

      Note: #ifdef APITESTMODE is for internal use only.
****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

#ifdef __cplusplus
extern "C" {
#endif
/* C include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* HCL C API for Notes/Domino include files */
#include <global.h>
#include <intl.h>
#include <nsf.h>
#include <nsfobjec.h>
#include <misc.h>
#include <osmem.h>
#include <osmisc.h>
#include <ostime.h>
#include <stdnames.h>
#include <textlist.h>
#include <editods.h>

/* NSF_DUMP - specific include file */
#include "nsf_dump.h"
#if defined (OS390)
#include <lapicinc.h>
#endif

/* globals */

/* data global to this file */
#define MAX_FLAGS_STRLEN    100
static  char    szFieldFlags[MAX_FLAGS_STRLEN+1];
	
/*
 * Type string table - used by GetLiteralString()
 */
char szTYPE_ERROR[]              = "TYPE_ERROR";
char szTYPE_UNAVAILABLE[]        = "TYPE_UNAVAILABLE";
char szTYPE_TEXT[]               = "TYPE_TEXT";
char szTYPE_TEXT_LIST[]          = "TYPE_TEXT_LIST";
char szTYPE_NUMBER[]             = "TYPE_NUMBER";
char szTYPE_NUMBER_RANGE[]       = "TYPE_NUMBER_RANGE";
char szTYPE_TIME[]               = "TYPE_TIME";
char szTYPE_TIME_RANGE[]         = "TYPE_TIME_RANGE";
char szTYPE_FORMULA[]            = "TYPE_FORMULA";
char szTYPE_USERID[]             = "TYPE_USERID";
char szTYPE_INVALID_OR_UNKNOWN[] = "TYPE_INVALID_OR_UNKNOWN";
char szTYPE_COMPOSITE[]          = "TYPE_COMPOSITE";
char szTYPE_COLLATION[]          = "TYPE_COLLATION";
char szTYPE_OBJECT[]             = "TYPE_OBJECT";
char szTYPE_NOTEREF_LIST[]       = "TYPE_NOTEREF_LIST";
char szTYPE_VIEW_FORMAT[]        = "TYPE_VIEW_FORMAT";
char szTYPE_ICON[]               = "TYPE_ICON";
char szTYPE_NOTELINK_LIST[]      = "TYPE_NOTELINK_LIST";
char szTYPE_SIGNATURE[]          = "TYPE_SIGNATURE";
char szTYPE_SEAL[]               = "TYPE_SEAL";
char szTYPE_SEALDATA[]           = "TYPE_SEALDATA";
char szTYPE_SEAL_LIST[]          = "TYPE_SEAL_LIST";
char szTYPE_HIGHLIGHTS[]         = "TYPE_HIGHLIGHTS";
char szTYPE_WORKSHEET_DATA[]     = "TYPE_WORKSHEET_DATA";
char szTYPE_USERDATA[]           = "TYPE_USERDATA";
char szTYPE_QUERY[]              = "TYPE_QUERY";
char szTYPE_ACTION[]             = "TYPE_ACTION";
char szTYPE_ASSISTANT_INFO[]     = "TYPE_ASSISTANT_INFO";
char szTYPE_VIEWMAP_DATASET[]    = "TYPE_VIEWMAP_DATASET";
char szTYPE_VIEWMAP_LAYOUT[]     = "TYPE_VIEWMAP_LAYOUT";
char szTYPE_LSOBJECT[]           = "TYPE_LSOBJECT";
char szTYPE_HTML[]               = "TYPE_HTML";
char szTYPE_MIME_PART[]          = "TYPE_MIME_PART";
char szTYPE_RFC822_TEXT[]        = "TYPE_RFC822_TEXT";
char szTYPE_UNKNOWN[]            = "UNKNOWN DATA TYPE";

LITERAL_NAME_ENTRY TypeStringTable [] =
{
	{TYPE_ERROR,				szTYPE_ERROR},
	{TYPE_UNAVAILABLE,			szTYPE_UNAVAILABLE},
	{TYPE_TEXT,					szTYPE_TEXT},
	{TYPE_TEXT_LIST,			szTYPE_TEXT_LIST},
	{TYPE_NUMBER,				szTYPE_NUMBER},
	{TYPE_NUMBER_RANGE,			szTYPE_NUMBER_RANGE},
	{TYPE_TIME,					szTYPE_TIME},
	{TYPE_TIME_RANGE,			szTYPE_TIME_RANGE},
	{TYPE_FORMULA,				szTYPE_FORMULA},
	{TYPE_USERID,				szTYPE_USERID},
	{TYPE_INVALID_OR_UNKNOWN,	szTYPE_INVALID_OR_UNKNOWN},
	{TYPE_COMPOSITE,			szTYPE_COMPOSITE},
	{TYPE_COLLATION,			szTYPE_COLLATION},
	{TYPE_OBJECT,				szTYPE_OBJECT},
	{TYPE_NOTEREF_LIST,			szTYPE_NOTEREF_LIST},
	{TYPE_VIEW_FORMAT,			szTYPE_VIEW_FORMAT},
	{TYPE_ICON,					szTYPE_ICON},
	{TYPE_NOTELINK_LIST,		szTYPE_NOTELINK_LIST},
	{TYPE_SIGNATURE,			szTYPE_SIGNATURE},
	{TYPE_SEAL,					szTYPE_SEAL},
	{TYPE_SEALDATA,				szTYPE_SEALDATA},
	{TYPE_SEAL_LIST,			szTYPE_SEAL_LIST},
	{TYPE_HIGHLIGHTS,			szTYPE_HIGHLIGHTS},
	{TYPE_WORKSHEET_DATA,		szTYPE_WORKSHEET_DATA},
	{TYPE_USERDATA,				szTYPE_USERDATA},
	{TYPE_QUERY,				szTYPE_QUERY},
	{TYPE_ACTION,				szTYPE_ACTION},
	{TYPE_ASSISTANT_INFO,		szTYPE_ASSISTANT_INFO},
	{TYPE_VIEWMAP_DATASET,		szTYPE_VIEWMAP_DATASET},
	{TYPE_VIEWMAP_LAYOUT,		szTYPE_VIEWMAP_LAYOUT},
	{TYPE_LSOBJECT,				szTYPE_LSOBJECT},
	{TYPE_HTML,					szTYPE_HTML},
	{TYPE_MIME_PART,			szTYPE_MIME_PART},
	{TYPE_RFC822_TEXT,			szTYPE_RFC822_TEXT},

		/* This must be the last entry! */
	{LITERAL_TABLE_END,			szTYPE_UNKNOWN}
};
LITERAL_NAME_ENTRY *TypeStringTablePtr = TypeStringTable;

/*
 *  ClassStringTable - used by GetLiteralString()
 */
char szCLASS_NOCOMPUTE[]   = "CLASS_NOCOMPUTE";
char szCLASS_ERROR[]       = "CLASS_ERROR";
char szCLASS_UNAVAILABLE[] = "CLASS_UNAVAILABLE";
char szCLASS_NUMBER[]      = "CLASS_NUMBER";
char szCLASS_TIME[]        = "CLASS_TIME";
char szCLASS_TEXT[]        = "CLASS_TEXT";
char szCLASS_FORMULA[]     = "CLASS_FORMULA";
char szCLASS_USERID[]      = "CLASS_USERID";
char szCLASS_TABLE_END[]   = "UNKNOWN TYPE CLASS";


LITERAL_NAME_ENTRY ClassStringTable [] =
{
	{CLASS_NOCOMPUTE,        szCLASS_NOCOMPUTE},
	{CLASS_ERROR,            szCLASS_ERROR},
	{CLASS_UNAVAILABLE,      szCLASS_UNAVAILABLE},
	{CLASS_NUMBER,           szCLASS_NUMBER},
	{CLASS_TIME,             szCLASS_TIME},
	{CLASS_TEXT,             szCLASS_TEXT},
	{CLASS_FORMULA,          szCLASS_FORMULA},
	{CLASS_USERID,           szCLASS_USERID},

		/* This must be the last entry! */
	{LITERAL_TABLE_END,		szCLASS_TABLE_END}
};
LITERAL_NAME_ENTRY *ClassStringTablePtr = ClassStringTable;

/*
 * HostTypeStringTable - used by GetLiteralString()
 */
char szHOST_MSDOS[]			= "HOST_MSDOS";
char szHOST_OLE[]			= "HOST_OLE";
char szHOST_MAC[]			= "HOST_MAC";
char szHOST_UNKNOWN[]		= "HOST_UNKNOWN";
char szHOST_HPFS[]			= "HOST_HPFS";
char szHOST_OLELIB[]		= "HOST_OLELIB";
char szHOST_BYTEARRAY_EXT[]	= "HOST_BYTEARRAY_EXT";
char szHOST_BYTEARRAY_PAGE[]= "HOST_BYTEARRAY_PAGE";
char szHOST_CDSTORAGE[]		= "HOST_CDSTORAGE";
char szHOST_STREAM[]		= "HOST_STREAM";
char szHOST_TYPE_UNKNOWN[]	= "UNKNOWN HOST TYPE";

LITERAL_NAME_ENTRY HostTypeStringTable [] =
{
	{HOST_MSDOS,			szHOST_MSDOS},
	{HOST_OLE,				szHOST_OLE},
	{HOST_MAC,				szHOST_MAC},
	{HOST_UNKNOWN,			szHOST_UNKNOWN},
	{HOST_HPFS,				szHOST_HPFS},
	{HOST_OLELIB,			szHOST_OLELIB},
	{HOST_BYTEARRAY_EXT,	szHOST_BYTEARRAY_EXT},
	{HOST_BYTEARRAY_PAGE,	szHOST_BYTEARRAY_PAGE},
	{HOST_CDSTORAGE,		szHOST_CDSTORAGE},
	{HOST_STREAM,			szHOST_STREAM},

		/* This must be the last entry! */
	{LITERAL_TABLE_END,		szHOST_TYPE_UNKNOWN}
};


/*
 * ItemFlagTable - used by GetFieldFlagsString()
 */

#define NUM_ITEM_FLAGS      8
char szITEM_SIGN[]          = "ITEM_SIGN ";
char szITEM_SEAL[]          = "ITEM_SEAL ";
char szITEM_SUMMARY[]       = "ITEM_SUMMARY ";
char szITEM_READWRITERS[]   = "ITEM_READWRITERS ";
char szITEM_NAMES[]         = "ITEM_NAMES ";
char szITEM_PLACEHOLDER[]   = "ITEM_PLACEHOLDER ";
char szITEM_PROTECTED[]     = "ITEM_PROTECTED ";
char szITEM_READERS[]       = "ITEM_READERS ";

static  struct
{
    WORD    fItemFlag;
    char   *szItemFlag;
}   ItemFlagTable[NUM_ITEM_FLAGS] = {
	ITEM_SIGN,          szITEM_SIGN,
	ITEM_SEAL,          szITEM_SEAL,
	ITEM_SUMMARY,       szITEM_SUMMARY,
	ITEM_READWRITERS,   szITEM_READWRITERS,
	ITEM_NAMES,         szITEM_NAMES,
	ITEM_PLACEHOLDER,   szITEM_PLACEHOLDER,
	ITEM_PROTECTED,     szITEM_PROTECTED,
	ITEM_READERS,       szITEM_READERS,
};

/*
 * ObjectClassTable - used by DumpObjectItem()
 * The symbolic constant NUM_NOTE_CLASSES is defined in nsf_dump.h
 */
char szNOTE_CLASS_DOCUMENT1[]    = "NOTE_CLASS_DOCUMENT";
char szNOTE_CLASS_INFO1[]        = "NOTE_CLASS_INFO";
char szNOTE_CLASS_FORM1[]        = "NOTE_CLASS_FORM";
char szNOTE_CLASS_VIEW1[]        = "NOTE_CLASS_VIEW";
char szNOTE_CLASS_ICON1[]        = "NOTE_CLASS_ICON";
char szNOTE_CLASS_DESIGN1[]      = "NOTE_CLASS_DESIGN";
char szNOTE_CLASS_ACL1[]         = "NOTE_CLASS_ACL";
char szNOTE_CLASS_HELP_INDEX1[]  = "NOTE_CLASS_HELP_INDEX";
char szNOTE_CLASS_HELP1[]        = "NOTE_CLASS_HELP";
char szNOTE_CLASS_FILTER1[]      = "NOTE_CLASS_FILTER";
char szNOTE_CLASS_FIELD1[]       = "NOTE_CLASS_FIELD";
char szNOTE_CLASS_REPLFORMULA1[] = "NOTE_CLASS_REPLFORMULA";
char szNOTE_CLASS_PRIVATE1[]		= "NOTE_CLASS_PRIVATE";

static  struct
{
    WORD    fNoteClass;                 /* NOTE_CLASS value */
    char   *szNoteClass;                /* descriptive string */
}   ObjectClassTable[NUM_NOTE_CLASSES] = {
	NOTE_CLASS_DOCUMENT,            szNOTE_CLASS_DOCUMENT1,
	NOTE_CLASS_INFO,                szNOTE_CLASS_INFO1,
	NOTE_CLASS_FORM,                szNOTE_CLASS_FORM1,
	NOTE_CLASS_VIEW,                szNOTE_CLASS_VIEW1,
	NOTE_CLASS_ICON,                szNOTE_CLASS_ICON1,
	NOTE_CLASS_DESIGN,              szNOTE_CLASS_DESIGN1,
	NOTE_CLASS_ACL,                 szNOTE_CLASS_ACL1,
	NOTE_CLASS_HELP_INDEX,          szNOTE_CLASS_HELP_INDEX1,
	NOTE_CLASS_HELP,                szNOTE_CLASS_HELP1,
	NOTE_CLASS_FILTER,              szNOTE_CLASS_FILTER1,
	NOTE_CLASS_FIELD,               szNOTE_CLASS_FIELD1,
	NOTE_CLASS_REPLFORMULA,         szNOTE_CLASS_REPLFORMULA1,
	NOTE_CLASS_PRIVATE,				  szNOTE_CLASS_PRIVATE1
};

/************************************************************************

    FUNCTION:   GetFieldFlagsString

    PURPOSE:    Get a printable string for field flags, e.g. ITEM_SUMMARY.

    DESCRIPTION:
	The second parameter is the address of a character pointer.
	This function initializes a global string szFieldFlags and
	sets the character pointer to this global string. This assumes
	the caller immediately uses the character pointer in a fprintf
	or similar funciton.  Do not save the pointer for later
	use. Subsequent calls to this funciton may change the contents
	of the global string szFieldFlags.

*************************************************************************/

void    LNPUBLIC  GetFieldFlagsString( WORD ItemFlags, char ** pszFlgStr )
{
    WORD     wTableEntry;

    szFieldFlags[0] = '\0';

    for (wTableEntry = 0; wTableEntry<NUM_ITEM_FLAGS; wTableEntry++)
    {
	if ( (ItemFlagTable[wTableEntry].fItemFlag) & ItemFlags )
	{
	    strcat( szFieldFlags, ItemFlagTable[wTableEntry].szItemFlag );
	}
    }
    *pszFlgStr = szFieldFlags;
    return;
}

/************************************************************************

   FUNCTION:   GetTDString

*************************************************************************/

void LNPUBLIC  GetTDString( TIMEDATE * ptdModified, char * szTimedate )
{
    WORD            wLen;

    ConvertTIMEDATEToText( NULL, NULL,
			    ptdModified,
			    szTimedate,
			    MAXALPHATIMEDATE,
			    &wLen );
    szTimedate[wLen] = '\0';
    return;
}

/************************************************************************

    FUNCTION:   DumpTextlistItem

*************************************************************************/

void  LNPUBLIC   DumpTextlistItem( char * pData, WORD length )
{
    LIST   *pList;
    WORD    wEntry;
    char   *pText;
    WORD    wTextSize;

    pList = (LIST*) pData;

    for (wEntry = 0; wEntry  < pList->ListEntries; wEntry++)
    {
		ListGetText( pList, FALSE, wEntry, &pText, &wTextSize );
		fprintf( dumpfile, "  Entry %d: wTextSize = %d  Text = ", wEntry, wTextSize );

		if (0 == wTextSize)
			fprintf( dumpfile, "\n");
		else
			PrintText(pText, wTextSize);
    }
    return;
}


/************************************************************************

    FUNCTION:   DumpNumberItem

    PURPOSE:    print the value of an item of TYPE_NUMBER

    DESCRIPTION:
	First use memcpy to copy the NUMBER pointed to by pData into
	a local variable. Then print the floating point double value
	using fprintf.

*************************************************************************/

void    LNPUBLIC  DumpNumberItem( char * pData, WORD wLength )
{
    double    Number;

    memcpy( (char*)&Number, pData, sizeof(NUMBER) );

#ifdef OS390
    /* Domino and Notes gave us an IEEE double which must be converted to native. */
    ConvertIEEEToDouble(&Number, &Number);
#endif /* OS390 */

    fprintf( dumpfile, "    %f\n", Number );
    return;
}

/************************************************************************

    FUNCTION:   DumpNumberRangeItem

*************************************************************************/

void    LNPUBLIC  DumpNumberRangeItem( char * pData, WORD wLength )
{
    RANGE    Range;
    WORD     wEntry;
    double   Number;

    /*   Get the RANGE structure from the location pointed to by pData */
    memcpy( (char*)&Range, pData, sizeof(RANGE) );

    /*   the first Number starts right after the RANGE structure */
    pData += sizeof(RANGE);

    /* For each item, get the number and use fprintf to display it. */
    for (wEntry = 0; wEntry < Range.ListEntries; wEntry++)
    {
	memcpy( (char*)&Number, pData, sizeof(double) );
	pData += sizeof(double);
#ifdef OS390
	/* Domino and Notes gave us an IEEE double which must be converted to native. */
	ConvertIEEEToDouble(&Number, &Number);
#endif /* OS390 */
	fprintf( dumpfile, "  Entry %d: %f\n", wEntry, Number );
    }
    return;
}

/************************************************************************

    FUNCTION:   DumpTimeItem

*************************************************************************/

void    LNPUBLIC  DumpTimeItem( char * pData, WORD wLength )
{
    char        szTimedate[MAXALPHATIMEDATE+1];

    GetTDString( (TIMEDATE*)pData, szTimedate );

#ifndef APITESTMODE
    fprintf( dumpfile, "    %s\n", szTimedate );
#endif

    return;
}


/************************************************************************

    FUNCTION:   DumpTimeRangeItem

*************************************************************************/

void    LNPUBLIC  DumpTimeRangeItem( char * pData, WORD wLength )
{
    RANGE       Range;
    TIMEDATE    TimeDate;
    TIMEDATE_PAIR TimeDatePair;
    USHORT      usListEntry;
    USHORT      usRangeEntry;
    char        szTimedate[MAXALPHATIMEDATE+1];
    char        szUpperTD[MAXALPHATIMEDATE+1];

    /* Get the RANGE from the location specified by pData */
    memcpy( (char*)&Range, pData, sizeof(RANGE) );

    /* Advance pData to point to the first TIMEDATE in the array */
    pData += sizeof(RANGE);

    /* First print the list entries, then the range entries.*/

    for (usListEntry = 0; usListEntry < Range.ListEntries; usListEntry++)
    {
	memcpy( (char*)&TimeDate, pData, sizeof(TIMEDATE) );
	pData += sizeof(TIMEDATE);
	GetTDString( &TimeDate, szTimedate );

#ifndef APITESTMODE
	fprintf( dumpfile, "  List Entry %d: %s\n", usListEntry, szTimedate );
#endif

    }

    for (usRangeEntry = 0; usRangeEntry < Range.RangeEntries; usRangeEntry++)
    {
	memcpy( (char*)&TimeDatePair, pData, sizeof(TIMEDATE_PAIR) );
	pData += sizeof(TIMEDATE_PAIR);
	GetTDString( &TimeDatePair.Lower, szTimedate );
	GetTDString( &TimeDatePair.Upper, szUpperTD );

#ifndef APITESTMODE
	fprintf( dumpfile, "  Range Entry %d: %s - %s\n", usRangeEntry,
	szTimedate, szUpperTD );
#endif

    }

    return;
}

/************************************************************************

    FUNCTION:   DumpUseridItem

    Notes V1/V2 User ID type item. Note that Domino and Notes V3+ does
    not create items of this type. Type Author fields are TYPE_TEXT
    with certain field flags set.

*************************************************************************/

void    LNPUBLIC  DumpUseridItem( char * pData, WORD wLength )
{
    WORD     wNameLen;

   /* All but the last 8 bytes of the data are the user name.
      The last 8 bytes are the License ID.
    */

    wNameLen = wLength - sizeof(LICENSEID);
    fprintf( dumpfile, "  user name = " );
    PrintText( pData, (DWORD)wNameLen );
    pData += wNameLen;
    fprintf( dumpfile, "  license ID = " );
    NSFDDUMPDumpMemory( pData, sizeof(LICENSEID) );
    return;
}


/************************************************************************

    FUNCTION:   DumpCompoundTextItem

*************************************************************************/

void  LNPUBLIC   DumpCompoundTextItem( char * Name,
				    WORD NameLength,
				    char * pData,
				    WORD length )
{
	dwCDRecord = 0;
	PrintCompositeBuffer( pData, length, DumpOneCDRecord, NULL );

    return;
}

/************************************************************************

    FUNCTION:   DumpObjectItem

*************************************************************************/

void  LNPUBLIC   DumpObjectItem( char * Name,
				    WORD NameLength,
				    char * pData,
				    WORD length )
{
    OBJECT_DESCRIPTOR   ObjectDesc;
    WORD                wObjectType;
    WORD                wObjectFlags;
    DBHANDLE            hDB;
    DWORD               dwObjectSize;
    WORD                wClass, wPrivs, i;
    FILEOBJECT          FileObject;
    WORD                wFileNameLen;
    WORD                wHostType;
    char               *szHostType;
    WORD                wCompress;
    char                szFileCreated[MAXALPHATIMEDATE+1];
    char                szFileModified[MAXALPHATIMEDATE+1];
    char               *szFileName;
    FILEOBJECT_MACEXT   MACFileObject;
    FILEOBJECT_HPFSEXT  HPFSFileObject;
    STATUS              error;
    char               *pObject;

    /* pData points to data in Domino and Notes canonical format. The first
       structure in the object item is an OBJECT_DESCRIPTOR. We want
       to convert the OBJECT_DESCRIPTOR to host format and store the
       result in ObjectDesc.

       Objects of type OBJECT_FILE consist of an OBJECT_DESCRIPTOR,
       followed by some additional information. To read the OBJECT_FILE
       properly, we need to read the entire object, starting with the
       OBJECT_DESCRIPTOR that we've already read. Since ODSReadMemory
       resets the buffer pointer to the next unread byte, we'll make a copy
       of the pointer to use in one of the calls to ODSReadMemory.
       In earlier versions of this sample program we didn't do this, and
       we weren't processing file objects properly.
    */

    pObject = pData;

    ODSReadMemory( &pObject, _OBJECT_DESCRIPTOR, &ObjectDesc, 1 );

    wObjectType = ObjectDesc.ObjectType;

    if (wObjectType != OBJECT_UNKNOWN)
    {
		/*  OBJECT_NO_COPY and OBJECT_PRESERVE are flags that may be bitwise
		    OR-ed into the object type word. If they are, store them in a
		    separate word, then mask them out of the object type word.
		 */
		wObjectFlags = ( wObjectType & (OBJECT_NO_COPY | OBJECT_PRESERVE) );
		if (wObjectFlags)
		{
		    wObjectType &= ~(OBJECT_NO_COPY | OBJECT_PRESERVE);
		}

		if ( wObjectType == OBJECT_FILE )
		{
		    fprintf( dumpfile, "  Object Type = OBJECT_FILE.\n" );
		}
		else if (wObjectType == OBJECT_FILTER_LEFTTODO)
		{
		    fprintf( dumpfile, "  Object Type = OBJECT_FILTER_LEFTTODO.\n");
		}
		else
		{
		    fprintf( dumpfile, "  Object Type %#x not recognized.\n",
			    wObjectType);
		}

		if ( wObjectFlags & OBJECT_NO_COPY )
		{
		    fprintf( dumpfile, "  ObjectFlag OBJECT_NO_COPY is set.\n");
		}

		if ( wObjectFlags & OBJECT_PRESERVE )
		{
		    fprintf( dumpfile, "  ObjectFlag OBJECT_PRESERVE is set.\n");
		}
    }
    else
    {
		fprintf( dumpfile, "  Object Type = OBJECT_UNKNOWN.\n");
    }

    NSFNoteGetInfo( hNote, _NOTE_DB, &hDB );
    if (error = NSFDbGetObjectSize(
	    hDB,
	    ObjectDesc.RRV,
	    ObjectDesc.ObjectType,
	    &dwObjectSize,
	    &wClass,
	    &wPrivs ))
    {
		fprintf( dumpfile, "   Error: unable to get object size.\n" );
		fprintf( dumpfile, "   Error code = %#x.\n", error );
    }
    else
    {

#ifndef APITESTMODE
		fprintf( dumpfile, "  Object Size = %ld bytes.\n", dwObjectSize );
#endif

		for (i=0; i<NUM_NOTE_CLASSES; i++)
		{
	    	if (wClass & ObjectClassTable[i].fNoteClass)
	    	{
			fprintf( dumpfile, "  Object Class = %s\n",
					ObjectClassTable[i].szNoteClass );
			break;
	    	}
		}

		if (i==NUM_NOTE_CLASSES)
		{
		    fprintf( dumpfile, "  Object Class Unrecognized.\n" );
		}

		fprintf( dumpfile, "  Object Privilege mask = %#x.\n", wPrivs );
    }

    if ( wObjectType != OBJECT_FILE )
    {
		return;
    }
    /* FROM HERE ON WE ARE DUMPING FILE OBJECTS ONLY */

   /* pData points to the start of the FILEOBJECT structure in
      canonical format. Convert it to host format, storing the
      result in FileObject. This advances pData to point to the
      next byte after the FILEOBJECT structure in the buffer,
      which is the statr of the original relative file path name.
    */
    ODSReadMemory( &pData, _FILEOBJECT, &FileObject, 1 );

    /* file name */
    wFileNameLen = FileObject.FileNameLength;
    if (szFileName = (char *) malloc(wFileNameLen+1))
    {
		memcpy( szFileName, pData, wFileNameLen );
		szFileName[wFileNameLen] = '\0';
		fprintf( dumpfile, "  File Name = '%s'.\n", szFileName );
		free(szFileName);
    }
    pData += wFileNameLen;

    /* host type */
    wHostType = (WORD)(FileObject.HostType & HOST_MASK);
    szHostType = GetLiteralString( HostTypeStringTable, wHostType );
    fprintf( dumpfile, "  Host Type = %s.\n", szHostType );

    /* compression technique */
    wCompress = (WORD)(FileObject.CompressionType & COMPRESS_MASK);
    if (wCompress == COMPRESS_NONE)
    {
		fprintf( dumpfile, "  Compression Type = NONE.\n" );
    }
    else if (wCompress == COMPRESS_HUFF)
    {
		fprintf( dumpfile, "  Compression Type = Huffman Encoding.\n" );
    }

    fprintf( dumpfile, "  File Attributes = " );
    if (FileObject.FileAttributes & ATTRIB_READONLY)
    {
		fprintf( dumpfile, "read-only " );
    }
    if (FileObject.FileAttributes & ATTRIB_PRIVATE)
    {
		fprintf( dumpfile, "private " );
    }
    fprintf( dumpfile, "\n" );

    fprintf( dumpfile, "  File Flags = " );
    if (FileObject.Flags & FILEFLAG_SIGN)
    {
		fprintf( dumpfile, "SIGN " );
    }
    if (FileObject.Flags & FILEFLAG_INDOC)
    {
      fprintf( dumpfile, "INDOC " );
    }
    if ((FileObject.Flags & ENCODE_MASK) == ENCODE_NONE)
    {
      fprintf( dumpfile, "ENCODE_NONE " );
    }
    if ((FileObject.Flags & ENCODE_MASK) == ENCODE_BASE64)
    {
      fprintf( dumpfile, "ENCODE_BASE64 " );
    }
    if ((FileObject.Flags & ENCODE_MASK) == ENCODE_QP)
    {
      fprintf( dumpfile, "ENCODE_QP " );
    }
    if ((FileObject.Flags & ENCODE_MASK) == ENCODE_UUENCODE)
    {
      fprintf( dumpfile, "ENCODE_UUENCODE " );
    }
    if ((FileObject.Flags & ENCODE_MASK) == ENCODE_EXTENSION)
    {
      fprintf( dumpfile, "ENCODE_EXTENSION " );
    }
    fprintf( dumpfile, "\n" );


#ifndef APITESTMODE
    fprintf( dumpfile, "  File size = %lu.\n", FileObject.FileSize );
    GetTDString( &FileObject.FileCreated, szFileCreated );
    fprintf( dumpfile, "  File created = %s.\n", szFileCreated );
    GetTDString( &FileObject.FileModified, szFileModified );
    fprintf( dumpfile, "  File modified = %s.\n", szFileModified );
#endif

    if ( wHostType == HOST_MAC )
    {
		ODSReadMemory( &pData, _FILEOBJECT_MACEXT, &MACFileObject, 1 );
		/* add code here to dump FILEOBJECT_MACEXT */
    }
    else if ( wHostType == HOST_HPFS )
    {
		ODSReadMemory( &pData, _FILEOBJECT_HPFSEXT, &HPFSFileObject, 1 );
		/* add code here to dump FILEOBJECT_HPFSEXT */
    }

    return;
}

/************************************************************************

    FUNCTION:   DumpNotelinkListItem

    PURPOSE:    Print out the contents of one NOTELINK list item.

    INPUTS:     pData - the item data, in host format.
		length - the data length, in bytes.

    DESCRIPTION:
		A NOTELINK List item is used in one of the two types of document
		links. (The other type uses CDLINKEXPORT2 records.) A NOTELINK
		list item usually has item name $Links. It is an item of type
		TYPE_NOTELINK_LIST. It consists of a LIST structure followed by an
		array of NOTELINK structures. Each NOTELINK describes a note
		universally, i.e. across all servers.

*************************************************************************/

void  LNPUBLIC   DumpNotelinkListItem (char * pData, WORD length)
{
    LIST        List;
    USHORT      i;
    NOTELINK    NoteLink;
    TIMEDATE    tdFile ;
    UNID        unidView;
    UNID        unidNote;

    memcpy( (char*)&List, pData, sizeof(LIST) );
    pData += sizeof(LIST);
    fprintf (dumpfile, "\tlist has %d NOTELINKs\n", List.ListEntries);

    for (i = 0; i<List.ListEntries; i++, pData+=sizeof(NOTELINK))
    {
		fprintf (dumpfile, "\tNotelink #%d:\n", i+1);
		memcpy( (char*)&NoteLink, pData, sizeof(NOTELINK) );
		tdFile = NoteLink.File;
		unidView = NoteLink.View;
		unidNote = NoteLink.Note;
		fprintf (dumpfile, "\t Database replica ID = %#.4lx - %#.4lx\n",
		    tdFile.Innards[1],
		    tdFile.Innards[0]);
		fprintf (dumpfile, "\t View UNID.File = %#.4lx - %#.4lx\n",
		    unidView.File.Innards[1],
		    unidView.File.Innards[0]);
		fprintf (dumpfile, "\t View UNID.Note = %#.4lx - %#.4lx\n",
		    unidView.Note.Innards[1],
		    unidView.Note.Innards[0]);
		fprintf (dumpfile, "\t Note UNID.File = %#.4lx - %#.4lx\n",
		    unidNote.File.Innards[1],
		    unidNote.File.Innards[0]);
		fprintf (dumpfile, "\t Note UNID.Note = %#.4lx - %#.4lx\n",
		    unidNote.Note.Innards[1],
		    unidNote.Note.Innards[0]);
    }
    return;
}

/************************************************************************

    FUNCTION:   DumpUserDataItem

************************************************************************/

void    LNPUBLIC  DumpUserDataItem( char * pData, WORD length)
{
    BYTE    bFmtNameLen;
    char   *achFmtNameLMBCS;
    char   *achFmtNameNative;
    WORD    wInBufSz, wOutBufSz;
	int     i;

    /* if ACLDigestItemFlag not set... */
    if (!ACLDigestItemFlag)
	{

      bFmtNameLen = *((BYTE*)pData);
	  if ((length - 1) < bFmtNameLen)
	    	bFmtNameLen = (BYTE) (length - 1);
      wInBufSz = (WORD)bFmtNameLen;
      pData += sizeof(BYTE);
      achFmtNameLMBCS = pData;
      achFmtNameNative = (char *) malloc (bFmtNameLen+1);

      wOutBufSz = OSTranslate (OS_TRANSLATE_LMBCS_TO_NATIVE,
	                           achFmtNameLMBCS, wInBufSz,
		                       achFmtNameNative, wInBufSz);

      achFmtNameNative[wOutBufSz] = '\0';

#ifndef APITESTMODE
      fprintf(dumpfile, "  User Data Format Name = '%s'.\n", achFmtNameNative);
#endif

      fprintf(dumpfile, "  User Data = ");
      NSFDDUMPDumpMemory ((achFmtNameLMBCS + bFmtNameLen),
	                 	 (WORD)( length - 1 - bFmtNameLen));
       free( achFmtNameNative );
	}
	/* else we have a $ACLDigest item and need to output data as bytes */
	else
	{
	  fprintf(dumpfile, "  Byte Data   = ");
	  for (i=0; i<=length; i++)
	    fprintf(dumpfile, "%02X ", ((BYTE)*(pData + i)));
	  fprintf(dumpfile, "\n");
	  ACLDigestItemFlag = FALSE;
	}
    return;
}


/************************************************************************

    FUNCTION:   DumpAgentQueryItem

************************************************************************/

void    LNPUBLIC  DumpAgentQueryItem (
	char *  Name,
	WORD    NameLength,
	char *  pData,
	WORD    length
) {
	dwActionRecord = 0;
	PrintCompositeBuffer( pData, length, DumpOneAgentQueryRecord, NULL );

    return;
}

/************************************************************************

    FUNCTION:   DumpActionItem

************************************************************************/

void    LNPUBLIC  DumpActionItem (
	char *  Name,
	WORD    NameLength,
	char *  pData,
	WORD    length
) {
	dwActionRecord = 0;
	PrintCompositeBuffer( pData, length, DumpOneActionRecord, NULL );

    return;
}

/************************************************************************

    FUNCTION:   DumpViewmapDatasetItem

************************************************************************/

void    LNPUBLIC  DumpViewmapDatasetItem (
	char *  Name,
	WORD    NameLength,
	char *  pData,
	WORD    length
) {
	dwNavigateRecord = 0;
	PrintCompositeBuffer( pData, length, DumpOneViewmapDatasetRecord, NULL );

    return;
}

/************************************************************************

    FUNCTION:   DumpViewmapLayoutItem

************************************************************************/

void    LNPUBLIC  DumpViewmapLayoutItem (
	char *  Name,
	WORD    NameLength,
	char *  pData,
	WORD    length
) {
	dwNavigateRecord = 0;
	PrintCompositeBuffer( pData, length, DumpOneViewmapLayoutRecord, NULL );

    return;
}

/************************************************************************

    FUNCTION:   DumpLotusScriptObjectItem

************************************************************************/

void    LNPUBLIC  DumpLotusScriptObjectItem (
	char *  Name,
	WORD    NameLength,
	char *  pData,
	WORD    length
) {
	fprintf (dumpfile, "    Lotus Script object length = %u\n",
		length);
	fprintf (dumpfile, "      (cannot decompile Lotus Script)\n");

	return;
}

/************************************************************************

    FUNCTION:   DumpHTMLItem

************************************************************************/

void    LNPUBLIC  DumpHTMLItem (
	char *  Name,
	WORD    NameLength,
	char *  pData,
	WORD    length
) {
	fprintf (dumpfile, "    <Beginning of HTML item>\n");
	PrintText (pData, length);
	fprintf (dumpfile, "    <End of HTML item>\n");
}

#ifdef __cplusplus
}
#endif
