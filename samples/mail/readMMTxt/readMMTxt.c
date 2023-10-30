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

    PROGRAM:    readMMTxt

    FILE:       readMMTxt.c

    SYNTAX:     readMMTxt <database>
             or readMMTxt

    PURPOSE:    Shows how to get text from MIME Part format Note.

    DESCRIPTION:
          This program scan the given DATABASE to find all notes documents.
          To those Multipart MIME messages it find, this program will extract 
          text from them and print them to an text file.
          
****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* HCL C API for Notes/Domino include files */
#include <stdio.h>
#include <string.h>

/* HCL C API for Notes/Domino include files */
#include <global.h>
#include <globerr.h>
#include <oserr.h>
#include <names.h>
#include <stdnames.h>                   /* FIELD_FORM, etc */
#include <osfile.h>
#include <nsf.h>
#include <nsfdb.h>
#include <nsfmime.h>
#include <nsfnote.h>
#include <osenv.h>                      /* OSGetEnvironmentString */
#include <mail.h>
#include <osmisc.h>
#include <osmem.h>
#include "../../printLog.h"
#include <mime.h>
#include <mimedir.h>
#include <mimeods.h>
#include <ods.h>

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/* constants */
#define LNULLPTR 0 
#define DEDAULTTDB      "MIMEMsg.nsf"
#define TEXT_BEGIN      "***********text begin ***************\n"
#define TEXT_END        "***********text end   ***************\n"
#define MAX_DB_NAME     256
#define ITEM_NAME_ATTACHMENT "$FILE" 
#define FILE_NAME "Plain.txt" 

/* Headers to be printed to the output file */
MIMESYMBOL mszHeaderList[] =
{
    MIME_SYMBOL_FROM,
    MIME_SYMBOL_TO,
    MIME_SYMBOL_SUBJECT,
    MIME_SYMBOL_CC,
    MIME_SYMBOL_VERSION,
    MIME_SYMBOL_LAST
};

char *pszHeaderName[] =
{
    "From",
    "To",
    "Subject",
    "CopyTo",
    "MIME_VERSION",
    NULL
};


/* function prototypes */
void   PrintAPIError ( STATUS );
STATUS LNPUBLIC EnumProc( void far *, SEARCH_MATCH far *, ITEM_TABLE far * );
STATUS LNPUBLIC ExtractMIMEText( NOTEHANDLE ); 
STATUS EnumItems( NOTEHANDLE ); 
#if defined(LINUX)
STATUS removeSpecialChar( char* inputStr );
#endif

/************************************************************************

    FUNCTION:   main

*************************************************************************/
int main( int argc, char * argv[] )
{
    STATUS      error = NOERROR;

    DBHANDLE    hDB = NULLHANDLE;
    NOTEHANDLE  hNote = NULLHANDLE;
    char        szDbName[MAX_DB_NAME] = { 0 }; 

    char        formula[] = "@All";     /* an ASCII selection formula. */
    FORMULAHANDLE    formula_handle;    /* a compiled selection formula */
    WORD        wdc;                    /* a word we don't care about */
     
    /* Initiate  the environment */
    error = NotesInitExtended( argc, argv );
    if ( error != NOERROR )
    {
        PRINTLOG( "\n Unable to initialize Notes. Error Code[0x%04x]\n", error );    
        return(1);
    }
    
    /* Check the input */
    if ( argc > 2 )
    {   
        PRINTLOG( "Error: incorrect syntax.\n" );
        PRINTLOG( "\nUsage:%s  <database>\n", argv[0] );
        NotesTerm();
        return (NOERROR);
    }

    memset( szDbName, '\0', MAX_DB_NAME );	    
    if ( argc == 2 ) 
    {
        strncpy( szDbName, argv[1], MAX_DB_NAME-1 );
    }
    else
        strncpy( szDbName, DEDAULTTDB, sizeof( DEDAULTTDB ) ); 


    /* Open the DB */
    error = NSFDbOpen( szDbName, &hDB );
    if ( error )
    {
        PrintAPIError( error );
        goto exit;
    }

    error = NSFFormulaCompile(
                              NULL,               
                              (WORD) 0,           
                              formula,            
                              (WORD) strlen( formula ), 
                              &formula_handle,    
                              &wdc,               
                              &wdc,               
                              &wdc, &wdc, &wdc, &wdc ); 
    if ( error != NOERROR )
    {
        PrintAPIError( error );
        goto exit;
    }

    error = NSFSearch(
                      hDB, 
                      formula_handle,
                      NULL, 
                      0,
                      NOTE_CLASS_DOCUMENT,
                      NULL,
                      EnumProc,
                      &hDB,
                      NULL );
    if ( error != NOERROR ) 
    {
        PrintAPIError( error );
        goto exit;
    }

    OSMemFree( formula_handle );
exit:
    if ( hDB )
        NSFDbClose( hDB );

    NotesTerm();

    if ( error )
    {
        PRINTLOG( "Fail to get text from %s.\n", szDbName );
        fflush( stdout );
        return ( 1 );
    }
    else
    { 
        PRINTLOG( "succeed return.\n" );
        fflush( stdout );
        return ( NOERROR );
    }
}


