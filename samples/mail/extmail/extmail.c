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

  PROGRAM:    extmail.dll

  FILE:       extmail.c

  Extension Manager HCL C API for Notes/Domino Example Program

  OVERVIEW:
  This sample program will register the Extension Manager hook EM_MAILSENDNOTE.
  When the program traps an EM_MAILSENDNOTE, if the notification type is EM_BEFORE
  (before the function is called), and the mail message subject text is "Extension Manager",
  the mail message body text is modified notifying the user that the Extension Manager has
  intercepted the mail.  If the notification type is EM_AFTER (after the function is called ),
  a new mail message is created with user mail information and sent back to the user.

*/

#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

/*
 *  Domino and Notes header files
 */
#include <global.h>
#include <osmem.h>
#include <osfile.h>
#include <osenv.h>
#include <nsfdata.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <editods.h>
#include <stdnames.h>
#include <mailserv.h>
#include <mail.h>
#include <textlist.h>

#include "extmail.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/*========================== GLOBAL VARIABLES =============================*/

#define ERR_SENDMAIL_SERVERNAME             (PKG_ADDIN+1)
#define ERR_SENDMAIL_MAILFILENAME           (PKG_ADDIN+2)
#define TEMP_MAIL_BODY_ITEM                 "TempBody"

EMHANDLER         gHandlerProc;
HEMREGISTRATION	  hHandler;
BOOL              gHooksRegistered=FALSE;
WORD              gRecursionID;
DBHANDLE          hMailBox = NULLHANDLE;
FILE              *gFStream = (__FILE *)0;
char              gTextBuffer[1024];
char              DataDir[MAXPATH+1];
char              FileName[MAXPATH+1];
char              szMailBoxPath[MAXPATH+1];
char              szString[MAXSPRINTF + 1];
char              UserName[MAXUSERNAME + 1];

/* Storage needed for synchronization under Windows NT */
CRITICAL_SECTION    gCriticalSection;

/************************* End of Global Variables *************************
****************************************************************************/


/*======================= LOCAL FUNCTION PROTOTYES ========================*/

STATUS LNPUBLIC DLL_EXPORT MainEntryPoint( void );

BOOL WINAPI DllMain( HINSTANCE hInstance, DWORD fdwReason,
                     LPVOID lpReserved );

STATUS LNPUBLIC DLL_EXPORT EMHandlerProc( EMRECORD FAR * pExRecord);
STATUS NSFSendMailReport(char *UserName);
void CleanUp( void );

/*
* LogLine: Attempts to write a line to the output.log
*
*   in: Line => The Line to log
*
*   out: If gFStream is not NULL an attempt will be made to
*        ship out the line. If that fails gFStream will
*        be dropped.
*/
void LogLine(char *Line)
{
    if (gFStream) {
        __WriteLine(Line, gFStream);
        if (__FileError(gFStream)) {
            __CloseFile(gFStream);
            gFStream = (__FILE *)0;
        }
    }
}

/*
* RegisterEntry: Attempts to Register one entry.
*
*   out: 0 if ok, returned error otherwise
*/
STATUS RegisterEntry()
{
    STATUS error = NOERROR;

    LogLine("-------------------------------------------------\n");

    sprintf(gTextBuffer, "Calling EMRegister with EM_MAILSENDNOTE.\n");
    LogLine( gTextBuffer );

    error = EMRegister(EM_MAILSENDNOTE,
                       EM_REG_BEFORE | EM_REG_AFTER,
                       (EMHANDLER)gHandlerProc,
                       gRecursionID,
                       &hHandler);

    if (error) {
        LogLine("EMRegister Failed\n");
        if (gFStream) __CloseFile(gFStream);
            gFStream = (__FILE *)0;
            return(error);
    }

    LogLine("-------------------------------------------------\n");

    return(error);
}

/*
* DeregisterEntry: Attempts to Deregister one entry.
*
*   out: 0 if ok, returned error otherwise
*/
STATUS DeregisterEntry()
{
    STATUS error = NOERROR;

    gFStream = __OpenFile( FileName, APPEND_PERMISSION );

    LogLine("-------------------------------------------------\n");

    sprintf(gTextBuffer, "Calling EMDeregister with EM_MAILSENDNOTE.\n");
    LogLine( gTextBuffer );

    error = EMDeregister(hHandler);

    if (error) {
        LogLine("EMDregister Failed\n");
        if (gFStream) __CloseFile(gFStream);
            gFStream = (__FILE *)0;
        return(error);
    }

    LogLine("-------------------------------------------------\n");
    __CloseFile(gFStream);

    return(error);
}

