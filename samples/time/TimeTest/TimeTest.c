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

PROGRAM:    TimeTest

FILE:       TimeTest.c

PURPOSE:    Shows the basic time related functions such as 
            1. TimeDateCollate -> Compares two binary TIMEDATE values.
            2. TimeExtractJulianDate -> Extracts the Julian date from a TIMEDATE value.
            3. TimeDateDifferenceFloat -> Floating-point difference between two TIMEDATE values.
               and prints their output to the screen.

SYNTAX:     TimeTest

*************************************************************************/
/* OS and C include files */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* IBM C API for Notes/Domino include files */

#include "global.h"
#include "osmisc.h"
#include "osfile.h"
#include "misc.h"
#include "ostime.h"
#include "../../printLog.h"

/* Local function prototypes */

STATUS error;
char errStr[100];

STATUS Cleanup(STATUS exit_status);


/* Program declaration */
int main(int argc, char *argv[]){
	
	/* Local data declarations */
	
	TIMEDATE Now,Future; 
	NUMBER   time_delta;
	char     retTextBuffer[MAXALPHATIMEDATE + 1];
	WORD     retTextLength;
	
	PRINTLOG("Test of Time functions TimeDateCollate, TimeExtractJulianDate, TimeDateDifferenceFloat\n");
	
	/* Domino/Notes runtime initialization routine */
	if(error=NotesInitExtended(argc,argv)){
		OSLoadString(NULLHANDLE,ERR(error),errStr,100);
		PRINTLOG("Notes initalization failed because %s\n",errStr);
		return Cleanup(error);
	}
	
	PRINTLOG("Get Now current timedate\n");
	/* Gets the system time/date */
	OSCurrentTIMEDATE(&Now);
	
	/* Converts binary TIMEDATE pair to a character text string */
	if(error=ConvertTIMEDATEToText(NULL,NULL,&Now,retTextBuffer,MAXALPHATIMEDATE+1,&retTextLength)){
		OSLoadString(NULLHANDLE,ERR(error),errStr,100);
		PRINTLOG("Error in ConvertTIMEDATEToText bacause %s\n",errStr);
		return Cleanup(error);
	}
	PRINTLOG("Now time/date is %s\n",retTextBuffer);
	
	PRINTLOG("Get Future current timedate\n");
	/* Gets the system time/date */
	OSCurrentTIMEDATE(&Future);
	
	/* Increment binary TIMEDATE value*/
	TimeDateIncrement(&Future, 6000L);
	
	/* Converts binary TIMEDATE pair to a character text string */
	if(error=ConvertTIMEDATEToText(NULL,NULL,&Future,retTextBuffer,MAXALPHATIMEDATE+1,&retTextLength)){
		OSLoadString(NULLHANDLE,ERR(error),errStr,100);
		PRINTLOG("Error in ConvertTIMEDATEToText bacause %s\n",errStr);
		return Cleanup(error);
	}
	PRINTLOG("Future time/date is %s\n",retTextBuffer);
	
	/* Extracts the Julian date froma a TIMEDATE value */
	PRINTLOG("Date converted to Julian date using TimeExtractJulianDate is %d\n",TimeExtractJulianDate(&Now));
	
	/* Floating-point difference between two TIMEDATE values */
	TimeDateDifferenceFloat(&Future, &Now, &time_delta); 
	
	PRINTLOG("Difference between Now and Future date using TimeDateDifferenceFloat is %f\n",time_delta);
	
	/* Compares two binary TIMEDATE values */
	if (TimeDateCollate(&Future, &Now) == 1)
	{
		PRINTLOG("The TimeDateCollate function has Passed as 'Future' date is bigger than 'Now' date\n");
	}
	else
	{
		PRINTLOG("The TimeDateCollate function has Failed\n");
	}
		
	return Cleanup(NOERROR);	
}

/* Clean up and Domino/Notes runtime shutdown routine */
STATUS Cleanup(STATUS exit_status){
	NotesTerm();
	return (exit_status);
}

