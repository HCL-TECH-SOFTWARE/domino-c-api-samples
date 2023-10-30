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

    PROGRAM:    mktable

    FILE:       cdtable.c

    PURPOSE:    Creates tables in a rich text field.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* C includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* HCL C API for Notes/Domino includes */

#include <global.h>
#include <globerr.h>
#include <nsf.h>
#include <nsferr.h>
#include <osmisc.h>
#include <oserr.h>
#include <osmem.h>
#include <pool.h>
#include <names.h>
#include <colorid.h>
#include <editods.h>
#include <ods.h>
#include <odserr.h>
#include <editdflt.h>
#include <osfile.h>
#include <textlist.h>
#include <stdnames.h>
#include <colorods.h>

#include "mktable.h"

/************************************************************************

    FUNCTION:   CDPutTable

    PURPOSE:    Create a complete table based on #rows, #columns, table
		width, textlist of cell text entries, etc., and write it
		to a buffer.

    ALGORITHM:
	Calls CDPutTableBegin, CDPutCell (rows * columns times), and
	finally CDPutTableEnd.  Empty leading and trailing paragraphs
	are added as required.

	This function also make use of a variable called "cur_pab_id",
	which is incremented whenever a new PABID needs to be created
	and used whenever a PABID needs to be assigned.

    INPUTS:
	 
	table_num - table number
	
	first_last_record -
		MIDDLE_REC Don't prefix or suffix w/ empty paragraph.
		FIRST_REC  Prefix w/ empty para.
		LAST_REC             Suffix w/ empty para.
		FIRST_REC | LAST_REC Table alone in Rich Text.

	rows -  Absolute limit of 255.  Overhead of all records assoc.
		with a table of rows x columns must fit into your
		MAXWORD - (4 * sizeof(WORD)) byte CD buffer.

	columns - Absolute limit calculated by fitting as many columns
		as possible into a paragraph whose maximum right margin
		is 22.75".

	table_width - Absolute limit of 22.75" minus the starting left
		margin.

	borders -  Border style to be used on all sides of the table.
		TABLE_BORDER_NONE
		TABLE_BORDER_SINGLE
		TABLE_BORDER_DOUBLE

	h_v_intercell_width - Horizontal and Vertical dist. from CELL
		border to edge of text. Minimum .0625" (1/16th").

	fit_flags - Determines whether or not table is to be automati-
		cally resized with the MDI Child window.
		CDTABLE_AUTO_CELL_WIDTH - Fit to Window table.

	textlist_ptr - A pointer to a valid TYPE_TEXT_LIST data struct
		or NULL if an empty table is desired.

	font_flags - Three styles have been defined for use with
		CDPutText().  Feel free to add your own!

	cur_pabdef_ptr - Pointer to current paragraph definition in use
		before the start of the table's paragraph.

	cd_buf_ptr - Pointer to the CD buffer.  A Domino and Notes dynamic memory
		object w/ maximum size of MAXWORD - (4 * sizeof(WORD)).

	cd_buf_cnt_ptr - Running length of CD buffer contents.

    RETURNS:
	TRUE - If able to successfully add CD records.
	FALSE- If ran out of space in CD buffer.

*************************************************************************/


