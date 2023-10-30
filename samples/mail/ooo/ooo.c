/*************************************************************************
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

    PROGRAM:   ooo

    FILE:      ooo.c

    PURPOSE:   This program will sets and gets user Out Of Office information.

    SYNTAX:    ooo


*************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* OS and C include files */
#include <stdio.h>
#include<time.h>

/* Notes API include files */
#if defined (OS390)
#include <lapicinc.h>
#endif


#if defined (LINUX)
#include <string.h>
#endif


#include <global.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <ostime.h>
#include <osmisc.h>
#include <osenv.h>

#include <osfile.h>
#include <mail.h>
#include <mailserv.h> 
#include "../../printLog.h"

#include <oooapi.h>

/* Local function prototypes */
STATUS getCurrDate(char* stDate, char *enDate);

#define MAIL_MAILSERVER_ITEM "MailServer"
#define SECOND               1
#define MINUTE               60 * SECOND
#define HOUR                 60 * MINUTE
#define DAY                  24 * HOUR

#define MESSAGELEN           500

/* Notes API subroutine */
int main (int argc, char *argv[])
{
        WORD            TimeStringLen;
        STATUS          error = NOERROR;   /* return code from API calls */
        OOOCTXHANDLE    hOOOContext = NULLHANDLE;
        OOOCTXPTR       *pOOOContext = NULL;
        TIMEDATE        tdStartAway,tdEndAway;
        char            timetext[MAXALPHATIMEDATE+1];   /* time/date in ASCII */
        char            far *text_pointer;              /* pointer to timetext */
        DHANDLE         hMailFile = NULLHANDLE;

        char            *szMailFileName;
        char            *szOwnerName;
        char            szServerName[MAXUSERNAME+1];
        char            szMailFilePath[MAXPATH+1];
        char            szStartAway[MAXALPHATIMEDATE+1],szEndAway[MAXALPHATIMEDATE+1];
        char            pGeneralMessage[MESSAGELEN];
        char            pGeneralSubject[MESSAGELEN];
        WORD            retVersion,retState,pGeneralMessageLen;
        BOOL            bExcludeInternet;

        char            altline[MAXPATH] = "I am out of office";
        char            currrentoooStatus[MAXPATH] = "";

        if (argc != 3)
        {
          PRINTLOG( "\nUsage:  %s <user's mail database filename> <user name> \n", argv[0] );
          return (0);
        }


        /* init ... */
        szOwnerName=argv[2];
        szMailFileName=argv[1];
        
        memset(szStartAway, '\0', sizeof(szStartAway));
        memset(szEndAway, '\0', sizeof(szEndAway));
        getCurrDate(szStartAway, szEndAway);
        PRINTLOG("Start date[ %s ]- End date[ %s ]\n", szStartAway, szEndAway);
        fflush(stdout);

        strcpy(pGeneralMessage,szOwnerName);
        strcat(pGeneralMessage," will out of office");
        strcpy(pGeneralSubject,szOwnerName);
        strcat(pGeneralSubject," is out of office");

        if ( error = NotesInitExtended (argc, argv) )
        {
            PRINTLOG("\n Unable to initialize Notes. Error Code[0x%04x]\n", error);
            fflush(stdout);
            return (1);
        }

        if (!OSGetEnvironmentString(MAIL_MAILSERVER_ITEM, szServerName, MAXUSERNAME))
        {
           PRINTLOG ("\nUnable to get mail server name ...\n\n ");
           fflush(stdout);
           strcpy(szServerName,"");
           NotesTerm();
           return(1);
        }


        OSPathNetConstruct( NULL, szServerName, szMailFileName,szMailFilePath);

        if (error = MailOpenMessageFile(szMailFilePath, &hMailFile))
        {
            PRINTERROR (error,"MailOpenMessageFile");
            NotesTerm();
            return (1);
        }

/* at add-in initialization */
        if (error = OOOInit())
        {
            PRINTERROR (error,"OOOInit");
            goto EXIT2;
        }

/* Enable OOO for one specific user */

        if ( error = OOOStartOperation(szOwnerName,
                                       szServerName,
                                       TRUE,
                                       hMailFile,
                                       &hOOOContext,
                                       &pOOOContext))
        {
            PRINTERROR (error,"OOOStartOperation");
            goto EXIT1;

        }

        if ( error = OOOGetState(pOOOContext,&retVersion,&retState))
        {
            PRINTERROR (error,"OOOGetState");
            goto EXIT1;
        }

        switch( retVersion )
        {
                case OOO_CONFIG_AGENT:
                        PRINTLOG("The version (agent, service) of the out of office functionality is agent\n");
                        break;
                case OOO_CONFIG_SERVICE:
                        PRINTLOG("The version (agent, service) of the out of office functionality is service\n");
                        break;
        }

        switch ( retState )
        {
                case OOO_STATE_DISABLED:
                        PRINTLOG("The state (disabled, enabled) of the out of office functionality is disabled\n");
                        break;
                case OOO_STATE_ENABLED:
                        PRINTLOG("The state (disabled, enabled) of the out of office functionality is enabled\n");
                        break;

        }

        if (OOOSetExcludeInternet(pOOOContext,TRUE))
        {
           PRINTERROR (error,"OOOSetExcludeInternet");
           goto EXIT1;

        }


        if (OOOGetExcludeInternet(pOOOContext,&bExcludeInternet))
        {
           PRINTERROR (error,"OOOGetExcludeInternet");
           goto EXIT1;

        }

        if (bExcludeInternet)
        {
           PRINTLOG("Gets ExcludeInternet is TRUE\n");
        }
        else 
        {
          PRINTLOG("Gets ExcludeInternet is FALSE\n");
        }

        strcpy (timetext, szStartAway);

/* Get a pointer to the time/date string. We need this for the next call. */

        text_pointer = szStartAway;

/* Put the time/date text into Domino and Notes binary form. */

        if (error = ConvertTextToTIMEDATE(NULL,NULL,
                                          &text_pointer,(WORD) strlen(szStartAway),&tdStartAway))
        {
            PRINTERROR (error,"ConvertTextToTIMEDATE");
            goto EXIT1;

        }

        strcpy (timetext, szEndAway);
/* Get a pointer to the time/date string. We need this for the next call. */

        text_pointer = timetext;

/* Put the time/date text into Domino and Notes binary form. */

        if (error = ConvertTextToTIMEDATE(NULL,
                                          NULL,
                                          &text_pointer,
                                          (WORD) strlen(timetext),
                                          &tdEndAway))
        {
            PRINTERROR (error,"ConvertTextToTIMEDATE");
            goto EXIT1;

        }

        if (error = OOOSetAwayPeriod(pOOOContext, tdStartAway, tdEndAway))
        {
            PRINTERROR (error,"OOOSetAwayPeriod");
            goto EXIT1;

        }

        if (error = OOOSetAlternateAwayLine(pOOOContext, altline))
        {
            PRINTERROR(error, "OOOSetAlternateAwayLine");
            goto EXIT1;
        }

        if (error = OOOEnable(pOOOContext, TRUE))
        {
            PRINTERROR (error,"OOOEnable");
            goto EXIT1;

        }

        if (OOOSetGeneralSubject(pOOOContext,pGeneralSubject,TRUE))
        {
            PRINTERROR (error,"OOOSetGeneralSubject");
            goto EXIT1;

        }

        if (OOOGetGeneralSubject(pOOOContext,pGeneralSubject))
        {
            PRINTERROR (error,"OOOGetGeneralSubject");
            goto EXIT1;

        }

        PRINTLOG("Gets OOO General Subject:\n");
        PRINTLOG("%s\n",pGeneralSubject);
        fflush(stdout);

        if (OOOSetGeneralMessage(pOOOContext,pGeneralMessage,sizeof(pGeneralMessage)))
        {
            PRINTERROR (error,"OOOSetGeneralMessage");
            goto EXIT1;
        }


        if (OOOGetGeneralMessage(pOOOContext,pGeneralMessage,&pGeneralMessageLen))
        {
            PRINTERROR (error,"OOOGetGeneralMessage");
            goto EXIT1;
        }

        PRINTLOG("Gets OOO General Message:\n");
        PRINTLOG("%s ",pGeneralMessage);
        fflush(stdout);

        if (error = OOOGetAwayPeriod(pOOOContext,&tdStartAway,&tdEndAway))
        {
            PRINTERROR (error,"OOOGetAwayPeriod");
            goto EXIT1;
        }

        if (error = ConvertTIMEDATEToText(NULL, NULL,&tdStartAway,
                                          timetext, MAXALPHATIMEDATE,
                                          &TimeStringLen))
        {
            PRINTERROR (error,"ConvertTIMEDATEToText");
            goto EXIT1;
        }

        timetext[TimeStringLen] = '\0';
        PRINTLOG("%s to ", timetext);


        if (error = ConvertTIMEDATEToText(NULL, NULL,&tdEndAway,
                                          timetext, MAXALPHATIMEDATE,
                                          &TimeStringLen))
        {
            PRINTERROR (error,"ConvertTIMEDATEToText");
            goto EXIT1;
        }

        timetext[TimeStringLen] = '\0';
        PRINTLOG("%s.", timetext);
        
        if (error = OOOGetAlternateAwayLine(pOOOContext, currrentoooStatus, sizeof(altline)))
        {
            PRINTERROR(error, "OOOGetAlternateAwayLine");
            goto EXIT1;
        }

        PRINTLOG("\n%s ", currrentoooStatus);

        if (hOOOContext && pOOOContext)
        {
            if (error = OOOEndOperation(hOOOContext, pOOOContext))
            {
                PRINTERROR (error,"OOOEndOperation");
                goto EXIT1;
            }
        }

/* at add-in termination */
EXIT1:
        if (error = OOOTerm())
        {
            PRINTERROR (error,"OOOTerm");
            NotesTerm();
            return (1);
        }
EXIT2:
        if (hMailFile != NULL) 
        {
           if (error = MailCloseMessageFile(hMailFile))
           {
                PRINTERROR (error,"MailCloseMessageFile");
                NotesTerm();
                return (1);
           }
        }

    /* End of subroutine. */
        PRINTLOG("\nProgram completed successfully.\n");
        fflush(stdout);
        NotesTerm();
        return (0);
}

/* This function is to get the start and end date for ooo */
/* To manipulate this set NotesDateFormat=1 for DMY 
 * NotesDateFormat=2 for YMD
 * for 3 MDY in notes.ini and restart notes client or server accordingly
 */

STATUS getCurrDate(char *stDate, char *enDate)
{
       time_t t = time(NULL);
       struct tm *time = localtime(&t);

#if defined (LINUX)
       sprintf(stDate, "%02d/%02d/%d", time->tm_mon + 1,  time->tm_mday , time->tm_year + 1900);
       t = t + (1 * DAY);
       time = localtime(&t);
       sprintf(enDate, "%02d/%02d/%d", time->tm_mon + 1,  time->tm_mday , time->tm_year + 1900);
#else
       sprintf(stDate, "%02d/%02d/%d",  time->tm_mday ,time->tm_mon + 1, time->tm_year + 1900);
       t = t + (1 * DAY);
       time = localtime(&t);
       sprintf(enDate, "%02d/%02d/%d", time->tm_mday, time->tm_mon + 1, time->tm_year + 1900);
#endif
       return NOERROR;
}
