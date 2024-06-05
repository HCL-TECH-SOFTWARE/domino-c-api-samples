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

    PROGRAM:      ConvertHtmlToNSF

    FILE:         CDRecord.c

    PURPOSE:      Functions for appending CD records to a buffer

****************************************************************************/

/* Include Windows header file only if compiling for Windows. */
 
#if defined (DOS) && defined (W)
#include <windows.h>
#endif

/* C header files */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <names.h>

/* HCL C API for Notes/Domino header files      */

#include <global.h>
#include <editdflt.h>
#include <nsfnote.h>
#include <osmem.h>
#include <editods.h>
#include <colorid.h>


/**********************************************************************
 *
 *    PutPabDef
 *        API function to write a "rich text" paragraph 
 *        attribute block (PAB) definition segment to a 
 *        buffer that will be written as a rich text field to Domino 
 *        and Notes
 * 
 *   Arguments:
 *       char **ppBuf         address of pointer to next available byte in 
 *                            buffer containing the item being created.
 *       WORD wPabDefNumber   A unique ID to use for this PabDefinition.
 *       WORD wLength         Number of bytes remaining in the buffer. 
 *       WORD wJustifyMode    Justify mode of paragraph. 
 *
 *    Returns:
 *        BOOL TRUE  if Successful
 *        BOOL FALSE if Not Successful
 *
 *    Comments:
 *
 **********************************************************************/

BOOL LNPUBLIC PutPabDef(char far * far * ppBuf,
                          WORD wPabDefNumber,
                          WORD wLength,
                          WORD wJustifyMode)
{
    CDPABDEFINITION  CDPabDef;      /* style definition for this para */

    /* If not enough space in buffer for this struct, then exit.        */

    if (wLength < ODSLength(_CDPABDEFINITION))
        return (FALSE);
    
   /* Fill in paragraph definition block.  We use all defaults, except  */
   /* for centered justification.                                       */

    memset (&CDPabDef, 0, sizeof(CDPABDEFINITION));

    CDPabDef.Header.Signature       = SIG_CD_PABDEFINITION;
    CDPabDef.Header.Length          = ODSLength(_CDPABDEFINITION);
    CDPabDef.PABID                  = wPabDefNumber;
    CDPabDef.JustifyMode            = wJustifyMode;
    CDPabDef.LineSpacing            = DEFAULT_LINE_SPACING;
    CDPabDef.ParagraphSpacingBefore = DEFAULT_ABOVE_PAR_SPACING;
    CDPabDef.ParagraphSpacingAfter  = DEFAULT_BELOW_PAR_SPACING;
    CDPabDef.LeftMargin             = DEFAULT_LEFT_MARGIN;
    CDPabDef.RightMargin            = DEFAULT_RIGHT_MARGIN;
    CDPabDef.FirstLineLeftMargin    = DEFAULT_FIRST_LEFT_MARGIN;
    CDPabDef.Tabs                   = DEFAULT_TABS;
    CDPabDef.Tab[0]                 = DEFAULT_TAB_INTERVAL;
    CDPabDef.Flags                  = 0;
    CDPabDef.TabTypes               = TAB_DEFAULT;
    CDPabDef.Flags2                 = 0;

/*
 *  Convert the PABDEFINITION item to Domino and Notes Canonical format and 
 *  store it in the ODS buffer.
 */

    ODSWriteMemory( (void far * far *)ppBuf, _CDPABDEFINITION, &CDPabDef, 1 );
     
    return (TRUE);
}


/**********************************************************************
 *    
 *   PutPara
 *       API function to write a CDPARAGRAPH segment to a buffer that will
 *       be written as a rich text field to Domino and Notes
 *
 *   Arguments:
 *       char far * far *ppBuf Address of pointer to next available byte in 
 *                             buffer containing the item being created.
 *       WORD wLength          Number of bytes remaining in the buffer. 
 *
 *    Returns:
 *        BOOL TRUE  if Successful
 *        BOOL FALSE if Not Successful
 *     
 * 
 *   Comments:
 *    
 ***********************************************************************/

BOOL LNPUBLIC PutPara( char far * far *ppBuf,
                         WORD wLength )
{
    CDPARAGRAPH CDPara;
    BYTE        bLength =  (BYTE) ODSLength(_CDPARAGRAPH);
    BYTE        bSig  =  (BYTE)SIG_CD_PARAGRAPH;
    

    /* If not enough space in buffer for this paragraph, then exit.   */
    
    if (wLength < ODSLength(_CDPARAGRAPH))
        return (FALSE);

    /* Fill in PARAGRAPH item structure  */
        
    CDPara.Header.Length = bLength;/* (BYTE) ODSLength(_CDPARAGRAPH); */
    CDPara.Header.Signature = bSig;/* SIG_CD_PARAGRAPH; */

/*
 *  Convert the CDPARAGRAPH item to Domino and Notes Canonical format and 
 *  store it in the ODS buffer.
 */

    ODSWriteMemory( (void far * far *)ppBuf, _CDPARAGRAPH, &CDPara, 1 );
     
    return (TRUE);
}

/**********************************************************************
 *
 *    PutPabRef
 *        API function to write a "rich text" paragraph 
 *        attribute block (PAB) reference segment to a 
 *        buffer that will be written as a rich text field to Domino 
 *        and Notes
 * 
 *   Arguments:
 *       char far * far *ppBuf  Address of pointer to next available byte in 
 *                              buffer containing the item being created.
 *       WORD wPabDefNumber     The paragraph definition to which this pabref
 *                              refers.
 *       WORD wLength           Number of bytes remaining in the buffer. 
 *
 *    Returns:
 *        BOOL TRUE  if Successful
 *        BOOL FALSE if Not Successful
 *
 *
 *    Comments:
 *
 **********************************************************************/

BOOL LNPUBLIC PutPabRef(char far * far *ppBuf,
                          WORD wPabDefNumber,
                          WORD wLength)
{
    CDPABREFERENCE  CDPabRef;      /* style reference for this para */
    BYTE        bLength =  (BYTE) ODSLength(_CDPABREFERENCE);
    BYTE        bSig  =  (BYTE)SIG_CD_PABREFERENCE;
    

    /* If not enough space in buffer for this struct, then exit.      */

    if (wLength < ODSLength(_CDPABREFERENCE))
        return (FALSE);

    /* Fill in PABREF item structure.    */ 
    
    CDPabRef.Header.Signature = bSig;/* SIG_CD_PABREFERENCE; */
    CDPabRef.Header.Length    = bLength;/* (BYTE) ODSLength(_CDPABREFERENCE); */
    CDPabRef.PABID            = wPabDefNumber;


/*
 *  Convert the CDPABREFERENCE item to Domino and Notes Canonical format and 
 *  store it in the ODS buffer.
 */

    ODSWriteMemory( (void far * far *)ppBuf, _CDPABREFERENCE, &CDPabRef, 1 );
     
    return (TRUE);
}
 
