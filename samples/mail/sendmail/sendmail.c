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

    PROGRAM:    sendmail
   
    FILE:       sendmail.c

    PURPOSE:    Send a Domino and Notes Mail Message using the Notes Mail 
                Gateway API functions.

    FUNCTIONS:

        MAINPROC - initializes applicaton.  Calls ProcessArgs and 
                   SendMailMainProc. 
        SendMailMainProc() - initializes "From" and "Date" fields and calls
                   SendMail. 
        SendMail() - Creates and delivers mail message.

    DESCRIPTION:

    SENDMAIL creates a Domino and Notes mail message and transfers the message
    to the router. SENDMAIL uses the Domino and Notes Mail Gateway API functions.

    SendMail() creates the message as a note in the user's mail file. 
    It transfers message to the router via mail.box. If successfull, 
    it also saves the message in the user's mail file.

    The routine SendMail() handles the case where the system running this
    program is a remote system not connected to the LAN. When attempting
    to open the user's Domino and Notes mail file, SendMail first tries to open 
    the mail file on the mail server. If unable to open the file because the
    LAN is not available, SendMail asks the user if they would like to save 
    the message in the local MAIL.BOX. If the user selects YES, then SendMail 
    opens the mail file on the local machine instead of on the mail server. 
    Later, when transferring the message to the router, if we are saving 
    locally, SendMail calls MailTransferMessageLocal to copy the message 
    to the MAIL.BOX in the local system's Domino and Notes data directory.

    Otherwise, SendMail() opens the mail box on mail server and updates
    the message to the MAIL.BOX there.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif


#if defined(UNIX) || defined(MAC)
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#else
#include <stdlib.h>
#include <malloc.h>
#include <io.h>
#include <stdio.h>
#include <direct.h>
#include <fcntl.h>
#endif

#include <string.h>                 /* strtok() */
#include <ctype.h>                  /* isspace() */
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>

/* HCL C API for Notes/Domino header files */

#include <global.h>
#include <stdnames.h>                   /* FIELD_FORM, etc */
#include <osfile.h>                     /* OSPathNetConstruct */
#include <osmisc.h>                     /* OSLoadString */
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

#include <osmisc.h>

#ifndef UNIX
#include <windows.h>
#endif

#include "sendmail.h"        /* specific to this program */

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

#define MAIL_BODY_LEN 1024
#define MAIL_SUB_LEN 255
#define STDOUT       stdout
#define STDIN        stdin

/* Global variables */
int argc;
char *argv;
char          szErrStr[ERR_STR_LEN+1];
/* 'status' stores the value returned from HCL C API for Notes/Domino functions. */
STATUS        status;
char          szSendTo[MAXUSERNAME+1];
wchar_t       wszSendTo[MAXUSERNAME+1];
char          szMailFile[MAXUSERNAME+1];
wchar_t       wszMailFile[MAXUSERNAME+1];
char          szCopyTo[MAXUSERNAME+1];
char          szFrom[MAXUSERNAME+1];
char          szMailServerName[MAXUSERNAME+1];
TIMEDATE      tdDate;
char          szDate[MAXALPHATIMEDATE+1];
char          szSubject[MAIL_SUB_LEN+1];
char          szBody[MAIL_BODY_LEN+1];
int           BodyLen;


/****************************************************************************

    FUNCTION:   main

    PURPOSE:    Main HCL C API for Notes/Domino subroutine

****************************************************************************/

int main(int argc, char *argv[])
{
    status = NotesInitExtended (argc, argv);
    if (status) 
    {
        PRINTLOG("\nUnable to initialize Notes. Error Code[0x%04x]\n", status);
        return(1);
    }

    /* get program arguments */
    ProcessArgs(argc, argv, szMailFile, szSendTo, szCopyTo, szSubject, szBody); 
    
    status = SendMailMainProc();

    if (status != NOERROR) 
    {
        PRINTERROR(status,"SendMailMainProc");
        NotesTerm();
        return(status);
    }

    status = PrintMail(szMailServerName);
    if (status) 
        PRINTERROR(status,"PrintMail");

    NotesTerm();
    return(status);
}

/****************************************************************************

    FUNCTION: SendMailMainProc()

    COMMENTS:
        Initialize "From" field with the user's name, and the "Date"
        field with the current date and time.  Call Sendmail routine to
        process and send mail message.

****************************************************************************/

