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

    PROGRAM:    hotspot

    FILE:       cdrecord.c

    PURPOSE:    Functions for appending CD records to a buffer.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

 /*
  *  Windows header file
  */


#if !defined(UNIX) && !defined(MAC)
#include <windows.h>
#include <io.h>
#endif


/*
 *  C header files
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <names.h>
#include <fcntl.h>

/*
 *  HCL C API for Notes/Domino header files
 */

#include <global.h>
#include <editdflt.h>
#include <nsfnote.h>
#include <osmem.h>
#include <editods.h>
#include <colorid.h>
#include <queryods.h>
#include <ods.h>


/*
 *  Local includes
 */

#include "hotspot.h"

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

BOOL PutPara( char far * far *ppBuf,
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
 *  Convert the CDPARAGRAPH item to Domino and Notes Canonical format
 *  and store it in the ODS buffer.
 */

    ODSWriteMemory( (void far * far *)ppBuf, _CDPARAGRAPH, &CDPara, 1 );

    return (TRUE);
}

/**********************************************************************
 *
 *    PutText
 *        API function to write a "rich text" text segment
 *        to a buffer that will be written as a rich text field to
 *        Domino and Notes.
 *
 *   Arguments:
 *       char far * far *ppBuf Address of pointer to next available byte in
 *                             buffer containing the item being created.
 *       char far * pText      Far ptr to text to go into CD record.
 *       WORD wLength          Number of bytes remaining in the buffer.
 *       int  Type             Text Type - only checks for
 *                             HOTSPOTREC_TYPE_HOTLINK
 *                             otherwise pass 0.
 *    Returns:
 *        BOOL TRUE  if Successful
 *        BOOL FALSE if Not Successful
 *
 *    Comments:
 *
 **********************************************************************/

BOOL PutText(char far * far *ppBuf,
                        char far * pText,
                        WORD wLength,
                        int Type)

{
    CDTEXT         CDText;        /* begin a run of text */
    FONTIDFIELDS   *pFont;        /* font definitions in rich text field */
    WORD           wTextLength;
    WORD           w;


    if (pText != NULL)
        wTextLength = strlen(pText);
    else
        wTextLength = 0;

    w = ODSLength(_CDTEXT)+wTextLength;

    if ( wLength < w)

/* (ODSLength(_CDTEXT) + wTextLength) > wLength */


        return (FALSE);

/* Fill in the TEXT item structure. */

    CDText.Header.Signature = SIG_CD_TEXT;
    CDText.Header.Length = ODSLength(_CDTEXT) + wTextLength;

/*
 *  Fill in the font information for this run of text. Note that
 *  we must overlay the FONTIDFIELDS structure onto the
 *  CDText.FontID data item.
 */

    pFont = (FONTIDFIELDS*)&CDText.FontID;

    pFont->Face = FONT_FACE_SWISS;

/*
 * Set the text to bold and underline if this is a Hotlink (URL).
 */

    if (Type == HOTSPOTREC_TYPE_HOTLINK)
      pFont->Attrib = ISBOLD | ISUNDERLINE;
    else
      pFont->Attrib = 0;

    pFont->Color = NOTES_COLOR_BLACK;
    pFont->PointSize = 10;


/*
 *  Convert the CDTEXT item to Domino and Notes Canonical format and store it in
 *  the ODS buffer.
 */

    ODSWriteMemory( (void far * far *)ppBuf, _CDTEXT, &CDText, 1 );

    if (pText != NULL)
    {
        memcpy( *ppBuf, pText, wTextLength );  /* Append string to CDTEXT. */
        *ppBuf += wTextLength;
    }

#if defined W64
    if (((long long)*ppBuf) % 2)
        *ppBuf += 1;
#else
    if (((long)*ppBuf) % 2)
        *ppBuf += 1;
#endif
  
    return (TRUE);
}


