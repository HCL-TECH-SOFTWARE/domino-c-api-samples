/***********************************************************************
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

   File:        hotspot.c

   Program:     hotspot

   Purpose:     shows how to create popups, sections, buttons, a URL,
                and a file attachment in a rich text field.

   Description:

       First, this program opens the database MAKEFORM.NSF and creates a
       new note in which to place the various objects. It then
       allocates a buffer in which to construct the rich text item. In
       this buffer it creates the following sequence of data structures:

       To define the popup:

       -  A CDHOTSPOTBEGIN record, with the flag member set to
          HOTSPOTREC_REC_POPUP, and with the other data members
          appropriately. The packed string containing the text to be
          displayed when the popup is clicked upon is appended to the
          CDHOTSPOTBEGIN record.

       -  A CDTEXT record, containing the text upon which one would click
          to display the popup text.

       -  A CDHOTSPOTEND record, to mark the end of the popup definition.


       To separate the popup from the button that follows:

       -  Two CDPARAGRAPH records. Since each paragraph starts on a new line,
          this has the effect of inserting a blank line after the popup above.


       To define the button:

       -  A CDHOTSPOTBEGIN record, with the flag member set to
          HOTSPOTREC_REC_BUTTON, and with the other data members
          appropriately. The formula to be run when the button is
          activated is appended to the CDHOTSPOTBEGIN record.

       -  A CDBUTTON record, which defines the appearance of the button.

       -  A CDHOTSPOTEND record, to mark the end of the button definition.


       To separate the button from the bar that follows:

       -  Two CDPARAGRAPH records. Since each paragraph starts on a new line,
          this has the effect of inserting a blank line after the button above.


       To define the bar:

       -  A CDV4HOTSPOTBEGIN record, with the type set to 
          HOTSPOTREC_TYPE_BUNDLE, and with the other data members 
          appropriately.

       -  A CDBAR record that will contain the section title and other
          information for the section.

       -  An empty CDTEXT record.

       -  A CDPARAGRAPH structure to the buffer;

       -  Append a CDPABREFERENCE structure in the buffer;

       -  A CDTEXT record that will contain the contents of the section.

       -  A CDV4HOTSPOTEND record to mark the end of the hotspot.


       To separate the bar from the Action that follows:

       -  Two CDPARAGRAPH records. Since each paragraph starts on a new line,
          this has the effect of inserting a blank line after the bar above.


       To define the Action:

       -  A CDV4HOTSPOTBEGIN record with the type set to
          HOTSPOTREC_TYPE_HOTREGION.

       -  A word containing TYPE_ACTION appended to the buffer.

       -  A CDACTIONHEADER structure to the buffer.

       -  A CDACTIONSENDMAIL structure to the buffer.

       -  A CDTEXT record that will contain the hotspot to click on.

       -  A CDV4HOTSPOTEND record to mark the end of the hotspot.


       To separate the action from the URL that follows:

       -  Two CDPARAGRAPH records. Since each paragraph starts on a new line,
          this has the effect of inserting a blank line after the bar above.
 

       To define the URL:

       -  A CDV4HOTSPOTBEGIN record with the type set to
          HOTSPOTREC_TYPE_HOTLINK.

       -  A CDTEXT record that will contain the URL address.

       -  A CDV4HOTSPOTEND record to mark the end of the hotspot.


       To separate the URL from the file attachment that follows:

       -  Two CDPARAGRAPH records. Since each paragraph starts on a new line,
          this has the effect of inserting a blank line after the bar above.


       To define the file attachment:

       -  Call NSFNoteAttachFile to create a $FILE item.

       -  A CDHOTSPOTBEGIN record with the type set to HOTSPOTREC_TYPE_FILE.

       -  A CDTEXT record that will contain the name of the attached file.

       -  A CDHOTSPOTEND record to mark the end of the hotspot.


       To separate the URL from the LotusScript button that follows:

       -  Two CDPARAGRAPH records. Since each paragraph starts on a new line,
          this has the effect of inserting a blank line after the bar above.


       To define the LS button:

       -  A CDBEGINRECORD record.

       -  A CDHOTSPOTBEGIN record, with the flag member set to
          HOTSPOTREC_REC_BUTTON, and with the other data members
          appropriately. The LotusScript to be run when the button is
          activated is appended to the CDHOTSPOTBEGIN record.

       -  A CDBUTTON record, which defines the appearance of the button.

       -  A CDHOTSPOTEND record.

       -  A CDENDRECORD record, to mark the end of the button definition.


       The buffer created is then appended to the note's rich text
       field. The note is updated and closed, the database is closed, and 
       the program exits.



************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/*
 *  OS includes
 */

#if defined(UNIX) || defined(MAC)
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#else
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#endif
/*
 *  HCL C API for Notes/Domino includes
 */

#include <global.h>
#include <nsf.h>
#include <osmem.h>
#include <ods.h>
#include <names.h>
#include <editods.h>
#include <editdflt.h>
#include <nsfnote.h>
#include <ixport.h>
#include <ixedit.h>
#include <osmisc.h>
#include <oserr.h>
#include <stdnames.h>
#include <agents.h>
#include <kfm.h>
#include "../../printLog.h"

/*
 *  Local includes
 */

#include "hotspot.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/*
 *  Forward Declaration
 */

STATUS InsertRichText(NOTEHANDLE hNote);

STATUS InsertPopup(NOTEHANDLE hNote,
                              char FAR * FAR *ppCDBuffer,
                              WORD FAR *pwItemSize,
                              WORD wBufferSize);

STATUS InsertButton(NOTEHANDLE hNote,
                               char FAR * FAR *ppCDBuffer,
                               WORD FAR *pwItemSize,
                               WORD wBufferSize);


STATUS InsertBar(NOTEHANDLE hNote,
                               char FAR * FAR *ppCDBuffer,
                               WORD FAR *pwItemSize,
                               WORD wBufferSize);


STATUS InsertAction(NOTEHANDLE hNote,
                               char FAR * FAR *ppCDBuffer,
                               WORD FAR *pwItemSize,
                               WORD wBufferSize);

