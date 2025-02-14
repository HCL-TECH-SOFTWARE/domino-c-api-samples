/****************************************************************************
 *
 * Copyright HCL Technologies 1996, 2024.
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
 */

#ifndef ICALSAMPLE_H

#define ICALSAMPLE_H

#if defined(OS400)
#pragma convert(850)
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "intl.h"

#define CRLF "\r\n"

/*
Generate start/end time for the first instance of the meeting, also generate RecurrenceID of the instance to be decliend.
The first instance will start at 10 am the next day, UTC time.

input:	none
output:	
pStartTime	start time of first instance
pEndTime	end time of first instance
pRecurrenceID	RecurrenceID of the instance to be decliend
*/
STATUS GetStartTime(char* pStartTime, char* pEndTime, char* pRecurrenceID)
{
	INTLFORMAT intlfmt = 
	{
		DATE_YMD | NUMBER_LEADING_ZERO | CLOCK_24_HOUR,	//	WORD Flags
		0,						//	BYTE CurrencyDigits
		sizeof(INTLFORMAT),		//	BYTE Length
		0,						//	int	 TimeZone
		"",						//	char AMString[ISTRMAX]
		"",						//	char PMString[ISTRMAX]
		"",						//	char CurrencyString[ISTRMAX]
		"",						//	char ThousandString[ISTRMAX]
		"",					//	char DecimalString[ISTRMAX]
		"",						//	char DateString[ISTRMAX]
		"",						//	char TimeString[ISTRMAX]
		"",						//	char YesterdayString[YTSTRMAX]
		"",						//	char TodayString[YTSTRMAX]
		""						//	char TomorrowString[YTSTRMAX]
	};

	TFMT fmt;
	TIMEDATE now;
	char pOut[1024] = {0};

	//memset(&intlfmt, 0, sizeof(INTLFORMAT));
	memset(&fmt, 0, sizeof(TFMT));

	fmt.Date = TDFMT_FULL4;	//only year,month,day

	OSCurrentTIMEDATE(&now);

	TimeDateAdjust(&now, 0, 0, 0, 1, 0, 0);
	ConvertTIMEDATEToText(&intlfmt, &fmt, &now, pOut, MAXALPHATIMEDATE, NULL);

	//"Z" here means UTC time, see rfc 5545
	// start at 10 am next day, end at 12 am next day, 
	sprintf(pStartTime, "%sT100000Z", pOut);
	sprintf(pEndTime, "%sT120000Z", pOut);

	TimeDateAdjust(&now, 0, 0, 0, 1, 0, 0);
	ConvertTIMEDATEToText(&intlfmt, &fmt,
		&now, pOut,
		MAXALPHATIMEDATE, NULL);

	// the instance at the day after tomorrow will be declined
	sprintf(pRecurrenceID, "%sT100000Z", pOut);

	return 0;
}

#endif
