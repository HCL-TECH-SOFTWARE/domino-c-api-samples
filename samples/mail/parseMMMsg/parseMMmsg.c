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

    PROGRAM:    parseMMmsg.c

    FILE:       parseMMsg.c

    SYNTAX:     parseMMsg <database>
             or ParseMMsg

    PURPOSE:    Prints structure of message fils to an output file 

    DESCRIPTION:
        parseMMsg scans messages of a specified NSF database. To 
        those messages which are multi-part MIME format, their structures
        will be written to an output file, and the attachment file names 
        are included if exist.          

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
#include <nsfnote.h>
#include <osenv.h>                      /* OSGetEnvironmentString */
#include <mail.h>
#include <osmisc.h>
#include <osmem.h>
#include <mime.h>
#include <mimedir.h>
#include <mimeods.h>
#include "../../printLog.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif


/* constants */
#define NULLPTR 0
#define TESTDB          "MIMEMsg.nsf"
#define MAX_NAME_LEN    256

/* Headers to be printed to the output file */
MIMESYMBOL mszHeaderList[]=
{
    MIME_SYMBOL_FROM,
    MIME_SYMBOL_TO,
    MIME_SYMBOL_SUBJECT,
    MIME_SYMBOL_LAST
};

char *pszHeaderName[]=
{
    "From",
    "To",
    "Subject",
    NULL
};


/* function prototypes */
STATUS LNPUBLIC EnumProc(void far *, SEARCH_MATCH far *, ITEM_TABLE far *);
STATUS LNPUBLIC ParseMIMEMsg( NOTEHANDLE); 
STATUS LNPUBLIC ScanMIMEMsgBody( NOTEHANDLE, HMIMEDIRECTORY, PMIMEENTITY, int );
void   GetSymbolName( MIMESYMBOL, char *, int *);


