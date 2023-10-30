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

    FILE:       agents.c

    PROGRAM:    nsf_dump
    
    DESCRIPTION:
        This file contains routines that print the contents of
        Query and Action records to the dumpfile. These routines
        mostly just dump structures defined in HCL C API for Domino and
		Notes header file queryods.h.

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
#include <misc.h>
#include <osmem.h>
#include <osmisc.h>
#include <ostime.h>
#include <stdnames.h>
#include <textlist.h>
#include <editods.h>
#include <queryods.h>

/* NSF_DUMP - specific include file */
#include "nsf_dump.h"
#if defined (OS390)
#include <lapicinc.h>
#endif

/*
 *		Common strings
 */
char sz_4_blankSpaces   [] = "    ";

	/* QueryByField operators */
char szQUERYBYFIELD_OP_GREATER[]        = "QUERYBYFIELD_OP_GREATER";
char szQUERYBYFIELD_OP_LESS[]           = "QUERYBYFIELD_OP_LESS";
char szQUERYBYFIELD_OP_NOTEQUAL[]       = "QUERYBYFIELD_OP_NOTEQUAL";
char szQUERYBYFIELD_OP_BETWEEN[]        = "QUERYBYFIELD_OP_BETWEEN";
char szQUERYBYFIELD_OP_NOTWITHIN[]      = "QUERYBYFIELD_OP_NOTWITHIN";
char szQUERYBYFIELD_OP_EQUAL[]          = "QUERYBYFIELD_OP_EQUAL";
char szQUERYBYFIELD_OP_CONTAINS[]       = "QUERYBYFIELD_OP_CONTAINS";
char szQUERYBYFIELD_OP_INTHELAST[]      = "QUERYBYFIELD_OP_INTHELAST";
char szQUERYBYFIELD_OP_INTHENEXT[]      = "QUERYBYFIELD_OP_INTHENEXT";
char szQUERYBYFIELD_OP_OLDERTHAN[]      = "QUERYBYFIELD_OP_OLDERTHAN";
char szQUERYBYFIELD_OP_DUEIN[]          = "QUERYBYFIELD_OP_DUEIN";
char szQUERYBYFIELD_OP_DOESNOTCONTAIN[] = "QUERYBYFIELD_OP_DOESNOTCONTAIN";
char szUNKNOWN_OPERATOR[]               = "UNKNOWN OPERATOR";

LITERAL_NAME_ENTRY QueryByFieldOpsTable [] =
{
	{QUERYBYFIELD_OP_GREATER,		 szQUERYBYFIELD_OP_GREATER},
	{QUERYBYFIELD_OP_LESS,			 szQUERYBYFIELD_OP_LESS},
	{QUERYBYFIELD_OP_NOTEQUAL,		 szQUERYBYFIELD_OP_NOTEQUAL},
	{QUERYBYFIELD_OP_BETWEEN,		 szQUERYBYFIELD_OP_BETWEEN},
	{QUERYBYFIELD_OP_NOTWITHIN,		 szQUERYBYFIELD_OP_NOTWITHIN},
	{QUERYBYFIELD_OP_EQUAL,			 szQUERYBYFIELD_OP_EQUAL},
	{QUERYBYFIELD_OP_CONTAINS,		 szQUERYBYFIELD_OP_CONTAINS},
	{QUERYBYFIELD_OP_INTHELAST,		 szQUERYBYFIELD_OP_INTHELAST},
	{QUERYBYFIELD_OP_INTHENEXT,		 szQUERYBYFIELD_OP_INTHENEXT},
	{QUERYBYFIELD_OP_OLDERTHAN,		 szQUERYBYFIELD_OP_OLDERTHAN},
	{QUERYBYFIELD_OP_DUEIN,			 szQUERYBYFIELD_OP_DUEIN},
	{QUERYBYFIELD_OP_DOESNOTCONTAIN, szQUERYBYFIELD_OP_DOESNOTCONTAIN},

		/* This must be the last entry! */
	{LITERAL_TABLE_END,				 szUNKNOWN_OPERATOR}
}; 

	/* QueryByField operators */
char szACTIONBYFIELD_OP_REPLACE[] = "ACTIONBYFIELD_OP_REPLACE";
char szACTIONBYFIELD_OP_APPEND[]  = "ACTIONBYFIELD_OP_APPEND";
char szACTIONBYFIELD_OP_REMOVE[]  = "ACTIONBYFIELD_OP_REMOVE";

LITERAL_NAME_ENTRY ActionByFieldOpsTable [] =
{
	{ACTIONBYFIELD_OP_REPLACE,		szACTIONBYFIELD_OP_REPLACE},
	{ACTIONBYFIELD_OP_APPEND,		szACTIONBYFIELD_OP_APPEND},
	{ACTIONBYFIELD_OP_REMOVE,		szACTIONBYFIELD_OP_REMOVE},

		/* This must be the last entry! */
	{LITERAL_TABLE_END,				szUNKNOWN_OPERATOR}
};

/*
 *  AssistTriggerTypeTable - used by GetLiteralString()
 */
char szASSISTTRIGGER_TYPE_NONE[]      = "ASSISTTRIGGER_TYPE_NONE";
char szASSISTTRIGGER_TYPE_SCHEDULED[] = "ASSISTTRIGGER_TYPE_SCHEDULED";
char szASSISTTRIGGER_TYPE_NEWMAIL[]   = "ASSISTTRIGGER_TYPE_NEWMAIL";
char szASSISTTRIGGER_TYPE_PASTED[]    = "ASSISTTRIGGER_TYPE_PASTED";
char szASSISTTRIGGER_TYPE_MANUAL[]    = "ASSISTTRIGGER_TYPE_MANUAL";
char szASSISTTRIGGER_TYPE_DOCUPDATE[] = "ASSISTTRIGGER_TYPE_DOCUPDATE";
char szUNKNOWN_TRIGGER_TYPE[]         = "UNKNOWN TRIGGER TYPE";

LITERAL_NAME_ENTRY AssistTriggerTypeTable [] =
{
	{ASSISTTRIGGER_TYPE_NONE,		szASSISTTRIGGER_TYPE_NONE},
	{ASSISTTRIGGER_TYPE_SCHEDULED,	szASSISTTRIGGER_TYPE_SCHEDULED},
	{ASSISTTRIGGER_TYPE_NEWMAIL,	szASSISTTRIGGER_TYPE_NEWMAIL},
	{ASSISTTRIGGER_TYPE_PASTED,		szASSISTTRIGGER_TYPE_PASTED},
	{ASSISTTRIGGER_TYPE_MANUAL,		szASSISTTRIGGER_TYPE_MANUAL},
	{ASSISTTRIGGER_TYPE_DOCUPDATE,	szASSISTTRIGGER_TYPE_DOCUPDATE},

		/* This must be the last entry! */
	{LITERAL_TABLE_END,		szUNKNOWN_TRIGGER_TYPE}
};

/*
 *  AssistSearchTypeTable - used by GetLiteralString()
 */
char szASSISTSEARCH_TYPE_NONE[] = "ASSISTSEARCH_TYPE_NONE";
char szASSISTSEARCH_TYPE_ALL[] = "ASSISTSEARCH_TYPE_ALL";
char szASSISTSEARCH_TYPE_NEW[] = "ASSISTSEARCH_TYPE_NEW";
char szASSISTSEARCH_TYPE_MODIFIED[] = "ASSISTSEARCH_TYPE_MODIFIED";
char szASSISTSEARCH_TYPE_SELECTED[] = "ASSISTSEARCH_TYPE_SELECTED";
char szASSISTSEARCH_TYPE_VIEW[] = "ASSISTSEARCH_TYPE_VIEW";
char szASSISTSEARCH_TYPE_UNREAD[] = "ASSISTSEARCH_TYPE_UNREAD";
char szASSISTSEARCH_TYPE_PROMPT[] = "ASSISTSEARCH_TYPE_PROMPT";
char szASSISTSEARCH_TYPE_UI[] = "ASSISTSEARCH_TYPE_UI";
char szUNKNOWN_SEARCH_TYPE[] = "UNKNOWN SEARCH TYPE";

