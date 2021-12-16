/****************************************************************************

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

#define STRING_LENGTH 256

STATUS LNPUBLIC TimeConversion();
void PrintAPIError (STATUS);


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
	printf("\n Unable to initialize Notes.\n");
	return (1);
    }

    sError=TimeConversion();

    if (sError == NOERROR)
        printf("\n\nProgram completed successfully.\n");
    else
        PrintAPIError (sError);

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
    STATUS sError = NOERROR;
    TIMEDATE_PAIR theApptInterval;
    WORD wLen;
          
    char lTimeRange[STRING_LENGTH];
    char startTime[STRING_LENGTH];
    char endTime[STRING_LENGTH];
    char lstartTime[STRING_LENGTH];
    char lendTime[STRING_LENGTH];
    struct tm *date = NULL;
    time_t timer;
    char *plTimeRange=(char*)&lTimeRange;

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
    printf("\nAfter Text to TIMEDATEPAIR Lower: %s - Upper: %s", lstartTime, lendTime);

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

    printf("\nTIMEDATEPAIR to Text: %s", lTimeRange);
    fflush(stdout);


  Done:
    return (sError);
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
    DWORD   text_len=0;
    /* Get the message for this HCL C API for Notes/Domino error code
       from the resource string table. */

    text_len = OSLoadString (NULLHANDLE,
                             string_id,
                             error_text,
                             sizeof(error_text));

    /* Print it. */

    fprintf (stderr, "\n%s\n", error_text);
}