BOOL    CDPutTable(int table_num, WORD first_last_record, 
                   WORD rows, WORD columns,
                   WORD table_width, BYTE borders,
                   WORD h_v_intercell_width,
                   WORD fit_flag, void *textlist_ptr, WORD font_flags,
                   CDPABDEFINITION *cur_pabdef_ptr,
                   BYTE **cd_buf_ptr, DWORD *cd_buf_cnt_ptr)
{
    STATUS  error = NOERROR;
    WORD    horiz_intercell_width, vert_intercell_width;
    WORD    left_margin, cell_text_width;
    WORD    cell_left_margin, cell_right_margin, next_cell_start;
    DWORD   frac_width, cell_width;
    BYTE    all_borders;
    WORD    all_borders_V42;
    WORD    list_entries, num_cells, row1_cell_size, rowx_cell_size;
    WORD    list_entry, avg_celltext_len, avg_celltext_chars;
    WORD    table_size, prefix_para_size, suffix_para_size;
    char    *temp_ptr;
    char    temp_buf[LINEOTEXT];
    WORD    buf_len, countr, countc, cur_left_margin;
    WORD    calc_columns;
    char    eString[] = "";

    CDPABDEFINITION *base_pabdef_ptr;


    /* Keep a pointer to the preceding paragraph definition. */

    base_pabdef_ptr = cur_pabdef_ptr;

    /* Get the number of cells for later calcs */

    num_cells = rows * columns;

    /* No such thing as a table without cells or a table with more */
    /* list entries than cells.  Table w/o list entries is OK.     */

    if (num_cells < 1)
	return (FALSE);

    /* Check right margin, number of rows, and horz/vert intercell */
    if ( (DWORD)table_width > (DWORD)(MAXRIGHT_MARGIN
				      - base_pabdef_ptr->LeftMargin)
	|| (rows > MAXROWS)
	|| ((DWORD) h_v_intercell_width < (DWORD) MININTER_CELL)
	|| (columns > MAXCOLUMNS) )
	return (FALSE);

    /* Get the average numbers of chars in a cell */
    if (textlist_ptr != NULL)
    {
	div_t   div_result;
	WORD    count, total_celltext_len = 0;

	list_entries  = ListGetNumEntries(textlist_ptr, FALSE);

	/* No such thing as a table with more */
	/* list entries than cells.           */
	if (list_entries > num_cells)
	    return (FALSE);

	for (count = 0; count < list_entries; count++)
	{
	    if (error = ListGetText(textlist_ptr, FALSE, count,
				    &temp_ptr, &buf_len))
		return (FALSE);
	    total_celltext_len += buf_len;
	}
	div_result = div(total_celltext_len, list_entries);
	if (div_result.rem)
	    avg_celltext_chars = div_result.quot + 1;
	else
	    avg_celltext_chars = div_result.quot;
    }
    else
    {
	list_entries  = 0;   /* If no text list provided, put empty */
			     /*  CDTEXT records in each cell.       */
	avg_celltext_chars = 1; /* Assume 1 char, as a table with   */
				/* max columns and zero length text */
				/* per column will not be able to   */
				/* accept ANY text!                 */
    }

    /* Calc avg_celltext_len in twips */
    switch(font_flags)
    {
	case BLDBLTR12:                       /* 72 points per inch */
	    avg_celltext_len= (WORD)((DWORD)
				     (avg_celltext_chars * 12 * ONEINCH)
				     / (DWORD) 72);
	    break;

	case ITLBKHV10:
	    avg_celltext_len= (WORD)((DWORD)
				     (avg_celltext_chars * 10 * ONEINCH)
				     / (DWORD) 72);
	    break;

	case PLNREDCR10:
	    avg_celltext_len= (WORD)((DWORD)
				     (avg_celltext_chars * 10 * ONEINCH)
				     / (DWORD) 72);
	    break;

	default:
	    return(FALSE);
    }

    /* Check if table with UNWRAPPED text will fit between left and */
    /* maximum right margin.                                        */
    calc_columns = (WORD) ((DWORD) (MAXRIGHT_MARGIN
				   - base_pabdef_ptr->LeftMargin)
			  / (DWORD) ((2 * h_v_intercell_width)
				      + avg_celltext_len));
    if (columns > calc_columns)
	return (FALSE);


    /* Will the requested table fit in the remaining CD buffer? */
    row1_cell_size = ODSLength( _CDPARAGRAPH )
                   + ODSLength( _CDPABDEFINITION )
                   + ODSLength( _CDPABREFERENCE )
                   + ODSLength( _CDTEXT )
                   + ODSLength( _CDTABLECELL )
                   + avg_celltext_chars;

    rowx_cell_size = ODSLength( _CDPARAGRAPH )
                   + ODSLength( _CDPABREFERENCE )
                   + ODSLength( _CDTEXT )
                   + ODSLength( _CDTABLECELL )
                   + avg_celltext_chars;

    table_size = ODSLength( _CDPARAGRAPH ) + ODSLength( _CDPABREFERENCE )
               + ODSLength( _CDBEGINRECORD )
               + ODSLength( _CDPRETABLEBEGIN )
               + ODSLength( _CDBORDERINFO)
               + ODSLength( _CDENDRECORD )
               + ODSLength( _CDTABLEBEGIN ) 
               + (columns * row1_cell_size)
               + ((num_cells - columns) * rowx_cell_size)
               + ODSLength( _CDTABLEEND );

    prefix_para_size = ODSLength( _CDPABDEFINITION ) + ODSLength( _CDPABREFERENCE )
                     + ODSLength( _CDTEXT );

    suffix_para_size = ODSLength( _CDPARAGRAPH ) + ODSLength( _CDPABREFERENCE )
                     + ODSLength( _CDBEGINRECORD )
                     + ODSLength( _CDTEXT )
                     + ODSLength( _CDENDRECORD );

    /* Determine if num_cells still fits into your CD buffer */
    if (*cd_buf_cnt_ptr == 0 )
    {
	if((prefix_para_size + table_size + suffix_para_size)
	    > MAXALLOC)
	    return (FALSE);
    }
    else
    {
	if((*cd_buf_cnt_ptr + table_size + suffix_para_size)
	    > MAXALLOC)
	    return (FALSE);
    }

    /* RULE 1. TABLE paragraph cannot be the first paragraph in */
    /*         a TYPE_COMPOSITE item.  An empty one will do...  */
    if (first_last_record & FIRST_REC)
    {
       if(!CDPutPara(cd_buf_ptr, *cd_buf_cnt_ptr, cd_buf_cnt_ptr))
          return (FALSE);

       if(!CDPutPabref(base_pabdef_ptr->PABID, cd_buf_ptr,
                       *cd_buf_cnt_ptr, cd_buf_cnt_ptr))
          return (FALSE);

       if(!CDPutBeginRecord( 0, cd_buf_ptr, *cd_buf_cnt_ptr, cd_buf_cnt_ptr))
          return (FALSE);
	
	/* CDTEXT record containing a 0 Length string */
       if(!CDPutText(ITLBKHV10, eString, 0, cd_buf_ptr, *cd_buf_cnt_ptr, cd_buf_cnt_ptr))
          return (FALSE);

       if(!CDPutEndRecord( 0, cd_buf_ptr, *cd_buf_cnt_ptr, cd_buf_cnt_ptr))
          return (FALSE);
    }


    /* Start table */
    /* RULE 2. TABLE must occupy its own paragraph. */
    if(!CDPutPara(cd_buf_ptr, *cd_buf_cnt_ptr,
		  cd_buf_cnt_ptr))
	return (FALSE);
    if(!CDPutPabref(base_pabdef_ptr->PABID, cd_buf_ptr, *cd_buf_cnt_ptr,
		    cd_buf_cnt_ptr))
	return (FALSE);


    /* Obtain LeftMargin from current PABDEFINITION */
    cell_width            = table_width / columns;
    left_margin           = base_pabdef_ptr->LeftMargin;
    horiz_intercell_width = vert_intercell_width = h_v_intercell_width;
    cell_text_width       = (WORD) cell_width
			     - (horiz_intercell_width * 2);
    all_borders           = (BYTE) SETALLAROUND((WORD) borders);
    all_borders_V42       = (WORD) SETALLAROUNDV42 (borders);

    if(!CDPutBeginRecord( 1, cd_buf_ptr, *cd_buf_cnt_ptr, cd_buf_cnt_ptr))
	      return (FALSE);

    if(!CDPutPreTableBegin(table_num, rows, columns, cd_buf_ptr, 
                           *cd_buf_cnt_ptr, cd_buf_cnt_ptr))
	   return (FALSE);

    if(!CDPutBorderInfo(table_num, cd_buf_ptr, *cd_buf_cnt_ptr, cd_buf_cnt_ptr))
	   return (FALSE);
    
    if(!CDPutEndRecord( 1, cd_buf_ptr, *cd_buf_cnt_ptr, cd_buf_cnt_ptr))
	   return (FALSE);

    if(!CDPutTableBegin(left_margin, horiz_intercell_width,
                        vert_intercell_width, fit_flag, cd_buf_ptr,
                        *cd_buf_cnt_ptr, cd_buf_cnt_ptr))
	   return (FALSE);

    /* Determine table width and fractional width for size to fit tables */
    if(fit_flag & CDTABLE_AUTO_CELL_WIDTH)
	frac_width = ONEINCH * 20 * cell_width / (DWORD) table_width;
    else
	frac_width = 0L;

    /* Place the correct number of cells and their text contents */
    for (countr = 0, list_entry = 0; countr < rows; countr++)
    {
	WORD   table_para_nums[LINEOTEXT];

	cur_left_margin = left_margin;

	for (countc = 0, buf_len = 0; countc < columns; countc++)
	{
	    /* Calc absolute cell dimensions */
	    cell_left_margin = cur_left_margin + horiz_intercell_width;
	    cell_right_margin = cell_left_margin + cell_text_width;
	    next_cell_start = cell_right_margin + horiz_intercell_width;

	    if(!CDPutCell((BYTE)countr, (BYTE)countc, cell_left_margin,
		      cell_right_margin, (WORD) frac_width,
		      all_borders, all_borders_V42, cd_buf_ptr,
		      *cd_buf_cnt_ptr, cd_buf_cnt_ptr))
		return (FALSE);

	    /* Create a new paragraph definition if req'd this cell */
	    if (countr == 0)
	    {
		cur_pab_id++;
		table_para_nums[countc] = cur_pab_id;
		if(!CDPutPara( cd_buf_ptr,
			      *cd_buf_cnt_ptr,
			      cd_buf_cnt_ptr))
		    return (FALSE);

		cur_pabdef_ptr = (CDPABDEFINITION *) *cd_buf_ptr;
		if(!CDPutPabdef(cur_pab_id, cd_buf_ptr,
				*cd_buf_cnt_ptr, cd_buf_cnt_ptr, 
				cell_left_margin, cell_right_margin, TABLE_PABFLAGS))
		    return (FALSE);

		if(!CDPutPabref(cur_pab_id, cd_buf_ptr,
				*cd_buf_cnt_ptr, cd_buf_cnt_ptr))
		    return (FALSE);
	    }
	    else
	    {
		if(!CDPutPara( cd_buf_ptr,
			      *cd_buf_cnt_ptr,
			      cd_buf_cnt_ptr))
		    return (FALSE);
		if(!CDPutPabref(table_para_nums[countc], cd_buf_ptr,
				*cd_buf_cnt_ptr, cd_buf_cnt_ptr))
		    return (FALSE);
	    }
	    /* Get the cell text */
	    buf_len = 0;
	    if (list_entries > 0)
	    {
		error = ListGetText(textlist_ptr, FALSE, list_entry++,
				    &temp_ptr, &buf_len);
		if (error == ERR_ODS_NO_SUCH_ENTRY)
		    temp_buf[0] = '\0';
		else if (error == NOERROR)
		    memcpy(temp_buf, temp_ptr, buf_len);
		else
		    return (FALSE);
	    }
	    temp_buf[buf_len] = '\0';

       if(!CDPutBeginRecord( 0, cd_buf_ptr, *cd_buf_cnt_ptr, cd_buf_cnt_ptr))
         return (FALSE);

		 /* Place CDTEXT record */
	    if(!CDPutText(font_flags, temp_buf, (WORD) strlen(temp_buf),
			            cd_buf_ptr, *cd_buf_cnt_ptr, cd_buf_cnt_ptr))
          return (FALSE);

       if(!CDPutEndRecord( 0, cd_buf_ptr, *cd_buf_cnt_ptr, cd_buf_cnt_ptr))
	      return (FALSE);
       
	    /* Start at left side of next cell in row */
	    cur_left_margin = next_cell_start;
	}
    }

    if (!CDPutTableEnd(cd_buf_ptr, *cd_buf_cnt_ptr, cd_buf_cnt_ptr))
	return (FALSE);

    /* RULE 3. There must be at least one paragraph following a */
    /*         table in a Rich Text item/field.                 */
    if (first_last_record & LAST_REC)
    {
       if(!CDPutPara( cd_buf_ptr, *cd_buf_cnt_ptr, cd_buf_cnt_ptr))
         return (FALSE);

       if(!CDPutPabref(base_pabdef_ptr->PABID, cd_buf_ptr,
                       *cd_buf_cnt_ptr, cd_buf_cnt_ptr))
         return (FALSE);

       if(!CDPutBeginRecord( 0, cd_buf_ptr, *cd_buf_cnt_ptr, cd_buf_cnt_ptr))
         return (FALSE);

       if(!CDPutText(ITLBKHV10, eString, 0, cd_buf_ptr, *cd_buf_cnt_ptr, cd_buf_cnt_ptr))
         return (FALSE);

       if(!CDPutEndRecord( 0, cd_buf_ptr, *cd_buf_cnt_ptr, cd_buf_cnt_ptr))
         return (FALSE);
    }
    return TRUE;
}