/**********************************************************************
 *
 *   PutHotSPotEnd
 *
 *   Arguments:
 *       char *pbRTItem       Ptr to buffer containing the CD record
 *                            being created.
 *       WORD wLength         Number of bytes remaining in the buffer.
 *       WORD FAR *pwRTLength Current length of the CD record;
 *
 *   Returns:
 *       STATUS NOERROR               if Successful
 *       STATUS ERR_CREATING_RICHTEXT if Not Successful
 *
 *   Comments:
 *
 ***********************************************************************/

STATUS PutHotSpotEnd(char far * far *pbRTItem, WORD wLength,
                          WORD FAR *pwRTLength)
{
    CDHOTSPOTEND pCDHotSpotEnd;

/*
 *  if nothing in CD record or current record len is 0, then exit.
 */

    if (pbRTItem == NULL || pwRTLength == (WORD far *)NULL)
        return (ERR_CREATING_RICHTEXT);

/*
 *  If not enough space in buffer for this CDHotSpotEnd, then exit.
 */

    if (wLength < ODSLength(_CDHOTSPOTEND))
        return (ERR_CREATING_RICHTEXT);

/*
 *  Fill in CDHOTSPOTEND item structure
 */



    pCDHotSpotEnd.Header.Length = (BYTE)ODSLength(_CDHOTSPOTEND);
    pCDHotSpotEnd.Header.Signature = SIG_CD_HOTSPOTEND;

    ODSWriteMemory( (void far * far *)pbRTItem, _CDHOTSPOTEND,
                    &pCDHotSpotEnd, 1 );


/*
 *  Adjust current record length, forcing to an even byte count.
 */

    *pwRTLength += pCDHotSpotEnd.Header.Length;

    if (*pwRTLength % 2)
        (*pwRTLength)++;

    return (NOERROR);
}

/**********************************************************************
 *
 *   PutV4HotSPotEnd
 *
 *   Arguments:
 *       char *pbRTItem       Ptr to buffer containing the CD record
 *                            being created.
 *       WORD wLength         Number of bytes remaining in the buffer.
 *       WORD FAR *pwRTLength Current length of the CD record;
 *
 *   Returns:
 *       STATUS NOERROR               if Successful
 *       STATUS ERR_CREATING_RICHTEXT if Not Successful
 *
 *   Comments:
 *
 ***********************************************************************/

STATUS PutV4HotSpotEnd(char far * far *pbRTItem, WORD wLength,
                          WORD FAR *pwRTLength)
{
    CDHOTSPOTEND pCDHotSpotEnd;

/*
 *  if nothing in CD record or current record len is 0, then exit.
 */

    if (pbRTItem == NULL || pwRTLength == (WORD far *)NULL)
        return (ERR_CREATING_RICHTEXT);

/*
 *  If not enough space in buffer for this CDHotSpotEnd, then exit.
 */

    if (wLength < ODSLength(_CDHOTSPOTEND))
        return (ERR_CREATING_RICHTEXT);

/*
 *  Fill in CDHOTSPOTEND item structure
 */



    pCDHotSpotEnd.Header.Length = (BYTE)ODSLength(_CDHOTSPOTEND);
    pCDHotSpotEnd.Header.Signature = SIG_CD_V4HOTSPOTEND;

    ODSWriteMemory( (void far * far *)pbRTItem,
                   _CDHOTSPOTEND, &pCDHotSpotEnd, 1 );


/*
 *  Adjust current record length, forcing to an even byte count.
 */

    *pwRTLength += pCDHotSpotEnd.Header.Length;

    if (*pwRTLength % 2)
        (*pwRTLength)++;

    return (NOERROR);
}



/**********************************************************************
 *
 *    PutButton
 *        API function to write a "rich text" button segment
 *        to a buffer that will be written as a rich text field to
 *        Domino and Notes.
 *
 *   Arguments:
 *       char *pbRTItem       ptr to buffer containing the CD record
 *                            being created.
 *       char *pButtonText    ptr to text to display on button.
 *       WORD wLength         Number of bytes remaining in the buffer.
 *       WORD FAR *pwRTLength Current length of the CD record;
 *
 *    Returns:
 *        STATUS NOERROR               if Successful
 *        STATUS ERR_CREATING_RICHTEXT if Not Successful
 *
 *    Comments:
 *
 **********************************************************************/

