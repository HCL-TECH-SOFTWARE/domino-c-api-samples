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

    PROGRAM: sendAttMM   

    FILE:    sendAttMM.c   

    SYNTAX:  sendAttMM <SendTo>   
          or sendAttMM

    PURPOSE: Shows how to send a MIME message with embedded BMP.

    DESCRIPTION:
          This program creats a MIME message with an embedded BMPm, and
          send the message to the specified recipient.
      
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
#include <nsfdb.h>
#include <nsfnote.h>
#include <osenv.h>                      /* OSGetEnvironmentString */
#include <mail.h>
#include <nsf.h>                        /* MAXUSERNAME, etc. */
#include <kfm.h>                        /* SECKFMGetUserName */
#include <textlist.h>                   /* ListAllocate, etc. */
#include <ostime.h>                     /* OSCurrentTIMEDATE */
#include <osmem.h>                      /* OSMemFree, etc. */
#include <osenv.h>                      /* OSGetEnvironmentString */
#include <mail.h>                       /* MAIL_SENDTO_ITEM_NUM, etc. */
#include <mailserv.h>                   /* MailOpenMessageFile */
#include <neterr.h>                     /* ERR_NO_NETBIOS */
#include <clerr.h>                      /* ERR_SERVER_NOT_RESPONDING */
#include <osmisc.h>

#include "mime.h"
#include "nsfmime.h"
#include "mimeods.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/* Global variables */
#define  STR_CRLF               "\015\012"
#define  ATTACHMENT_NAME        "New.bmp"

/* predefined message */
char *szMsgRecipient = "administrator <admin@cn.ibm.com>";

char *pszMsgHeader[] =
{
     /* headers */
     "To: administrator <admin@cn.ibm.com>",
     "From: Dizzy Dean <ddean@cardinals.com>",
     "Subject: Test message: mp/alt with text/plain, text/html",
     "Date: Fri, 02 Jun 2006 08:21:37 -0400",
     "MIME-Version: 1.0",
     NULL
};

char *pszMsgBody1 =
     "Content-type: multipart/mixed; boundary=" "\"__MixedBoundaryString__\"" STR_CRLF
     "" STR_CRLF
     "This is a MIME Message.  If you can read this, you should treat" STR_CRLF
     "yourself to a real mail reader...." STR_CRLF
     "" STR_CRLF;

char *pszMsgBody2 =
        "--__MixedBoundaryString__" STR_CRLF
        "Content-Type: multipart/alternative; boundary=" "\"==AltBoundaryString==\"" STR_CRLF
        "" STR_CRLF;

char *pszMsgBody3 =
        "--==AltBoundaryString==" STR_CRLF
        "Content-Type: text/plain" STR_CRLF
        "" STR_CRLF
        "this is a test message" STR_CRLF
        "" STR_CRLF;


char *pszMsgBody4 =
        "--==AltBoundaryString==" STR_CRLF
        "Content-Type: text/html" STR_CRLF
        "" STR_CRLF
        "<br><font size=4 face=sans-serif> This is a <b>test message</b></font><br>" STR_CRLF
        "" STR_CRLF;

char *pszMsgBody5 =
        "--==AltBoundaryString==--" STR_CRLF 
        "" STR_CRLF;
  
char *pszMsgBody6 =
        "--__MixedBoundaryString__"  STR_CRLF       
        "Content-Type: image/bmp; name=\"New.bmp\"" STR_CRLF
        "Content-Transfer-Encoding: base64" STR_CRLF
        "Content-Disposition: attachment; filename=\"New.bmp\"" STR_CRLF       
        ""STR_CRLF; 

char *pszMsgBody7 =
        "--__MixedBoundaryString__--" STR_CRLF  
        "" STR_CRLF;