/************************************************************************

    FUNCTION:   CDPutBeginRecord

    PURPOSE:    Create a CDBEGINRECORD record in the supplied buffer.

    INPUTS:
      version       - version of this element.
		
      cd_out_ptr    - pointer to CD buffer under construction.

      cd_input_cnt  - current offset into CD buffer.

      cd_output_cnt - offset into CD buffer after CDBEGINRECORD
		      was added.

    RETURNS:
	Returns TRUE if successful, FALSE if not enough memory in buffer.

*************************************************************************/

BOOL    CDPutBeginRecord(WORD version,
			BYTE **cd_out_ptr,
			DWORD cd_input_cnt,
			DWORD *cd_output_cnt)
{
    CDBEGINRECORD    cd_begrec;

    if ((cd_input_cnt + ODSLength( _CDBEGINRECORD )) > CDBUFSIZE)
       return (FALSE);

    cd_begrec.Header.Signature = SIG_CD_BEGIN;
    cd_begrec.Header.Length = (BYTE)ODSLength( _CDBEGINRECORD );

    cd_begrec.Version = version;
    if (version)
		 cd_begrec.Signature = SIG_CD_PRETABLEBEGIN;
    else
	 cd_begrec.Signature = SIG_CD_TEXT;

    ODSWriteMemory( cd_out_ptr, _CDBEGINRECORD, &cd_begrec, 1 );

    *cd_output_cnt = cd_input_cnt + ODSLength( _CDBEGINRECORD );

    return (TRUE);
}