STATUS InsertURL(NOTEHANDLE hNote,
                               char FAR * FAR *ppCDBuffer,
                               WORD FAR *pwItemSize,
                               WORD wBufferSize);

STATUS InsertFile(NOTEHANDLE hNote,
                               char FAR * FAR *ppCDBuffer,
                               WORD FAR *pwItemSize,
                               WORD wBufferSize);

STATUS InsertLSButton(NOTEHANDLE hNote,
                               char FAR * FAR *ppCDBuffer,
                               WORD FAR *pwItemSize,
                               WORD wBufferSize);


/*
 *  Body of main program
 */

int main(int argc, char *argv[])
{

    char        szNSFFile[] = "makeform.nsf";
    DHANDLE     hNewNote;
    DBHANDLE    hDbNSFFile;
    STATUS      sError = NOERROR;

    /* Start by calling Notes Init. */

    if (sError = NotesInitExtended (argc, argv))
    {
        PRINTLOG("\n Unable to initialize Notes.\n");
        return (1);
    }


    /* Open the Database, assuming it's been created with the   */
    /* template shipped with this demo.                         */

    if (sError = NSFDbOpen(szNSFFile, &hDbNSFFile))
    {
        PRINTERROR (sError,"NSFDbOpen");
        NotesTerm();
        return (1);
    }

    /* Create a Note */

    if (sError = NSFNoteCreate(hDbNSFFile, &hNewNote))
    {
        NSFDbClose(hDbNSFFile);    /* ERROR - Close database before exit.*/
        PRINTERROR (sError,"NSFNoteCreate");
        NotesTerm();
        return (1);
    }

    /* Add the Form name  */

    if (sError = NSFItemSetText(hNewNote,
                                 "Form",
                                 "Simple Form",
                                 (WORD) strlen("Simple Form")))
    {
        NSFNoteClose(hNewNote);    /* ERROR - Close note before exit.    */
        NSFDbClose(hDbNSFFile);    /* ERROR - Close database before exit.*/
        PRINTERROR (sError,"NSFItemSetText");
        NotesTerm();
        return (1);
    }

    /* Set CompanyName field  */

    if (sError = NSFItemSetText(hNewNote,
                                 "CompanyName",
                                 "HCL",
                                 (WORD) strlen("HCL")))
    {
        NSFNoteClose(hNewNote);    /* ERROR - Close note before exit.    */
        NSFDbClose(hDbNSFFile);    /* ERROR - Close database before exit.*/
        PRINTERROR (sError,"NSFItemSetText");
        NotesTerm();
        return (1);
    }


    /*
     *  Create the richtext item, which will contain objects.
     */

    if (sError = InsertRichText(hNewNote))

    {
        NSFNoteClose(hNewNote);    /* Close note before exit.    */
        NSFDbClose(hDbNSFFile);    /* Close database before exit.*/
        PRINTERROR (sError,"InsertRichText");
        NotesTerm();
        return (1);
    }

    /* write the note */

    if (sError = NSFNoteUpdate(hNewNote, 0))
    {
        NSFNoteClose(hNewNote);    /* ERROR - Close note before exit.    */
        NSFDbClose(hDbNSFFile);    /* ERROR - Close database before exit.*/
        PRINTERROR (sError,"NSFNoteUpdate");
        NotesTerm();
        return (1);
    }

    /* Compile the LotusScript */
    if (sError = NSFNoteLSCompile(hDbNSFFile,hNewNote,0))
    {
        NSFNoteClose(hNewNote);    /* ERROR - Close note before exit.    */
        NSFDbClose(hDbNSFFile);    /* ERROR - Close database before exit.*/
        PRINTERROR (sError,"NSFNoteLSCompile");
        NotesTerm();
        return (1);
    }

    /* write the note */

    if (sError = NSFNoteUpdate(hNewNote, 0))
    {
        NSFNoteClose(hNewNote);    /* ERROR - Close note before exit.    */
        NSFDbClose(hDbNSFFile);    /* ERROR - Close database before exit.*/
        PRINTERROR (sError,"NSFNoteUpdate");
        NotesTerm();
        return (1);
    }

    /* Start cleaning up.  First, Close the Note */
    NSFNoteClose(hNewNote);

    /* Now close the database.              */

    if (sError = NSFDbClose(hDbNSFFile))
    {
        PRINTERROR (sError,"NSFDbClose");
        NotesTerm();
        return (1);
    }


    PRINTLOG ("\nProgram completed successfully.\n");

    /* Return normally.  */
    NotesTerm();
    return (0);

}

/***********************************************************************
 *
 * InsertRichText
 *
 *      Allocate a buffer in which to build the rich text item. Insert a
 *      popup and a button, separated by two CDPARAGRAPH records. Then
 *      append the buffer to the note.
 *
 * Inputs:   hNote   - handle to the new note being created.
 *
 * Outputs:  None.
 *
 ************************************************************************/


STATUS InsertRichText(NOTEHANDLE hNote)