STATUS LNPUBLIC DLL_EXPORT MainEntryPoint( void )
{
    STATUS    error;
    WORD      x;

    error = NOERROR;
	
/*
*    When run on a server the dll is called multiple times. the flag
*    keeps the main code from being executed more than once.
*/
    if ( gHooksRegistered )
        return(NOERROR);
    gHooksRegistered = TRUE;

	
/* Open log __FILE for DLL. */

    x = OSGetDataDirectory(DataDir);
    strcpy(FileName, DataDir);

    strcat(FileName, LOG_FILE_NAME);

    gFStream = __OpenFile( FileName, WRITE_PERMISSION );

/*  Next get a recursion ID  for the run */

    LogLine("-------------------------------------------------\n");
    LogLine("Calling EMCreateRecursionID\n");

    error = EMCreateRecursionID( &gRecursionID );

    if (error)
      LogLine("EMCreateRecursionID Failed\n");

/* Loop through, testing each entry */

    else
    {
        LogLine("EMCreateRecursionID Returned Success\n");
        error = RegisterEntry();
    }

    if ( gFStream ) {
        __CloseFile(gFStream);
        gFStream = (__FILE *)0;
    }
    return( error );
}

/*=========================================================================*/

STATUS LNPUBLIC EMHandlerProc( EMRECORD FAR * pExRecord )
{

    static BOOL BeforeMailSent = FALSE;
    static BOOL AfterMailSent = FALSE;
    STATUS sError = NOERROR;

    /* open log file... */
    gFStream = __OpenFile( FileName, APPEND_PERMISSION );
     
    switch (pExRecord->EId)
    {
        /* Extension Manager hook EM_MAILSENDNOTE */
        case EM_MAILSENDNOTE:
        {
            DHANDLE hNote;
            void   *pViewDesc;
            WORD   Flags;
            BOOL   *pModified;
            VARARG_PTR ap;

            char AppendLine[1024];
            char *Value, *SubjectText;
            char TargetText[]="Extension Manager";

            DHANDLE far hBody;
            DWORD far dwBodyLen;
            WORD Len;

            BLOCKID bhValue;
            WORD ValueType,  SubjectTextLen;
            DWORD ValueLength;

            /* get the arguments */
            ap = pExRecord->Ap;
            hNote = VARARG_GET (ap, DHANDLE);
            pViewDesc = VARARG_GET (ap, VOID *);
            Flags = VARARG_GET (ap, WORD);
            pModified = VARARG_GET (ap, BOOL *);

            /* check error code */
            if (pExRecord->Status != NOERROR)
                break;

            LogLine("-------------------------------------------------\n");
            sprintf(gTextBuffer, "Calling EM_MAILSENDNOTE\n");
            LogLine( gTextBuffer );

            if (pExRecord->NotificationType == EM_BEFORE)
                sprintf(gTextBuffer, "EM_BEFORE...\n");
            else if (pExRecord->NotificationType == EM_AFTER)
                sprintf(gTextBuffer, "EM_AFTER...\n");
    
            LogLine( gTextBuffer );
            sprintf(gTextBuffer, "hNote:%X\n",hNote);
            LogLine( gTextBuffer );
            sprintf(gTextBuffer, "Flags:%X\n",Flags);
            LogLine( gTextBuffer );
            sprintf(gTextBuffer, "*pModified:%d\n",*pModified);
            LogLine( gTextBuffer );
            LogLine("-------------------------------------------------\n");
            sprintf(gTextBuffer, "Calling SECKFMGetUserName...\n");
            LogLine( gTextBuffer );

            /* get the logged in user's name */
            sError = SECKFMGetUserName(UserName);

            if (sError)
            {
                sprintf(gTextBuffer, "SECKFMGetUserName->sError:%X\n", sError);
                LogLine( gTextBuffer );
                goto Exit0;
            }

            sprintf(gTextBuffer, "SECKFMGetUserName->UserName:%s\n", UserName);
            LogLine( gTextBuffer );

            sprintf(gTextBuffer, "Calling MailGetMessageItemHandle...\n");
            LogLine( gTextBuffer );
    
            /* get the subject text of this message */
            sError = MailGetMessageItemHandle(hNote, MAIL_SUBJECT_ITEM_NUM, &bhValue, &ValueType,
                                              &ValueLength);
    
            if (sError)
            {
                sprintf(gTextBuffer, "MailGetMessageItemHandle->sError %X\n", sError);
                LogLine( gTextBuffer );
                goto Exit0;
            }
    
            Value = OSLockBlock(char, bhValue);
            SubjectText = Value + sizeof(WORD);
            SubjectTextLen = (WORD) ValueLength - sizeof(WORD);
            SubjectText[SubjectTextLen]='\0';
            sprintf(gTextBuffer, "MailGetMessageItemHandle->SubjectText:%s\n", SubjectText);
            LogLine( gTextBuffer );
    
        OSUnlockBlock(bhValue);

            /* if before function check the Subject text of the mail message.  If we have
             * a match for "Extension Manager", append line to mail message.
             */
            if (pExRecord->NotificationType == EM_BEFORE)
            {

                /* compare mail message subject with our target string */
                if (strcmp(SubjectText, TargetText)!=0)
                    goto Exit0;

                sprintf(gTextBuffer, "Calling MailCreateBodyItem...\n");
                LogLine( gTextBuffer );

                sError = MailCreateBodyItem(&hBody, &dwBodyLen);
    
                if (sError)
                {
                    sprintf(gTextBuffer, "MailCreateBodyItem->sError %X\n", sError);
                    LogLine( gTextBuffer );
                    goto Exit0;
                }

                sprintf(gTextBuffer, "Calling MailAppendBodyItemLine...\n");
                LogLine( gTextBuffer );
	
                sError = MailAppendBodyItemLine(hBody, &dwBodyLen, NULL, 0);
	
                if (sError)
                {
                    sprintf(gTextBuffer, "MailAppendBodyItemLine->sError %X\n", sError);
                    LogLine( gTextBuffer );
                    goto Exit0;
                }

                sError = MailAppendBodyItemLine(hBody, &dwBodyLen, NULL, 0);
	
                if (sError)
                {
                    sprintf(gTextBuffer, "MailAppendBodyItemLine->sError %X\n", sError);
                    LogLine( gTextBuffer );
                    goto Exit0;
                }

                strcpy(AppendLine, "*** This mail message has been intercepted by the EXTENSION MANAGER! ***");
                Len = strlen(AppendLine);

                sError = MailAppendBodyItemLine(hBody, &dwBodyLen, AppendLine, Len);
	
                if (sError)
                {
                    sprintf(gTextBuffer, "MailAppendBodyItemLine->sError %X\n", sError);
                    LogLine( gTextBuffer );
                    goto Exit0;
                }
	
                strcpy(AppendLine, "You will be receiving a message report shortly.");
                Len = strlen(AppendLine);

                sError = MailAppendBodyItemLine(hBody, &dwBodyLen, AppendLine, Len);
	
                if (sError)
                {
                    sprintf(gTextBuffer, "MailAppendBodyItemLine->sError %X\n", sError);
                    LogLine( gTextBuffer );
                    goto Exit0;
                }
   
                sprintf(gTextBuffer, "Calling MailAddBodyItemExt...\n");
                LogLine( gTextBuffer );
	
                sError = MailAddBodyItemExt(hNote, hBody, dwBodyLen, NULL);
	
                if (sError)
                {
                    sprintf(gTextBuffer, "MailAddBodyItem->sError %X\n", sError);
                    LogLine( gTextBuffer );
                    goto Exit0;
                }

                /* s et flag that an EM_BEFORE was processed */
                BeforeMailSent = TRUE;

            }

            /* if after function and we have processed before function,
             * call NSFSendMailReport routine.
            */ 
            if (pExRecord->NotificationType == EM_AFTER)
            {
                /* if we just processed an EM_BEFORE... */
                if (BeforeMailSent == TRUE)
                {
                    LogLine("-------------------------------------------------\n");
                    sprintf(gTextBuffer, "Calling NSFSendMailReport...\n");
                    LogLine( gTextBuffer );
                    __CloseFile(gFStream);

                    /* send a mail report back to the user */
                    sError = NSFSendMailReport(UserName);

                    gFStream = __OpenFile( FileName, APPEND_PERMISSION );
                    LogLine("-------------------------------------------------\n");
                    sprintf(gTextBuffer, "After NSFSendMailReport...\n");
                    LogLine( gTextBuffer );
    
                    BeforeMailSent= FALSE;
                }
            }

            break;
        } /* end of case */

    } /* end of switch */

Exit0:

    if (sError)
    {
        LogLine("-------------------------------------------------\n");
        if (sError != ERR_SENDMAIL_SERVERNAME && sError != ERR_SENDMAIL_MAILFILENAME)
            OSLoadString( NULLHANDLE, sError, szString, sizeof(szString)-1);
        else if (sError == ERR_SENDMAIL_SERVERNAME)
            strcpy(szString, "Mail Server Name not defined in notes.ini.");
        else if (sError == ERR_SENDMAIL_MAILFILENAME)
            strcpy(szString, "Mail File Name not defined in notes.ini.");

        sprintf(gTextBuffer, "Error Message->%s\n", szString);
        LogLine( gTextBuffer );
    }

    __CloseFile(gFStream);

    return( ERR_EM_CONTINUE );
}