LITERAL_NAME_ENTRY AssistSearchTypeTable [] =
{
	{ASSISTSEARCH_TYPE_NONE,		szASSISTSEARCH_TYPE_NONE},
	{ASSISTSEARCH_TYPE_ALL,			szASSISTSEARCH_TYPE_ALL},
	{ASSISTSEARCH_TYPE_NEW,			szASSISTSEARCH_TYPE_NEW},
	{ASSISTSEARCH_TYPE_MODIFIED,	szASSISTSEARCH_TYPE_MODIFIED},
	{ASSISTSEARCH_TYPE_SELECTED,	szASSISTSEARCH_TYPE_SELECTED},
	{ASSISTSEARCH_TYPE_VIEW,		szASSISTSEARCH_TYPE_VIEW},
	{ASSISTSEARCH_TYPE_UNREAD,		szASSISTSEARCH_TYPE_UNREAD},
	{ASSISTSEARCH_TYPE_PROMPT,		szASSISTSEARCH_TYPE_PROMPT},
	{ASSISTSEARCH_TYPE_UI,			szASSISTSEARCH_TYPE_UI},

		/* This must be the last entry! */
	{LITERAL_TABLE_END,		szUNKNOWN_SEARCH_TYPE}
};

/*
 *  AssistIntervalTypeTable - used by GetLiteralString()
 */
char szASSISTINTERVAL_TYPE_NONE[]    = "ASSISTINTERVAL_TYPE_NONE";
char szASSISTINTERVAL_TYPE_MINUTES[] = "ASSISTINTERVAL_TYPE_MINUTES";
char szASSISTINTERVAL_TYPE_DAYS[]    = "ASSISTINTERVAL_TYPE_DAYS";
char szASSISTINTERVAL_TYPE_WEEK[]    = "ASSISTINTERVAL_TYPE_WEEK";
char szASSISTINTERVAL_TYPE_MONTH[]   = "ASSISTINTERVAL_TYPE_MONTH";
char szUNKNOWN_INTERVAL_TYPE[]    = "UNKNOWN INTERVAL TYPE";

LITERAL_NAME_ENTRY AssistIntervalTypeTable [] =
{
	{ASSISTINTERVAL_TYPE_NONE,		szASSISTINTERVAL_TYPE_NONE},
	{ASSISTINTERVAL_TYPE_MINUTES,	szASSISTINTERVAL_TYPE_MINUTES},
	{ASSISTINTERVAL_TYPE_DAYS,		szASSISTINTERVAL_TYPE_DAYS},
	{ASSISTINTERVAL_TYPE_WEEK,		szASSISTINTERVAL_TYPE_WEEK},
	{ASSISTINTERVAL_TYPE_MONTH,		szASSISTINTERVAL_TYPE_MONTH},

		/* This must be the last entry! */
	{LITERAL_TABLE_END,		szUNKNOWN_INTERVAL_TYPE}
};

/************************************************************************

    FUNCTION:   DumpOneAgentQueryRecord

    PURPOSE:    Dump one Agent Query Record

    ALGORITHM:  This is the action routine for EnumCompositeBuffer
                specified in DumpAgentQueryItem().

    INPUTS:      
         char      *RecordPtr       -   pointer to start of CD record data
                                        This data is in Canonical format.
         WORD       RecordType      -   CD record type (SIG_QUERY_*)
         DWORD      RecordLength    -   length of CD record
         void far * Unused

*************************************************************************/

STATUS  LNPUBLIC  DumpOneAgentQueryRecord( char   *RecordPtr, 
                                     WORD    RecordType, 
                                     DWORD   RecordLength, 
                                     void   *Unused )
{
    fprintf( dumpfile,"  Agent Query Record : %lu\n", ++dwActionRecord );
    fprintf( dumpfile,"    Record Length = %lu bytes.\n", RecordLength );
    fprintf( dumpfile,"    Record Type   = " );

    switch (RecordType) 
    {
			/* Agent CD records - added in Releae 4.0 */
		case SIG_QUERY_HEADER:
			DumpQueryHeader( RecordPtr, RecordLength );
			break;

		case SIG_QUERY_TEXTTERM:
			DumpQueryTextterm( RecordPtr, RecordLength );
			break;

		case SIG_QUERY_BYFIELD:
		case SIG_QUERY_BYDATE:
		case SIG_QUERY_BYAUTHOR:
			DumpQueryByfield( RecordPtr, RecordLength, RecordType );
			break;

		case SIG_QUERY_FORMULA:
			DumpQueryFormula( RecordPtr, RecordLength );
			break;

		case SIG_QUERY_BYFORM:
			DumpQueryByform( RecordPtr, RecordLength );
			break;

		case SIG_QUERY_BYFOLDER:
			DumpQueryByfolder( RecordPtr, RecordLength );
			break;

		case SIG_QUERY_USESFORM:
			DumpQueryUsesform( RecordPtr, RecordLength );
			break;

		case SIG_QUERY_TOPIC:
			DumpQueryTopic( RecordPtr, RecordLength );
			break;

        default:
            fprintf( dumpfile, "Unidentified.\n");
            fprintf( dumpfile, "    Unknown Type = 0x%#X.\n", RecordType );
            NSFDDUMPDumpMemory( RecordPtr, (WORD) RecordLength );
            break;
   }
   return NOERROR;
}

/************************************************************************

    FUNCTION:   DumpQueryHeader

*************************************************************************/

void    LNPUBLIC  DumpQueryHeader (
	char *	RecordPtr,
	DWORD	RecordLength
) {
    char far *		p = RecordPtr;
    CDQUERYHEADER	QHeadRec;

    fprintf (dumpfile, "  CDQUERYHEADER\n");

    ODSReadMemory (&p, _CDQUERYHEADER, &QHeadRec, 1);

	fprintf (dumpfile, "    dwFlags (0x%lX) = ", QHeadRec.dwFlags);
	if (QUERY_FLAG_INCLUDEDESCENDANTS & QHeadRec.dwFlags)
		fprintf (dumpfile, "INCLUDE DESCENDANTS ");
	fprintf (dumpfile, "\n");

    return;
}

/************************************************************************

    FUNCTION:   DumpQueryTextterm

*************************************************************************/

void    LNPUBLIC  DumpQueryTextterm (
	char *	RecordPtr,
	DWORD	RecordLength
) {
    char far *		p = RecordPtr;
    CDQUERYTEXTTERM	QTextTermRec;
    int				index;

    fprintf (dumpfile, "  CDQUERYTEXTTERM\n");

    ODSReadMemory (&p, _CDQUERYTEXTTERM, &QTextTermRec, 1);

	fprintf (dumpfile, "    dwFlags (0x%lX) = ", QTextTermRec.dwFlags);
	if (TEXTTERM_FLAG_RAW & QTextTermRec.dwFlags)
		fprintf (dumpfile, "RAW ");
	if (TEXTTERM_FLAG_VERITY & QTextTermRec.dwFlags)
		fprintf (dumpfile, "VERITY ");
	if (TEXTTERM_FLAG_AND & QTextTermRec.dwFlags)
		fprintf (dumpfile, "AND ");
	if (TEXTTERM_FLAG_ACCRUE & QTextTermRec.dwFlags)
		fprintf (dumpfile, "ACCRUE ");
	if (TEXTTERM_FLAG_NEAR & QTextTermRec.dwFlags)
		fprintf (dumpfile, "NEAR ");
	if (TEXTTERM_FLAG_PLAINTEXT & QTextTermRec.dwFlags)
		fprintf (dumpfile, "PLAINTEXT ");
	fprintf (dumpfile, "\n");

	for (index = 0; index < MAXTEXTTERMCOUNT; index++)
	{
		fprintf (dumpfile, "    dwLength[%d] = %lu  TextTerm = ",
			index, QTextTermRec.dwLength[index]);
		PrintText (p, QTextTermRec.dwLength[index]);
		if (0 == QTextTermRec.dwLength[index])
			fprintf (dumpfile, "\n");
		p += QTextTermRec.dwLength[index];
	}

    return;
}

/************************************************************************

    FUNCTION:   DumpQueryByfield

	PURPOSE:	Dump the contents of all the records that share the
				CDQUERYBYFIELD structure - SIG_QUERY_BYFIELD,
				SIG_QUERY_BYDATE, and SIG_QUERY_BYAUTHOR.

*************************************************************************/

