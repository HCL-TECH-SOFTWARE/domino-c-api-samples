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

    PROGRAM:      makeform

    FILE:         makeform.c

    PURPOSE:      Shows how to create a form note, a subform note, and a 
                  frameset note in a database.

    DESCRIPTION:  Create a new form, a subform, and a frameset in a database 
                  (all forms are documents of class NOTE_CLASS_FORM).

                  The form document has the following required components: 
  
         $TITLE  item: This is a item of type TYPE_TEXT.  It specifies the
                       name of the form.
  
         $INFO   item: This is an item of type TYPE_COMPOSITE, containing
                       a CDDOCUMENT item.

         $BODY   item: This is an item of type TYPE_COMPOSITE, which
                       contains a CDFIELD item for each field defined in
                       the form.

      Other components that a form note may contain include:

         $FIELDS item: This is an item of type TYPE_TEXT_LIST, containing
                       a list of all of the field names in the form.
                       One could get the name of all fields in the form
                       by reading the text list instead of having to
                       search for all CDFIELD items in the $BODY item.

         Placeholders: The Domino and Notes user interface creates in the form note
                       a placeholder item for each field in the form. 
                       These items are items of type TYPE_INVALID_OR_UNKNOWN,
                       and have the ITEM_PLACEHOLDER ItemFlag set. Domino and Notes
                       uses these placeholder items to generate the list
                       of field names displayed when the "Add Field" button
                       is pressed in the "Design...View" dialog box. If the
                       placeholder items are not defined, a blank list of
                       field names would be displayed.
 
      The subform document has the following component for identification:
                  
         $Flags   item:  This is a item of type TYPE_TEXT with the following
                         flag values set:
         
                         DESIGN_FLAG_ADD;
                         DESIGN_FLAG_NO_COMPOSE;
                         DESIGN_FLAG_SUBFORM;
                         DESIGN_FLAG_HIDE_FROM_V3;

          The frameset document has the following component for identification:
                  
         $Flags   item:  This is a item of type TYPE_TEXT with the following
                         flag values set:

                                                 DESIGN_FLAG_ADD;
                         DESIGN_FLAG_NO_COMPOSE;
                         DESIGN_FLAG_FRAMESET;
                         DESIGN_FLAG_HIDE_FROM_V3;

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/*
 * Windows header file. Include only if compiling for Windows.
 */

#if (defined(DOS) && defined(W)) || (defined(NT))
#include <windows.h>
#endif

/* C header files     */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>

/* HCL C API for Notes/Domino Header files.  */

#if defined(OS390)
#include <lapicinc.h>
#endif

#include <global.h>
#include <nsfdata.h>
#include <nif.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <editods.h>
#include <osmem.h>
#include <stdnames.h>
#include <nsfsearc.h>
#include <colorid.h>
#include <miscerr.h>
#include <actods.h>
#include <actprop.h>
#include <editdflt.h>
#include <easycd.h>
#include <fsods.h>
#include <rsrcods.h>
#include <osmisc.h>
#include "../../printLog.h"

/*
 * Local header files
 */

#include "makeform.h"

#ifdef OS390
#include <_Ascii_a.h> /* NOTE: must be the LAST file included */
#endif

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

#define ACTIONTITLE  "Test ACTIONBAR"
#define IMAGE_SEGSIZE 10240

/* global database handle */
DBHANDLE hDB;

/* print any errors to this trace file */
FILE *pTraceFile;      
char errMsg[129];      

/* form names */
char FormName[]    = "Test Form 1";
char SubFormName[] = "Test SubForm 1";
char FrameName[]   = "Frameset1";
char HelpDoc[]     = "Help-About Database";

/* GIF names */
char szImage_LotusGif[] = "lotus.gif";
char szImage_DominoGif[] = "domino.gif";

/* Note Link global variables */
NOTEID far      ViewNoteID;
NOTEID far      FormNoteID;
ORIGINATORID    ViewOID;  
ORIGINATORID    FormOID;  
DBREPLICAINFO   DBReplica;

int main(int argc, char *argv[])

{
    STATUS sError=NOERROR;

    char   szFileName[] = "makeform.nsf";


/*
 *  Initialize Domino and Notes. 
 */

    if (sError = NotesInitExtended (argc, argv))
    {
        printf("\n Unable to initialize Notes.\n");
        return (1);
    }

/*
 * Open an error log file "makeform.err" in the current working directory.
 */

    if ( (pTraceFile = fopen("makeform.err", "w")) == NULL )
    {
        goto Exit1;
    }
    fprintf(pTraceFile,
            "HCL Notes API.\nMAKEFORM\n");

/*
 * Open database
 */

    if (sError = NSFDbOpen(szFileName, &hDB))
    {
        fprintf(pTraceFile,
                "Error: Unable to open database '%s'.\n", szFileName );
        goto Exit1;

    }

/*
 * Create Main Form
 */
    if (sError = CreateForm())
    {
        fprintf(pTraceFile, errMsg);
        goto Exit2;
    }

/*
 * Create Sub Form
 */
    if (sError = CreateSubForm())
    {
        fprintf(pTraceFile, errMsg);
        goto Exit2;
    }

/*
 * Insert Sub Form into Main Form
 */
    if (sError = SubformInsert(hDB,SubFormName,FormName,0))
    {
        fprintf(pTraceFile, errMsg);
    }

/*
 * Create Frame
 */
    if (sError = CreateFrame())
    {
        fprintf(pTraceFile, errMsg);
        goto Exit2;
    }

Exit2:
 
    NSFDbClose( hDB );

Exit1:
 
    fclose( pTraceFile );

    if (!sError)
    {
      PRINTLOG("Program completed successfully.\n");
    }
    else
    {
      PRINTLOG("This program encountered an error.  Please see trace file makeform.err!\n");
    }
      NotesTerm();
      return (1);


}

