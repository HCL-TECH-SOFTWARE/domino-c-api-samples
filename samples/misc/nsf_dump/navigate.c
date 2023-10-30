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

    FILE:       navigate.c

    PROGRAM:    nsf_dump

    DESCRIPTION:
        This file contains routines that print the contents of
        Viewmap records to the dumpfile. These routines mostly just
        dump structures defined in HCL C API for Notes/Domino header
		file vmods.h.

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

/* Notes API include files */
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
#include <vmods.h>

/* NSF_DUMP - specific include file */
#include "nsf_dump.h"

/*
 *		Widely used strings
 */

char DumpStringTrue		[] = "True";
char DumpStringFalse    [] = "False";
char sz_4_blankSpaces1  [] = "    ";
char sz_6_blankSpaces   [] = "      ";
char sz_8_blankSpaces   [] = "        ";

/*
 *  VMActionStringTable - used by GetLiteralString()
 */
char szVM_ACTION_NONE[]         = "VM_ACTION_NONE";
char szVM_ACTION_SWITCHVIEW[]   = "VM_ACTION_SWITCHVIEW";
char szVM_ACTION_SWITCHNAV[]    = "VM_ACTION_SWITCHNAV";
char szVM_ACTION_ALIAS_FOLDER[] = "VM_ACTION_ALIAS_FOLDER";
char szVM_ACTION_GOTO_LINK[]    = "VM_ACTION_GOTO_LINK";
char szVM_ACTION_RUNSCRIPT[]    = "VM_ACTION_RUNSCRIPT";
char szVM_ACTION_RUNFORMULA[]   = "VM_ACTION_RUNFORMULA";
char szUNKNOWN_ACTION[]         = "UNKNOWN ACTION";

LITERAL_NAME_ENTRY VMActionStringTable [] =
{
	{VM_ACTION_NONE,			szVM_ACTION_NONE},
	{VM_ACTION_SWITCHVIEW,		szVM_ACTION_SWITCHVIEW},
	{VM_ACTION_SWITCHNAV,		szVM_ACTION_SWITCHNAV},
	{VM_ACTION_ALIAS_FOLDER,	szVM_ACTION_ALIAS_FOLDER},
	{VM_ACTION_GOTO_LINK,		szVM_ACTION_GOTO_LINK},
	{VM_ACTION_RUNSCRIPT,		szVM_ACTION_RUNSCRIPT},
	{VM_ACTION_RUNFORMULA,		szVM_ACTION_RUNFORMULA},

		/* This must be the last entry! */
	{LITERAL_TABLE_END,			szUNKNOWN_ACTION}
};

/*
 *		Component dump routines
 */

	/* Dump a VIEWMAP_HIGHLIGHT_DEFAULTS record */
void LNPUBLIC DumpHighlightDefaults (
	char *pIndent,
	VIEWMAP_HIGHLIGHT_DEFAULTS *pData
) {
	fprintf( dumpfile, "%sHighlight	(VIEWMAP_HIGHLIGHT_DEFAULTS structure):\n",
		pIndent);
	fprintf( dumpfile, "%s  bHighlightTouch	= %s\n",
		pIndent, (pData->bHighlightTouch) ? DumpStringTrue : DumpStringFalse);
	fprintf( dumpfile, "%s  bHighlightCurrent	= %s\n",
		pIndent, (pData->bHighlightCurrent) ? DumpStringTrue : DumpStringFalse);
	fprintf( dumpfile, "%s  HLOutlineColor	= %u\n",
		pIndent, pData->HLOutlineColor);
	fprintf( dumpfile, "%s  HLOutlineWidth	= %u\n",
		pIndent, pData->HLOutlineWidth);
	fprintf( dumpfile, "%s  HLOutlineStyle	= %u\n",
		pIndent, pData->HLOutlineStyle);
	fprintf( dumpfile, "%s  HLFillColor	= %u\n",
		pIndent, pData->HLFillColor);
}

	/* Dump a VMODSdrobj subrecord */
