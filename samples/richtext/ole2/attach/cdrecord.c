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

    PROGRAM:    embedole

    FILE:       cdrecord.c

    PURPOSE:    Functions for appending CD records to a buffer.

****************************************************************************/

 /* Windows header file */

#include <windows.h>

/* C header files         */

#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <names.h>
#include <fcntl.h>

/* HCL C API for Notes/Domino header files      */

#include <global.h>
#include <ods.h>
#include <editdflt.h>
#include <editods.h>
#include <colorid.h>
#include "../../../printLog.h"


/**********************************************************************
 *
 *    PutText
 *        API function to write a "rich text" text segment 
 *        to a buffer that will be written as a rich text field to 
 *        Domino and Notes.
 * 
 *   Arguments:
 *       BYTE *pbRTItem         ptr to buffer containing the CD record
 *                              being created.
 *       LPSTR pText            Far ptr to text to go into CD record.
 *       DWORD dwLength         Number of bytes remaining in the buffer. 
 *       DWORD FAR *pdwRTLength Current length of the CD record;
 *
 *    Returns:
 *       BOOL TRUE  if Successful
 *       BOOL FALSE if Not Successful
 *
 *    Comments:
 *
 **********************************************************************/

BOOL LNPUBLIC PutText(BYTE *pbRTItem, LPSTR pText,
                            DWORD dwLength, DWORD FAR *pdwRTLength)

{
    CDTEXT         cdText;        /* begin a run of text */
    WORD           wTextLength;
    BYTE *         pBuffer;       /* Pointer into CD record buffer */
    union {
                FONTIDFIELDS   fontFields;
                FONTID         fontId;
    } fontData;

    /* if nothing in CD record or current record len is 0, then exit. */
    
    if (pbRTItem == NULL || pdwRTLength == (DWORD far *)NULL)
    {
        PRINTLOG("Error in PutText: Current buffer is empty\n");
        return (FALSE);
    }

    if (pText != NULL)
        wTextLength = lstrlen(pText);
    else
        wTextLength = 0;

    if (((DWORD) (ODSLength(_CDTEXT) + wTextLength)) > dwLength)
    {
        PRINTLOG("Error in PutText: Not enough space left in the buffer\n");
        return (FALSE);
    }

    pBuffer = pbRTItem + *pdwRTLength;

    /* Fill in the TEXT item structure.       */
    
    cdText.Header.Signature = SIG_CD_TEXT;
    cdText.Header.Length = ODSLength(_CDTEXT) + wTextLength;
  
   /*
    *  Fill in the font information for this run of text. Note that
    *  we must overlay the FONTIDFIELDS structure onto the
    *  pbRTItem.text.FontID data item.
    */

    fontData.fontFields.Face = FONT_FACE_SWISS;
    fontData.fontFields.Attrib = 0;
    fontData.fontFields.Color = NOTES_COLOR_BLACK;
    fontData.fontFields.PointSize = 10;

    cdText.FontID = fontData.fontId;

    ODSWriteMemory (&pBuffer, _CDTEXT, &cdText, 1);

    if (pText != NULL)
        memmove(pBuffer, pText, wTextLength);   /* Append text string
                                                       to CDTEXT record. */
    *pdwRTLength += cdText.Header.Length;

    if (*pdwRTLength % 2)
        (*pdwRTLength)++;

    return (TRUE);
}

/**********************************************************************
 *
 *    PutOLEBegin
 *         API function to write a "rich text" OLEBEGIN
 *        segment to a buffer that will be written as a rich text 
 *        field in Domino and Notes.
 * 
 *    Arguments:
 *        BYTE FAR *pbRTItem;       Far pointer to CD record.
 *        LPSTR pszAttachmentName;  Name of the attached file.
 *        DWORD dwLength;           Number of bytes remaining in buffer.
 *        DWORD FAR *pdwRTLength;   Current length of CD record.
 *
 *    Returns:
 *        BOOL TRUE  if Successful
 *        BOOL FALSE if Not Successful
 *
 *    Comments:
 *
 **********************************************************************/