/************************************************************************

    FUNCTION:   CDPutEndRecord

    PURPOSE:    Create a CDENDRECORD record in the supplied buffer.

    INPUTS:
      version       - version of this element.
		
      cd_out_ptr    - pointer to CD buffer under construction.

      cd_input_cnt  - current offset into CD buffer.

      cd_output_cnt - offset into CD buffer after CDENDRECORD
		      was added.

    RETURNS:
	Returns TRUE if successful, FALSE if not enough memory in buffer.

*************************************************************************/

BOOL    CDPutEndRecord(WORD version,
			BYTE **cd_out_ptr,
			DWORD cd_input_cnt,
			DWORD *cd_output_cnt)
{
    CDENDRECORD    cd_endrec;

    if ((cd_input_cnt + ODSLength( _CDENDRECORD )) > CDBUFSIZE)
       return (FALSE);

    cd_endrec.Header.Signature = SIG_CD_END;
    cd_endrec.Header.Length = (BYTE)ODSLength( _CDENDRECORD );

    cd_endrec.Version = version;
    if (version)
		 cd_endrec.Signature = SIG_CD_PRETABLEBEGIN;
    else
	 cd_endrec.Signature = SIG_CD_TEXT;

    ODSWriteMemory( cd_out_ptr, _CDBEGINRECORD, &cd_endrec, 1 );

    *cd_output_cnt = cd_input_cnt + ODSLength( _CDENDRECORD );

    return (TRUE);
}

