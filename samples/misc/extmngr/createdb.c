 /*
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
 *
 * Domino and Notes header files
 */

#if defined(OS400)
#pragma convert(850)
#endif

#if defined(OS390)
#include <lapicinc.h>
#endif

#include <stdio.h>
#include <string.h>

#include <global.h>
#include <ixport.h>
#include <dbdrv.h>
#include <dbdrverr.h>
#include <osmem.h>
#include <nsfdata.h>
#include <names.h>
#include <osfile.h>

#include <nif.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <editods.h>
#include <stdnames.h>
#include <nsfsearc.h>
#include <colorid.h>
#include <miscerr.h>
#include <actods.h>
#include <nifcoll.h>
#include <osmisc.h>
#include <viewfmt.h>
#include <colorods.h>

#include "extmngr.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/* log file vars */
extern char FileName[256];
extern char DataDir[256];
extern __FILE *gFStream;
extern char gTextBuffer[1024];
extern void LogLine(char *);

#if defined(OS400)
extern DBRECORD r_pDBRecord[5];
int icount = 0;
#endif

/****************************************************************************
 *  FUNCTION: ReadDataBase:
 *
 *      This routine reads the contents of the ANIMALS.DB database and parses
 *      the information into a linked list of database records.
 *
 *  Inputs:
 *      **pDBRec - The address of a pointer to a DBRECORD structure.
 *
 *  Outputs:
 *      **pDBRec - Fills in the pointer to a linked list of DBRECORD
 *                 structures.
 *
 *  Returns:
 *      STATUS of call.
 ***************************************************************************/

STATUS LNPUBLIC ReadDataBase(DBRECORD **pDBRec)
{
DBRECORD   *pDBNew=0;
DBRECORD   *pDBHead=0;
DBRECORD   *pDBPrev=0;
STATUS     sError = NOERROR;

FILE       *pFile;

WORD       wDirNameLen = 0;
char       DBFileName[MAXPATH];   /* database filename          */
char       String[128];            /* String var. for reading from file.    */
#ifndef OS400
char       *pFullPathName;         /* Full pathname for database file       */
#else
char       pFullPathName[MAXPATH];         /* Full pathname for database file       */
#endif
char       *pString;               /* Pointer to help parse input from file.*/
int        item;

/*
 *  Allocate a buffer in which to store the filename, and construct
 *  the full path to the database file.
 */

#ifndef OS400
  pFullPathName = (char *) __AllocateMemory((size_t) MAXPATH); /* Allocate buffer */
#endif
  memset(pFullPathName, 0, MAXPATH);                 /* Init to zero    */

#if defined (OS400)
  wDirNameLen = OSGetDataDirectory(pFullPathName);
  strcat(pFullPathName, "/");
  wDirNameLen = strlen(pFullPathName);
  strcpy(DBFileName, pFullPathName);
  strcat(DBFileName, DB_NAME);
#else
  wDirNameLen = OSGetDataDirectory(pFullPathName);
  strcat(pFullPathName, "\\");
  wDirNameLen = strlen(pFullPathName);
  strcpy(DBFileName, pFullPathName);
  strcat(DBFileName, DB_NAME);
#endif

#ifndef OS400
  if (pFullPathName)
    __FreeMemory(pFullPathName);
#endif
  

/*
 *  Open the database.
 */
  pFile = __OpenFile(DBFileName, READ_PERMISSION);
  if (pFile == NULL)
  {
      return (ERR_FILE_OPEN); /* Couldn't open the file. */
  }

/* Reset file ptr to start of file.  */
  __FileSeek(pFile, 0L, SEEK_SET);

/* Read line from file */
    while ( __ReadLine(String, 80, pFile))
    {
        /* malloc a new DBRECORD for each line */
#ifndef OS400
        pDBNew = (DBRECORD *) __AllocateMemory(sizeof(DBRECORD));
#else
        pDBNew = &(r_pDBRecord[icount]);
        icount++;
#endif

        if (!pDBNew)
            return(ERR_MEMORY);
        /* if we haven't created the first record yet init it */
        if (pDBHead == NULL)
        {
            pDBHead = pDBNew;
            pDBHead->Next = 0;
            pDBPrev = pDBHead;
        }
        else
            pDBPrev->Next = pDBNew;

        /* search for "|" in string.  This is the delimiter per field or item */
        pString = strtok((char *)String,"|");
        strcpy(pDBNew->CommonName,pString); /* copy first item */

        item = 0;

        while (pString != NULL)
        {
            pString = strtok(NULL,"|");

            switch(item++)
            {
                case 0:  strcpy(pDBNew->LatinName,pString); /* copy into the LName item */
                    break;

                case 1:  strcpy(pDBNew->Range,pString); /* copy into the Range item */
                    break;
                case 2:  strcpy(pDBNew->Status,pString); /* copy into the Status item */
                    if (pDBNew->Status[strlen(pDBNew->Status)-1]=='\n')
                        pDBNew->Status[strlen(pDBNew->Status)-1]='\0';
                    break;
                default:
                    break;
            }
        }
        pDBNew->Next = 0;
        pDBPrev = pDBNew;
    }
  
    __CloseFile(pFile); 
    *pDBRec = pDBHead;

    return(NOERROR);
} 


/****************************************************************************
 *  FUNCTION: CreateDBForm: - Create the Database Form.
 *
 *      This routine creates the main form in the "animals.nsf" database.
 *
 *  Inputs:
 *      *pDBHead - Points to a linked list of DBRECORD structures.
 *
 *  Outputs:
 *      N/A
 *
 *  Returns:
 *      STATUS of call.
 ***************************************************************************/