void    LNPUBLIC  DumpQueryByfield (
	char *	RecordPtr,
	DWORD	RecordLength,
	WORD	RecordType
) {
    char far *		p = RecordPtr;
    CDQUERYBYFIELD	QByFieldRec;
    char *			pName;
    char			szTimedate[MAXALPHATIMEDATE+1];
    double			Number;

	switch (RecordType)
	{
		case SIG_QUERY_BYFIELD:
	    	fprintf (dumpfile, "  CDQUERYBYFIELD\n");
			break;

		case SIG_QUERY_BYDATE:
	    	fprintf (dumpfile, "  CDQUERYBYDATE\n");
			break;

		case SIG_QUERY_BYAUTHOR:
	    	fprintf (dumpfile, "  CDQUERYBYAUTHOR\n");
			break;

		default:
			fprintf (dumpfile, "  Unkown signature:  %u (0x%X)\n",
				RecordType, RecordType);
			break;
	}

    ODSReadMemory (&p, _CDQUERYBYFIELD, &QByFieldRec, 1);

	fprintf (dumpfile, "    dwFlags (0x%lX) = ", QByFieldRec.dwFlags);
	if (QUERYBYFIELD_FLAG_BYDATE & QByFieldRec.dwFlags)
		fprintf (dumpfile, "BYDATE ");
	if (QUERYBYFIELD_FLAG_BYAUTHOR & QByFieldRec.dwFlags)
		fprintf (dumpfile, "BYAUTHOR ");
	fprintf (dumpfile, "\n");
	pName = GetLiteralString (TypeStringTablePtr, QByFieldRec.wDataType);
	fprintf (dumpfile, "    wDataType (%u) = %s\n",
		QByFieldRec.wDataType, pName);
	pName = GetLiteralString (QueryByFieldOpsTable, QByFieldRec.wOperator);
	fprintf (dumpfile, "    wOperator (%u) = %s\n",
		QByFieldRec.wOperator, pName);
    GetTDString ((TIMEDATE*) (&QByFieldRec.Date1), szTimedate);
#ifndef APITESTMODE
	fprintf (dumpfile, "    Date1 = %s  ", szTimedate);
#endif
    GetTDString ((TIMEDATE*) (&QByFieldRec.Date2), szTimedate);
#ifndef APITESTMODE
	fprintf (dumpfile, "Date2 = %s\n", szTimedate);
#endif
	memcpy ((char *) (&Number), (char *) (&QByFieldRec.Number1), sizeof(NUMBER));
#ifdef OS390
	/* Domino and Notes gave us an IEEE double which must be converted to native. */
	ConvertIEEEToDouble(&Number, &Number);
#endif /* OS390 */
	fprintf (dumpfile, "    Number1 = %f  ", Number);
	memcpy ((char *) (&Number), (char *) (&QByFieldRec.Number2), sizeof(NUMBER));
#ifdef OS390
	/* Domino and Notes gave us an IEEE double which must be converted to native. */
        ConvertIEEEToDouble(&Number, &Number);
#endif /* OS390 */
	fprintf (dumpfile, "Number2 = %f\n", Number);
	fprintf (dumpfile, "    wFieldNameLen = %u\n",
		QByFieldRec.wFieldNameLen);
	fprintf (dumpfile, "    wValueLen = %u\n",
		QByFieldRec.wValueLen);
	fprintf (dumpfile, "    FieldName = ");
	if (QByFieldRec.wFieldNameLen)
	{
		PrintText (p, QByFieldRec.wFieldNameLen);
		p += QByFieldRec.wFieldNameLen;
	}
	else
		fprintf (dumpfile, "\n");
	fprintf (dumpfile, "    Value = ");
	if (QByFieldRec.wValueLen)
	{
		PrintCanonicalValue (p, QByFieldRec.wDataType, QByFieldRec.wValueLen);
		p += QByFieldRec.wValueLen;
	}
	else
		fprintf (dumpfile, "\n");
	
    return;
}

/************************************************************************

    FUNCTION:   DumpQueryFormula

*************************************************************************/

void    LNPUBLIC  DumpQueryFormula (
	char *	RecordPtr,
	DWORD	RecordLength
) {
    char far *		p = RecordPtr;
    CDQUERYFORMULA	QFormulaRec;

    fprintf (dumpfile, "  CDQUERYFORMULA\n");

    ODSReadMemory (&p, _CDQUERYFORMULA, &QFormulaRec, 1);

	fprintf (dumpfile, "    dwFlags (0x%lX) = ", QFormulaRec.dwFlags);
	if (QUERYFORMULA_FLAG_PLAINTEXT & QFormulaRec.dwFlags)
		fprintf (dumpfile, "PLAINTEXT ");
	fprintf (dumpfile, "\n");
	fprintf (dumpfile, "    wFormulaLen = %u\n",
		QFormulaRec.wFormulaLen);
	PrintFormula (p, QFormulaRec.wFormulaLen);

    return;
}

/************************************************************************

    FUNCTION:   DumpODSAssistFieldStruct

	PURPOSE:	Dump the contents of the individual field sub-records.

	ARGUMENTS:
		ppData		Address of pointer variable;  this pointer is moved
					to the first byte following the field structure
					on return.
		pIndent		Prefix string so printing is pretty.

*************************************************************************/

void LNPUBLIC DumpODSAssistFieldStruct (
	char ** 		ppData,
	char *			pIndent,
	LITERAL_NAME_ENTRY *pTable
) {
	char *			p = *ppData;
	ODS_ASSISTFIELDSTRUCT FieldStruct;
	char *			pName;		/* Name for constant value */

	ODSReadMemory (&p, _ODS_ASSISTFIELDSTRUCT, &FieldStruct, 1);

	fprintf (dumpfile, "%s  wTotalLen = %u\n",
		pIndent, FieldStruct.wTotalLen);
	pName = GetLiteralString (pTable, FieldStruct.wOperator);
	fprintf (dumpfile, "%s  wOperator (%u) = %s\n",
		pIndent, FieldStruct.wOperator, pName);
	fprintf (dumpfile, "%s  wFieldNameLen = %u\n",
		pIndent, FieldStruct.wFieldNameLen);
	fprintf (dumpfile, "%s  wValueLen = %u\n",
		pIndent, FieldStruct.wValueLen);
	pName = GetLiteralString (TypeStringTablePtr, FieldStruct.wValueDataType);
	fprintf (dumpfile, "%s  wValueDataType (%u) = %s\n",
		pIndent, FieldStruct.wValueDataType, pName);
	fprintf (dumpfile, "%s  wSpare = %u\n",
		pIndent, FieldStruct.wSpare);
	fprintf (dumpfile, "%s  FieldName = ", pIndent);
	if (FieldStruct.wFieldNameLen)
	{
		PrintText (p, FieldStruct.wFieldNameLen);
		p += FieldStruct.wFieldNameLen;
	}
	else
		fprintf (dumpfile, "\n");
	fprintf (dumpfile, "%s  Value = ", pIndent);
	PrintCanonicalValue (p, FieldStruct.wValueDataType, FieldStruct.wValueLen);

	*ppData += FieldStruct.wTotalLen;
	return;
}

/************************************************************************

    FUNCTION:   DumpQueryByform

*************************************************************************/

void    LNPUBLIC  DumpQueryByform (
	char *	RecordPtr,
	DWORD	RecordLength
) {
    char *			p = RecordPtr;
    CDQUERYBYFORM	QByFormRec;
    WORD			index;

    fprintf (dumpfile, "  CDQUERYBYFORM\n");

    ODSReadMemory (&p, _CDQUERYBYFORM, &QByFormRec, 1);

	fprintf (dumpfile, "    dwFlags (0x%lX) = (Unused)\n",
		QByFormRec.dwFlags);
	fprintf (dumpfile, "    wFieldCount = %u\n",
		QByFormRec.wFieldCount);
	fprintf (dumpfile, "    wFormNameLen = %u\n",
		QByFormRec.wFormNameLen);
	for (index = 0; index < QByFormRec.wFieldCount; index++)
	{
		fprintf (dumpfile, "    Field data (ODS_ASSISTFIELDSTRUCT[%d]):\n", index);
		DumpODSAssistFieldStruct ( &p, sz_4_blankSpaces, QueryByFieldOpsTable);
	}
	fprintf (dumpfile, "    FormName = ");
	if (QByFormRec.wFormNameLen)
	{
		PrintText (p, QByFormRec.wFormNameLen);
		p += QByFormRec.wFormNameLen;
	}
	else
		fprintf (dumpfile, "\n");

	return;
}

/************************************************************************

    FUNCTION:   DumpQueryByfolder

*************************************************************************/