{
    DHANDLE   hMem;
    STATUS    sError;         /* Domino and Notes error status        */
    char FAR  *pCDBuffer;
    char FAR  *pCDBufferStart;
    char      szPopUpContentsString[] = "This is the text of the popup";
    char      szPopUpText[] = "Click Here";
    WORD      wItemSize = 0, wBufferSize = MAXONESEGSIZE;
    int       i=0;

/*
 *  Allocate a buffer and build the CD record containing the popup and
 *  the button.
 */

    if (sError = OSMemAlloc(0, wBufferSize, &hMem))
        return (ERR_MEMORY);

    if ((pCDBuffer = (char far *) OSLockObject(hMem)) == NULL)
        return (ERR_MEMORY);

    memset(pCDBuffer, '\0', wBufferSize);
    pCDBufferStart = pCDBuffer;

/*
 * First insert a popup in the rich text field.
 */

    if (sError = InsertPopup(hNote, &pCDBuffer, &wItemSize, wBufferSize))

    {
        OSUnlockObject(hMem);
        OSMemFree(hMem);
        return (sError);
    }

/*
 *  Now insert two CDPARAGRAPH records.
 */

    for (i=0;i<2;i++) {
       sError = PutPara(&pCDBuffer,
                (WORD) (wBufferSize - (pCDBuffer - pCDBufferStart)));

       if (sError == FALSE)
       {
          OSUnlockObject(hMem);
          OSMemFree(hMem);
          return (sError);
       }
    }

/*
 * Next, insert a button into the rich text field.
 */

    if (sError = InsertButton(hNote, &pCDBuffer, &wItemSize, wBufferSize))
    {
        OSUnlockObject(hMem);
        OSMemFree(hMem);
        return (sError);
    }

/*
 *  Now insert two CDPARAGRAPH records.
 */

    for (i=0;i<2;i++) {
       sError = PutPara(&pCDBuffer,
                (WORD) (wBufferSize - (pCDBuffer - pCDBufferStart)));

       if (sError == FALSE)
       {
          OSUnlockObject(hMem);
          OSMemFree(hMem);
          return (sError);
       }
    }


/*
 * Next, insert a bar into the rich text field.
 */

    if (sError = InsertBar(hNote, &pCDBuffer, &wItemSize, wBufferSize))
    {
        OSUnlockObject(hMem);
        OSMemFree(hMem);
        return (sError);
    }

/*
 *  Now insert two CDPARAGRAPH records.
 */

    for (i=0;i<2;i++) {
       sError = PutPara(&pCDBuffer,
                (WORD) (wBufferSize - (pCDBuffer - pCDBufferStart)));

       if (sError == FALSE)
       {
          OSUnlockObject(hMem);
          OSMemFree(hMem);
          return (sError);
       }
    }


/*
 * Next, insert an action into the rich text field.
 */

    if (sError = InsertAction(hNote, &pCDBuffer, &wItemSize, wBufferSize))
    {
        OSUnlockObject(hMem);
        OSMemFree(hMem);
        return (sError);
    }

/*
 *  Now insert two CDPARAGRAPH records.
 */

    for (i=0;i<2;i++) {
       sError = PutPara(&pCDBuffer,
                (WORD) (wBufferSize - (pCDBuffer - pCDBufferStart)));

       if (sError == FALSE)
       {
          OSUnlockObject(hMem);
          OSMemFree(hMem);
          return (sError);
       }
    }

/*
 * Next, insert a URL into the rich text field.
 */

    if (sError = InsertURL(hNote, &pCDBuffer, &wItemSize, wBufferSize))
    {
        OSUnlockObject(hMem);
        OSMemFree(hMem);
        return (sError);
    }

/*
 *  Now insert two CDPARAGRAPH records.
 */

    for (i=0;i<2;i++) {
       sError = PutPara(&pCDBuffer,
                (WORD) (wBufferSize - (pCDBuffer - pCDBufferStart)));

       if (sError == FALSE)
       {
          OSUnlockObject(hMem);
          OSMemFree(hMem);
          return (sError);
       }
    }

/*
 * Next, insert a file attachment into the rich text field.
 */

    if (sError = InsertFile(hNote, &pCDBuffer, &wItemSize, wBufferSize))
    {
        OSUnlockObject(hMem);
        OSMemFree(hMem);
        return (sError);
    }

/*
 *  Now insert two CDPARAGRAPH records.
 */

    for (i=0;i<2;i++) {
       sError = PutPara(&pCDBuffer,
                (WORD) (wBufferSize - (pCDBuffer - pCDBufferStart)));

       if (sError == FALSE)
       {
          OSUnlockObject(hMem);
          OSMemFree(hMem);
          return (sError);
       }
    }


/*
 *  Next insert a LS button in the rich text field.
 */

    if (sError = InsertLSButton(hNote, &pCDBuffer, &wItemSize, wBufferSize))

    {
        OSUnlockObject(hMem);
        OSMemFree(hMem);
        return (sError);
    }

/*
 *  All done creating the richtext field.  Now append it to the note.
 */

     if (sError = NSFItemAppend(hNote,
                                0,
                                "Body",
                                (WORD) strlen("Body"),
                                TYPE_COMPOSITE,
                                pCDBufferStart,
                                pCDBuffer-pCDBufferStart))

     {
        OSUnlockObject(hMem);
        OSMemFree(hMem);
        return (sError);
     }

    OSUnlockObject(hMem);
    OSMemFree(hMem);

    return (NOERROR);
}


/***********************************************************************
 * InsertPopup
 *                The steps needed to do this are:
 *
 *                - Place a CDPARAGRAPH structure in the buffer;
 *
 *                - Append a CDPABDEFINITION structure in the buffer;
 *
 *                - Append a CDPABREFERENCE structure in the buffer;
 *
 *                - Create an appropriate CDHOTSPOTBEGIN record
 *                  and append it to the buffer. This record will
 *                  contain the text that will be displayed when the
 *                  popup is "popped up".
 *
 *                - Add a CDTEXT record that will comprise the area on which
 *                  to click in order to display the popup.
 *
 *                - Add a CDHOTSPOTEND record to mark the end of the hotspot.
 *
 *
 *
 * Inputs:     hNote        - handle to the new note being created.
 *             ppCDBuffer   - Handle to buffer containing CD record
 *                            being created.
 *             pwItemSize   - current length of item.
 *             wBufferSize  - total size of CD buffer.
 *
 * Outputs:    None.
 *************************************************************************/


STATUS InsertPopup(NOTEHANDLE hNote,
                   char FAR * FAR *ppCDBuffer,
                   WORD FAR *pwItemSize,
                   WORD wBufferSize)

