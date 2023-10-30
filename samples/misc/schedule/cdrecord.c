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

    FILE:       cdrecord.c

    PURPOSE:    Functions for appending CD records to a buffer

****************************************************************************/

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


/************************************************************************

    FUNCTION:  CDPutPabdef

    PURPOSE:   Create a paragraph definition paragraph attribute block
               (PAB) and write it to a buffer.  Uses defaults from
               editdflt.h.

    ARGUMENTS:
      WORD para_num        - A unique ID to use for this PabDefinition.

      BYTE **cd_out_ptr    - pointer to CD buffer under construction.

      DWORD cd_input_cnt   - current offset into CD buffer.

      DWORD *cd_output_cnt - offset into CD buffer after CDTABLEBEGIN
                             was added.

      WORD Left_Margin     - Leftmost Margin value for the paragraph.

      WORD Right_Margin    - Rightmost Margin value for the paragraph.

      WORD Flags           - paragraph attribute flags.

    RETURNS:
        Returns TRUE if successful, FALSE if not enough memory in buffer.

*************************************************************************/

BOOL  CDPutPabdef(WORD para_num, BYTE **cd_buffer_ptr)
{
    CDPABDEFINITION pabdef;      /* style definition for this paragraph */

    memset (&pabdef, 0, sizeof(CDPABDEFINITION));

    pabdef.PABID                  = para_num;
    pabdef.Header.Signature       = SIG_CD_PABDEFINITION;
    pabdef.Header.Length          = ODSLength( _CDPABDEFINITION );
    pabdef.JustifyMode            = DEFAULT_JUSTIFICATION;
    pabdef.LineSpacing            = DEFAULT_LINE_SPACING;
    pabdef.ParagraphSpacingBefore = DEFAULT_ABOVE_PAR_SPACING;
    pabdef.ParagraphSpacingAfter  = DEFAULT_BELOW_PAR_SPACING;
    
    pabdef.RightMargin            = DEFAULT_RIGHT_MARGIN;
    pabdef.LeftMargin             = DEFAULT_LEFT_MARGIN; 
    pabdef.FirstLineLeftMargin    = DEFAULT_FIRST_LEFT_MARGIN;

    pabdef.Tabs                   = DEFAULT_TABS;
    pabdef.Tab[0]                 = DEFAULT_TAB_INTERVAL;
    pabdef.Flags                  = 0;
    pabdef.TabTypes               = TAB_DEFAULT;
    pabdef.Flags2                 = 0;

/*
 *  Convert the PABDEFINITION item to Domino and Notes Canonical format 
 *  and store it in the ODS buffer.
 */

    ODSWriteMemory( cd_buffer_ptr, _CDPABDEFINITION, &pabdef, 1 );

    return (TRUE);
}


/************************************************************************

    FUNCTION:   CDPutPabref

    PURPOSE:    Create a paragraph attribute block (PAB) reference
                and write it to a buffer.

    ARGUMENTS:
     WORD para_num        - The paragraph definition ID to which this pabref
                            refers.

     BYTE **cd_out_ptr    - pointer to CD buffer under construction.

     DWORD cd_input_cnt   - current offset into CD buffer.

     DWORD *cd_output_cnt - offset into CD buffer after CDTABLEBEGIN
                            was added.

    RETURNS:
        Returns TRUE if successful, FALSE if not enough memory in buffer.

************************************************************************/

BOOL  CDPutPabref(WORD para_num, BYTE **cd_buffer_ptr)
{
    CDPABREFERENCE PabRef;


    PabRef.Header.Signature = (BYTE) SIG_CD_PABREFERENCE;
    PabRef.Header.Length    = (BYTE) ODSLength( _CDPABREFERENCE );
    PabRef.PABID            = para_num;

 
/*
 *  Convert the CDPABREFERENCE item to Domino and Notes Canonical format 
 *  and store it in the ODS buffer.
 */

    ODSWriteMemory( cd_buffer_ptr, _CDPABREFERENCE, &PabRef, 1 );

    return (TRUE);
}


/************************************************************************

    FUNCTION:   CDPutText

    PURPOSE:    Create a text block, fill it in, and append it and a
                block of text to a buffer

    ARGUMENTS:

      font_choice  - which of the PROGRAMMER defined font/styles
                     to use.

      text_buf     - pointer to text buffer to copy to CDTEXT record.

      text_size    - length of contents of text buffer.

      cd_out_ptr   - pointer to CD buffer under construction.

      cd_input_cnt - current offset into CD buffer.

      cd_output_cnt - offset into CD buffer after CDTABLEBEGIN
                      was added.

    RETURN:
        TRUE if successful, FALSE if not enough memory in buffer.

************************************************************************/

BOOL  CDPutText(char *text_buf, WORD text_size,
                BYTE **cd_buffer_ptr)
{
    CDTEXT        CDText;

    memset (&CDText,0,sizeof(CDTEXT));
    CDText.Header.Signature = SIG_CD_TEXT;
    CDText.Header.Length    = ODSLength( _CDTEXT ) + text_size;
    CDText.FontID           = DEFAULT_FONT_ID;
    
 /* Convert the CDTEXT item to Domino and Notes Canonical format and store 
 *  it in the ODS buffer.
 */
 
    ODSWriteMemory( cd_buffer_ptr, _CDTEXT, &CDText, 1 );


 /* Fill in the actual text of this text run and advance buffer pointer. Do not
    append a null to the text run but ensure even byte size. */

   if (text_size != 0)
   {
    memcpy( (char *) *cd_buffer_ptr, text_buf, (text_size % 2) ? text_size + 1 : text_size );

    cd_buffer_ptr += (text_size % 2) ? text_size + 1 : text_size;
   }

    return (TRUE);
}