void    LNPUBLIC  DumpQueryByfolder (
	char *	RecordPtr,
	DWORD	RecordLength
) {
    char far *		p = RecordPtr;
    CDQUERYBYFOLDER	QFolderRec;

    fprintf (dumpfile, "  CDQUERYBYFOLDER\n");

    ODSReadMemory (&p, _CDQUERYBYFOLDER, &QFolderRec, 1);

	fprintf (dumpfile, "    dwFlags (0x%lX) = ", QFolderRec.dwFlags);
	if (QUERYBYFOLDER_FLAG_PRIVATE & QFolderRec.dwFlags)
		fprintf (dumpfile, "PRIVATE ");
	fprintf (dumpfile, "\n");
	fprintf (dumpfile, "    wFolderNameLen = %u\n",
		QFolderRec.wFolderNameLen);
	fprintf (dumpfile, "    FolderName = ");
	if (QFolderRec.wFolderNameLen)
	{
		PrintText (p, QFolderRec.wFolderNameLen);
		p += QFolderRec.wFolderNameLen;
	}
	else
		fprintf (dumpfile, "\n");

    return;
}

/************************************************************************

    FUNCTION:   DumpQueryUsesform

*************************************************************************/

void    LNPUBLIC  DumpQueryUsesform (
	char *	RecordPtr,
	DWORD	RecordLength
) {
    char *			p = RecordPtr;
    CDQUERYUSESFORM	QUsesFormRec;
	LIST			List;
	USHORT			index;
	char *			pText;
	WORD				textLen;

    fprintf (dumpfile, "  CDQUERYUSESFORM\n");

    ODSReadMemory (&p, _CDQUERYUSESFORM, &QUsesFormRec, 1);

	fprintf (dumpfile, "    dwFlags (0x%lX) = (Unused)\n",
		QUsesFormRec.dwFlags);

		/* Process the form name list */
	ODSReadMemory (&p, _LIST, ((void *) (&List)), 1);
	fprintf (dumpfile, "    Number of entries: %u\n",
		List.ListEntries);
	pText = p + (List.ListEntries * ODSLength (_LIST));
	for (index = 0; index < List.ListEntries; index++)
	{
		ODSReadMemory (&p, _WORD, ((void *) (&textLen)), 1);
		fprintf (dumpfile, "      Entry %d: ", textLen);
		PrintText (pText, textLen);
		pText += textLen;
	}

	return;
}

/************************************************************************

    FUNCTION:   DumpQueryTopic

*************************************************************************/

void    LNPUBLIC  DumpQueryTopic (
	char *	RecordPtr,
	DWORD	RecordLength
) {
    char *			p = RecordPtr;
    CDQUERYTOPIC	QTopicRec;

    fprintf (dumpfile, "  CDQUERYTOPIC\n");

    ODSReadMemory (&p, _CDQUERYTOPIC, &QTopicRec, 1);

	fprintf (dumpfile, "    dwFlags (0x%lX) = (Unused)\n",
		QTopicRec.dwFlags);
	fprintf (dumpfile, "    wTopicQueryLen = %u\n",
		QTopicRec.wTopicQueryLen);
	fprintf (dumpfile, "    wDataLen = %u\n",
		QTopicRec.wDataLen);

    return;
}

/************************************************************************

    FUNCTION:   DumpOneActionRecord

    PURPOSE:    Dump one Action Record

    ALGORITHM:  This is the action routine for EnumCompositeBuffer
                specified in DumpActionItem().

    INPUTS:      
         char      *RecordPtr       -   pointer to start of CD record data
                                        This data is in Canonical format.
         WORD       RecordType      -   CD record type (SIG_ACTION_*)
         DWORD      RecordLength    -   length of CD record
         void far * Unused

*************************************************************************/

STATUS  LNPUBLIC  DumpOneActionRecord( char   *RecordPtr, 
                                     WORD    RecordType, 
                                     DWORD   RecordLength, 
                                     void   *Unused )
{
    fprintf( dumpfile,"  Action Record : %ld\n", ++dwActionRecord );
    fprintf( dumpfile,"   Record Length = %ld bytes.\n", RecordLength );
    fprintf( dumpfile,"   Record Type   = " );

    switch (RecordType) 
    {
			/* Agent CD records - added in Releae 4.0 */
		case SIG_ACTION_HEADER:
			DumpActionHeader( RecordPtr, RecordLength );
			break;

		case SIG_ACTION_MODIFYFIELD:
			DumpActionModifyfield( RecordPtr, RecordLength );
			break;

		case SIG_ACTION_REPLY:
			DumpActionReply( RecordPtr, RecordLength );
			break;

		case SIG_ACTION_FORMULA:
		case SIG_ACTION_FORMULAONLY:
			DumpActionFormula( RecordPtr, RecordLength, RecordType );
			break;

		case SIG_ACTION_LOTUSSCRIPT:
			DumpActionLotusscript( RecordPtr, RecordLength );
			break;

		case SIG_ACTION_SENDMAIL:
			DumpActionSendmail( RecordPtr, RecordLength );
			break;

		case SIG_ACTION_DBCOPY:
			DumpActionDbcopy( RecordPtr, RecordLength );
			break;

		case SIG_ACTION_DELETE:
			DumpActionDelete( RecordPtr, RecordLength );
			break;

		case SIG_ACTION_BYFORM:
			DumpActionByform( RecordPtr, RecordLength );
			break;

		case SIG_ACTION_MARKREAD:
		case SIG_ACTION_MARKUNREAD:
			DumpActionReadMarks( RecordPtr, RecordLength, RecordType );
			break;

		case SIG_ACTION_MOVETOFOLDER:
		case SIG_ACTION_COPYTOFOLDER:
		case SIG_ACTION_REMOVEFROMFOLDER:
			DumpActionFolder( RecordPtr, RecordLength, RecordType );
			break;

		case SIG_ACTION_NEWSLETTER:
			DumpActionNewsletter( RecordPtr, RecordLength );
			break;

		case SIG_ACTION_RUNAGENT:
			DumpActionRunagent( RecordPtr, RecordLength );
			break;

		case SIG_ACTION_SENDDOCUMENT:
			DumpActionSenddocument( RecordPtr, RecordLength );
			break;

			/* New in Release 4.6 */
		case SIG_ACTION_JAVAAGENT:
			DumpActionJavaAgent( RecordPtr, RecordLength );
			break;

        default:
            fprintf( dumpfile, "Unidentified.\n");
            fprintf( dumpfile, "    Unknown Type = %#x.\n", RecordType );
            NSFDDUMPDumpMemory( RecordPtr, (WORD) RecordLength );
            break;
   }
   return NOERROR;
}

/************************************************************************

    FUNCTION:   DumpActionHeader

*************************************************************************/

void    LNPUBLIC  DumpActionHeader (
	char *	RecordPtr,
	DWORD	RecordLength
) {
    fprintf( dumpfile,"  ACTION_HEADER\n" );
    return;
}

/************************************************************************

    FUNCTION:   DumpActionModifyfield

*************************************************************************/

void    LNPUBLIC  DumpActionModifyfield (
	char *	RecordPtr,
	DWORD	RecordLength
) {
    char far *		p = RecordPtr;
    CDACTIONMODIFYFIELD	ActModFieldRec;

    fprintf (dumpfile, "  CDACTIONMODIFYFIELD\n");

    ODSReadMemory (&p, _CDACTIONMODIFYFIELD, &ActModFieldRec, 1);

	fprintf (dumpfile, "    dwFlags (0x%lX) = ", ActModFieldRec.dwFlags);
	if (MODIFYFIELD_FLAG_REPLACE & ActModFieldRec.dwFlags)
		fprintf (dumpfile, "REPLACE ");
	if (MODIFYFIELD_FLAG_APPEND & ActModFieldRec.dwFlags)
		fprintf (dumpfile, "APPEND ");
	fprintf (dumpfile, "\n");
	fprintf (dumpfile, "    wFieldNameLen = %u\n",
		ActModFieldRec.wFieldNameLen);
	fprintf (dumpfile, "    wValueLen = %u\n",
		ActModFieldRec.wValueLen);
	fprintf (dumpfile, "    FieldName = ");
	if (ActModFieldRec.wFieldNameLen)
	{
		PrintText (p, ActModFieldRec.wFieldNameLen);
		p += ActModFieldRec.wFieldNameLen;
	}
	else
		fprintf (dumpfile, "\n");

	fprintf (dumpfile, "    New value = ");
	if (ActModFieldRec.wValueLen)
	{
		PrintText (p, ActModFieldRec.wValueLen);
		p += ActModFieldRec.wValueLen;
	}
	else
		fprintf (dumpfile, "\n");

    return;
}

/************************************************************************

    FUNCTION:   DumpActionReply

*************************************************************************/

