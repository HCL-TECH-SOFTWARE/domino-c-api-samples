/*
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
* MAIN - Main loop of the example Domino and Notes mail gateway.
*/

#include "mgate.h"
#include "misc.h"
#include "ostime.h"
#include "osmisc.h"
#include "oserr.h"
#include "osenv.h"
#include "addin.h"
#include "mail.h"
#include "mailserv.h"
#include "mgateerr.h"


STATUS LNPUBLIC AddInMain(HMODULE hResourceModule, int argc, char *argv[])

/*	Main entry point
*
*	Inputs:
*		hResourceModule = Module handle
*		argc,argv = Command line
*
*	Outputs:
*		(Routine) = Error status
*/

{
	STATUS   error = NOERROR;
	DWORD    DeliveryInterval;
	TIMEDATE CurrentTime, ModifiedTime;
	TIMEDATE NextInboundTime, LastOutboundTime, NextOutboundTime;

	hModule = hResourceModule;
	AddInSetStatus(ERR_MGATE_INITIALIZING);
	AddInLogError(ERR_MGATE_STARTED, NOERROR, NULL);

	/* Get the gateway's foreign domain name. */

	if (!OSGetEnvironmentString(MGATE_DOMAIN, GatewayDomain, sizeof(GatewayDomain)-1))
	{
		AddInLogError(ERR_MGATE_NODOMAIN, NOERROR, NULL);
		goto quit;
	}

	/* Get the gateway's inbound/outbound directory drive letter. */

	if (!OSGetEnvironmentString(MGATE_DRIVE, GatewayDrive, sizeof(GatewayDrive)-1))
	{
		AddInLogError(ERR_MGATE_NODRIVE, NOERROR, NULL);
		goto quit;
	}

	/* Get the outbound delivery interval retry time */

	DeliveryInterval = OSGetEnvironmentLong(MGATE_INTERVAL) * 60 * 100;
	if (DeliveryInterval == 0)
		DeliveryInterval = DEFAULT_DELIVERYINTERVAL * (60 * 100);

	OSCurrentTIMEDATE(&NextOutboundTime);
	LastOutboundTime = NextOutboundTime;
	TimeDateIncrement(&NextOutboundTime, 10L * 100L); /* Attempt delivery in 15 seconds */
	NextInboundTime = NextOutboundTime;

	/* Open the outbound message file. Create it if it doesn't exist yet. */

	error = MailOpenMessageFile(MGATE_MSGFILE_NAME, &hMessageFile);
	if (error == ERR_NOEXIST)
	{
		error = MailCreateMessageFile(MGATE_MSGFILE_NAME, 
						MAILBOX_TEMPLATE_NAME, "Mail Gateway", &hMessageFile);
		AddInLogError(ERR_MGATE_MSGFILE_CREATE, NOERROR, MGATE_MSGFILE_NAME);
	}
	if (error) 
	{
		AddInLogError(ERR_MGATE_MSGFILE_OPEN, error, MGATE_MSGFILE_NAME);
		goto quit;
	}

	/* Loop */

	while (TRUE)
		{

		/*	Yield the processor to other tasks and reset schedules.  Also,
			break out of the loop if we should terminate. */

		AddInSetStatus(ERR_MGATE_IDLE);
		if (AddInIdle())
			break;

		/* If outbound message file has been modified or */
		/* sufficient time has passed, run outbound task. */

		OSCurrentTIMEDATE(&CurrentTime);
		MailGetMessageFileModifiedTime(hMessageFile, &ModifiedTime);

		if ((TimeDateCompare(&CurrentTime, &NextOutboundTime) > 0) ||
			(TimeDateCompare(&ModifiedTime, &LastOutboundTime) > 0))
		{
			OutboundTask();
			OSCurrentTIMEDATE(&LastOutboundTime); /* Set AFTER running task */
			NextOutboundTime = LastOutboundTime;
			TimeDateIncrement(&NextOutboundTime, DeliveryInterval);
		}

		/* If it is time to poll for inbound messages, run the task. */

		OSCurrentTIMEDATE(&CurrentTime);
		if (TimeDateCompare(&CurrentTime, &NextInboundTime) > 0)

		{
			InboundTask();
			OSCurrentTIMEDATE(&NextInboundTime);
			TimeDateIncrement(&NextInboundTime, DeliveryInterval);
		}

		/* Do once/day housekeeping, if necessary. */

		if (AddInDayHasElapsed())
			MailPurgeMessageFile(hMessageFile);
		}

	/* Shutdown */

quit:
	if (hMessageFile != NULLHANDLE)
		MailCloseMessageFile(hMessageFile);

	AddInLogError(ERR_MGATE_SHUTDOWN, error, NULL);
	return(NOERROR);
}


