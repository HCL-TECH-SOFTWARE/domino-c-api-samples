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

   File:        jvapplet.c

   Program:     jvapplet

   Purpose:     shows how to embed a Java Applet in the document.

   Description:

       This sample creates a document in the database specified 
       in the input parameter.  This document will contain a local 
       applet which prints the title of the current database.  The 
       server name and database name of the current database are 
       passed to the local applet through the @dbName formula.

       First, this program opens the specified database, and creates 
       a new note. It then allocates a buffer in which to construct 
       the rich text item. In this buffer, it creates the following 
       sequence of data structures to provide information for the 
       Java Applet:

       CDRECORDBEING
       CDV4HOTSPOTBEGIN
       CDACTIVEOBJECT
       CDACTIVEOBJECTPARAM  (this structure is used to pass a parameter
                             to the Java Applet.  In this sample,
                             we will use 2 CDACTIVEOBJECTPARAM
                             structures, one for passing the servername
                             and one for passing the db name.)
       CDACTIVEOBJECTSTORAGELINK
       CDV4HOTSPOTEND
       CDRECORDEND

 
       The buffer thus created is then appended to the note's rich text
       field. 
       
       The program then attachs the Java Applet class file to the note.
       
       The note is then updated and closed, the database is closed, and 
       the program exits.



************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/*
 *  OS includes
 */

#include <stdio.h>
#include <stdlib.h>  
#include <string.h>

/*
 *  HCL C API for Notes/Domino includes
 */

#include <global.h>
#include <nsf.h>
#include <osmem.h>
#include <names.h>
#include <editods.h>
#include <nsfnote.h>
#include <oserr.h>
#include <stdnames.h>
#include <osmisc.h>
#include "../../printLog.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif


#define FORMNAME                "Simple Form"
#define APPLET_PARM_NAME_SERVER "serverName"
#define APPLET_PARM_NAME_DB     "dbName"
#define SERVERNAME_VALUE_FORMULA "@Subset(@DbName;1)"
#define DBNAME_VALUE_FORMULA "@Subset(@DbName;-1)"

#define STRING_LENGTH  256
#define APPLET_WIDTH   462
#define APPLET_HEIGHT  120

char AppletCodeBase[]  = "notes:///./$FILE";
char CodeString[]      = "IntroLocalApplet.class";

char server_name[80];
char db_name[MAXPATH];

#if defined(OS400)
char CodeStringFullPath[] = "<your server data directory>/IntroLocalApplet.class";
#elif defined(UNIX)
char CodeStringFullPath[] = "/opt/lotus/notesapi/samples/richtext/jvapplet/IntroLocalApplet.class";
#else
char CodeStringFullPath[] = "c:\\notesapi\\samples\\richtext\\jvapplet\\IntroLocalApplet.class";
#endif

/*
 *  Forward Declaration
 */ 

void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                             char *db_filename); 

STATUS InsertRichText(NOTEHANDLE hNote);

         
STATUS InsertJavaAppletLink(NOTEHANDLE hNote,
                               char FAR * FAR *ppCDBuffer,
                               WORD FAR *pwItemSize,
                               WORD wBufferSize);  
                         
STATUS InsertJavaAppletAttachment(NOTEHANDLE hNote,
                               char FAR * FAR *ppCDBuffer,
                               WORD FAR *pwItemSize,
                               WORD wBufferSize);

/*
 *  Body of main program
 */