void    LNPUBLIC  DumpActionReply (
	char *	RecordPtr,
	DWORD	RecordLength
) {
    char far *		p = RecordPtr;
    CDACTIONREPLY	ActReplyRec;

    fprintf (dumpfile, "  CDACTIONREPLY\n");

    ODSReadMemory (&p, _CDACTIONREPLY, &ActReplyRec, 1);

	fprintf (dumpfile, "    dwFlags (0x%lX) = ", ActReplyRec.dwFlags);
	if (ACTIONREPLY_FLAG_REPLYTOALL & ActReplyRec.dwFlags)
		fprintf (dumpfile, "REPLYTOALL ");
	if (ACTIONREPLY_FLAG_INCLUDEDOC & ActReplyRec.dwFlags)
		fprintf (dumpfile, "INCLUDEDOC ");
	if (ACTIONREPLY_FLAG_SAVEMAIL & ActReplyRec.dwFlags)
		fprintf (dumpfile, "SAVEMAIL ");
	if (ACTIONREPLY_FLAG_NOAGENTREPLY & ActReplyRec.dwFlags)
		fprintf (dumpfile, "NOAGENTREPLY ");
	if (ACTIONREPLY_FLAG_REPLYONCE & ActReplyRec.dwFlags)
		fprintf (dumpfile, "REPLYONCE ");
	fprintf (dumpfile, "\n");
	fprintf (dumpfile, "    wBodyLen = %u\n",
		ActReplyRec.wBodyLen);
	fprintf (dumpfile, "    Body = ");
	if (ActReplyRec.wBodyLen)
		PrintText (p, ActReplyRec.wBodyLen);
	else
		fprintf (dumpfile, "\n");

	return;
}

/************************************************************************

    FUNCTION:   DumpActionFormula
	
	PURPOSE:	Dump the contents of a CDACTIONFORMULA record - used for
				SIG_ACTION_FORMULA and SIG_ACTION_FORMULAONLY.

*************************************************************************/

void    LNPUBLIC  DumpActionFormula (
	char *	RecordPtr,
	DWORD	RecordLength,
	WORD	RecordType
) {
    char far *		p = RecordPtr;
    CDACTIONFORMULA	ActFormulaRec;

	if (SIG_ACTION_FORMULA == RecordType)
	    fprintf (dumpfile, "  CDACTIONFORMULA\n");
	else if (SIG_ACTION_FORMULAONLY == RecordType)
	    fprintf (dumpfile, "  CDACTIONFORMULAONLY\n");
	else
		fprintf (dumpfile, "  Unknown signature: %u (0x%X)\n",
			RecordType, RecordType);

    ODSReadMemory (&p, _CDACTIONFORMULA, &ActFormulaRec, 1);

	fprintf (dumpfile, "    dwFlags (0x%lX) = ", ActFormulaRec.dwFlags);
	if (ACTIONFORMULA_FLAG_SELECTDOCS & ActFormulaRec.dwFlags)
		fprintf (dumpfile, "SELECTDOCS ");
	if (ACTIONFORMULA_FLAG_NEWCOPY & ActFormulaRec.dwFlags)
		fprintf (dumpfile, "NEWCOPY ");
	fprintf (dumpfile, "\n");
	fprintf (dumpfile, "    wFormulaLen = %u\n",
		ActFormulaRec.wFormulaLen);
	PrintFormula (p, ActFormulaRec.wFormulaLen);

    return;
}

/************************************************************************

    FUNCTION:   DumpActionLotusscript

*************************************************************************/

void    LNPUBLIC  DumpActionLotusscript (
	char *	RecordPtr,
	DWORD	RecordLength
) {
    char far *		p = RecordPtr;
    CDACTIONLOTUSSCRIPT	ActScriptRec;

    fprintf (dumpfile, "  CDACTIONLOTUSSCRIPT\n");

    ODSReadMemory (&p, _CDACTIONLOTUSSCRIPT, &ActScriptRec, 1);

	fprintf (dumpfile, "    dwFlags (0x%lX) = (Unused)\n", ActScriptRec.dwFlags);
	fprintf (dumpfile, "    dwScriptLen = %lu\n",
		ActScriptRec.dwScriptLen);
	PrintScriptSource (p, ActScriptRec.dwScriptLen);

    return;
}

/************************************************************************

    FUNCTION:   DumpActionSendmail

*************************************************************************/

void    LNPUBLIC  DumpActionSendmail (
	char *	RecordPtr,
	DWORD	RecordLength
) {
    char far *		p = RecordPtr;
    CDACTIONSENDMAIL	ActSendMailRec;

    fprintf (dumpfile, "  CDACTIONSENDMAIL\n");

    ODSReadMemory (&p, _CDACTIONSENDMAIL, &ActSendMailRec, 1);

	fprintf (dumpfile, "    dwFlags (0x%lX) = ", ActSendMailRec.dwFlags);
	if (ACTIONSENDMAIL_FLAG_INCLUDEDOC & ActSendMailRec.dwFlags)
		fprintf (dumpfile, "INCLUDEDOC ");
	if (ACTIONSENDMAIL_FLAG_INCLUDELINK & ActSendMailRec.dwFlags)
		fprintf (dumpfile, "INCLUDELINK ");
	if (ACTIONSENDMAIL_FLAG_SAVEMAIL & ActSendMailRec.dwFlags)
		fprintf (dumpfile, "SAVEMAIL ");
	if (ACTIONSENDMAIL_FLAG_TOFORMULA & ActSendMailRec.dwFlags)
		fprintf (dumpfile, "TOFORMULA ");
	if (ACTIONSENDMAIL_FLAG_CCFORMULA & ActSendMailRec.dwFlags)
		fprintf (dumpfile, "CCFORMULA ");
	if (ACTIONSENDMAIL_FLAG_BCCFORMULA & ActSendMailRec.dwFlags)
		fprintf (dumpfile, "BCCFORMULA ");
	if (ACTIONSENDMAIL_FLAG_SUBJECTFORMULA & ActSendMailRec.dwFlags)
		fprintf (dumpfile, "SUBJECTFORMULA ");
	fprintf (dumpfile, "\n");

	fprintf (dumpfile, "    wFieldLen[TOFIELD] = %u\n",
		ActSendMailRec.wFieldLen[ACTIONSENDMAIL_TOFIELD]);
	fprintf (dumpfile, "    To Field = ");
	if (ActSendMailRec.wFieldLen[ACTIONSENDMAIL_TOFIELD])
	{
		if (ACTIONSENDMAIL_FLAG_TOFORMULA & ActSendMailRec.dwFlags)
			PrintFormula (p, ActSendMailRec.wFieldLen[ACTIONSENDMAIL_TOFIELD]);
		else
			PrintText (p, ActSendMailRec.wFieldLen[ACTIONSENDMAIL_TOFIELD]);
		p += ActSendMailRec.wFieldLen[ACTIONSENDMAIL_TOFIELD];
	}
	else
		fprintf (dumpfile, "\n");

	fprintf (dumpfile, "    wFieldLen[CCFIELD] = %u\n",
		ActSendMailRec.wFieldLen[ACTIONSENDMAIL_CCFIELD]);
	fprintf (dumpfile, "    cc Field = ");
	if (ActSendMailRec.wFieldLen[ACTIONSENDMAIL_CCFIELD])
	{
		if (ACTIONSENDMAIL_FLAG_CCFORMULA & ActSendMailRec.dwFlags)
			PrintFormula (p, ActSendMailRec.wFieldLen[ACTIONSENDMAIL_CCFIELD]);
		else
			PrintText (p, ActSendMailRec.wFieldLen[ACTIONSENDMAIL_CCFIELD]);
		p += ActSendMailRec.wFieldLen[ACTIONSENDMAIL_CCFIELD];
	}
	else
		fprintf (dumpfile, "\n");

	fprintf (dumpfile, "    wFieldLen[BCCFIELD] = %u\n",
		ActSendMailRec.wFieldLen[ACTIONSENDMAIL_BCCFIELD]);
	fprintf (dumpfile, "    bcc Field = ");
	if (ActSendMailRec.wFieldLen[ACTIONSENDMAIL_BCCFIELD])
	{
		if (ACTIONSENDMAIL_FLAG_BCCFORMULA & ActSendMailRec.dwFlags)
			PrintFormula (p, ActSendMailRec.wFieldLen[ACTIONSENDMAIL_BCCFIELD]);
		else
			PrintText (p, ActSendMailRec.wFieldLen[ACTIONSENDMAIL_BCCFIELD]);
		p += ActSendMailRec.wFieldLen[ACTIONSENDMAIL_BCCFIELD];
	}
	else
		fprintf (dumpfile, "\n");

	fprintf (dumpfile, "    wFieldLen[SUBJECTFIELD] = %u\n",
		ActSendMailRec.wFieldLen[ACTIONSENDMAIL_SUBJECTFIELD]);
	fprintf (dumpfile, "    Subject Field = ");
	if (ActSendMailRec.wFieldLen[ACTIONSENDMAIL_SUBJECTFIELD])
	{
		if (ACTIONSENDMAIL_FLAG_SUBJECTFORMULA & ActSendMailRec.dwFlags)
			PrintFormula (p, ActSendMailRec.wFieldLen[ACTIONSENDMAIL_SUBJECTFIELD]);
		else
			PrintText (p, ActSendMailRec.wFieldLen[ACTIONSENDMAIL_SUBJECTFIELD]);
		p += ActSendMailRec.wFieldLen[ACTIONSENDMAIL_SUBJECTFIELD];
	}
	else
		fprintf (dumpfile, "\n");

	fprintf (dumpfile, "    wFieldLen[BODYFIELD] = %u\n",
		ActSendMailRec.wFieldLen[ACTIONSENDMAIL_BODYFIELD]);
	fprintf (dumpfile, "    Body Field = ");
	if (ActSendMailRec.wFieldLen[ACTIONSENDMAIL_BODYFIELD])
	{
		PrintText (p, ActSendMailRec.wFieldLen[ACTIONSENDMAIL_BODYFIELD]);
		p += ActSendMailRec.wFieldLen[ACTIONSENDMAIL_BODYFIELD];
	}
	else
		fprintf (dumpfile, "\n");

    return;
}

