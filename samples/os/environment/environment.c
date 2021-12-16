/*************************************************************************

PROGRAM:	environment

FILE:		environment.c

PURPOSE:	Shows functions related to the OS such as 
			1.OSSetEnvironmentTIMEDATE	->	Set a timedate into an environment variable
			2.OSGetEnvironmentTIMEDATE	->	Get a timedate from an environment variable
			3.OSGetEnvironmentSeqNo 	->	returns a sequence number that represents the number of 
											times any environment variable has been changed.
											The sequence number changing means the notes.ini file has been changed. 
											If our variable is dynamic then we can also check if our variable has changed or not.

SYNTAX:		environment

*************************************************************************/
/* OS and C include files */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#if defined LINUX
#include <unistd.h>
#endif

/* HCL C API for Notes/Domino include files */

#include "global.h"
#include "osenv.h"
#include "osfile.h"
#include "osmisc.h"
#include "ostime.h"
#include "misc.h"
#include "oserr.h"

#define STRLENGTH 256

/* Local function prototypes. */
STATUS Cleanup(STATUS exit_status);

/* Program declaration. */
int main(int argc, char *argv[])
{
	
	/* Local data declarations. */
	STATUS		error;
	char		szErrStr[STRLENGTH] = {0};
	char		szEnvVariable[STRLENGTH] = "INI_TIME_TESTDATE"; //Environment variable we are adding to notes.ini file.
	TIMEDATE	tdReadEnv = {0} ,tdReadEnvNew = {0}; //TIMEDATE structure to receive value.
	WORD		wRetTextLength = 0;
	char		szRetTextBuffer[MAXALPHATIMEDATE + 1] = {0};
	static WORD		wEnvSeqNum = 0, wNewEnvSeqNum = 0;
	static BOOL     bCheck = FALSE; 
		
	/* Domino/Notes runtime initialization routine. */
	if(error = NotesInitExtended (argc, argv))
	{
		OSLoadString (NULLHANDLE, ERR(error), szErrStr,  sizeof(szErrStr));
		printf ("Notes initalization failed because %s\n", szErrStr);
		return 0;
	}
	
	/* Sets the current timedate into the Environment variable "INI_TIME_TESTDATE". */
	OSSetEnvironmentTIMEDATE (szEnvVariable, NULL);
		
	/* Gets the timedate from the environment variable "INI_TIME_TESTDATE". */
	if(OSGetEnvironmentTIMEDATE (szEnvVariable, &tdReadEnv))
	{
		printf ("OSGetEnvironmentTimedate was successful for %s.\n",szEnvVariable);
		
		/* Printing environment variable "INI_TIME_TESTDATE" value. */
		if(error = IntlTIMEDATEConvertToText(NULL, NULL, NULL, NULL, &tdReadEnv, sizeof(szRetTextBuffer), szRetTextBuffer, &wRetTextLength))
		{
			OSLoadString (NULLHANDLE, ERR(error), szErrStr, sizeof(szErrStr));
			printf ("Error in IntlTIMEDATEConvertToText bacause %s\n", szErrStr);
			return Cleanup (error);
		}
		printf ("Timedate value of %s is %s\n", szEnvVariable, szRetTextBuffer);
	}	
	else
	{
		printf ("Variable not found.\n");
	}	
	
	/* Get the current environment sequence number. */
	wNewEnvSeqNum = OSGetEnvironmentSeqNo();
	 
	/* 
		Check if the current environment sequence number is equal to the previous sequence number.
		If TRUE then the notes.ini file has not been changed/modified.
		If FALSE then the notes.ini file has been changed/modified. 
	*/
	if (wEnvSeqNum != wNewEnvSeqNum)
	{
		printf("The notes.ini file has been changed/modified.\n");
		wEnvSeqNum = wNewEnvSeqNum;
		
		/* Get the value of "INI_TIME_TESTDATE" from the notes.ini file. */
		OSGetEnvironmentTIMEDATE (szEnvVariable, &tdReadEnvNew);
		
		/*
			Check if the environment variable "INI_TIME_TESTDATE" has been changed/modified dynamically.
		*/
		bCheck = (TimeDateCompare (&tdReadEnv, &tdReadEnvNew) != 0);
		if(bCheck)
		{
			printf("The %s environment variable has been changed in notes.ini file dynamically.\n", szEnvVariable);
		}
		else
		{
			printf("The %s environment variable has not been changed in notes.ini file.\n", szEnvVariable);
		}
	}
	else
	{
		printf("The notes.ini file has not been changed/modified.\n");
	}
	
	return Cleanup (NOERROR);	
	
}

/* Clean up and Domino/Notes runtime shutdown routine. */
STATUS Cleanup (STATUS exit_status)
{
	NotesTerm();
	return (exit_status);
}
	