{
    STATUS         sError;
    CDHOTSPOTBEGIN pHotSpotBegin;
    char           szPopUpContentsString[] = "This is the text of the popup";
    char           szPopUpText[] = "Click Here";
    char far      *bufPtr;
/*
 *  Set various flags
 */

    pHotSpotBegin.Type = HOTSPOTREC_TYPE_POPUP;
    pHotSpotBegin.Flags = HOTSPOTREC_RUNFLAG_BEGIN | HOTSPOTREC_RUNFLAG_BOX;

/*
 *  Append data string, set length of data string in structure.
 */

    pHotSpotBegin.DataLength = sizeof(szPopUpContentsString);

/*
 *  Set the header info.
 */

    pHotSpotBegin.Header.Signature = SIG_CD_HOTSPOTBEGIN;
    pHotSpotBegin.Header.Length = ODSLength(_CDHOTSPOTBEGIN)
                                    + pHotSpotBegin.DataLength;



    *pwItemSize += pHotSpotBegin.Header.Length;

    if (*pwItemSize %2)
        *pwItemSize +=1;


    ODSWriteMemory( (void far * far *)ppCDBuffer, _CDHOTSPOTBEGIN, &pHotSpotBegin, 1 );

    memcpy(*ppCDBuffer,
    szPopUpContentsString,
    sizeof(szPopUpContentsString));

    *ppCDBuffer += sizeof(szPopUpContentsString);
    
#if defined W64
    if (((long long)*ppCDBuffer) % 2)
        *ppCDBuffer += 1;
#else
    if (((long)*ppCDBuffer) % 2)
        *ppCDBuffer += 1;
#endif

/*
 * Done with the CDHOTSPOTBEGIN. Now add a CDTEXT item that will deliniate
 * the area on which to click to activate the popup.  This could be
 * a graphic as well as a string.
 */

    bufPtr = *ppCDBuffer;

    sError = PutText(ppCDBuffer, szPopUpText,
        (WORD)(wBufferSize - *pwItemSize), 0);

    if (sError == FALSE)
    {
        return (sError);
    }

    *pwItemSize += *ppCDBuffer - bufPtr;
    bufPtr = *ppCDBuffer;


/*
 * Done with the CDTEXT item.  Now insert a CDHOTSPOTEND item.
 */

    if (sError = PutHotSpotEnd(ppCDBuffer,
                 (WORD)(wBufferSize - *pwItemSize),
                 pwItemSize))
    {
        return (ERR(sError));
    }

    *pwItemSize += *ppCDBuffer - bufPtr;

/*
 *  Done constructing popup.
 */

    return (NOERROR);
}

/*************************************************************************
 * InsertButton
 *      To define the button:
 *
 *      - Append to the CD buffer a CDHOTSPOTBEGIN record, with the
 *        flag member set to HOTSPOTREC_REC_BUTTON, and with the other
 *        data members set appropriately. The formula to be run when
 *        the button is activated is appended to the CDHOTSPOTBEGIN record.
 *
 *      - A CDBUTTON record, which defines the appearance of the button.
 *
 *      - A CDHOTSPOTEND record, to mark the end of the button definition.
 *
 *
 * Inputs:     hNote       - handle to the new note being created.
 *             ppCDBuffer  - handle to buffer containing CD record being
 *                           created.
 *             pwItemSize  - current length of item.
 *             wBufferSize - total size of CD buffer.
 *
 * Outputs:    None.
 *************************************************************************/

STATUS InsertButton(NOTEHANDLE hNote,
                    char FAR * FAR *ppCDBuffer,
                    WORD FAR *pwItemSize,
                    WORD wBufferSize)

{
    STATUS         sError;            /* Domino and Notes error status */
    CDHOTSPOTBEGIN pHotSpotBegin;
    char           szButtonText[] = "Print...";
    char           szButtonFormula[] = "@Command([FilePrint])";
    char far      *bufPtr;
    FORMULAHANDLE  hFormula;
    char          *pFormula;         /* pointer to compiled formula. */
    WORD           wFormulaLen = 0;
    WORD           wdc;              /* "We Don't Care" - We're not interested in some
                                      * of the info passed back by NSFFormulaCompile(),
                                      * but the function call requires the addresses of
                                      * several words to be passed in. The address of
                                      * this word is used for all parameters in which
                                      * we have no interest.
                                      */

    /*
     * Set various flags
     */

    pHotSpotBegin.Type = HOTSPOTREC_TYPE_BUTTON;
    pHotSpotBegin.Flags = HOTSPOTREC_RUNFLAG_BEGIN |
                                  HOTSPOTREC_RUNFLAG_NOBORDER;

/*
 *  Compile the button formula, append compiled formula to
 *  HotSpotBegin data structure.
 */

    sError = NSFFormulaCompile(NULL,
                               0,
                               szButtonFormula,
                               (WORD)strlen(szButtonFormula),
                               &hFormula,
                               &wFormulaLen,
                               &wdc, &wdc, &wdc, &wdc, &wdc);

    if (sError)
    {
        return (ERR(sError));
    }

        pHotSpotBegin.DataLength = wFormulaLen;

/*
 *  Set the header info.
 */

    pHotSpotBegin.Header.Signature = SIG_CD_HOTSPOTBEGIN;
    pHotSpotBegin.Header.Length = ODSLength(_CDHOTSPOTBEGIN)
                                    + pHotSpotBegin.DataLength;


    *pwItemSize += pHotSpotBegin.Header.Length;

    if (*pwItemSize %2)
        *pwItemSize +=1;

    ODSWriteMemory( (void far * far *)ppCDBuffer, _CDHOTSPOTBEGIN, &pHotSpotBegin, 1 );

    pFormula = (char far *) OSLockObject(hFormula);

    memcpy(*ppCDBuffer,
            pFormula,
            wFormulaLen);

    OSUnlockObject(hFormula);    /* unlock and free formula's memory. */

    OSMemFree(hFormula);

    *ppCDBuffer += wFormulaLen;

#if defined W64
    if (((long long)*ppCDBuffer) % 2)
        *ppCDBuffer += 1;
#else
    if (((long)*ppCDBuffer) % 2)
        *ppCDBuffer += 1;
#endif

/*
 * Done with the CDHOTSPOTBEGIN. Now add a CDBUTTON item to define the
 * appearance of the button being inserted.
 */

    bufPtr = *ppCDBuffer;
    if (sError = PutButton(ppCDBuffer,
                      (char *)szButtonText,
                      (WORD)(wBufferSize - *pwItemSize),
                      pwItemSize))
    {
        return (ERR(sError));
    }

    *pwItemSize += *ppCDBuffer - bufPtr;

/*
 * Done with the CDBUTTON item.  Now insert a CDHOTSPOTEND item.
 */

    if (sError = PutHotSpotEnd(ppCDBuffer,
                           (WORD)(wBufferSize - *pwItemSize),
                           pwItemSize))
    {
        return (ERR(sError));
    }

/*
 *  Done defining the button.
 */

    return (NOERROR);
}

