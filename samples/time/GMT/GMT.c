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

    PROGRAM:    Gmt

    FILE:       Gmt.c

    SYNTAX:     Gmt 

    PURPOSE:    Demonstrates how to use the time functions

    DESCRIPTION:
            With the given sample date and time extracted and compared with time
            derived from different date and time functions.
        
            parameters compared are  date,time, daylight saving time and timezone 

            APIs explored : TimeExtractTicks,TimeExtractDate,TimeGMToLocalZone.
                            TimeGMToLocal,ConvertTIMEDATEtoRFC3339Date
        
****************************************************************************/
/*    C Includes */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Notes Includes */
#include <global.h>
#include <osmisc.h>
#include <log.h>
#include <osfile.h>
#include <ostime.h>
#include <misc.h>
#include "../../printLog.h"


#define ZONE_MASK                0x0f000000L
#define DST_MASK                 0x80000000L
#define DATE_MASK                0x00ffffffL
#define ZONE_HOUR_MIN_MASK       0x7f000000
#define CLEANUP_NOTHING          0x0000
/*Number of bits to be shifted to get Most significant byte from word */
#define BIT_SHIFT_COUNT               24
/* Below macros are  re-used from misc    ime.c of Notes source code */
#define    ZONE_MINUTE_GRANULARITY    15
#define    ZONE_HOURS(x)              ((x)&0x0f)
#define    ZONE_MINUTES(x)            ((((x)&0x30)>>4)*ZONE_MINUTE_GRANULARITY)
#define    ZONE_EAST_OF_GMT(x)        (((x)&0x40)!=0)

#define    TEXTBUFSIZE                 24

STATUS after_cleanup(STATUS exit_status);

/* Global Variables */
WORD    cleanup = CLEANUP_NOTHING;

int main(int argc, char *argv[])
{

    STATUS    error;                /* Notes return code variable */
    DWORD     dwDate; 
    DWORD     dwTime;
    TIMEDATE  tdNowTime;
    TIMEDATE  tdReturnTime;
    TIME      tTime;
    int       iTimeZone, iDst, iGMT_hour, iGMT_min, iGMT_datetime;
     

    PRINTLOG("Test of TimeExtractDate\n");

    /* Start Notes */
    if(error = NotesInitExtended (argc, argv)){
        PRINTLOG("\n Unable to initialize Notes.\n");
        return (1);
    }

    /* Reading current Timezone and Dayy light saving time */
    PRINTLOG("Calling OSCurrentTimeZone to get the time zone\n");
    OSCurrentTimeZone(&iTimeZone, &iDst);
  
    /* Reading Current time into tdNowTime */
    PRINTLOG("Calling OSCurrentTIMEDATE to put something in tdNowTime\n");
    OSCurrentTIMEDATE(&tdNowTime);

    /* Extracting ticks from current time */
    PRINTLOG("Calling TimeExtractTicks to get the Time part\n");
    dwTime = TimeExtractTicks(&tdNowTime);

    /* Extracting date from current time including Timezone and DST */
    PRINTLOG("Calling TimeExtractDate to get the Date part\n");
    dwDate = TimeExtractDate(&tdNowTime);
  
    PRINTLOG("TimeExtractDate returned with %#8.8x\n", dwDate);

    PRINTLOG("Comparing OS DST to returned DST\n");

    /* Extract DST from Date and compare it to value returned by OSCurrentTimeZone */

    if(iDst != (int) ((dwDate & DST_MASK)>>31)){
        PRINTLOG("Daylight Savings Times do not match - FAILED\n");
        return(after_cleanup(-1));
    }
    PRINTLOG("Daylight Savings Times match - VERIFIED\n");  

    /* Mask off the time zone and Daylight Savings Time (Probably not needed)*/

    dwDate = dwDate & DATE_MASK;

    PRINTLOG("Calling TimeConstruct with Date = %#x Time = %#x\n",
            (ULONG)dwDate, (ULONG)dwTime);
    TimeConstruct(dwDate, dwTime, &tdReturnTime);

    PRINTLOG("Comparing original TIMEDATE to returned TIMEDATE\n");
    if(!TimeDateEqual(&tdNowTime, &tdReturnTime)){
        PRINTLOG( "TIMEDATEs do not match - FAILED\n");
        return(after_cleanup(-1));
    }
    PRINTLOG("TIMEDATEs match - VERIFIED\n");

    /* converting current timezone into GMT timeZone */
     
    memcpy(&(tTime.GM), &tdNowTime, sizeof(TIMEDATE));
    tTime.zone = -3005; /* GMT+5:30 */
    tTime.dst = 0;
    TimeGMToLocalZone(&tTime);
    PRINTLOG("hour : %d minute : %d second : %d \n ", tTime.hour, tTime.minute, tTime.second);
    tTime.zone = 0; 
    tTime.dst = 0;
    TimeGMToLocalZone(&tTime);
    PRINTLOG("hour : %d minute : %d second : %d \n ", tTime.hour, tTime.minute, tTime.second);
    TimeGMToLocal(&tTime);
    PRINTLOG("hour : %d minute : %d second : %d \n", tTime.hour, tTime.minute, tTime.second);
    
    /* converting current timedate to RFC339 timedate */

    PRINTLOG("Test of ConvertTIMEDATEtoRFC3339Date\n");

    /* print Notes TIMEDATE*/

    char retTextBuffer[MAXALPHATIMEDATE + 1] = {0};
    WORD dwTextLength = 0;

    if(error = ConvertTIMEDATEToText(NULL,NULL,&tdNowTime,retTextBuffer,MAXALPHATIMEDATE+1,&dwTextLength)){
        PRINTLOG("Error in ConvertTIMEDATEToText\n");
        return(after_cleanup(-1));
    }
    PRINTLOG("Notes TIMEDATE = %s\n",retTextBuffer);

    char textBuffer[MAXSPRINTF] = {0}; /*String buffer to get the converted TIMEDATE. */
    /*
     * Parameters for ConvertTIMEDATEtoRFC3339Date API -
     * tdNowTime      = TIMEDATE that is needed for conversion.
     * textBuffer     = String Buffer to recieve converted TIMEDATE.
     *                  It should be of MAXSPRINTF = 256.
     * textBufferSize = String Buffer Size. Big enough to inlude the trailing null char.
     *                  It should be greater than or equal to TIME_RFC3339_LEN(23) + 1.
    */
    if (error = ConvertTIMEDATEtoRFC3339Date(&tdNowTime,textBuffer,TEXTBUFSIZE)){
        PRINTLOG( "Error in converting TIMEDATE to RFC339 TIMEDATE\n");
        return(after_cleanup(-1));
    }
    else {
        PRINTLOG("RFC339 TIMEDATE = %s\n",textBuffer);
        PRINTLOG("Successfully converted Notes TIMEDATE to RFC339 TIMEDATE.\n");
    }

    return(after_cleanup(0));
}


/*
* after_cleanup: Cleans up resources and return the current exit status
*
*     in: Global cleanup with various flags set
*
*    out: As described
*/
STATUS after_cleanup(STATUS exit_status)
{

  /* Done with Notes */
    NotesTerm();

    return(exit_status);
}
