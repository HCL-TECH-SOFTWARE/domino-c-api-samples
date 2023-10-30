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

    PROGRAM:    makeview

    FILE:       makeview.c

    SYNTAX:     makeview

    PURPOSE:    Create a view note via the API

    DESCRIPTION:
        This program creates a view note in the MakeView test database.
        
        The database file, makeview.nsf, is opened.  A new note
        is created and the note class is set to to NOTE_CLASS_VIEW. 
        The view title to set to, Test View.
        
        The selection formula for the view is "@ALL". The formula,
        "@ALL" is compiled to form the basis of the $FORMULA item. 
        
        Then, for each column in the view, a formula that consists 
        of an item name is compiled.  The item name is set as 
        one of the summary items for the view, and the compiled 
        formula is merged into the view selection formula. Then, the
        $Conflict and $REF summary item names are added to the selection 
        formula.  Then the finished, merged $FORMULA item is appended 
        to the note using NSFItemAppend.

        Next, the $VIEWFORMAT item, (also known as "View Table Format" item), 
        which consists of several structures is created and appended.

        Then, the View Collation ($Collation) item, which also 
        consists of several structures is created and appended. A 
        View Collation item is required for views containing sorted 
        or categorized columns.
    
        The new View Note is updated and closed.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/*
 * C header files
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * HCL C API for Notes/Domino Header files.
 */

#include <global.h>
#include <nsfdata.h>
#include <nif.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <editods.h>
#include <osmem.h>
#include <stdnames.h>
#include <nsfsearc.h>
#include <nifcoll.h>
#include <osmisc.h>
#include <viewfmt.h>
#include <colorid.h>
#include <miscerr.h>
#include "../../printLog.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/************************************************************************

    FUNCTION:   main

*************************************************************************/


