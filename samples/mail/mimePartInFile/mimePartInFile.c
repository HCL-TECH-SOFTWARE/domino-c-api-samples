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


    PROGRAM: mimePartInFile   

    FILE:    mimePartInFile.c   

    SYNTAX:  mimePartInFile <recipient>   

    PURPOSE: Shows how to send a MIME message part as a file content
             and uses NSFMimePartInFile to check message is in
             file content.
****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

#include <stdio.h>
#include <string.h>
#include <global.h>
#include <globerr.h>
#include <oserr.h>
#include <names.h>
#include <stdnames.h>                   /* FIELD_FORM, etc */
#include <osfile.h>
#include <nsf.h>
#include <osenv.h>                      /* OSGetEnvironmentString */
#include <nsfdb.h>
#include <nsfnote.h>
#include <mail.h>
#include <nsf.h>                        /* MAXUSERNAME, etc. */
#include <kfm.h>                        /* SECKFMGetUserName */
#include <mail.h>                       /* MAIL_SENDTO_ITEM_NUM, etc. */
#include <mailserv.h>                   /* MailOpenMessageFile */
#include <neterr.h>                     /* ERR_NO_NETBIOS */
#include <clerr.h>                      /* ERR_SERVER_NOT_RESPONDING */
#include <osmisc.h>
#include "../../printLog.h"

#include "mime.h"
#include "nsfmime.h"
#include "mimeods.h"

/* Global variables */
#define  STR_CRLF               "\015\012"
#define  ATTACHMENT_NAME        "New.bmp"
#define  MIME_FILE_NAME         "test.eml"   //Mime file name for exporting
#define  MAILBOX                "mail.box"
#define  MAIL2BOX               "mail2.box"
#define  ITEM_BODY              "Body"


char *pszMsgHeader[] =
{
     /* headers */
     "To: administrator <dadmin@sample.com>",
     "From: Dizzy Dean <ddean@cardinals.com>",
     "Subject: Test message: mp/alt with text/plain, text/html",
     "Date: Fri, 02 Jun 2006 08:21:37 -0400",
     "MIME-Version: 1.0",
     NULL
};

char *pszMsgBody1 =
        "Content-type: multipart/mixed; boundary=" "\"__MixedBoundaryString__\"" STR_CRLF
        "Content-Type: image/bmp; name=\"New.bmp\"" STR_CRLF
        "Content-Transfer-Encoding: base64" STR_CRLF
        "Content-Disposition: attachment; filename=\"New.bmp\"" STR_CRLF       
        "" STR_CRLF; 

char *pszMsgBody2 =
        "--__MixedBoundaryString__--" STR_CRLF  
        "" STR_CRLF;

char        szMailServerName[MAXUSERNAME+1];
char        szMailFileName[MAXUSERNAME+1] = MAILBOX_NAME;
char        szMailBoxPath[MAXPATH+1] = { 0 };

/* Function declaration */
STATUS  CreateRFC822Header( char **, NOTEHANDLE );
STATUS  AppendMIMEPart( NOTEHANDLE );
STATUS 	CheckMIMEPartInFile( NOTEHANDLE );