void LNPUBLIC DumpDrawObjectHeader (
	char *pIndent,
	VMODSdrobj *pData
) {
	fprintf( dumpfile, "%sCommon fields (VMODSdrobj record):\n", pIndent);
    fprintf( dumpfile, "%s  ObjRect = [%ld,%ld]-[%ld, %ld]\n", pIndent,
    	pData->ObjRect.left, pData->ObjRect.top,
    	pData->ObjRect.right, pData->ObjRect.bottom);
    fprintf( dumpfile, "%s  flags = %u\n", pIndent, pData->flags );
    fprintf( dumpfile, "%s  NameLen = %u\n", pIndent, pData->NameLen );
    fprintf( dumpfile, "%s  LabelLen = %u\n", pIndent, pData->LabelLen );
	fprintf( dumpfile, "%s  FontID = ",  pIndent);
    DumpFontIDFields ((FONTIDFIELDS *) &(pData->FontID));
    fprintf( dumpfile, "%s  TextColor = %u\n", pIndent, pData->TextColor );
    fprintf( dumpfile, "%s  Alignment = %u\n", pIndent, pData->Alignment );
    fprintf( dumpfile, "%s  bWrap = %s\n", pIndent,
    	(pData->bWrap) ? DumpStringTrue : DumpStringFalse );
}

	/* Dump a VMODSbigobj subrecord */
void LNPUBLIC DumpBigDrawObjectHeader (
	char *pIndent,
	VMODSbigobj *pData
) {
	fprintf( dumpfile, "%sCommon fields (VMODSbigobj record):\n", pIndent);
    fprintf( dumpfile, "%s  ObjRect = [%ld,%ld]-[%ld, %ld]\n", pIndent,
    	pData->ObjRect.left, pData->ObjRect.top,
    	pData->ObjRect.right, pData->ObjRect.bottom);
    fprintf( dumpfile, "%s  flags = %u\n", pIndent, pData->flags );
    fprintf( dumpfile, "%s  NameLen = %u\n", pIndent, pData->NameLen );
    fprintf( dumpfile, "%s  LabelLen = %u\n", pIndent, pData->LabelLen );
	fprintf( dumpfile, "%s  FontID = ",  pIndent);
    DumpFontIDFields ((FONTIDFIELDS *) &(pData->FontID));
    fprintf( dumpfile, "%s  TextColor = %u\n", pIndent, pData->TextColor );
    fprintf( dumpfile, "%s  Alignment = %u\n", pIndent, pData->Alignment );
    fprintf( dumpfile, "%s  bWrap = %s\n", pIndent,
    	(pData->bWrap) ? DumpStringTrue : DumpStringFalse );
}

/************************************************************************

    FUNCTION:   DumpOneViewmapDatasetRecord

    PURPOSE:    Dump one Navigator viewmap dataset record

    ALGORITHM:  This is the action routine for EnumCompositeBuffer
                specified in DumpViewmapDatasetItem().

    INPUTS:
         char      *RecordPtr       -   pointer to start of CD record data
                                        This data is in Canonical format.
         WORD       RecordType      -   CD record type (SIG_VIEWMAP_DATASET)
         DWORD      RecordLength    -   length of CD record
         void far * Unused

*************************************************************************/