/************************************************************************

    FUNCTION:   CDPutPreTableBegin

    PURPOSE:    Create a CDPRETABLEBEGIN record in the supplied buffer.

    INPUTS:
      table_num     - table number
		
      rows          - number of rows.
		
      columns       - number of coulmns.

      cd_out_ptr    - pointer to CD buffer under construction.

      cd_input_cnt  - current offset into CD buffer.

      cd_output_cnt - offset into CD buffer after CDPRETABLEBEGIN
		      was added.

    RETURNS:
	Returns TRUE if successful, FALSE if not enough memory in buffer.

*************************************************************************/

BOOL    CDPutPreTableBegin(int table_num,
			WORD rows,
			WORD columns,
			BYTE **cd_out_ptr,
			DWORD cd_input_cnt,
			DWORD *cd_output_cnt)
{
	 CDPRETABLEBEGIN    cd_prebegtbl;
	 COLOR_VALUE ColorValue;

	 ColorValue.Flags = COLOR_VALUE_FLAGS_ISRGB;
	 if (table_num == 1)
	 {
		 ColorValue.Component1 = 0;
		 ColorValue.Component2 = 0;
		 ColorValue.Component3 = 0;
		 ColorValue.Component4 = 0;
	 }
	 if (table_num == 2)
	 {
		 ColorValue.Component1 = 255;
		 ColorValue.Component2 = 0;
		 ColorValue.Component3 = 0;
		 ColorValue.Component4 = 0;
	 }
	 if (table_num == 2)
	 {
		 ColorValue.Component1 = 0;
		 ColorValue.Component2 = 0;
		 ColorValue.Component3 = 255;
		 ColorValue.Component4 = 0;
	 }
    

    if ((cd_input_cnt + ODSLength( _CDPRETABLEBEGIN )) > CDBUFSIZE)
       return (FALSE);

    cd_prebegtbl.Header.Signature = SIG_CD_PRETABLEBEGIN;
    cd_prebegtbl.Header.Length = ODSLength( _CDPRETABLEBEGIN );

    if (table_num == 1)
		cd_prebegtbl.Flags = CDPRETABLE_AUTO_CELL_WIDTH|CDPRETABLE_USEBORDERCOLOR|CDPRETABLE_DROPSHADOW;
    else
		cd_prebegtbl.Flags = CDPRETABLE_AUTO_CELL_WIDTH|CDPRETABLE_USEBORDERCOLOR;

    cd_prebegtbl.Rows = (BYTE)rows;
    cd_prebegtbl.Columns = (BYTE)columns;
    cd_prebegtbl.ColumnSizingBits1 = 0x00000000;
    cd_prebegtbl.ColumnSizingBits2 = 0x00000000;
    cd_prebegtbl.ViewerType = 0;
    cd_prebegtbl.Spare = 0;
    cd_prebegtbl.MinRowHeight = 0;
    cd_prebegtbl.Spares[1] = 0;

    if (table_num == 1)
    {
       cd_prebegtbl.StyleColor1 = 0x00000000;
       cd_prebegtbl.StyleColor2 = 0x00000000;
    }
    if (table_num == 2)
    {
       cd_prebegtbl.StyleColor1 = 0x0000FF00;
       cd_prebegtbl.StyleColor2 = 0x0000FFFF;
    }
    if (table_num ==3)
    {
       cd_prebegtbl.StyleColor1 = 0x00FF0000;
       cd_prebegtbl.StyleColor2 = 0x00FF00FF;
    }

    cd_prebegtbl.InnerBorderColor = ColorValue;
    cd_prebegtbl.NameLength = 0;
    cd_prebegtbl.ImagePacketLength = 0;
    cd_prebegtbl.RowLabelDataLength = 0;

    ODSWriteMemory( cd_out_ptr, _CDPRETABLEBEGIN, &cd_prebegtbl, 1 );

    *cd_output_cnt = cd_input_cnt + ODSLength( _CDPRETABLEBEGIN );

    return (TRUE);
}