STATUS LNPUBLIC CreateDBForm(DBRECORD *pDBHead )
{
    NOTEHANDLE hNote;
    NOTEID FormNoteID;
    STATUS sError;
    DBHANDLE hDB;

    char    error_text[200];

    char szFileName[] = "animals.nsf";

    DHANDLE hTextInputTransFormula;
    WORD   wTextInputTransFormulaLen;
    char  *pTextInputTransFormula;

    WORD   wdc;           /* "We Don't Care" - Used to receive data that */
                          /* we don't care about that is returned by     */
                          /* NSFFormulaCompile.                          */

    WORD wCDBufferLength = MAXONESEGSIZE; /* Length of current CD buffer */

    CDFIELD           CDField;
    CDBEGINRECORD     CDBegin;
    CDENDRECORD       CDEnd;
    CDEXT2FIELD           CDExt2Field;


    char far   *pBufferStart, far *pBuffer;

    char LatinNameString[] = "Latin Name:\t";
    char RangeString[]     = "Range:\t\t";
    char StatusString[]    = "Status:\t\t";

    char CommonNameField[] = "CommonNameField";
    char CommonNameDescription[] =  "This is the Common Name Field";

    char LatinNameField[]  = "LatinNameField";
    char LatinNameDescription[] =  "This is the Latin Name Field";
    char TextInputTransFormula[] = "@Uppercase(TextField)";

    char RangeField[]   =    "RangeField";
    char RangeDescription[] =  "This is the Range Number Field";

    char StatusField[]     = "StatusField";
    char StatusDescription[]   =  "This is the Status Field";

    char FormName[] = "Animals Form";

    BOOL bError;
    DHANDLE hMem;
    FONTIDFIELDS *pFontFields;
    WORD ClassForm = NOTE_CLASS_FORM;

    CDDOCUMENT InfoStruct;
    BYTE       bInfoLength;

    char far  *pInfoBufStart, far *pInfoBuf;
    DHANDLE      hInfoBuf;
    WORD       wInfoBufLen;
    DWORD      dwItemLength;

    STATUS  error = NOERROR;              /* error code from API calls */

    if ((gFStream = __OpenFile(FileName, APPEND_PERMISSION)) == NULL)
    {
        sError = ERR_FILE_OPEN;
        goto Exit2;
    }



    LogLine("---------------------------------------------------\n");
    LogLine("Creating DBForm \n");
    LogLine("---------------------------------------------------\n");
 // fflush(gFStream);

/*
 * Open database
 */

    if (sError = NSFDbOpen(szFileName, &hDB))
    {
        goto Exit2;
    }

/*
 * Check to see if we can find a form with this name already (disallow duplicates)
 */

    if ((sError = NIFFindDesignNote(hDB, FormName, NOTE_CLASS_FORM,
                                    &FormNoteID)) != ERR_NOT_FOUND)
    {
        sError = 1;
        goto Exit3;
    }
/*
 * Now create a note in database
 */

    if (sError = NSFNoteCreate(hDB, &hNote))
    {
        goto Exit3;
    }

/*******************************************************************
 *
 * All fields for this form have been defined. Set the NOTE_CLASS to
 * NOTE_CLASS_FORM, then update the note.
 *
 ******************************************************************/

  NSFNoteSetInfo( hNote, _NOTE_CLASS, &ClassForm );

 /*******************************************************
 *
 * Set the form name. ($TITLE field)
 *
 *******************************************************/

    if (sError = NSFItemSetText(hNote,
                                ITEM_NAME_TEMPLATE_NAME,
                                FormName,
                                MAXWORD))
    {
        goto Exit4;
    }

/*********************************************************
 *
 * Construct the $INFO field, append it to the note.
 *
 *********************************************************/

/*
 *  First, allocate a buffer and get a pointer to it.
 */

    wInfoBufLen = ODSLength(_CDDOCUMENT);
    if (sError = OSMemAlloc(0, wInfoBufLen, &hInfoBuf))
    {
        goto Exit4;
    }

/*
 *  Get pointer to start of CDDOCUMENT BUFFER. pInfoBufStart will remain
 *  pointing at the start of the buffer, while pInfoBuf will be set to
 *  point to the next available byte.
 */

    pInfoBufStart = (char*)OSLockObject(hInfoBuf);
    memset( pInfoBufStart, 0, (size_t)wInfoBufLen );
    pInfoBuf = pInfoBufStart;

/*
 *  Construct the CDDOCUMENT item.
 */

    /* set CDDOCUMENT structure to all zeros */
    memset(&InfoStruct ,0, sizeof(CDDOCUMENT));

    InfoStruct.Header.Signature = SIG_CD_DOCUMENT;
    InfoStruct.Header.Length = bInfoLength;   /* ODSLength(_CDDOCUMENT);*/
    InfoStruct.PaperColor = NOTES_COLOR_WHITE;
    InfoStruct.FormFlags = 0;
    InfoStruct.NotePrivileges = 0;
    InfoStruct.FormFlags2 = 0;
    InfoStruct.InherFieldNameLength = 0;
    InfoStruct.PaperColorExt = 0;

/*
 *  Convert the CDDOCUMENT item to Domino and Notes Canonical format
 *  and store it in the ODS buffer. pInfoBufStart remains pointing to
 *  the beginning of the buffer. ODSWriteMemory advances pInfoBuf to
 *  point to the next byte after the end of the CDDOCUMENT record.
 */

  ODSWriteMemory( (void far * far *)&pInfoBuf, _CDDOCUMENT,
                  &InfoStruct, 1 );

/*
 * Append the ODS buffer to the note as the $INFO field
 */
  dwItemLength = (DWORD) wInfoBufLen;

  sError = NSFItemAppend(hNote,
                         0,
                         ITEM_NAME_DOCUMENT,
                         (WORD) strlen(ITEM_NAME_DOCUMENT),
                         TYPE_COMPOSITE,
                         (void far *)pInfoBufStart,
                         dwItemLength);

    OSUnlockObject(hInfoBuf);
    OSMemFree(hInfoBuf);

    if (sError)
    {
        goto Exit4;
    }

/********************************************************************
 *
 *  Insert "placeholder" items into the form note, one for each item that
 *  will be contained in the form.  The placeholder items are used
 *  by the Domino and Notes user interface to generate the list of field
 *  names when the user chooses "Add Field" from the "Design...Views"
 *  dialog box.
 *
 *********************************************************************/

/*
 *  Text item placeholder.
 */

    if (sError = NSFItemAppend(hNote,
                               ITEM_PLACEHOLDER,
                               CommonNameField,
                               (WORD) strlen (CommonNameField),
                               TYPE_INVALID_OR_UNKNOWN,
                               NULL,
                               0))
    {
        goto Exit4;
    }

/*
 *  Text item placeholder.
 */

    if (sError = NSFItemAppend(hNote,
                               ITEM_PLACEHOLDER,
                               LatinNameField,
                               (WORD) strlen (LatinNameField),
                               TYPE_INVALID_OR_UNKNOWN,
                               NULL,
                               0))
    {
        goto Exit4;
    }

/*
 *  Text item placeholder.
 */

    if (sError = NSFItemAppend(hNote,
                               ITEM_PLACEHOLDER,
                               RangeField,
                               (WORD) strlen (RangeField),
                               TYPE_INVALID_OR_UNKNOWN,
                               NULL,
                               0))
    {
        goto Exit4;
    }

/*
 *  Text item placeholder.
 */

    if (sError = NSFItemAppend(hNote,
                               ITEM_PLACEHOLDER,
                               StatusField,
                               (WORD) strlen (StatusField),
                               TYPE_INVALID_OR_UNKNOWN,
                               NULL,
                               0))
    {
        goto Exit4;
    }


/*****************************************************************
 *
 * Construct the $BODY field.  For each field that will be defined
 * in the new form, define a CDFIELD item, append it to the note.
 *
 *****************************************************************/

/*
 * Allocate a buffer in which the CDFIELD item will be built
 */
    if (sError = OSMemAlloc (0, wCDBufferLength, &hMem))
    {
        goto Exit4;
    }

/*
 *  Lock global buffer and initialize it.
 */

    pBufferStart = (char far *)OSLockObject(hMem);
    memset( pBufferStart, 0, (size_t) wCDBufferLength );
    pBuffer = pBufferStart;


/*********************************************************************
 *
 *  Create the first field in the form.  This field will be an editable
 *  field of type TYPE_TEXT.
 *
 **********************************************************************/

/*
 * Setup a pabdef
 */

    bError = PutPabDef(&pBuffer,
                       1,
                       (WORD)(wCDBufferLength - (pBuffer - pBufferStart)));

/*
 * Leave if error returned...
 */
    if (bError == FALSE)
    {
        goto Exit5;
    }

/*
 * Now add a pabref
 */

    bError = PutPabRef(&pBuffer,
                       1,
                       (WORD)(wCDBufferLength - (pBuffer - pBufferStart)));

/*
 * Leave if error returned...
 */

    if (bError == FALSE)
    {
        goto Exit5;
    }

/*
 * New CDBEGINRECORD for R5 before a CDFIELD type.
 */
    CDBegin.Header.Length = (BYTE)ODSLength(_CDBEGINRECORD);
    CDBegin.Header.Signature = SIG_CD_BEGIN;
    CDBegin.Version = 0;
    CDBegin.Signature = SIG_CD_FIELD;

    ODSWriteMemory( (void far * far *)&pBuffer, _CDBEGINRECORD,
                    (void far *) &CDBegin, 1 );

/*
 * New CDEXT2FIELD for R5 that follows a CDBEGINRECORD type.
 */

    /* init full structure */
    memset(&CDExt2Field, 0, sizeof(CDEXT2FIELD));

    CDExt2Field.Header.Length = (WORD)ODSLength(_CDEXT2FIELD);
    CDExt2Field.Header.Signature = SIG_CD_EXT2_FIELD;

    ODSWriteMemory( (void far * far *)&pBuffer, _CDEXT2FIELD,
                    (void far *) &CDExt2Field, 1 );

/*
 * Create a CDFIELD structure to define this text field.
 */

    CDField.Header.Signature = SIG_CD_FIELD;
    CDField.Flags = FEDITABLE;
    CDField.DataType = TYPE_TEXT;
    CDField.ListDelim = LDDELIM_SEMICOLON;


/*
 *  The NumberFormat paramter is not used for text fields, so clear it.
 */

    CDField.NumberFormat.Digits     = 0;
    CDField.NumberFormat.Format     = 0;
    CDField.NumberFormat.Attributes = 0;
    CDField.NumberFormat.Unused     = 0;

/*
 *  The TimeFormat paramter is not used for text fields, so clear it.
 */

    CDField.TimeFormat.Date      = 0;
    CDField.TimeFormat.Time      = 0;
    CDField.TimeFormat.Zone      = 0;
    CDField.TimeFormat.Structure = 0;

/*
 *  Set up FontID
 */

    pFontFields = (FONTIDFIELDS *)&CDField.FontID;

    pFontFields->Face  = FONT_FACE_ROMAN;
    pFontFields->Attrib = 0;
    pFontFields->Color = NOTES_COLOR_BLACK;
    pFontFields->PointSize  = 14;

/*
 * Compile the input translation formula.  This formula will translate
 * text entered into this field into all UPPERCASE.
 */

    if (sError = NSFFormulaCompile(NULL,
                                   0,
                                   TextInputTransFormula,
                                   (WORD) strlen(TextInputTransFormula),
                                   &hTextInputTransFormula,
                                   &wTextInputTransFormulaLen,
                                   &wdc, &wdc, &wdc,
                                   &wdc, &wdc))
    {
        goto Exit5;
    }

/*
 *  Fill in the rest of the CDFIELD item.
 */

    CDField.DVLength = 0;  /* No default value formula         */
    CDField.ITLength = wTextInputTransFormulaLen;
    CDField.TabOrder = 0;
    CDField.IVLength = 0;  /* No Input Validity Check formula  */
    CDField.NameLength = strlen(CommonNameField);
    CDField.DescLength = strlen(CommonNameDescription);

    CDField.TextValueLength = 0; /* Not a text list.   */


    CDField.Header.Length = ODSLength(_CDFIELD) +
                            CDField.DVLength +
                            CDField.ITLength +
                            CDField.IVLength +
                            CDField.NameLength +
                            CDField.DescLength +
                            CDField.TextValueLength;

/*
 *  If total length of CDFIELD item is not even, make it so.
 */

    if (CDField.Header.Length % 2)
        CDField.Header.Length++;


/*
 *  Convert the CDFIELD item to Domino and Notes Canonical format and
 *  store it in the ODS buffer. pBufferStart will remain pointing to
 *  the beginning of the buffer; ODSWriteMemory advances pBuffer to
 *  point to the next byte after the CDFIELD record in the buffer.
 */

    ODSWriteMemory( (void far * far *)&pBuffer, _CDFIELD,(void far *)&CDField, 1 );

/*
 *  Get a pointer to the compiled input translation formula.
 */

    pTextInputTransFormula = OSLock( char, hTextInputTransFormula );

 /*
  *  First, set the input translation formula.
  */

    memcpy( pBuffer, pTextInputTransFormula, wTextInputTransFormulaLen );
    pBuffer += CDField.ITLength;
                            /* Unlock, free memory for formula */
    OSUnlockObject(hTextInputTransFormula);
    OSMemFree(hTextInputTransFormula);

/*
 *  Now set the field name.
 */

    memcpy( pBuffer, CommonNameField, CDField.NameLength );
    pBuffer += CDField.NameLength;

/*
 *  And set the field description.
 */

    memcpy( pBuffer, CommonNameDescription, CDField.DescLength );
    pBuffer += CDField.DescLength;

/* Ensure record length is even */

    if ((pBuffer-pBufferStart) %2)
        pBuffer++;

/*
 * New CDENDRECORD for R5 after a CDFIELD type.
 */

    CDEnd.Header.Length = (BYTE)ODSLength(_CDENDRECORD);
    CDEnd.Header.Signature = SIG_CD_END;
    CDEnd.Version = 0;
    CDEnd.Signature = SIG_CD_FIELD;

    ODSWriteMemory( (void far * far *)&pBuffer, _CDENDRECORD,
                    (void far *) &CDEnd, 1 );


/*********************************************************************
 *
 *  Create the second field in the form.  This field will be an editable
 *  field of type TYPE_TEXT.
 *
 **********************************************************************/

/*
 *  Insert two CDPARAGRAPH items so that this field is separated from
 *  the previous field by a blank line.
 */

    bError = PutPara(&pBuffer,
                    (WORD) (wCDBufferLength - (pBuffer - pBufferStart)));

/*
 * Leave if error returned...
 */
    if (bError == FALSE)
    {
        goto Exit5;
    }

    bError = PutPara(&pBuffer,
                     (WORD) (wCDBufferLength - (pBuffer - pBufferStart)));

    if (bError == FALSE)
    {
        goto Exit5;
    }


/*
 * Insert a string into the rich text field.  This string contains the text
 * of this field's label.
 */

  bError = PutText (&pBuffer,
                    LatinNameString,
                    (WORD)(wCDBufferLength - (pBuffer - pBufferStart)));


    if (bError == FALSE)
    {
        goto Exit5;
    }

/*
 * New CDBEGINRECORD for R5 before a CDFIELD type.
 */

    CDBegin.Header.Length = (BYTE)ODSLength(_CDBEGINRECORD);
    CDBegin.Header.Signature = SIG_CD_BEGIN;
    CDBegin.Version = 0;
    CDBegin.Signature = SIG_CD_FIELD;

    ODSWriteMemory( (void far * far *)&pBuffer, _CDBEGINRECORD,
                    (void far *) &CDBegin, 1 );

/*
 * New CDEXT2FIELD for R5 that follows a CDBEGINRECORD type.
 */

    /* init full structure */
    memset(&CDExt2Field, 0, sizeof(CDEXT2FIELD));

    CDExt2Field.Header.Length = (WORD)ODSLength(_CDEXT2FIELD);
    CDExt2Field.Header.Signature = SIG_CD_EXT2_FIELD;

  ODSWriteMemory( (void far * far *)&pBuffer, _CDEXT2FIELD,
                  (void far *) &CDExt2Field, 1 );

/*
 * Create a CDFIELD structure to define this text field.
 */

    CDField.Header.Signature = SIG_CD_FIELD;
    CDField.Flags = FEDITABLE;
    CDField.DataType = TYPE_TEXT;
    CDField.ListDelim = LDDELIM_SEMICOLON;

/*
 *  The NumberFormat paramter is not used for text fields, so clear it.
 */

    CDField.NumberFormat.Digits     = 0;
    CDField.NumberFormat.Format     = 0;
    CDField.NumberFormat.Attributes = 0;
    CDField.NumberFormat.Unused     = 0;

/*
 *  The TimeFormat paramter is not used for text fields, so clear it.
 */

    CDField.TimeFormat.Date      = 0;
    CDField.TimeFormat.Time      = 0;
    CDField.TimeFormat.Zone      = 0;
    CDField.TimeFormat.Structure = 0;

/*
 *  Set up FontID
 */

    pFontFields = (FONTIDFIELDS *)&CDField.FontID;

    pFontFields->Face  = FONT_FACE_ROMAN;
    pFontFields->Attrib = 0;
    pFontFields->Color = NOTES_COLOR_BLACK;
    pFontFields->PointSize  = 14;

/*
 * Compile the input translation formula.  This formula will translate
 * text entered into this field into all UPPERCASE.
 */

  if (sError = NSFFormulaCompile(NULL,
                                 0,
                                 TextInputTransFormula,
                                 (WORD) strlen(TextInputTransFormula),
                                 &hTextInputTransFormula,
                                 &wTextInputTransFormulaLen,
                                 &wdc, &wdc, &wdc,
                                 &wdc, &wdc))
    {
        goto Exit5;
    }

/*
 *  Fill in the rest of the CDFIELD item.
 */

    CDField.DVLength = 0;  /* No default value formula         */
    CDField.ITLength = wTextInputTransFormulaLen;
    CDField.TabOrder = 0;
    CDField.IVLength = 0;  /* No Input Validity Check formula  */
    CDField.NameLength = strlen(LatinNameField);
    CDField.DescLength = strlen(LatinNameDescription);

    CDField.TextValueLength = 0; /* Not a text list.   */


    CDField.Header.Length = ODSLength(_CDFIELD) +
                            CDField.DVLength +
                            CDField.ITLength +
                            CDField.IVLength +
                            CDField.NameLength +
                            CDField.DescLength +
                            CDField.TextValueLength;

/*
 *  If total length of CDFIELD item is not even, make it so.
 */

    if (CDField.Header.Length % 2)
        CDField.Header.Length++;

/*
 *  Convert the CDFIELD item to Domino and Notes Canonical format
 *  and store it in the ODS buffer. pBufferStart will remain pointing
 *  to the beginning of the buffer; ODSWriteMemory advances pBuffer
 *  to point to the next byte after the CDFIELD record in the buffer.
 */

  ODSWriteMemory( (void far * far *)&pBuffer, _CDFIELD,
                  (void far *)&CDField, 1 );

/*
 *  Get a pointer to the compiled input translation formula.
 */

    pTextInputTransFormula = OSLock( char, hTextInputTransFormula );

 /*
  *  First, set the input translation formula.
  */

    memcpy( pBuffer, pTextInputTransFormula, wTextInputTransFormulaLen );
    pBuffer += CDField.ITLength;

    /* Unlock, free memory for formula */
    OSUnlockObject(hTextInputTransFormula);
    OSMemFree(hTextInputTransFormula);

/*
 *  Now set the field name.
 */

    memcpy( pBuffer, LatinNameField, CDField.NameLength );
    pBuffer += CDField.NameLength;

/*
 *  And set the field description.
 */

    memcpy( pBuffer, LatinNameDescription, CDField.DescLength );
    pBuffer += CDField.DescLength;

/* Ensure record length is even */

    if ((pBuffer-pBufferStart) %2)
        pBuffer++;

/*
 * New CDENDRECORD for R5 after a CDFIELD type.
 */

    CDEnd.Header.Length = (BYTE)ODSLength(_CDENDRECORD);
    CDEnd.Header.Signature = SIG_CD_END;
    CDEnd.Version = 0;
    CDEnd.Signature = SIG_CD_FIELD;

    ODSWriteMemory( (void far * far *)&pBuffer, _CDENDRECORD,
                    (void far *) &CDEnd, 1 );


/*********************************************************************
 *
 *  Create the third field in the form.  This field will be an editable
 *  field of type TYPE_TEXT.
 *
 **********************************************************************/

/*
 *  Insert two CDPARAGRAPH items so that this field is separated from
 *  the previous field by a blank line.
 */

    bError = PutPara(&pBuffer,
                     (WORD) (wCDBufferLength - (pBuffer - pBufferStart)));

/*
 * Leave if error returned...
 */
    if (bError == FALSE)
    {
        goto Exit5;
    }

    bError = PutPara(&pBuffer,
                    (WORD) (wCDBufferLength - (pBuffer - pBufferStart)));

    if (bError == FALSE)
    {
        goto Exit5;
    }

/*
 * Insert a string into the rich text field.  This string contains the text
 * of this field's label.
 */

    bError = PutText (&pBuffer,
                      RangeString,
                      (WORD)(wCDBufferLength - (pBuffer - pBufferStart)));


    if (bError == FALSE)
    {
        goto Exit5;
    }

/*
 * New CDBEGINRECORD for R5 before a CDFIELD type.
 */

    CDBegin.Header.Length = (BYTE)ODSLength(_CDBEGINRECORD);
    CDBegin.Header.Signature = SIG_CD_BEGIN;
    CDBegin.Version = 0;
    CDBegin.Signature = SIG_CD_FIELD;

    ODSWriteMemory( (void far * far *)&pBuffer, _CDBEGINRECORD,
                    (void far *) &CDBegin, 1 );

/*
 * New CDEXT2FIELD for R5 that follows a CDBEGINRECORD type.
 */

    /* init full structure */
    memset(&CDExt2Field, 0, sizeof(CDEXT2FIELD));

    CDExt2Field.Header.Length = (WORD)ODSLength(_CDEXT2FIELD);
    CDExt2Field.Header.Signature = SIG_CD_EXT2_FIELD;

    ODSWriteMemory( (void far * far *)&pBuffer, _CDEXT2FIELD,
                    (void far *) &CDExt2Field, 1 );

/*
 * Create a CDFIELD structure to define this text field.
 */

    CDField.Header.Signature = SIG_CD_FIELD;
    CDField.Flags = FEDITABLE;
    CDField.DataType = TYPE_TEXT;
    CDField.ListDelim = LDDELIM_SEMICOLON;

/*
 *  The NumberFormat paramter is not used for text fields, so clear it.
 */

    CDField.NumberFormat.Digits     = 0;
    CDField.NumberFormat.Format     = 0;
    CDField.NumberFormat.Attributes = 0;
    CDField.NumberFormat.Unused     = 0;

/*
 *  The TimeFormat paramter is not used for text fields, so clear it.
 */

    CDField.TimeFormat.Date      = 0;
    CDField.TimeFormat.Time      = 0;
    CDField.TimeFormat.Zone      = 0;
    CDField.TimeFormat.Structure = 0;

/*
 *  Set up FontID
 */

    pFontFields = (FONTIDFIELDS *)&CDField.FontID;

    pFontFields->Face  = FONT_FACE_ROMAN;
    pFontFields->Attrib = 0;
    pFontFields->Color = NOTES_COLOR_BLACK;
    pFontFields->PointSize  = 14;

/*
 * Compile the input translation formula.  This formula will translate
 * text entered into this field into all UPPERCASE.
 */

    if (sError = NSFFormulaCompile(NULL,
                                   0,
                                   TextInputTransFormula,
                                   (WORD) strlen(TextInputTransFormula),
                                   &hTextInputTransFormula,
                                   &wTextInputTransFormulaLen,
                                   &wdc, &wdc, &wdc,
                                   &wdc, &wdc))
    {
        goto Exit5;
    }

/*
 *  Fill in the rest of the CDFIELD item.
 */

    CDField.DVLength = 0;  /* No default value formula         */
    CDField.ITLength = wTextInputTransFormulaLen;
    CDField.TabOrder = 0;
    CDField.IVLength = 0;  /* No Input Validity Check formula  */
    CDField.NameLength = strlen(RangeField);
    CDField.DescLength = strlen(RangeDescription);

    CDField.TextValueLength = 0; /* Not a text list.   */


    CDField.Header.Length = ODSLength(_CDFIELD) +
                            CDField.DVLength +
                            CDField.ITLength +
                            CDField.IVLength +
                            CDField.NameLength +
                            CDField.DescLength +
                            CDField.TextValueLength;

/*
 *  If total length of CDFIELD item is not even, make it so.
 */

    if (CDField.Header.Length % 2)
        CDField.Header.Length++;

/*
 *  Convert the CDFIELD item to Domino and Notes Canonical format and
 *  store it in the ODS buffer. pBufferStart will remain pointing to
 *  the beginning of the buffer; ODSWriteMemory advances pBuffer to point
 *  to the next byte after the CDFIELD record in the buffer.
 */

    ODSWriteMemory( (void far * far *)&pBuffer, _CDFIELD,
                    (void far *)&CDField, 1 );

/*
 *  Get a pointer to the compiled input translation formula.
 */

    pTextInputTransFormula = OSLock( char, hTextInputTransFormula );

 /*
  *  First, set the input translation formula.
  */

    memcpy( pBuffer, pTextInputTransFormula, wTextInputTransFormulaLen );
    pBuffer += CDField.ITLength;

  /* Unlock, free memory for formula */
    OSUnlockObject(hTextInputTransFormula);
    OSMemFree(hTextInputTransFormula);

/*
 *  Now set the field name.
 */

    memcpy( pBuffer, RangeField, CDField.NameLength );
    pBuffer += CDField.NameLength;

/*
 *  And set the field description.
 */

    memcpy( pBuffer, RangeDescription, CDField.DescLength );
    pBuffer += CDField.DescLength;

/* Ensure record length is even */

    if ((pBuffer-pBufferStart) %2)
        pBuffer++;

/*
 * New CDENDRECORD for R5 after a CDFIELD type.
 */

    CDEnd.Header.Length = (BYTE)ODSLength(_CDENDRECORD);
    CDEnd.Header.Signature = SIG_CD_END;
    CDEnd.Version = 0;
    CDEnd.Signature = SIG_CD_FIELD;

    ODSWriteMemory( (void far * far *)&pBuffer, _CDENDRECORD,
                    (void far *) &CDEnd, 1 );

/*********************************************************************
 *
 *  Create the fourth field in the form.  This field will be an editable
 *  field of type TYPE_TEXT.
 *
 **********************************************************************/

/*
 *  Insert two CDPARAGRAPH items so that this field is separated from
 *  the previous field by a blank line.
 */

    bError = PutPara(&pBuffer,
                     (WORD) (wCDBufferLength - (pBuffer - pBufferStart)));

/*
 * Leave if error returned...
 */
    if (bError == FALSE)
    {
        goto Exit5;
    }

    bError = PutPara(&pBuffer,
                     (WORD) (wCDBufferLength - (pBuffer - pBufferStart)));

    if (bError == FALSE)
    {
        goto Exit5;
    }

/*
 * Insert a string into the rich text field.  This string contains the text
 * of this field's label.
 */

    bError = PutText (&pBuffer,
                    StatusString,
                    (WORD)(wCDBufferLength - (pBuffer - pBufferStart)));


    if (bError == FALSE)
    {
        goto Exit5;
    }

/*
 * New CDBEGINRECORD for R5 before a CDFIELD type.
 */

    CDBegin.Header.Length = (BYTE)ODSLength(_CDBEGINRECORD);
    CDBegin.Header.Signature = SIG_CD_BEGIN;
    CDBegin.Version = 0;
    CDBegin.Signature = SIG_CD_FIELD;

    ODSWriteMemory( (void far * far *)&pBuffer, _CDBEGINRECORD,
                    (void far *) &CDBegin, 1 );

/*
 * New CDEXT2FIELD for R5 that follows a CDBEGINRECORD type.
 */

  /* init full structure */
    memset(&CDExt2Field, 0, sizeof(CDEXT2FIELD));

    CDExt2Field.Header.Length = (WORD)ODSLength(_CDEXT2FIELD);
    CDExt2Field.Header.Signature = SIG_CD_EXT2_FIELD;

    ODSWriteMemory( (void far * far *)&pBuffer, _CDEXT2FIELD,
                    (void far *) &CDExt2Field, 1 );

/*
 * Create a CDFIELD structure to define this text field.
 */

    CDField.Header.Signature = SIG_CD_FIELD;
    CDField.Flags = FEDITABLE;
    CDField.DataType = TYPE_TEXT;
    CDField.ListDelim = LDDELIM_SEMICOLON;

/*
 *  The NumberFormat paramter is not used for text fields, so clear it.
 */

    CDField.NumberFormat.Digits     = 0;
    CDField.NumberFormat.Format     = 0;
    CDField.NumberFormat.Attributes = 0;
    CDField.NumberFormat.Unused     = 0;

/*
 *  The TimeFormat paramter is not used for text fields, so clear it.
 */

    CDField.TimeFormat.Date      = 0;
    CDField.TimeFormat.Time      = 0;
    CDField.TimeFormat.Zone      = 0;
    CDField.TimeFormat.Structure = 0;

/*
 *  Set up FontID
 */

  pFontFields = (FONTIDFIELDS *)&CDField.FontID;

    pFontFields->Face  = FONT_FACE_ROMAN;
    pFontFields->Attrib = 0;
    pFontFields->Color = NOTES_COLOR_BLACK;
    pFontFields->PointSize  = 14;

/*
 * Compile the input translation formula.  This formula will translate
 * text entered into this field into all UPPERCASE.
 */

    if (sError = NSFFormulaCompile(NULL,
                                   0,
                                   TextInputTransFormula,
                                   (WORD) strlen(TextInputTransFormula),
                                   &hTextInputTransFormula,
                                   &wTextInputTransFormulaLen,
                                   &wdc, &wdc, &wdc,
                                   &wdc, &wdc))
    {
        goto Exit5;
    }

/*
 *  Fill in the rest of the CDFIELD item.
 */

    CDField.DVLength = 0;  /* No default value formula         */
    CDField.ITLength = wTextInputTransFormulaLen;
    CDField.TabOrder = 0;
    CDField.IVLength = 0;  /* No Input Validity Check formula  */
    CDField.NameLength = strlen(StatusField);
    CDField.DescLength = strlen(StatusDescription);

    CDField.TextValueLength = 0; /* Not a text list.   */


    CDField.Header.Length = ODSLength(_CDFIELD) +
                            CDField.DVLength +
                            CDField.ITLength +
                            CDField.IVLength +
                            CDField.NameLength +
                            CDField.DescLength +
                            CDField.TextValueLength;

/*
 *  If total length of CDFIELD item is not even, make it so.
 */

    if (CDField.Header.Length % 2)
        CDField.Header.Length++;

/*
 *  Convert the CDFIELD item to Domino and Notes Canonical format and
 *  store it in the ODS buffer. pBufferStart will remain pointing to
 *  the beginning of the buffer; ODSWriteMemory advances pBuffer to point
 *  to the next byte after the CDFIELD record in the buffer.
 */

  ODSWriteMemory( (void far * far *)&pBuffer, _CDFIELD,
                  (void far *)&CDField, 1 );

/*
 *  Get a pointer to the compiled input translation formula.
 */

  pTextInputTransFormula = OSLock( char, hTextInputTransFormula );

 /*
  *  First, set the input translation formula.
  */

    memcpy( pBuffer, pTextInputTransFormula, wTextInputTransFormulaLen );
    pBuffer += CDField.ITLength;

    /* Unlock, free memory for formula */
    OSUnlockObject(hTextInputTransFormula);
    OSMemFree(hTextInputTransFormula);

/*
 *  Now set the field name.
 */

    memcpy( pBuffer, StatusField, CDField.NameLength );
    pBuffer += CDField.NameLength;

/*
 *  And set the field description.
 */

    memcpy( pBuffer, StatusDescription, CDField.DescLength );
    pBuffer += CDField.DescLength;

/* Ensure record length is even */

    if ((pBuffer-pBufferStart) %2)
        pBuffer++;

/*
 * New CDENDRECORD for R5 after a CDFIELD type.
 */

    CDEnd.Header.Length = (BYTE)ODSLength(_CDENDRECORD);
    CDEnd.Header.Signature = SIG_CD_END;
    CDEnd.Version = 0;
    CDEnd.Signature = SIG_CD_FIELD;

    ODSWriteMemory( (void far * far *)&pBuffer, _CDENDRECORD,
                    (void far *) &CDEnd, 1 );

/***********************************************************************
 *
 *  All form fields have been defined. Now append the buffer to the note.
 *
 ***********************************************************************/

  if (sError = NSFItemAppend(hNote,
                             0,
                             ITEM_NAME_TEMPLATE,
                             (WORD) strlen (ITEM_NAME_TEMPLATE),
                             TYPE_COMPOSITE,
                             (void*)pBufferStart,
                             pBuffer-pBufferStart))
    {
        goto Exit5;
    }


    sError = NSFNoteUpdate( hNote, 0 );
    if (sError != NOERROR) 
    {
        OSLoadString (NULLHANDLE,
                      sError,
                      error_text,
                      sizeof(error_text));
        goto Exit5;
    }
 

Exit5:

    OSUnlockObject (hMem);
    OSMemFree (hMem);

Exit4:

    NSFNoteClose( hNote );

Exit3:

    NSFDbClose( hDB );

Exit2:
  
    return( sError );

}