/************************************************************************

    FUNCTION:   DumpActionDbcopy

*************************************************************************/

void    LNPUBLIC  DumpActionDbcopy (
	char *	RecordPtr,
	DWORD	RecordLength
) {
    char far *		p = RecordPtr;
    CDACTIONDBCOPY	ActCopyRec;

    fprintf (dumpfile, "  CDACTIONDBCOPY\n");

    ODSReadMemory (&p, _CDACTIONDBCOPY, &ActCopyRec, 1);

	fprintf (dumpfile, "    dwFlags (0x%lX) = ", ActCopyRec.dwFlags);
	if (ACTIONDBCOPY_FLAG_MOVE & ActCopyRec.dwFlags)
		fprintf (dumpfile, "MOVE ");
	fprintf (dumpfile, "\n");

	fprintf (dumpfile, "    wServerLen = %lu\n",
		ActCopyRec.wServerLen);
	fprintf (dumpfile, "    wDatabaseLen = %lu\n",
		ActCopyRec.wDatabaseLen);
	fprintf (dumpfile, "    ServerName = ");
	if (ActCopyRec.wServerLen)
		PrintText (p, ActCopyRec.wServerLen);
	else
		fprintf (dumpfile, "\n");

	fprintf (dumpfile, "    DatabasePath = ");
	if (ActCopyRec.wDatabaseLen)
		PrintText (p, ActCopyRec.wDatabaseLen);
	else
		fprintf (dumpfile, "\n");

    return;
}

/************************************************************************

    FUNCTION:   DumpActionDelete

*************************************************************************/

void    LNPUBLIC  DumpActionDelete (
	char *	RecordPtr,
	DWORD	RecordLength
) {
    char far *		p = RecordPtr;
    CDACTIONDELETE	ActDeleteRec;

    fprintf (dumpfile, "  CDACTIONDELETE\n");

    ODSReadMemory (&p, _CDACTIONDELETE, &ActDeleteRec, 1);

	fprintf (dumpfile, "    dwFlags (0x%lX) = (Unused)\n", ActDeleteRec.dwFlags);

    return;
}

/************************************************************************

    FUNCTION:   DumpActionReadMarks

*************************************************************************/

void    LNPUBLIC  DumpActionReadMarks (
	char *	RecordPtr,
	DWORD	RecordLength,
	WORD	RecordType
) {
    char far *		p = RecordPtr;
    CDACTIONREADMARKS	ActMarkRec;

	if (SIG_ACTION_MARKREAD == RecordType)
    	fprintf (dumpfile, "  ACTION: Mark Read (CDACTIONREADMARKS)\n");
	else if (SIG_ACTION_MARKUNREAD == RecordType)
    	fprintf (dumpfile, "  ACTION: Mark Unread (CDACTIONREADMARKS)\n");
	else
    	fprintf (dumpfile, "  Unknown record type: %u (0x%X)\n",
    		RecordType, RecordType);

    ODSReadMemory (&p, _CDACTIONREADMARKS, &ActMarkRec, 1);

	fprintf (dumpfile, "    dwFlags (0x%lX) = (Unused)\n", ActMarkRec.dwFlags);

    return;
}

/************************************************************************

    FUNCTION:   DumpActionByform

*************************************************************************/

void    LNPUBLIC  DumpActionByform (
	char *	RecordPtr,
	DWORD	RecordLength
) {
    char *			p = RecordPtr;
    CDACTIONBYFORM	ActByFormRec;
	WORD			index;

    fprintf (dumpfile, "  CDACTIONBYFORM\n");

    ODSReadMemory (&p, _CDACTIONBYFORM, &ActByFormRec, 1);

	fprintf (dumpfile, "    dwFlags (0x%lX) = (Unused)\n",
		ActByFormRec.dwFlags);
	fprintf (dumpfile, "    wFieldCount = %u\n",
		ActByFormRec.wFieldCount);
	fprintf (dumpfile, "    wFormNameLen = %u\n",
		ActByFormRec.wFormNameLen);
	for (index = 0; index < ActByFormRec.wFieldCount; index++)
	{
		fprintf (dumpfile, "    Field data (ODS_ASSISTFIELDSTRUCT[%d]):\n", index);
		DumpODSAssistFieldStruct ( &p, sz_4_blankSpaces, ActionByFieldOpsTable);
	} 
	fprintf (dumpfile, "    FormName = ");
	if (ActByFormRec.wFormNameLen)
	{
		PrintText (p, ActByFormRec.wFormNameLen);
		p += ActByFormRec.wFormNameLen;
	}
	else
		fprintf (dumpfile, "\n");

	return;
}

/************************************************************************

    FUNCTION:   DumpActionFolder

*************************************************************************/

void    LNPUBLIC  DumpActionFolder (
	char *	RecordPtr,
	DWORD	RecordLength,
	WORD	RecordType
) {
    char far *		p = RecordPtr;
    CDACTIONFOLDER	ActFolderRec;

	if (SIG_ACTION_MOVETOFOLDER == RecordType)
    	fprintf (dumpfile, "  ACTION: Move To Folder (CDACTIONFOLDER)\n");
	if (SIG_ACTION_COPYTOFOLDER == RecordType)
    	fprintf (dumpfile, "  ACTION: Copy To Folder (CDACTIONFOLDER)\n");
	if (SIG_ACTION_REMOVEFROMFOLDER == RecordType)
    	fprintf (dumpfile, "  ACTION: Remove From Folder (CDACTIONFOLDER)\n");
	else
		fprintf (dumpfile, "  Unknown folder action: %u (0x%X)\n",
			RecordType, RecordType);

    ODSReadMemory (&p, _CDACTIONFOLDER, &ActFolderRec, 1);

	fprintf (dumpfile, "    dwFlags (0x%lX) = ",
		ActFolderRec.dwFlags);
	if (ACTIONFOLDER_FLAG_NEWFOLDER & ActFolderRec.dwFlags)
		fprintf (dumpfile, "NEWFOLDER ");
	if (ACTIONFOLDER_FLAG_PRIVATEFOLDER & ActFolderRec.dwFlags)
		fprintf (dumpfile, "PRIVATEFOLDER ");
	fprintf (dumpfile, "\n");

	fprintf (dumpfile, "    wFolderNameLen = %u\n",
		ActFolderRec.wFolderNameLen);
	fprintf (dumpfile, "    wSpare = %u (0x%X)\n",
		ActFolderRec.wSpare, ActFolderRec.wSpare);
	fprintf (dumpfile, "    FolderName = ");
	if (ActFolderRec.wFolderNameLen)
		PrintText (p, ActFolderRec.wFolderNameLen);
	else
		fprintf (dumpfile, "\n");

    return;
}

/************************************************************************

    FUNCTION:   DumpActionNewsletter

*************************************************************************/