/************************************************************************

    FUNCTION:   main
    DESCRIPTION:  Main entry point.
    ARGS:
         argc - Number of arguments
     argv - array of character pointer
         return -  1 Error
               0 NoError
    
    USAGE:    mimePartInFile <Domino user name>
*************************************************************************/
int main( int argc, char * argv[] )
{
    STATUS      error = NOERROR;


    char        szRecipient[MAXUSERNAME+1] = { 0 };
    DBHANDLE    hMailBox = NULLHANDLE;
    NOTEHANDLE  hMemo = NULLHANDLE;
    
    /* Parse  the input parameter*/ 
    if ( argc > 2 )
    {
        PRINTLOG( "Error: incorrect syntax.\n" );
        PRINTLOG( "\nUsage:%s  <recipient>\n", argv[0] );
        return 1;
    }

    if ( argc == 2 )
    {
        strncpy( szRecipient, argv[1], sizeof( szRecipient )-1 );
    }
    else
    {
        PRINTLOG( "Error: incorrect syntax.\n" );
        PRINTLOG( "\nUsage:%s  <recipient>\n", argv[0] );
        return 1;
    }
        
    /* Initiate  the environment */
    if ( error = NotesInitExtended( argc, argv ) )
    {
        PRINTLOG( "\n Unable to initialize Notes. Error Code[0x%04x]\n", error );
        return 1;
    }
    
    /* Open the mail.box on server */
    if ( !OSGetEnvironmentString( "MAILSERVER", szMailServerName, MAXUSERNAME ) )
    {
        PRINTLOG( "\nUnable to get mail server name ...\n\n Adding message local 'mail.box' file ...\n\n" );
        szMailServerName[0]='\0';

        /* In the event of multiple "mail.box" files, if the NOTES.INI 
        parameter "MAIL_ENABLE_MAILBOX_COMPATIBILITY =1" is set, then 
        the file "mail.box" will be used, otherwise "mail2.box" will be 
        used. */

        if ( !OSGetEnvironmentInt( "MAIL_ENABLE_MAILBOX_COMPATIBILITY" ) )
        {
            PRINTLOG( "\nEnable mailbox parameter is not set ...\n\n Adding message to local 'mail2.box' file ...\n\n" );
            strncpy( szMailFileName, MAIL2BOX, sizeof( szMailFileName )-1 );
        }
    }

    PRINTLOG( "mail path :[%s].\n", szMailServerName );    

    /* In the event of multiple "mail.box" databases, ensure message 
    is successfully deposited. */
    do
    {
        if ( error = OSPathNetConstruct( NULL,               /* port name  */
                                         szMailServerName,
                                         szMailFileName,
                                          szMailBoxPath) )
        {
            PRINTLOG( "Error: Server Name path error\n" );
            PRINTERROR( error,"OSPathNetConstruct" );
            goto Done;
        } 

        if ( error = NSFDbOpen( szMailBoxPath, &hMailBox ) )
        {

            /* If multiple mail.box files do not exist, place memo 
            in standard "mail.box" file. */
            if (( error == ERR_NOEXIST ) && ( !strcmp( szMailFileName, MAIL2BOX )) )
            {
                strncpy( szMailFileName, MAILBOX, sizeof( szMailFileName )-1 );
            }
            else
            {
                PRINTLOG( "Error: unable to open '%s'.\n", szMailBoxPath );
                PRINTERROR( error,"NSFDbOpen" );
                goto Done;
            }
        }
    } while( error );

    /* Create new note in mail.box */
    if ( error = NSFNoteCreate( hMailBox, &hMemo ) )
    {
        PRINTLOG( "Error: unable to create memo in %s.\n", szMailBoxPath );
        PRINTERROR( error,"NSFNoteCreate" );
        goto Done1;
    }

    /* create RFC822 Header to the note created */
    if ( error = CreateRFC822Header( pszMsgHeader, hMemo ) )
    {
        PRINTLOG( "Error: Fail to create MIME mail.\n" );
        PRINTERROR( error,"CreateRFC822Header" );
        goto Done2;
    }

    if ( error = AppendMIMEPart( hMemo ) )
    {
        PRINTLOG( "Error: Fail to create MIME mail.\n" );
        PRINTERROR( error,"AppendMIMEPart" );
        goto Done2;
    }

     /* Append the recipient to the note*/
    if ( error = NSFItemSetText( hMemo, /* use NSFItemCreateTextList if > 1*/
                                 MAIL_RECIPIENTS_ITEM,   /* "Recipients" */
                                 szRecipient,
                                 MAXWORD) )
    {
        PRINTLOG( "Error: unable to set item '%s' into memo.\n",
                  MAIL_RECIPIENTS_ITEM );
        PRINTERROR( error,"NSFItemSetText" );
        goto Done2;
    }

    if ( error = NSFNoteUpdate( hMemo, UPDATE_DUPLICATES ) )
    {
        PRINTLOG( "Error: unable to update note in %s.\n", szMailBoxPath );
        PRINTERROR( error,"NSFNoteUpdate" );
        goto Done2;
    }

    /* To check created mime part message is in file */
    error = CheckMIMEPartInFile( hMemo );

    if ( error )
    {
        PRINTERROR( error,"CheckMIMEPartInFile" );
    }
    else if ( error == NOERROR )
    {
        PRINTLOG( "\nSuccessfully completed!!\n" );
    }

Done2:
    NSFNoteClose( hMemo );

Done1:
    NSFDbClose( hMailBox );

Done:
    NotesTerm();
    if ( error )
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


/*************************************************************************
 *  FUNCTION: CheckMIMEPartInFile
 *
 *  DESCRIPTION: Check the Note has MIME part in file as content.
 *
 *  INPUT: hNote - Open Note Handle.
 *  
 *  RETURN: STATUS - Error from API on failure, otherwise NOERROR.
 *************************************************************************/
STATUS CheckMIMEPartInFile( NOTEHANDLE hNote )
{

    BLOCKID bhPart = { NULL };
    char szFileName[MAXPATH+1] = { 0 };
    STATUS error = NOERROR;
    WORD wPartType = 0;
    WORD wPartLen = 0;
    WORD wFileLen = 0;
    NOTEID  noteID  = NULL;

    NSFNoteGetInfo( hNote, _NOTE_ID, &noteID );

    /* Get the info for the part we appended */
    error = NSFMimePartGetInfo( hNote,
                                ITEM_BODY,
                                strlen(ITEM_BODY),
                                &wPartType,
                                NULL,
                                NULL,
                                &wPartLen,
                                &bhPart );
    if ( ERR( error ) != NOERROR )
    {
        return error;
    }

    if ( wPartType != MIME_PART_BODY )
    {
        PRINTLOG( "\nPart type is not MIME type.\n" );
        return error;
    }

    if ( NSFIsMimePartInFile( hNote, bhPart, szFileName, sizeof( szFileName )-1 ) )
    {
        PRINTLOG( "\nMIME part content is in file.\n" );
        wFileLen = strlen( szFileName );
        if ( wFileLen )
        {
            PRINTLOG( "File Name from MIME part [%s]\n", szFileName );
        } 
        else
        {
           PRINTLOG( "File Name not present.\n" );
        }
    }
    else 
    {
         PRINTLOG( "\nMIME part is not present in file.\n" );
    }

    PRINTLOG( "\n Noteid[%d]", noteID );
    error = MIMEEMLExport3( szMailBoxPath, noteID, MIME_FILE_NAME, TRUE );
    if ( error != NOERROR )
    {
    PRINTLOG( "\nMIMEEMLExport3 Error:" );
        return error;
    }
    else
    {
       PRINTLOG( "\nSuccessfully created MIME EML File [%s].\n", MIME_FILE_NAME );
    }
    return error;
}


/*************************************************************************

    FUNCTION:   CreateRFC822Header

    PURPOSE:  Create the message header items  

**************************************************************************/
STATUS  CreateRFC822Header( char **pszMsgWriteLines, NOTEHANDLE hNote )
{
    STATUS      error = NOERROR;          
    MIMEHANDLE  hMIMEStream = NULLMIMEHANDLE;
    int iter = 0;
 
    /* Test for writing messages to stream. */
    error = MIMEStreamOpen( hNote,               /* note handle */
                            NULL,                /* item name */
                            0,                   /* item name length */
                            MIME_STREAM_OPEN_WRITE,   /* open flags */
                            &hMIMEStream );
    if ( error != NOERROR )
    {
          return error;
    }

    /* write the test message to the open stream, line by line */
    for ( iter=0; pszMsgWriteLines[iter]; ++iter)
    {
        error = (STATUS) MIMEStreamPutLine( (char *)pszMsgWriteLines[iter],
                                            hMIMEStream );
        if ( error == MIME_STREAM_IO )
        {
            PRINTLOG( "MIMEStreamPutLine error.\n" );
            MIMEStreamClose( hMIMEStream );
            return error;
        }
    }

    /* itemize the mime stream to the note */
    error = MIMEStreamItemize( hNote,
                               NULL,
                               0,
                               MIME_STREAM_ITEMIZE_FULL,
                               hMIMEStream );
    if ( error != NOERROR )
    {
          MIMEStreamClose( hMIMEStream );
          return error;
    }

    MIMEStreamClose( hMIMEStream );

    return NOERROR;
}


/*************************************************************************

    FUNCTION:   AppendMIMEPart

    PURPOSE:    Append the multipart MIME Body items and the inline
                attachment.

    RETURN:     STATUS - error on failure, NOERROR otherwise. 
**************************************************************************/
STATUS  AppendMIMEPart( NOTEHANDLE hNote )
{
    STATUS error = NOERROR, sErr = NOERROR;
    DHANDLE hCtx = NULLHANDLE;

    PRINTLOG( " Append Body 1.\n" );
    error = NSFMimePartCreateStream( hNote,               /* note handle */
                                     ITEM_BODY,                 /* item name to append */
                                     (WORD)strlen(ITEM_BODY),   /* length of item name */
                                     MIME_PART_BODY,         /* type of MIME part item */
                                     MIME_PART_HAS_BOUNDARY | MIME_PART_HAS_HEADERS | MIME_PART_BODY_IN_DBOBJECT,     /* flags for MIME part */
                                     &hCtx );
    if ( error )
    {
        return error;
    }

    if ( (error = NSFMimePartAppendStream( hCtx, pszMsgBody1, strlen( pszMsgBody1 ) )) ) 
    {
        PRINTLOG( "Error: Fail to append MIME mail.\n" );
        goto ERRORAPPEND;
    }

    if ( error = NSFMimePartAppendFileToStream( hCtx, ATTACHMENT_NAME ) ) 
    {
        PRINTLOG( "Error: fail to append file to stream.\n" );
        goto ERRORAPPEND;
    } 

ERRORAPPEND: 
    sErr = NSFMimePartCloseStream( hCtx, TRUE );
    if ( sErr )
    {
        if ( error )
        {
            PRINTERROR( error,"NSFMimePartAppendStream" );
        }
        return sErr;
    }
    if ( error )
    {
        return error;
    }

    PRINTLOG( " Append Body 2.\n" );
    error = NSFMimePartCreateStream( hNote,                  /* note handle */
                                     ITEM_BODY,                     /* item name to append */
                                     (WORD)strlen(ITEM_BODY),       /* length of item name */
                                     MIME_PART_BODY,             /* type of MIME part item */
                                     MIME_PART_HAS_BOUNDARY,     /* flags for MIME part */
                                     &hCtx );
    if ( error )
    {
        return error;
    }

    if ( error = NSFMimePartAppendStream( hCtx, pszMsgBody2, strlen( pszMsgBody2 )) ) 
    {
        PRINTLOG( "Error: Fail to append MIME mail.\n" );
    }

    sErr = NSFMimePartCloseStream( hCtx, TRUE );
    if ( sErr )
    {
        if ( error )
        {
            PRINTERROR( error,"NSFMimePartCloseStream" );
        }
        return sErr;
    }
    if ( error )
    {
        return error;
    }

    return NOERROR;
}