BOOL LNPUBLIC PutOLEBegin(BYTE far *pbRTItem,
                          LPSTR pszAttachmentName,
                          LPSTR pszClassName,
                          DWORD dwLength, 
                          DWORD far *pdwRTLength)
{
    CDOLEBEGIN      CDOLEBegin;
    BYTE *          pBuffer;

    /*
     * if nothing in CD record or current record len is 0, then exit.
     */
    
    if (pbRTItem == NULL || pdwRTLength == (DWORD far *)NULL)
    {
        PRINTLOG("Error in PutOLEBegin: Item buffer is invalid\n");
        return (FALSE);
    }

    /*
     * If not enough space in buffer for this struct, then exit.
     */

    if (((DWORD) (ODSLength(_CDOLEBEGIN) +
         lstrlen(pszAttachmentName) +
         lstrlen(pszClassName))) > dwLength)
    {
        PRINTLOG("Error in PutOLEBegin: Not enough space left in the item buffer\n");
        return (FALSE);
    }

    pBuffer = pbRTItem + *pdwRTLength;

    /*
     *  Set the Domino and Notes OLE implementation version.
     */

    CDOLEBegin.Version = NOTES_OLEVERSION2;

    /*
     * Set Flags.
     */
    
    CDOLEBegin.Flags = OLEREC_FLAG_OBJECT;

    /*
     * Set ClipFormat as metafile for now
     */
    
    CDOLEBegin.ClipFormat = DDEFORMAT_METAFILE;        


    /*
     * Set string lengths as required
     */
    
    CDOLEBegin.AttachNameLength = lstrlen(pszAttachmentName);
    CDOLEBegin.ClassNameLength = lstrlen(pszClassName);
    CDOLEBegin.TemplateNameLength = 0;
          
    /*
     *  Build header.
     */
    
    CDOLEBegin.Header.Signature = SIG_CD_OLEBEGIN;
    CDOLEBegin.Header.Length = ODSLength(_CDOLEBEGIN) +
                                 CDOLEBegin.AttachNameLength +
                                 CDOLEBegin.ClassNameLength +
                                 CDOLEBegin.TemplateNameLength;

    ODSWriteMemory (&pBuffer, _CDOLEBEGIN, &CDOLEBegin, 1);

/*
 *  Point to start of variable length part of structure, then copy
 *  the strings.
 */
 
    memcpy(pBuffer, pszAttachmentName, CDOLEBegin.AttachNameLength);
    pBuffer += lstrlen(pszAttachmentName);
    memcpy(pBuffer, pszClassName, CDOLEBegin.ClassNameLength);

    *pdwRTLength += CDOLEBegin.Header.Length;

    if (*pdwRTLength % 2)
        (*pdwRTLength)++;

    return (TRUE);
}

/**********************************************************************
 *    
 *   PutOLEEnd
 *       API function to write a "rich text" OLEEND segment
 *       to a buffer that will be written as a rich text field to 
 *       Domino and Notes
 *     
 *   Arguments:
 *       BYTE *pbRTItem         Ptr to buffer containing the CD record
 *                              being created.
 *       DWORD dwLength         Number of bytes remaining in the buffer. 
 *       DWORD FAR *pdwRTLength Current length of the CD record;
 *    
 *   Returns:
 *       BOOL TRUE  if Successful
 *       BOOL FALSE if Not Successful
 * 
 *   Comments:
 *    
 ***********************************************************************/

BOOL LNPUBLIC PutOLEEnd(BYTE *pbRTItem, DWORD dwLength, 
                          DWORD FAR *pdwRTLength)
{
    CDOLEEND        CDOLEEnd;
    BYTE *          pBuffer;

    /* if nothing in CD record or current record len is 0, then exit. */
    
    if (pbRTItem == NULL || pdwRTLength == (DWORD far *)NULL)
    {
        PRINTLOG("Error in PutOLEEnd: Invalid buffer\n");
        return (FALSE);
    }

    /* If not enough space in buffer for this CDOLEEnd, then exit.   */
    
    if (dwLength < ODSLength(_CDOLEEND))
    {
        PRINTLOG("Error in PutOLEEnd: Not enough space left in the buffer\n");
        return (FALSE);
    }

    /* Fill in CDOLEEND item structure  */
        
    CDOLEEnd.Header.Length = ODSLength(_CDOLEEND);
    CDOLEEnd.Header.Signature = SIG_CD_OLEEND;
    CDOLEEnd.Flags = 0L;

    pBuffer = pbRTItem + *pdwRTLength;

    ODSWriteMemory (&pBuffer, _CDOLEEND, &CDOLEEnd, 1);

    /* Adjust current record length, forcing to an even byte count.   */
    
    *pdwRTLength += CDOLEEnd.Header.Length;

    if (*pdwRTLength % 2)
        (*pdwRTLength)++;

    return (TRUE);
 }