int SendMailMainProc()
{
    WORD          wDateLen;

    /* Get user's name and initialize "From" field. */
    if (status = SECKFMGetUserName(szFrom))
    {  /* Unable to get user name. */
        PRINTLOG ("\nError: unable to get user name.\n");
        return(status);
    }

    /* Get current time and date. Initialize "Date" field */
    OSCurrentTIMEDATE(&tdDate);
    if (status = ConvertTIMEDATEToText (NULL, NULL,
                                        &tdDate, szDate, MAXALPHATIMEDATE,
                                        (WORD FAR*)&wDateLen))
    {   /* Unable to initialize date field */
        PRINTLOG ("\nError: unable to initialize Date field.\n");
        return(status);
    }
  
    szDate[wDateLen] = '\0';

    status = SendMail();

    return(status);

}

/****************************************************************************

    FUNCTION: SendMail()

    PURPOSE:  Creates message with strings szSendTo, etc., obtained from
              program arguments. Transfers message to the router via mail.box.
              Also saves a copy of the message in the user's mail file.

    ALGORITHM:
        1.  Get name of the user's mail server and mail file from the
            NOTES.INI variables "MailServer" and "MailFile".
        2.  Construct full network path to the mail file on mail server.
        3.  Open the mail file on the mail server. If unable to open
            because network not up or not available, then ask user if
            they would like to save locally. If so, set fLocal to TRUE
            and try to open to the local mail file.
        4.  Create a message
        5.  Allocate lists for SendTo, CopyTo, and Recipients items.
        6.  Parse SendTo string entered by user into the dialog box.
            Add each name to the SendTo list and also Recipients.
            This recognizes commas and semicolons as delimiters.
        7.  Parse CopyTo string entered by user into the dialog box.
            Add each name to the CopyTo list and also Recipients.
        8.  Add Recipients, SendTo, and CopyTo items to the message.
        9.  Add Form item to the message: "Memo".
        10. Add From item to the message.
        11. Add Subject item to the message.
        12. Add Delivery Priority item to the message: "N" = Normal.
        13. Add Delivery Report item to the message: "B" = Basic.
        14. Add Composed Date item to the message: use dialog box Date.
        15. Create a Body item. Append text from the global string szBody
            to the body item. Add body item to the message.
        16. Get the current time/date right now and add it to the
            message as the PostedDate item.
        17. Transfer the message to the router.
            a.  If we are saving locally because this is a remote laptop
                not connected to the LAN, try to transfer the message to
                the local MAIL.BOX by calling MailTransferMessageLocal.
                If successful transfering message to local MAIL.BOX,
                update the message to the user's local mail file.
            b   If we are connected to a LAN and saving to files on the
                mail server, construct a full net path to the router's
                MAIL.BOX file on the mail server. Try to open MAIL.BOX.
                If successful opening MAIL.BOX on mail server, then
                copy the message to the mail box on the server. Then
                update the message to the user's mail file on the server.
        18. Clean up: Unlock and free memory. Close messages and
            databases.
        19. Return.

****************************************************************************/