int main (int argc, char *argv[])
{
    char       *db_filename;
    DHANDLE     hNewNote;
    DBHANDLE    hDbNSFFile;
    STATUS      sError = NOERROR;
    char        database_name[STRING_LENGTH];

    /* Read in the database name. */
    db_filename = database_name;
    ProcessArgs(argc, argv, db_filename);

    /* Start by calling Notes Init. */

    if (sError = NotesInitExtended (argc, argv))
    {
       PRINTLOG("\n Unable to initialize Notes.\n");
       return (1);
    }


    /* Open the Database which we're going to create a new */
    /* document to host the local applet.                  */
    
    if (sError = NSFDbOpen(db_filename, &hDbNSFFile))
        goto exit0;


    /* Create a Note */
    
    if (sError = NSFNoteCreate(hDbNSFFile, &hNewNote))
        goto exitCloseDB;

    /* Fill in the Form name  */
    
    if (sError = NSFItemSetText(hNewNote,
                                 "Form",
                                 FORMNAME,
                                 (WORD) strlen(FORMNAME)))
        goto exitCloseNote;


    /*
     *  Create the richtext item, which will contain inf. about the
     *  applet.
     */

    if (sError = InsertRichText(hNewNote))
        goto exitCloseNote;

    /*
     *  Attach the Java class file to the note.
     */

    if (sError = NSFNoteAttachFile(hNewNote,
                                   ITEM_NAME_ATTACHMENT,
                                   (WORD) strlen(ITEM_NAME_ATTACHMENT),
                                   CodeStringFullPath,
                                   CodeString,
                                   HOST_STREAM))
     {
        PRINTLOG("Error: Couldn't find the class file at %s\n",CodeStringFullPath);
        goto exitCloseNote;
     }

    /* write the note */

    if (sError = NSFNoteUpdate(hNewNote, 0))
        goto exitCloseNote;


exitCloseNote:
    /* Start cleaning up.  First, Close the Note */
    NSFNoteClose(hNewNote);


exitCloseDB:
    /* Now close the database.              */
    NSFDbClose(hDbNSFFile);


exit0:
    if (sError)
    {
        PRINTERROR(sError, "NSFNoteUpdate");
    }
    else
    {
        PRINTLOG("\nProgram completed successfully.\n");
    }

    NotesTerm();
    return(sError);

}


/***********************************************************************
 *
 * InsertRichText
 *
 *      Allocate a buffer in which to build the rich text item. Insert
 *      inf. about the Java Applet. Then append the buffer to the note.
 *
 * Inputs:   hNote   - handle to the new note being created.
 *
 * Outputs:  None.
 *
 ************************************************************************/


STATUS InsertRichText(NOTEHANDLE hNote)