STATUS PutButton(char far * far *pbRTItem, char *pButtonText,
                            WORD wLength, WORD FAR *pwRTLength)

{
    CDBUTTON       pcdButton;    /* begin a run of text */
    FONTIDFIELDS   *pFont;       /* font definitions in rich text field */
    WORD           wTextLength;
/*
 *  if nothing in CD record or current record len is 0, then exit.
 */

    if (pbRTItem == NULL || pwRTLength == (WORD far *)NULL)
        return (ERR_CREATING_RICHTEXT);

    if (pButtonText != NULL)
        wTextLength = strlen(pButtonText);
    else
        wTextLength = 0;

    if ((ODSLength(_CDBUTTON) + wTextLength) > wLength)
        return (ERR_CREATING_RICHTEXT);

/*
 *  Fill in the BUTTON item structure.
 */

    pcdButton.Header.Signature = SIG_CD_BUTTON;
    pcdButton.Header.Length = ODSLength(_CDBUTTON) + wTextLength;


    pcdButton.Flags = 0;          /* No flags set for button. */
    pcdButton.Width = 2*ONEINCH;  /* Button is 2 inches wide. */
    pcdButton.Height = 0;
    pcdButton.Lines = 1;

 /*
  *  Fill in the font information for this button. Note that
  *  we must overlay the FONTIDFIELDS structure onto the
  *  pbRTItem.button.FontID data item.
  */

    pFont = (FONTIDFIELDS*)&pcdButton.FontID;

    pFont->Face = FONT_FACE_SWISS;
    pFont->Attrib = 0;
    pFont->Color = NOTES_COLOR_BLACK;
    pFont->PointSize = 10;

    ODSWriteMemory( (void far * far *)pbRTItem, _CDBUTTON, &pcdButton, 1 );

 /*
  *  Append text string to CDTEXT record.
  */
    if (pButtonText != NULL)
        {
        memcpy(*pbRTItem, pButtonText, wTextLength);
                *pbRTItem += wTextLength;
        }

#if defined W64
    if (((long long)*pbRTItem) % 2)
        *pbRTItem += 1;
#else
    if (((long)*pbRTItem) % 2)
        *pbRTItem += 1;
#endif

    *pwRTLength += pcdButton.Header.Length;

    if (*pwRTLength % 2)
        (*pwRTLength)++;

    return (NOERROR);
}


/**********************************************************************
 *
 *    PutBar
 *        API function to write a "rich text" bar segment
 *        to a buffer that will be written as a rich text field to
 *        Domino and Notes.
 *
 *   Arguments:
 *       char *pbRTItem       ptr to buffer containing the CD record
 *                            being created.
 *       char *pBarText       ptr to text to display on bar.
 *       WORD wLength         Number of bytes remaining in the buffer.
 *       WORD FAR *pwRTLength Current length of the CD record;
 *
 *    Returns:
 *        STATUS NOERROR               if Successful
 *        STATUS ERR_CREATING_RICHTEXT if Not Successful
 *
 *    Comments:
 *
 **********************************************************************/

STATUS PutBar(char far * far *pbRTItem, char *pBarText,
                            WORD wLength, WORD FAR *pwRTLength)