int SendMail()
{
  char        szMailFileName[MAXUSERNAME+1];
  char        szMailFilePath[MAXPATH+1];
  char        szMailBoxPath[MAXPATH+1];
  char        szLocalMailBox[10];
  STATUS      error = NOERROR;    
  DBHANDLE    hMailFile = NULLHANDLE;
  DBHANDLE    hMailBox = NULLHANDLE;
  BOOL        fLocal = FALSE;
  NOTEHANDLE  hMsg;
  DHANDLE       hRecipientsList = NULLHANDLE;
  DHANDLE       hSendToList = NULLHANDLE;
  DHANDLE       hCopyToList = NULLHANDLE;
  WORD        wRecipientsSize = 0, wSendToSize = 0, wCopyToSize = 0;
  WORD        wRecipientsCount = 0, wSendToCount = 0, wCopyToCount = 0;
  LIST        *plistRecipients, *plistSendTo, *plistCopyTo;
  char        *szNextName;        /* used when parsing szSendTo, etc. */
  char        szPriority[] = "N"; /* "Normal" */
  char        szReport[] = "B";   /* "Basic" */
  DHANDLE       hBodyItem = NULLHANDLE;
  DWORD       dwBodyItemLen;
  DBHANDLE    hOrigDB;
  NOTEID      OrigNoteID;
  OID         OrigNoteOID, NewNoteOID;
    
  BodyLen = strlen(szBody);

  /* prepare the message strings */
  ConvertNewlinesToNulls(szBody, BodyLen);

  if (!OSGetEnvironmentString(MAIL_MAILSERVER_ITEM, /*"MailServer"*/
               szMailServerName, MAXUSERNAME))
    {   /* Unable to get mail server name */
        PRINTLOG ("\nError: unable to get mail server name.\n");
        return(status);
    }
  
    memset(szMailFileName, '\0', sizeof(szMailFileName));
    strncpy(szMailFileName, szMailFile, sizeof(szMailFileName)-1);

  /*  First try to open the user's mail file on the mail server.
      If unable to open because LAN not available, then ask user if
      they would like to save locally.
  */
    OSPathNetConstruct( NULL, szMailServerName, szMailFileName,
                        szMailFilePath);
    status = MailOpenMessageFile(szMailFilePath, &hMailFile);
  
#if 0
    if ( (ERR(status) == ERR_NO_NETBIOS) ||
        (ERR(status) == ERR_SERVER_NOT_RESPONDING) )
#endif
    if ( (ERR(status) == ERR_SERVER_NOT_RESPONDING) )
    /* laptop not connected to server */
    {
        status = NOERROR;

        /* Ask user if they would like to deliver msg to local MAIL.BOX */
        PRINTLOG("Would you like to deliver the mail message to your local Mail Box (YES/NO)?: ");      
        gets(szLocalMailBox);
       
        /* Unable to reach Mail Server. Save in local MAIL.BOX? */
        if (strcmp(szLocalMailBox, "YES") == 0)
        {
            /* yes - create and save message in local mail file */
            fLocal = TRUE;
            status = MailOpenMessageFile(szMailFileName, &hMailFile);
        }
        else
        { /* no - user wants to connect to server. Error condition.*/
            status = ERR_SERVER_NOT_RESPONDING;
        }
    }
  
    if (status)
    { /* Unable to open user's Domino and Notes mail file */
        PRINTLOG ("\nError: unable to open user's mail file.\n");
        return(status);
    }

    if (status = MailCreateMessage(hMailFile, &hMsg))
    { /* Unable to create memo in mail file */
        PRINTLOG ("\nError: unable to create memo in mail file.\n");
        error=SENDMAILERROR;
        goto CloseFile;
    }

    if (status = ListAllocate(0, 0, TRUE, &hRecipientsList,
                              &plistRecipients, &wRecipientsSize))
    {  /* Unable to allocate list */
        PRINTLOG ("\nError: unable to allocate list.\n");
        error=SENDMAILERROR;
        goto CloseMsg;
    }
    OSUnlockObject(hRecipientsList);
  
    if (status = ListAllocate(0, 0, TRUE, &hSendToList,
                              &plistSendTo, &wSendToSize))
    {
        PRINTLOG ("\nError: unable to allocate list.\n");
        error=SENDMAILERROR;
        goto CloseMsg;
    }
    OSUnlockObject(hSendToList);
  
    if (status = ListAllocate(0, 0, TRUE, &hCopyToList,
                              &plistCopyTo, &wCopyToSize))
    {
        PRINTLOG ("\nError: unable to allocate list.\n");
        error=SENDMAILERROR;
        goto CloseMsg;
    }
  OSUnlockObject(hCopyToList);

    /* Parse SendTo string. Add names to SendTo and Recipients lists. */
    PRINTLOG("\nSend To mailIDs:%s", szSendTo);
    for (szNextName = strtok(szSendTo, ",");
         szNextName != (char*)NULL;
         szNextName = strtok(NULL, ","))
    {
        unsigned char* uc = (unsigned char*)szNextName;
        while (isspace(*uc))    /* Skip white space before name */
        szNextName++;
      
        if (status = ListAddEntry(hSendToList, TRUE, &wSendToSize,
                                  wSendToCount++, szNextName,
                                  (WORD)strlen(szNextName)))
        {   /* Unable to add name to SendTo list */
            PRINTLOG ("\nError: unable to add name to SendTo list.\n");
            error=SENDMAILERROR;
            goto CloseMsg;
        }

        if (status = ListAddEntry(hRecipientsList, TRUE, &wRecipientsSize,
                                  wRecipientsCount++, szNextName,
                                 (WORD)strlen(szNextName)))
        {   /* Unable to add name to Recipients list */
            PRINTLOG ("\nError: unable to add name to Recipients list.\n");
            error=SENDMAILERROR;
            goto CloseMsg;
        }
    }
  
    /* Parse CopyTo string. Add names to CopyTo and Recipients list. */
  
    PRINTLOG("\nCopy To mailIDs:%s", szCopyTo);
    for (szNextName = strtok(szCopyTo, ",");
        szNextName != (char*)NULL;
        szNextName = strtok(NULL, ","))
    {
        while (isspace(*szNextName))
        szNextName++;
        
        if (status = ListAddEntry(hCopyToList, TRUE, &wCopyToSize,
                                  wCopyToCount++, szNextName,
                                  (WORD)strlen(szNextName)))
        { /* Unable to add name to CopyTo list */
            PRINTLOG ("\nError: unable to add name to CopyTo list.\n");
            error=SENDMAILERROR;
            goto CloseMsg;
        }

        if (status = ListAddEntry(hRecipientsList, TRUE, &wRecipientsSize,
                                  wRecipientsCount++, szNextName,
                                  (WORD)strlen(szNextName)))
        
        {
            PRINTLOG ("\nError: unable to add name to Recipients list.\n");
            error=SENDMAILERROR;
            goto CloseMsg;
        }
    }
  
  /*  Suggested enhancements: You might want to add code here to verify
      that the name & address book on the mail server contains person or
      group documents for each of the named recipients. See HCL C API 
      for Domino and Notes function NAMELookup(). Possibly query user to 
      resolve unknown recipient names. You might also want to check the 
      recipients list to ensure it contains no duplicate names.
  */

  /* Add the Recipients item to the message. */
    if (wRecipientsCount == 0)  /* Mail memo has no recipients. */
    {
        PRINTLOG ("\nError: mail message has no recipients.\n");
        error=SENDMAILERROR;
        goto CloseMsg;
    }
  
    if (status = MailAddRecipientsItem( hMsg, hRecipientsList,
                                        wRecipientsSize))
    {
        /* Unable to set Recipient item in memo */
        PRINTLOG ("\nError: unable to set Recipient item in memo.\n");
        error=SENDMAILERROR;
        goto CloseMsg;
    }
  /*  MailAddRecipientsItem and MailAddHeaderItemByHandle both attach
      the memory used by the list to the message. Set handle to NULL
      after these functions succeed so the code at CloseMsg: below does
      not attempt to free it.
  */
    hRecipientsList = NULLHANDLE;
  
  /* Add the SendTo and CopyTo items to the message. */
  
    if (status = MailAddHeaderItemByHandle( hMsg, MAIL_SENDTO_ITEM_NUM,
                                            hSendToList, wSendToSize, 0))
    {  /* Unable to set SendTo item in memo */
        PRINTLOG ("\nError: unable to set SendTo item in memo.\n");
        error=SENDMAILERROR;
        goto CloseMsg;
    }
    hSendToList = NULLHANDLE;
  
    if (status = MailAddHeaderItemByHandle( hMsg, MAIL_COPYTO_ITEM_NUM,
                                            hCopyToList, wCopyToSize, 0))
    { /* Unable to set CopyTo item in memo */
        PRINTLOG ("\nError: unable to set CopyTo item in memo.\n");
        goto CloseMsg;
    }
    hCopyToList = NULLHANDLE;
  
  /* Add the Form item to the message */
    if (status = MailAddHeaderItem( hMsg, MAIL_FORM_ITEM_NUM,
                                    MAIL_MEMO_FORM,
                                    (WORD)strlen(MAIL_MEMO_FORM)))
    { /* Unable to set Form item in memo  */
        PRINTLOG ("\nError: unable to set Form item in memo.\n");
        error=SENDMAILERROR;
        goto CloseMsg;
    }

  /* Add From, Subject, Delivery Priority, & etc. items to the message */
    if (status = MailAddHeaderItem( hMsg, MAIL_FROM_ITEM_NUM,
             szFrom, (WORD)strlen(szFrom)))
    { /* Unable to set From item in memo */
        PRINTLOG ("\nError: unable to set From item in memo.\n");
        error=SENDMAILERROR;
        goto CloseMsg;
    }
    PRINTLOG("\nSubject:%s", szSubject);
    if (status = MailAddHeaderItem( hMsg, MAIL_SUBJECT_ITEM_NUM,
                                    szSubject, (WORD)strlen(szSubject)))
    { /* Unable to set Subject item in memo */
        PRINTLOG ("\nError: unable to set Subject item in memo.\n");
        error=SENDMAILERROR;
        goto CloseMsg;
    }
  
    PRINTLOG("\nPriority:%s", szPriority);
    if (status = MailAddHeaderItem( hMsg, MAIL_DELIVERYPRIORITY_ITEM_NUM,
                                    szPriority, (WORD)strlen(szPriority)))
    { /* Unable to set Delivery Priority item in memo */
        PRINTLOG ("\nError: unable to set Delivery Priority item in memo.\n");
        error=SENDMAILERROR;
        goto CloseMsg;
    }

    if (status = MailAddHeaderItem( hMsg, MAIL_DELIVERYREPORT_ITEM_NUM,
                                    szReport, (WORD)strlen(szReport)))
    { /* Unable to set Delivery Report into memo */
        PRINTLOG ("\nError: unable to set Delivery Report item in memo.\n");
        error=SENDMAILERROR;
        goto CloseMsg;
    }

  /*  set "ComposedDate" to tdDate = when dialog box was initialized */
    if (status = MailAddHeaderItem( hMsg, MAIL_COMPOSEDDATE_ITEM_NUM,
                                    (char*)(&tdDate),
                                    (WORD)sizeof(TIMEDATE)))
    {   /* Unable to set Composed Date in memo */
        PRINTLOG ("\nError: unable to set Composed Date item in memo.\n");
        error=SENDMAILERROR;
        goto CloseMsg;
    }
  
    if (status = MailCreateBodyItem (&hBodyItem, &dwBodyItemLen))
    { /* Unable to create body item in message */
        PRINTLOG ("\nError: unable to create body item in memo.\n");
        hBodyItem = NULLHANDLE;
        error=SENDMAILERROR;
        goto CloseMsg;
    }
    PRINTLOG("\nBody:%s", szBody);
    if (status = MailAppendBodyItemLine (hBodyItem, &dwBodyItemLen,
                                         szBody, (WORD)BodyLen))
    { /* Unable to append text to body */
        PRINTLOG ("\nError: unable to append text to body.\n");
        error=SENDMAILERROR;
        goto CloseMsg;
    }

    if (status = MailAddBodyItemExt(hMsg, hBodyItem, dwBodyItemLen, NULL))
    { /* Unable to add Body item to memo */
        PRINTLOG ("\nError: unable to add body item to memo.\n");
        error=SENDMAILERROR;
        goto CloseMsg;
    }

    /* Set "PostedDate" to the current time/date right now */
    OSCurrentTIMEDATE(&tdDate);
    if (status = MailAddHeaderItem( hMsg, MAIL_POSTEDDATE_ITEM_NUM,
                                    (char*)(&tdDate),
                                    (WORD)sizeof(TIMEDATE)))
    {
        PRINTLOG ("\nError: unable to set date.\n");
        error=SENDMAILERROR;
        goto CloseMsg;
    }
 
    /*  Deliver the message. */
    /*  If local, transfer message to the local mail.box */
    if (fLocal)
    {
        if (status = MailTransferMessageLocal(hMsg))
        {
            /* Unable to transfer message to local mail box */
            PRINTLOG ("\nError: unable to transfer message to local mail box.\n");
            error=SENDMAILERROR;
            goto CloseMsg;
        }
        /* else we successfully transferred msg to local mail box */
        /* Save msg to user's mail file and clean up.*/
        if (status = NSFNoteUpdate(hMsg, UPDATE_NOCOMMIT))
        {   /* Unable to update message in local mail file */
            PRINTLOG ("\nError: unable to update message in local mail box.\n");
            error=SENDMAILERROR;
        }
        goto CloseMsg;
    }
  
    /*  System is connected to the LAN (fLocal == FALSE). Open the
      router's MAIL.BOX on the mail server, then copy msg there.
    */
    OSPathNetConstruct(NULL, szMailServerName, MAILBOX_NAME, szMailBoxPath);
    
    if (status = NSFDbOpen(szMailBoxPath, &hMailBox))
    {
        PRINTLOG ("\nError: unable to open mail box.\n");
        error=SENDMAILERROR;
        goto CloseMsg;
    }
  /*  Copy the message, which is a note in the user's mail file,
      to the mail box. Perform this copy by changing the origin to
      the mail box then updating. Save the message's DBID, NOTEID,
      and OID. Set the DBID to the MAIL.BOX handle, the NOTEID to zero,
      and the OID to a newly generated OID associated with MAIL.BOX.
      Then update the message. This stores it in the MAIL.BOX file.
      Finally, restore the DBID, NOTEID, and OID.
   */
    NSFNoteGetInfo(hMsg, _NOTE_ID,  &OrigNoteID);
    NSFNoteGetInfo(hMsg, _NOTE_DB,  &hOrigDB);
    NSFNoteGetInfo(hMsg, _NOTE_OID, &OrigNoteOID);
  
    /* Set the message's OID database ID to match the mail box */
    if (status = NSFDbGenerateOID (hMailBox, &NewNoteOID))
    {
        /* Unable to generate originator ID for mail box */
        PRINTLOG ("\nError: unable to generate id for mail box.\n");
        error=SENDMAILERROR;
        goto CloseMsg;
    }
    NSFNoteSetInfo(hMsg, _NOTE_DB,  &hMailBox);
    NSFNoteSetInfo(hMsg, _NOTE_ID,  0);
    NSFNoteSetInfo(hMsg, _NOTE_OID, &NewNoteOID);
  
    /* Update message into MAIL.BOX on mail server. */
    if (status = NSFNoteUpdate(hMsg, UPDATE_NOCOMMIT))
    {
        /* Unable to update message to router mail box */
        PRINTLOG ("\nError: unable to update message to router mail box.\n");
        error=SENDMAILERROR;
    }
    else/* message successfully copied into router's mail box */
    {   /* restore msg to user's mail file and Update to save it there.*/
        NSFNoteSetInfo(hMsg, _NOTE_DB,  &hOrigDB);
        NSFNoteSetInfo(hMsg, _NOTE_ID,  &OrigNoteID);
        NSFNoteSetInfo(hMsg, _NOTE_OID, &OrigNoteOID);
        
        status = NSFNoteUpdate(hMsg, UPDATE_NOCOMMIT);
    }
  

CloseMsg:
    if (hRecipientsList != NULLHANDLE)  OSMemFree(hRecipientsList);
    if (hSendToList != NULLHANDLE)      OSMemFree(hSendToList);
    if (hCopyToList != NULLHANDLE)      OSMemFree(hCopyToList);
  
    if (status) 
        PRINTERROR(status,"NSFDbGenerateOID ");

    if (error == NOERROR)
    {
        if (status = MailCloseMessage(hMsg))
        { /* Unable to close memo */
            PRINTLOG ("\nError: unable to close memo.\n");
            error=SENDMAILERROR;
        }
        else
            if (hBodyItem != NULLHANDLE)  OSMemFree (hBodyItem);
    }
    else
    {
        status = MailCloseMessage(hMsg);
        if ((status == NOERROR) && (hBodyItem != NULLHANDLE))
            OSMemFree (hBodyItem);
    }
    if (hMailBox != NULLHANDLE)
        NSFDbClose (hMailBox);

CloseFile:
    if (error == NOERROR)
    {
        if (status = MailCloseMessageFile(hMailFile))
        { /* Unable to close mail file. */
            PRINTLOG ("\nError: unable to close mail file.\n");
            error=SENDMAILERROR;
        }
    }
    else /* have already encountered an error condition */
    {    /* so don't check for further errors closing */
        MailCloseMessageFile(hMailFile);
    }

    if (error == NOERROR)
    /* SendMail Success */
    PRINTLOG ("\nSendMail successful.\n");

    return(status);
}


