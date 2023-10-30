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

    PROGRAM:    sendmemo

    FILE:       sendmemo.c

    SYNTAX:     sendmemo  "SendTo"  "Subject"  "BodyField"

    PURPOSE:    Send a Domino and Notes memo using only NSF functions.

    DESCRIPTION:
      Command-line HCL C API for Notes/Domino program demonstrating
      simple mail send using only HCL C API for Notes/Domino NSF
      functions (no Mail* or VIM*). Sends a memo to the recipient specified
      in the SendTo field of the command line argument. Does not look up
      the name in the Name & Address book. Sets the SendTo and the Recipients
      fields of the memo field to the SendTo value specified on the command
      line. Also sets the Subject and Body according to the command line.
      Sets the Composed Date to the current date and time, and sets the Form
      field to "Memo". Sets the Delivery Priority to Normal, the Delivery
      Report to basic, and the Return Receipt to No. Puts the memo in the
      "mail.box" file on the mail server specified in the NOTES.INI file.
      
      In the event of multiple "mail.box" files, if no mail server is
      specified (local access), and the NOTES.INI parameter
      "MAIL_ENABLE_MAILBOX_COMPATIBILITY =1" is set, then the memo is put in
      the local "mail.box" file.  If no mail server is specified, and the
      NOTES.INI parameter "MAIL_ENABLE_MAILBOX_COMPATIBILITY" is not set, the
      sample will attempt to deposit the memo in the "mail2.box" file.
      
      For more information on multiple "mail.box" files, please consult your
      Domino documentation.
      
****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif


#include <stdio.h>
#include <string.h>
#if defined(UNIX)
#include <unistd.h>
#else
#include <stdlib.h>
#include <malloc.h>
#include <io.h>
#endif

#include <global.h>
#include <globerr.h>
#include <oserr.h>

#include <names.h>
#include <stdnames.h>                   /* FIELD_FORM, etc */
#include <osfile.h>
#include <nsf.h>
#include <kfm.h>                        /* SECKFMUserInfo */
#include <nsfdb.h>
#include <nsfnote.h>
#include <nsfsearc.h>
#include <nsferr.h>
#include <ostime.h>
#include <osmem.h>
#include <ods.h>
#include <osenv.h>                      /* OSGetEnvironmentString */
#include <fontid.h>
#include <mail.h>
#include <osmisc.h>
#include"../../printLog.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

#define TEMP_MAIL_BODY_ITEM   "TempBody"
 