/***********************************************************************
 * InsertBar
 *                The steps needed to do this are:
 *
 *                - Create a CDV4HOTSPOTBEGIN record
 *                  and append it to the buffer.
 *
 *                - Add a CDBAR record that will contain the section title
 *                  and other information for the section.
 *
 *                - Add an empty CDTEXT record
 *
 *                - Add a CDPARAGRAPH structure to the buffer;
 *
 *                - Append a CDPABREFERENCE structure in the buffer;
 *
 *                - Add a CDTEXT record that will contain the contents of the
 *                  section.
 *
 *                - Add a CDV4HOTSPOTEND record to mark the end of the hotspot.
 *
 *
 *
 * Inputs:     hNote       - handle to the new note being created.
 *             ppCDBuffer  - Handle to buffer containing CD record being
 *                           created.
 *             pwItemSize  - current length of item.
 *             wBufferSize - total size of CD buffer.
 *
 * Outputs:    None.
 *************************************************************************/


STATUS InsertBar(NOTEHANDLE hNote,
                 char FAR * FAR *ppCDBuffer,
                 WORD FAR *pwItemSize,
                 WORD wBufferSize)

{
    STATUS         sError;
    CDHOTSPOTBEGIN HotSpotBegin;
    char           szBarString[] = "This is my section title";
    char           szSectionText[] = "This is text in my section";
    char far      *bufPtr;
    char           eString[] = "";

/*
 *  Set various flags
 */

    HotSpotBegin.Type = HOTSPOTREC_TYPE_BUNDLE;
    HotSpotBegin.Flags = HOTSPOTREC_RUNFLAG_BEGIN | HOTSPOTREC_RUNFLAG_NOBORDER;

/*
 *  Append data string, set length of data string in structure.
 */

    HotSpotBegin.DataLength = 0;

/*
 *  Set the header info.
 */

    HotSpotBegin.Header.Signature = SIG_CD_V4HOTSPOTBEGIN;
    HotSpotBegin.Header.Length = ODSLength(_CDHOTSPOTBEGIN);



    *pwItemSize += HotSpotBegin.Header.Length;

    if (*pwItemSize %2)
        *pwItemSize +=1;


    ODSWriteMemory( (void far * far *)ppCDBuffer, _CDHOTSPOTBEGIN, &HotSpotBegin, 1 );

    bufPtr = *ppCDBuffer;

/*
 * Done with the CDHOTSPOTBEGIN. Now add a CDBAR item to define the
 * appearance of the bar being inserted.
 */

    if (sError = PutBar(ppCDBuffer,
                      (char *)szBarString,
                      (WORD)(wBufferSize - *pwItemSize),
                      pwItemSize))
    {
        return (ERR(sError));
    }

    *pwItemSize += *ppCDBuffer - bufPtr;
    bufPtr = *ppCDBuffer;

    sError = PutText(ppCDBuffer, eString, (WORD)(wBufferSize - *pwItemSize), 0);

    if (sError == FALSE)
    {
        return (sError);
    }

    sError = PutPara(ppCDBuffer,(WORD) (wBufferSize - *pwItemSize));

    if (sError == FALSE)
    {
        return (sError);
    }

/*
 * Then write a CDPABREFERENCE
 */

    if (sError = PutPabRef(ppCDBuffer,
                           (WORD)1,
                           (WORD)(wBufferSize - *pwItemSize),
                           pwItemSize))
    {
        return (sError);
    }

    sError = PutText(ppCDBuffer, szSectionText, (WORD)(wBufferSize - *pwItemSize), 0);

    if (sError == FALSE)
    {
        return (sError);
    }

/*
 * Done with the CDTEXT item.  Now insert a CDHOTSPOTEND item.
 */

    if (sError = PutV4HotSpotEnd(ppCDBuffer,
                           (WORD)(wBufferSize - *pwItemSize),
                           pwItemSize))
    {
        return (ERR(sError));
    }

    *pwItemSize += *ppCDBuffer - bufPtr;

/*
 *  Done constructing popup.
 */

    return (NOERROR);
}

/***********************************************************************
 * InsertAction
 *                The steps needed to do this are:
 *
 *                - Create an appropriate CDV4HOTSPOTBEGIN record
 *                  and append it to the buffer.
 *
 *                - Add a TYPE_ACTION word to the buffer.
 *                - Add a CDACTIONHEADER structure to the buffer.
 *                - Add a CDACTIONSENDMAIL structure to the buffer..
 *                - Add a CDTEXT record that will contain the URL address
 *                - Add a CDV4HOTSPOTEND record to mark the end of the hotspot.
 *
 *
 * Inputs:     hNote       - handle to the new note being created.
 *             ppCDBuffer  - Handle to buffer containing CD record being
 *                           created.
 *             pwItemSize  - current length of item.
 *             wBufferSize - total size of CD buffer.
 *
 * Outputs:    None.
 *************************************************************************/


STATUS InsertAction(NOTEHANDLE hNote,
                    char FAR * FAR *ppCDBuffer,
                    WORD FAR *pwItemSize,
                    WORD wBufferSize)