/************************************************************************

    FUNCTION:   ConvertNewlinesToNulls(char* szBody, int BodyLen)

    PURPOSE:    Filter CR from body text. Replace with NUL char.

*************************************************************************/
 
void  LNPUBLIC  ConvertNewlinesToNulls(char* szBody, int BodyLen)
{
    int    wByte;

    for (wByte=0; wByte<BodyLen; wByte++)
    {
        if (szBody[wByte] == '\n' ) 
            szBody[wByte] = '\0'; 
    }
  
    return;
}


/****************************************************************************

    FUNCTION: ProcessArgs (int argc, char *argv[], 
                             char *szSendTo, char *szCopyTo, char *szSubject,
                             char *szBody)

    PURPOSE:  Get command line arguments from user.

    INPUTS:   szSendTo  - Send message to User Name.
              szCopyTo  - Copy message to User Name.
              szSubject - Mail Message Subject.
              szBody    - Body of Mail Message.

****************************************************************************/


void  LNPUBLIC  ProcessArgs (int argc, char *argv[], 
                             char *szMailFile, char *szSendTo, 
                             char *szCopyTo, char *szSubject,
                             char *szBody)
{ 
    memset(szMailFile, '\0', MAXUSERNAME+1);
    memset(szSendTo, '\0', MAXUSERNAME+1);
    memset(szCopyTo, '\0', MAXUSERNAME+1);
    memset(szSubject, '\0', MAIL_SUB_LEN+1);
    memset(szBody, '\0', MAIL_BODY_LEN+1);

    if (argc != 6)  
    {

        printf("Enter SendTo: ");      
        fflush (stdout);
#ifdef LINUX
        setlocale(LC_ALL, "xx_XX.UTF-8");
#else
        SetConsoleCP(852);
#endif
        gets(szSendTo);

/*input u+00e4u+00f6/test
char szFinSendTo[] = {0x84, 0x94, '/', 't', 'e', 's', 't', '\0'};
szSendTo and szFinSendTo should be same*/

        printf("\n");
        printf ("Enter CopyTo:  ");
        fflush (stdout);
        fgets(szCopyTo, MAXUSERNAME, STDIN);
        printf ("Enter Subject:  ");
        fflush (stdout);
        fgets(szSubject, MAIL_SUB_LEN, STDIN);
        printf ("Enter Body of message:  ");
        fflush (stdout);
        fgets(szBody, MAIL_BODY_LEN, STDIN);
        printf ("Enter Mail File Name:  ");
        fflush (stdout);
        fgets(szMailFile, MAXUSERNAME, STDIN);
    }    
    else
    {
        strncpy(szMailFile, argv[1], MAXUSERNAME);    
        strncpy(szSendTo, argv[2], MAXUSERNAME);    
        strncpy(szCopyTo, argv[3], MAXUSERNAME);    
        strncpy(szSubject, argv[4], MAIL_SUB_LEN);    
        strncpy(szBody, argv[5], MAIL_BODY_LEN);    
    } /* end if */
} /* ProcessArgs */


