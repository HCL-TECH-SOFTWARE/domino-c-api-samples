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

    PROGRAM:    mktable

    FILE:       mktable.h

    PURPOSE:    Header file for table creation in a rich text field.


****************************************************************************/
 
#ifndef __MKTABLE_H
#define __MKTABLE_H

/* Function declarations */
BOOL    CDPutPara(BYTE **cd_out_ptr,
                  DWORD cd_input_cnt, DWORD *cd_output_cnt);

BOOL    CDPutPabdef(WORD para_num, BYTE **cd_out_ptr,
                    DWORD cd_input_cnt, DWORD *cd_output_cnt,
                    WORD Left_Margin, WORD Right_Margin, WORD Flags);

BOOL    CDPutPabref(WORD para_num, BYTE **cd_out_ptr,
                    DWORD cd_input_cnt, DWORD *cd_output_cnt);

BOOL    CDPutText(WORD font_choice, char *text_buf, WORD text_size,
                  BYTE **cd_out_ptr, DWORD cd_input_cnt,
                  DWORD *cd_output_cnt);

BOOL    CDPutTable(int table_num,
                   WORD first_last_record, WORD rows, WORD columns,
                   WORD table_width, BYTE borders,
                   WORD h_v_intercell_width,
                   WORD fit_flag, void *textlist_ptr, WORD font_flags,
                   CDPABDEFINITION *cur_pabdef_ptr,
                   BYTE **cd_buf_ptr, DWORD *cd_buf_cnt_ptr);

BOOL    CDPutBeginRecord(WORD version,
			          BYTE **cd_out_ptr,
			          DWORD cd_input_cnt,
			          DWORD *cd_output_cnt);

BOOL    CDPutEndRecord(WORD version,
			          BYTE **cd_out_ptr,
			          DWORD cd_input_cnt,
			          DWORD *cd_output_cnt);


BOOL    CDPutPreTableBegin(int table_num,
								WORD rows,
                        WORD columns,
                        BYTE **cd_out_ptr,
                        DWORD cd_input_cnt,
                        DWORD *cd_output_cnt);

BOOL    CDPutBorderInfo(int table_num,
								BYTE **cd_out_ptr,
                        DWORD cd_input_cnt,
                        DWORD *cd_output_cnt);


BOOL    CDPutTableBegin(WORD left_margin,
                        WORD horiz_intercell_width,
                        WORD vert_intercell_width,
                        FLAG flags,
                        BYTE **cd_out_ptr,
                        DWORD cd_input_cnt,
                        DWORD *cd_output_cnt);

BOOL    CDPutCell(BYTE row, BYTE column,  WORD cell_left_margin,
                  WORD cell_right_margin, WORD frac_width, 
		  BYTE borders, WORD borders_V42, BYTE **cd_out_ptr,
                  DWORD cd_input_cnt, DWORD *cd_output_cnt);

BOOL    CDPutTableEnd(BYTE **cd_out_ptr, DWORD cd_input_cnt,
                      DWORD *cd_output_cnt);

#define SETALLAROUND(btype)      ((btype << CDTC_S_Left)   \
                                 | (btype << CDTC_S_Right)  \
                                 | (btype << CDTC_S_Top)    \
                                 | (btype << CDTC_S_Bottom))


#define SETALLAROUNDV42(btype)  (  (btype << CDTC_S_V42_Left) & CDTC_M_V42_Left   \
                                 | (btype << CDTC_S_V42_Right) & CDTC_M_V42_Right \
                                 | (btype << CDTC_S_V42_Top) & CDTC_M_V42_Top     \
                                 | (btype << CDTC_S_V42_Bottom) & CDTC_M_V42_Bottom)


/* Document design constants */
#define BLDBLTR12  0x0000 /* header: 12 pt bold blue Times Roman    */
#define ITLBKHV10  0x0001 /* para bodies:  10 pt italic black Helv  */
#define PLNREDCR10 0x0002 /* table para bodies: 10pt plain Red Cour */

/* Memory and Table defines */

#define  MAXALLOC        61440    /* Largest mem obj: 60 Kbytes */
#define  CDBUFSIZE  MAXALLOC
#define  MAXROWS         255
#define  MAXCOLUMNS      64
#define  MAXRIGHT_MARGIN (22.75*ONEINCH) /* max Rt. margin in twips */
#define  MININTER_CELL   (.0625*ONEINCH) /* min inter cell dist.    */

/* First para text */
#define REQS "This document was written by MKTABLE.C."

/* Local CleanUp defines */
#define DO_NOTHING          0x0000
#define CLOSE_DB            0x0001
#define UNLOCK_BUFFER       0x0002
#define FREE_BUFFER         0x0004
#define CLOSE_NOTE          0x0008
#define UNLOCK_LIST         0x0010
#define FREE_LIST           0x0011

/* Other local defines */
#define LINEOTEXT           255
#define MIDDLE_REC          0x0000
#define FIRST_REC           0x0001
#define LAST_REC            0x0002

static  WORD    cur_pab_id;    /*  Used by all CDPutxxxxx() funcs */

#endif