void    LNPUBLIC  DumpActionNewsletter (
	char *	RecordPtr,
	DWORD	RecordLength
) {
    char far *		p = RecordPtr;
    CDACTIONNEWSLETTER	ActNewsletterRec;

    fprintf (dumpfile, "  CDACTIONNEWSLETTER\n");

    ODSReadMemory (&p, _CDACTIONNEWSLETTER, &ActNewsletterRec, 1);

	fprintf (dumpfile, "    dwFlags (0x%lX) = ", ActNewsletterRec.dwFlags);
	if (ACTIONNEWSLETTER_FLAG_SUMMARY & ActNewsletterRec.dwFlags)
		fprintf (dumpfile, "SUMMARY ");
	if (ACTIONNEWSLETTER_FLAG_GATHER & ActNewsletterRec.dwFlags)
		fprintf (dumpfile, "GATHER ");
	if (ACTIONNEWSLETTER_FLAG_INCLUDEALL & ActNewsletterRec.dwFlags)
		fprintf (dumpfile, "INCLUDEALL ");
	if (ACTIONSENDMAIL_FLAG_TOFORMULA & ActNewsletterRec.dwFlags)
		fprintf (dumpfile, "TOFORMULA ");
	if (ACTIONSENDMAIL_FLAG_CCFORMULA & ActNewsletterRec.dwFlags)
		fprintf (dumpfile, "CCFORMULA ");
	if (ACTIONSENDMAIL_FLAG_BCCFORMULA & ActNewsletterRec.dwFlags)
		fprintf (dumpfile, "BCCFORMULA ");
	if (ACTIONSENDMAIL_FLAG_SUBJECTFORMULA & ActNewsletterRec.dwFlags)
		fprintf (dumpfile, "SUBJECTFORMULA ");
	fprintf (dumpfile, "\n");

	fprintf (dumpfile, "    dwGather = %lu\n",
		ActNewsletterRec.dwGather);
	fprintf (dumpfile, "    wViewNameLen = %u\n",
		ActNewsletterRec.wViewNameLen);
	fprintf (dumpfile, "    wSpare = %u\n",
		ActNewsletterRec.wSpare);

	fprintf (dumpfile, "    ViewName = ");
	if (ActNewsletterRec.wViewNameLen)
	{
		PrintText (p, ActNewsletterRec.wViewNameLen);
		p += ActNewsletterRec.wViewNameLen;
	}
	else
		fprintf (dumpfile, "\n");

	fprintf (dumpfile, "    wFieldLen[TOFIELD] = %u\n",
		ActNewsletterRec.wFieldLen[ACTIONSENDMAIL_TOFIELD]);
	fprintf (dumpfile, "    To Field = ");
	if (ActNewsletterRec.wFieldLen[ACTIONSENDMAIL_TOFIELD])
	{
		if (ACTIONSENDMAIL_FLAG_TOFORMULA & ActNewsletterRec.dwFlags)
			PrintFormula (p, ActNewsletterRec.wFieldLen[ACTIONSENDMAIL_TOFIELD]);
		else
			PrintText (p, ActNewsletterRec.wFieldLen[ACTIONSENDMAIL_TOFIELD]);
		p += ActNewsletterRec.wFieldLen[ACTIONSENDMAIL_TOFIELD];
	}
	else
		fprintf (dumpfile, "\n");

	fprintf (dumpfile, "    wFieldLen[CCFIELD] = %u\n",
		ActNewsletterRec.wFieldLen[ACTIONSENDMAIL_CCFIELD]);
	fprintf (dumpfile, "    cc Field = ");
	if (ActNewsletterRec.wFieldLen[ACTIONSENDMAIL_CCFIELD])
	{
		if (ACTIONSENDMAIL_FLAG_CCFORMULA & ActNewsletterRec.dwFlags)
			PrintFormula (p, ActNewsletterRec.wFieldLen[ACTIONSENDMAIL_CCFIELD]);
		else
			PrintText (p, ActNewsletterRec.wFieldLen[ACTIONSENDMAIL_CCFIELD]);
		p += ActNewsletterRec.wFieldLen[ACTIONSENDMAIL_CCFIELD];
	}
	else
		fprintf (dumpfile, "\n");

	fprintf (dumpfile, "    wFieldLen[BCCFIELD] = %u\n",
		ActNewsletterRec.wFieldLen[ACTIONSENDMAIL_BCCFIELD]);
	fprintf (dumpfile, "    bcc Field = ");
	if (ActNewsletterRec.wFieldLen[ACTIONSENDMAIL_BCCFIELD])
	{
		if (ACTIONSENDMAIL_FLAG_BCCFORMULA & ActNewsletterRec.dwFlags)
			PrintFormula (p, ActNewsletterRec.wFieldLen[ACTIONSENDMAIL_BCCFIELD]);
		else
			PrintText (p, ActNewsletterRec.wFieldLen[ACTIONSENDMAIL_BCCFIELD]);
		p += ActNewsletterRec.wFieldLen[ACTIONSENDMAIL_BCCFIELD];
	}
	else
		fprintf (dumpfile, "\n");

	fprintf (dumpfile, "    wFieldLen[SUBJECTFIELD] = %u\n",
		ActNewsletterRec.wFieldLen[ACTIONSENDMAIL_SUBJECTFIELD]);
	fprintf (dumpfile, "    Subject Field = ");
	if (ActNewsletterRec.wFieldLen[ACTIONSENDMAIL_SUBJECTFIELD])
	{
		if (ACTIONSENDMAIL_FLAG_SUBJECTFORMULA & ActNewsletterRec.dwFlags)
			PrintFormula (p, ActNewsletterRec.wFieldLen[ACTIONSENDMAIL_SUBJECTFIELD]);
		else
			PrintText (p, ActNewsletterRec.wFieldLen[ACTIONSENDMAIL_SUBJECTFIELD]);
		p += ActNewsletterRec.wFieldLen[ACTIONSENDMAIL_SUBJECTFIELD];
	}
	else
		fprintf (dumpfile, "\n");

	fprintf (dumpfile, "    wFieldLen[BODYFIELD] = %u\n",
		ActNewsletterRec.wFieldLen[ACTIONSENDMAIL_BODYFIELD]);
	fprintf (dumpfile, "    Body Field = ");
	if (ActNewsletterRec.wFieldLen[ACTIONSENDMAIL_BODYFIELD])
	{
		PrintText (p, ActNewsletterRec.wFieldLen[ACTIONSENDMAIL_BODYFIELD]);
		p += ActNewsletterRec.wFieldLen[ACTIONSENDMAIL_BODYFIELD];
	}
	else
		fprintf (dumpfile, "\n");

    return;
}

/************************************************************************

    FUNCTION:   DumpActionRunagent

*************************************************************************/

void    LNPUBLIC  DumpActionRunagent (
	char *	RecordPtr,
	DWORD	RecordLength
) {
    char far *		p = RecordPtr;
    CDACTIONRUNAGENT	ActAgentRec;

    fprintf (dumpfile, "  CDACTIONRUNAGENT\n");

    ODSReadMemory (&p, _CDACTIONRUNAGENT, &ActAgentRec, 1);

	fprintf (dumpfile, "    dwFlags (0x%lX) = (Unused)\n",
		ActAgentRec.dwFlags);

	fprintf (dumpfile, "    wAgentNameLen = %u\n",
		ActAgentRec.wAgentNameLen);
	fprintf (dumpfile, "    wSpare = %u (0x%X)\n",
		ActAgentRec.wSpare, ActAgentRec.wSpare);
	fprintf (dumpfile, "    AgentName = ");
	if (ActAgentRec.wAgentNameLen)
		PrintText (p, ActAgentRec.wAgentNameLen);
	else
		fprintf (dumpfile, "\n");

    return;
}

/************************************************************************

    FUNCTION:   DumpActionSenddocument

*************************************************************************/

void    LNPUBLIC  DumpActionSenddocument (
	char *	RecordPtr,
	DWORD	RecordLength
) {
    char far *		p = RecordPtr;
    CDACTIONSENDDOCUMENT	ActSendDocRec;

    fprintf (dumpfile, "  CDACTIONSENDDOCUMENT\n");

    ODSReadMemory (&p, _CDACTIONSENDDOCUMENT, &ActSendDocRec, 1);

	fprintf (dumpfile, "    dwFlags (0x%lX) = (Unused)\n",
		ActSendDocRec.dwFlags);

    return;
}

/************************************************************************

    FUNCTION:   DumpActionJavaAgent

*************************************************************************/

