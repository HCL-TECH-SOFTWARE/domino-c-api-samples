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

    PROGRAM:    sendeasyMM

    FILE:       sendeasyMM.c

    SYNTAX:      sendMIME <Recipient>
              or sendMIME 

    PURPOSE:    Send a MIME format mail note using NSF functions.

    DESCRIPTION:
      This sample demonstrates how to use the Notes MIME APIs to create a 
      simple MIME message, and send it to a Recipient specified in command
      line. The recipeient can be different with the "SendTo" defined in this 
      sample.

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
#include <osmisc.h>
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
#include "../../printLog.h"



#include "mime.h"

/* Constants */
char *szMsgRecipient = "administrator <admin@cn.ibm.com>";
char *pszMsgLines[] =
{
     /* headers */
     "To: administrator <admin@cn.ibm.com>",
     "From: Dizzy Dean <ddean@cardinals.com>",
     "Subject: Test message: mp/alt with text/plain, text/html",
     "Date: Fri, 02 Jun 2006 08:21:37 -0400",
     "MIME-Version: 1.0",
     "Content-type: multipart/alternative; boundary=" "__BoundaryString__",

     /* this 'blank' line terminates the headers */
     "",

     /* start of the body (which begins with a multipart preamble) */
     "This is a MIME Message.  If you can read this, you should treat",
     "yourself to a real mail reader....",
     "",
     "",
     "--__BoundaryString__",
     "Content-Type: text/plain",
     "",
     "This is a test message with two renditions of this text.",
     "",
     "--__BoundaryString__",
     "Content-Type: text/html",
     "",
     "<I>This is a test message with two renditions of <B>this</B> text.</I>",
     "",
     "--__BoundaryString__--",
     NULL
};

/* */
STATUS  PrintMail();
STATUS  CreateMIMEMsg(char **, NOTEHANDLE);
char        szMailServerName[MAXUSERNAME+1];
char        szMailFileName[MAXUSERNAME+1] = MAILBOX_NAME;
TIMEDATE      tdDate;
char          szDate[MAXALPHATIMEDATE+1];
char          szSubject[255];
char          szSendTo[MAXUSERNAME+1];
wchar_t       wszSendTo[MAXUSERNAME+1];
char          szMailFile[MAXUSERNAME+1];
wchar_t       wszMailFile[MAXUSERNAME+1];
char          szCopyTo[MAXUSERNAME+1];
char          szFrom[MAXUSERNAME+1];

 

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
        PRINTLOG ("Error: incorrect syntax.\n");
        PRINTLOG ("\nUsage:%s  <recipient>\n", argv[0]);
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
        PRINTLOG("\n Unable to initialize Notes. Error Code[0x%04x]\n", error);
        return (1);
    }
    
    /* Open the mail.box on server */
    if (!OSGetEnvironmentString("MAILSERVER", szMailServerName, MAXUSERNAME))
    {
        PRINTLOG ("\nUnable to get mail server name ...\n\n Adding message local 'mail.box' file ...\n\n");
        strcpy(szMailServerName,"");

        /* In the event of multiple "mail.box" files, if the NOTES.INI 
        parameter "MAIL_ENABLE_MAILBOX_COMPATIBILITY =1" is set, then 
        the file "mail.box" will be used, otherwise "mail2.box" will be 
        used. */

        if (!OSGetEnvironmentInt("MAIL_ENABLE_MAILBOX_COMPATIBILITY"))
        {
            PRINTLOG ("\nEnable mailbox parameter is not set ...\n\n Adding message to local 'mail2.box' file ...\n\n");
            strcpy(szMailFileName, "mail2.box");
        }
    }

    PRINTLOG( "mail path :[%s].\n", szMailServerName );    

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
                PRINTLOG ("Error: unable to open '%s'.\n", szMailBoxPath);
                goto Done;
            }
        }
    }
    while (error);

    /* Create new note in mail.box */
    if (error = NSFNoteCreate(hMailBox, &hMemo))
    {
        PRINTLOG ("Error: unable to create memo in %s.\n", szMailBoxPath);
        PRINTERROR (error,"NSFNoteCreate");
        goto Done1;
    }

    /* create MIME format mail */
    if ( error = CreateMIMEMsg(pszMsgLines, hMemo) )
    {
        PRINTLOG ("Error: Fail to creae MIME mail.\n");
        goto Done2;
    }

    /* Append the recipient */
    /* For sending a Notes Memo, you must specify then recipient */
    if (error = NSFItemSetText( hMemo, /* use NSFItemCreateTextList if > 1*/
                                MAIL_RECIPIENTS_ITEM,   /* "Recipients" */
                                szRecipient,
                                MAXWORD))
    {
        PRINTLOG ("Error: unable to set item '%s' into memo.\n",
                  MAIL_RECIPIENTS_ITEM);
        PRINTERROR (error,"NSFItemSetText");
        goto Done2;
    }

    if (error = NSFNoteUpdate(hMemo, 0))
    {
        PRINTLOG ("Error: unable to update note in %s.\n", szMailBoxPath);
        PRINTERROR (error,"NSFNoteUpdate");
        goto Done2;
    }

     PRINTLOG ("successfully deposited memo \n");

Done2:
    NSFNoteClose(hMemo);

Done1:
    NSFDbClose(hMailBox);
    if (error == NOERROR)
        PrintMail();

Done:
    NotesTerm();
    return (error); 
}