/************************************************************************

    FUNCTION:   main

*************************************************************************/
int main(int argc, char * argv[])
{
    STATUS      error = NOERROR;
    DBHANDLE    hDB=NULLHANDLE;
    NOTEHANDLE  hNote=NULLHANDLE;
    char        szDbName[MAX_NAME_LEN]; 
    char        formula[] = "@All";     /* an ASCII selection formula. */
    FORMULAHANDLE  formula_handle;      /* a compiled selection formula */
    WORD        wdc;                    /* a word we don't care about */
     
    /* Initiate  the environment */
    error = NotesInitExtended (argc, argv);
    if ( error != NOERROR )
    {
        PRINTLOG("\n Unable to initialize Notes. Error Code[0x%04x]\n", error);    
        goto exit;
    }

    if ( argc > 2 )
    {   
        PRINTLOG ("Error: incorrect syntax.\n");
        PRINTLOG ("\nUsage:%s  <database>\n", argv[0]);
        NotesTerm();
        return NOERROR;
    }

    if ( argc == 2 )
    {
        memset(szDbName, '\0', MAX_NAME_LEN);    
        strncpy( szDbName, argv[1], MAX_NAME_LEN-1 );
    }
    else
    {
        strncpy( szDbName, TESTDB, sizeof(TESTDB) ); 
        szDbName[sizeof(TESTDB)]='\0';
    }

    /* Open the test DB */
    error = NSFDbOpen(szDbName, &hDB);
    if ( error )
    {
        PRINTERROR(error,"NSFDbOpen");
        goto exit;
    }

    error = NSFFormulaCompile (
                               NULL,               
                               (WORD) 0,           
                               formula,            
                               (WORD) strlen(formula),   
                               &formula_handle,    
                               &wdc,               
                               &wdc,               
                               &wdc, &wdc, &wdc, &wdc); 
    if ( error != NOERROR )
    {
        PRINTERROR(error,"NSFFormulaCompile");
        goto exit;
    }

    error = NSFSearch (
                       hDB,      
                       formula_handle, 
                       NULL,           
                       0,              
                       NOTE_CLASS_DOCUMENT,
                       NULL,         
                       EnumProc, 
                       &hDB, 
                       NULL);
    if ( error != NOERROR ) 
    {
        PRINTERROR(error,"NSFSearch");
        goto exit;
    }     

    OSMemFree (formula_handle);

exit:
    if( hDB )
        NSFDbClose(hDB);

    NotesTerm();

    if ( error )
    {
        PRINTLOG( "\n Fail to parse mime messages.\n" );
        return (1);
    }
    else
    {    
        PRINTLOG ( "\n succeed return.\n" );
        return (NOERROR);
    }

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

STATUS  LNPUBLIC  EnumProc(void far *phDB, SEARCH_MATCH far *pSearchMatch, ITEM_TABLE far *summary_info)
{
    SEARCH_MATCH SearchMatch;
    NOTEHANDLE   hNote;
    STATUS       error;

    memcpy( (char*)&SearchMatch, (char*)pSearchMatch, sizeof(SEARCH_MATCH) );

    /* Skip this note if it does not really match the search criteria (it is
    now deleted or modified).  This is not necessary for full searches,
    but is shown here in case a starting date was used in the search. */
    if (!(SearchMatch.SERetFlags & SE_FMATCH))
        return (NOERROR);

    /* Print the note ID. */
    PRINTLOG ( "\n\n+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    PRINTLOG ( "+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++\n");
    PRINTLOG ( "Note ID is: [%lX].\n", SearchMatch.ID.NoteID);

    fflush(stdout);
    /* Open the note. */
    error = NSFNoteOpenExt (
                            *(DBHANDLE far *)phDB,
                            SearchMatch.ID.NoteID,
                            OPEN_RAW_MIME,        
                            &hNote);
    if ( error != NOERROR )           
        return (error);

    /* skip the note if it has no mime content */
    if ( NSFNoteHasMIMEPart(hNote) )
    {
        PRINTLOG(  "============================================================\n");
        error = ParseMIMEMsg( hNote );
        if ( error != NOERROR )
        {
            PRINTLOG( "Error when output MIME part content to file.\n" );
            return( error );        
        }
    
        PRINTLOG(  "============================================================\n");
        fflush(stdout);
    }

    error = NSFNoteClose (hNote);
    if ( error != NOERROR )
        return (ERR(error));

/* End of subroutine. */

    return (NOERROR);
}


/*************************************************************************

    FUNCTION:   ParseMIMEMsg

    PURPOSE:    Parse the input note, print its structure to the output 
                file if it's a multipart mime message.

**************************************************************************/
STATUS  LNPUBLIC  ParseMIMEMsg(NOTEHANDLE hNote )
{  
    STATUS              error=NOERROR;
    int                 i;
    HMIMEDIRECTORY      hMD = NULL;
    PMIMEENTITY         pMERoot;
    MIMESYMBOL          iContentType;
    MIMESYMBOL          iSubType;  
    char                szName[MAX_NAME_LEN];  
    int                 iNameLen;  
  
    error = MIMEOpenDirectory(hNote, &hMD);
    if (error != NOERROR)
    {
        PRINTERROR(error,"MIMEOpenDirectory");
        goto done;
    }

    /* get the root entity */
    error = MIMEGetRootEntity(hMD, &pMERoot);
    if (error != NOERROR)
    {
        PRINTERROR(error,"MIMEGetRootEntity");
        goto done;
    }

    /* Check the message type.
    We don't parse this note if it is not a multi part message */
    if ( MIMEEntityIsDiscretePart(pMERoot) == TRUE )
        PRINTLOG( "This is not neither a message nor a multipart.\n");
 
    if ( MIMEEntityIsMessagePart(pMERoot) == TRUE )
        PRINTLOG(  "This is a message part.\n" );

    fflush(stdout);
    if ( MIMEEntityIsMultiPart(pMERoot) == TRUE)
    {
        PRINTLOG( "This is a multipart message:\n\n");
        PRINTLOG( "Header:\n");
        fflush(stdout);
        /* first print the header to the output file. */
        for ( i=0; mszHeaderList[i]; i++ )
        {
            if (MIMEEntityGetHeader(pMERoot,mszHeaderList[i]) != NULL)
                PRINTLOG(  "\t[%s : %s]\n", pszHeaderName[i],
                           MIMEEntityGetHeader(pMERoot, mszHeaderList[i]) );
            fflush(stdout);
        }

        /* 2nd print the body text to the output file. */
        PRINTLOG(  "\nBody:\n");  

        iContentType = MIMEEntityContentType(pMERoot);
        iSubType = MIMEEntityContentSubtype(pMERoot);

        GetSymbolName(iContentType, szName, &iNameLen);
        PRINTLOG(  "Content-Type: %s/",  szName );
        GetSymbolName(iSubType, szName, &iNameLen);
        PRINTLOG(  "%s\n", szName );
        fflush(stdout);

        error = ScanMIMEMsgBody( hNote, hMD, pMERoot, 0 );
        if ( error != NOERROR )
        {
            PRINTLOG( "Error when scan the multi part mime body item.\n" );
            goto done;
        }
    } 
    
done:
    if ( hMD )
        MIMEFreeDirectory(hMD);

    if ( error != NOERROR )
        return (1);
    else
        return (NOERROR);
}

/*************************************************************************

    FUNCTION:   ScanMIMEMsgBody

    PURPOSE:    Recursively scan all the MIME message body part and print 
                their content type and subcontent type.

**************************************************************************/
STATUS  LNPUBLIC  ScanMIMEMsgBody( NOTEHANDLE hNote, HMIMEDIRECTORY hMD, PMIMEENTITY pME, int iDepth )
{
    STATUS              error=NOERROR;

    PMIMEENTITY         pMEChild=NULLPTR;
    MIMESYMBOL          iContentType;
    MIMESYMBOL          iSubType;  
 
    DHANDLE             hTypeParam;
    DWORD               dwTypeParamLen;
    char                *pczParaName;
    DWORD               dwPartFlags;
    char                szName[MAX_NAME_LEN];  
    int                 iNameLen;
    int                 i;

    iDepth ++; 

    /* start scan from the first child */
    error = MIMEGetFirstSubpart( hMD, pME, &pMEChild );
    if ( error != NOERROR )
    {
       PRINTERROR(error,"MIMEGetFirstSubpart");
        return (1);        
    }

    /* return if no child found */
    if ( pMEChild == NULLPTR || pMEChild == NULL)
        return (0);
    
    iContentType = MIMEEntityContentType(pMEChild);
    iSubType = MIMEEntityContentSubtype(pMEChild);

    /* loop until to the last child at the same level */
    while ( iContentType  != 0 )
    {
        for ( i=0; i<iDepth; i++ )
            PRINTLOG(  "\t" );        
        fflush(stdout);
        GetSymbolName(iContentType, szName, &iNameLen);
        PRINTLOG(  "Content-Type: %s/",  szName );
        GetSymbolName(iSubType, szName, &iNameLen);
        PRINTLOG(  "%s\n", szName );
        fflush(stdout);

        /* to see if the body includes attachment */
        error = MIMEGetEntityPartFlags( hNote,
                                        pMEChild,
                                        &dwPartFlags );
        if ( error != NOERROR )
        {
            PRINTERROR(error,"MIMEGetEntityPartFlags");
            return(1);
        }

        if ( dwPartFlags & MIME_PART_BODY_IN_DBOBJECT )
        {
            /* get the attachment file name */
            /* NOTES: embedded object has the same flag, so only output the file name */
            /*        when the function succeed      */
            error = MIMEEntityGetTypeParam(pMEChild,
                                           MIME_SYMBOL_NAME,
                                           &hTypeParam,
                                           &dwTypeParamLen );
            if (error == NOERROR)
            {
                pczParaName = OSLock(char, hTypeParam);
                OSUnlock(hTypeParam);  

                for ( i=0; i<iDepth; i++ )
                PRINTLOG(  "\t" );
                PRINTLOG(  "********Have an attachment :[%s]\n",  pczParaName );      
                fflush(stdout);
                OSMemFree(hTypeParam);
            }
        }

        /* Recursively scan */
        ScanMIMEMsgBody( hNote, hMD, pMEChild, iDepth );

        /* Now go to next part*/
        MIMEGetNextSibling( hMD, pMEChild, &pMEChild);
        if (pMEChild == NULLPTR || pMEChild == NULL)
            return 0;
        iContentType = MIMEEntityContentType(pMEChild);
        iSubType = MIMEEntityContentSubtype(pMEChild);
    }
  
    return (NOERROR);
}

/*************************************************************************

    FUNCTION:   GetSymbolName

    PURPOSE:    Interpret the input symbol to its corresponding name string. 

**************************************************************************/
void GetSymbolName(MIMESYMBOL symbol, char *szSymbolName, int *piSymbolNameLen )
{
    switch( symbol )
    {
        case MIME_SYMBOL_UNKNOWN:
             *piSymbolNameLen = sizeof("UnKnown");
             strncpy(szSymbolName, "UnKnown", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_TEXT:
             *piSymbolNameLen = sizeof("Text");
             strncpy(szSymbolName, "text", *piSymbolNameLen ); 
             break;       
        case MIME_SYMBOL_MULTIPART:
             *piSymbolNameLen = sizeof("mulipart");
             strncpy(szSymbolName, "mulipart", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_MESSAGE:
             *piSymbolNameLen = sizeof("message");
             strncpy(szSymbolName, "message", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_APPLICATION:
             *piSymbolNameLen = sizeof("application");
             strncpy(szSymbolName, "application", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_IMAGE:
             *piSymbolNameLen = sizeof("image");
             strncpy(szSymbolName, "image", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_AUDIO:
             *piSymbolNameLen = sizeof("audio");
             strncpy(szSymbolName, "audio", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_VIDEO:
             *piSymbolNameLen = sizeof("video");
             strncpy(szSymbolName, "video", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_NONE:
             *piSymbolNameLen = sizeof("none");
             strncpy(szSymbolName, "none", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_TO:
             *piSymbolNameLen = sizeof("SendTo");
             strncpy(szSymbolName, "SendTo", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_FROM:
             *piSymbolNameLen = sizeof("From");
             strncpy(szSymbolName, "from", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_COMMENTS:
             *piSymbolNameLen = sizeof("Comments");
             strncpy(szSymbolName, "Comments", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_DATE:
             *piSymbolNameLen = sizeof("Date");
             strncpy(szSymbolName, "Date", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_ENCRYPTED:
             *piSymbolNameLen = sizeof("Encrypt");
             strncpy(szSymbolName, "Encrypt", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_IN_REPLY_TO:
             *piSymbolNameLen = sizeof("ReplyTo");
             strncpy(szSymbolName, "encrypted", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_KEYWORDS:
             *piSymbolNameLen = sizeof("KeyWords");
             strncpy(szSymbolName, "KeyWords", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_PLAIN:
             *piSymbolNameLen = sizeof("plain");
             strncpy(szSymbolName, "plain", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_OCTET_STREAM:
             *piSymbolNameLen = sizeof("octet-stream");
             strncpy(szSymbolName, "octet-stream", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_HTML:
             *piSymbolNameLen = sizeof("html");
             strncpy(szSymbolName, "html", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_INLINE:
             *piSymbolNameLen = sizeof("inline");
             strncpy(szSymbolName, "inline", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_ATTACHMENT:
             *piSymbolNameLen = sizeof("attachment");
             strncpy(szSymbolName, "attachment", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_FILENAME:
             *piSymbolNameLen = sizeof("filename");
             strncpy(szSymbolName, "filename", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_ALTERNATIVE:
             *piSymbolNameLen = sizeof("alternative");
             strncpy(szSymbolName, "alternative", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_MIXED:
             *piSymbolNameLen = sizeof("mixed");
             strncpy(szSymbolName, "mixed", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_RELATED:
             *piSymbolNameLen = sizeof("related");
             strncpy(szSymbolName, "related", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_GIF:
             *piSymbolNameLen = sizeof("gif");
             strncpy(szSymbolName, "gif", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_JPEG:
             *piSymbolNameLen = sizeof("jpeg");
             strncpy(szSymbolName, "jpeg", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_BMP:
             *piSymbolNameLen = sizeof("bmp");
             strncpy(szSymbolName, "bmp", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_CGM:
             *piSymbolNameLen = sizeof("cgm");
             strncpy(szSymbolName, "cgm", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_CSS:
             *piSymbolNameLen = sizeof("css");
             strncpy(szSymbolName, "css", *piSymbolNameLen ); 
             break;
        case MIME_SYMBOL_TIFF:
             *piSymbolNameLen = sizeof("tiff");
             strncpy(szSymbolName, "tiff", *piSymbolNameLen ); 
             break;
        default:
             *piSymbolNameLen = sizeof("UnKnowName");
             strncpy(szSymbolName, "UnKnowName", *piSymbolNameLen ); 
    }
}