STATUS  LNPUBLIC  DumpOneViewmapDatasetRecord (
	char   *RecordPtr,
	WORD    RecordType,
	DWORD   RecordLength,
	void   *Unused
) {
    fprintf( dumpfile,"  Viewmap Dataset Record : %lu\n", ++dwNavigateRecord );
    fprintf( dumpfile,"   Record Length = %lu bytes.\n", RecordLength );
    fprintf( dumpfile,"   Record Type   = " );

    switch (RecordType)
    {
			/* Navigator CD records - added in Release 4.0 */
		case SIG_VIEWMAP_DATASET:
			DumpCDVmdataset( RecordPtr, RecordLength );
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

    FUNCTION:   DumpCDVmdataset

*************************************************************************/

void    LNPUBLIC  DumpCDVmdataset (
	char *	RecordPtr,
	DWORD	RecordLength
) {
    char far *				p = RecordPtr;
    VIEWMAP_DATASET_RECORD	vmDataRec;
    int						i, j;
#define NUMS_PER_LINE	8

    fprintf( dumpfile,"  VIEWMAP_DATASET_RECORD\n" );

    ODSReadMemory( &p, _VIEWMAP_DATASET_RECORD, &vmDataRec, 1 );

    fprintf( dumpfile, "    Version = %u\n",		vmDataRec.Version );
    fprintf( dumpfile, "    ViewNameLen = %u\n",	vmDataRec.ViewNameLen );
    fprintf( dumpfile, "    Gridsize = %u\n",		vmDataRec.Gridsize );
	fprintf( dumpfile, "    Flags (0x%X) = ",		vmDataRec.Flags);
	if (0 != (VM_DSET_SHOW_GRID & vmDataRec.Flags))
		fprintf ( dumpfile, "VM_DSET_SHOW_GRID ");
	if (0 != (VM_DSET_SNAPTO_GRID & vmDataRec.Flags))
		fprintf ( dumpfile, "VM_DSET_SNAPTO_GRID ");
	fprintf ( dumpfile, "\n");
    fprintf( dumpfile, "    bAutoAdjust = %s\n",
    	(vmDataRec.bAutoAdjust) ? DumpStringTrue : DumpStringFalse );
    fprintf( dumpfile, "    BGColor = %u\n",		vmDataRec.BGColor );
	fprintf( dumpfile, "    SeqNums:\n");
	for (i = 0; (i * NUMS_PER_LINE) < VM_MAX_OBJTYPES; i++)
	{																			
		fprintf (dumpfile, "      [%2.2u]: ", (i * NUMS_PER_LINE));
		for (j = 0; (j < NUMS_PER_LINE)
		  && (((i * NUMS_PER_LINE) + j) < VM_MAX_OBJTYPES); j++)
			fprintf (dumpfile, "%5.5u ",
				vmDataRec.SeqNums[(i * NUMS_PER_LINE) + j]);
		fprintf (dumpfile, "\n");
	}

		/* The StyleDefaults record */
	fprintf( dumpfile, "    StyleDefaults (VIEWMAP_STYLE_DEFAULTS structure):\n");

		/* The StyleDefaults.Shapes record */
	fprintf( dumpfile, "      Shapes	(VIEWMAP_SHAPE_DEFAULTS structure):\n");
	DumpHighlightDefaults (sz_8_blankSpaces, &vmDataRec.StyleDefaults.Shapes.Highlight);
	fprintf( dumpfile, "        LineColor	= %u\n",
		vmDataRec.StyleDefaults.Shapes.LineColor);
	fprintf( dumpfile, "        FillFGColor	= %u\n",
		vmDataRec.StyleDefaults.Shapes.FillFGColor);
	fprintf( dumpfile, "        FillBGColor	= %u\n",
		vmDataRec.StyleDefaults.Shapes.FillBGColor);
	fprintf( dumpfile, "        LineStyle	= %u\n",
		vmDataRec.StyleDefaults.Shapes.LineStyle);
	fprintf( dumpfile, "        LineWidth	= %u\n",
		vmDataRec.StyleDefaults.Shapes.LineWidth);
	fprintf( dumpfile, "        FillStyle	= %u\n",
		vmDataRec.StyleDefaults.Shapes.FillStyle);
	fprintf( dumpfile, "        FontID		= ");
    DumpFontIDFields ((FONTIDFIELDS *) &vmDataRec.StyleDefaults.Shapes.FontID);

		/* The StyleDefaults.Lines record */
	fprintf( dumpfile, "      Lines		(VIEWMAP_LINE_DEFAULTS structure):\n");
	DumpHighlightDefaults (sz_8_blankSpaces, &vmDataRec.StyleDefaults.Shapes.Highlight);
	fprintf( dumpfile, "        LineColor	= %u\n",
		vmDataRec.StyleDefaults.Lines.LineColor);
	fprintf( dumpfile, "        FillFGColor	= %u\n",
		vmDataRec.StyleDefaults.Lines.FillFGColor);
	fprintf( dumpfile, "        FillBGColor	= %u\n",
		vmDataRec.StyleDefaults.Lines.FillBGColor);
	fprintf( dumpfile, "        LineStyle	= %u\n",
		vmDataRec.StyleDefaults.Lines.LineStyle);
	fprintf( dumpfile, "        LineWidth	= %u\n",
		vmDataRec.StyleDefaults.Lines.LineWidth);
	fprintf( dumpfile, "        FillStyle	= %u\n",
		vmDataRec.StyleDefaults.Lines.FillStyle);

		/* The StyleDefaults.Regions record */
	fprintf( dumpfile, "      Regions	(VIEWMAP_REGION_DEFAULTS structure):\n");
	DumpHighlightDefaults (sz_8_blankSpaces, &vmDataRec.StyleDefaults.Shapes.Highlight);

		/* The StyleDefaults.Buttons record */
	fprintf( dumpfile, "      Buttons	(VIEWMAP_BUTTON_DEFAULTS structure):\n");
	DumpHighlightDefaults (sz_8_blankSpaces, &vmDataRec.StyleDefaults.Shapes.Highlight);
	fprintf( dumpfile, "        LineColor	= %u\n",
		vmDataRec.StyleDefaults.Buttons.LineColor);
	fprintf( dumpfile, "        FillFGColor	= %u\n",
		vmDataRec.StyleDefaults.Buttons.FillFGColor);
	fprintf( dumpfile, "        FillBGColor	= %u\n",
		vmDataRec.StyleDefaults.Buttons.FillBGColor);
	fprintf( dumpfile, "        LineStyle	= %u\n",
		vmDataRec.StyleDefaults.Buttons.LineStyle);
	fprintf( dumpfile, "        LineWidth	= %u\n",
		vmDataRec.StyleDefaults.Buttons.LineWidth);
	fprintf( dumpfile, "        FillStyle	= %u\n",
		vmDataRec.StyleDefaults.Buttons.FillStyle);
	fprintf( dumpfile, "        FontID		= ");
    DumpFontIDFields ((FONTIDFIELDS *) &vmDataRec.StyleDefaults.Buttons.FontID);

		/* The StyleDefaults.Bitmaps record */
	fprintf( dumpfile, "      Bitmaps	(VIEWMAP_BITMAP_DEFAULTS structure):\n");
	DumpHighlightDefaults (sz_8_blankSpaces, &vmDataRec.StyleDefaults.Shapes.Highlight);

		/* The StyleDefaults.TextBoxes record */
	fprintf( dumpfile, "      TextBoxes	(VIEWMAP_TEXTBOX_DEFAULTS structure):\n");
	DumpHighlightDefaults (sz_8_blankSpaces, &vmDataRec.StyleDefaults.Shapes.Highlight);
	fprintf( dumpfile, "        LineColor	= %u\n",
		vmDataRec.StyleDefaults.TextBoxes.LineColor);
	fprintf( dumpfile, "        FillFGColor	= %u\n",
		vmDataRec.StyleDefaults.TextBoxes.FillFGColor);
	fprintf( dumpfile, "        FillBGColor	= %u\n",
		vmDataRec.StyleDefaults.TextBoxes.FillBGColor);
	fprintf( dumpfile, "        LineStyle	= %u\n",
		vmDataRec.StyleDefaults.TextBoxes.LineStyle);
	fprintf( dumpfile, "        LineWidth	= %u\n",
		vmDataRec.StyleDefaults.TextBoxes.LineWidth);
	fprintf( dumpfile, "        FillStyle	= %u\n",
		vmDataRec.StyleDefaults.TextBoxes.FillStyle);
	fprintf( dumpfile, "        FontID		= ");
    DumpFontIDFields ((FONTIDFIELDS *) &vmDataRec.StyleDefaults.TextBoxes.FontID);
    fprintf( dumpfile, "    NumPaletteEntries = %u\n",	vmDataRec.NumPaletteEntries );
    fprintf( dumpfile, "    ViewDesignType = %u\n",		vmDataRec.ViewDesignType );

	if (0 != vmDataRec.ViewNameLen)
	{
		fprintf( dumpfile, "    Initial view name = ");
		PrintText ( p, vmDataRec.ViewNameLen );
	}

    return;
}

/************************************************************************

    FUNCTION:   DumpOneViewmapLayoutRecord

    PURPOSE:    Dump one Navigator viewmap layout record

    ALGORITHM:  This is the action routine for EnumCompositeBuffer
                specified in DumpViewmapLayoutItem().

    INPUTS:
         char      *RecordPtr       -   pointer to start of CD record data
                                        This data is in Canonical format.
         WORD       RecordType      -   CD record type (SIG_CD_VM*)
         DWORD      RecordLength    -   length of CD record
         void far * Unused

*************************************************************************/

STATUS  LNPUBLIC  DumpOneViewmapLayoutRecord (
	char   *RecordPtr,
	WORD    RecordType,
	DWORD   RecordLength,
	void   *Unused
) {
    fprintf( dumpfile,"  Viewmap Layout Record : %lu\n", ++dwNavigateRecord );
    fprintf( dumpfile,"   Record Length = %lu bytes.\n", RecordLength );
    fprintf( dumpfile,"   Record Type   = " );

    switch (RecordType)
    {
			/* Navigator CD records - added in Release 4.0 */
		case SIG_CD_VMHEADER:
			DumpCDVmheader( RecordPtr, RecordLength );
			break;

		case SIG_CD_VMBITMAP:
			DumpCDVmbitmap( RecordPtr, RecordLength );
			break;

		case SIG_CD_VMRECT:
		case SIG_CD_VMELLIPSE:
		case SIG_CD_VMRNDRECT:
		case SIG_CD_VMBUTTON:
			DumpCDVmrect( RecordPtr, RecordLength, RecordType );
			break;

		case SIG_CD_VMPOLYGON:
		case SIG_CD_VMPOLYRGN:
			DumpCDVmpolygon( RecordPtr, RecordLength, RecordType );
			break;

		case SIG_CD_VMPOLYLINE:
			DumpCDVmpolyline( RecordPtr, RecordLength );
			break;

		case SIG_CD_VMCIRCLE:
		case SIG_CD_VMREGION:
			DumpCDVmregion( RecordPtr, RecordLength, RecordType );
			break;

		case SIG_CD_VMACTION:
		case SIG_CD_VMACTION_2:
			DumpCDVmaction( RecordPtr, RecordLength, RecordType );
			break;

		case SIG_CD_VMTEXTBOX:
			DumpCDVmtextbox( RecordPtr, RecordLength );
			break;

			/* Navigators can contain bitmaps */
        case SIG_CD_BITMAPHEADER:
            DumpCDBitmapheader( RecordPtr, RecordLength );
            break;

        case SIG_CD_BITMAPSEGMENT:
            DumpCDBitmapsegment( RecordPtr, RecordLength );
            break;

        case SIG_CD_COLORTABLE:
            DumpCDColortable( RecordPtr, RecordLength );
            break;

        case SIG_CD_PATTERNTABLE:
            DumpCDPatterntable( RecordPtr, RecordLength );
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

    FUNCTION:   DumpCDVmheader

*************************************************************************/

void    LNPUBLIC  DumpCDVmheader (
	char *	RecordPtr,
	DWORD	RecordLength
) {
    char far *				p = RecordPtr;
    VIEWMAP_HEADER_RECORD	vmRec;

    fprintf( dumpfile,"  VIEWMAP_HEADER_RECORD\n" );

    ODSReadMemory( &p, _VIEWMAP_HEADER_RECORD, &vmRec, 1 );

    fprintf( dumpfile, "    Version = %u\n",		vmRec.Version );
    fprintf( dumpfile, "    NameLen = %u\n",		vmRec.NameLen );

	if (0 != vmRec.NameLen)
	{
		fprintf( dumpfile, "    ViewMap name = ");
		PrintText ( p, vmRec.NameLen );
	}

    return;
}

/************************************************************************

    FUNCTION:   DumpCDVmbitmap

*************************************************************************/

void    LNPUBLIC  DumpCDVmbitmap (
	char *	RecordPtr,
	DWORD	RecordLength
) {
    char far *				p = RecordPtr;
    VIEWMAP_BITMAP_RECORD		vmRec;

    fprintf( dumpfile,"  VIEWMAP_BITMAP_RECORD\n" );

    ODSReadMemory( &p, _VIEWMAP_BITMAP_RECORD, &vmRec, 1 );

	DumpDrawObjectHeader (sz_4_blankSpaces1, &vmRec.DRobj);
    fprintf( dumpfile, "    DataLen = %u\n",	vmRec.DataLen );
    fprintf( dumpfile, "    xBytes = %u\n",	vmRec.xBytes );
    fprintf( dumpfile, "    yBits = %u\n",	vmRec.yBits );
    fprintf( dumpfile, "    zBits = %u\n",	vmRec.zBits );

	if (0 != vmRec.DRobj.NameLen)
	{
		fprintf( dumpfile, "    Instance name = ");
		PrintText ( p, vmRec.DRobj.NameLen );
		p += vmRec.DRobj.NameLen;
	}

	if (0 != vmRec.DataLen)
	{
	    fprintf( dumpfile, "    <start of bitmap data>\n" );
	    NSFDDUMPDumpMemory( p, vmRec.DataLen );
	    fprintf( dumpfile, "    <end of bitmap data>\n" );
		p += vmRec.DataLen;
	}

    return;
}

/************************************************************************

    FUNCTION:   DumpCDVmrect

*************************************************************************/

void    LNPUBLIC  DumpCDVmrect (
	char *	RecordPtr,
	DWORD	RecordLength,
	WORD	RecordType
) {
    char far *				p = RecordPtr;
    VIEWMAP_RECT_RECORD		vmRec;

	if (SIG_CD_VMRECT == RecordType)
    	fprintf (dumpfile, "  Rectangle - VIEWMAP_RECT_RECORD\n");
	else if (SIG_CD_VMELLIPSE == RecordType)
	    fprintf (dumpfile, "  Ellipse - VIEWMAP_RECT_RECORD\n");
	else if (SIG_CD_VMRNDRECT == RecordType)
	    fprintf (dumpfile, "  Rounded Rectangle - VIEWMAP_RECT_RECORD\n");
	else if (SIG_CD_VMBUTTON == RecordType)
	    fprintf (dumpfile, "  Button - VIEWMAP_RECT_RECORD\n");
	else
		fprintf (dumpfile, "  Unknown rectangle type: %u\n",
			RecordType);

    ODSReadMemory( &p, _VIEWMAP_RECT_RECORD, &vmRec, 1 );

	DumpDrawObjectHeader (sz_4_blankSpaces1, &vmRec.DRobj);
    fprintf( dumpfile, "    LineColor = %u\n",	vmRec.LineColor );
    fprintf( dumpfile, "    FillFGColor = %u\n",	vmRec.FillFGColor );
    fprintf( dumpfile, "    FillBGColor = %u\n",	vmRec.FillBGColor );
    fprintf( dumpfile, "    LineStyle = %u\n",	vmRec.LineStyle );
    fprintf( dumpfile, "    LineWidth = %u\n",	vmRec.LineWidth );
    fprintf( dumpfile, "    FillStyle = %u\n",	vmRec.FillStyle );

	fprintf( dumpfile, "    Instance name = ");
	if (0 != vmRec.DRobj.NameLen)
	{
		PrintText ( p, vmRec.DRobj.NameLen );
		p += vmRec.DRobj.NameLen;
	}
	else
		fprintf (dumpfile, "\n");

	fprintf( dumpfile, "    Label = ");
	if (0 != vmRec.DRobj.LabelLen)
	{
		PrintText ( p, vmRec.DRobj.LabelLen );
		p += vmRec.DRobj.LabelLen;
	}
	else
		fprintf (dumpfile, "\n");

    return;
}

/************************************************************************

    FUNCTION:   DumpCDVmpolygon

*************************************************************************/

void    LNPUBLIC  DumpCDVmpolygon (
	char *	RecordPtr,
	DWORD	RecordLength,
	WORD	RecordType
) {
    char far *				p = RecordPtr;
    VIEWMAP_POLYGON_RECORD	vmRec;
	WORD					i;
	struct {
		LONG	x;
		LONG	y;
	} PolyPoint;

	if (SIG_CD_VMPOLYGON == RecordType)
    	fprintf (dumpfile, "  Polygon - VIEWMAP_POLYGON_RECORD\n");
	else if (SIG_CD_VMPOLYRGN == RecordType)
	    fprintf( dumpfile,"  PolyRgn - VIEWMAP_POLYGON_RECORD\n" );
	else
		fprintf (dumpfile, "  Unknown polygon type code: %u\n",
			RecordType);

    ODSReadMemory( &p, _VIEWMAP_POLYGON_RECORD, &vmRec, 1 );

	DumpBigDrawObjectHeader (sz_4_blankSpaces1, &vmRec.DRobj);
    fprintf( dumpfile, "    LineColor = %u\n",	vmRec.LineColor );
    fprintf( dumpfile, "    FillFGColor = %u\n",	vmRec.FillFGColor );
    fprintf( dumpfile, "    FillBGColor = %u\n",	vmRec.FillBGColor );
    fprintf( dumpfile, "    LineStyle = %u\n",	vmRec.LineStyle );
    fprintf( dumpfile, "    LineWidth = %u\n",	vmRec.LineWidth );
    fprintf( dumpfile, "    FillStyle = %u\n",	vmRec.FillStyle );
    fprintf( dumpfile, "    nPts = %u\n",		vmRec.nPts );

	if (0 != vmRec.DRobj.NameLen)
	{
		fprintf( dumpfile, "    Instance name = ");
		PrintText ( p, vmRec.DRobj.NameLen );
		p += vmRec.DRobj.NameLen;
	}

	if (0 != vmRec.DRobj.LabelLen)
	{
		fprintf( dumpfile, "    Label = ");
		PrintText ( p, vmRec.DRobj.LabelLen );
		p += vmRec.DRobj.LabelLen;
	}

	fprintf( dumpfile, "    Points:\n");
	for (i = 0; i < vmRec.nPts; i++)
	{
		ODSReadMemory ( &p, _LONG, &PolyPoint, 2);
		fprintf (dumpfile, "      [%ld, %ld]\n", PolyPoint.x, PolyPoint.y);
	}

    return;
}

/************************************************************************

    FUNCTION:   DumpCDVmpolyline

*************************************************************************/

void    LNPUBLIC  DumpCDVmpolyline (
	char *	RecordPtr,
	DWORD	RecordLength
) {
    char far *				p = RecordPtr;
    VIEWMAP_POLYLINE_RECORD	vmRec;
	WORD					i;
	struct {
		LONG	x;
		LONG	y;
	} PolyPoint;

    fprintf( dumpfile,"  VIEWMAP_POLYLINE_RECORD\n" );

    ODSReadMemory( &p, _VIEWMAP_POLYLINE_RECORD, &vmRec, 1 );

	DumpBigDrawObjectHeader (sz_4_blankSpaces1, &vmRec.DRobj);
    fprintf( dumpfile, "    LineColor = %u\n",	vmRec.LineColor );
    fprintf( dumpfile, "    LineStyle = %u\n",	vmRec.LineStyle );
    fprintf( dumpfile, "    LineWidth = %u\n",	vmRec.LineWidth );
    fprintf( dumpfile, "    nPts = %u\n",		vmRec.nPts );

	if (0 != vmRec.DRobj.NameLen)
	{
		fprintf( dumpfile, "    Instance name = ");
		PrintText ( p, vmRec.DRobj.NameLen );
		p += vmRec.DRobj.NameLen;
	}

	if (0 != vmRec.DRobj.LabelLen)
	{
		fprintf( dumpfile, "    Label = ");
		PrintText ( p, vmRec.DRobj.LabelLen );
		p += vmRec.DRobj.LabelLen;
	}

	fprintf( dumpfile, "    Points:\n");
	for (i = 0; i < vmRec.nPts; i++)
	{
		ODSReadMemory ( &p, _LONG, &PolyPoint, 2);
		fprintf (dumpfile, "      [%ld, %ld]\n", PolyPoint.x, PolyPoint.y);
	}

    return;
}

/************************************************************************

    FUNCTION:   DumpCDVmregion

*************************************************************************/

void    LNPUBLIC  DumpCDVmregion (
	char *	RecordPtr,
	DWORD	RecordLength,
	WORD	RecordType
) {
    char far *				p = RecordPtr;
    VIEWMAP_REGION_RECORD	vmRec;

	if (SIG_CD_VMREGION == RecordType)
		fprintf( dumpfile,"  Region - VIEWMAP_REGION_RECORD\n" );
	else if (SIG_CD_VMCIRCLE == RecordType)
		fprintf( dumpfile,"  Region - VIEWMAP_REGION_RECORD\n" );
	else
		fprintf( dumpfile, "  Unkown region type: %d (0x%X)\n", RecordType, RecordType);

    ODSReadMemory( &p, _VIEWMAP_REGION_RECORD, &vmRec, 1 );

	DumpDrawObjectHeader (sz_4_blankSpaces1, &vmRec.DRobj);
    fprintf( dumpfile, "    LineColor = %u\n",	vmRec.LineColor );
    fprintf( dumpfile, "    LineStyle = %u\n",	vmRec.LineStyle );
    fprintf( dumpfile, "    LineWidth = %u\n",	vmRec.LineWidth );
    fprintf( dumpfile, "    FillStyle = %u\n",	vmRec.FillStyle );

	if (0 != vmRec.DRobj.NameLen)
	{
		fprintf( dumpfile, "    Instance name = ");
		PrintText ( p, vmRec.DRobj.NameLen );
		p += vmRec.DRobj.NameLen;
	}

	if (0 != vmRec.DRobj.LabelLen)
	{
		fprintf( dumpfile, "    Label = ");
		PrintText ( p, vmRec.DRobj.LabelLen );
		p += vmRec.DRobj.LabelLen;
	}

    return;
}

/************************************************************************

    FUNCTION:   DumpCDVmaction

*************************************************************************/

void    LNPUBLIC  DumpCDVmaction (
	char *	RecordPtr,
	DWORD	RecordLength,
	WORD	RecordType
) {
    char far *				p = RecordPtr;
    VIEWMAP_ACTION_RECORD	vmRec;
	char *	pActionString;

	if (SIG_CD_VMACTION == RecordType)
	    fprintf( dumpfile,"  VIEWMAP_ACTION_RECORD\n" );
	else
		fprintf( dumpfile,"  VIEWMAP_ACTION_RECORD (CD_VMACTION_2)\n" );

    ODSReadMemory( &p, _VIEWMAP_ACTION_RECORD, &vmRec, 1 );

    fprintf( dumpfile, "    bHighlightTouch = %u\n",
    	(vmRec.bHighlightTouch) ? DumpStringTrue : DumpStringFalse);
    fprintf( dumpfile, "    bHighlightCurrent = %u\n",
    	(vmRec.bHighlightCurrent) ? DumpStringTrue : DumpStringFalse);
    fprintf( dumpfile, "    HLOutlineColor = %u\n",		vmRec.HLOutlineColor );
    fprintf( dumpfile, "    HLFillColor = %u\n",		vmRec.HLFillColor );
	pActionString = GetLiteralString (VMActionStringTable, vmRec.ClickAction);
    fprintf( dumpfile, "    ClickAction = %s\n",		pActionString );
    fprintf( dumpfile, "    ActionStringLen = %u\n",	vmRec.ActionStringLen );
    fprintf( dumpfile, "    HLOutlineWidth = %u\n",		vmRec.HLOutlineWidth );
    fprintf( dumpfile, "    HLOutlineStyle = %u\n",		vmRec.HLOutlineStyle );
	fprintf( dumpfile, "    LinkInfo = \n");
	PrintNOTELINK (sz_6_blankSpaces, &vmRec.LinkInfo);
    fprintf( dumpfile, "    ExtDataLen = %u\n",			vmRec.ExtDataLen );

	if (0 != vmRec.ActionStringLen)
	{
		switch (vmRec.ClickAction)
		{
			case VM_ACTION_NONE:
				fprintf (dumpfile, "    Action: No action present\n");
				break;
				
			case VM_ACTION_SWITCHVIEW:
				fprintf (dumpfile, "    Action: Switch to view - view name = ");
				if (vmRec.ActionStringLen)
					PrintText ( p, vmRec.ActionStringLen );
				else
					fprintf (dumpfile, "\n");
				break;				
				
			case VM_ACTION_SWITCHNAV:
				fprintf (dumpfile, "    Action: Switch to Navigator - navigator name = ");
				if (vmRec.ActionStringLen)
					PrintText ( p, vmRec.ActionStringLen );
				else
					fprintf (dumpfile, "\n");
				break;

			case VM_ACTION_ALIAS_FOLDER:
				fprintf (dumpfile, "    Action: Switch to Folder - folder name = ");
				if (vmRec.ActionStringLen)
					PrintText ( p, vmRec.ActionStringLen );
				else
					fprintf (dumpfile, "\n");
				break;				
				
			case VM_ACTION_GOTO_LINK:
				fprintf (dumpfile, "    Action: Go to link\n");
				break;

			case VM_ACTION_RUNSCRIPT:
				fprintf (dumpfile, "    Action: Run script =\n");
				PrintScriptSource ( p, vmRec.ActionStringLen );
				break;

			case VM_ACTION_RUNFORMULA:
				fprintf (dumpfile, "    Action: Run formula =\n");
				PrintFormula ( p, vmRec.ActionStringLen );
				break;

			default:
				fprintf (dumpfile, "    Action: Unknown action type: %u\n",
					vmRec.ClickAction);
				break;
		}
	}

    return;
}

/************************************************************************

    FUNCTION:   DumpCDVmtextbox

*************************************************************************/

void    LNPUBLIC  DumpCDVmtextbox (
	char *	RecordPtr,
	DWORD	RecordLength
) {
    char far *				p = RecordPtr;
    VIEWMAP_TEXT_RECORD		vmRec;

    fprintf( dumpfile,"  TextBox VIEWMAP_TEXT_RECORD\n" );

    ODSReadMemory( &p, _VIEWMAP_TEXT_RECORD, &vmRec, 1 );

	DumpBigDrawObjectHeader (sz_4_blankSpaces1, &vmRec.DRobj);
    fprintf( dumpfile, "    LineColor = %u\n",	vmRec.LineColor );
    fprintf( dumpfile, "    FillFGColor = %u\n",	vmRec.FillFGColor );
    fprintf( dumpfile, "    FillBGColor = %u\n",	vmRec.FillBGColor );
    fprintf( dumpfile, "    LineStyle = %u\n",	vmRec.LineStyle );
    fprintf( dumpfile, "    LineWidth = %u\n",	vmRec.LineWidth );
    fprintf( dumpfile, "    FillStyle = %u\n",	vmRec.FillStyle );

	if (0 != vmRec.DRobj.NameLen)
	{
		fprintf( dumpfile, "    Instance name = ");
		PrintText ( p, vmRec.DRobj.NameLen );
		p += vmRec.DRobj.NameLen;
	}

	if (0 != vmRec.DRobj.LabelLen)
	{
		fprintf( dumpfile, "    Label = ");
		PrintText ( p, vmRec.DRobj.LabelLen );
		p += vmRec.DRobj.LabelLen;
	}

    return;
}

#ifdef __cplusplus
}
#endif