void    LNPUBLIC  DumpActionJavaAgent (
	char *	RecordPtr,
	DWORD	RecordLength
) {
    char far *		p = RecordPtr;
    CDACTIONJAVAAGENT	ActJavaAgent;

    fprintf (dumpfile, "  CDACTIONJAVAAGENT\n");

    ODSReadMemory (&p, _CDACTIONJAVAAGENT, &ActJavaAgent, 1);

	fprintf (dumpfile, "    wClassNameLen  = %d\n", ActJavaAgent.wClassNameLen);
	fprintf (dumpfile, "    wCodePathLen   = %d\n", ActJavaAgent.wCodePathLen);
	fprintf (dumpfile, "    wFileListBytes = %d\n", ActJavaAgent.wFileListBytes);
    fprintf (dumpfile, "    wLibraryListBytes = %d\n", ActJavaAgent.wLibraryListBytes);
    fprintf (dumpfile, "    wSpare[0]      = %d\n", ActJavaAgent.wSpare[0]);
    fprintf (dumpfile, "    wSpare[1]      = %d\n", ActJavaAgent.wSpare[1]);
	fprintf (dumpfile, "    dwSpare[0]     = %d\n", ActJavaAgent.dwSpare[0]);
	fprintf (dumpfile, "    dwSpare[1]     = %d\n", ActJavaAgent.dwSpare[1]);

		/* Print the class name string */
	fprintf (dumpfile, "      Class name = ");
	if (0 != ActJavaAgent.wClassNameLen)
	{
		PrintText (p, ActJavaAgent.wClassNameLen);
		p += ActJavaAgent.wClassNameLen;
	}
	else
	{
		fprintf (dumpfile, "\n");
	}

		/* Print the code path string */
	fprintf (dumpfile, "      Code string = ");
	if (0 != ActJavaAgent.wCodePathLen)
	{
		PrintText (p, ActJavaAgent.wCodePathLen);
		p += ActJavaAgent.wCodePathLen;
	}
	else
	{
		fprintf (dumpfile, "\n");
	}

		/* Print the file list string */
	fprintf (dumpfile, "      File list = ");
	if (0 != ActJavaAgent.wFileListBytes)
	{
		PrintText (p, ActJavaAgent.wFileListBytes);
		p += ActJavaAgent.wFileListBytes;
	}
	else
	{
		fprintf (dumpfile, "\n");
	}

    return;
}

/************************************************************************

	FUNCTION:   PickApartTime

	PURPOSE:	Divide a time value into hours, minutes, seconds, and
				milliseconds (NOT ticks!!)

************************************************************************/

void PickApartTime (
	DWORD		time,
	WORD *		pHours,
	WORD *		pMinutes,
	WORD *		pSeconds,
	WORD *		pMillies
) {
	*pHours = (WORD) (time / TICKS_IN_HOUR);
	time %= TICKS_IN_HOUR;
	*pMinutes = (WORD) (time / TICKS_IN_MINUTE);
	time %= TICKS_IN_MINUTE;
	*pSeconds = (WORD) (time / TICKS_IN_SECOND);
	time %= TICKS_IN_SECOND;
	*pMillies = (WORD) (10 * time);
}

/************************************************************************

    FUNCTION:   DumpAgentInfoItem

************************************************************************/

void    LNPUBLIC  DumpAgentInfoItem (
	char *  Name,
	WORD    NameLength,
	char *  pData,
	WORD    length
) {
	char *	p = pData;
	ODS_ASSISTSTRUCT	assistStruct;
	char *	pMessage;
	WORD	hours;
	WORD	minutes;
	WORD	seconds;
	WORD	millies;

	static char szSunday[] = "Sunday";
        static char szMonday[] = "Monday";
        static char szTuesday[] = "Tuesday";
        static char szWednesday[] = "Wednesday";
        static char szThursday[] = "Thursday";
        static char szFriday[] = "Friday";
        static char szSaturday[] = "Saturday";

	char *	DaysOfWeek [7] =
		{
			szSunday,
			szMonday,
			szTuesday,
			szWednesday,
			szThursday,
			szFriday,
			szSaturday
		};
    char        szTimedate[MAXALPHATIMEDATE+1];

	fprintf (dumpfile, "    ODS_ASSISTSTRUCT\n");

	ODSReadMemory ( &p, _ODS_ASSISTSTRUCT, &assistStruct, 1);

	fprintf (dumpfile, "      wVersion = %u\n",
		assistStruct.wVersion);
	fprintf (dumpfile, "      dwFlags (0x%lX) = ",
		assistStruct.dwFlags);
	if (ASSISTODS_FLAG_HIDDEN & assistStruct.dwFlags)
		fprintf (dumpfile, "HIDDEN ");
	if (ASSISTODS_FLAG_NOWEEKENDS & assistStruct.dwFlags)
		fprintf (dumpfile, "NOWEEKENDS ");
	if (ASSISTODS_FLAG_STOREHIGHLIGHTS & assistStruct.dwFlags)
		fprintf (dumpfile, "STOREHIGHLIGHTS ");
	if (ASSISTODS_FLAG_MAILANDPASTE & assistStruct.dwFlags)
		fprintf (dumpfile, "MAILANDPASTE ");
	if (ASSISTODS_FLAG_CHOOSEWHENENABLED & assistStruct.dwFlags)
		fprintf (dumpfile, "CHOOSEWHENENABLED ");
	fprintf (dumpfile, "\n");

	pMessage = GetLiteralString (AssistTriggerTypeTable, assistStruct.wTriggerType);
	fprintf (dumpfile, "      TriggerType (%u) = %s\n",
		assistStruct.wTriggerType, pMessage);
	pMessage = GetLiteralString (AssistSearchTypeTable, assistStruct.wSearchType);
	fprintf (dumpfile, "      SearchType (%u) = %s\n",
		assistStruct.wSearchType, pMessage);
	pMessage = GetLiteralString (AssistIntervalTypeTable, assistStruct.wIntervalType);
	fprintf (dumpfile, "      IntervalType (%u) = %s\n",
		assistStruct.wIntervalType, pMessage);

	switch (assistStruct.wIntervalType)
	{
		case 1:		/* Minutes */
			fprintf (dumpfile, "      wInterval = %u minutes\n",
				assistStruct.wInterval);
			PickApartTime (assistStruct.dwTime1, &hours, &minutes,
				&seconds, &millies);
			fprintf (dumpfile,
				"      dwTime1 (start time-of-day) = %2u:%2.2u:%2.2u.%3.3u\n",
				hours, minutes, seconds, millies);
			PickApartTime (assistStruct.dwTime2, &hours, &minutes,
				&seconds, &millies);
			fprintf (dumpfile,
				"      dwTime2 (end time-of-day)   = %2u:%2.2u:%2.2u.%3.3u\n",
				hours, minutes, seconds, millies);
			break;

		case 2:		/* Days */
			fprintf (dumpfile, "      wInterval = %u days\n",
				assistStruct.wInterval);
			PickApartTime (assistStruct.dwTime1, &hours, &minutes,
				&seconds, &millies);
			fprintf (dumpfile,
				"      dwTime1 (start time-of-day) = %2u:%2.2u:%2.2u.%3.3u\n",
				hours, minutes, seconds, millies);
			fprintf (dumpfile, "      dwTime2 = (unused - 0x%lX)\n",
				assistStruct.dwTime2);
			break;

		case 3:		/* Weeks */
			fprintf (dumpfile, "      wInterval = %u weeks\n",
				assistStruct.wInterval);
			PickApartTime (assistStruct.dwTime1, &hours, &minutes,
				&seconds, &millies);
			fprintf (dumpfile,
				"      dwTime1 (start time-of-day) = %2u:%2.2u:%2.2u.%3.3u\n",
				hours, minutes, seconds, millies);
			fprintf (dumpfile, "      dwTime2 (%lu - day of week) = %s\n",
				assistStruct.dwTime2, DaysOfWeek[(assistStruct.dwTime2 - 1) % 7]);
			break;

		case 4:		/* Months */
			fprintf (dumpfile, "      wInterval = %u months\n",
				assistStruct.wInterval);
			PickApartTime (assistStruct.dwTime1, &hours, &minutes,
				&seconds, &millies);
			fprintf (dumpfile,
				"      dwTime1 (start time-of-day) = %2u:%2.2u:%2.2u.%3.3u\n",
				hours, minutes, seconds, millies);
			fprintf (dumpfile, "      dwTime2 (day of month) = %u\n",
				assistStruct.dwTime2);
			break;

		default:
			break;
	}

    GetTDString (&assistStruct.StartTime, szTimedate);

#ifndef APITESTMODE
    fprintf (dumpfile, "    StartTime = %s\n", szTimedate);
#endif
    
    GetTDString (&assistStruct.EndTime, szTimedate);

#ifndef APITESTMODE
    fprintf (dumpfile, "    EndTime = %s\n", szTimedate);
#endif

    return;
}
#ifdef __cplusplus
}
#endif