{
    CDBAR           pcdBar;       /* begin a run of text */
    FONTIDFIELDS   *pFont;        /* font definitions in rich text field */
    WORD            wTextLength;
    WORD            color;
/*
 *  if nothing in CD record or current record len is 0, then exit.
 */

    if (pbRTItem == NULL || pwRTLength == (WORD far *)NULL)
        return (ERR_CREATING_RICHTEXT);

    if (pBarText != NULL)
        wTextLength = strlen(pBarText);
    else
        wTextLength = 0;

    if ((ODSLength(_CDBAR) + wTextLength + ODSLength(_WORD) + 1)> wLength)
        return (ERR_CREATING_RICHTEXT);

/*
 *  Fill in the BAR item structure.
 */

    pcdBar.Header.Signature = SIG_CD_BAR;
    pcdBar.Header.Length = ODSLength(_CDBAR) + wTextLength + ODSLength(_WORD) + 1;


    pcdBar.Flags = BARREC_HAS_COLOR;

    SETBORDERTYPE(pcdBar.Flags, BARREC_BORDER_NONE);

 /*
  *  Fill in the font information for this button. Note that
  *  we must overlay the FONTIDFIELDS structure onto the
  *  pbRTItem.button.FontID data item.
  */

    pFont = (FONTIDFIELDS*)&pcdBar.FontID;

    pFont->Face = FONT_FACE_SWISS;
    pFont->Attrib = 0;
    pFont->Color = NOTES_COLOR_BLACK;
    pFont->PointSize = 10;

    ODSWriteMemory( (void far * far *)pbRTItem, _CDBAR, &pcdBar, 1 );
        color = NOTES_COLOR_DKCYAN;
    ODSWriteMemory( (void far * far *)pbRTItem, _WORD, &color, 1 );

 /*
  *  Append text string to CDBAR record.
  */
    if (pBarText != NULL)
        {
                strcpy(*pbRTItem, pBarText);
                *pbRTItem += 1;
                *pbRTItem += wTextLength;
        }

#if defined W64
    if (((long long)*pbRTItem) % 2)
        *pbRTItem += 1;
#else
    if (((long)*pbRTItem) % 2)
        *pbRTItem += 1;
#endif

    *pwRTLength += pcdBar.Header.Length;

    if (*pwRTLength % 2)
        (*pwRTLength)++;

    return (NOERROR);
}

/**********************************************************************
 *
 *    PutAction
 *        API function to write a "rich text" action segment
 *        to a buffer that will be written as a rich text field to
 *        Domino and Notes.
 *
 *   Arguments:
 *       char *pbRTItem       ptr to buffer containing the CD record
 *                            being created.
 *       char *pBarText       Far ptr to text to display for action.
 *       WORD wLength         Number of bytes remaining in the buffer.
 *       WORD FAR *pwRTLength Current length of the CD record;
 *
 *    Returns:
 *        STATUS NOERROR               if Successful
 *        STATUS ERR_CREATING_RICHTEXT if Not Successful
 *
 *    Comments:
 *
 **********************************************************************/

STATUS PutAction(char far * far *pbRTItem, char * pToInfo,
                 char * pSubjectField, char * pBodyField,
                 WORD wLength, WORD FAR *pwRTLength)