/************************************************************************

    FUNCTION:   CDPutBorderInfo

    PURPOSE:    Create a CDBORDERINFO record in the supplied buffer.

    INPUTS:

      table_num     - table number
		
      cd_out_ptr    - pointer to CD buffer under construction.

      cd_input_cnt  - current offset into CD buffer.

      cd_output_cnt - offset into CD buffer after CDBORDERINFO
		      was added.

    RETURNS:
	Returns TRUE if successful, FALSE if not enough memory in buffer.

*************************************************************************/

BOOL    CDPutBorderInfo(int table_num,
			BYTE **cd_out_ptr,
			DWORD cd_input_cnt,
			DWORD *cd_output_cnt)
{
	 CDBORDERINFO    cd_binfo;
	 COLOR_VALUE ColorValue;
	 
	 memset(&cd_binfo, 0, sizeof(CDBORDERINFO));

	 ColorValue.Flags = COLOR_VALUE_FLAGS_ISRGB;
	 if (table_num == 1)
	 {
		ColorValue.Component1 = 0;
		ColorValue.Component2 = 0;
		ColorValue.Component3 = 0;
		ColorValue.Component4 = 0;
	 }
	 if (table_num == 2)
	 {
		ColorValue.Component1 = 0;
		ColorValue.Component2 = 0;
		ColorValue.Component3 = 255;
		ColorValue.Component4 = 0;
	 }
	 if (table_num == 3)
	 {
		ColorValue.Component1 = 255;
		ColorValue.Component2 = 0;
		ColorValue.Component3 = 0;
		ColorValue.Component4 = 0;
	 }

    if ((cd_input_cnt + ODSLength( _CDBORDERINFO )) > CDBUFSIZE)
       return (FALSE);

    cd_binfo.Header.Signature = SIG_CD_BORDERINFO;
    cd_binfo.Header.Length = ODSLength( _CDBORDERINFO );

    cd_binfo.Flags = 0;

    if (table_num == 1)
    {
       cd_binfo.BorderStyle = CDBORDERSTYLE_NONE;
       cd_binfo.BorderWidthTop = 2;
       cd_binfo.BorderWidthLeft = 2;
       cd_binfo.BorderWidthBottom = 2;
       cd_binfo.BorderWidthRight = 2;
    }
    if (table_num == 2)
    {
       cd_binfo.BorderStyle = CDBORDERSTYLE_DASHED;
       cd_binfo.BorderWidthTop = 2;
       cd_binfo.BorderWidthLeft = 2;
       cd_binfo.BorderWidthBottom = 2;
       cd_binfo.BorderWidthRight = 2;
    }
    if (table_num == 3)
    {
       cd_binfo.BorderStyle = CDBORDERSTYLE_OUTSET;
       cd_binfo.BorderWidthTop = 5;
       cd_binfo.BorderWidthLeft = 5;
       cd_binfo.BorderWidthBottom = 5;
       cd_binfo.BorderWidthRight = 5;
    }

    cd_binfo.dwSpare = 0;

    if (table_num == 1)
    {
       cd_binfo.BorderFlags = CDBORDER_FLAGS_DROP_SHADOW;
       cd_binfo.DropShadowWidth = 12;
    }
    else
    {
       cd_binfo.BorderFlags = 0;
       cd_binfo.DropShadowWidth = 0;
    }
    
    cd_binfo.InnerWidthTop = 2;
    cd_binfo.InnerWidthLeft = 2;
    cd_binfo.InnerWidthBottom = 2;
    cd_binfo.InnerWidthRight = 2;
    cd_binfo.OuterWidthTop = 2;
    cd_binfo.OuterWidthLeft = 2;
    cd_binfo.OuterWidthBottom = 2;
    cd_binfo.OuterWidthRight = 2;
    cd_binfo.Color = ColorValue;

    ODSWriteMemory( cd_out_ptr, _CDBORDERINFO, &cd_binfo, 1 );

    *cd_output_cnt = cd_input_cnt + ODSLength( _CDBORDERINFO );

    return (TRUE);
}