{
    STATUS         sError;
    CDHOTSPOTBEGIN HotSpotBegin;
    WORD           ActionType = TYPE_ACTION;
    WORD           wToLength, wSubjectLength, wBodyLength;
    char           far *bufPtr;
    char           szTitleText[] = "This is an action that sends a message";

    /* ToInfo contains the name of the person the action will send the
       mail to. */
    char           ToInfo[] = "Jan Doe/CAM/Lotus";

    char           SubjectField[] = "This is from an API program";
    char           BodyField[] = "This is the body of the message";

/*
 *  Set various flags
 */

    HotSpotBegin.Type = HOTSPOTREC_TYPE_HOTREGION;
    HotSpotBegin.Flags = HOTSPOTREC_RUNFLAG_BEGIN | HOTSPOTREC_RUNFLAG_ACTION
                         | HOTSPOTREC_RUNFLAG_BOX;

/*
 *  Set data length
 */

    /* calculate string lengths and add the null, and if odd add padding byte */
    wToLength = strlen(ToInfo) + 1;
    if (wToLength % 2)
        wToLength += 1;

    wSubjectLength = strlen(SubjectField) + 1;
    if (wSubjectLength % 2)
        wSubjectLength += 1;

    wBodyLength = strlen(BodyField) + 1;
    if (wBodyLength % 2)
        wBodyLength += 1;

    HotSpotBegin.DataLength = 2 + (BYTE)ODSLength(_CDACTIONHEADER) +
                             ODSLength(_CDACTIONSENDMAIL) + wToLength + wSubjectLength
                             + wBodyLength;
/*
 *  Set the header info.
 */

    HotSpotBegin.Header.Signature = SIG_CD_V4HOTSPOTBEGIN;
    HotSpotBegin.Header.Length = ODSLength(_CDHOTSPOTBEGIN) +
                                 HotSpotBegin.DataLength;


    *pwItemSize += HotSpotBegin.Header.Length;

    if (*pwItemSize %2)
        *pwItemSize +=1;


    ODSWriteMemory( (void far * far *)ppCDBuffer, _CDHOTSPOTBEGIN, &HotSpotBegin, 1 );


    bufPtr = *ppCDBuffer;

/*
 * Done with the CDHOTSPOTBEGIN. Now add the action
 */

    if (sError = PutAction(ppCDBuffer,
                          (char *)ToInfo,
                          (char *)SubjectField,
                          (char *)BodyField,
                          (WORD)(wBufferSize - *pwItemSize),
                          pwItemSize))
    {
        return (ERR(sError));
    }

    *pwItemSize += *ppCDBuffer - bufPtr;

    sError = PutText(ppCDBuffer, szTitleText, (WORD)(wBufferSize - *pwItemSize),0);

    if (sError == FALSE)
    {
        return (sError);
    }

/*
 * Done with the CDTEXT item. Now insert a CDHOTSPOTEND item.
 */

    if (sError = PutV4HotSpotEnd(ppCDBuffer,
                           (WORD)(wBufferSize - *pwItemSize),
                           pwItemSize))
    {
        return (ERR(sError));
    }

    *pwItemSize += *ppCDBuffer - bufPtr;

/*
 *  Done constructing Action.
 */

    return (NOERROR);
}


/***********************************************************************
 * InsertURL
 *                The steps needed to do this are:
 *
 *                - Create a CDV4HOTSPOTBEGIN record
 *                  and append it to the buffer.
 *
 *                - Add a CDTEXT record that will contain the URL address
 *                - Add a CDV4HOTSPOTEND record to mark the end of the hotspot.
 *
 *
 *
 * Inputs:     hNote       - handle to the new note being created.
 *             ppCDBuffer  - Handle to buffer containing CD record being
 *                           created.
 *             pwItemSize  - current length of item.
 *             wBufferSize - total size of CD buffer.
 *
 * Outputs:    None.
 *************************************************************************/


STATUS InsertURL(NOTEHANDLE hNote,
                 char FAR * FAR *ppCDBuffer,
                 WORD FAR *pwItemSize,
                 WORD wBufferSize)

{
    STATUS         sError;
    CDHOTSPOTBEGIN HotSpotBegin;
    char far       *bufPtr;
    char           szURL[] = "http://www.lotus.com";
/*
 *  Set various flags
 */

    HotSpotBegin.Type = HOTSPOTREC_TYPE_HOTLINK;
    HotSpotBegin.Flags = HOTSPOTREC_RUNFLAG_BEGIN |
                         HOTSPOTREC_RUNFLAG_NOBORDER |
                         HOTSPOTREC_RUNFLAG_INOTES;

/*
 *  Set data length
 */

    HotSpotBegin.DataLength = sizeof(szURL);

/*
 *  Set the header info.
 */

    HotSpotBegin.Header.Signature = SIG_CD_V4HOTSPOTBEGIN;
    HotSpotBegin.Header.Length = ODSLength(_CDHOTSPOTBEGIN) +
                                 HotSpotBegin.DataLength;



    *pwItemSize += HotSpotBegin.Header.Length;

    if (*pwItemSize %2)
        *pwItemSize +=1;

    bufPtr = *ppCDBuffer;

    ODSWriteMemory( (void far * far *)ppCDBuffer, _CDHOTSPOTBEGIN, &HotSpotBegin, 1 );

    memcpy(*ppCDBuffer, szURL, sizeof(szURL));

    *ppCDBuffer += sizeof(szURL);

#if defined W64
    if (((long long)*ppCDBuffer) % 2)
        *ppCDBuffer += 1;
#else
    if (((long)*ppCDBuffer) % 2)
        *ppCDBuffer += 1;
#endif

    sError = PutText(ppCDBuffer, szURL,
        (WORD)(wBufferSize - *pwItemSize), HOTSPOTREC_TYPE_HOTLINK);

    if (sError == FALSE)
    {
        return (sError);
    }

/*
 * Done with the CDTEXT item.  Now insert a CDHOTSPOTEND item.
 */

    if (sError = PutV4HotSpotEnd(ppCDBuffer,
                           (WORD)(wBufferSize - *pwItemSize),
                           pwItemSize))
    {
        return (ERR(sError));
    }

    *pwItemSize += *ppCDBuffer - bufPtr;

/*
 *  Done constructing URL.
 */

    return (NOERROR);
}