/*===========================================================================

   void CleanUp(void)

===========================================================================*/
void CleanUp(void)
{
  gHooksRegistered = FALSE;
}


/* ======================================================================= */
/* =================== STARTUP FUNCTIONS ================================= */
/* ======================================================================= */

/*===========================================================================

  Startup and Shutdown Function DllMain for Windows NT
  ----------------------------------------------------

   BOOL WINAPI DllMain( HINSTANCE hInstance, DWORD fdwReason,
						   LPVOID lpReserved )

  OVERVIEW:
  ---------
   Standard windows NT DLL entrypoint, does initialzation required to get the
   FARPROC for the Extension Manager callback function.

===========================================================================*/
BOOL WINAPI DllMain( HINSTANCE hInstance, DWORD fdwReason, LPVOID lpReserved )
{
    STATUS error=NOERROR;

    switch(fdwReason)
    {
        case DLL_PROCESS_ATTACH:
            InitializeCriticalSection(&gCriticalSection);
            gHandlerProc = (EMHANDLER)MakeProcInstance((FARPROC)EMHandlerProc, hInstance);
            break;

        case DLL_PROCESS_DETACH:
            CleanUp();
            /* Free procedure instance */
            FreeProcInstance( gHandlerProc );
            error = DeregisterEntry();
            DeleteCriticalSection(&gCriticalSection);
            break;
    }

    return( TRUE );

  UNREFERENCED_PARAMETER(lpReserved);
}