/****************************************************************************
 *  FUNCTION: CreateDBNotes:
 *
 *      - This routine creates the Database Notes from the DBRECORD structures.
 *
 *  Inputs:
 *      *pDBRec - pointer to linked list of DBRECORD structures.
 *
 *  Outputs:
 *      N/A
 *
 *  Returns:
 *      STATUS of call.
 ***************************************************************************/

STATUS LNPUBLIC CreateDBNotes(DBRECORD *pDBRec)
{

    char        path_name[]="animals.nsf";   /* pathname of database */
    DBHANDLE    db_handle;                   /* database handle */
    NOTEHANDLE  note_handle;        /* note handle */
    STATUS      error = NOERROR;    /* return code from API calls */

	
    gFStream = __OpenFile(FileName, APPEND_PERMISSION);
	
	
   //FILE *gFStream = __OpenFile(FileName, APPEND_PERMISSION);



    LogLine("---------------------------------------------------\n");
    LogLine("Creating CreateDBNotes \n");
    LogLine("---------------------------------------------------\n");
    fflush(gFStream);
  /* Open the database. */

    if (error = NSFDbOpen (path_name, &db_handle))
    {
        goto Exit;
    }

    while (pDBRec != NULL)
    {
      /* Create a new data note. */

        if (error = NSFNoteCreate (db_handle, &note_handle))
        {
            NSFDbClose (db_handle);
            goto Exit;
        }

        /* Write a field named FORM to the note -- this field specifies the
         default form to use when displaying the note. */

        if (error = NSFItemSetText ( note_handle,
                                     "FORM",
                                     "Animals Form",
                                     MAXWORD))
        {
            NSFNoteClose (note_handle);
            NSFDbClose (db_handle);
            goto Exit;
        }

        /* Write a text field to the note. */

        if (error = NSFItemSetText ( note_handle,
                                     "CommonNameField",
                                     pDBRec->CommonName,
                                     MAXWORD))
        {
            NSFNoteClose (note_handle);
            NSFDbClose (db_handle);
            goto Exit;
        }

        /* Write a text field to the note. */

        if (error = NSFItemSetText ( note_handle,
                                     "LatinNameField",
                                     pDBRec->LatinName,
                                     MAXWORD))
        {
            NSFNoteClose (note_handle);
            NSFDbClose (db_handle);
            goto Exit;
        }

        /* Write a text field to the note. */

        if (error = NSFItemSetText ( note_handle,
                                     "RangeField",
                                     pDBRec->Range,
                                     MAXWORD))
        {
            NSFNoteClose (note_handle);
            NSFDbClose (db_handle);
            goto Exit;
        }


        /* Write a text field to the note. */

        if (error = NSFItemSetText ( note_handle,
                                     "StatusField",
                                     pDBRec->Status,
                                     MAXWORD))
        {
            NSFNoteClose (note_handle);
            NSFDbClose (db_handle);
            goto Exit;
        }


        /* Add the entire new note (with all fields) to the database. */

        if (error = NSFNoteUpdate (note_handle, 0))
        {
            NSFNoteClose (note_handle);
            NSFDbClose (db_handle);
            goto Exit;
        }

        /* Close the note. (Remove its structure from memory.) */

        if (error = NSFNoteClose (note_handle))
        {
            NSFDbClose (db_handle);
            goto Exit;
        }

        pDBRec = pDBRec->Next;
    }

    /* Close the database */

    if (error = NSFDbClose (db_handle))
    {
        goto Exit;
    }

Exit:    
    return (error);
}