int main(int argc, char *argv[])
{
    STATUS         sError = NOERROR;
    DBHANDLE       hDB;
    NOTEHANDLE     hNote;
    NOTEID         ViewNoteID;
    char           szViewName[] = "Test View";     /* Title of view to be created. */
    char           szFileName[] = "makeview.nsf";  /* Database in which to create it. */

    WORD           wNumColumns = 3;     /* This view will contain this many columns. */
    WORD           ClassView = NOTE_CLASS_VIEW;
/*
 * Variables pertaining to the view selection formula
 */
 
    char           szSelFormula[] = "@ALL";
    FORMULAHANDLE  hSelFormula = NULLHANDLE;
    WORD           wSelFormulaLen;

/*
 * Variables pertaining to the first column in the view.
 * This will be a category column, sorting by keyword.
 */
 
    char           szFormula_1[] = "KeyWordField";
    FORMULAHANDLE  hFormula_1 = NULLHANDLE;
    WORD           wFormula_1_Len;
    char           szItemName_1[] = "ItemName1";
    WORD           wItemName_1_Len = strlen( szItemName_1 );

/*
 * Variables pertaining to the second column in the view.
 */

    char           szFormula_2[] = "NumberField";
    FORMULAHANDLE  hFormula_2 = NULLHANDLE;
    WORD           wFormula_2_Len;
    char           szTitle_2[] = "Number Column";
    WORD           wTitle_2_Len = strlen(szTitle_2);
    char           szItemName_2[] = "ItemName2";
    WORD           wItemName_2_Len = strlen(szItemName_2);
    
/*
 * Variables pertaining to the third column in the view.
 */

    char           szFormula_3[] = "TextField";
    FORMULAHANDLE  hFormula_3 = NULLHANDLE;
    WORD           wFormula_3_Len;
    char           szTitle_3[] = "Text Column";
    WORD           wTitle_3_Len = strlen( szTitle_3 );
    char           szItemName_3[] = "ItemName3";
    WORD           wItemName_3_Len = strlen( szItemName_3 );

/*
 * Variables pertaining to the $Conflict and $REF
 * summary item names in the view
 */

    char           szConflictName[] = VIEW_CONFLICT_ITEM;
    WORD           wConflictName_Len = strlen (szConflictName);
    char           szRefName[] = FIELD_LINK;
    WORD           wRefName_Len = strlen (szRefName);
 
/*
 * Variables pertaining to the $VIEWFORMAT item.
 */

    VIEW_TABLE_FORMAT  ViewTableFormat;
    VIEW_COLUMN_FORMAT ViewColumnFormat;
    VIEW_TABLE_FORMAT2 ViewTableFormat2;

    WORD           wViewFormatBufLen;
    DHANDLE        hViewFormatBuffer;
    char           *pViewFormatBuffer;
    char           *pVFBuf;

/*
 * Variables pertaining to the Collation buffer ($Collation) item.
 */

    COLLATION      Collation;
    COLLATE_DESCRIPTOR CollateDesc;
    COLLATE_DESCRIPTOR CollateDesc2;
    WORD           wCollationBufLen;
    DHANDLE        hCollationBuffer;
    char           *pCollationBuffer;
    char           *pCBuf;


    WORD            wdc;         /* "We Don't Care" - We're not interested in some of
                                  *  the info passed back by NSFFormulaCompile(), but
                                  *  the function call requires the addresses of several
                                  *  words to be passed in. The address of this word
                                  *  is used for all parameters in which we have no
                                  *  interest .
                                  */

    PRINTLOG("HCL Notes API\nmakeview sample program\n");

    if (sError = NotesInitExtended (argc, argv))
    {
        PRINTLOG("\n Unable to initialize Notes.\n");
        return (1);
    }

   
/*
 * Open database
 */

    if (sError = NSFDbOpen( szFileName, &hDB ))
    {
        PRINTLOG("Error: unable to open database '%s'.\n", szFileName );
        goto Exit2;
    }

/*
 * Check to see if we can find a view with this name already (disallow duplicates)
 */

    if ((sError = NIFFindView(hDB, szViewName, &ViewNoteID)) != ERR_NOT_FOUND)
    {
        PRINTLOG("Error: View named '%s' already exists in database.\n",
                szViewName );
        PRINTLOG("Error: Duplicates not allowed.\n" );
        goto Exit3;
    }

/*
 * Now create a note in database
 */

    sError = NSFNoteCreate( hDB, &hNote );
    if (sError) 
    {
        PRINTLOG("Error: unable to create note in database.\n" );
        goto Exit3;
    }
    
        
/*
 * Set the NOTE_CLASS to NOTE_CLASS_VIEW.
 */
 

    NSFNoteSetInfo( hNote, _NOTE_CLASS, &ClassView );

    
/*
 * Set the view name. ($TITLE field)
 */
     
    sError = NSFItemSetText( hNote,
                             VIEW_TITLE_ITEM,
                             szViewName,
                             MAXWORD );
    if (sError)
    {
        PRINTLOG("Error: unable to set text item '%s' in view note.\n",
                VIEW_TITLE_ITEM );
        goto Exit4;
    }
    
/*
 * Compile and set the selection formula to show all notes.
 */

    sError = NSFFormulaCompile( NULL,
                                0,
                                szSelFormula,
                                (WORD) strlen(szSelFormula),
                                &hSelFormula,
                                &wSelFormulaLen,
                                &wdc, &wdc, &wdc,
                                &wdc, &wdc );

    if (sError)
    {
        PRINTLOG("Error: unable to compile selection formula '%s'.\n", 
                szSelFormula );
        goto Exit4;
    }    

/*
 * Compile the formula for column 1.
 */
     
    sError = NSFFormulaCompile( szItemName_1,
                                wItemName_1_Len,
                                szFormula_1,
                                (WORD) strlen(szFormula_1),
                                &hFormula_1,
                                &wFormula_1_Len,
                                &wdc, &wdc, &wdc,
                                &wdc, &wdc );

    if (sError)
    {
        PRINTLOG("Error: unable to compile column 1 formula '%s'.\n", 
                  szFormula_1 );
        goto Exit5;
    }    


/*
 * Set summary item by merging column 1 formula into selection formula.
 */

    sError = NSFFormulaSummaryItem( hSelFormula,
                                    szItemName_1,
                                    wItemName_1_Len );
    if (sError)
    {
        PRINTLOG("Error: unable to set summary item '%s'.\n", 
                  szItemName_1 );
        goto Exit5;
    }    
    
    
    sError = NSFFormulaMerge( hFormula_1, hSelFormula );
    if (sError)
    {
        PRINTLOG("Error: unable to merge column 1 formula into selection formula.\n");
        goto Exit5;
    }
    

/*
 * Compile the formula for column 2.
 */
     
    sError = NSFFormulaCompile( szItemName_2,
                                wItemName_2_Len,
                                szFormula_2,
                                (WORD) strlen(szFormula_2),
                                &hFormula_2,
                                &wFormula_2_Len,
                                &wdc, &wdc, &wdc,
                                &wdc, &wdc );
    if (sError)
    {
        PRINTLOG("Error: unable to compile column 2 formula '%s'.\n", 
                 szFormula_2 );
        goto Exit5;
    }    


/*
 * Set this item as a summary item by merging the column 2
 * formula into the selection formula.
 */

    sError = NSFFormulaSummaryItem( hSelFormula,
                                    szItemName_2,
                                    wItemName_2_Len );
    if (sError)
    {
        PRINTLOG("Error: unable to merge column 2 item name into selection formula.\n");
        goto Exit5;
    }    

    sError = NSFFormulaMerge( hFormula_2, hSelFormula );
    if (sError)
    {
        PRINTLOG("Error: unable to merge formula 2 into view selection formula.\n");
        goto Exit5;
    }


/*
 * Compile the formula for column 3.
 */
     
    sError = NSFFormulaCompile( szItemName_3,
                                wItemName_3_Len,
                                szFormula_3,
                                (WORD) strlen(szFormula_3),
                                &hFormula_3,
                                &wFormula_3_Len,
                                &wdc, &wdc, &wdc,
                                &wdc, &wdc );

    if (sError)
    {
        PRINTLOG("Error: unable to compile formula for column 3: '%s'.\n", 
                 szFormula_3 );
        goto Exit5;
    }

/*
 * Set this item as a summary item, and merge the column 3
 * formula into the selection formula.
 */

    sError = NSFFormulaSummaryItem( hSelFormula,
                                    szItemName_3,
                                    wItemName_3_Len );

    if (sError)
    {
        PRINTLOG("Error: unable to merge col 3 item name into selection formula.\n");
        goto Exit5;
    }

    sError = NSFFormulaMerge( hFormula_3, hSelFormula );

    if (sError)
    {
        PRINTLOG("Error: unable to merge col 3 formula into selection formula.\n");
        goto Exit5;
    }

/*
 *  Add $Conflict and $REF summary item names to selection formula
 */

    sError = NSFFormulaSummaryItem( hSelFormula,
                                     szConflictName,
                                     wConflictName_Len);
    if (sError)
    {
        PRINTLOG("Error: unable to merge $Conflict item name into selection formula.\n");
        goto Exit5;
    }

     sError = NSFFormulaSummaryItem( hSelFormula,
                                     szRefName,
                                     wRefName_Len);
    if (sError)
    {
        PRINTLOG("Error: unable to merge $REF item name into selection formula.\n");
        goto Exit5;
    }

/*
 * Get the size of the merged formula.
 */

    sError = NSFFormulaGetSize( hSelFormula, &wSelFormulaLen );
    if (sError)
    {
        PRINTLOG("Error: unable to get size of selection formula.\n" );
        goto Exit5;
    }


/*
 * Append the selection formula item to the view note.
 */

    sError = NSFItemAppend( hNote,
                            ITEM_SUMMARY,
                            VIEW_FORMULA_ITEM,
                            (WORD) strlen(VIEW_FORMULA_ITEM),
                            TYPE_FORMULA,
                            OSLockObject(hSelFormula),
                            (DWORD) wSelFormulaLen);

    OSUnlockObject( hSelFormula );

    if (sError) 
    {
        PRINTLOG("Error: unable to append item '%s' to view note.\n",
                 VIEW_FORMULA_ITEM );
        goto Exit4;
    }

/*
 * Create the $VIEWFORMAT item. The $VIEWFORMAT item is an item
 * of TYPE_VIEW_FORMAT with name VIEW_VIEW_FORMAT_ITEM. 
 *
 * The $VIEWFORMAT item for this view will consist of the following 
 * series of structures converted to Domino and Notes canonical format and packed 
 * together:
 *
 *          VIEW_TABLE_FORMAT
 *          VIEW_COLUMN_FORMAT (for column 1)
 *          VIEW_COLUMN_FORMAT (for column 2)
 *          VIEW_COLUMN_FORMAT (for column 3)
 *          Item Name for column 1
 *          formula for column 1
 *          Item Name for column 2
 *          Title for column 2
 *          formula for column 2
 *          Item Name for column 3
 *          Title for column 3
 *          formula for column 3
 *          VIEW_TABLE_FORMAT2
 *          VIEW_COLUMN_FORMAT2 (for column 1)
 *          VIEW_COLUMN_FORMAT2 (for column 2)
 *          VIEW_COLUMN_FORMAT2 (for column 3)
 *          VIEW_TABLE_FORMAT3
 *
 *
 * First, allocate a buffer, pViewFormatBuffer, that will contain the 
 * entire $VIEWFORMAT item. 
 */

    wViewFormatBufLen = ODSLength( _VIEW_TABLE_FORMAT )   +
                        ODSLength( _VIEW_COLUMN_FORMAT )  +
                        ODSLength( _VIEW_COLUMN_FORMAT )  +
                        ODSLength( _VIEW_COLUMN_FORMAT )  +
                        wItemName_1_Len                   +
                        wFormula_1_Len                    +
                        wItemName_2_Len                   +
                        wTitle_2_Len                      +
                        wFormula_2_Len                    +
                        wItemName_3_Len                   +
                        wTitle_3_Len                      +
                        wFormula_3_Len                    +
                        ODSLength( _VIEW_TABLE_FORMAT2 )  ;
    

    if (sError = OSMemAlloc( 0, wViewFormatBufLen, &hViewFormatBuffer ))
    {
        PRINTLOG("Error: unable to allocate %d bytes memory.\n", 
                 wViewFormatBufLen);
        goto Exit5;
    }

    pViewFormatBuffer = (char*) OSLockObject( hViewFormatBuffer );
    memset( pViewFormatBuffer, 0, (size_t) wViewFormatBufLen );

/* 
 * Initialize pVFBuf. pViewFormatBuffer will remain pointing to the top 
 * of the buffer. pVFBuf will move to point to the next available byte.
 */
    pVFBuf = pViewFormatBuffer;

/*
 * Initialize the VIEW_TABLE_FORMAT structure.
 */

    /* init to all zeros */
    memset (&ViewTableFormat, 0, sizeof(VIEW_TABLE_FORMAT));

    ViewTableFormat.Header.Version = VIEW_FORMAT_VERSION;
    ViewTableFormat.Header.ViewStyle = VIEW_STYLE_TABLE;
    ViewTableFormat.Columns = wNumColumns;
    ViewTableFormat.ItemSequenceNumber = 0;  /* Reserved - should be 0 */
    
    ViewTableFormat.Flags = VIEW_TABLE_FLAG_FLATINDEX |
                            VIEW_TABLE_FLAG_DISP_UNREADDOCS |
                            VIEW_TABLE_FLAG_CONFLICT;

    ViewTableFormat.Flags2 = 0;

/*
 * Call ODSWriteMemory to convert the VIEW_TABLE_FORMAT structure from
 * host-specific format to Domino and Notes canonical format, and copy it into the 
 * buffer at location pVFBuf. ODSWriteMemory increments pVFBuf to point
 * to the next byte in the buffer after the written data structure.
 */

    ODSWriteMemory( &pVFBuf, _VIEW_TABLE_FORMAT, &ViewTableFormat, 1 );
    
/*
 *  Create the VIEW_COLUMN_FORMAT item for column 1. Since this column
 *  is neither a TIME field or a NUMBER field, we don't need to set the
 *  TimeFormat or the NumberFormat fields.
 */

    ViewColumnFormat.Signature = VIEW_COLUMN_FORMAT_SIGNATURE;
    ViewColumnFormat.Flags1 = (VCF1_M_Sort | VCF1_M_SortCategorize);

    ViewColumnFormat.DisplayWidth = 8;
    ViewColumnFormat.FontID = DEFAULT_BOLD_FONT_ID;
    ViewColumnFormat.Flags2 = 0;

    ViewColumnFormat.FormatDataType = VIEW_COL_TEXT;
    ViewColumnFormat.ListSep = LDDELIM_COMMA;

    ViewColumnFormat.FormulaSize = wFormula_1_Len;
    ViewColumnFormat.ItemNameSize = wItemName_1_Len;
    ViewColumnFormat.TitleSize = 0;   /* No column title for categories */
    ViewColumnFormat.ConstantValueSize = 0; /* RESERVED _ SHOULD BE 0 */

/*
 * Convert the View Column Format structure for Col 1 to Domino and Notes canonical
 * format and append it to the buffer. This increments pVFBuf to point to
 * the next byte in the buffer after the View Column Format.
 */

    ODSWriteMemory( &pVFBuf, _VIEW_COLUMN_FORMAT, &ViewColumnFormat, 1 );

/*
 *  Now create the VIEW_COLUMN_FORMAT item for column 2. Since this column
 *  is not a TIME field, we don't need to set the TimeFormat field.
 *  Also, since this column will be a sort key, set the Flags1
 *  structure member appropriately.
 */

    memset( (char *)&ViewColumnFormat, 0, sizeof(VIEW_COLUMN_FORMAT) );

    ViewColumnFormat.Signature = VIEW_COLUMN_FORMAT_SIGNATURE;
    ViewColumnFormat.Flags1 = VCF1_M_Response; /* Set this flag to see indented response documents */
    ViewColumnFormat.DisplayWidth = 80;
    ViewColumnFormat.FontID = DEFAULT_FONT_ID;
    ViewColumnFormat.Flags2 = 0;

/*
 *  Set up Number format
 */    

    ViewColumnFormat.NumberFormat.Digits     = 0;
    ViewColumnFormat.NumberFormat.Format     = NFMT_GENERAL;
    ViewColumnFormat.NumberFormat.Attributes = 0;
    ViewColumnFormat.NumberFormat.Unused     = 0;

    ViewColumnFormat.FormatDataType = VIEW_COL_TEXT;
    ViewColumnFormat.ListSep = LDDELIM_COMMA;

/*
 * Now set up lengths of the various strings
 */

    ViewColumnFormat.FormulaSize = wFormula_2_Len;
    ViewColumnFormat.ItemNameSize = wItemName_2_Len;
    ViewColumnFormat.TitleSize = wTitle_2_Len;
    ViewColumnFormat.ConstantValueSize = 0; /* RESERVED _ SHOULD BE 0 */

/*
 * Call ODSWriteMemory to convert the View Column Format structure for
 * Col 2 to Domino and Notes canonical format and append it to the buffer. This
 * increments pVFBuf to point to the next byte after the View Column Format
 * structure.
 */

    ODSWriteMemory( &pVFBuf, _VIEW_COLUMN_FORMAT, &ViewColumnFormat, 1 );

/*
 *  Next, create the VIEW_COLUMN_FORMAT item for column 3. Since this column
 *  is neither a TIME field nor a NUMBER field, we don't need to set the
 *  TimeFormat or the NumberFormat fields.
 */
 
    memset( (char *)&ViewColumnFormat, 0, sizeof(VIEW_COLUMN_FORMAT) );

    ViewColumnFormat.Signature = VIEW_COLUMN_FORMAT_SIGNATURE;
    ViewColumnFormat.Flags1 = 0;
    ViewColumnFormat.DisplayWidth = 120;
    ViewColumnFormat.FontID = DEFAULT_FONT_ID;
    ViewColumnFormat.Flags2 = 0;
    ViewColumnFormat.FormatDataType = VIEW_COL_TEXT;
    ViewColumnFormat.ListSep = LDDELIM_COMMA;

/*
 * Now set up lengths of the various strings
 */

    ViewColumnFormat.FormulaSize = wFormula_3_Len;
    ViewColumnFormat.ItemNameSize = wItemName_3_Len;
    ViewColumnFormat.TitleSize = wTitle_3_Len;
    ViewColumnFormat.ConstantValueSize = 0; /* RESERVED _ SHOULD BE 0 */

/*
 * Convert the View Column Format structure for Col 3 to Domino and Notes canonical 
 * format and append it to the buffer. This increments pVFBuf to point 
 * to the next byte after the View Column Format structure.
 */

    ODSWriteMemory( &pVFBuf, _VIEW_COLUMN_FORMAT, &ViewColumnFormat, 1 );

/*
 * Append column 1's item name and formula to the buffer.
 */

    memcpy( pVFBuf, szItemName_1, wItemName_1_Len );
    pVFBuf += wItemName_1_Len;

    memcpy( pVFBuf, OSLockObject(hFormula_1), wFormula_1_Len );
    OSUnlockObject( hFormula_1 );
    pVFBuf += wFormula_1_Len;

/*
 * Append column 2's item name, title, and formula to the buffer.
 */

    memcpy( pVFBuf, szItemName_2, wItemName_2_Len );
    pVFBuf += wItemName_2_Len;

    memcpy( pVFBuf, szTitle_2, wTitle_2_Len );
    pVFBuf += wTitle_2_Len;

    memcpy( pVFBuf, OSLockObject(hFormula_2), wFormula_2_Len );
    OSUnlockObject( hFormula_2 );
    pVFBuf += wFormula_2_Len;

/*
 * Append column 3's item name, title, and formula to the buffer.
 */

    memcpy( pVFBuf, szItemName_3, wItemName_3_Len );
    pVFBuf += wItemName_3_Len;

    memcpy( pVFBuf, szTitle_3, wTitle_3_Len );
    pVFBuf += wTitle_3_Len ;

    memcpy( pVFBuf, OSLockObject(hFormula_3), wFormula_3_Len );
    OSUnlockObject( hFormula_3 );
    pVFBuf += wFormula_3_Len;

/*
 * Initialize the VIEW_TABLE_FORMAT2 structure.
 */

    /* init to all zeros */
    memset (&ViewTableFormat2, 0, sizeof(VIEW_TABLE_FORMAT2));

    ViewTableFormat2.Length = ODSLength(_VIEW_TABLE_FORMAT2);
    ViewTableFormat2.BackgroundColor = NOTES_COLOR_WHITE;
    ViewTableFormat2.V2BorderColor = 0;
    ViewTableFormat2.TitleFont = DEFAULT_BOLD_FONT_ID;
    ViewTableFormat2.UnreadFont = DEFAULT_FONT_ID;
    ViewTableFormat2.TotalsFont = DEFAULT_FONT_ID;
    ViewTableFormat2.AutoUpdateSeconds = 0;

        /* needed to initialize this value to the following for R5 */
    ViewTableFormat2.wSig = VALID_VIEW_FORMAT_SIG;

/*
 * Convert the View Table Format2 structure to Domino and Notes canonical format 
 * and append it to the buffer. This increments pVFBuf to point to the
 * next byte after the View Table Format2 structure.
 */

    ODSWriteMemory( &pVFBuf, _VIEW_TABLE_FORMAT2, &ViewTableFormat2, 1 );

/*
 * Now append the $VIEWFORMAT item to the note.
 */

    sError = NSFItemAppend( hNote,
                            ITEM_SUMMARY,
                            VIEW_VIEW_FORMAT_ITEM,
                            sizeof(VIEW_VIEW_FORMAT_ITEM) - 1,
                            TYPE_VIEW_FORMAT,
                            pViewFormatBuffer,
                            (DWORD)wViewFormatBufLen );

    OSUnlockObject( hViewFormatBuffer );
    OSMemFree( hViewFormatBuffer );

    if (sError) 
    {
        PRINTLOG("Error: unable to append item '%s' to view note.\n", 
                 VIEW_VIEW_FORMAT_ITEM );
        goto Exit5;
    }

/*
 * Create the View Collation Item. The View Collation Item is an item of 
 * TYPE_COLLATION with name VIEW_COLLATION_ITEM ($Collation). The View
 * Collation item consists of a COLLATION structure followed by one 
 * or more COLLATE_DESCRIPTOR structures, one for each sorted or categorized
 * column in the view, followed by the item name of each column to be collated. 
 * 
 * This view contains two collated columns. The first column in the view is
 * both sorted and categorized. The second column is sorted. The View Collation
 * Item therfore consists of the following series of structures converted to 
 * Domino and Notes canonical format and packed together:
 *
 *           COLLATION
 *           COLLATE_DESCRIPTOR (for column 1)
 *           COLLATE_DESCRIPTOR (for column 2)
 *           Item Name for column 1
 *           Item Name for column 2
 */

    wCollationBufLen =  ODSLength( _COLLATION )          +
                        ODSLength( _COLLATE_DESCRIPTOR ) +
                        ODSLength( _COLLATE_DESCRIPTOR ) +
                        wItemName_1_Len                  +
                        wItemName_2_Len                  ;


    if (sError = OSMemAlloc( 0, wCollationBufLen, &hCollationBuffer ))
    {
        PRINTLOG( "Error: unable to allocate %d bytes memory.\n", 
                  wCollationBufLen);
        goto Exit5;
    }

    pCollationBuffer = (char*) OSLockObject( hCollationBuffer );
    memset( pCollationBuffer, 0, (size_t) wCollationBufLen );

/* 
 * Initialize pCBuf. Keep pCollationBuffer pointing to the top 
 * of the buffer, and pCBuf pointing to the next available byte.
 */
    pCBuf = pCollationBuffer;

/*
 * Initialize the Collation structure.
 */
    Collation.Items = 2;                    /* View has 1 sort key, 1 category. */
    Collation.Flags = 0;
    Collation.signature = COLLATION_SIGNATURE;
    Collation.BufferSize = wCollationBufLen;
/*
 * Convert the Collation structure to Domino and Notes canonical format and store it
 * in the ODS buffer. This increments pCBuf to point to the next byte 
 * after the Collation structure in the buffer.
 */

    ODSWriteMemory( &pCBuf, _COLLATION, &Collation, 1 );
    
/*
 *  Initialize the collation descriptor for the first column. The
 *  first column is a category.
 */

    CollateDesc.Flags = 0;   
    CollateDesc.signature = COLLATE_DESCRIPTOR_SIGNATURE;
    CollateDesc.keytype = COLLATE_TYPE_CATEGORY;
    CollateDesc.NameOffset = 0;
    CollateDesc.NameLength = wItemName_1_Len;

/*
 * Convert the collation descriptor for the first column to Domino and Notes 
 * canonical format, and store it in the buffer. This increments
 * pCBuf to point to the next byte in the buffer after the collation
 * descriptor.
 */

    ODSWriteMemory( &pCBuf, _COLLATE_DESCRIPTOR, &CollateDesc, 1 );

/*
 * Initialize the collation descriptor for the second column. The
 * second column is sorted.
 */

    CollateDesc2.Flags = 0;
    CollateDesc2.signature = COLLATE_DESCRIPTOR_SIGNATURE;
    CollateDesc2.keytype = COLLATE_TYPE_KEY;
    CollateDesc2.NameOffset =  wItemName_1_Len;
    CollateDesc2.NameLength = wItemName_2_Len;

/*
 * Convert the collation descriptor for the second column to Domino and Notes 
 * canonical format, and store it in the buffer. This increments
 * pCBuf, as before.
 */

    ODSWriteMemory( &pCBuf, _COLLATE_DESCRIPTOR, &CollateDesc2, 1 );

/*
 *  Append the column 1 item name to the buffer.
 */
    
    memcpy( pCBuf, szItemName_1, wItemName_1_Len );
    pCBuf += wItemName_1_Len;

/*
 *  Append the column 2 item name to the buffer.
 */
    
    memcpy( pCBuf, szItemName_2, wItemName_2_Len );
    pCBuf += wItemName_2_Len;

/*
 *  Now append the $COLLATION item to the note.
 */
    
    sError = NSFItemAppend( hNote,
                            ITEM_SUMMARY,
                            VIEW_COLLATION_ITEM,
                            sizeof(VIEW_COLLATION_ITEM) - 1,
                            TYPE_COLLATION,
                            pCollationBuffer,
                            (DWORD) wCollationBufLen );

    OSUnlockObject( hCollationBuffer);
    OSMemFree( hCollationBuffer );

    if (sError)
    {
        PRINTLOG("Error: unable to append Collation item to view note.\n");
        goto Exit5;
    }
    
/*
 *  Done constructing the view note. Now store the view note
 *  in the database.
 */

    sError = NSFNoteUpdate( hNote, 0 );
                   
    if (sError)
    {
        PRINTLOG("Error: unable to update view note in database.\n" );
    }
    else
    {
        PRINTLOG("Successfully created view note in database.\n" );
        PRINTLOG("\nProgram completed successfully.\n" );
    }


Exit5:

    if (hSelFormula != NULLHANDLE)
    {
        OSMemFree( hSelFormula );
    }

    if (hFormula_1 != NULLHANDLE)
    {
        OSMemFree( hFormula_1 );
    }
    
    if (hFormula_2 != NULLHANDLE)
    {
        OSMemFree( hFormula_2 );
    }
    
    if (hFormula_3 != NULLHANDLE)
    {
        OSMemFree( hFormula_3 );
    }
        
Exit4:
    
    NSFNoteClose( hNote );

Exit3:    

    NSFDbClose( hDB );

Exit2:

    PRINTERROR (sError,"NSFDbOpen");
    NotesTerm();
    return (sError); 

}