{
    CDACTIONSENDMAIL    ActSendMailRec;
    WORD                wToLength, wSubjectLength, wBodyLength;
    CDACTIONHEADER      ActionHeader;
    WORD                ActionType = TYPE_ACTION;

/*
 *  if nothing in CD record or current record len is 0, then exit.
 */

    if (pbRTItem == NULL || pwRTLength == (WORD far *)NULL)
        return (ERR_CREATING_RICHTEXT);

    /* include the Nulls and padding byte if odd length */
    wToLength = strlen(pToInfo) + 1;
    if (wToLength % 2)
        wToLength += 1;

    wSubjectLength = strlen(pSubjectField) + 1;
    if (wSubjectLength % 2)
        wSubjectLength += 1;

    wBodyLength = strlen(pBodyField) + 1;
    if (wBodyLength % 2)
        wBodyLength += 1;

    if ((2 + ODSLength(_CDACTIONHEADER) +
             ODSLength(_CDACTIONSENDMAIL) + wToLength +
             wSubjectLength +  wBodyLength)> wLength)
        return (ERR_CREATING_RICHTEXT);

    /* write a TYPE_ACTION word to the buffer */
    ODSWriteMemory( (void far * far *)pbRTItem, _WORD, &ActionType, 1 );

    *pwRTLength += 2;

/*
 *  Fill in the Action Header.
 */

    ActionHeader.Header.Signature = SIG_ACTION_HEADER;
    ActionHeader.Header.Length = (BYTE) ODSLength(_CDACTIONHEADER);


    *pwRTLength += ActionHeader.Header.Length;

    ODSWriteMemory( (void far * far *)pbRTItem, _CDACTIONHEADER, &ActionHeader, 1 );

/*
 *  Fill in the ACTIONSENDMAIL item structure.
 */

    ActSendMailRec.Header.Signature = SIG_ACTION_SENDMAIL;
    ActSendMailRec.Header.Length = ODSLength(_CDACTIONSENDMAIL) +
         wToLength + wSubjectLength +  wBodyLength;


    ActSendMailRec.dwFlags = 0;

 /* Set the various fields  we are using */
    ActSendMailRec.wFieldLen[ACTIONSENDMAIL_TOFIELD] = wToLength;
    ActSendMailRec.wFieldLen[ACTIONSENDMAIL_SUBJECTFIELD] = wSubjectLength;
    ActSendMailRec.wFieldLen[ACTIONSENDMAIL_BODYFIELD] = wBodyLength;

 /* Set the unused fields to 0 */
    ActSendMailRec.wFieldLen[ACTIONSENDMAIL_CCFIELD] = 0;
    ActSendMailRec.wFieldLen[ACTIONSENDMAIL_BCCFIELD] = 0;

    ODSWriteMemory( (void far * far *)pbRTItem, _CDACTIONSENDMAIL, &ActSendMailRec, 1 );


 /* write the strings to the buffer */

    memset(*pbRTItem, NULL, wToLength);
    memcpy(*pbRTItem, pToInfo, wToLength);
    *pbRTItem += wToLength;

    memset(*pbRTItem, NULL, wSubjectLength);
    memcpy(*pbRTItem, pSubjectField, wSubjectLength);
    *pbRTItem += wSubjectLength;

    memset(*pbRTItem, NULL, wBodyLength);
    memcpy(*pbRTItem, pBodyField, wBodyLength);
    *pbRTItem += wBodyLength;

    *pwRTLength += ActSendMailRec.Header.Length;

    if (*pwRTLength % 2)
        (*pwRTLength)++;

    return (NOERROR);
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
 *       BYTE **ppbRTItem     Handle to buffer containing the CD record
 *                            being created.
 *       WORD wPabDefNumber   The ID of the PabDef to use in this PabRef.
 *       WORD wLength         Number of bytes remaining in the buffer.
 *       WORD FAR *pwRTLength Current length of the CD record;
 *
 *    Returns:
 *        STATUS NOERROR               if Successful
 *        STATUS ERR_CREATING_RICHTEXT if Not Successful
 *
 *    Comments:
 *
 **********************************************************************/

STATUS PutPabRef(char far * far *ppbRTItem, WORD wPabDefNumber,
                            WORD wLength, WORD FAR *pwRTLength)
{
    CDPABREFERENCE  pcdPabRef;      /* style reference for this para */

/*
 *  if nothing in CD record or current record len is 0, then exit.
 */

    if (*ppbRTItem == NULL || pwRTLength == (WORD far *)NULL)
        return (ERR_CREATING_RICHTEXT);

/*
 *  If not enough space in buffer for this struct, then exit.
 */

    if (wLength < ODSLength(_CDPABREFERENCE))
        return (ERR_CREATING_RICHTEXT);

/*
 *  Fill in PABREF item structure.
 */


    pcdPabRef.Header.Signature = SIG_CD_PABREFERENCE;
    pcdPabRef.Header.Length    = (BYTE)ODSLength(_CDPABREFERENCE);
    pcdPabRef.PABID            = wPabDefNumber;

    *pwRTLength += ODSLength(_CDPABREFERENCE);

    if (*pwRTLength % 2)
        (*pwRTLength)++;

    ODSWriteMemory( (void far * far *)ppbRTItem, _CDPABREFERENCE, &pcdPabRef, 1 );

    return (NOERROR);
}