/************************************************************************

    FUNCTION:    CreateForm

    PURPOSE:     Create the main form in the database.

*************************************************************************/
STATUS LNPUBLIC CreateForm()
{
    NOTEHANDLE   hNote; 
    NOTEID       FormNoteID; 
    STATUS       sError=NOERROR;

    DHANDLE      hTextInputTransFormula;
    DHANDLE      hTextDefValFormula;
    WORD         wTextInputTransFormulaLen;
    WORD         wTextDefValFormulaLen;
    char         *pTextInputTransFormula;
    char         *pTextDefValFormula;
    
    DHANDLE      hTimeDefValFormula;
    WORD         wTimeDefValFormulaLen;
    char         *pTimeDefValFormula;
    
    WORD         wdc;     /* "We Don't Care" - Used to receive data that */
                          /* we don't care about that is returned by     */
                          /* NSFFormulaCompile.                          */

    WORD         wCDBufferLength = MAXONESEGSIZE; /* Length of current CD buffer */
    
    CDFIELD      CDField;
    char far     *pBufferStart, far *pBuffer;
    
    char         TextString[]     = "Text String:";
    char         NumberString[]   = "Number String:";
    char         TimeString[]     = "Time String:";
    char         RichTextString[] = "Rich Text String:";
    char         KeyWordString[]  = "Keyword String:";
    char         GifImageString[]  = "Gif Image:";

    char         TextFieldName[]  = "TextField";
    char         TextDescription[] =  "This is a Simple Text Field";
    char         TextInputTransFormula[] = "@Uppercase(TextField)";
    char         TextDefValFormula[] = "\"Default\"";
        
    char         NumberFieldName[]   = "NumberField";
    char         NumberDescription[] =  "This is a Number Field";

    char         TimeFieldName[]     = "TimeField";
    char         TimeDescription[]   =  "This is a Time Field";
    char         TimeDefValFormula[] = "@Today";

    char         RichTextFieldName[]   = "RichTextField";
    char         RichTextDescription[] =  "This is a Rich Text Field";

    char         KeyWordFieldName[]   = "KeyWordField";
    char         KeyWordDescription[] =  "This is a KeyWord Field";

    char         GifImageFieldName[]   = "GifImageField";
    char         GifImageDescription[] =  "This is a Gif Image Field";

    char         FormName[] = "Test Form 1";
        
    LIST         sList;
    char         Keyword1[] = "First Keyword";
    char         Keyword2[] = "Second Keyword";
    char         Keyword3[] = "Third Keyword";
    WORD         wKeyword1Len = strlen(Keyword1);
    WORD         wKeyword2Len = strlen(Keyword2);
    WORD         wKeyword3Len = strlen(Keyword3);
    WORD         wTextListLen;

    BOOL         bError;
    DHANDLE      hMem;
    FONTIDFIELDS *pFontFields;
    WORD         ClassForm = NOTE_CLASS_FORM;

    CDDOCUMENT   InfoStruct;
    BYTE         bInfoLength;

    char far     *pInfoBufStart, far *pInfoBuf;
    DHANDLE      hInfoBuf;
    WORD         wInfoBufLen;
    DWORD        dwItemLength;

    STATUS       error = NOERROR;     /* error code from API calls */

    BYTE         *FormActionPtr;      /* Pointer for $ACTION field contents */
    BYTE         *buff_ptr;           /* Point to the beginning of $ACTION field contents */
    DWORD        buff_len;

    CDACTIONBAR  ActionBar;
    CDACTION     Action;
    char         Formula[] = 
      "@Prompt([OK];\"Sample Button\";\"You pushed my button!\")";

    FORMULAHANDLE hFormula;
    WORD          FormulaLen;
    WORD          TitleLen;

    WORD          width=0x009A;
    WORD          height=0x003E;

    CDBEGINRECORD CDBegin;
    CDENDRECORD   CDEnd;
    CDEXT2FIELD   CDExt2Field;

/*
 * Check to see if we can find a form with this name already (disallow duplicates)
 */

    if ((sError = NIFFindDesignNote(hDB, FormName, NOTE_CLASS_FORM, 
                                 &FormNoteID)) != ERR_NOT_FOUND)
    {
        sprintf(errMsg, "Error: Form named '%s' already exists in database. Duplicates not allowed.\n",
                FormName );
        sError = ERR_ALREADY_EXIST;
        goto Exit3;
    }

/*
 * Now create a note in database
 */

    if (sError = NSFNoteCreate(hDB, &hNote))
    {
        sprintf(errMsg, "Error: Unable to create note in database.\n" );
        sError = ERR_ALREADY_EXIST;
        goto Exit3;
    }

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
        sprintf(errMsg,
                "Error: Unable to set text item '%s' in form note.\n",
                ITEM_NAME_TEMPLATE_NAME );
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
        sprintf(errMsg, "Error: Unable to allocate %d bytes memory.\n",
                wInfoBufLen);
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
    bInfoLength = (BYTE) ODSLength(_CDDOCUMENT);
    
/*
 *  Construct the CDDOCUMENT item.
 */
    /* init the structure... */
    memset(&InfoStruct, 0, sizeof(CDDOCUMENT));
 
    InfoStruct.Header.Signature = SIG_CD_DOCUMENT;
    InfoStruct.Header.Length = bInfoLength;   /* ODSLength(_CDDOCUMENT);*/
    InfoStruct.PaperColor = NOTES_COLOR_WHITE;
    InfoStruct.FormFlags = 0;
    InfoStruct.NotePrivileges = 0;
    InfoStruct.FormFlags2 = 0;
    InfoStruct.InherFieldNameLength = 0;
    InfoStruct.PaperColorExt = 0;

/*
 *  Convert the CDDOCUMENT item to Domino and Notes Canonical format and store it in
 *  the ODS buffer. pInfoBufStart remains pointing to the beginning of
 *  the buffer. ODSWriteMemory advances pInfoBuf to point to the next
 *  byte after the end of the CDDOCUMENT record.
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
        sprintf(errMsg,
                "Error: Unable to append item '%s' to form note.\n",
                ITEM_NAME_DOCUMENT );
        goto Exit4;
    }

/********************************************************************
 *
 *  Insert "placeholder" items into the form note, one for each item that
 *  will be contained in the form.  The placeholder items are used
 *  by the Domino and Notes user interface to generate the list of field 
 *  names when the user chooses "Add Field" from the "Design...Views" dialog box.
 *
 *********************************************************************/

/*
 *  Gif Image item placeholder.
 */

    if (sError = NSFItemAppend(hNote,
                               ITEM_PLACEHOLDER,
                               GifImageFieldName,
                               (WORD) strlen (GifImageFieldName),
                               TYPE_INVALID_OR_UNKNOWN,
                               NULL,
                               0))
    {
        sprintf(errMsg,
                "Error: Unable to append placeholder for field '%s' to form note.\n",
                GifImageFieldName );
        goto Exit4;

    }


/*
 *  Text item placeholder.
 */

    if (sError = NSFItemAppend(hNote,
                               ITEM_PLACEHOLDER,
                               TextFieldName,
                               (WORD) strlen (TextFieldName),
                               TYPE_INVALID_OR_UNKNOWN,
                               NULL,
                               0))
    {
        sprintf(errMsg,
                "Error: Unable to append placeholder for field '%s' to form note.\n",
                TextFieldName );
        goto Exit4;
    }


/*
 *  Number item placeholder.
 */

    if (sError = NSFItemAppend(hNote,
                               ITEM_PLACEHOLDER,
                               NumberFieldName,
                               (WORD) strlen (NumberFieldName),
                               TYPE_INVALID_OR_UNKNOWN,
                               NULL,
                               0))
    {
        sprintf(errMsg,
                "Error: Unable to append placeholder for field '%s' to form note.\n",
                NumberFieldName );
        goto Exit4;
    }

/*
 *  Time item placeholder.
 */

    if (sError = NSFItemAppend(hNote,
                               ITEM_PLACEHOLDER,
                               TimeFieldName,
                               (WORD) strlen (TimeFieldName),
                               TYPE_INVALID_OR_UNKNOWN,
                               NULL,
                               0))
    {
        sprintf(errMsg,
                "Error: Unable to append placeholder for field '%s' to form note.\n",
                TimeFieldName );
        goto Exit4;
    }

/*
 *  Rich Text item placeholder.
 */

    if (sError = NSFItemAppend(hNote,
                               ITEM_PLACEHOLDER,
                               RichTextFieldName,
                               (WORD) strlen (RichTextFieldName),
                               TYPE_INVALID_OR_UNKNOWN,
                               NULL,
                               0))
    {
        sprintf(errMsg,
                "Error: Unable to append placeholder for field '%s' to form note.\n",
                RichTextFieldName );
        goto Exit4;

    }

/*
 *  Keyword item placeholder.
 */

    if (sError = NSFItemAppend(hNote,
                               ITEM_PLACEHOLDER,
                               KeyWordFieldName,
                               (WORD) strlen (KeyWordFieldName),
                               TYPE_INVALID_OR_UNKNOWN,
                               NULL,
                               0))
    {

        sprintf(errMsg,
                "Error: Unable to append placeholder for field '%s' to form note.\n",
                KeyWordFieldName );
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
        sprintf(errMsg, "Error: Unable to allocate %d bytes memory.\n",
                wCDBufferLength);
        goto Exit4;
    }

/*
 *  Lock global buffer and initialize it.
 */
    
    pBufferStart = (char far *)OSLockObject(hMem);
    memset( pBufferStart, 0, (size_t) wCDBufferLength );
    pBuffer = pBufferStart;


/*
 * Setup a pabdef
 */

    bError = PutPabDef(&pBuffer, 
                       1, 
                       (WORD)(wCDBufferLength - (pBuffer - pBufferStart)),
                       JUSTIFY_LEFT);
    
/*
 * Leave if error returned...
 */
    if (bError == FALSE)
    {
        sprintf(errMsg, "Error: Unable to create PABDEFINITION.\n");
        goto Exit5;
    }

/*
 * Setup a pabdef
 */

    bError = PutPabDef(&pBuffer, 
                       2, 
                       (WORD)(wCDBufferLength - (pBuffer - pBufferStart)),
                       JUSTIFY_CENTER);
    
/*
 * Leave if error returned...
 */
    if (bError == FALSE)
    {
        sprintf(errMsg, "Error: Unable to create PABDEFINITION.\n");
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
        sprintf(errMsg, "Error: Unable to create PABREFERENCE.\n");
        goto Exit5;
    }


/***********************************************************************
 *
 *  Create the first field in the form of type TYPE_COMPOSITE for 
 *  Gif Image (aka Rich Text).
 *
 ***********************************************************************/

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
    CDField.DataType = TYPE_COMPOSITE;
    CDField.ListDelim = LDDELIM_SEMICOLON;

/*
 *  The NumberFormat paramter is not used for rich text fields, so clear it.
 */    

    CDField.NumberFormat.Digits     = 0;
    CDField.NumberFormat.Format     = 0;
    CDField.NumberFormat.Attributes = 0;
    CDField.NumberFormat.Unused     = 0;

/*
 *  The TimeFormat paramter is not used for rich text fields, so clear it.
 */    

    CDField.TimeFormat.Date      = 0;
    CDField.TimeFormat.Time      = 0;
    CDField.TimeFormat.Zone      = 0;
    CDField.TimeFormat.Structure = 0;

/*
 *  Set up FontID
 */    
   
    pFontFields = (FONTIDFIELDS *)&CDField.FontID;
    pFontFields->Face  = FONT_FACE_SWISS; 
    pFontFields->Attrib = ISBOLD;
    pFontFields->Color = NOTES_COLOR_BLACK;
    pFontFields->PointSize  = 18;

    CDField.DVLength = 0;  /* No default value formula   */
    CDField.ITLength = 0; /* No Input translation formula   */
    CDField.TabOrder = 0;
    CDField.IVLength = 0; /* No Input Validity Check formula   */

    CDField.NameLength = (strlen(GifImageFieldName));
    CDField.DescLength = (strlen(GifImageDescription));
    CDField.TextValueLength = 0;

    CDField.Header.Length = ODSLength( _CDFIELD ) +
                            CDField.DVLength +
                            CDField.ITLength +
                            CDField.IVLength +
                            CDField.NameLength +
                            CDField.DescLength +
                            CDField.TextValueLength;    

/*
 *  Ensure length of CDFIELD item is even.
 */
 
    if (CDField.Header.Length % 2)
        CDField.Header.Length++;
 
/*
 *  Convert the CDFIELD item to Domino and Notes Canonical format and store it in
 *  the ODS buffer. pBufferStart will remain pointing to the beginning of
 *  the buffer; pBuffer will point to the next available byte.
 */

    ODSWriteMemory( (void far * far *)&pBuffer, _CDFIELD,
                    (void far *) &CDField, 1 );

/*
 *  Copy the field name and description to the buffer.
 */
    
    memcpy( pBuffer, GifImageFieldName, CDField.NameLength );
    pBuffer += CDField.NameLength;

    memcpy( pBuffer, GifImageDescription, CDField.DescLength );
    pBuffer += CDField.DescLength;

/*
 *  Make sure the length of the CD record is an even number so the next CD
 *  record begins on an even byte boundary.
 */

    if ((pBuffer - pBufferStart) %2)
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

/*
 *  Import a Gif image into the main form.
 */
    pBuffer = ImportGifImage(szImage_LotusGif, pBuffer, width, height);

    if (!pBuffer)
        goto Exit5;

 /*********************************************************************
 *
 *  Create the next field in the form.  This field will be an editable
 *  field of type TYPE_TEXT.
 *
 **********************************************************************/
  
/*
 *  Insert two CDPARAGRAPH items so that this field is separated from
 *  the previous field by a blank line.
 */
 
    bError = PutPara(&pBuffer,
                    (WORD) (wCDBufferLength - (pBuffer - pBufferStart)));
             
    if (bError == FALSE)
    {
        sprintf(errMsg, "Error: Unable to create PARAGRAPH.\n");
        goto Exit5;
    }
    
    bError = PutPara(&pBuffer,
                     (WORD) (wCDBufferLength - (pBuffer - pBufferStart)));
             
    if (bError == FALSE)
    {
        sprintf(errMsg, "Error: Unable to create PARAGRAPH.\n");
        goto Exit5;
    }

/*
 * Now add a pabref
 */

    bError = PutPabRef(&pBuffer, 
                       2, 
                       (WORD)(wCDBufferLength - (pBuffer - pBufferStart)));

/*
 * Leave if error returned...
 */

    if (bError == FALSE)

    {
        sprintf(errMsg, "Error: Unable to create PABREFERENCE.\n");
        goto Exit5;
    }


/*********************************************************************
 *
 *  Create the next field in the form.  This field will be an editable
 *  field of type TYPE_TEXT.
 *
 **********************************************************************/


/*
 * Insert a string into the rich text field.  This string contains the text
 * of this field's label.
 */

    bError = PutText (&pBuffer,
                     TextString,
                     (WORD) (wCDBufferLength - (pBuffer - pBufferStart)));

    if (bError == FALSE)
    {
        sprintf(errMsg, "Error: Unable to create CDTEXT.\n");
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


    if (sError = NSFFormulaCompile(NULL,
                                   0,
                                   TextDefValFormula,
                                   (WORD) strlen(TextDefValFormula),
                                   &hTextDefValFormula,
                                   &wTextDefValFormulaLen,
                                   &wdc, &wdc, &wdc,
                                   &wdc, &wdc))
    {
        sprintf(errMsg, "Error: Unable to create CDTEXT Formula Default Value.\n");
        goto Exit5;
    }

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
        sprintf(errMsg, "Error: Unable to create CDTEXT.\n");
        goto Exit5;
    }


/*
 *  Fill in the rest of the CDFIELD item.
 */
    
    CDField.DVLength = wTextDefValFormulaLen;
    CDField.ITLength = wTextInputTransFormulaLen;
    CDField.TabOrder = 0;
    CDField.IVLength = 0;  /* No Input Validity Check formula  */
    CDField.NameLength = strlen(TextFieldName);
    CDField.DescLength = strlen(TextDescription);
    
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
 *  Convert the CDFIELD item to Domino and Notes Canonical format and store it in
 *  the ODS buffer. pBufferStart will remain pointing to the beginning of
 *  the buffer; ODSWriteMemory advances pBuffer to point to the next byte
 *  after the CDFIELD record in the buffer.
 */

    ODSWriteMemory( (void far * far *)&pBuffer, _CDFIELD, 
          (void far *)&CDField, 1 );

/*
 *  Get a pointer to the compiled input translation formula.
 */

    pTextDefValFormula = OSLock( char, hTextDefValFormula );

 /*
  *  First, set the input translation formula.
  */

    memcpy( pBuffer, pTextDefValFormula, wTextDefValFormulaLen );
    pBuffer += CDField.DVLength;
    /* Unlock, free memory for formula */
    OSUnlockObject(hTextDefValFormula); 
    OSMemFree(hTextDefValFormula);

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

    memcpy( pBuffer, TextFieldName, CDField.NameLength );
    pBuffer += CDField.NameLength;

/*
 *  And set the field description.
 */

    memcpy( pBuffer, TextDescription, CDField.DescLength );
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
 *  Now define a field of type TYPE_NUMBER.
 *
 ***********************************************************************/

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
        sprintf(errMsg, "Error: Unable to create PARAGRAPH.\n");
        goto Exit5;    
    }
    
    bError = PutPara(&pBuffer,
                     (WORD) (wCDBufferLength - (pBuffer - pBufferStart)));
             
    if (bError == FALSE)
    {
        sprintf(errMsg, "Error: Unable to create PARAGRAPH.\n");
        goto Exit5;
    }