/* ======================================================================= */
/* =================== SHUTDOWN FUNCTIONS ================================ */
/* ======================================================================= */
/*
   NOTE:
   -----
      SHUTDOWN PROCESSES ARE DONE IN THE DLLMAIN FUNCTION IN NT
*/

/*=========================================================================*/

STATUS NSFSendMailReport(char *UserName)
{

    STATUS      sError = NOERROR;
    DBHANDLE    hMailBox;
    NOTEHANDLE  hMemo;
    DHANDLE       hRichTextBody;
    TIMEDATE    tdCurrent;
    BLOCKID     bidValue;
    BLOCKID     bidItem;
    WORD        wDataType;
    DWORD       dwValueLen;
    DWORD       dwRichTextLen;
    BYTE        *pRichTextBody;
    BOOL        sResult;
    char        szSubject[] = "Extension Manager Message Report";
    char        szBody[1024];
    char        szFrom[MAXUSERNAME+1] = "Extension Manager";
    char        MailServerName[MAXUSERNAME + 1];
    char        MailFileName[MAXPATH + 1];
    char        szMailBoxPath[MAXPATH+1];

    /* start logging... */
    gFStream = __OpenFile( FileName, APPEND_PERMISSION );
    LogLine("-------------------------------------------------\n");

    /* get the Mail Server from notes.ini */
    sResult = OSGetEnvironmentString(MAIL_MAILSERVER_ITEM, MailServerName, MAXUSERNAME);

    if (!sResult)
    {
        sError = ERR_SENDMAIL_SERVERNAME;
        sprintf(gTextBuffer, "OSGetEnvironmentString->sError:%X\n", sError);
        LogLine( gTextBuffer );
        goto Exit;
    }

    sprintf(gTextBuffer, "OSGetEnvironmentString->sError:%X\n", sError);
    LogLine( gTextBuffer );

    /* get the Mail File from notes.ini */
    sResult = OSGetEnvironmentString(MAIL_MAILFILE_ITEM, MailFileName, MAXUSERNAME);

    if (!sResult)
    {
        sError = ERR_SENDMAIL_MAILFILENAME;
        sprintf(gTextBuffer, "OSGetEnvironmentString->sError:%X\n", sError);
        LogLine( gTextBuffer );
        goto Exit;
    }

    sprintf(gTextBuffer, "OSGetEnvironmentString->sError:%X\n", sError);
    LogLine( gTextBuffer );

    sError = OSPathNetConstruct(NULL, MailServerName, MAILBOX_NAME, szMailBoxPath);

    sprintf(gTextBuffer, "OSPathNetConstruct->sError:%X\n", sError);
    LogLine( gTextBuffer );

    if (sError)
        goto Exit;

  /* open mail.box */
    sError = NSFDbOpen(szMailBoxPath, &hMailBox);

    sprintf(gTextBuffer, "NSFDbOpen->sError:%X\n", sError);
    LogLine( gTextBuffer );

    if (sError)
    {
        NSFDbClose(hMailBox);
        goto Exit;
    }

  OSCurrentTIMEDATE(&tdCurrent);

    /* create the note */
    sError = NSFNoteCreate(hMailBox, &hMemo);

    sprintf(gTextBuffer, "NSFNoteCreate->sError:%X\n", sError);
    LogLine( gTextBuffer );

    if (sError)
    {
        NSFDbClose(hMailBox);
        goto Exit;
    }

  sError = NSFItemSetText(hMemo,
                          FIELD_FORM,     /* "Form" */
                          MAIL_MEMO_FORM, /* "Memo" = Standard memo */
                          MAXWORD);

    sprintf(gTextBuffer, "NSFItemSetText->sError:%X\n", sError);
    LogLine( gTextBuffer );

    if (sError)
    {
        NSFNoteClose(hMemo);
        NSFDbClose(hMailBox);
        goto Exit;
    }

    sError = NSFItemSetText(hMemo,
                            MAIL_SENDTO_ITEM,  /* "SendTo" */
                            UserName,
                            MAXWORD);

    sprintf(gTextBuffer, "NSFItemSetText->sError:%X\n", sError);
    LogLine( gTextBuffer );

    if (sError)
    {
        sprintf(gTextBuffer, "Error setting MAIL_SENDTO_ITEM.\n");
        LogLine( gTextBuffer );
        NSFNoteClose(hMemo);
        NSFDbClose(hMailBox);
        goto Exit;
    }

    sError = NSFItemSetText(hMemo, /* use NSFItemCreateTextList if > 1*/
                            MAIL_RECIPIENTS_ITEM,   /* "Recipients" */
                            UserName,
                            MAXWORD);

  sprintf(gTextBuffer, "NSFItemSetText->sError:%X\n", sError);
  LogLine( gTextBuffer );

  if (sError)
  {
    sprintf(gTextBuffer, "Error setting MAIL_RECIPIENTS_ITEM.\n");
    LogLine( gTextBuffer );
    NSFNoteClose(hMemo);
    NSFDbClose(hMailBox);
    goto Exit;
  }


  sError = NSFItemSetText(hMemo,
                          MAIL_FROM_ITEM,     /* "From" */
                          szFrom,
                          MAXWORD);

    sprintf(gTextBuffer, "NSFItemSetText->sError:%X\n", sError);
    LogLine( gTextBuffer );

    if (sError)
    {
        sprintf(gTextBuffer, "Error setting MAIL_FROM_ITEM.\n");
        LogLine( gTextBuffer );
        NSFNoteClose(hMemo);
        NSFDbClose(hMailBox);
        goto Exit;
    }


    sError = NSFItemSetText(hMemo,
                            MAIL_SUBJECT_ITEM,     /* "Subject" */
                            szSubject,
                            MAXWORD);

    sprintf(gTextBuffer, "NSFItemSetText->sError:%X\n", sError);
    LogLine( gTextBuffer );

    if (sError)
    {
        sprintf(gTextBuffer, "Error setting MAIL_SUBJECT_ITEM.\n");
        LogLine( gTextBuffer );
        NSFNoteClose(hMemo);
        NSFDbClose(hMailBox);
        goto Exit;
    }


    sError = NSFItemSetText(hMemo,
                            MAIL_DELIVERYPRIORITY_ITEM, /* "DeliveryPriority" */
                            "Normal",
                            MAXWORD);

    sprintf(gTextBuffer, "NSFItemSetText->sError:%X\n", sError);
    LogLine( gTextBuffer );

    if (sError)
    {
        sprintf(gTextBuffer, "Error setting MAIL_DELIVERYPRIORITY_ITEM.\n");
        LogLine( gTextBuffer );
        NSFNoteClose(hMemo);
        NSFDbClose(hMailBox);
        goto Exit;
    }

    sError = NSFItemSetText(hMemo,
                            MAIL_DELIVERYREPORT_ITEM, /* "DeliveryReport" */
                            "Basic",
                            MAXWORD);

    sprintf(gTextBuffer, "NSFItemSetText->sError:%X\n", sError);
    LogLine( gTextBuffer );

    if (sError)
    {
        sprintf(gTextBuffer, "Error setting MAIL_DELIVERYREPORT_ITEM.\n");
        LogLine( gTextBuffer );
        NSFNoteClose(hMemo);
        NSFDbClose(hMailBox);
        goto Exit;
    }

    sError = NSFItemSetText(hMemo,
                            MAIL_RETURNRECEIPT_ITEM,  /* "ReturnReceipt" */
                            "No",
                            MAXWORD);

    sprintf(gTextBuffer, "NSFItemSetText->sError:%X\n", sError);
    LogLine( gTextBuffer );

    if (sError)
    {
        sprintf(gTextBuffer, "Error setting MAIL_RETURNRECEIPT_ITEM.\n");
        LogLine( gTextBuffer );
        NSFNoteClose(hMemo);
        NSFDbClose(hMailBox);
        goto Exit;
    }

    sError = NSFItemSetTime(hMemo,
                            MAIL_COMPOSEDDATE_ITEM, /* "ComposedDate" */
                            &tdCurrent);

    sprintf(gTextBuffer, "NSFItemSetText->sError:%X\n", sError);
    LogLine( gTextBuffer );

    if (sError)
    {
        sprintf(gTextBuffer, "Error setting MAIL_COMPOSEDDATE_ITEM.\n");
        LogLine( gTextBuffer );
        NSFNoteClose(hMemo);
        NSFDbClose(hMailBox);
        goto Exit;
    }

    strcpy(szBody, "*** Extension Manager Message Report ***\nUser Name:\t\t");
    strcat(szBody, UserName);
    strcat(szBody, "\n");
    strcat(szBody, "User Mail File:\t\t");
    strcat(szBody, MailFileName);
    strcat(szBody, "\n");
    strcat(szBody, "User Mail Server:\t");
    strcat(szBody, MailServerName);
    strcat(szBody, "\n\n");
    strcat(szBody, "Attached is the Extension Manager Log.\n");

    /*sprintf(szBody, "*** Extension Manager Message Report ***\nUser Name:\t\t%s\nUser Mail File:\t\t%s\nUser Mail Server:\t%s\n\nAttached is the Extension Manager Log", UserName, MailFileName, MailServerName);
    */

    sError = NSFItemSetText(hMemo,
                            TEMP_MAIL_BODY_ITEM,  /* "TempBody" */
                            szBody,
                            MAXWORD);

    sprintf(gTextBuffer, "NSFItemSetText->sError:%X\n", sError);
    LogLine( gTextBuffer );

    if (sError)
    {
        sprintf(gTextBuffer, "Error setting TEMP_MAIL_BODY_ITEM.\n");
        LogLine( gTextBuffer );
        NSFNoteClose(hMemo);
        NSFDbClose(hMailBox);
        goto Exit;
    }

    sError = NSFItemInfo(hMemo,
                         TEMP_MAIL_BODY_ITEM,
                         (WORD)strlen(TEMP_MAIL_BODY_ITEM),
                         &bidItem, &wDataType, &bidValue, &dwValueLen);

    sprintf(gTextBuffer, "NSFItemInfo->sError:%X\n", sError);
    LogLine( gTextBuffer );

    if (sError)
    {
        NSFNoteClose(hMemo);
        NSFDbClose(hMailBox);
        goto Exit;
    }

    sError = ConvertItemToCompositeExt(bidValue, dwValueLen,
                                       DEFAULT_FONT_ID, "\r\n", PARADELIM_ANYLINE,
                                       &hRichTextBody, &dwRichTextLen,
                                       NULL, 0, FALSE);

    sprintf(gTextBuffer, "ConvertItemToCompositeExt->sError:%X\n", sError);
    LogLine( gTextBuffer );

    if (sError)
    {
        NSFNoteClose(hMemo);
        NSFDbClose(hMailBox);
        goto Exit;
    }

    pRichTextBody = OSLockObject(hRichTextBody) ;
    pRichTextBody += sizeof(WORD);
    dwRichTextLen -= sizeof(WORD);

    sError = NSFItemAppend(hMemo, 0, MAIL_BODY_ITEM,
                           (WORD)strlen(MAIL_BODY_ITEM), TYPE_COMPOSITE, pRichTextBody,
                           dwRichTextLen);

    sprintf(gTextBuffer, "NSFItemAppend->sError:%X\n", sError);
    LogLine( gTextBuffer );

    if (sError)
    {
        sprintf(gTextBuffer, "Error appending MAIL_BODY_ITEM.\n");
        LogLine( gTextBuffer );OSUnlockObject(hRichTextBody);
        OSMemFree(hRichTextBody);
        NSFNoteClose(hMemo);
        NSFDbClose(hMailBox);
        goto Exit;
    }

    OSUnlockObject(hRichTextBody) ;
    OSMemFree(hRichTextBody);

    sError = NSFItemDelete(hMemo, TEMP_MAIL_BODY_ITEM,
                           (WORD)strlen(TEMP_MAIL_BODY_ITEM));

    sprintf(gTextBuffer, "NSFItemDelete->sError:%X\n", sError);
    LogLine( gTextBuffer );

    if (sError)
    {
        NSFNoteClose(hMemo);
        NSFDbClose(hMailBox);
        goto Exit;
    }

    /* let's close the log file here so we get most of the info in the attachment */
    __CloseFile(gFStream);

    sError = NSFNoteAttachFile(hMemo,
                               ITEM_NAME_ATTACHMENT,
                               (WORD) strlen(ITEM_NAME_ATTACHMENT),
                               FileName,
                               "extmail.log",
                               HOST_LOCAL | COMPRESS_NONE);

    /* reopen for complete logging */
    gFStream = __OpenFile( FileName, APPEND_PERMISSION );

    sprintf(gTextBuffer, "NSFNoteAttachFile->sError:%X\n", sError);
    LogLine( gTextBuffer );

    if (sError)
    {
        NSFNoteClose(hMemo);
        NSFDbClose(hMailBox);
        goto Exit;
    }

    sError = NSFNoteUpdate(hMemo, 0);

    sprintf(gTextBuffer, "NSFNoteUpdate->sError:%X\n", sError);
    LogLine( gTextBuffer );

    if (sError)
    {
        NSFNoteClose(hMemo);
        NSFDbClose(hMailBox);
        goto Exit;
    }

    sError = NSFNoteClose(hMemo);

    sprintf(gTextBuffer, "NSFNoteClose->sError:%X\n", sError);
    LogLine( gTextBuffer );

    if (sError)
    {
        NSFDbClose(hMailBox);
        goto Exit;
    }

    sError = NSFDbClose(hMailBox);

    sprintf(gTextBuffer, "NSFDbClose->sError:%X\n", sError);
    LogLine( gTextBuffer );

Exit:

    __CloseFile(gFStream);
    return(sError);

}