/***********************************************************************
 * InsertFile
 *                The steps needed to do this are:
 *
 *                - Call NSFNoteAttachFile to create a $FILE item.
 *
 *                - Create an appropriate CDHOTSPOTBEGIN record
 *                  and append it to the buffer.
 *
 *                - Add a CDTEXT record that will contain the name of the
 *                  attached file.
 *
 *                - Add a CDHOTSPOTEND record to mark the end of the hotspot.
 *
 *
 * Inputs:     hNote       - handle to the new note being created.
 *             ppCDBuffer  - Handle to buffer containing CD record being
 *                           created.
 *             pwItemSize  - current length of item.
 *             wBufferSize - total size of CD buffer.
 *
 * Outputs:    None.
 *************************************************************************/


STATUS InsertFile(
        NOTEHANDLE hNote,
        char FAR * FAR *ppCDBuffer,
        WORD FAR *pwItemSize,
        WORD wBufferSize)
{
    STATUS         sError;
    CDHOTSPOTBEGIN HotSpotBegin;
    char far       *bufPtr;
#if defined(UNIX) 
    char szSourceFileName[] =
        ".//readme.txt";

#elif defined(OS400)
    char szSourceFileName[] =
        "/myserver/notes/data/readme.txt";
#elif defined(MAC)
    char szSourceFileName[] =
        ".//readme.txt";
#else
    char szSourceFileName[] =
        "e:\\readme.txt";
#endif

    char szAttachLinkName[] = "attach.lnk";
    char szAttachFileName[] = "readme.txt";

    if (sError = NSFNoteAttachFile( hNote,
                                    ITEM_NAME_ATTACHMENT,
                                    (WORD) strlen(ITEM_NAME_ATTACHMENT),
                                    szSourceFileName,
                                    szAttachLinkName,
                                    EFLAGS_INDOC | HOST_LOCAL | COMPRESS_NONE))
    {
        return(ERR(sError));
    }

/*
 *  Set various flags
 */

    HotSpotBegin.Type = HOTSPOTREC_TYPE_FILE;
    HotSpotBegin.Flags = HOTSPOTREC_RUNFLAG_BEGIN | HOTSPOTREC_RUNFLAG_NOBORDER;

/*
 *  Set data length
 */

    HotSpotBegin.DataLength = strlen(szAttachLinkName) + strlen(szAttachFileName) + 2;

/*
 *  Set the header info.
 */

    HotSpotBegin.Header.Signature = SIG_CD_HOTSPOTBEGIN;
    HotSpotBegin.Header.Length = ODSLength(_CDHOTSPOTBEGIN) + HotSpotBegin.DataLength;

    *pwItemSize += HotSpotBegin.Header.Length;

    if (*pwItemSize %2)
        *pwItemSize +=1;

    bufPtr = *ppCDBuffer;

    ODSWriteMemory( (void far * far *)ppCDBuffer, _CDHOTSPOTBEGIN, &HotSpotBegin, 1 );

    memcpy(*ppCDBuffer, szAttachLinkName, sizeof(szAttachLinkName));
    *ppCDBuffer += sizeof(szAttachLinkName);

    memcpy(*ppCDBuffer, szAttachFileName, sizeof(szAttachFileName));
    *ppCDBuffer += sizeof(szAttachFileName);

#if defined W64
    if (((long long)*ppCDBuffer) % 2)
        *ppCDBuffer += 1;
#else
    if (((long)*ppCDBuffer) % 2)
        *ppCDBuffer += 1;
#endif

    sError = PutText(ppCDBuffer, szAttachFileName, (WORD)(wBufferSize - *pwItemSize), 0);

    if (sError == FALSE)
    {
        return (sError);
    }

/*
 * Done with the CDTEXT item.  Now insert a CDHOTSPOTEND item.
 */

    if (sError = PutHotSpotEnd(ppCDBuffer,
                              (WORD)(wBufferSize - *pwItemSize),
                              pwItemSize))
    {
        return (ERR(sError));
    }

    *pwItemSize += *ppCDBuffer - bufPtr;

/*
 *  Done constructing file attachment.
 */

    return (NOERROR);
}


/*************************************************************************
 * InsertLSButton
 *      To define the LS button:
 *
 *      - Append to the CD buffer a CDBEGINRECORD record.
 *
 *      - A CDHOTSPOTBEGIN record, with the
 *        flag member set to HOTSPOTREC_REC_BUTTON, and with the other
 *        data members set appropriately. The LotusScript to be run when
 *        the button is activated is appended to the CDHOTSPOTBEGIN record.
 *
 *      - A CDBUTTON record, which defines the appearance of the button.
 *
 *      - A CDHOTSPOTEND record.
 *
 *      - A CDENDRECORD record, to mark the end of the button definition.
 *
 *
 * Inputs:     hNote       - handle to the new note being created.
 *             ppCDBuffer  - handle to buffer containing CD record being
 *                           created.
 *             pwItemSize  - current length of item.
 *             wBufferSize - total size of CD buffer.
 *
 * Outputs:    None.
 *************************************************************************/

STATUS InsertLSButton(NOTEHANDLE hNote,
                    char FAR * FAR *ppCDBuffer,
                    WORD FAR *pwItemSize,
                    WORD wBufferSize)