/*
 * Insert a string into the rich text field.  This string contains the text
 * of this field's label.
 */

    bError = PutText (&pBuffer,
                      NumberString,
                      (WORD)(wCDBufferLength - (pBuffer - pBufferStart)));


    if (bError == FALSE)
    {
        sprintf(errMsg, "Error: Unable to create CDTEXT.\n");
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
 * Create a CDFIELD structure to define this number field.
 */

    CDField.Header.Signature = SIG_CD_FIELD;
    CDField.Flags = FEDITABLE;
    CDField.DataType = TYPE_NUMBER;
    CDField.ListDelim = LDDELIM_SEMICOLON;

/*
 *  Set up Number format
 */    

    CDField.NumberFormat.Digits     = 6;
    CDField.NumberFormat.Format     = NFMT_GENERAL;
    CDField.NumberFormat.Attributes = 0;
    CDField.NumberFormat.Unused     = 0;

/*
 *  The TimeFormat paramter is not used for number fields, so clear it.
 */    

    CDField.TimeFormat.Date      = 0;
    CDField.TimeFormat.Time      = 0;
    CDField.TimeFormat.Zone      = 0;
    CDField.TimeFormat.Structure = 0;

/*
 *  Set up FontID
 */    

    pFontFields = (FONTIDFIELDS *)&CDField.FontID;

    pFontFields->Face  = FONT_FACE_SWISS; 
    pFontFields->Attrib = 0;
    pFontFields->Color = NOTES_COLOR_BLACK;
    pFontFields->PointSize  = 10;

/*
 *  Fill in the rest of the CDFIELD item.
 */

    CDField.DVLength = 0;  /* No default value formula     */
    CDField.ITLength = 0;  /* No Input translation formula */
    CDField.TabOrder = 0;
    CDField.IVLength = 0;  /* No Input Validity Check formula */
    

    CDField.NameLength = strlen(NumberFieldName);
    CDField.DescLength = strlen(NumberDescription);
    
    CDField.TextValueLength = 0; /* Not a text list.   */

    CDField.Header.Length = ODSLength( _CDFIELD ) +
                             CDField.DVLength +
                             CDField.ITLength +
                             CDField.IVLength +
                             CDField.NameLength +
                             CDField.DescLength +
                             CDField.TextValueLength;    

/*
 *  Make sure total length of CDFIELD record is even.
 */
 
    if (CDField.Header.Length % 2)
        CDField.Header.Length++;

/*
 *  Convert the CDFIELD item to Domino and Notes Canonical format and store it in
 *  the ODS buffer. pBufferStart will remain pointing to the beginning of
 *  the buffer; pBuffer will point to the next available byte.
 */

    ODSWriteMemory( (void far * far *)&pBuffer, _CDFIELD, 
                    (void far *)&CDField, 1 );
    
/*
 *  Now set the field name.
 */

    memcpy( pBuffer, NumberFieldName, CDField.NameLength );
    pBuffer += CDField.NameLength;

/*
 *  And set the field description.
 */

    memcpy(pBuffer, NumberDescription, CDField.DescLength );
    pBuffer += CDField.DescLength;

/*
 * If this CD record is an odd number of bytes long, increment pBuffer
 * ensure the next CD record begins on an even byte boundary.
 */

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
 *  Now define a field of type TYPE_TIME.
 *
 ***********************************************************************/

/*
 *  Insert two CDPARAGRAPH items so that this field is separated from
 *  the previous field by a blank line.
 */

    bError = PutPara(&pBuffer,
                    (WORD) (wCDBufferLength - (pBuffer - pBufferStart)));
             
    if (bError == FALSE)
    {
        sprintf(errMsg, "Error: Unable to create PARAGRAPH.\n");
        goto Exit5;
    }
    
    bError = PutPara(&pBuffer,
                     (WORD) (wCDBufferLength - (pBuffer - pBufferStart)));
             
    if (bError == FALSE)
    {
        sprintf(errMsg, "Error: Unable to create PARAGRAPH.\n");
        goto Exit5;
    }

/*
 * Insert a string into the rich text field.  This string contains the text
 * of this field's label.
 */

    bError = PutText (&pBuffer,
                      TimeString,
                      (WORD) (wCDBufferLength - (pBuffer - pBufferStart)));

    if (bError == FALSE)
    {
        sprintf(errMsg, "Error: Unable to create CDTEXT.\n");
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
 * Create a CDFIELD structure to define the Time field.
 */

    CDField.Header.Signature = SIG_CD_FIELD;
    CDField.Flags = FEDITABLE;
    CDField.DataType = TYPE_TIME;
    CDField.ListDelim = LDDELIM_SEMICOLON;

/*
 *  The NumberFormat paramter is not used for time fields, so clear it.
 */    
 
    CDField.NumberFormat.Digits     = 0;
    CDField.NumberFormat.Format     = 0;
    CDField.NumberFormat.Attributes = 0;
    CDField.NumberFormat.Unused     = 0;

/*
 *  Set up Time format
 */    

    CDField.TimeFormat.Date = TDFMT_FULL;
    CDField.TimeFormat.Time = TTFMT_PARTIAL;
    CDField.TimeFormat.Zone = TZFMT_NEVER;
    CDField.TimeFormat.Structure = TSFMT_DATETIME;

/*
 *  Set up FontID
 */    
   
    pFontFields = (FONTIDFIELDS *)&CDField.FontID;
    pFontFields->Face  = FONT_FACE_SWISS; 
    pFontFields->Attrib = ISBOLD | ISITALIC;
    pFontFields->Color = NOTES_COLOR_BLACK;
    pFontFields->PointSize  = 10;

/*
 * Compile the default value formula.  The formula will set the
 * default value of the time field to today's date.
 */

    if (sError = NSFFormulaCompile(NULL,
                                   0,
                                   TimeDefValFormula,
                                   (WORD) strlen(TimeDefValFormula),
                                   &hTimeDefValFormula,
                                   &wTimeDefValFormulaLen,
                                   &wdc, &wdc, &wdc,
                                   &wdc, &wdc))
    {
        sprintf(errMsg, "Error: Unable to compile formula.\n");
        goto Exit5;
    }

/*
 *  Fill in the rest of the CDFIELD item.
 */

    CDField.DVLength = wTimeDefValFormulaLen;
    CDField.ITLength = 0; /* No Input translation formula   */
    CDField.TabOrder = 0;
    CDField.IVLength = 0; /* No Input Validity Check formula   */
    CDField.TextValueLength = 0;
    CDField.NameLength = strlen(TimeFieldName);
    CDField.DescLength = strlen(TimeDescription);


    CDField.Header.Length = ODSLength(_CDFIELD) +
                            CDField.DVLength +
                            CDField.ITLength +
                            CDField.IVLength +
                            CDField.NameLength +
                            CDField.DescLength +
                            CDField.TextValueLength;    

/*
 *  Make sure the total length of CDFIELD item is even.
 */
 
    if (CDField.Header.Length % 2)
        CDField.Header.Length++;
                            
/*
 *  Convert the CDFIELD item to Domino and Notes Canonical format and store it in
 *  the ODS buffer. pBufferStart will remain pointing to the beginning of
 *  the buffer; pBuffer will point to the next available byte.
 */

    ODSWriteMemory( (void far * far *)&pBuffer, _CDFIELD, 
                    (void far *)&CDField, 1);
    
/*
 *  Get a pointer ot the compiled Default Value formula.
 */
    
    pTimeDefValFormula = (char*) OSLockObject(hTimeDefValFormula);

/*
 *  Copy the default value formula to the buffer.
 */

    memcpy( pBuffer, pTimeDefValFormula, wTimeDefValFormulaLen );
    pBuffer += wTimeDefValFormulaLen;
                                  /* Unlock, free memory for formula */
    OSUnlockObject(hTimeDefValFormula);
    OSMemFree(hTimeDefValFormula); 
    
/*
 *  Now set the field name.
 */

    memcpy( pBuffer, TimeFieldName, CDField.NameLength );
    pBuffer += CDField.NameLength;

/*
 *  And set the field description.
 */

    memcpy( pBuffer, TimeDescription, CDField.DescLength );
    pBuffer += CDField.DescLength;

/* Ensure record length is even */

    if ((pBuffer-pBufferStart) % 2)
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
 *  Definition of a Time field is complete.  Now define field of
 *  type TYPE_COMPOSITE (aka Rich Text).
 *
 ***********************************************************************/

/*
 *  Insert two CDPARAGRAPH items so that this field is separated from
 *  the previous field by a blank line.
 */
 
    bError = PutPara(&pBuffer,
                    (WORD) (wCDBufferLength - (pBuffer - pBufferStart)));
             
    if (bError == FALSE)
    {
        sprintf(errMsg, "Error: Unable to create PARAGRAPH.\n");
        goto Exit5;
    }
    
    bError = PutPara(&pBuffer,
                     (WORD) (wCDBufferLength - (pBuffer - pBufferStart)));
             
    if (bError == FALSE)
    {
        sprintf(errMsg, "Error: Unable to create PARAGRAPH.\n");
        goto Exit5;
    }

/*
 * Insert a string into the rich text field.  This string contains the text
 * of this field's label.
 */

    bError = PutText (&pBuffer,
                      RichTextString,
                      (WORD) (wCDBufferLength - (pBuffer - pBufferStart)));

    if (bError == FALSE)
    {
        sprintf(errMsg, "Error: Unable to create CDTEXT.\n");
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
    CDField.DataType = TYPE_COMPOSITE;
    CDField.ListDelim = LDDELIM_SEMICOLON;

/*
 *  The NumberFormat paramter is not used for rich text fields, so clear it.
 */    

    CDField.NumberFormat.Digits     = 0;
    CDField.NumberFormat.Format     = 0;
    CDField.NumberFormat.Attributes = 0;
    CDField.NumberFormat.Unused     = 0;

/*
 *  The TimeFormat paramter is not used for rich text fields, so clear it.
 */    
                                          
    CDField.TimeFormat.Date      = 0;
    CDField.TimeFormat.Time      = 0;
    CDField.TimeFormat.Zone      = 0;
    CDField.TimeFormat.Structure = 0;

/*
 *  Set up FontID
 */    
   
    pFontFields = (FONTIDFIELDS *)&CDField.FontID;
    pFontFields->Face  = FONT_FACE_SWISS; 
    pFontFields->Attrib = ISBOLD;
    pFontFields->Color = NOTES_COLOR_BLACK;
    pFontFields->PointSize  = 18;

    CDField.DVLength = 0;  /* No default value formula   */
    CDField.ITLength = 0; /* No Input translation formula   */
    CDField.TabOrder = 0;
    CDField.IVLength = 0; /* No Input Validity Check formula   */

    CDField.NameLength = (strlen(RichTextFieldName));
    CDField.DescLength = (strlen(RichTextDescription));
    CDField.TextValueLength = 0;

    CDField.Header.Length = ODSLength( _CDFIELD ) +
                            CDField.DVLength +
                            CDField.ITLength +
                            CDField.IVLength +
                            CDField.NameLength +
                            CDField.DescLength +
                            CDField.TextValueLength;    

/*
 *  Ensure length of CDFIELD item is even.
 */
 
    if (CDField.Header.Length % 2)
        CDField.Header.Length++;
 
/*
 *  Convert the CDFIELD item to Domino and Notes Canonical format and store it in
 *  the ODS buffer. pBufferStart will remain pointing to the beginning of
 *  the buffer; pBuffer will point to the next available byte.
 */

    ODSWriteMemory( (void far * far *)&pBuffer, _CDFIELD,
                    (void far *) &CDField, 1 );

/*
 *  Copy the field name and description to the buffer.
 */
    
    memcpy( pBuffer, RichTextFieldName, CDField.NameLength );
    pBuffer += CDField.NameLength;

    memcpy( pBuffer, RichTextDescription, CDField.DescLength );
    pBuffer += CDField.DescLength;

/*
 *  Make sure the length of the CD record is an even number so the next CD
 *  record begins on an even byte boundary.
 */

    if ((pBuffer - pBufferStart) %2)
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
 *  Definition of a rich text field is complete. Now create a KEYWORD field. 
 *
 **********************************************************************/

/*
 *  Insert two CDPARAGRAPH items so that this field is separated from
 *  the previous field by a blank line.
 */
 
    bError = PutPara(&pBuffer, 
                     (WORD) (wCDBufferLength - (pBuffer - pBufferStart)));

    if (bError == FALSE)
    {
        sprintf(errMsg, "Error: Unable to create PARAGRAPH.\n");
        goto Exit5;
    }

    bError = PutPara(&pBuffer, 
                     (WORD) (wCDBufferLength - (pBuffer - pBufferStart)));
             
    if (bError == FALSE)
    {
        sprintf(errMsg, "Error: Unable to create PARAGRAPH.\n");
        goto Exit5;
    }

/*
 * Insert a string into the rich text field.  This string contains the text
 * of this field's label.
 */

    bError = PutText (&pBuffer,
                      KeyWordString,
                      (WORD) (wCDBufferLength - (pBuffer - pBufferStart)));

    if (bError == FALSE)
    {
        sprintf(errMsg, "Error: Unable to create PARAGRAPH.\n");
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
 * Create a CDFIELD structure to define this keyword field.
 */

    CDField.Header.Signature = SIG_CD_FIELD;
    CDField.Flags = FKEYWORDS | FEDITABLE;
    CDField.DataType = TYPE_TEXT_LIST;
    CDField.ListDelim = LDDELIM_SEMICOLON;

/*
 *  The NumberFormat paramter is not used for keyword fields, so clear it.
 */    

    CDField.NumberFormat.Digits     = 0;
    CDField.NumberFormat.Format     = 0;
    CDField.NumberFormat.Attributes = 0;
    CDField.NumberFormat.Unused     = 0;

/*
 *  The TimeFormat paramter is not used for keyword fields, so clear it.
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
    pFontFields->PointSize  = 10;

    CDField.DVLength = 0;  /* No default value formula   */
    CDField.ITLength = 0;  /* No Input translation formula   */
    CDField.TabOrder = 0;
    CDField.IVLength = 0;  /* No Input Validity Check formula   */

    CDField.NameLength = strlen(KeyWordFieldName);
    CDField.DescLength = strlen(KeyWordDescription);

/*
 *  Calculate the ODS length of the text list that will hold the keywords,
 *  and set the CDField.TextValueLength appropriately.
 */

    wTextListLen = ODSLength(_LIST)    + /* length of LIST header */    
                   3 * ODSLength(_WORD) + /* array of 3 length words. */
                   wKeyword1Len         + /* length of 1st string. */
                   wKeyword2Len         + /* length of 2nd string. */
                   wKeyword3Len;          /* length of 3rd string. */

    CDField.TextValueLength = wTextListLen;

    CDField.Header.Length = ODSLength( _CDFIELD ) +
                            CDField.DVLength +
                            CDField.ITLength +
                            CDField.IVLength +
                            CDField.NameLength +
                            CDField.DescLength +
                            CDField.TextValueLength;    

/*
 *  Make sure total length of CDFIELD item is even.
 */
 
    if (CDField.Header.Length % 2)
        CDField.Header.Length++;

/*
 *  Convert the CDFIELD item to Domino and Notes Canonical format and store it in
 *  the ODS buffer. pBufferStart will remain pointing to the beginning of
 *  the buffer; pBuffer will point to the next available byte.
 */

    ODSWriteMemory( (void far * far *)&pBuffer, _CDFIELD, 
                    (void far *)&CDField, 1 );

/*
 *  Now, copy the field name into the buffer.
 */
 
    memcpy( pBuffer, KeyWordFieldName, CDField.NameLength );
    pBuffer += CDField.NameLength;

/*
 *  Next, fill in field description info
 */
 
    memcpy( pBuffer, KeyWordDescription, CDField.DescLength );
    pBuffer += CDField.DescLength;

/*
 *  Now, fill in text list of keywords.  In this example, there are
 *  3 keywords.  They are defined in the variables Keyword1, Keyword2, and
 *  Keyword3. The keywords in the textlist should not be null terminated,
 *  so the null terminator is not copied into the list.
 */
 
    /*  Set number of keywords.  */
    sList.ListEntries = 3;    
    ODSWriteMemory( (void far * far *)&pBuffer, _LIST, 
                    (void far *)&sList, 1 );
    
    ODSWriteMemory( (void far * far *)&pBuffer, _WORD, 
                    (void far *)&wKeyword1Len, 1 );

    ODSWriteMemory( (void far * far *)&pBuffer, _WORD, 
                    (void far *)&wKeyword2Len, 1 );

    ODSWriteMemory( (void far * far *)&pBuffer, _WORD, 
                    (void far *)&wKeyword3Len, 1 );

    memcpy( pBuffer, Keyword1, wKeyword1Len ); /* Copy 1st keyword */
    pBuffer += wKeyword1Len;

    memcpy( pBuffer, Keyword2, wKeyword2Len ); /* Copy 2nd keyword */
    pBuffer += wKeyword2Len;

    memcpy( pBuffer, Keyword3, wKeyword3Len ); /* Copy 3rd keyword */
    pBuffer += wKeyword3Len;

/*
 * Ensure the length of the CDFIELD record is even.
 */

    if ((pBuffer - pBufferStart) %2)
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
        sprintf(errMsg, "Error: Unable to create $BODY item.\n");
        goto Exit5;
    }

/**************************************************************************
 *
 *  Now create the $FIELDS item. This item is simply a text list containing
 *  the names of all of the fields in the form.
 *
 **************************************************************************/

/*
 *  Set the first item in the text list to be the name of the text field.
 */
 
    if (sError = NSFItemCreateTextList(hNote,
                                       "$FIELDS",
                                       TextFieldName,
                                       (WORD) strlen(TextFieldName)))
    {
        sprintf(errMsg, "Error: Unable to create $FIELDS item.\n");
        goto Exit5;
    }

/*
 *  Now add the name of the number field to the text list.
 */
 
    if (sError = NSFItemAppendTextList(hNote,
                                       "$FIELDS",
                                       NumberFieldName,
                                       (WORD) strlen(NumberFieldName),
                                       TRUE))
    {
        sprintf(errMsg, "Error: Unable to append to $FIELDS item.\n");
        goto Exit5;
    }

/*
 *  Now add the name of the time field to the text list.
 */
 
    if (sError = NSFItemAppendTextList(hNote,
                                       "$FIELDS",
                                       TimeFieldName,
                                       (WORD) strlen(TimeFieldName),
                                       TRUE))
    {
        sprintf(errMsg, "Error: Unable to append to $FIELDS item.\n");
        goto Exit5;
    }

/*
 *  Now add the name of the rich text field to the text list.
 */
 
    if (sError = NSFItemAppendTextList(hNote,
                                       "$FIELDS",
                                       RichTextFieldName,
                                       (WORD) strlen(RichTextFieldName),
                                       TRUE))
    {
        sprintf(errMsg, "Error: Unable to append to $FIELDS item.\n");
        goto Exit5;
    }

/*
 *  Now add the name of the keyword field to the text list.
 */
 
    if (sError = NSFItemAppendTextList(hNote,
                                       "$FIELDS",
                                       KeyWordFieldName,
                                       (WORD) strlen(KeyWordFieldName),
                                       TRUE))
                                       
    {
        sprintf(errMsg, "Error: Unable to append to $FIELDS item.\n");
        goto Exit5;
    }


/*
 *  Now add the name of the Gif image rich text field to the text list.
 */
 
    if (sError = NSFItemAppendTextList(hNote,
                                       "$FIELDS",
                                       GifImageFieldName,
                                       (WORD) strlen(GifImageFieldName),
                                       TRUE))
    {
        sprintf(errMsg, "Error: Unable to append to $FIELDS item.\n");
        goto Exit5;
    }


/**************************************************************************
 *
 *  Now create a custom designed action that contains a formula.  When the
 *  user clicks the Action button on the Test ActionBar, a dialog box will
 *  be displayed.
 *
 **************************************************************************/


/*  
 * compile the formula.
 */
    if (error = NSFFormulaCompile(NULL, 0, Formula, (WORD)strlen(Formula),
                                  &hFormula, &FormulaLen, &wdc, &wdc, 
                                  &wdc, &wdc, &wdc))
    {
        sprintf(errMsg, "Error compiling formula.\n");
        goto Exit5;
    }

    FormulaLen += FormulaLen%2;
    TitleLen = (WORD)strlen(ACTIONTITLE);
    TitleLen += TitleLen%2;
/*
 * There is going to be only one ACTION button on the ACTION BAR.
 */
    FormActionPtr = (BYTE *)malloc(ODSLength(_CDACTIONBAR) +
                    ODSLength(_CDACTION) +
                    TitleLen +
                    FormulaLen);

    if (FormActionPtr == (BYTE *)NULL)
    {
        sprintf(errMsg, "Error:  Unable to allocate memory.\n");
        goto Exit5;
    }

    buff_ptr = FormActionPtr;

/*
 * Fill in ActionBar structure. 
 */
    /* init the structure... */
    memset(&ActionBar, 0, sizeof(CDACTIONBAR));

    ActionBar.Header.Signature = SIG_CD_ACTIONBAR;
    ActionBar.Header.Length    = (BYTE)ODSLength(_CDACTIONBAR);
    ActionBar.BackColor        = NOTES_COLOR_BLUE;
    ActionBar.LineColor        = NOTES_COLOR_YELLOW;
    ActionBar.LineStyle        = ACTION_LINE_TWO;
    ActionBar.BorderStyle      = ACTION_BORDER_ABS;
    ActionBar.BorderWidth      = 0;
    ActionBar.dwFlags          = 0;
    ActionBar.ShareID          = 0;
    ActionBar.FontID           = 0;

    ODSWriteMemory(&buff_ptr, _CDACTIONBAR, &ActionBar, 1);

/*
 * Fill in Action structure.
 */

    /* init the structure... */
    memset(&Action, 0, sizeof(CDACTION));

    Action.Header.Signature = SIG_CD_ACTION;
    Action.Header.Length = ODSLength(_CDACTION) + TitleLen + FormulaLen;
    Action.Type          = ACTION_RUN_FORMULA;
    Action.IconIndex     = 2;
    Action.Flags         = 0x19F;
    Action.TitleLen      = TitleLen;
    Action.FormulaLen    = 0;     /* No "Hide-When" formula */
    Action.ShareId       = 0;

    ODSWriteMemory(&buff_ptr, _CDACTION, &Action, 1);

    memcpy((char *)buff_ptr, ACTIONTITLE, Action.TitleLen);
    buff_ptr += Action.TitleLen;
    memcpy((char *)buff_ptr, OSLockObject(hFormula), FormulaLen);
    buff_ptr += FormulaLen; 

    buff_len = (DWORD)(buff_ptr - FormActionPtr);

/*
 * Append Action Item to Note.
 */
    if (error = NSFItemAppend(
                              hNote,            /* handle to note to append to */
                              ITEM_SIGN,        /* item flags */
                              ACTION_ITEM,      /* item name $ACTIONS */
                              (WORD)strlen(ACTION_ITEM),
                              TYPE_COMPOSITE,      /* item type */
                              FormActionPtr,    /* item value */
                              (DWORD)buff_len))
    {
        sprintf(errMsg, "Error:  NSFItemAppend\n");
        goto Exit5;
    }
    free (FormActionPtr);

/*******************************************************************
 *
 * All fields for this form have been defined. Set the NOTE_CLASS to
 * NOTE_CLASS_FORM, then update the note.
 *
 ******************************************************************/

    NSFNoteSetInfo( hNote, _NOTE_CLASS, &ClassForm );

    sError = NSFNoteUpdate( hNote, 0 );

Exit5:

    OSUnlockObject (hMem);
    OSMemFree (hMem);

Exit4:
    
    NSFNoteClose( hNote );
    
Exit3:

    return( sError );

}


/************************************************************************

    FUNCTION:    CreateSubForm

    PURPOSE:     Create a sub form in the database.

*************************************************************************/
STATUS LNPUBLIC CreateSubForm()
{
    NOTEID      FormNoteID; 
    NOTEHANDLE  hNewNote; 
    STATUS      sError = NOERROR;

    WORD        wCDBufferLength = MAXONESEGSIZE; /* Length of current CD buffer */
    
    CDFIELD     CDField;
    char far    *pBufferStart, far *pBuffer;
    char far    FlagData[5];
    
    char        SubFormName[] = "Test SubForm 1";
    char        GifImageString[]  = "Gif Image:";
    char        GifImageFieldName2[]   = "GifImageField2";
    char        GifImageDescription[] =  "This is a Gif Image Field for a Sub Form";
    
    BOOL        bError;
    DHANDLE     hMem;
    FONTIDFIELDS *pFontFields;
    WORD        ClassForm = NOTE_CLASS_FORM;

    CDDOCUMENT  InfoStruct;
    BYTE        bInfoLength;

    char far    *pInfoBufStart, far *pInfoBuf;
    DHANDLE     hInfoBuf;
    WORD        wInfoBufLen;
    WORD        wLen;
    DWORD       dwItemLength;

    STATUS      error = NOERROR;              /* error code from API calls */

    /* image width and height */
    WORD        width =0x0064;
    WORD        height=0x0023;

    CDBEGINRECORD CDBegin;
    CDENDRECORD   CDEnd;
    CDEXT2FIELD   CDExt2Field;

/*
 * Check to see if we can find a form with this name already (disallow duplicates)
 */

    if ((sError = NIFFindDesignNote(hDB, SubFormName, NOTE_CLASS_FORM, 
                                    &FormNoteID)) != ERR_NOT_FOUND)
    {
        sprintf(errMsg, "Error: Form named '%s' already exists in database. Duplicates not allowed.\n",
             SubFormName );
        sError = ERR_ALREADY_EXIST;
        goto Exit3;
    }

/*
 * Now create a note in database
 */

    if (sError = NSFNoteCreate(hDB, &hNewNote))
    {
        sprintf(errMsg, "Error: Unable to create note in database.\n" );
        goto Exit3;
    }

/*******************************************************
 *
 * Set the form name. ($TITLE field)
 *
 *******************************************************/
     
    if (sError = NSFItemSetText(hNewNote,
                                ITEM_NAME_TEMPLATE_NAME,
                                SubFormName,
                                MAXWORD))
    {
        sprintf(errMsg,
                "Error: Unable to set text item '%s' in form note.\n",
                ITEM_NAME_TEMPLATE_NAME );
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
        sprintf(errMsg, "Error: Unable to allocate %d bytes memory.\n",
                wInfoBufLen);
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
    /* init the structure... */
    memset(&InfoStruct, 0, sizeof(CDDOCUMENT));
    
    bInfoLength = (BYTE) ODSLength(_CDDOCUMENT);
 
    InfoStruct.Header.Signature = SIG_CD_DOCUMENT;
    InfoStruct.Header.Length = bInfoLength;   /* ODSLength(_CDDOCUMENT);*/
    InfoStruct.PaperColor = NOTES_COLOR_WHITE;
    InfoStruct.FormFlags = 0;
    InfoStruct.NotePrivileges = 0;
    InfoStruct.FormFlags2 = 0;
    InfoStruct.InherFieldNameLength = 0;
    InfoStruct.PaperColorExt = 0;
    InfoStruct.FormFlags3 = 0;
    InfoStruct.Spare[0] = 0;

/*
 *  Convert the CDDOCUMENT item to Domino and Notes Canonical format and store it in
 *  the ODS buffer. pInfoBufStart remains pointing to the beginning of
 *  the buffer. ODSWriteMemory advances pInfoBuf to point to the next
 *  byte after the end of the CDDOCUMENT record.
 */

    ODSWriteMemory( (void far * far *)&pInfoBuf, _CDDOCUMENT, 
                    &InfoStruct, 1 );
     
/*
 * Append the ODS buffer to the note as the $INFO field
 */
    dwItemLength = (DWORD) wInfoBufLen;
    
    sError = NSFItemAppend(hNewNote,
                           0,
                           ITEM_NAME_DOCUMENT,
/*                         strlen(ITEM_NAME_DOCUMENT), */
             (WORD) strlen(ITEM_NAME_DOCUMENT),
                           TYPE_COMPOSITE,
                           (void far *)pInfoBufStart,
             dwItemLength);
    
/*                           (DWORD) wInfoBufLen); */

    OSUnlockObject(hInfoBuf);
    OSMemFree(hInfoBuf);

    if (sError)
    {
        sprintf(errMsg,
                "Error: Unable to append item '%s' to form note.\n",
                ITEM_NAME_DOCUMENT );
        goto Exit4;
    }

/***********************************************************************
 *
 *  Append the $Flags field to this note to identify the form as a 
 *  subform.
 *
 ***********************************************************************/    

    FlagData[0] = DESIGN_FLAG_ADD;
    FlagData[1] = DESIGN_FLAG_NO_COMPOSE;
    FlagData[2] = DESIGN_FLAG_SUBFORM;
    FlagData[3] = DESIGN_FLAG_HIDE_FROM_V3;
    wLen = 4;

    if (sError = NSFItemAppend(hNewNote,
                               ITEM_SUMMARY,
                               DESIGN_FLAGS,
                               (WORD) strlen (DESIGN_FLAGS),
                               TYPE_TEXT,
                               (void*)&FlagData[0],
                               wLen))
    {
        sprintf(errMsg, "Error: Unable to create $Flags item.\n");
        goto Exit4;
    }


/********************************************************************
 *
 *  Insert "placeholder" items into the form note, one for each item that
 *  will be contained in the form.  The placeholder items are used
 *  by the Domino and Notes user interface to generate the list of field names when
 *  the user chooses "Add Field" from the "Design...Views" dialog box.
 *
 *********************************************************************/

/*
 *  Gif Image item placeholder.
 */

    if (sError = NSFItemAppend(hNewNote,
                               ITEM_PLACEHOLDER,
                               GifImageFieldName2,
                               (WORD) strlen (GifImageFieldName2),
                               TYPE_INVALID_OR_UNKNOWN,
                               NULL,
                               0))
    {
        sprintf(errMsg, 
                "Error: Unable to append placeholder for field '%s' to form note.\n",
                GifImageFieldName2 );
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
        sprintf(errMsg, "Error: Unable to allocate %d bytes memory.\n",
                wCDBufferLength);
        goto Exit4;
    }

/*
 *  Lock global buffer and initialize it.
 */
    
    pBufferStart = (char far *)OSLockObject(hMem);
    memset( pBufferStart, 0, (size_t) wCDBufferLength );
    pBuffer = pBufferStart;

/*
 * Setup a pabdef
 */

    bError = PutPabDef(&pBuffer, 
                       2, 
                       (WORD)(wCDBufferLength - (pBuffer - pBufferStart)),
                       JUSTIFY_RIGHT);
/*
 * Leave if error returned...
 */
    if (bError == FALSE)
    {
        sprintf(errMsg, "Error: Unable to create PABDEFINITION.\n");
        goto Exit5;
    }

/*
 *  Insert two CDPARAGRAPH items so that this field is separated from
 *  the previous field by a blank line.
 */
 
    bError = PutPara(&pBuffer,
                     (WORD) (wCDBufferLength - (pBuffer - pBufferStart)));
             
    if (bError == FALSE)
    {
        sprintf(errMsg, "Error: Unable to create PARAGRAPH.\n");
        goto Exit5;
    }
    
    bError = PutPara(&pBuffer,
                     (WORD) (wCDBufferLength - (pBuffer - pBufferStart)));
             
    if (bError == FALSE)
    {
        sprintf(errMsg, "Error: Unable to create PARAGRAPH.\n");
        goto Exit5;
    }

/*
 * Now add a pabref
 */

    bError = PutPabRef(&pBuffer, 
                       2, 
                       (WORD)(wCDBufferLength - (pBuffer - pBufferStart)));

/*
 * Leave if error returned...
 */

    if (bError == FALSE)

    {
        sprintf(errMsg, "Error: Unable to create PABREFERENCE.\n");
        goto Exit5;
    }

/***********************************************************************
 *
 *  Create the first field in the form of type TYPE_COMPOSITE for 
 *  Gif Image (aka Rich Text).
 *
 ***********************************************************************/
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
    CDField.DataType = TYPE_COMPOSITE;
    CDField.ListDelim = LDDELIM_SEMICOLON;

/*
 *  The NumberFormat paramter is not used for rich text fields, so clear it.
 */    

    CDField.NumberFormat.Digits     = 0;
    CDField.NumberFormat.Format     = 0;
    CDField.NumberFormat.Attributes = 0;
    CDField.NumberFormat.Unused     = 0;

/*
 *  The TimeFormat paramter is not used for rich text fields, so clear it.
 */    

    CDField.TimeFormat.Date      = 0;
    CDField.TimeFormat.Time      = 0;
    CDField.TimeFormat.Zone      = 0;
    CDField.TimeFormat.Structure = 0;

/*
 *  Set up FontID
 */    
   
    pFontFields = (FONTIDFIELDS *)&CDField.FontID;
    pFontFields->Face  = FONT_FACE_SWISS; 
    pFontFields->Attrib = ISBOLD;
    pFontFields->Color = NOTES_COLOR_BLACK;
    pFontFields->PointSize  = 18;

    CDField.DVLength = 0;  /* No default value formula   */
    CDField.ITLength = 0; /* No Input translation formula   */
    CDField.TabOrder = 0;
    CDField.IVLength = 0; /* No Input Validity Check formula   */

    CDField.NameLength = (strlen(GifImageFieldName2));
    CDField.DescLength = (strlen(GifImageDescription));
    CDField.TextValueLength = 0;

    CDField.Header.Length = ODSLength( _CDFIELD ) +
                            CDField.DVLength +
                            CDField.ITLength +
                            CDField.IVLength +
                            CDField.NameLength +
                            CDField.DescLength +
                            CDField.TextValueLength;    

/*
 *  Ensure length of CDFIELD item is even.
 */
 
    if (CDField.Header.Length % 2)
        CDField.Header.Length++;
 
/*
 *  Convert the CDFIELD item to Domino and Notes Canonical format and store it in
 *  the ODS buffer. pBufferStart will remain pointing to the beginning of
 *  the buffer; pBuffer will point to the next available byte.
 */

    ODSWriteMemory( (void far * far *)&pBuffer, _CDFIELD,
                    (void far *) &CDField, 1 );

/*
 *  Copy the field name and description to the buffer.
 */
    
    memcpy( pBuffer, GifImageFieldName2, CDField.NameLength );
    pBuffer += CDField.NameLength;

    memcpy( pBuffer, GifImageDescription, CDField.DescLength );
    pBuffer += CDField.DescLength;

/*
 *  Make sure the length of the CD record is an even number so the next CD
 *  record begins on an even byte boundary.
 */

    if ((pBuffer - pBufferStart) %2)
        pBuffer++;

/*
 *  Import a Gif image into the subform.
 */
 
    pBuffer = ImportGifImage(szImage_DominoGif, pBuffer, width, height);

    if (!pBuffer)
      goto Exit5;

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

    if (sError = NSFItemAppend(hNewNote,
                   0,
                   ITEM_NAME_TEMPLATE,
                   (WORD) strlen (ITEM_NAME_TEMPLATE),
                   TYPE_COMPOSITE,
                   (void*)pBufferStart,
                   pBuffer-pBufferStart))
    {
        sprintf(errMsg, "Error: Unable to create $BODY item.\n");
        goto Exit5;
    }


/*******************************************************************
 *
 * All fields for this form have been defined. Set the NOTE_CLASS to
 * NOTE_CLASS_FORM, then update the note.
 *
 ******************************************************************/

    NSFNoteSetInfo( hNewNote, _NOTE_CLASS, &ClassForm );

    if (sError = NSFNoteUpdate( hNewNote, 0 ))
    {
        sprintf(errMsg, "Error: Unable to update Note.\n");
    }

Exit5:

    OSUnlockObject (hMem);
    OSMemFree (hMem);

Exit4:

    NSFNoteClose( hNewNote );
    
Exit3:
    return( sError );

}

/************************************************************************

    FUNCTION:    CreateFrame

    PURPOSE:     Create a frame set in the database consisting of a View
                     frame and a Document frame.

*************************************************************************/
STATUS LNPUBLIC CreateFrame()
{
    NOTEHANDLE hNewNote = NULLHANDLE; 
    NOTEHANDLE hNote = NULLHANDLE;
    DHANDLE hMem;
    STATUS sError = NOERROR;
    
    char far   *pBufferStart, far *pBuffer;
    char far   FlagData[5];
    char far   FrameLaunchFlag[]="wF";
    
    WORD       wCDBufferLength = MAXONESEGSIZE; /* Length of current CD buffer */
    WORD       ClassForm = NOTE_CLASS_FORM;
    WORD       wLen;

/*
 * First create a note in database
 */

    if (sError = NSFNoteCreate(hDB, &hNewNote))
    {
        sprintf(errMsg, "Error: Unable to create note in database.\n" );
        goto Exit3;
    }

/*******************************************************
 *
 * Set the frame set name. ($TITLE field)
 *
 *******************************************************/
     
    if (sError = NSFItemSetText(hNewNote,
                                ITEM_NAME_TEMPLATE_NAME,
                                FrameName,
                                MAXWORD))
    {
        sprintf(errMsg,
                "Error: Unable to set text item '%s' in form note.\n",
                FrameName );
        goto Exit4;
    }


/*
 * Allocate a buffer in which the frame data will be built
 */
    if (sError = OSMemAlloc (0, wCDBufferLength, &hMem))
    {
        sprintf(errMsg, "Error: Unable to allocate %d bytes memory.\n",
                wCDBufferLength);
        goto Exit4;
    }


/*
 *  Lock global buffer and initialize it.
 */
    
    pBufferStart = (char far *)OSLockObject(hMem);
    memset( pBufferStart, 0, (size_t) wCDBufferLength );
    pBuffer = pBufferStart;

/*
 *  Add a View frame and Document frame to the buffer
 */
 
    pBuffer = AddFrame(&pBuffer, pBufferStart);

    if (!pBuffer)
        goto Exit5;

/***********************************************************************
 *
 *  Append the $Flags field to this note to identify the form as a 
 *  frameset.
 *
 ***********************************************************************/    

    FlagData[0] = DESIGN_FLAG_ADD;
    FlagData[1] = DESIGN_FLAG_NO_COMPOSE;
    FlagData[2] = DESIGN_FLAG_FRAMESET;
    FlagData[3] = DESIGN_FLAG_HIDE_FROM_V3;
    wLen = 4;

    if (sError = NSFItemAppend(hNewNote,
                               ITEM_SUMMARY,
                               DESIGN_FLAGS,
                               (WORD) strlen (DESIGN_FLAGS),
                               TYPE_TEXT,
                               (void*)&FlagData[0],
                               wLen))
    {
        sprintf(errMsg, "Error: Unable to create $Flags item.\n");
        goto Exit5;
    }


/***********************************************************************
 *
 *  Now append the frame data to the note.
 *
 ***********************************************************************/    

    if (sError = NSFItemAppend(hNewNote,
                               0,
                               ITEM_NAME_FRAMESET,
                               (WORD) strlen (ITEM_NAME_FRAMESET),
                               TYPE_COMPOSITE,
                               (void*)pBufferStart,
                               pBuffer-pBufferStart))
    {
        sprintf(errMsg, "Error: Unable to create $FrameSet item.\n");
        goto Exit5;
    }


 /* 
  * Get information to build a NOTELINK structure.  This is used for the View
  * frame to be linked to the Document frame. 
  */

    if (sError = GetNoteLinkInfo())
    {  
        goto Exit5;
    }
    
/*
 *  initialize buffer.
 */
 
    memset( pBufferStart, 0, (size_t) wCDBufferLength );
    pBuffer = pBufferStart;


/*
 *  Add a NOTELINK structure to the buffer
 */
   
    pBuffer = AddNoteLink(&pBuffer, pBufferStart);

    if (!pBuffer)
        goto Exit5;


/***********************************************************************
 *
 *  Now append the buffer to the note.
 *
 ***********************************************************************/    

    if (sError = NSFItemAppend(hNewNote,
                               0,
                               ITEM_NAME_FORMLINK,
                               (WORD) strlen (ITEM_NAME_FORMLINK),
                               TYPE_NOTELINK_LIST,
                               (void*)pBufferStart,
                               pBuffer-pBufferStart))
    {
        sprintf(errMsg, "Error: Unable to create $FormLinks item.\n");
        goto Exit5;
    }


/*******************************************************************
 *
 * All fields for this form have been defined. Set the NOTE_CLASS to
 * NOTE_CLASS_FORM, then update the note.
 *
 ******************************************************************/

    NSFNoteSetInfo( hNewNote, _NOTE_CLASS, &ClassForm );

    if (sError = NSFNoteUpdate( hNewNote, 0 ))
    {
        sprintf(errMsg, "Error: Unable to update Note.\n");
        goto Exit5;
    }


/*
 * When opening the database, we want the information to appear within the new frame we
 * have just added.  To accomplish this we must modify the field flags of the Icon Note that
 * will cause the frame set to launch on opening.
 */

    if (sError = NSFNoteOpen (hDB, NOTE_ID_SPECIAL | NOTE_CLASS_ICON | NOTE_CLASS_DEFAULT, 0, &hNote))
    {
        sprintf(errMsg, "Error: Opening Icon Note.\n" );
        goto Exit5;
    }

    /* add the default frameset to the icon note */
    if (sError = NSFItemSetText(hNote,
                                DEFAULT_FRAMESET_ITEM,
                                FrameName,
                                MAXWORD))
    {
        sprintf(errMsg,
                "Error: Unable to set text item '%s' in icon note.\n",
                DEFAULT_FRAMESET_ITEM );
        goto Exit5;
    }

    /* add the flags to launch the frame */
    if (sError = NSFItemSetText(hNote,
                                DESIGN_FLAGS,
                                FrameLaunchFlag,
                                MAXWORD))
    {
        sprintf(errMsg,
                "Error: Unable to set text item '%s' in icon note.\n",
                DESIGN_FLAGS );
        goto Exit5;
    }

    /* update icon note */
    if (sError = NSFNoteUpdate( hNote, 0 ))
    {
        sprintf(errMsg, "Error: Unable to update icon Note.\n");
    }

Exit5:

    OSUnlockObject (hMem);
    OSMemFree (hMem);

Exit4:

    if (hNewNote)
        NSFNoteClose( hNewNote );

    if (hNote)
        NSFNoteClose( hNote );
    
Exit3:
    return( sError );

}

/************************************************************************

    FUNCTION:    ImportGifImage

    PURPOSE:     Imports a GIF image into a buffer.

*************************************************************************/
char far * ImportGifImage(char *ImageFileName, char far *pBuf, WORD width, WORD height)
{

    FILE            *pImageFD; 
    CDGRAPHIC       CDGraphic;
    CDIMAGEHEADER   CDImageHeader;
    CDIMAGESEGMENT  CDImageSegment;
    int             ImageFDHandle;
    long            ImageFDSize;
    int             numread;
    DWORD           CopyCount;
    DWORD           BytesLeft;
    int             i;  
    WORD            bufsize;
    BYTE            *pSegmentBits;
    struct stat     buf;


  /* open the target Gif image file to import */
    pImageFD = fopen(ImageFileName, "r+b");

    if (!pImageFD)
    {
        sprintf(errMsg, "Error: Unable to open Gif file:%s.\n",ImageFileName);
        return(NULL);
    }
    
    /* get a handle to the file */
#ifdef LINUX
    ImageFDHandle = fileno(pImageFD);
#else
    ImageFDHandle = _fileno(pImageFD);
#endif

    /* get the file length of the file */
    if((fstat(ImageFDHandle, &buf) == -1))
    {
        sprintf(errMsg, "Error: Unable to obtain file size.\n");
        return(NULL);
    }

    ImageFDSize = buf.st_size;

    /* init the structure... */
    memset(&CDGraphic, 0, sizeof(CDGRAPHIC));

    CDGraphic.Header.Signature = SIG_CD_GRAPHIC;
    CDGraphic.Header.Length    = (BYTE)ODSLength(_CDGRAPHIC);
    CDGraphic.Version          = CDGRAPHIC_VERSION3;

    CDGraphic.wReserved        = 0;
    CDGraphic.fResize          = FALSE;
    CDGraphic.DestSize.width   = 0;
    CDGraphic.DestSize.height  = 0;
    CDGraphic.CropSize.width   = 0;
    CDGraphic.CropSize.height  = 0;
    CDGraphic.CropOffset.left  = 0;
    CDGraphic.CropOffset.right = 0;
    CDGraphic.CropOffset.top   = 0;
    CDGraphic.CropOffset.bottom = 0;

    /* write CDGRAPHIC structure to CD buffer */
    ODSWriteMemory(( void far * far* )&pBuf, _CDGRAPHIC, &CDGraphic, 1);

    /* init the structure... */
    memset(&CDImageHeader, 0, sizeof(CDIMAGEHEADER));

    CDImageHeader.Header.Signature = SIG_CD_IMAGEHEADER;
    CDImageHeader.Header.Length    = (BYTE)ODSLength(_CDIMAGEHEADER);
    CDImageHeader.ImageType        = CDIMAGETYPE_GIF;
    CDImageHeader.Width            = width;
    CDImageHeader.Height           = height;
    CDImageHeader.ImageDataSize    = ImageFDSize;
    CDImageHeader.SegCount         = ImageFDSize / IMAGE_SEGSIZE;

    if (CDImageHeader.ImageDataSize % IMAGE_SEGSIZE)
        CDImageHeader.SegCount++;

    if (CDImageHeader.SegCount == 0)
        CDImageHeader.SegCount = 1;

    /* write CDIMAGEHEADER structure to CD buffer */
    ODSWriteMemory(( void far * far* )&pBuf, _CDIMAGEHEADER, &CDImageHeader, 1);

   /* init the structure... */
    memset(&CDImageSegment, 0, sizeof(CDIMAGESEGMENT));

    CDImageSegment.Header.Signature = SIG_CD_IMAGESEGMENT;
    CDImageSegment.Header.Length    = ODSLength(_CDIMAGESEGMENT);

    pSegmentBits = (BYTE *)malloc(IMAGE_SEGSIZE);

    if (!pSegmentBits)
        return(NULL);

    for (i=0,BytesLeft=CDImageHeader.ImageDataSize;i<(int)CDImageHeader.SegCount;i++)
    {
        CopyCount = (BytesLeft < IMAGE_SEGSIZE) ? BytesLeft : IMAGE_SEGSIZE;

        CDImageSegment.DataSize = (int)CopyCount;

        if (CopyCount & 1)
            CDImageSegment.SegSize = (int)CopyCount+ 1;
        else  
            CDImageSegment.SegSize = (int)CopyCount;
     
        CDImageSegment.Header.Length = ODSLength(_CDIMAGESEGMENT)+ CDImageSegment.SegSize;

        /* write CDIMAGESEGMENT structure to CD buffer */
        ODSWriteMemory(( void far * far* )&pBuf, _CDIMAGESEGMENT, &CDImageSegment, 1);
    
        bufsize = ODSLength(_CDIMAGESEGMENT);

        /* read from Gif image file to CD buffer */
        numread = fread((void *)pSegmentBits,sizeof(BYTE),(size_t)CopyCount,pImageFD);

        if (ferror(pImageFD))
        {
            sprintf(errMsg, "Error: Unable to read image data.\n");
            goto Exit0;
        }
        else if (!numread)
            goto Exit0;

        /* write Gif image data */
        memcpy((BYTE *)pBuf, (BYTE *)pSegmentBits, (size_t)numread);

        pBuf += (size_t)numread;

        if (CopyCount & 1)
        {
            *pSegmentBits = '?';
            memcpy(pBuf, pSegmentBits, 1);
            pBuf += sizeof(BYTE);
        }
      
        BytesLeft -= CopyCount; 
    }

Exit0:

    if (fclose(pImageFD))
    {
        sprintf(errMsg, "Error: Unable to close image file.\n");
    } 
    
    if(pSegmentBits)
        free(pSegmentBits);
 
    return(pBuf);
}


/************************************************************************

    FUNCTION:    AddFrame

    PURPOSE:     Adds a Frame set and associated frames into a buffer.

*************************************************************************/
char far * AddFrame(char far * far *pBuf, char far *pBufStart)
{

    CDFRAMESETHEADER  CDFrameSetHeader;
    CDFRAMESET        CDFrameSet;
    CDFRAME           CDFrame;
    CDRESOURCE        CDResource;
    FRAMESETLENGTH    FrameLen1, FrameLen2, FrameLen3; 
    TIMEDATE          TimeDate;
    WORD              LinkID;
    char              FrameName1[]="Frame1";
    char              FrameName2[]="Frame2";
    char              ViewName[]="New View";

    /* init the CDFRAMESETHEADER structure... */
    memset(&CDFrameSetHeader, 0, sizeof(CDFRAMESETHEADER));

    CDFrameSetHeader.Header.Signature = SIG_CD_FRAMESETHEADER;
    CDFrameSetHeader.Header.Length    = (BYTE)ODSLength(_CDFRAMESETHEADER);
    CDFrameSetHeader.Version          = FRAMESETHEADER_VERSION;
    CDFrameSetHeader.RecCount         = 0x0003;
    CDFrameSetHeader.Reserved[0]      = 0;
    CDFrameSetHeader.Reserved[1]      = 0;
    CDFrameSetHeader.Reserved[2]      = 0;
    CDFrameSetHeader.Reserved[3]      = 0;

    /* write CDFRAMESETHEADER structure to CD buffer */
    ODSWriteMemory(( void far * far* )pBuf, _CDFRAMESETHEADER, &CDFrameSetHeader, 1);

    /* init the CDFRAMESET structure... */
    memset(&CDFrameSet, 0, sizeof(CDFRAMESET));

    CDFrameSet.Header.Signature  = SIG_CD_FRAMESET;
    CDFrameSet.Header.Length     = (BYTE)ODSLength(_CDFRAMESET) + ((BYTE)ODSLength(_FRAMESETLENGTH) * 3);
    CDFrameSet.Flags             = 0;
    CDFrameSet.BorderEnable      = 0x01;
    CDFrameSet.byAvail1          = 0;
    CDFrameSet.Reserved1         = 0;
    CDFrameSet.Reserved2         = 0;
    CDFrameSet.FrameBorderWidth  = 0x0007;
    CDFrameSet.Reserved3         = 0;
    CDFrameSet.FrameSpacingWidth = 0;
    CDFrameSet.Reserved4         = 0;
    CDFrameSet.RowQty            = 0x0001;
    CDFrameSet.ColQty            = 0x0002;
    CDFrameSet.Reserved5         = 0;
    CDFrameSet.Reserved6         = 0;
    CDFrameSet.Reserved7         = 0;
 

    /* write CDFRAMESET structure to CD buffer */
    ODSWriteMemory(( void far * far* )pBuf, _CDFRAMESET, &CDFrameSet, 1);

    /* write the variable data now to the buffer */

    /* row info...*/
    FrameLen1.Type               = RELATIVE_LengthType;	
    FrameLen1.Value              = 0x0001;

    /* column info...*/
    FrameLen2.Type               = PERCENTAGE_LengthType;
    FrameLen2.Value              = 0x0024;
    /* column info...*/
    FrameLen3.Type               = PERCENTAGE_LengthType;
    FrameLen3.Value              = 0x0040;

    /* write FRAMESETLENGTH structure to CD buffer */
    ODSWriteMemory(( void far * far* )pBuf, _FRAMESETLENGTH, &FrameLen1, 1);
    ODSWriteMemory(( void far * far* )pBuf, _FRAMESETLENGTH, &FrameLen2, 1);
    ODSWriteMemory(( void far * far* )pBuf, _FRAMESETLENGTH, &FrameLen3, 1);

    /* init the CDFRAME structure for Frame1 ... */
    memset(&CDFrame, 0, sizeof(CDFRAME));

    CDFrame.Header.Signature  = SIG_CD_FRAME;
    CDFrame.Flags             = 0;
    CDFrame.DataFlags         = 0;
    CDFrame.BorderEnable      = 0x01;
    CDFrame.NoResize          = 0x00;
    CDFrame.ScrollBarStyle    = AUTO_ScrollStyle;
    CDFrame.MarginWidth       = 0x0001;
    CDFrame.MarginHeight      = 0x0001;
    CDFrame.dwReserved        = 0;
    CDFrame.FrameNameLength   = strlen(FrameName1);
    CDFrame.Reserved1         = 0;
    CDFrame.FrameTargetLength = strlen(FrameName2);
    CDFrame.wReserved         = 0;
    CDFrame.Header.Length     = (BYTE)ODSLength(_CDFRAME) + CDFrame.FrameNameLength + CDFrame.FrameTargetLength;

    /* write CDFRAME structure to CD buffer */
    ODSWriteMemory(( void far * far* )pBuf, _CDFRAME, &CDFrame, 1);
    
    /* write the variable data now to the buffer */
    memcpy( *pBuf, &FrameName1[0], CDFrame.FrameNameLength);
    *pBuf += CDFrame.FrameNameLength;
        
    memcpy( *pBuf, &FrameName2[0], CDFrame.FrameTargetLength);
    *pBuf += CDFrame.FrameTargetLength;

/*
 *  Make sure the length of the CD record is an even number so the next CD
 *  record begins on an even byte boundary.
 */

    if ((*pBuf - pBufStart) %2)
        (*pBuf)++;

 /* init the CDRESOURCE structure... */
    memset(&CDResource, 0, sizeof(CDRESOURCE));

    CDResource.Header.Signature  = SIG_CD_HREF;
    CDResource.Flags             = 0;
    CDResource.Type              = CDRESOURCE_TYPE_NAMEDELEMENT;
    CDResource.ResourceClass     = CDRESOURCE_CLASS_VIEW;
    CDResource.Length1           = strlen(ViewName);
    CDResource.ServerHintLength  = 0;
    CDResource.FileHintLength    = 0;
    CDResource.Header.Length     = (BYTE)ODSLength(_CDRESOURCE) + (BYTE)ODSLength(_TIMEDATE) + CDResource.Length1;

    /* write CDRESOURCE structure to CD buffer */
    ODSWriteMemory(( void far * far* )pBuf, _CDRESOURCE, &CDResource, 1);

    /* write the variable data now to the buffer */
    memset(&TimeDate, 0, sizeof(TIMEDATE));
 
    /* write CDRESOURCE structure to CD buffer */
    ODSWriteMemory(( void far * far* )pBuf, _TIMEDATE, &TimeDate, 1);

    memcpy( *pBuf, &ViewName[0], CDResource.Length1);
    *pBuf += CDResource.Length1;

/*
 *  Make sure the length of the CD record is an even number so the next CD
 *  record begins on an even byte boundary.
 */

    if ((*pBuf - pBufStart) %2)
        (*pBuf)++;

    /* init the CDFRAME structure for Frame2... */
    memset(&CDFrame, 0, sizeof(CDFRAME));

    CDFrame.Header.Signature  = SIG_CD_FRAME;
    CDFrame.Flags             = 0;
    CDFrame.DataFlags         = 0;
    CDFrame.BorderEnable      = 0x01;
    CDFrame.NoResize          = 0x00;
    CDFrame.ScrollBarStyle    = AUTO_ScrollStyle;
    CDFrame.MarginWidth       = 0x0001;
    CDFrame.MarginHeight      = 0x0001;
    CDFrame.dwReserved        = 0;
    CDFrame.FrameNameLength   = strlen(FrameName2);
    CDFrame.Reserved1         = 0;
    CDFrame.FrameTargetLength = 0;
    CDFrame.wReserved         = 0;
    CDFrame.Header.Length     = (BYTE)ODSLength(_CDFRAME) + CDFrame.FrameNameLength + CDFrame.FrameTargetLength;

    /* write CDFRAME structure to CD buffer */
    ODSWriteMemory(( void far * far* )pBuf, _CDFRAME, &CDFrame, 1);
    
    /* write the variable data now to the buffer */
    memcpy( *pBuf, &FrameName2[0], CDFrame.FrameNameLength);
    *pBuf += CDFrame.FrameNameLength;
        

/*
 *  Make sure the length of the CD record is an even number so the next CD
 *  record begins on an even byte boundary.
 */

    if ((*pBuf - pBufStart) %2)
        (*pBuf)++;

 /* init the CDRESOURCE structure... */
    memset(&CDResource, 0, sizeof(CDRESOURCE));

    CDResource.Header.Signature  = SIG_CD_HREF;
    CDResource.Flags             = 0;
    CDResource.Type              = CDRESOURCE_TYPE_NOTELINK;
    CDResource.ResourceClass     = CDRESOURCE_CLASS_DOCUMENT;
    CDResource.Length1           = 0;
    CDResource.ServerHintLength  = 0;
    CDResource.FileHintLength    = 0;
    CDResource.Header.Length     = (BYTE)ODSLength(_CDRESOURCE) + (BYTE)ODSLength(_WORD);

    /* write CDRESOURCE structure to CD buffer */
    ODSWriteMemory(( void far * far* )pBuf, _CDRESOURCE, &CDResource, 1);

    /* write the variable data now to the buffer */
    /* zero LINK ID */
    memset(&LinkID, 0, sizeof(WORD));
    ODSWriteMemory(( void far * far* )pBuf, _WORD, &LinkID, 1);
        
    return(*pBuf);
}

/************************************************************************

    FUNCTION:    GetNoteLinkInfo

    PURPOSE:     This function gets the link information necessary for
                 building the contents of the Document frame ("Frame2").
                 The frame contents consist of a link to the Document from 
                 the View frame (Frame1).

*************************************************************************/
STATUS LNPUBLIC GetNoteLinkInfo()
{

    NOTEHANDLE        hNote;
    STATUS            sError=NOERROR;
    char              ViewName[]="New View";

    /* first get replica id information to build a NOTELINK structure */
    if (sError = NSFDbReplicaInfoGet (hDB, &DBReplica))
    {                          
        sprintf(errMsg, "Error: Getting database replica information.\n");
        return(sError);
    }

    /* next get the NOTEID of the view */
    if (sError = NIFFindDesignNote(hDB, ViewName, NOTE_CLASS_VIEW, 
                                 &ViewNoteID))
    {
        sprintf(errMsg, "Error: View '%s' not found.\n", ViewName );
        return(sError);
    }

        
    /* next get the NOTEID of the Help-About document */
    if (sError = NSFDbGetSpecialNoteID(hDB, SPECIAL_ID_NOTE | NOTE_CLASS_INFO, 
                                       &FormNoteID))
    {
        sprintf(errMsg, "Error: Form '%s' not found.\n", HelpDoc );
        return(sError);
    }


    /* Open the View Note */
    if (sError = NSFNoteOpen (hDB, ViewNoteID, 0, &hNote))
    {
        sprintf(errMsg, "Error: Opening View Note.\n" );
        return(sError);
    }

    /* Get the OID from the view note */
    NSFNoteGetInfo (hNote, _NOTE_OID, &ViewOID);

    /* close the note */
    if (sError = NSFNoteClose (hNote))
    {
        sprintf(errMsg, "Error: Closing View Note.\n" );
        return(sError);
    }

    /* Open the Form Note */
    if (sError = NSFNoteOpen (hDB, FormNoteID, 0, &hNote))
    {
        sprintf(errMsg, "Error: Opening Form Note.\n" );
        return(sError);
    }

    /* Get the OID from the form note */

    NSFNoteGetInfo (hNote, _NOTE_OID, &FormOID);

    if (sError = NSFNoteClose (hNote))
    {
        sprintf(errMsg, "Error: Closing Form Note.\n" );
        return(sError);
    }

    return (sError);

}

/************************************************************************

    FUNCTION:    AddNoteLink

    PURPOSE:     Adds a NoteLink structure into a buffer.

*************************************************************************/
char far * AddNoteLink(char far * far *pBuf, char far *pBufStart)
{

    char              ViewName[]="New View";
    LIST              List;
    NOTELINK          NoteLink;

    List.ListEntries = 1;

    /* fill NoteLink structure... */
    NoteLink.File = DBReplica.ID;
    memmove(&NoteLink.View.File, &ViewOID.File, sizeof(DBID));
    memmove(&NoteLink.View.Note, &ViewOID.Note, sizeof(TIMEDATE));
    memmove(&NoteLink.Note.File, &FormOID.File, sizeof(DBID));
    memmove(&NoteLink.Note.Note, &FormOID.Note, sizeof(TIMEDATE));

    /* write LIST structure to buffer */
    memcpy(*pBuf, &List, sizeof(LIST));
    *pBuf += sizeof(LIST);

    /* write the variable data now to the buffer */
    memcpy(*pBuf, &NoteLink, sizeof(NOTELINK));
    *pBuf += sizeof(NOTELINK);

/*
 *  Make sure the length of the buffer is an even number
 */

    if ((*pBuf - pBufStart) %2)
        (*pBuf)++;

    return(*pBuf);

}