/*************************************************************************

    FUNCTION:   CreateMIMEMsg

    PURPOSE: 

**************************************************************************/
STATUS  CreateMIMEMsg(char **pszMsgWriteLines, NOTEHANDLE hNote)
{
    STATUS      error = NOERROR;          
    MIMEHANDLE  hMIMEStream;
    int   i;
 
    /* Test for writing messages to stream. */
    error = MIMEStreamOpen(hNote,       /* note handle */
                           NULL,           /* item name */
                           0,              /* item name length */
                           MIME_STREAM_OPEN_WRITE, /* open flags */
                           &hMIMEStream );
    if ( error != NOERROR)
    {
          PRINTERROR (error,"MIMEStreamOpen");
          return (1);
    }

    /* write the test message to the open stream, line by line */
    for ( i=0; pszMsgWriteLines[i]; ++i)
    {
        error = MIMEStreamPutLine((char *)pszMsgWriteLines[i],
                                   hMIMEStream);
        if ( error == MIME_STREAM_IO )
        {
            PRINTLOG("MIMEStreamPutLine error.\n");
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
        PRINTERROR (error,"MIMEStreamItemize");
        MIMEStreamClose(hMIMEStream);
        return (1);
    }

    MIMEStreamClose(hMIMEStream);
   
    return NOERROR;
}

/************************************************************************
 *     Validate mail sent and Print
 **************************************************************************/

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
        PRINTLOG ("Error: unable to open '%s'.\n", szMailFilePath);
        goto Exit0;
    }

    /* Create message list of messages in the file - just 64K */

    if (error = MailCreateMessageList(hMessageFile,
                                      &hMessageList, &MessageList, &MessageCount))
    {
        PRINTLOG ("Error: unable to create message list.\n");
        goto Exit1;
    }

    PRINTLOG ("Mail file contains %d message(s).\n", MessageCount);

    /* Print out each of the outbound messages. */

    for (Msg = 0; Msg < MessageCount; Msg++)
    {
        PRINTLOG ("\nMessage #%d: \n", Msg+1);

        if (error = MailOpenMessage (MessageList, Msg, &hMessage))
        {
            PRINTLOG ("Error: unable to open message number %d.\n", Msg+1);
            goto Exit2;
        }

        /* Get the originator's name/address. */

        if (error = MailGetMessageOriginator(MessageList, Msg,
                                             Originator, sizeof(Originator)-1, &OriginatorLength))
        {
            PRINTLOG ("Error: unable to get message originator.\n");
            goto Exit2;
        }

        Originator[OriginatorLength] = '\0';

        PRINTLOG ("\tOriginator = '%s'\n", Originator);
        if (error = MailGetMessageInfo(MessageList, Msg,
                                       &RecipientCount, NULL, NULL))
        {
            PRINTLOG ("Error: unable to get number of recipients in message.\n");
            MailCloseMessage (hMessage);
            goto Exit2;
        }

        PRINTLOG ("\tNumber of Recipients = %d.\n", RecipientCount);

        for (Rec = 0; Rec < RecipientCount; Rec++)
        {
            MailGetMessageRecipient(MessageList, Msg, Rec, RecipientName,
                                    sizeof(RecipientName)-1, &RecipientNameLength);
            MailParseMailAddress(RecipientName, RecipientNameLength,
                                 UserName, sizeof(UserName)-1, &UserNameLength,
                                 DomainName, sizeof(DomainName)-1, &DomainNameLength);

            UserName[UserNameLength] = '\0';
            DomainName[DomainNameLength] = '\0';

            PRINTLOG ("\t\tRecipient %d = '%s'\t Domain = '%s'\n", Rec+1,
                                UserName, DomainName);
        }   /* end of loop over recipients */

        /* SendTo */
        MailGetMessageItem (hMessage, MAIL_SENDTO_ITEM_NUM, String,
                            MAXSPRINTF, &StringLength);

        String[StringLength] = '\0';
        PRINTLOG ("\tTo: %s\n", String);

        /* CopyTo */
        MailGetMessageItem (hMessage, MAIL_COPYTO_ITEM_NUM, String,
                            MAXSPRINTF, &StringLength);
        String[StringLength] = '\0';
        PRINTLOG ("\tCc: %s\n", String);

        /* From */
        MailGetMessageItem (hMessage, MAIL_FROM_ITEM_NUM, String,
                            MAXSPRINTF, &StringLength);
        String[StringLength] = '\0';
        PRINTLOG ("\tFrom: %s\n", String);

                /* PostedDate */
        MailGetMessageItemTimeDate(hMessage, MAIL_POSTEDDATE_ITEM_NUM, &Time);
        ConvertTIMEDATEToText(NULL, NULL, &Time, String,
                                    sizeof(String) - 1, &StringLength);
        String[StringLength] = '\0';
        PRINTLOG("\tDate: %s\n", String);

        /* Subject. If non-delivery report, prefix with "NonDelivery of:" */

        MailGetMessageItem (hMessage, MAIL_SUBJECT_ITEM_NUM, String,
                            MAXSPRINTF, &StringLength);
        String[StringLength] = '\0';
        if (NonDeliveryReport = MailIsNonDeliveryReport(hMessage))
        {
            PRINTLOG ("\tNonDelivery of: %s\n", String);
        }
        else
        {
            PRINTLOG ("\tSubject: %s\n", String);
        }

        if (NonDeliveryReport)
        {
            MailGetMessageItem(hMessage, MAIL_INTENDEDRECIPIENT_ITEM_NUM,
                            String, sizeof(String), &StringLength);
            String[StringLength] = '\0';
            PRINTLOG("\tIntended Recipients: %s\n", String);

            MailGetMessageItem(hMessage, MAIL_FAILUREREASON_ITEM_NUM,
                            String, sizeof(String), &StringLength);
            String[StringLength] = '\0';
            PRINTLOG("\tFailure Reason: %s\n", String);
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
    {
        PRINTERROR(error,"MailOpenMessageFile"); 
    } else {
        PRINTLOG("\nProgram completed successfully.\n");
    }

    return(NOERROR);
}

