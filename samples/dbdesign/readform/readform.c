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

    PROGRAM:   readform

    FILE:      readform.c

    PURPOSE:   Shows how to read a form note and the fields in it.

    DESCRIPTION:

    This reads a form note, and lists all of its components. The form
    document has three main components:

       $TITLE item:  This is a item of type TYPE_TEXT.  It specifies the
                     name of the form.
       $INFO  item:  This is an item of type TYPE_COMPOSITE, containing
                     a CDDOCUMENT item.
       $BODY  item:  This is an item of type TYPE_COMPOSITE, which contains
                     a CDFIELD item for each field defined in the form. In 
                     addition this program has been enhanced to support R5 
                     Subforms and associated Subform fields that may be 
                     contained within the form's $BODY.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

#ifdef __cplusplus
extern "C" {
#endif
/*
 *  C header files
 */

#include <stdio.h>
#include <string.h>

/*
 *  HCL C API for Notes/Domino Header files.
 */

#include <global.h>
#include <nsfdata.h>
#include <nif.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <editods.h>
#include <osmem.h>
#include <osmisc.h>     /*  for OSLoadString */
#include <stdnames.h>
#include <nsferr.h>
#include "../../printLog.h"

/*
 *  Local header files
 */

#include "readform.h"

/*
 *  Global variables
 */

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

DBHANDLE       hDB;                       /* database handle */
char far       *pOutputBuffer;            /* Buffer to hold output strings. */
STATUS         sError=0;                  /* return code */

/************************************************************************

  FUNCTION:   main C API for Domino and Notes subroutine

  ALGORITHM:

    - READFORM first opens the database READFORM.NSF.  It finds the
      NoteID of the form note for the form called "Test Form 1" by
      calling NIFFindDesignNote.  READFORM then opens the form note.

    - The title of the database is obtained by calling NSFItemInfo(),
      locking the block of memory returned, and copying the title to a
      buffer to be displayed later. (The database title is explicitly
      defined in this program, so this call was not really needed, but
      it demonstrates the NSFItemInfo() function).

    - The program then calls NSFItemInfo() and NSFItemInfoNext() to get the
      blocks of data describing the fields in this form.

    - For each block of data READFORM calls ReadBody() to get the names
      and datatypes of each field in the form.  ReadBody() calls
      EnumCompositeBuffer(), passing the data block returned by the
      NSFItemInfo() and NSFItemInfoNext() calls above.  For each Composite
      Data record, EnumCompositeBuffer() will invoke the action routine
      FormFields().

    - The action routine FormFields() reads the name of each field and
      its datatype, and appends the name and type to a character buffer.

    - Additionally, the action routine has been enhanced to read Subforms 
      for Version R5.  A Subform is not contained in a CDFIELD record but 
      within a CD_V4HOTSPOTBEGIN record.  See the FormFields() routine.

    - When all fields (including any Subforms and associated fields) in the
      form have been processed, the database title, the field names, and the 
      datatypes are displayed.

*************************************************************************/
 
int main(int argc, char *argv[])
{

    char szFileName[] = "readform.nsf";
   char FormName[] = "Test Form 1";

    /* Domino and Notes initialization */
    if (sError = NotesInitExtended (argc, argv))
    {
        PRINTLOG("\n Unable to initialize Notes.\n");
        return (1);
    }
      
    PRINTLOG("");
    fflush(stdout);

    /*
     * Now open the database, get the NoteID of the Form note, and open the
     * Form note.
     */

    if (sError = NSFDbOpen(szFileName, &hDB))
    {
        PRINTERROR (sError, "NSFDbOpen");  
        NotesTerm();
        return (1);
    }

    /* read the initial form */
    sError = ReadForm(FormName);
  
    /* if no error close the database */
    if (!sError)
    {
        NSFDbClose(hDB);
    }
    else 
    {
        PRINTERROR (sError, "ReadForm");  
        NotesTerm();
        return (1);
    }
  
    /*
     * leave with no error
     */
    PRINTLOG("\nProgram completed successfully.\n");   
    NotesTerm();
    return (0); 

}

/************************************************************************

    FUNCTION:    ReadForm

    PURPOSE:     Finds the form if it exists in the database and calls 
                 NSFItemInfo and ReadBody routines to get the names
                 and datatypes of all fields in a form.

*************************************************************************/

STATUS LNPUBLIC ReadForm(char *pFormName)
{
    NOTEHANDLE  hNote; 
    DHANDLE     hMem;
    WORD        ClassForm = NOTE_CLASS_FORM;
    WORD        wDataType;
    DWORD       dwLength;
    DWORD       dwTextBufferLength = TEXT_BUFFER_LENGTH;
    NOTEID      FormNoteID;           /* Note id of the Form document. */
    BLOCKID     ItemBlockID;
    BLOCKID     prevItemBlockID;
    BLOCKID     ValueBlockID;
    char far    *pData=NULL;
    char        szTitleString[128];   /* Form title will be constructed here.  */

    /*  
     * find the design note form
     */
 
    if (sError = NIFFindDesignNote(hDB, pFormName, NOTE_CLASS_FORM, 
                                   &FormNoteID))
    {
        NSFDbClose(hDB);
        return(ERR(sError));
    }

    /*  
     * open the design note
     */

    if (sError = NSFNoteOpen(hDB, FormNoteID, 0, &hNote))
    {
        NSFDbClose(hDB);
        return(ERR(sError));
    }

    /*
     * Allocate a buffer in which the field names and datatypes will
     * be displayed.
     */

    if (sError = OSMemAlloc (0, dwTextBufferLength, &hMem))
        return(FALSE);
        
    /*
     *  Lock global buffer.
     */
    
    if ((pOutputBuffer = (char*) OSLockObject(hMem)) == NULL)
        return(FALSE);

    /*
    * Read the title of the form from the Form note.
    * Note: $TITLE item is TYPE_TEXT.
    */
 
    if (sError = NSFItemInfo(hNote,
                             ITEM_NAME_TEMPLATE_NAME,
                             sizeof(ITEM_NAME_TEMPLATE_NAME) - 1,
                             NULL,
                             &wDataType,
                             &ValueBlockID,
                             &dwLength))
    {
        OSUnlockObject(hMem);
        OSMemFree (hMem);
        NSFDbClose(hDB);
        return(ERR(sError));
    }

    /*
     *  Lock the block returned, write title of form to the output buffer.
     */

    pData = OSLockBlock(char, ValueBlockID);
    pData += sizeof(WORD);

    memcpy(szTitleString, pData, (int) (dwLength - sizeof(WORD)));
    szTitleString[dwLength - sizeof(WORD)] = '\0';    
    PRINTLOG (pOutputBuffer, "\nForm Name:  %s\n\n", szTitleString);
    OSUnlockBlock(ValueBlockID);
      
    /*
     *  Now read the Body of the form. Since there may be multiple body fields,
     *  we'll call NSFItemInfo() to get the first one, then we'll call
     *  NSFItemInfoNext() in a loop to get the remaining body fields.
     */       

    if (sError = NSFItemInfo(hNote,
                             ITEM_NAME_TEMPLATE,
                             sizeof(ITEM_NAME_TEMPLATE) - 1, 
                             &ItemBlockID,
                             &wDataType,
                             &ValueBlockID,
                             &dwLength))
    {
        OSUnlockObject(hMem);
        OSMemFree (hMem);
        NSFDbClose(hDB);
        return(ERR(sError));
    }

  /*
   *  Call function to read field names and datatypes from the body field.
   */
 
    if (sError = ReadBody(ValueBlockID, dwLength, pOutputBuffer))
    {
        OSUnlockObject(hMem);
        OSMemFree (hMem);
        NSFDbClose(hDB);
        return(ERR(sError));
    }

    /*
     *  Read the remaining Body fields in the form note.
     */  
 
    while (TRUE)
    {
        prevItemBlockID = ItemBlockID;
        sError = NSFItemInfoNext(hNote,
                                 prevItemBlockID,
                                 ITEM_NAME_TEMPLATE,
                                 sizeof(ITEM_NAME_TEMPLATE) - 1,
                                 &ItemBlockID,
                                 &wDataType,
                                 &ValueBlockID,
                                 &dwLength);

    /*
     *  A return status of ERR_ITEM_NOT_FOUND is not an error, it just
     *  indicates that there are no more items to find.  If this is the
     *  case, set sError to NOERROR and break out of loop.
     */
                                 
    if (sError == ERR_ITEM_NOT_FOUND)
    {
        sError = NOERROR;
        break;
    }

    /*
     *  If any other errorcode except NOERROR is returned,
     *  break out of loop. 
     */

    else if (sError)
        break;    

    /*
     *  Call function to read the field names and datatypes from the
     *  body field. If an error is returned, break out of loop.
     */
     
    if ((sError = ReadBody(ValueBlockID,
                           dwLength, pOutputBuffer)) != NOERROR)
        break;

    }  /* End of "while (TRUE)" loop */

  /*
   *  If an error occurred in the above loop, display an error message,
   *  clean up, and exit.
   */    

    if (sError)
    {
        OSUnlockObject(hMem);
        OSMemFree (hMem);
        NSFDbClose(hDB);
        return(ERR(sError));
    }

    /*
     *  Display the form title, along with the names and data types of all the
     *  fields in this form.
     */

    PRINTLOG("%s", pOutputBuffer);
    
    /*
     *  Close the note and the database.
     */
 
    OSUnlockObject(hMem);
    OSMemFree (hMem);
    NSFNoteClose(hNote);
  
    /*
     * leave with no error
     */
    return(NOERROR);
}

/************************************************************************

    FUNCTION:    ReadBody

    PURPOSE:     Calls EnumCompositeBuffer to get the  names and datatypes
                 of all fields in a form.

*************************************************************************/

STATUS LNPUBLIC ReadBody(
BLOCKID ValueBlockID,
DWORD dwLength,
char far *pOutputBuffer)
{

  /*
   * For each CD record found in the Body of the Form note,
   * EnumCompositeBuffer will invoke the action routine FormFields.
   */

  sError = EnumCompositeBuffer(ValueBlockID,
                               dwLength,
                               (ActionRoutinePtr)&FormFields,
                               pOutputBuffer);
  return(sError);
}

/************************************************************************

    FUNCTION:    FormFields

    PURPOSE:     Action routine invoked by EnumCompositeBuffer.

    COMMENTS:
        For each field in a form, the $Body field of the Form note will
        contain a record of type CDFIELD, which defines that field. If
        the current record is a CDFIELD record, this routine will extract
        the field name and data type, and write these to a global buffer.
        
        For version R5 this routine has been enhanced to read Subforms
        within a form's body.  A Subform is detected by a
        SIG_CD_V4HOTSPOTBEGIN record and is not in a CDFIELD record.

*************************************************************************/
STATUS NOTESCALLBACK FormFields(
char NOTESPTR        RecordPtr,
WORD                        RecordType,                    
DWORD                        dwRecordLength,                
void far        *pCtx)                                
{

    void FAR       *p = (void *)RecordPtr;
    CDFIELD        CDField;
    CDHOTSPOTBEGIN CDV4HotSpot;
    CDHOTSPOTBEGIN *pCDV4HotSpot=NULL;
    NOTEHANDLE     hNote;
    DHANDLE        hMem;
    NOTEID         SubFormNoteID;
    BLOCKID        ValueBlockID, PrevBlockID, NewBlockID;
    DWORD          dwLength;
    WORD           wDataType;
    WORD           binDataType;

    char           FieldString[256]={0};
    char           szFieldName[128]={0};
    char           szDataType[128]={0};
    char far       *pFieldName=NULL;
    char           *pBuf = (char *)pCtx;
  
    void           *pItemValue=NULL;

    sError = 0;

    /*
     * if the record type is a SIG_CD_V4HOTSPOTBEGIN check to see if the type 
     * is a Subform.
     */

    if (RecordType == SIG_CD_V4HOTSPOTBEGIN)
    {
        ODSReadMemory( &p, _CDHOTSPOTBEGIN, &CDV4HotSpot, 1);
        pCDV4HotSpot = (CDHOTSPOTBEGIN *)&CDV4HotSpot;

        /*
         * if the type is a Subform, find the Subform design note, get it's info, 
         * call EnumCompositeBuffer() to make this the new action routine so we can
         * recursively come in here and detect any nested Subforms.
         */

        if (pCDV4HotSpot->Type == HOTSPOTREC_TYPE_SUBFORM &&
            !(pCDV4HotSpot->Flags & HOTSPOTREC_RUNFLAG_FORMULA))
        {
            char * pszSubForm=NULL;

            if (sError = OSMemAlloc(0,pCDV4HotSpot->DataLength+1,&hMem))
                return sError;

            pszSubForm = (char *) OSLockObject(hMem);
            strncpy(pszSubForm,(char *)p,pCDV4HotSpot->DataLength);

            sError = NIFFindDesignNote(hDB, pszSubForm, NOTE_CLASS_FORM, &SubFormNoteID);

            OSUnlockObject(hMem);
            OSMemFree(hMem);
        
            if (sError)
                return sError;

            if (sError = NSFNoteOpen(hDB, SubFormNoteID, 0, &hNote))
                return sError;
      
          /* Now read the Body of the SubForm form */        
            if (sError = NSFItemInfo(hNote, ITEM_NAME_TEMPLATE,
                                     (WORD)strlen(ITEM_NAME_TEMPLATE), &NewBlockID,
                                     &wDataType, &ValueBlockID, &dwLength))
            {
                NSFNoteClose (hNote);
                return sError;
            }

            /* keep calling this action routine until no more Subforms... */
            while ( TRUE )
            {  
                PrevBlockID = NewBlockID;
                if (sError = EnumCompositeBuffer(ValueBlockID, dwLength, 
                                                 &FormFields, pOutputBuffer))
                {
                    NSFNoteClose ( hNote );
                    return NOERROR;
                }

                sError = NSFItemInfoNext (hNote, PrevBlockID, ITEM_NAME_TEMPLATE,
                                          (WORD)strlen(ITEM_NAME_TEMPLATE), &NewBlockID,
                                          &wDataType, &ValueBlockID, &dwLength);

                if (sError && (ERR(sError) == ERR_ITEM_NOT_FOUND) )
                {
                    NSFNoteClose ( hNote );
                    break;
                }

                if(sError && (sError != NOERROR ) )
                {
                    NSFNoteClose ( hNote );
                    return NOERROR;
                }

            } /* End of While */
        } /* if (pCDV4HotSpot->Type == HOTSPOTREC_TYPE_SUBFORM) */

    } /* End of if (RecordType == SIG_CD_V4HOTSPOTBEGIN &&  Gbl_bV4HotSpot) */

    /* Return if not a CDFIELD record */
    if (RecordType != SIG_CD_FIELD && RecordType != SIG_CD_FIELD_PRE_36 )
        return NOERROR;

    /*
     * if record type is SIG_CD_FIELD get field type.
     */
   
    if (RecordType == SIG_CD_FIELD)
    {
            
        pItemValue = RecordPtr;

        /* 
         * Convert data from Domino and Notes canonical format to the
         * host specific format
         */

        ODSReadMemory (&pItemValue, _CDFIELD, &CDField, 1);
            
        /*
         * Get the data type of this field
         */

        binDataType = CDField.DataType;
    
        switch (binDataType)
        {
            case TYPE_TEXT:
                strcpy (szDataType, "Text");
                break;

            case TYPE_TEXT_LIST:
                strcpy (szDataType, "Multi-Value Text");
                break;

            case TYPE_NUMBER:
                strcpy (szDataType, "Number");
                break;

            case TYPE_NUMBER_RANGE:
                strcpy (szDataType, "Multi-Value Number");
                break;

            case TYPE_TIME:
                strcpy (szDataType, "Time/Date");
                break;

            case TYPE_TIME_RANGE:
                strcpy (szDataType, "Multi-Value Time/Date");
                break;

            case TYPE_COMPOSITE:
                strcpy (szDataType, "Rich Text");
                break;

            default:
                strcpy (szDataType, "Unknown Data Type");
                break;
        }

        /*
         * Now get the name of this field.
        */

        pFieldName = (RecordPtr + ODSLength(_CDFIELD) + CDField.DVLength
                      + CDField.ITLength + CDField.IVLength);

        memcpy(szFieldName, pFieldName, CDField.NameLength);
        szFieldName[CDField.NameLength] = '\0';

        /*
         *  Construct a string containing the field name and datatype,
         *  and append it to a global buffer.
         */

        sprintf(FieldString, "Field Name = %s, Data Type = %s\n",
                szFieldName, szDataType);

        strcat(pBuf, FieldString);
    
    }

  /* Return status */
  return (sError);

}   
#ifdef __cplusplus
}
#endif
