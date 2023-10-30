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

    PROGRAM:      timedatepair

    FILE:         timedateapir.c

    SYNTAX:       timedatepair

    PURPOSE:      Demonstrates ConvertTIMEDATEPAIRToText & ConvertTextToTIMEDATEPAIR usage

    DESCRIPTION:  This is a sample code that demonstrates two APIs, how to use those.
                  ConvertTIMEDATEPAIRToText - Converts TIMEDATEPAIR to text format
                  ConvertTextToTIMEDATEPAIR - Converts Text to TIMEDATEPAIR
                  No command line args needed as code generates current TIME & DATE
                  for processing.

 ***************************************************************************/

/*
 * Windows header file. Include only if compiling for Windows.
 */

#if(defined(NT))
#include <windows.h>
#endif

/* C header files     */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* HCL C API for Notes/Domino Header files.  */

#include <global.h>
#include <stdnames.h>
#include <miscerr.h>
#include <misc.h>
#include <osmem.h>
#include <oserr.h>
#include <time.h>
#include <osmisc.h>
#include "../../printLog.h"

#define STRING_LENGTH 256

STATUS LNPUBLIC TimeConversion();

/************************************************************************

    FUNCTION:    Main

*************************************************************************/

int main(int argc, char *argv[])
{
    STATUS  sError = NOERROR;

    /*
     *  Initialize Domino and Notes.
     */

    if (sError = NotesInitExtended (argc, argv))
    {
	PRINTLOG("\n Unable to initialize Notes.\n");
	return (1);
    }

    sError=TimeConversion();

    if (sError == NOERROR)
    {
        PRINTLOG("\n\nProgram completed successfully.\n");
    }
    else
    {
        PRINTERROR (sError,"TimeConversion");
    }

    NotesTerm();
    return (0);

}

/************************************************************************

    FUNCTION:     TimeConversion

    PURPOSE:      This routine will demonstrate how to use the following 
                  APIs, ConvertTextToTIMEDATEPAIR & ConvertTIMEDATEToText

*************************************************************************/

STATUS LNPUBLIC TimeConversion()
{
    STATUS        sError = NOERROR;
    TIMEDATE_PAIR theApptInterval;
    WORD          wLen;
          
    char          lTimeRange[STRING_LENGTH];
    char          startTime[STRING_LENGTH];
    char          endTime[STRING_LENGTH];
    char          lstartTime[STRING_LENGTH];
    char          lendTime[STRING_LENGTH];
    struct        tm *date = NULL;
    time_t        timer;
    char          *plTimeRange=(char*)&lTimeRange;

    /* Get current date and time */
    timer=time(NULL);
    date = gmtime( &timer ) ;
    strftime(startTime, sizeof(startTime), "%m/%d/%Y %I:%M:00 %p", date);

    /* Get +1 day of current date and time */
    date->tm_year = date->tm_year;
    date->tm_mon = date->tm_mon;
    date->tm_mday = date->tm_mday + 1;
    timer = mktime( date ) ;
    date = gmtime( &timer ) ;

    strftime(endTime, sizeof(endTime), "%m/%d/%Y %I:%M:00 %p", date);

    memset(lTimeRange, '\0', sizeof(lTimeRange));
    snprintf(lTimeRange, sizeof(lTimeRange)-1, "%s-%s", startTime, endTime);
    
    /* Convert Text Range to Internal TIMEDATE pair */
    sError = ConvertTextToTIMEDATEPAIR(NULL,
                                       NULL,
                                       &plTimeRange,
                                       (WORD)strlen(lTimeRange),
                                       &theApptInterval);
    if (sError)
    {
      goto Done;
    }

    memset(lstartTime, '\0', sizeof(lstartTime));
    if (sError = ConvertTIMEDATEToText(NULL, NULL,&theApptInterval.Lower,
                            lstartTime, sizeof(lstartTime),
                            &wLen))
    {
       goto Done;
    }

    memset(lendTime, '\0', sizeof(lendTime));
    if (sError = ConvertTIMEDATEToText(NULL, NULL,&theApptInterval.Upper,
                            lendTime, sizeof(lendTime),
                            &wLen))
    {
       goto Done;
    }
    PRINTLOG("\nAfter Text to TIMEDATEPAIR Lower: %s - Upper: %s", lstartTime, lendTime);

    /* Instead of using above ConvertTIMEDATEToText function to convert TIMEDATE to text 
     * we can use ConvertTIMEDATEPAIRToText function to convert to get the time date range
     */
    memset(lTimeRange, '\0', sizeof(lTimeRange));
    /*  convert an internal date/time PAIR to a string */
    sError = ConvertTIMEDATEPAIRToText( NULL, NULL,
                                    &theApptInterval,
                                    lTimeRange,
                                    (WORD) sizeof(lTimeRange),
                                    &wLen );

    PRINTLOG("\nTIMEDATEPAIR to Text: %s", lTimeRange);
    fflush(stdout);


  Done:
    return (sError);
}