char        szMailServerName[MAXUSERNAME+1];
char        szMailFileName[MAXUSERNAME+1] = MAILBOX_NAME;
TIMEDATE    tdDate;
char        szDate[MAXALPHATIMEDATE+1];
char        szSubject[255];
char        szSendTo[MAXUSERNAME+1];
wchar_t     wszSendTo[MAXUSERNAME+1];
char        szMailFile[MAXUSERNAME+1];
wchar_t     wszMailFile[MAXUSERNAME+1];
char        szCopyTo[MAXUSERNAME+1];
char        szFrom[MAXUSERNAME+1];

/* */
void    PrintAPIError (STATUS);
STATUS  CreateRFC822Header(char **, NOTEHANDLE);
STATUS  AppendMIMEPart(NOTEHANDLE);
STATUS  PrintMail();
 

/************************************************************************

    FUNCTION:   main

*************************************************************************/
int main(int argc, char * argv[])
{
    STATUS      error = NOERROR;

    char        szMailBoxPath[MAXPATH+1];
    char        szRecipient[MAXUSERNAME+1];
    DBHANDLE    hMailBox;
    NOTEHANDLE  hMemo;
    
    /* Parse  the input parameter*/ 
    if ( argc > 2 )
    {
        printf ("Error: incorrect syntax.\n");
        printf ("\nUsage:%s  <recipient>\n", argv[0]);
        return (NOERROR);
    }

    memset( szRecipient, '\0', MAXUSERNAME+1 );
    if ( argc == 2 )
       memcpy( szRecipient, argv[1], strlen(argv[1]) );
    else
       memcpy( szRecipient, szMsgRecipient, strlen(szMsgRecipient) ); 
        
    /* Initiate  the environment */
    if (error = NotesInitExtended (argc, argv))
    {
        printf("\n Unable to initialize Notes. Error Code[0x%04x]\n", error);
        return (1);
    }
    
    /* Open the mail.box on server */
    if (!OSGetEnvironmentString("MAILSERVER", szMailServerName, MAXUSERNAME))
    {
        printf ("\nUnable to get mail server name ...\n\n Adding message local 'mail.box' file ...\n\n");
        strcpy(szMailServerName,"");

        /* In the event of multiple "mail.box" files, if the NOTES.INI 
        parameter "MAIL_ENABLE_MAILBOX_COMPATIBILITY =1" is set, then 
        the file "mail.box" will be used, otherwise "mail2.box" will be 
        used. */

        if (!OSGetEnvironmentInt("MAIL_ENABLE_MAILBOX_COMPATIBILITY"))
        {
           printf ("\nEnable mailbox parameter is not set ...\n\n Adding message to local 'mail2.box' file ...\n\n");
           strcpy(szMailFileName, "mail2.box");
        }
    }

    printf( "mail path :[%s].\n", szMailServerName );    

    /* In the event of multiple "mail.box" databases, ensure message 
    is successfully deposited. */
    do
    {
        OSPathNetConstruct( NULL,               /* port name  */
                            szMailServerName,
                            szMailFileName,
                            szMailBoxPath);

        if (error = NSFDbOpen (szMailBoxPath, &hMailBox))
        {

            /* If multiple mail.box files do not exist, place memo 
            in standard "mail.box" file. */
            if ((error == ERR_NOEXIST) && (!strcmp (szMailFileName, "mail2.box")))
            {
               strcpy(szMailFileName, "mail.box");
            }
            else
            {
               printf ("Error: unable to open '%s'.\n", szMailBoxPath);
               PrintAPIError(error);
               goto Done;
            }
        }
    }
    while (error);

    /* Create new note in mail.box */
    if (error = NSFNoteCreate(hMailBox, &hMemo))
    {
        printf ("Error: unable to create memo in %s.\n", szMailBoxPath);
        PrintAPIError(error);
        goto Done1;
    }

    /* create RFC822 Header to the note created */
    if ( error = CreateRFC822Header(pszMsgHeader, hMemo) )
    {
        printf ("Error: Fail to creae MIME mail.\n");
        goto Done2;
    }

    if ( error = AppendMIMEPart(hMemo) )
    {
        printf ("Error: Fail to creae MIME mail.\n");
        goto Done2;
    }

     /* Append the recipient to the note*/
    if (error = NSFItemSetText( hMemo, /* use NSFItemCreateTextList if > 1*/
                                MAIL_RECIPIENTS_ITEM,   /* "Recipients" */
                                szRecipient,
                                MAXWORD))
    {
        printf ("Error: unable to set item '%s' into memo.\n",
                                MAIL_RECIPIENTS_ITEM);
        PrintAPIError (error);
        goto Done2;
    }

    if (error = NSFNoteUpdate(hMemo, 0))
    {
        printf ("Error: unable to update note in %s.\n", szMailBoxPath);
        PrintAPIError (error);
        goto Done2;
    }

    printf ("successfully deposited memo \n");

Done2:
    NSFNoteClose(hMemo);

Done1:
    NSFDbClose(hMailBox);

    if(error == NOERROR)
      PrintMail();

Done:
    NotesTerm();
    return (error); 
}


