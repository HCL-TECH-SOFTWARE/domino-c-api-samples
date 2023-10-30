/************************************************************************
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

    PROGRAM:    readmail

    FILE:       readmail.c

    PURPOSE:    Prints contents of message file to the screen

    SYNTAX:     readmail <server_name> <msg file database>

    DESCRIPTION:
        Readmail prints the entire contents of a Domino and Notes message
        file to the screen. Suitable target Domino databases include any
        database created using the Mail template.
        Warning: message files containing more than two or three messages
        may cause the output from readmail to scroll off the screen.
        READMAIL does not stop in response to Ctl+Break or similar
        interrupt. Test readmail on small message files. The file
        hfinn.nsf is a small message file included in the API toolkit.

*************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* OS and C include files */

#if defined(OS400)
#include <global.h>           /* global.h must be first for i5/OS */
#endif

#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#if defined(MAC)
#include <stdlib.h>
#endif

#if defined(UNIX) || defined(MAC)
#include <unistd.h>
#else
#include <stdlib.h>
#include <malloc.h>
#include <io.h>
#include <stdio.h>
#include <direct.h>
#include <fcntl.h>
#endif

/* HCL C API for Notes/Domino include files */

#if !defined(OS400)
#include <global.h>
#endif
#include <globerr.h>
#include <oserr.h>
#include <names.h>
#include <stdnames.h>                   /* FIELD_FORM, etc */
#include <mail.h>
#include <mailserv.h>
#include <nsf.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <nsfsearc.h>
#include <nsferr.h>
#include <osfile.h>                     /* OSPathNetConstruct */
#include <osmem.h>                      /* OSUnlockObject */
#include <osenv.h>                      /* OSGetEnvironmentString */
#include <osmisc.h>
#if defined(OS400)
#include <misc.h>
#endif
#include "../../printLog.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif


/* constants */
#define     READMAIL_BODY_LINELEN   40
#define     SERVER_NAME_LEN         80
#define     STDIN                   stdin
char szDriveLetter_blank[] = "";
char szExtensionName_TMP[] = "TMP";
char szNewline_Terminate[] = "\r\n";



/* function prototypes */

STATUS near pascal GetUniqueFileName(char *Drive, char *Ext,
                                        char *FileName);

void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                               char *server_name, char *db_name);


/************************************************************************

    FUNCTION:   main
    ALGORITHM:
        This opens the specified message file and read memos from it
        using the HCL C API for Notes/Domino Mail functions.

        Calls MailCreateMessageList only once, which reads in just the
        first 64K worth of messages, including their summary buffers.

        For each message, this prints out the following items:

            Originator
            Recipient Name(s) and Domain (s)
            SendTo
            CopyTo
            From
            PostedDate
            Subject
            Body
            Attachments

*************************************************************************/

int main(int argc, char *argv[])
{
    STATUS      error = NOERROR;
    char        *szServerName;
    char        *szMailFile;
    char        szMailFilePath[MAXPATH+1];
    DHANDLE     hMessageFile;
    DHANDLE     hMessageList = NULLHANDLE, hMessage;
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
    DWORD       BodyFileSize;
    char        BodyFileName[MAXPATH_OLE];
    FILE       *BodyFile;
    char        OriginalFileName[MAXPATH+1];
    WORD        Att;
    BLOCKID     bhAttachment;
    char        szServer[SERVER_NAME_LEN];           /* server names are < 80 chars */
    char        szDBName[MAXPATH];


    error = NotesInitExtended (argc, argv);

    if (error)
    {
        PRINTLOG("\nUnable to initialize Notes. Error Code[0x%04x]\n", error);
        return(1);
    }

    szServerName = szServer;
    szMailFile = szDBName;
    ProcessArgs(argc, argv, szServerName, szMailFile);

    OSPathNetConstruct( NULL,               /* port name  */
                        szServerName,
                        szMailFile,
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
                                             Originator, sizeof(Originator), &OriginatorLength))
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
                                    sizeof(RecipientName), &RecipientNameLength);
            MailParseMailAddress(RecipientName, RecipientNameLength,
                                 UserName, sizeof(UserName), &UserNameLength,
                                 DomainName, sizeof(DomainName), &DomainNameLength);

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

        /* Body */


        if (error = GetUniqueFileName(szDriveLetter_blank, szExtensionName_TMP, BodyFileName))
        {
            PRINTLOG ("Error: unable to create temporary file name.\n");
            MailCloseMessage (hMessage);
            goto Exit2;
        }
        if (error = MailGetMessageBodyText(hMessage,
                                           NULL,     /* Use standard Body item */
                                           szNewline_Terminate,
                                           80,        /* 80 chars per line */
                                           TRUE,     /* Convert TABs */
                                           BodyFileName,
                                           &BodyFileSize))
        {
            PRINTLOG ("Error: unable to get Message body into temporary file.\n");
#ifdef LINUX
            unlink(BodyFileName);
#else
            _unlink(BodyFileName);
#endif
            PRINTERROR(error,"MailGetMessageBodyText");
            MailCloseMessage (hMessage);
            goto Exit2;
        }

        /* Print each line of body text to the screen. */

    #if defined(OS400)
        if (!(BodyFile = fopen(BodyFileName, "rb")))
    #else
        if (!(BodyFile = fopen(BodyFileName, "r")))
    #endif
        {
            PRINTLOG ("Error: unable to open temporary file.\n");
            unlink(BodyFileName);
            MailCloseMessage (hMessage);
            goto Exit2;
        }

        PRINTLOG ("\tBody:\n");
        while (fgets(String, READMAIL_BODY_LINELEN, BodyFile))
        {
            PRINTLOG("\t\t%s\n", String);
        }
        fclose(BodyFile);
        unlink(BodyFileName);

        /* Attachments */

        for (Att = 0; MailGetMessageAttachmentInfo(hMessage, Att,
                                &bhAttachment, OriginalFileName,
                                NULL, NULL, NULL, NULL, NULL); Att++)
        {
            PRINTLOG("\tAttachment %d = '%s'\n", Att+1, OriginalFileName);
        }

        MailCloseMessage (hMessage);
    }   /* end of loop over messages */

    /* Free the message list and close the message file */