/****************************************************************************
 *  FUNCTION: CreateDBView:
 *
 *       - This routine creates the Database View.
 *
 *  Inputs:
 *      N/A
 *
 *  Outputs:
 *      N/A
 *
 *  Returns:
 *      STATUS of call.
 ***************************************************************************/
STATUS LNPUBLIC CreateDBView()
{

    STATUS sError = NOERROR;
    DBHANDLE hDB;
    NOTEHANDLE hNote;
    NOTEID     ViewNoteID;
    char szViewName[] = "Animals View"; /* Title of view to be created. */
    char szFileName[] = "animals.nsf";  /* Database in which to create it. */

    WORD wNumColumns = 2;     /* This view will contain this many columns. */
    WORD ClassView = NOTE_CLASS_VIEW;
/*
 * Variables pertaining to the view selection formula
 */

    char szSelFormula[] = "@ALL";
    FORMULAHANDLE hSelFormula = NULLHANDLE;
    WORD wSelFormulaLen;
    

/*
 * Variables pertaining to the first column in the view.
 */

    char szFormula_1[] = "CommonNameField";
    FORMULAHANDLE hFormula_1 = NULLHANDLE;
    WORD wFormula_1_Len;
    char szTitle_1[] = "Common Name Column";
    WORD wTitle_1_Len = strlen(szTitle_1);
    char szItemName_1[] = "ItemName1";
    WORD wItemName_1_Len = strlen( szItemName_1 );

/*
 * Variables pertaining to the second column in the view.
 */

    char szFormula_2[] = "LatinNameField";
    FORMULAHANDLE hFormula_2 = NULLHANDLE;
    WORD wFormula_2_Len;
    char szTitle_2[] = "Latin Name Column";
    WORD wTitle_2_Len = strlen(szTitle_2);
    char szItemName_2[] = "ItemName2";
    WORD wItemName_2_Len = strlen(szItemName_2);

/*
 * Variables pertaining to the $VIEWFORMAT item (also known as "View Table Format" item).
 */

    VIEW_TABLE_FORMAT  ViewTableFormat;
    VIEW_COLUMN_FORMAT ViewColumnFormat;
    VIEW_TABLE_FORMAT2 ViewTableFormat2;

    WORD    wViewFormatBufLen;
    DHANDLE  hViewFormatBuffer;
    char   *pViewFormatBuffer;
    char   *pVFBuf;

    WORD wdc;         /* "We Don't Care" - We're not interested in some of
                       *  the info passed back by NSFFormulaCompile(), but
                       *  the function call requires the addresses of several
                       *  words to be passed in. The address of this word
                       *  is used for all parameters in which we have no
                       *  interest.
                       */



/*
 * Open database
 */

    if (sError = NSFDbOpen( szFileName, &hDB ))
    {
        sprintf(gTextBuffer, "Error: unable to open database '%s'.\n", szFileName );
        LogLine( gTextBuffer);
        goto Exit2;
    }

/*
 * Check to see if we can find a view with this name already (disallow duplicates)
 */

    if ((sError = NIFFindView(hDB, szViewName, &ViewNoteID)) != ERR_NOT_FOUND)
    {
        LogLine("-------------------------------------------------\n");
        sprintf(gTextBuffer, "Error: View named '%s' already exists in database.\n",
        szViewName );
        LogLine( gTextBuffer);
        sError = 1;
        goto Exit3;
    }

/*
 * Now create a note in database
 */

    sError = NSFNoteCreate( hDB, &hNote );
    if (sError)
    {
        LogLine("-------------------------------------------------\n");
        sprintf(gTextBuffer, "Error: unable to create note in database.\n" );
        LogLine( gTextBuffer);
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
        LogLine("-------------------------------------------------\n");
        sprintf(gTextBuffer, "Error: unable to set text item '%s' in view note.\n",
                VIEW_TITLE_ITEM );
        LogLine( gTextBuffer);
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
        LogLine("-------------------------------------------------\n");
        sprintf(gTextBuffer, "Error: unable to compile selection formula '%s'.\n",
                szSelFormula );
        LogLine( gTextBuffer); 
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
        LogLine("-------------------------------------------------\n");
        sprintf(gTextBuffer, "Error: unable to compile column 1 formula '%s'.\n",
                szFormula_1 );
        LogLine( gTextBuffer );
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
        LogLine("-------------------------------------------------\n");
        sprintf(gTextBuffer, "Error: unable to set summary item '%s'.\n",
                szItemName_1 );
        LogLine( gTextBuffer);
        goto Exit5;
    }


    sError = NSFFormulaMerge( hFormula_1, hSelFormula );
    if (sError)
    {
        LogLine("-------------------------------------------------\n");
        sprintf(gTextBuffer, "Error: unable to merge column 1 formula into selection formula.\n");
        LogLine( gTextBuffer );
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
        LogLine("-------------------------------------------------\n");
        sprintf(gTextBuffer, "Error: unable to compile column 2 formula '%s'.\n",
                szFormula_2 );
        LogLine( gTextBuffer );
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
        LogLine("-------------------------------------------------\n");
        sprintf(gTextBuffer, "Error: unable to merge column 2 item name into selection formula.\n");
        LogLine( gTextBuffer);
        goto Exit5;
    }

    sError = NSFFormulaMerge( hFormula_2, hSelFormula );
    if (sError)
    {
        LogLine("-------------------------------------------------\n");
        sprintf(gTextBuffer, "Error: unable to merge formula 2 into view selection formula.\n");
        LogLine( gTextBuffer);
        goto Exit5;
    }


/*
 * Get the size of the merged formula.
 */

    sError = NSFFormulaGetSize( hSelFormula, &wSelFormulaLen );
    if (sError)
    {
        LogLine("-------------------------------------------------\n");
        sprintf(gTextBuffer, "Error: unable to get size of selection formula.\n" );
        LogLine( gTextBuffer );
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
        LogLine("-------------------------------------------------\n");
        sprintf(gTextBuffer, "Error: unable to append item '%s' to view note.\n",
                VIEW_FORMULA_ITEM );
        LogLine( gTextBuffer); 
        goto Exit4;
    }


/*
 * Create the $VIEWFORMAT item. The $VIEWFORMAT item is an item
 * of TYPE_VIEW_FORMAT with name VIEW_VIEW_FORMAT_ITEM.
 *
 * The $VIEWFORMAT item for this view will consist of the following
 * series of structures converted to Domino and Notes canonical format and
 * packed together:
 *
 *          VIEW_TABLE_FORMAT
 *          VIEW_COLUMN_FORMAT (for column 1)
 *          VIEW_COLUMN_FORMAT (for column 2)
 *          Item Name for column 1
 *          Title for column 1
 *          formula for column 1
 *          Item Name for column 2
 *          Title for column 2
 *          formula for column 2
 *          VIEW_TABLE_FORMAT2
 *          VIEW_COLUMN_FORMAT2 (for column 1)
 *          VIEW_COLUMN_FORMAT2 (for column 2)
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
                        wTitle_1_Len                      +
                        wFormula_1_Len                    +
                        wItemName_2_Len                   +
                        wTitle_2_Len                      +
                        wFormula_2_Len                    +
                        ODSLength( _VIEW_TABLE_FORMAT2 )  ;


    if (sError = OSMemAlloc( 0, wViewFormatBufLen, &hViewFormatBuffer ))
    {
        LogLine("-------------------------------------------------\n");
        sprintf(gTextBuffer, "Error: unable to allocate %d bytes memory.\n",
                wViewFormatBufLen);
        LogLine( gTextBuffer);
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
 * host-specific format to Domino and Notes canonical format, and copy it
 * into the buffer at location pVFBuf. ODSWriteMemory increments pVFBuf to
 * point to the next byte in the buffer after the written data structure.
 */

    ODSWriteMemory( &pVFBuf, _VIEW_TABLE_FORMAT, &ViewTableFormat, 1 );

/*
 *  Create the VIEW_COLUMN_FORMAT item for column 1. Since this column
 *  is neither a TIME field or a NUMBER field, we don't need to set the
 *  TimeFormat or the NumberFormat fields.
 */

    ViewColumnFormat.Signature = VIEW_COLUMN_FORMAT_SIGNATURE;
    ViewColumnFormat.Flags1 = 0;

    ViewColumnFormat.DisplayWidth = 100;
    ViewColumnFormat.FontID = DEFAULT_BOLD_FONT_ID;
    ViewColumnFormat.Flags2 = 0;

    ViewColumnFormat.FormatDataType = VIEW_COL_TEXT;
    ViewColumnFormat.ListSep = LDDELIM_COMMA;

    ViewColumnFormat.FormulaSize = wFormula_1_Len;
    ViewColumnFormat.ItemNameSize = wItemName_1_Len;
    ViewColumnFormat.TitleSize = wTitle_1_Len;
    ViewColumnFormat.ConstantValueSize = 0; /* RESERVED _ SHOULD BE 0 */

/*
 * Convert the View Column Format structure for Col 1 to Domino and Notes
 * canonical format and append it to the buffer. This increments pVFBuf to
 * point to the next byte in the buffer after the View Column Format.
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
    ViewColumnFormat.Flags1 = 0;

    ViewColumnFormat.DisplayWidth = 150;
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
 * Col 2 to Domino and Notes canonical format and append it to the buffer.
 * This increments pVFBuf to point to the next byte after the View Column
 * Format structure.
 */

    ODSWriteMemory( &pVFBuf, _VIEW_COLUMN_FORMAT, &ViewColumnFormat, 1 );

/*
 * Append column 1's item name and formula to the buffer.
 */

    memcpy( pVFBuf, szItemName_1, wItemName_1_Len );
    pVFBuf += wItemName_1_Len;

    memcpy( pVFBuf, szTitle_1, wTitle_1_Len );
    pVFBuf += wTitle_1_Len;

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
 * Initialize the VIEW_TABLE_FORMAT2 structure.
 */
    memset( (char *)&ViewTableFormat2, 0, sizeof(VIEW_TABLE_FORMAT2) );

    ViewTableFormat2.Length = sizeof(VIEW_TABLE_FORMAT2);
    ViewTableFormat2.BackgroundColor = NOTES_COLOR_WHITE;
    ViewTableFormat2.V2BorderColor = 0;
    ViewTableFormat2.TitleFont = DEFAULT_BOLD_FONT_ID;
    ViewTableFormat2.UnreadFont = DEFAULT_FONT_ID;
    ViewTableFormat2.TotalsFont = DEFAULT_FONT_ID;
    ViewTableFormat2.AutoUpdateSeconds = 0;

    /* needed to initialize this member for R5 */
    ViewTableFormat2.wSig = VALID_VIEW_FORMAT_SIG;

/*
 * Convert the View Table Format2 structure to Domino and Notes canonical
 * format and append it to the buffer. This increments pVFBuf to point to
 * the next byte after the View Table Format2 structure.
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
        LogLine("-------------------------------------------------\n");
        sprintf(gTextBuffer, "Error: unable to append item '%s' to view note.\n",
                VIEW_VIEW_FORMAT_ITEM );
        LogLine( gTextBuffer);
        goto Exit5;
    }

/*
 *  Done constructing the view note. Now store the view note
 *  in the database.
 */

    sError = NSFNoteUpdate( hNote, 0 );

    if (sError)
    {
        LogLine("-------------------------------------------------\n");
        sprintf(gTextBuffer, "Error: unable to update view note in database.\n" );
        LogLine( gTextBuffer);
    }

Exit5:

    fflush(gFStream);
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

Exit4:

    NSFNoteClose( hNote );

Exit3:

    NSFDbClose( hDB );

Exit2:

    return (ERR(sError));
}