{
    DHANDLE    hMem;
    STATUS     sError=NOERROR;   /* Domino and Notes error status  */
    char FAR  *pCDBuffer;
    char FAR  *pCDBufferStart;
    WORD       wItemSize = 0, wBufferSize = MAXONESEGSIZE;

/*
 *  Allocate a buffer.
 */

    if (sError = OSMemAlloc(0, wBufferSize, &hMem))
        return (ERR_MEMORY);
        
    if ((pCDBuffer = (char far *) OSLockObject(hMem)) == NULL)
        return (ERR_MEMORY);
        
    memset(pCDBuffer, '\0', wBufferSize);
    pCDBufferStart = pCDBuffer;


/*
 *  Insert inf. about the Applet.
 */
    if (sError = InsertJavaAppletLink(hNote, &pCDBuffer, &wItemSize, 
                                      wBufferSize))
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
 * InsertJavaAppletLink
 *
 *             The steps requed are:
 *
 *             - Create an CDBEGINRECOR, append it to the CD buffer.
 *
 *             - Create and append the following structures to the
 *               CD buffer:
 *
 *                   CDV4HOTSPOTBEGIN
 *                   CDACTIVEOBJECT
 *                   CDACTIVEOBJECTPARAM
 *                   CDACTIVEOBJECTSTORAGELINK
 *
 *               To sign the hotspot, you need to first create a variable
 *               data buffer, copy CDACTIVEOBJECT,CDACTIVEOBJECTPARAM,
 *               and CDACTIVEOBJECTSTORAGELINK inf. and their associate
 *               variables to this variable data buffer, then sign it
 *               with the NSFHotSpotSign API.  Be sure to:
 *                   --  add the length of the signature returned from 
 *                       this API to V4HOTSPOTBEGIN header length
 *                   --  copy the signature to the CD buffer after the
 *                       variable data of CDACTIVEOBJECTSTORAGELINK.
 *
 *             - Create a CDV4HOTSPOTEND record to the CD buffer.
 *
 *             - Add a CDRECORDEND record to mark the end of the CD
 *               buffer.
 *
 *
 * Inputs:     hNote       - handle to the new note being created.
 *             ppCDBuffer  - Handle to buffer containing CD records being 
 *                           created.
 *             pwItemSize  - current length of item.
 *             wBufferSize - total size of CD buffer.
 *             
 * Outputs:    None.
 *************************************************************************/


STATUS InsertJavaAppletLink(NOTEHANDLE hNote,
                 char FAR * FAR *ppCDBuffer,
                 WORD FAR *pwItemSize,
                 WORD wBufferSize)

{
    STATUS                  sError = NOERROR;
    CDBEGINRECORD           BeginRecord;
    CDHOTSPOTBEGIN          V4HotSpotBegin;
    ACTIVEOBJECT            ao;
    ACTIVEOBJECTPARAM       aoParam1, aoParam2;
    ACTIVEOBJECTSTORAGELINK aoStorageLink;
    CDENDRECORD             EndRecord;
    CDHOTSPOTEND            V4HotSpotEnd;
  
    FORMULAHANDLE           hFormula1;
    FORMULAHANDLE           hFormula2;
    char *                  pFormula1;
    char *                  pFormula2;
    WORD                    wFormulaLen1;
    WORD                    wFormulaLen2;
    WORD                    wdc;
	
    /* Info Needed to Sign Variable Data */
    DHANDLE		    hMem, hSignature;
    DWORD		    dwSigLen;
    BLOCK		    bSigData = NULLBLOCK;
    char FAR	           *pVariableData, *pBeginVariData;
    WORD		    wVariSize = MAXONESEGSIZE, TotalVariLen=0;
    char		   *pData;	

/*
 *  Compile Java Applet's parameter.
 */
    if (sError = NSFFormulaCompile(NULL,0,SERVERNAME_VALUE_FORMULA,
                                   (WORD)strlen(SERVERNAME_VALUE_FORMULA),
                                   &hFormula1,
                                   &wFormulaLen1,
                                   &wdc, &wdc, &wdc,
                                   &wdc, &wdc))
        return(sError);

    pFormula1 = (char *) OSLockObject(hFormula1);

    if (sError = NSFFormulaCompile(NULL,0,DBNAME_VALUE_FORMULA,
                                   (WORD)strlen(DBNAME_VALUE_FORMULA),
                                   &hFormula2,
                                   &wFormulaLen2,
                                   &wdc, &wdc, &wdc,
                                   &wdc, &wdc))
    {
        OSUnlockObject(hFormula1);
        OSMemFree(hFormula1);
        return(sError);
    }

    pFormula2 = (char *) OSLockObject(hFormula2);

    
/*  ***********************************************
    Fill up structure "Begin - V4HotspoBegin"
    *********************************************** 
*/

    BeginRecord.Signature = SIG_CD_V4HOTSPOTBEGIN;
    BeginRecord.Version   = 0;

    
    BeginRecord.Header.Signature = SIG_CD_BEGIN;
    BeginRecord.Header.Length    = (BYTE)ODSLength(_CDBEGINRECORD);

    ODSWriteMemory( (void far * far *) ppCDBuffer, 
                   _CDBEGINRECORD, &BeginRecord, 1);

    *pwItemSize += BeginRecord.Header.Length;

    if (*pwItemSize %2)
        *pwItemSize +=1;


/*  ***********************************************
    Allocate a working buffer for Variable data that is going 
    to be signed
    ***********************************************
 */
    if (sError = OSMemAlloc(0, wVariSize, &hMem))
    {
       OSUnlockObject(hFormula1);
       OSUnlockObject(hFormula2);
       OSMemFree(hFormula1);
       OSMemFree(hFormula2);
       return (ERR_MEMORY);
    }

    if((pVariableData = (char far *) OSLockObject(hMem)) == NULL)
    {
       OSUnlockObject(hFormula1);
       OSUnlockObject(hFormula2);
       OSMemFree(hFormula1);
       OSMemFree(hFormula2);
       OSMemFree(hMem);
       return(ERR_MEMORY);
    }

    memset(pVariableData, '\0', wVariSize);
    pBeginVariData = pVariableData;


/*
 *  Set ACTIVEOBJECT inf. and copy it to the variable data buffer.
 */
    TotalVariLen += ODSLength(_ACTIVEOBJECT);

    (void) memset(&ao,0,sizeof(ACTIVEOBJECT));
    ao.ObjectType       = ACTIVEOBJECT_TYPE_JAVA;
    ao.WidthUnitType    = ACTIVEOBJECT_UNIT_PIXELS;
    ao.HeightUnitType   = ACTIVEOBJECT_UNIT_PIXELS;
    ao.Width            = APPLET_WIDTH;
    ao.Height           = APPLET_HEIGHT;
    ao.CodebaseLength   = strlen(AppletCodeBase);
    ao.CodeLength       = strlen(CodeString);
    ao.ObjectNameLength = 0;
    ao.StorageLinkCount = 1;
    ao.ParamCount       = 2;    /* one for serverName, one for dbName */
    ODSWriteMemory( &pVariableData, _ACTIVEOBJECT, &ao, 1 );

/*
 *  copy ACTIVEOBJECT variable data to the variable data buffer.
 */
    if (ao.CodebaseLength)
    {
       memcpy(pVariableData, AppletCodeBase, ao.CodebaseLength);
       pVariableData += ao.CodebaseLength;
       TotalVariLen += ao.CodebaseLength;
    }

    if (ao.CodeLength)
    {
       memcpy(pVariableData, CodeString, ao.CodeLength);
       pVariableData += ao.CodeLength;
       TotalVariLen += ao.CodeLength; 
    }


/*
 *  Set ActiveObjectParam - this structure is used to pass the server
 *  name to the Java Applet - and copy it to the variable data buffer.
 */
    TotalVariLen += ODSLength(_ACTIVEOBJECTPARAM);

    (void) memset(&aoParam1,0,sizeof(ACTIVEOBJECTPARAM));
    aoParam1.Length        = strlen(APPLET_PARM_NAME_SERVER);
    aoParam1.FormulaLength =wFormulaLen1;
    ODSWriteMemory( &pVariableData, _ACTIVEOBJECTPARAM, &aoParam1,1);

/*
 *  copy ActiveObjectParam variable data to the variable data buffer.
 */
    if (aoParam1.Length)
    {
       memcpy(pVariableData, APPLET_PARM_NAME_SERVER, aoParam1.Length);
       pVariableData += aoParam1.Length;
       TotalVariLen += aoParam1.Length;
    }

    if (aoParam1.FormulaLength)
    {
       memcpy(pVariableData, pFormula1, aoParam1.FormulaLength);
       pVariableData += aoParam1.FormulaLength;
       TotalVariLen += aoParam1.FormulaLength;
    }

/*
 *  Set ActiveObjectParam - this structure is used to pass the Db
 *  name to the Java Applet - and copy it to the variable data buffer.
 */
    TotalVariLen += ODSLength(_ACTIVEOBJECTPARAM);

    (void) memset(&aoParam2,0,sizeof(ACTIVEOBJECTPARAM));
    aoParam2.Length        = strlen(APPLET_PARM_NAME_DB);
    aoParam2.FormulaLength =wFormulaLen2;
    ODSWriteMemory( &pVariableData, _ACTIVEOBJECTPARAM, &aoParam2,1);

/*
 *  copy ActiveObjectParam variable data to the variable data buffer.
 */
    if (aoParam2.Length)
    {
       memcpy(pVariableData, APPLET_PARM_NAME_DB, aoParam2.Length);
       pVariableData += aoParam2.Length;
       TotalVariLen += aoParam2.Length;
    } 

    if (aoParam2.FormulaLength)
    {
       memcpy(pVariableData, pFormula2, aoParam2.FormulaLength);
       pVariableData += aoParam2.FormulaLength;
       TotalVariLen += aoParam2.FormulaLength;
    }

    OSUnlockObject(hFormula1);
    OSUnlockObject(hFormula2);
    OSMemFree(hFormula1);
    OSMemFree(hFormula2);


/*
 *  Set ACTIVEOBJECTSTORAGELINK and copy it to the variable data buffer.
 */
    TotalVariLen += ODSLength(_ACTIVEOBJECTSTORAGELINK);

    (void) memset(&aoStorageLink,0,sizeof(ACTIVEOBJECTSTORAGELINK));
    aoStorageLink.Length = strlen(CodeString); 
    ODSWriteMemory(&pVariableData, 
                   _ACTIVEOBJECTSTORAGELINK, &aoStorageLink, 1);    

/*
 *  copy ACTIVEOBJECTSTORAGELINK variable data to the variable data 
 *  buffer.
 */
    if (aoStorageLink.Length)
    {
       memcpy(pVariableData,CodeString,aoStorageLink.Length);
       pVariableData+=aoStorageLink.Length;
       TotalVariLen += aoStorageLink.Length;
    }


/*
 *  Sign the variable data buffer.
 */
    if (sError = NSFHotSpotSign((BYTE *)pBeginVariData,TotalVariLen,NULL,0,
                                &hSignature,&dwSigLen))
    {
       OSUnlockObject(hMem);
       OSMemFree(hMem);
       return(sError);
    }

    pData=OSLock(char,hSignature);


/*  ***********************************************
    Set V4HotSpotBegin flags 
    ***********************************************
 */
    
    V4HotSpotBegin.Type  = HOTSPOTREC_TYPE_ACTIVEOBJECT;
    V4HotSpotBegin.Flags = HOTSPOTREC_RUNFLAG_NOBORDER | HOTSPOTREC_RUNFLAG_SIGNED;

/*
 *  Set V4HotSpotBegin data length
 */

    V4HotSpotBegin.DataLength = ODSLength(_ACTIVEOBJECT) +
                                ODSLength(_ACTIVEOBJECTSTORAGELINK) + 
                                strlen(AppletCodeBase) +
                                strlen(CodeString) +
                                ODSLength(_ACTIVEOBJECTPARAM) + 
                                strlen(APPLET_PARM_NAME_SERVER) +
                                wFormulaLen1 +
                                ODSLength(_ACTIVEOBJECTPARAM) + 
                                strlen(APPLET_PARM_NAME_DB) +
                                wFormulaLen2 +
                                strlen(CodeString);


/*
 *  Set V4HotSpotBegin header info.
 */
    
    V4HotSpotBegin.Header.Signature = SIG_CD_V4HOTSPOTBEGIN;
    V4HotSpotBegin.Header.Length    = ODSLength(_CDHOTSPOTBEGIN) +
                                ODSLength(_ACTIVEOBJECT) +
                                ODSLength(_ACTIVEOBJECTSTORAGELINK) + 
                                strlen(AppletCodeBase) +
                                strlen(CodeString) +
                                ODSLength(_ACTIVEOBJECTPARAM) + 
                                strlen(APPLET_PARM_NAME_SERVER) +
                                wFormulaLen1 +
                                ODSLength(_ACTIVEOBJECTPARAM) + 
                                strlen(APPLET_PARM_NAME_DB) +
                                wFormulaLen2 +
                                strlen(CodeString)+
                                (WORD)dwSigLen + 
                                ODSLength(_WORD);

/*
 *  Copy V4HotSpotBegin to the CD buffer.
 */

    ODSWriteMemory( (void far * far *) ppCDBuffer, 
                   _CDHOTSPOTBEGIN, &V4HotSpotBegin, 1);

	
/*  ***********************************************
    Copy ACTIVEOBJECT,ACTIVEOBJECTPARAM, and ACTIVEOBJECTSTORAGELINK
    inf. (which was saved in the variable data buffer) to the CD buffer.
    ***********************************************
 */
    memcpy(*ppCDBuffer, pBeginVariData, TotalVariLen);
    *ppCDBuffer += TotalVariLen;

#if defined W64
    if (((long long) *ppCDBuffer) % 2)
       *ppCDBuffer += 1;
#else
    if (((long) *ppCDBuffer) % 2)
       *ppCDBuffer += 1;
#endif


/*  ***********************************************
    Copy the signature to the CD buffer.
    ***********************************************
 */
    ODSWriteMemory((void far * far *)ppCDBuffer, _WORD, &dwSigLen,1);
    memcpy(*ppCDBuffer,pData,dwSigLen);
    *ppCDBuffer += dwSigLen;
	
#if defined W64
    if (((long long) *ppCDBuffer) % 2)
       *ppCDBuffer += 1;
#else
    if (((long) *ppCDBuffer) % 2)
       *ppCDBuffer += 1;
#endif

    OSUnlock(hSignature);
    OSMemFree(hSignature);
    OSUnlock(hMem);
    OSMemFree(hMem);
    
    *pwItemSize += V4HotSpotBegin.Header.Length;
    if (*pwItemSize %2)
        *pwItemSize +=1;



/*  ************************************************
    Fill up structure "V4HotspotEnd"
    ************************************************
*/

    V4HotSpotEnd.Header.Length    = (BYTE)ODSLength(_CDHOTSPOTEND);
    V4HotSpotEnd.Header.Signature = SIG_CD_V4HOTSPOTEND;
    ODSWriteMemory( (void far * far *) ppCDBuffer, 
                   _CDHOTSPOTEND, &V4HotSpotEnd, 1);    

  
    *pwItemSize += V4HotSpotEnd.Header.Length;
    if (*pwItemSize %2)
        *pwItemSize +=1;

/*  ************************************************
    Fill up structure "End - V4HotspotEnd"
    ************************************************
*/

    EndRecord.Signature = SIG_CD_V4HOTSPOTEND;
    EndRecord.Version   = 0;

    
    EndRecord.Header.Signature = SIG_CD_END;
    EndRecord.Header.Length    = (BYTE)ODSLength(_CDENDRECORD);

    ODSWriteMemory( (void far * far *) ppCDBuffer, 
                   _CDENDRECORD, &EndRecord, 1);    

  
    *pwItemSize += EndRecord.Header.Length;
    if (*pwItemSize %2)
        *pwItemSize +=1;
    
/*
 *  Done.
 */
    return (NOERROR);
}


/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:     argc, argv - directly from the command line
   
    OUTPUTS:    db_filename get data from the command line or from what 
                the user types at a prompt 

*************************************************************************/

void  LNPUBLIC  ProcessArgs (int argc, char *argv[], char *db_filename)
{ 

    if (argc != 2)  
    {
        printf("Enter name of database: ");      
        fflush (stdout);
        gets(db_filename);
    }    
    else
    {
        strcpy(db_filename, argv[1]);    
    }   /* end if */

} /* ProcessArgs */



/* This function prints the HCL C API for Notes/Domino error message
   associated with an error code. */

void PrintAPIError (STATUS api_error)

{
    STATUS  string_id = ERR(api_error);
    char    error_text[200];
    WORD    text_len;

    /* Get the message for this HCL C API for Notes/Domino error code
       from the resource string table. */

    text_len = OSLoadString (NULLHANDLE,
                             string_id,
                             error_text,
                             sizeof(error_text));

    /* Print it. */
    fprintf (stderr, "\n%s\n", error_text);

}