Exit2:
    if (hMessageList != NULLHANDLE)
    {
        OSUnlockObject(hMessageList);
        OSMemFree(hMessageList);
    }

Exit1:
    if (hMessageFile != NULLHANDLE)
        MailCloseMessageFile(hMessageFile);
Exit0:
    if (ERR(error))
    {
        PRINTERROR(error, "MailOpenMessageFile");
    }
    else
    {
        PRINTLOG("\nProgram completed successfully.\n");
    }
    NotesTerm();
    return(ERR(error));
}

/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:     argc, argv - directly from the command line

    OUTPUTS:    server_name - server name obtained from command line or
                              from prompt.
                db_name -     database name obtained from command line or
                              from prompt.

*************************************************************************/

void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                               char *server_name, char *db_name)
{
    if (argc != 3)
    {

        printf("Enter server name: ");
        fflush (stdout);
        fgets(server_name, SERVER_NAME_LEN, STDIN);
        printf("\n");
        printf ("Enter database filename:  ");
        fflush (stdout);
        fgets(db_name, MAXPATH-1, STDIN);
    }
    else
    {
        memset(server_name, '\0', SERVER_NAME_LEN);    
        strncpy(server_name, argv[1], SERVER_NAME_LEN-1);
        memset(db_name, '\0', MAXPATH);    
        strncpy(db_name, argv[2], MAXPATH-1);
    } /* end if */
} /* ProcessArgs */


/************************************************************************

    FUNCTION:   GetUniqueFileName

    INPUTS:
          Drive - Drive letter string pointer.
          Ext - Extension name string pointer.

    OUTPUTS:
       FileName - Unqiue file name.

*************************************************************************/


#if defined(UNIX) || defined(MAC)  /* UNIX platforms */

STATUS near pascal GetUniqueFileName(char *Drive, char *Ext,
                                     char *FileName)

{
    int     file;
    WORD    Num;
    char    Name[17];
    char   *Dir;

    Dir="/tmp/";

    for (Num = 0; Num <= 32767; Num++)
    {
        sprintf(Name, "%i", Num); /*_itoa(Num, Name, 10);*/
        sprintf(FileName, "%s%s.%s", Dir,Name,Ext); /* _makepath(FileName, Drive, Dir, Name, Ext);*/
        if ((file = open(FileName, O_RDONLY, 0666)) == -1)
            return(NOERROR);
        close(file);
    }
    FileName[0] = '\0';
    return(ERR_EXISTS);
}

#else  /* Wintel */

STATUS near pascal GetUniqueFileName(char *Drive, char *Ext,
                                     char *FileName)

{
    int     file;
    WORD    Num;
    char    Name[17];
    char    cwd[MAXPATH];
    char   *Dir;

    /* Increment through numbered file names until a non-existent one found. */
    getcwd(cwd, MAXPATH);
    Dir = (char *)&cwd;

    for (Num = 0; Num <= 32767; Num++)
    {
        _itoa(Num, Name, 10);
        _makepath(FileName, Drive, Dir, Name, Ext);
        if ((file = open(FileName, O_BINARY | O_RDONLY)) == -1)
            return(NOERROR);
        close(file);
    }
    FileName[0] = '\0';
    return(ERR_EXISTS);
}

#endif