/************************************************************************

    FUNCTION:   CDPutTableBegin

    PURPOSE:    Create a CDTABLEBEGIN record in the supplied buffer.

    INPUTS:
      left_margin - left margin of table's para in twips.

      horiz_intercell_width - dist from cell's border to cell
			      para's left or right margin.

      vert_intercell_width - dist from cell's border to top or
			     bottom of text in cell's para.

      flags         - Determines whether or not table is to be automati-
	              cally resized with the MDI Child window.
	              CDTABLE_AUTO_CELL_WIDTH - Fit to Window table.

      cd_out_ptr    - pointer to CD buffer under construction.

      cd_input_cnt  - current offset into CD buffer.

      cd_output_cnt - offset into CD buffer after CDTABLEBEGIN
		      was added.

    RETURNS:
	Returns TRUE if successful, FALSE if not enough memory in buffer.

*************************************************************************/

BOOL    CDPutTableBegin(WORD left_margin,
			WORD horiz_intercell_width,
			WORD vert_intercell_width,
			FLAG flags,
			BYTE **cd_out_ptr,
			DWORD cd_input_cnt,
			DWORD *cd_output_cnt)
{
    CDTABLEBEGIN    cd_begtbl;

    if ((cd_input_cnt + ODSLength( _CDTABLEBEGIN )) > CDBUFSIZE)
       return (FALSE);

    cd_begtbl.Header.Signature = SIG_CD_TABLEBEGIN;
    cd_begtbl.Header.Length = (BYTE) ODSLength( _CDTABLEBEGIN );

    cd_begtbl.LeftMargin = left_margin;
    cd_begtbl.HorizInterCellSpace = horiz_intercell_width;
    cd_begtbl.VertInterCellSpace = vert_intercell_width;

    cd_begtbl.V4HorizInterCellSpace = horiz_intercell_width;
    cd_begtbl.V4VertInterCellSpace  = vert_intercell_width;
    cd_begtbl.Flags = flags;

    ODSWriteMemory( cd_out_ptr, _CDTABLEBEGIN, &cd_begtbl, 1 );

    *cd_output_cnt = cd_input_cnt + ODSLength( _CDTABLEBEGIN );

    return (TRUE);
}

