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
*/

/* INBOUND - Inbound message task of the mail gateway. */

#include "mgate.h"
#include "ostime.h"
#include "osmem.h"
#include "misc.h"
#include "textlist.h"
#include "mail.h"
#include "addin.h"
#include "mailserv.h"
#include "mgateerr.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

static char FailureReasonFormat[] = "BFailure reason: %s\r\n";


void near pascal InboundTask()

/* InboundTask - Inbound (from foreign mail system) message handling task.
*
*	Inputs:
*		None.
*
*	Outputs:
*		None.
*/

{
    STATUS          error;
    DHANDLE         hMailboxFile, hMessage;
    TIMEDATE        Time;
    char            String[258], Code, *Value, OriginatorName[MAXRECIPIENTNAME+1];
    WORD            ValueLength;
    char            BodyFileName[_MAX_PATH], ForeignFileName[_MAX_PATH];
    FILE           *BodyFile, *ForeignFile;

    DHANDLE         hRecipientsItem, hToItem, hCCItem;
    WORD            RecipientsItemSize, ToItemSize, CCItemSize;
    WORD            Recipients, ToEntries, CCEntries;
    LIST           *RecipientsItem, *ToItem, *CCItem;

    HANDLE          hDir;
    WIN32_FIND_DATA FindBuf;
	
    hMailboxFile = hMessage = NULLHANDLE;

    /* Find a new foreign mail message in the inbound message directory. */

    _makepath(ForeignFileName, GatewayDrive, MGATE_INBOUND_DIR,
			  "*", MGATE_MSG_EXT);

    hDir = FindFirstFile(ForeignFileName, &FindBuf);
    if (hDir == (HANDLE) MAXWORD)
    {
	     return;
    }

    _makepath(ForeignFileName, GatewayDrive, MGATE_INBOUND_DIR,
			  FindBuf.cFileName, NULL);

    /* Transfer the foreign message to Domino and Notes. */

    while (TRUE)
	{
		hRecipientsItem = hToItem = hCCItem = NULLHANDLE;
		Recipients = ToEntries = CCEntries = 0;

		AddInSetStatus(ERR_MGATE_RECEIVING, (char far *) ForeignFileName);
		ForeignFile = fopen(ForeignFileName, "r+");
		if (ForeignFile == NULL)
		{
			AddInLogError(ERR_MGATE_MSGOPEN, NOERROR, ForeignFileName);
			goto done;
		}

		/* Create a Domino and Notes mail message. */

		if (hMailboxFile == NULLHANDLE)
		{
			error = MailOpenMessageFile(NULL, &hMailboxFile);
			if (error) goto Close;
		}
		error = MailCreateMessage(hMailboxFile, &hMessage);
		if (error) goto Close;

		/* Create the recipients list items. */

		error = ListAllocate(0, 0, TRUE, &hRecipientsItem, &RecipientsItem, &RecipientsItemSize);
		if (error) goto Close;
		OSUnlockObject(hRecipientsItem);

		error = ListAllocate(0, 0, TRUE, &hToItem, &ToItem, &ToItemSize);
		if (error) goto Close;
		OSUnlockObject(hToItem);

		error = ListAllocate(0, 0, TRUE, &hCCItem, &CCItem, &CCItemSize);
		if (error) goto Close;
		OSUnlockObject(hCCItem);

		/* Create a temporary body text file. */

		GetUniqueFileName(GatewayDrive, "", "TMP", BodyFileName);
		BodyFile = fopen(BodyFileName, "w");
		if (BodyFile == NULL)
		{
			AddInLogError(ERR_MGATE_BODYFILE, NOERROR, BodyFileName);
			error = ERR_MGATE_BODYFILE;
			goto Close;
		}

		/* Get each item line from the foreign message file. */

		while (fgets(String, sizeof(String), ForeignFile))
		{
			Code = String[0];
			Value = &String[1];
			ValueLength = strlen(Value);
			if (Value[ValueLength-1] == '\n')  /* Strip trailing newline */
			{
				Value[ValueLength-1] = '\0';
				ValueLength--;
			}

			switch (Code)
			{
				case 'A':
					{

					/* Add an attachment. Original attachment file name
					   separated from actual file name by a comma. */

					char *OriginalName = strchr(Value, ',');
					*OriginalName++ = '\0';
					
					error = MailAddMessageAttachment(hMessage,
									Value, OriginalName);
					unlink(Value);
					break;
					}

				case 'B':

					/* Write a line of body text to the body file. */

					fprintf(BodyFile, Value);
					break;

				case 'C':

					/* Add a CC item name. */

					error = ListAddEntry(hCCItem, TRUE,
								&CCItemSize, CCEntries,
								Value, ValueLength);
					CCEntries++;
					break;

		
				case 'D':

					/* Add Posted Date item. */

					ConvertTextToTIMEDATE (NULL, NULL, &Value, ValueLength, &Time);
					error = MailAddHeaderItem(hMessage, MAIL_POSTEDDATE_ITEM_NUM,
												(char *)&Time, sizeof(TIMEDATE));
					break;

				case 'O':
			
					/* Add the From item. */

					error = MailAddHeaderItem(hMessage, MAIL_FROM_ITEM_NUM,
										Value, ValueLength);
					if (error) goto Close;
					strcpy(OriginatorName, Value); /* Save originator name */

					/* Add the "FromDomain" header item using gateway's domain name. */

					error = MailAddHeaderItem(hMessage, MAIL_FROMDOMAIN_ITEM_NUM,
										GatewayDomain, (WORD) strlen(GatewayDomain));
					break;

				case 'R':

					/* Add a Recipient list item name. */

					error = ListAddEntry(hRecipientsItem, TRUE,
								&RecipientsItemSize, Recipients,
								Value, ValueLength);
					Recipients++;
					break;

				case 'S':

					/* Add a Subject item. */

					error = MailAddHeaderItem(hMessage, MAIL_SUBJECT_ITEM_NUM,
									Value, ValueLength);
					break;

				case 'T':

					/* Add a To item name. */

					error = ListAddEntry(hToItem, TRUE,
								&ToItemSize, ToEntries,
								Value, ValueLength);
					ToEntries++;
					break;

				default:
					AddInLogError(ERR_MGATE_UNKNOWNITEM, NOERROR, String);
			}
			if (error) 
				goto Close;
		}


		/* Add the Recipients item to the message. */

		if (Recipients == 0) 
			goto Close; /* No recipients? Maybe all bad names. */
		error = MailAddRecipientsItem(hMessage, hRecipientsItem, RecipientsItemSize);
		if (error) 
			goto Close;
		hRecipientsItem = NULLHANDLE;

		/* Add the To and CC items to the message. */

		error = MailAddHeaderItemByHandle(hMessage, MAIL_SENDTO_ITEM_NUM, hToItem, ToItemSize, 0);
		if (error) 
			goto Close;
		hToItem = NULLHANDLE;
		error = MailAddHeaderItemByHandle(hMessage, MAIL_COPYTO_ITEM_NUM, hCCItem, CCItemSize, 0);
		if (error) 
			goto Close;
		hCCItem = NULLHANDLE;

		/* Add the body item to the message. */

		fclose(BodyFile);
		BodyFile = NULL;
		error = MailAddMessageBodyTextExt(hMessage,
							NULL,	        /* Use standard Body item */
							BodyFileName,
							0L,		/* Use standard font */
							"\r\n\x1a",	/* CR/LF/Ctrl-Z delimited lines */
							0,		/* New paragraph at any blank lines */
							NULL);	        /* No translation table */
		if (error) 
			goto Close;

		/* Transfer the message to the local mail router */

		error = MailTransferMessageLocal(hMessage);
		if (!error)
			AddInLogError(ERR_MGATE_TRANSFERSUCCESS, NOERROR, NULL);

	Close:
		if (BodyFile != NULL)
			fclose(BodyFile);
		unlink(BodyFileName);

		if (hMessage != NULLHANDLE)
		{
			MailCloseMessage(hMessage);
			hMessage = NULLHANDLE;
		}

		/* If no error, delete the original foreign message.  If there was
			an error, return the file to the originator. */

		if (!error)
		{

			/* Delete the Foreign message file. */
	
			fclose(ForeignFile);
			unlink(ForeignFileName);
		}
		else
		{
			char BadFileName[_MAX_PATH];

			AddInLogError(ERR_MGATE_TRANSFERERROR, error, NULL);

			/* Add failure reason text to end of message. */

			AddInFormatError(String, error);
			fseek(ForeignFile, 0, SEEK_END);
			fprintf(ForeignFile, FailureReasonFormat, String);
			fclose(ForeignFile);

			/* Return the file to the originator's directory. */

			_makepath(BadFileName, GatewayDrive, OriginatorName, FindBuf.cFileName, "MSG");
			rename(ForeignFileName, BadFileName);
		}

	/* Find another foreign message file. */


	if (!FindNextFile(hDir, &FindBuf))
	{
		 FindClose(hDir);
		 return;
	}

	_makepath(ForeignFileName, GatewayDrive, MGATE_INBOUND_DIR,
			  FindBuf.cFileName, NULL);

	} /* End of per-message loop */

	/* Close file find directory */

	FindClose(hDir);

done:
	if (hMailboxFile != NULLHANDLE)
		MailCloseMessageFile(hMailboxFile);

	AddInSetStatus(ERR_MGATE_IDLE);
}	