/*************************************************************************

    FUNCTION:   PrintAPIError

    PURPOSE:    This function prints the HCL C API for Notes/Domino 
                error message associated with an error code.

**************************************************************************/

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

/*************************************************************************

    FUNCTION:   CreateRFC822Header

    PURPOSE:  Create the message header items  

**************************************************************************/
STATUS  CreateRFC822Header(char **pszMsgWriteLines, NOTEHANDLE hNote)
{
    STATUS      error = NOERROR;          
    MIMEHANDLE  hMIMEStream;
    int         i;
 
    /* Test for writing messages to stream. */
    error = MIMEStreamOpen(hNote,               /* note handle */
                           NULL,                /* item name */
                           0,                   /* item name length */
                           MIME_STREAM_OPEN_WRITE,   /* open flags */
                           &hMIMEStream );
    if ( error != NOERROR)
    {
          PrintAPIError (error);
          return (1);
    }

    /* write the test message to the open stream, line by line */
    for ( i=0; pszMsgWriteLines[i]; ++i)
    {
          error = MIMEStreamPutLine((char *)pszMsgWriteLines[i],
                                    hMIMEStream);
          if ( error == MIME_STREAM_IO )
          {
            printf("MIMEStreamPutLine error.\n");
            MIMEStreamClose(hMIMEStream);
            return (1);
          }
    }

    /* itemize the mime stream to the note */
    error = MIMEStreamItemize( hNote,
                               NULL,
                               0,
                               MIME_STREAM_ITEMIZE_FULL,
                               hMIMEStream);
    if ( error != NOERROR)
    {
          PrintAPIError (error);
          MIMEStreamClose(hMIMEStream);
          return (1);
    }

    MIMEStreamClose(hMIMEStream);

    return NOERROR;
}