/*************************************************************************

    FUNCTION:   PrintAPIError

    PURPOSE:    This function prints the HCL C API for Notes/Domino 
                error message associated with an error code.

**************************************************************************/
void PrintAPIError( STATUS api_error ) 

{
    STATUS  string_id = ERR( api_error );
    char    error_text[MAXSPRINTF] = { 0 };
    WORD    text_len = 0;

    /* Get the message for this HCL C API for Notes/Domino error code
       from the resource string table. */

    text_len = OSLoadString( NULLHANDLE,
                             string_id,
                             error_text,
                             sizeof( error_text )-1 );

    /* Print it. */
    fprintf( stderr, "\n%s\n", error_text );

}

/************************************************************************

    FUNCTION:   EnumProc

    PURPOSE:    Called by NSFSearch for each note that matches the 
                selection criteria

    INPUTS:

    RETURNS:   
        NOERROR or STATUS if an error condition is encountered.

    DISCUSSION:

 *************************************************************************/
STATUS LNPUBLIC EnumProc( void far *phDB, SEARCH_MATCH far *pSearchMatch, ITEM_TABLE far *summary_info )
{
    SEARCH_MATCH SearchMatch;
    NOTEHANDLE   hNote = NULLHANDLE;
    STATUS       error = NOERROR;
    BLOCKID      bhItem = { NULL };

    memcpy( (char*)&SearchMatch, (char*)pSearchMatch, sizeof(SEARCH_MATCH) );

    /* Skip this note if it does not really match the search criteria (it is
    now deleted or modified).  This is not necessary for full searches,
    but is shown here in case a starting date was used in the search. */

    if ( !( SearchMatch.SERetFlags & SE_FMATCH ) )
        return (NOERROR);

    /* Print the note ID. */
    PRINTLOG ( "\n\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
    PRINTLOG ( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n" );
    PRINTLOG ( "Note ID is: [%lX].\n", SearchMatch.ID.NoteID );
    fflush( stdout );

    /* Open the note. */
    error = NSFNoteOpenExt(
                           *(DBHANDLE far *)phDB,
                           SearchMatch.ID.NoteID,
                           OPEN_RAW_MIME,        
                           &hNote );
    if ( error != NOERROR )           
        return (error);

    if ( NSFNoteHasMIMEPart( hNote ) )
    {
        PRINTLOG(  "============================================================\n" );
        fflush( stdout );
        error = ExtractMIMEText( hNote );
        if ( error != NOERROR )
        {
            PRINTLOG( "Error when output MIME part content to file, return 1 to caller.\n" );
            return( error );        
        }
    
        PRINTLOG(  "============================================================\n" );
        fflush( stdout );

        if ( (error = EnumItems( hNote )) != NOERROR )
        {
            PRINTLOG( "Error in iterating item for noteid[%x]", SearchMatch.ID.NoteID );
            PrintAPIError( error );
        }
    }
    error = NSFNoteClose( hNote );
    if ( error != NOERROR )
        return ( ERR( error ) );

/* End of subroutine. */

    return ( NOERROR );
}


/*************************************************************************
 *
 * FUNCTION: EnumItems
 *
 * PURPOSE:  This function iterates all the item of $FILE in the given document
 *           and check if attached file is MIME Part.
 *
 ************************************************************************/
STATUS EnumItems( NOTEHANDLE hNote )
{
    STATUS          sErr = NOERROR;
    BLOCKID         bidItem = { 0 };
    BLOCKID         bidPrev = { 0 };
    BLOCKID         bidValue = { 0 };
    WORD            wDataType = 0;
    DWORD           dwValueLength = 0;
    BOOL            bAttFlag = FALSE;
    char            *cpValue = NULL;
    FILEOBJECT      fo = { 0 };

    if ( (sErr = NSFItemInfo( hNote, ITEM_NAME_ATTACHMENT, sizeof( ITEM_NAME_ATTACHMENT )-1,
                           &bidItem, &wDataType,
                           &bidValue, &dwValueLength )) == NOERROR )
    {
        do
        {
            if ( wDataType != TYPE_OBJECT )
            {
                bidPrev = bidItem;
                continue;
            }
            if ( dwValueLength < ODSLength( _FILEOBJECT ) + sizeof( WORD ) )
            {
                bidPrev = bidItem;
                continue;
        }
        cpValue = OSLockBlock( char, bidValue ) + sizeof( WORD );
        ODSReadMemory( &cpValue, _FILEOBJECT, &fo, 1 );
        OSUnlockBlock( bidValue );

        if ( fo.Header.ObjectType != OBJECT_FILE )
        {
             bidPrev = bidItem;
             continue;
        }

        if ( fo.FileNameLength != ( sizeof( FILE_NAME )-1 ) ||
              IntlTextCompare( cpValue, fo.FileNameLength, FILE_NAME, 
              fo.FileNameLength, INTL_ACCENT_SENSITIVE) )
        {
            bidPrev = bidItem;
            continue;
        } 

        if ( NSFIsFileItemMimePart( hNote, bidItem ) )
        {
            PRINTLOG( "Attached file item [%s] is a MIME part in [%s].\n", FILE_NAME, ITEM_NAME_ATTACHMENT );
        }

        bidPrev = bidItem;
        }
        while ( (sErr = NSFItemInfoNext( hNote, bidPrev, ITEM_NAME_ATTACHMENT, 
                 sizeof( ITEM_NAME_ATTACHMENT )-1, &bidItem, 
                 &wDataType, &bidValue, &dwValueLength )) == NOERROR );
        if ( sErr == ERR_ITEM_NOT_FOUND )
        {
            sErr = NOERROR;
        }
        else if ( sErr != NOERROR )
        {
            return sErr;
        }
    }
    else
    {
        if ( sErr == ERR_ITEM_NOT_FOUND )
        {
            sErr = NOERROR;
        }

        return sErr;
    }

    return NOERROR;
}


/*************************************************************************

    FUNCTION:   ExtractMIMEText

    PURPOSE:    This function extract the text from the MIME format Note

**************************************************************************/
STATUS LNPUBLIC ExtractMIMEText( NOTEHANDLE hNote )
{
  
    STATUS              error = NOERROR;
    int                 i = 0;

    HMIMEDIRECTORY      hMD = NULL;
    PMIMEENTITY         pMERoot = NULLPTR;
    PMIMEENTITY         pME = NULLPTR;
    PMIMEENTITY         pME_new = NULLPTR;
    DHANDLE             hEntityData = NULLHANDLE;
    DWORD               dwEntityDataLen = 0;
    DWORD               dwEntityEncodeLen = 0;
    char                *szEntityData = NULL;
    MIMESYMBOL          msContentType = MIME_SYMBOL_UNKNOWN;
    MIMESYMBOL          msSubType = MIME_SYMBOL_UNKNOWN;
    DWORD               dwPartFlags = 0;
    BOOL                bAttFlag = FALSE;
    DHANDLE             hTypeParam = NULLHANDLE;
    DWORD               dwTypeParamLen = 0;
    char                *pczParaName = NULL;
    const char          *pczMimeHeader = NULL;
    
   
    error = MIMEOpenDirectory( hNote, &hMD );
    if ( error != NOERROR )
    {
        PrintAPIError( error );
        goto done;
    }

    /* get the root entity */
    error = MIMEGetRootEntity( hMD, &pMERoot );
    if ( error != NOERROR )
    {
        PrintAPIError( error );
        goto done;
    }

    /* Check the message type. */
    if ( MIMEEntityIsDiscretePart( pMERoot ) == TRUE )
        PRINTLOG( "This is not a neither a message nor a multipart.\n");
 
    if ( MIMEEntityIsMessagePart( pMERoot ) == TRUE )
        PRINTLOG(  "This is a message part.\n" );

    fflush( stdout );
    if ( MIMEEntityIsMultiPart( pMERoot ) == TRUE )
    {
        PRINTLOG( "This is a multipart message:\n\n" );
        PRINTLOG( "Header:\n" );
        fflush( stdout );
        
        /* first print the header to the output file. */
        for ( i=0; mszHeaderList[i]< MIME_SYMBOL_LAST; i++ )
        {
            if ( pszHeaderName[i] != NULL ) 
            {
                pczMimeHeader = MIMEEntityGetHeader( pMERoot, mszHeaderList[i] ); 
                PRINTLOG( "\t[%s : %s]\n", pszHeaderName[i], ( pczMimeHeader != NULL ? pczMimeHeader : "" )
                         );
                fflush( stdout );
            }
        }

        /* 2nd print the body text to the output file. */
        PRINTLOG( "\nBody:\n" );
        fflush( stdout );

        pME = pMERoot;
        msContentType = MIMEEntityContentType( pME );

        while ( msContentType != MIME_SYMBOL_UNKNOWN )
        {
            msSubType = MIMEEntityContentSubtype( pME );

            /* to see if the body includes attachment */
            error = MIMEGetEntityPartFlags( hNote,
                                          pME,
                                          &dwPartFlags );
            if ( error != NOERROR )
            {
                PrintAPIError( error );
                goto done;
            }

            if ( dwPartFlags & MIME_PART_BODY_IN_DBOBJECT )
            {
                bAttFlag = TRUE;
            }
            else
            {
                bAttFlag = FALSE;
            }

           /* output the text both plain & html */
            if ( msContentType == MIME_SYMBOL_TEXT  && bAttFlag == FALSE )
            {
                /* check the subtype of content */
                if ( msSubType == MIME_SYMBOL_PLAIN )
                    PRINTLOG( "\nHere is the plain text:\n" );
                if ( msSubType == MIME_SYMBOL_HTML )
                    PRINTLOG( "\nHere is the HTML text:\n" );
                  
                fflush( stdout );
                /* get the character set */
                error = MIMEEntityGetTypeParam( pME,
                                                MIME_SYMBOL_CHARSET,
                                                &hTypeParam,
                                                &dwTypeParamLen );
                if ( error != NOERROR )
                {
                    PrintAPIError( error );
                    goto done;
                }
        
                pczParaName = OSLock( char, hTypeParam );
                OSUnlock( hTypeParam );           

                PRINTLOG( "charset = %s\n", pczParaName );

                fflush( stdout );
                {
                    error = MIMEGetDecodedEntityData( hNote,
                                                      pME,
                                                      0,
                                                      MAXDWORD,
                                                      &hEntityData,
                                                      &dwEntityDataLen,
                                                      &dwEntityEncodeLen );
                }

                OSMemFree( hTypeParam );
                if ( error != NOERROR )
                {
                    PrintAPIError( error );
                    goto done;
                }
 
                szEntityData = OSLock( char, hEntityData );
                szEntityData[dwEntityDataLen] = '\0';

                PRINTLOG( TEXT_BEGIN );
                fflush( stdout );

#if defined (LINUX)
                if ( !removeSpecialChar( szEntityData ) ) 
                    PRINTLOG( "[%s]\n",szEntityData );
#else
                PRINTLOG( "[%s]\n", szEntityData );
#endif 

                PRINTLOG( TEXT_END );
                fflush( stdout );

                OSUnlock( hEntityData );
                OSMemFree( hEntityData );
            }
            pME_new = LNULLPTR;
            MIMEIterateNext( hMD, pMERoot, pME, &pME_new );
            if ( pME_new != LNULLPTR )
            {
                pME = pME_new;   
                msContentType = MIMEEntityContentType( pME );          
            }
            else 
            {
                pME = NULLPTR;
                msContentType = MIME_SYMBOL_UNKNOWN;
            }
        } 
    } 
    
done:
    if ( hMD )
    {
        MIMEFreeDirectory( hMD );
    }
    
    if ( error != NOERROR )
    {
        return( 1 );
    }
    else
    {
        return( NOERROR );
    }
}

#if defined(LINUX)

STATUS removeSpecialChar( char* inputStr )
{
    const int slen = strlen( inputStr );
    char tmpStr[slen+1];
    char *sztmpStr = tmpStr;
    char *szbase = inputStr;
    int len = 0;
    memset( tmpStr, '\0', sizeof(tmpStr) );

    while ( inputStr != NULL && len < slen )
    {
        if ( *inputStr != '\r' ) 
        {
            *sztmpStr = *inputStr;
            sztmpStr++;
        }
        inputStr++;
        len++;
    }
    tmpStr[len]='\0';
    strcpy( szbase, tmpStr );
    return NOERROR;
}
#endif
