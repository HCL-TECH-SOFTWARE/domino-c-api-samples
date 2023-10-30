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

    PROGRAM:   hashpass

    FILE:      hashpass.c

	PURPOSE:   This program will use various methods to create digest for the ID file which 
			   run this program, and then verify the passowrd.

    SYNTAX:    hashpass


*************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* OS and C include files */
#include <stdio.h>
#include <string.h>
/* Notes API include files */
#if defined (OS390)
#include <lapicinc.h>
#endif

#include <global.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <ostime.h>
#include <osmisc.h>
#include "misc.h"
#include "secerr.h"
#include "bsafe.h"
#include "../../printLog.h"

/* Notes API subroutine */
int main (int argc, char *argv[])
{
	WORD	Digest2Len = 0;
	BYTE	Digest2 [52];
	WORD	Digest3Len = 0;
	BYTE	Digest3 [52];
	STATUS	Verify = NOERROR;
	STATUS	error;

	if (error = NotesInitExtended (argc, argv))
	{
	   PRINTLOG("\n Unable to initialize Notes.\n");
	   return (1);
	}

	if (error = SECHashPassword (strlen ("viking"), (BYTE*) "viking",
	    52, &Digest2Len, Digest2, 0, NULL))
	{
	   PRINTERROR (error,"SECHashPassword");
	}

	Verify = SECVerifyPassword(strlen ("viking"), (BYTE*)"viking",
		Digest2Len, Digest2, 0, NULL);

	if (Verify != NOERROR)
	{
	   PRINTLOG("Verify good password fail.\n");
	}
	else
	{
	   PRINTLOG("Verify good password pass.\n");
	}
						   	 
	Verify = SECVerifyPassword(strlen ("viking2"), (BYTE*)"viking2", Digest2Len, Digest2, 0, NULL);
	
	if (Verify != ERR_SECURE_BADPASSWORD)
	{
	   PRINTLOG("Verify bad password fail.\n");
	}
	else
	{
	   PRINTLOG("Verify bad password pass.\n");
	}

	if (error = SECHashPassword3(strlen ("viking"), (BYTE*)"viking",
	                             SEC_pwddigest_V1, 0, NULL,
	                             12345, NULL, 0,
	                             52, &Digest3Len, Digest3, 0, NULL))
	{
	   PRINTERROR (error,"SECHashPassword3");
	}

	Verify = SECVerifyPassword(strlen ("viking"), (BYTE*)"viking",
	                           Digest3Len, Digest3, 0, NULL);

	if (Verify != NOERROR)
	{
	   PRINTLOG("Verify good password fail.\n");
	}
	else
	{
	   PRINTLOG("Verify good password pass.\n");
	}
						   	 

	if (error = SECHashPassword3(strlen ("viking"), (BYTE*)"viking",
	                             SEC_pwddigest_V2, 0, NULL,
	                             12345, NULL, 0,
	                             52, &Digest3Len, Digest3, 0, NULL))
	{
	   PRINTERROR (error,"SECHashPassword3");
	}

	Verify = SECVerifyPassword(strlen ("viking"), (BYTE*)"viking",
	                           Digest3Len, Digest3, 0, NULL);

	if (Verify != NOERROR)
	{
	   PRINTLOG("Verify good password fail.\n");
	}
	else
	{
	   PRINTLOG("Verify good password pass.\n");
	}

	if (error = SECHashPassword3(strlen ("viking"), (BYTE*)"viking",
	                             SEC_pwddigest_V3, SEC_ai_HMAC_SHA1, NULL,
	                             12345, NULL, 0,
	                             52, &Digest3Len, Digest3, 0, NULL))
	{
	   PRINTERROR (error,"SECHashPassword3");
	}

	Verify = SECVerifyPassword(strlen ("viking"), (BYTE*)"viking", Digest3Len, Digest3, 0, NULL);

	if (Verify != NOERROR)
	{
	   PRINTLOG("Verify good password fail.\n");
	}
	else
	{
	   PRINTLOG("Verify good password pass.\n");
	}

	Verify = SECVerifyPassword(strlen ("viking2"), (BYTE*)"viking2", Digest3Len, Digest3, 0, NULL);
	
	if (Verify != ERR_SECURE_BADPASSWORD)
	{
	   PRINTLOG("Verify bad password fail.\n");
	}
	else
	{
	   PRINTLOG("Verify bad password pass.\n");
	}

	NotesTerm();

	return	0;
}