/*************************************************************************

    FUNCTION:   AppendMIMEPart

    PURPOSE:    Append the multipart MIME Body items and the inline
                attachment.

**************************************************************************/
STATUS  AppendMIMEPart(NOTEHANDLE hNote)
{
    STATUS error=NOERROR;
    DHANDLE hCtx;

     error = NSFMimePartCreateStream(hNote,              /*note handle */
                                     "Body",                 /*item name to append */
                                     (WORD)sizeof("Body"),   /*length of item name */
                                     MIME_PART_BODY,         /*type of MIME part item */
                                     MIME_PART_HAS_HEADERS,  /*flags for MIME part */
                                     &hCtx);
     if (error)
     {
        PrintAPIError(error);
        return (1);
     }

     printf( " Append Body 1.\n" );
     if ( error = NSFMimePartAppendStream(hCtx, pszMsgBody1, strlen(pszMsgBody1)) ) 
     {
        printf ("Error: Fail to append MIME mail.\n");
        return (1);
     }

     error = NSFMimePartCloseStream(hCtx, TRUE);
     if ( error )
     {
         PrintAPIError(error);
         return (1);
     }
      
     printf( " Append Body 2.\n" );
     error = NSFMimePartCreateStream(hNote,                            /* note handle */
                                     "Body",                               /* item name to append */
                                     (WORD)sizeof("Body"),                 /* length of item name */
                                     MIME_PART_BODY,                       /* type of MIME part item */
                                     MIME_PART_HAS_BOUNDARY | MIME_PART_HAS_HEADERS,        /* flags for MIME part */
                                     &hCtx);
     if (error)
     {
        PrintAPIError(error);
        return (1);
     }

   
     if ( error = NSFMimePartAppendStream(hCtx, pszMsgBody2, strlen(pszMsgBody2)) ) 
     {
        printf ("Error: Fail to append MIME mail.\n");
        return (1);
     }

     error = NSFMimePartCloseStream(hCtx, TRUE);
     if ( error )
     {
         PrintAPIError(error);
         return (1);
     }
      
     printf( " Append Body 3.\n" );
     error = NSFMimePartCreateStream(hNote,                            /* note handle */
                                     "Body",                               /* item name to append */
                                    (WORD)sizeof("Body"),                 /* length of item name */
                                    MIME_PART_BODY,                       /* type of MIME part item */
                                    MIME_PART_HAS_BOUNDARY | MIME_PART_HAS_HEADERS,        /* flags for MIME part */
                                    &hCtx);
     if (error)
     {
        PrintAPIError(error);
        return (1);
     }

     if ( error = NSFMimePartAppendStream(hCtx, pszMsgBody3, strlen(pszMsgBody3)) ) 
     {
        printf ("Error: Fail to append MIME mail.\n");
        return (1);
     }

     error = NSFMimePartCloseStream(hCtx, TRUE);
     if ( error )
     {
          PrintAPIError(error);
          return (1);
     }
      
    
     printf( " Append Body 4.\n" );
     error = NSFMimePartCreateStream(hNote,                            /* note handle */
                                     "Body",                               /* item name to append */
                                     (WORD)sizeof("Body"),                 /* length of item name */
                                     MIME_PART_BODY,                       /* type of MIME part item */
                                     MIME_PART_HAS_BOUNDARY | MIME_PART_HAS_HEADERS,        /* flags for MIME part */
                                     &hCtx);
     if (error)
     {
        PrintAPIError(error);
        return (1);
     }

     if ( error = NSFMimePartAppendStream(hCtx, pszMsgBody4, strlen(pszMsgBody4)) ) 
     {
        printf ("Error: Fail to append MIME mail.\n");
        return (1);
     }

     error = NSFMimePartCloseStream(hCtx, TRUE);
     if ( error )
     {
         PrintAPIError(error);
         return (1);
     }
      

     printf( " Append Body 5.\n" );
     error = NSFMimePartCreateStream(hNote,              /* note handle */
                                     "Body",                 /* item name to append */
                                     (WORD)sizeof("Body"),   /* length of item name */
                                     MIME_PART_BODY,         /* type of MIME part item */
                                     MIME_PART_HAS_BOUNDARY, /* flags for MIME part */
                                     &hCtx);
     if (error)
     {
        PrintAPIError(error);
        return (1);
     }
 
     if ( error = NSFMimePartAppendStream(hCtx, pszMsgBody5, strlen(pszMsgBody5)) ) 
     {
        printf ("Error: Fail to append MIME mail.\n");
        return (1);
     }


     error = NSFMimePartCloseStream(hCtx, TRUE);
     if ( error )
     {
          PrintAPIError(error);
          return (1);
     }
      
     printf( " Append Body 6.\n" );
     error = NSFMimePartCreateStream(hNote,               /* note handle */
                                     "Body",                 /* item name to append */
                                     (WORD)sizeof("Body"),   /* length of item name */
                                     MIME_PART_BODY,         /* type of MIME part item */
                                     MIME_PART_HAS_BOUNDARY | MIME_PART_HAS_HEADERS,     /* flags for MIME part */
                                     &hCtx);
     if (error)
     {
        PrintAPIError(error);
        return (1);
     }

     if ( error = NSFMimePartAppendStream(hCtx, pszMsgBody6, strlen(pszMsgBody6)) ) 
     {
        printf ("Error: Fail to append MIME mail.\n");
        return (1);
     }

     if (error = NSFMimePartAppendFileToStream(hCtx, ATTACHMENT_NAME)) 
     {
        PrintAPIError(error);
        error = NSFMimePartCloseStream(hCtx, TRUE);
        if ( error )
        {
             PrintAPIError(error);
             return (1);
        }
        return (1);
     } 

     error = NSFMimePartCloseStream(hCtx, TRUE);
     if ( error )
     {
         PrintAPIError(error);
         return (1);
     }      

     printf( " Append Body 7.\n" );
     error = NSFMimePartCreateStream(hNote,                  /* note handle */
                                     "Body",                     /* item name to append */
                                     (WORD)sizeof("Body"),       /* length of item name */
                                     MIME_PART_BODY,             /* type of MIME part item */
                                     MIME_PART_HAS_BOUNDARY,     /* flags for MIME part */
                                     &hCtx);
     if (error)
     {
        PrintAPIError(error);
        return (1);
     }

     if ( error = NSFMimePartAppendStream(hCtx, pszMsgBody7, strlen(pszMsgBody7)) ) 
     {
        printf ("Error: Fail to append MIME mail.\n");
        PrintAPIError(error);
        error = NSFMimePartCloseStream(hCtx, TRUE);
        if ( error )
        {
             PrintAPIError(error);
             return (1);
        }
        return (1);
     }


     error = NSFMimePartCloseStream(hCtx, TRUE);
     if ( error )
     {
         PrintAPIError(error);
         return (1);
     }
      
     return (NOERROR);
}
 