{
    STATUS              sError=NOERROR;  /* Domino and Notes error status  */
    CDHOTSPOTBEGIN      pHotSpotBegin;
    CDBEGINRECORD       pBeginRecord;
    CDENDRECORD         pEndRecord;
    char far           *bufPtr;
    char                szButtonText[]="LS Button";

    char               *pFormattedLS;
    int                 FormattedLSLen;

    int                 sourceAllocated=0;
    int                 destAllocated=0;
    int                 contextAllocated=0;
    int                 errorBufferAllocated=0;

    DHANDLE             hSource=NULL;
    DHANDLE             hDest=NULL;
    DHANDLE             hErrorBuffer=NULL;
    DHANDLE             hData=NULLHANDLE;
    SCRIPTCONTEXTDESCR *pSCD;
    
    static char         szScript[] = "Sub Click(Source As Button)\n\
                                     \tMsgbox(\"I love LotusScript.\")\n\
                                      End Sub\n";


    /* Following steps will convert raw Lotus Script to a format
       that IDE uses when rendering the script. */

    /* Be sure to allocate enough memory for the script and the 
       null terminator. */
    if (sError = OSMemAlloc(0,strlen(szScript)+1,&hSource))
       return(ERR_MEMORY);
    sourceAllocated=1;

    /* Copy the raw Lotus Script into the newly allocated memory 
       space. */
    pFormattedLS=OSLock(char,hSource);
    strcpy(pFormattedLS,szScript);
    OSUnlock(hSource);

    /* Set up SCRIPTCONTEXTDESCR */
    if (sError=OSMemAlloc(0,sizeof(SCRIPTCONTEXTDESCR),&hData))
       goto Exit1;
    contextAllocated=1;
    pSCD=OSLock(SCRIPTCONTEXTDESCR,hData);
    pSCD->Length=sizeof(SCRIPTCONTEXTDESCR);
    strcpy(pSCD->szNameOfContextClass,"BUTTON");
    OSUnlock(hData);
      
    /* Convert the raw Lotus Script to IDE compliant format. */
    sError = AgentLSTextFormat(hSource,&hDest, &hErrorBuffer,0,&hData);

    /* Free hSource handle, and update memory allocation flags for
       hSource, hDest and hErrorBuffer handles. */
    OSMemFree(hSource);
    sourceAllocated=0;
    destAllocated=1;
    errorBufferAllocated=1;     

    if (sError )
    { 
       PRINTLOG("Error: Error in AgentLSTextFormat.\n");
       goto Exit1;
    }

    /* If any script error, retrieve the error text from hErrorBuffer 
       handle; otherwise, retrieve the IDE compliant script. */
    if (hErrorBuffer)
    {
       pFormattedLS=OSLock(char,hErrorBuffer);
       PRINTLOG("\nError from AgentLSTextFormat: %s\n",pFormattedLS);
       OSUnlock(hErrorBuffer);
       sError=1;
       goto Exit1;
    }
    else if (hDest)
    {
       OSMemFree(hErrorBuffer);
       errorBufferAllocated=0;
       pFormattedLS=OSLock(char,hDest);

       /* When saving the formatted LS in the buffer,
          the script should be ended with a null terminator, therefore, 
          we'll add one to the strlen(pFormattedLS) and save it in the 
          FormattedLSLen variable. */

       FormattedLSLen=strlen(pFormattedLS)+1;
    }
   
/*
 * write CDBEGINRECORD
 */
    pBeginRecord.Signature = SIG_CD_V4HOTSPOTBEGIN;
    pBeginRecord.Version = 0;
    pBeginRecord.Header.Signature = (BYTE)SIG_CD_BEGIN;
    pBeginRecord.Header.Length = (BYTE)ODSLength(_CDBEGINRECORD);

    *pwItemSize += pBeginRecord.Header.Length;

    if (*pwItemSize %2)
        *pwItemSize +=1;

    ODSWriteMemory( (void far * far *)ppCDBuffer, _CDBEGINRECORD, &pBeginRecord, 1 );

/*
 * write CDHOTSPOTBEGIN
 */

/*
 * Set various flags
 */

    pHotSpotBegin.Type = HOTSPOTREC_TYPE_BUTTON;
    pHotSpotBegin.Flags = HOTSPOTREC_RUNFLAG_BEGIN |
                          HOTSPOTREC_RUNFLAG_NOBORDER |
                          HOTSPOTREC_RUNFLAG_SCRIPT ;

    pHotSpotBegin.DataLength = FormattedLSLen;

/*
 *  Set the header info.
 */

    pHotSpotBegin.Header.Signature = SIG_CD_V4HOTSPOTBEGIN;
    pHotSpotBegin.Header.Length = ODSLength(_CDHOTSPOTBEGIN)
                                    + pHotSpotBegin.DataLength;


    *pwItemSize += pHotSpotBegin.Header.Length;

    if (*pwItemSize %2)
        *pwItemSize +=1;

    ODSWriteMemory( (void far * far *)ppCDBuffer, _CDHOTSPOTBEGIN, &pHotSpotBegin, 1 );

/*
 * write button content.
 */

    memcpy(*ppCDBuffer,
            pFormattedLS,
            FormattedLSLen);


    *ppCDBuffer += FormattedLSLen;

#if defined W64
    if (((long long)*ppCDBuffer) % 2)
        *ppCDBuffer += 1;
#else
    if (((long)*ppCDBuffer) % 2)
        *ppCDBuffer += 1;
#endif

/*
 * Done with the CDHOTSPOTBEGIN. Now add a CDBUTTON item to define the
 * appearance of the button being inserted.
 */

    bufPtr = *ppCDBuffer;
    if (sError = PutButton(ppCDBuffer,
                      (char *)szButtonText,
                      (WORD)(wBufferSize - *pwItemSize),
                      pwItemSize))
       goto Exit1;

    *pwItemSize += *ppCDBuffer - bufPtr;

/*
 * Done with the CDBUTTON item.  Now insert a CDHOTSPOTEND item.
 */

    if (sError = PutV4HotSpotEnd(ppCDBuffer,
                           (WORD)(wBufferSize - *pwItemSize),
                           pwItemSize))
       goto Exit1;

/*
 * write CDENDRECORD
 */
    pEndRecord.Signature = SIG_CD_V4HOTSPOTEND;
    pEndRecord.Version = 0;
    pEndRecord.Header.Signature = (BYTE)SIG_CD_END;
    pEndRecord.Header.Length = (BYTE)ODSLength(_CDENDRECORD);

    *pwItemSize += pEndRecord.Header.Length;

    if (*pwItemSize %2)
        *pwItemSize +=1;

    ODSWriteMemory( (void far * far *)ppCDBuffer, _CDENDRECORD, &pEndRecord, 1 );

Exit1:
    if (sourceAllocated)
       OSMemFree(hSource);

    if (destAllocated)
    {
       OSUnlock(hDest);
       OSMemFree(hDest);
    }

    if (contextAllocated)
    {
       OSMemFree(hData);
    }

    if (errorBufferAllocated)
       OSMemFree(hErrorBuffer);

/*
 *  Done defining the button.
 */

    return (sError);
}