/************************************************************************
    Validate mail sent and Print 
*************************************************************************/

STATUS PrintMail(char *szServerName)
{

    DHANDLE     hMessageFile;
    DHANDLE     hMessageList = NULLHANDLE, hMessage;
    DARRAY     *MessageList;
    WORD        MessageCount, Msg, RecipientCount, Rec, MsgNum = 0;
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
    char        szMailFilePath[MAXPATH+1];
    FILE       *BodyFile;
    char        OriginalFileName[MAXPATH+1];
    WORD        Att;
    BLOCKID     bhAttachment;
    char        szServer[80];           /* server names are < 80 chars */
    char        szDBName[MAXPATH];
    STATUS      error;

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
        if (error = MailGetMessageInfo(MessageList, Msg,
                                     &RecipientCount, NULL, NULL))
        {
            PRINTLOG("Error: unable to get number of recipients in message.\n");
            MailCloseMessage(hMessage);
            goto Exit2;
        }
   
        if (RecipientCount > 0)           /*To print mails which are sent*/
        {
            PRINTLOG("\nMessage #%d: \n", ++MsgNum);
        
            if (error = MailOpenMessage(MessageList, Msg, &hMessage))
            {
                PRINTLOG("Error: unable to open message number %d.\n", Msg + 1);
                goto Exit2;
            }
        
           /* Get the originator's name/address. */
            if (error = MailGetMessageOriginator(MessageList, Msg,
                  Originator, sizeof(Originator) - 1, &OriginatorLength))
            {
                PRINTLOG("Error: unable to get message originator.\n");
                goto Exit2;
            }
        
            Originator[OriginatorLength] = '\0';
        
            PRINTLOG("\tOriginator = '%s'\n", Originator);
        
            PRINTLOG("\tNumber of Recipients = %d.\n", RecipientCount);
       
            for (Rec = 0; Rec < RecipientCount; Rec++)
            {
                MailGetMessageRecipient(MessageList, Msg, Rec, RecipientName,
                sizeof(RecipientName) - 1, &RecipientNameLength);
                MailParseMailAddress(RecipientName, RecipientNameLength,
                                     UserName, sizeof(UserName) - 1, &UserNameLength,
                                     DomainName, sizeof(DomainName) - 1, &DomainNameLength);
                UserName[UserNameLength] = '\0';
                DomainName[DomainNameLength] = '\0';

                PRINTLOG("\t\tRecipient %d = '%s'\t Domain = '%s'\n", Rec + 1,
                UserName, DomainName);
            }  /* end of loop over recipients */
        
            /* SendTo */
            MailGetMessageItem(hMessage, MAIL_SENDTO_ITEM_NUM, String,
            MAXSPRINTF, &StringLength);
        
            String[StringLength] = '\0';
            PRINTLOG("\tTo: %s\n", String);
 
            /* CopyTo */
            MailGetMessageItem(hMessage, MAIL_COPYTO_ITEM_NUM, String,
            MAXSPRINTF, &StringLength);
            String[StringLength] = '\0';
            PRINTLOG("\tCc: %s\n", String);
        
            /* From */
            MailGetMessageItem(hMessage, MAIL_FROM_ITEM_NUM, String,
            MAXSPRINTF, &StringLength);
            String[StringLength] = '\0';
            PRINTLOG("\tFrom: %s\n", String);

            /* PostedDate */
            MailGetMessageItemTimeDate(hMessage, MAIL_POSTEDDATE_ITEM_NUM, &Time);
            ConvertTIMEDATEToText(NULL, NULL, &Time, String,
                                  sizeof(String) - 1, &StringLength);
            String[StringLength] = '\0';
            PRINTLOG("\tDate: %s\n", String);
        
            /* Subject. If non-delivery report, prefix with "NonDelivery of:" */
        
            MailGetMessageItem(hMessage, MAIL_SUBJECT_ITEM_NUM, String,
                               MAXSPRINTF, &StringLength);
            String[StringLength] = '\0';
            if (NonDeliveryReport = MailIsNonDeliveryReport(hMessage))
            {
                PRINTLOG("\tNonDelivery of: %s\n", String);
            }
            else
            {
                PRINTLOG("\tSubject: %s\n", String);
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
                PRINTLOG("Error: unable to create temporary file name.\n");
                MailCloseMessage(hMessage);
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
                PRINTLOG("Error: unable to get Message body into temporary file.\n");
#ifdef LINUX
                unlink(BodyFileName);
#else
                _unlink(BodyFileName);
#endif
                MailCloseMessage(hMessage);
                goto Exit2;
            }
        
             /* Print each line of body text to the screen. */

#if defined(OS400)
            if (!(BodyFile = fopen(BodyFileName, "rb")))
#else
            if (!(BodyFile = fopen(BodyFileName, "r")))
#endif
            {
                PRINTLOG("Error: unable to open temporary file.\n");
                unlink(BodyFileName);
                MailCloseMessage(hMessage);
                goto Exit2;
            }
            
            PRINTLOG("\tBody:\n");
            while (fgets(String, READMAIL_BODY_LINELEN, BodyFile))
            {
                PRINTLOG("\t\t%s\n", String);
            }
            fclose(BodyFile); 
            unlink(BodyFileName);
            MailCloseMessage(hMessage);
        }
    }     /* end of loop over messages */
    
    /* Free the message list and close the message file */
     
Exit2:
    if (hMessageList != NULLHANDLE)
    {
        MailFreeMessageList(hMessageList,MessageList);
        /*OSUnlockObject(hMessageList);
        OSMemFree(hMessageList);*/
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
     
    return(NOERROR);
}


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

    Dir = (char *)"/tmp/";

    for (Num = 0; Num <= 32767; Num++)
    {
        sprintf(Name, "%i", Num); /*_itoa(Num, Name, 10);*/
        sprintf(FileName, "%s%s.%s", Dir,Name,Ext); /* _makepath(FileName, Drive, Dir, Name, Ext);*/
        if ((file = open(FileName, O_RDONLY, 0777)) == -1)
            return(NOERROR);
        close(file);
    }
    FileName[0] = '\0';
    return(NOERROR);
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
    return(NOERROR);
}

#endif