/************************************************************************
 *  *     Validate mail sent and Print
 ***************************************************************************/

STATUS PrintMail()
{

    DHANDLE       hMessageFile;
    DHANDLE       hMessageList = NULLHANDLE, hMessage;
    DARRAY     *MessageList;
    WORD        MessageCount, Msg, RecipientCount, Rec;
    char        Originator[MAXRECIPIENTNAME+1];
    WORD        OriginatorLength;
    char        RecipientName[MAXRECIPIENTNAME+1];
    WORD        RecipientNameLength;
    char        UserName[MAXUSERNAME + 1];
    WORD        UserNameLength;
    char        DomainName[MAXDOMAINNAME+1];
    WORD        DomainNameLength;
    char        String[MAXSPRINTF+1];
    WORD        StringLength;
    TIMEDATE    Time;
    BOOL        NonDeliveryReport;
    char        szMailFilePath[MAXPATH+1];
    char        OriginalFileName[MAXPATH+1];
    STATUS      error;

    OSPathNetConstruct( NULL,               /* port name  */
                        szMailServerName,
                        szMailFileName,
                        szMailFilePath);

    /* Open the message file. */
    if (error = MailOpenMessageFile(szMailFilePath, &hMessageFile))
    {
        printf ("Error: unable to open '%s'.\n", szMailFilePath);
        goto Exit0;
    }

    /* Create message list of messages in the file - just 64K */

    if (error = MailCreateMessageList(hMessageFile,
                        &hMessageList, &MessageList, &MessageCount))
    {
        printf ("Error: unable to create message list.\n");
        goto Exit1;
    }
    printf ("Mail file contains %d message(s).\n", MessageCount);
    fflush(stdout);

    /* Print out each of the outbound messages. */

    for (Msg = 0; Msg < MessageCount; Msg++)
    {
        printf ("\nMessage #%d: \n", Msg+1);
        fflush(stdout);

        if (error = MailOpenMessage (MessageList, Msg, &hMessage))
        {
            printf ("Error: unable to open message number %d.\n", Msg+1);
            goto Exit2;
        }

        /* Get the originator's name/address. */

        if (error = MailGetMessageOriginator(MessageList, Msg,
                Originator, sizeof(Originator)-1, &OriginatorLength))
        {
            printf ("Error: unable to get message originator.\n");
            goto Exit2;
        }

        Originator[OriginatorLength] = '\0';

        printf ("\tOriginator = '%s'\n", Originator);
        fflush(stdout);
        if (error = MailGetMessageInfo(MessageList, Msg,
                            &RecipientCount, NULL, NULL))
        {
            printf ("Error: unable to get number of recipients in message.\n");
            MailCloseMessage (hMessage);
            goto Exit2;
        }

        printf ("\tNumber of Recipients = %d.\n", RecipientCount);
        fflush(stdout);

        for (Rec = 0; Rec < RecipientCount; Rec++)
        {
            MailGetMessageRecipient(MessageList, Msg, Rec, RecipientName,
                    sizeof(RecipientName)-1, &RecipientNameLength);
            MailParseMailAddress(RecipientName, RecipientNameLength,
                    UserName, sizeof(UserName)-1, &UserNameLength,
                    DomainName, sizeof(DomainName)-1, &DomainNameLength);

            UserName[UserNameLength] = '\0';
            DomainName[DomainNameLength] = '\0';

            printf ("\t\tRecipient %d = '%s'\t Domain = '%s'\n", Rec+1,
                                UserName, DomainName);
            fflush(stdout);
        }   /* end of loop over recipients */

        /* SendTo */
        MailGetMessageItem (hMessage, MAIL_SENDTO_ITEM_NUM, String,
                                        MAXSPRINTF, &StringLength);

        String[StringLength] = '\0';
        printf ("\tTo: %s\n", String);
        fflush(stdout);

        /* CopyTo */
        MailGetMessageItem (hMessage, MAIL_COPYTO_ITEM_NUM, String,
                                        MAXSPRINTF, &StringLength);
        String[StringLength] = '\0';
        printf ("\tCc: %s\n", String);
        fflush(stdout);

        /* From */
        MailGetMessageItem (hMessage, MAIL_FROM_ITEM_NUM, String,
                                        MAXSPRINTF, &StringLength);
        String[StringLength] = '\0';
        printf ("\tFrom: %s\n", String);
        fflush(stdout);

                /* PostedDate */
        MailGetMessageItemTimeDate(hMessage, MAIL_POSTEDDATE_ITEM_NUM, &Time);
        ConvertTIMEDATEToText(NULL, NULL, &Time, String,
                                    sizeof(String) - 1, &StringLength);
        String[StringLength] = '\0';
        printf("\tDate: %s\n", String);
        fflush(stdout);

        /* Subject. If non-delivery report, prefix with "NonDelivery of:" */

        MailGetMessageItem (hMessage, MAIL_SUBJECT_ITEM_NUM, String,
                                        MAXSPRINTF, &StringLength);
        String[StringLength] = '\0';
        if (NonDeliveryReport = MailIsNonDeliveryReport(hMessage))
        {
            printf ("\tNonDelivery of: %s\n", String);
            fflush(stdout);
        }
        else
        {
            printf ("\tSubject: %s\n", String);
            fflush(stdout);
        }

        if (NonDeliveryReport)
        {
            MailGetMessageItem(hMessage, MAIL_INTENDEDRECIPIENT_ITEM_NUM,
                            String, sizeof(String), &StringLength);
            String[StringLength] = '\0';
            printf("\tIntended Recipients: %s\n", String);
            fflush(stdout);

            MailGetMessageItem(hMessage, MAIL_FAILUREREASON_ITEM_NUM,
                            String, sizeof(String), &StringLength);
            String[StringLength] = '\0';
            printf("\tFailure Reason: %s\n", String);
            fflush(stdout);
        }
        MailCloseMessage (hMessage);
     }
Exit2:
    if (hMessageList != NULLHANDLE)
    {
        MailFreeMessageList(hMessageList,MessageList);
    }

Exit1:
    if (hMessageFile != NULLHANDLE)
        MailCloseMessageFile(hMessageFile);
Exit0:
    if (ERR(error))
       PrintAPIError(error);
    else 
    {
       printf("\nProgram completed successfully.\n");
       fflush(stdout);
    }

    return(NOERROR);
}