/************************************************************************

    FUNCTION:   CDPutCell

    PURPOSE:    Create a CDTABLECELL  record in the supplied buffer.

    INPUTS:
      row            -  row number of cell being added to the table.

      column         -  column number of cell being added to the table.

      cell_left_margin - cell left border+horiz intercell width.

      cell_right_margin - cell right border-horiz intercell width.

      frac_width     -  0 for fixed width table.
			ONEINCH*20*cell_width/table_width for fit to
			window table.

      borders        -  border style to be used on all sides of table.

      borders_V42    -  (for Notes R4.5) border style to be used on 
			all sides of table.

      cd_out_ptr     -  pointer to CD buffer under construction.

      cd_input_cnt   -  current offset into CD buffer.

      cd_output_cnt  -  offset into CD buffer after CDTABLEBEGIN
			was added.

    RETURNS:
	Returns TRUE if successful, FALSE if not enough memory in buffer.

*************************************************************************/

BOOL    CDPutCell(BYTE row, BYTE column,  WORD cell_left_margin,
		  WORD cell_right_margin, WORD frac_width,
		  BYTE borders, WORD borders_V42, BYTE **cd_out_ptr,
		  DWORD cd_input_cnt, DWORD *cd_output_cnt)
{
    CDTABLECELL    cd_cell;

    if ((cd_input_cnt + ODSLength( _CDTABLECELL )) > CDBUFSIZE)
       return (FALSE);

    cd_cell.Header.Signature = SIG_CD_TABLECELL;
    cd_cell.Header.Length = (BYTE) ODSLength( _CDTABLECELL );

    cd_cell.Row = row;
    cd_cell.Column = column;
    cd_cell.LeftMargin = cell_left_margin;
    cd_cell.RightMargin = cell_right_margin;
    cd_cell.FractionalWidth = frac_width;

    /* We still have to specify this member, so that the table
       can be read by earlier releases of Domino and Notes */
    cd_cell.Border = borders;

    /* Specify Notes R4.5 borders here: 
       See SETALLAROUNDV42 in mktable.h */
    cd_cell.v42Border = 0;
    cd_cell.v42Border = borders_V42;

    /* Specify that the table is created with Notes R4.5 */
    cd_cell.Flags = CDTABLECELL_USE_V42BORDERS;

    /* These structure members are for use when merging contiguous cells.
       Set to 1 as initial value. */
    cd_cell.RowSpan = 1;
    cd_cell.ColumnSpan = 1;

    ODSWriteMemory( cd_out_ptr, _CDTABLECELL, &cd_cell, 1 );

    *cd_output_cnt = cd_input_cnt + ODSLength( _CDTABLECELL );

    return (TRUE);
}


/************************************************************************

    FUNCTION:  CDPutTableEnd

    PURPOSE:   Create a CDTABLEEND   record in the supplied buffer.

    INPUTS:
      cd_out_ptr    - pointer to CD buffer under construction.

      cd_input_cnt  - current offset into CD buffer.

      cd_output_cnt - offset into CD buffer after CDTABLEBEGIN
		      was added.

    RETURNS:
      Returns TRUE if successful, FALSE if not enough memory in buffer.

*************************************************************************/

BOOL    CDPutTableEnd(BYTE **cd_out_ptr, DWORD cd_input_cnt,
		      DWORD *cd_output_cnt)
{
    CDTABLEEND    cd_endtbl;

    if ((cd_input_cnt + ODSLength( _CDTABLEEND )) > CDBUFSIZE)
       return (FALSE);

    cd_endtbl.Header.Signature = SIG_CD_TABLEEND;
    cd_endtbl.Header.Length = LOBYTE(ODSLength( _CDTABLEEND ));
    cd_endtbl.Spare = 0;
     
    ODSWriteMemory( cd_out_ptr, _CDTABLEEND, &cd_endtbl, 1 );

    *cd_output_cnt = cd_input_cnt + ODSLength( _CDTABLEEND );

    return (TRUE);
}
