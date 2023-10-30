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

    PROGRAM:      rfc3339totimedate

    FILE:         rfc3339totimedate.c

    SYNTAX:       rfc3339totimedate

    PURPOSE:      Demonstrates ConvertRFC3339DateToTIMEDATE usage

    DESCRIPTION:  This is a sample code that demonstrates on how to convert 
                  RFC3339 date format generated from current time date 
		  to Note TIMEDATE format.

 ***************************************************************************/

/*
 * Windows header file. Include only if compiling for Windows.
 */

#if(defined(NT))
#include <windows.h>
#endif

#if defined (LINUX)
#include <sys/time.h>
#include <errno.h>
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
#include <oserr.h>
#include <time.h>
#include <osmisc.h>
#include "../../printLog.h"

#define STRING_LENGTH 256
#define SUCCESS 0

STATUS RFC3339TimeConversion();

/************************************************************************

    FUNCTION:    Main

*************************************************************************/

int main(int argc, char *argv[])
{
    STATUS sError = NOERROR;

    /*
     *  Initialize Domino and Notes.
     */

    if ((sError = NotesInitExtended (argc, argv)) != NOERROR)
    {
	PRINTLOG("\n Unable to initialize Notes.\n");
	return (1);
    }

    sError = RFC3339TimeConversion();

    if (sError == NOERROR)
    {
        PRINTLOG("\n\nProgram completed successfully.\n");
    }
    else
    {
        PRINTERROR (sError,"RFC3339TimeConversion");
    }

    NotesTerm();
    return (0);

}

/************************************************************************
 * FUNCTION:      SubString
 *
 * PURPOSE:       This function returns the substring based on given length
 * 
 * INPUT:         source - Soruce string delimited by NULL
 *                pos    - Start position of the substring
 *                len    - Length of the substring populated from source string.
 *                         len shall not be greater than STRING_LENGTH
 * OUTPUT:
 *                subStr - Computed/populated substring for the given length from position
 *
***************************************************************************/

char* SubString(char *source, char *subStr, int pos, int len)
{
   int c = 0; /* Counter variable to count substring */

   if (len >= STRING_LENGTH)
   {
      return subStr;
   }

   memset(subStr, '\0', STRING_LENGTH);

   while (c < len) 
   {
      if ((pos+c-1) >= STRING_LENGTH)
          break;	      
      subStr[c] = source[pos+c-1];
      c++;
   }
   
   return subStr;
}

/************************************************************************

    FUNCTION:     RFC3339TimeConversion

    PURPOSE:      This routine will demonstrate how to use the following 
                  API, 
		  ConvertRFC3339DateToTIMEDATE

*************************************************************************/

STATUS RFC3339TimeConversion()
{
    STATUS sError = NOERROR;
    
    char     rfc3339Date[STRING_LENGTH] = { 0 };
    char     timeDateBuf[STRING_LENGTH] = { 0 };
    char     errBuf[STRING_LENGTH]      = { 0 };
    char     subStr[STRING_LENGTH]      = { 0 };
    char     subStr2[STRING_LENGTH]     = { 0 };
    WORD     timedateLen                = 0;
    char     sZone[STRING_LENGTH]       = { 0 };
    TIMEDATE td                         = { 0 };
    char     *pcrfc3339Date             = (char*) rfc3339Date;
    time_t   rawtime;
    short    count                      = 0;
 
    do 
    {

#if defined (W64)
      SYSTEMTIME lt;
      GetLocalTime(&lt); /* Get Local time */
      /* Format rfc 3339 dateTtime using SYSTEMTIME structure */
      sprintf(rfc3339Date, "%04d-%02d-%02dT%02d:%02d:%02d", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond);
      time( &rawtime ); /* Get the current time */
#elif defined (LINUX)
      struct timeval tv;
      if (gettimeofday(&tv, NULL) != SUCCESS)
      {
         PRINTLOG("\n Error getting gettimeofday.");
         strerror_r(errno, errBuf, sizeof(errBuf)-1);
         PRINTLOG("\n Error:%s", errBuf);
         return -1;
      }
      rawtime = tv.tv_sec;
      /* Format rfc 3339 dateTtime using localtime */
      strftime(rfc3339Date, sizeof(rfc3339Date), "%Y-%m-%dT%T", localtime(&rawtime));
#endif    

      /* Get the Zone from gmtime */
      strftime(sZone, sizeof(sZone)-1, "%z",  localtime(&rawtime));

      /* Format the RFC specific date time with zone */
      if (!strncmp(SubString(sZone, subStr2, 1, 3), "+00", strlen("+00")) && !strncmp(SubString(sZone, subStr, 4, 2), "00", strlen("00")))
      {
         sprintf(rfc3339Date,"%sZ", rfc3339Date);
      }
      else
      {
         if (count == 0)
         {
            /* For adding secs fraction to the rfc3339 time date */
#if defined (W64)
           sprintf(rfc3339Date,"%s.%02d%s:%s", rfc3339Date, lt.wMilliseconds/10, 
			        SubString(sZone, subStr2, 1, 3), SubString(sZone, subStr, 4, 2));
#elif defined (LINUX)
           /* tv.tv_usec is micro sec fraction to convert to 2 digit fraction sec div by 10000 */
           sprintf(rfc3339Date,"%s.%02d%s:%s", rfc3339Date, tv.tv_usec/10000, 
			        SubString(sZone, subStr2, 1, 3), SubString(sZone, subStr, 4, 2));
#endif
         }
         else
         {
            /* For non secs fraction to the rfc3339 time date */
           sprintf(rfc3339Date,"%s%s:%s", rfc3339Date, SubString(sZone, subStr2, 1, 3), 
                                          SubString(sZone, subStr, 4, 2));
         }
      }

      PRINTLOG("\n RFC3339 date format [%s]", rfc3339Date);

      /* Convert Text Range to Internal TIMEDATE pair */
      sError = ConvertRFC3339DateToTIMEDATE( &pcrfc3339Date,
                                       (WORD)strlen(rfc3339Date),
                                       FALSE,
                                       &td );
      if (sError)
      {
        goto Done;
      }

      /* Converts binary TIMEDATE pair to a character text string */
      if ((sError = ConvertTIMEDATEToText(NULL,NULL,&td,timeDateBuf,STRING_LENGTH,&timedateLen)) != NOERROR)
      {
        return sError;
      }
    
      PRINTLOG("\n Notes TIMEDATE Format [%s]", timeDateBuf);
      fflush(stdout);

    } while( count++ < 1); /* Loop to iterate twice */


  Done:
    return (sError);
}