/************************************************************************

    FUNCTION:   main

*************************************************************************/
int main(int argc, char * argv[])
{
    STATUS      error = NOERROR;
    char       *szSendTo;
    char       *szSubject;
    char       *szBody;
    char        szFrom[MAXUSERNAME+1];
    char        szMailServerName[MAXUSERNAME+1];
    char        szMailFileName[MAXUSERNAME+1] = MAILBOX_NAME;
    char        szMailBoxPath[MAXPATH+1];
    DBHANDLE    hMailBox;
    NOTEHANDLE  hMemo;
    TIMEDATE    tdCurrent;
    BLOCKID     bidItem;
    WORD        wDataType;
    BLOCKID     bidValue;
    DWORD       dwValueLen;
    DHANDLE     hRichTextBody;
    DWORD       dwRichTextLen;
    BYTE      * pRichTextBody;

    if (argc != 4)
    {
        printf ("Error: incorrect syntax.\n");
        printf ("%s  \"to\" \"subject\" \"body\"\n", argv[0]);
        return (NOERROR);
    }
    szSendTo = argv[1];
    szSubject = argv[2];
    szBody = argv[3];

    if (error = NotesInitExtended (argc, argv))
    {
        printf("\n Unable to initialize Notes. Error Code[0x%04x]\n", error);
        return (1);
    }
    
    if (error = SECKFMUserInfo(KFM_ui_GetUserInfo, szFrom, NULL))
    {
        printf ("Error: unable to get user name.\n");
        goto Done;
    }

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
                goto Done;
            }
        }
    }
    while (error);

    OSCurrentTIMEDATE(&tdCurrent);

    if (error = NSFNoteCreate(hMailBox, &hMemo))
    {
        printf ("Error: unable to create memo in %s.\n", szMailBoxPath);
        goto Done1;
    }

    if (error = NSFItemSetText( hMemo,
                                FIELD_FORM,     /* "Form" */
                                MAIL_MEMO_FORM, /* "Memo" = Standard memo */
                                MAXWORD))
    {
        printf ("Error: unable to set item '%s' into memo.\n", FIELD_FORM);
        goto Done2;
    }

    if (error = NSFItemSetText( hMemo,
                                MAIL_SENDTO_ITEM,  /* "SendTo" */
                                szSendTo,
                                MAXWORD))
    {
        printf ("Error: unable to set item '%s' into memo.\n",
                                                        MAIL_SENDTO_ITEM);
        goto Done2;
    }

    if (error = NSFItemSetText( hMemo, /* use NSFItemCreateTextList if > 1*/
                                MAIL_RECIPIENTS_ITEM,   /* "Recipients" */
                                szSendTo,
                                MAXWORD))
    {
        printf ("Error: unable to set item '%s' into memo.\n",
                 MAIL_RECIPIENTS_ITEM);
        goto Done2;
    }

    if (error = NSFItemSetText( hMemo,
                                MAIL_FROM_ITEM,     /* "From" */
                                szFrom,
                                MAXWORD))
    {
        printf ("Error: unable to set item '%s' into memo.\n", MAIL_FROM_ITEM);
        goto Done2;
    }

    if (error = NSFItemSetText( hMemo,
                                MAIL_SUBJECT_ITEM,     /* "Subject" */
                                szSubject,
                                MAXWORD))
    {
        printf ("Error: unable to set item '%s' into memo.\n",
                                MAIL_SUBJECT_ITEM);
        goto Done2;
    }

    if (error = NSFItemSetText( hMemo,
                                MAIL_DELIVERYPRIORITY_ITEM, /* "DeliveryPriority" */
                                "Normal",
                                MAXWORD))
    {
        printf ("Error: unable to set item '%s' into memo.\n",
                                MAIL_DELIVERYPRIORITY_ITEM);
        goto Done2;
    }

    if (error = NSFItemSetText( hMemo,
                                MAIL_DELIVERYREPORT_ITEM, /* "DeliveryReport" */
                                "Basic",
                                MAXWORD))
    {
        printf ("Error: unable to set item '%s' into memo.\n",
                 MAIL_DELIVERYREPORT_ITEM);
        goto Done2;
    }

    if (error = NSFItemSetText( hMemo,
                                MAIL_RETURNRECEIPT_ITEM,  /* "ReturnReceipt" */
                                "No",
                                MAXWORD))
    {
        printf ("Error: unable to set item '%s' into memo.\n",
                                MAIL_RETURNRECEIPT_ITEM);
        goto Done2;
    }

    if (error = NSFItemSetTime( hMemo,
                                MAIL_COMPOSEDDATE_ITEM, /* "ComposedDate" */
                                &tdCurrent))
    {
        printf ("Error: unable to set item '%s' into memo.\n",
                                MAIL_COMPOSEDDATE_ITEM);
        goto Done2;
    }

    if (error = NSFItemSetTime( hMemo,
                                MAIL_POSTEDDATE_ITEM, /* "PostedDate" */
                                &tdCurrent))
    {
        printf ("Error: unable to set item '%s' into memo.\n",
                                MAIL_POSTEDDATE_ITEM);
        goto Done2;
    }

    if (error = NSFItemSetText( hMemo,
                                TEMP_MAIL_BODY_ITEM,  /* "TempBody" */
                                szBody,
                                MAXWORD))
    {
        printf ("Error: unable to set item '%s' into memo.\n",
                                TEMP_MAIL_BODY_ITEM);
        goto Done2;
    }

    if (error = NSFItemInfo(hMemo,
                            TEMP_MAIL_BODY_ITEM,
                            (WORD)strlen(TEMP_MAIL_BODY_ITEM),
                            &bidItem, &wDataType, &bidValue, &dwValueLen))
    {
        printf ("Error: unable to get info re item '%s'.\n",
                            TEMP_MAIL_BODY_ITEM);
        goto Done2;
    }

    if (error = ConvertItemToCompositeExt(bidValue, dwValueLen,
                                          DEFAULT_FONT_ID, "", PARADELIM_BLANKLINE,
                                          &hRichTextBody, &dwRichTextLen,
                                          NULL, 0, FALSE))
    {
        printf ("Error: unable to convert text item to composite.\n");
        goto Done2;
    }

    pRichTextBody = (BYTE *)OSLockObject(hRichTextBody) ;
    pRichTextBody += sizeof(WORD);
    dwRichTextLen -= sizeof(WORD);

    if (error = NSFItemAppend(hMemo, 0, MAIL_BODY_ITEM,
                              (WORD) strlen(MAIL_BODY_ITEM), TYPE_COMPOSITE, pRichTextBody,
                              dwRichTextLen))
    {
        printf ("Error: unable to append item '%s' to note.\n",
                            MAIL_BODY_ITEM);
        OSUnlockObject(hRichTextBody);
        OSMemFree(hRichTextBody);
        goto Done2;
    }

    OSUnlockObject(hRichTextBody) ;
    OSMemFree(hRichTextBody);

    if (error = NSFItemDelete(hMemo, TEMP_MAIL_BODY_ITEM,
                              (WORD) strlen(TEMP_MAIL_BODY_ITEM)))
    {
        printf ("Error: unable to delete item '%s' to note.\n",
                            TEMP_MAIL_BODY_ITEM);
        goto Done2;
    }

    if (error = NSFNoteUpdate(hMemo, 0))
    {
        printf ("Error: unable to update note in %s.\n", szMailBoxPath);
        goto Done2;
    }

Done2:
    NSFNoteClose(hMemo);

Done1:
    NSFDbClose(hMailBox);

Done:
    if (error)
    {
        PRINTERROR(error, "NSFDbOpen");
    }
    else
    {
        printf ("%s: successfully deposited memo '%s' in '%s'.\n",
                argv[0], szSubject, szMailBoxPath);
        fflush(stdout);
    }

    NotesTerm();
    return (error); 
}
