/*
* Copyright HCL Technologies 1996, 2023.
*
* Licensed under the Apache License, Version 2.0 (the "License");
*you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
*WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* OUTBOUND - Outbound delivery task of the mail gateway */

#include "mgate.h"

#include "ostime.h"
#include "osmem.h"
#include "osmisc.h"
#include "osenv.h"
#include "intl.h"
#include "misc.h"
#include "textlist.h"
#include "addin.h"
#include "mail.h"
#include "mailserv.h"
#include "routeerr.h"
#include "mgateerr.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/* Foreign mail message header format strings */

static char AttachmentFormat[] = "A%s,%s\n";
static char BodyFormat[] =  "B%s\n";
static char CCFormat[]   =	"C%.*s\n";
static char DateFormat[] =	"D%s\n";
static char OriginatorFormat[] = "O%s\n";
static char SubjectFormat[] = "S%s\n";
static char ToFormat[]   =	"T%.*s\n";

static char IntendedRecipientsFormat[] = "BIntended recipients: %s\n";
static char FailureReasonFormat[] = "BFailure reason: %s\n";
static char NonDeliverySubject[] = "Non-Delivery of:";

STATUS CreateForeignMessage(DHANDLE hMessage,
		 		char *OriginatorName,
		 		char *RecipientName);
STATUS AddToItems(FILE *ForeignFile,
		 		DHANDLE hMessage, WORD ItemNum, char *Format);

void near pascal OutboundTask()

/* OutboundTask - Outbound (from Domino and Notes to foreign mail system) delivery task.
*
*	Inputs:
*		hMessageFile - Outbound message file handle.
*
*	Outputs:
*		None.
*/
{
	register STATUS error;
	char            RecipientName[MAXRECIPIENTNAME+1];
	WORD            RecipientNameLength;
	char            UserName[MAXUSERNAME + 1];
	WORD            UserNameLength;
	char            DomainName[MAXDOMAINNAME+1];
	WORD            DomainNameLength;
	char            String[MAXSPRINTF], Originator[MAXRECIPIENTNAME+1];
	WORD            StringLength;
	DHANDLE         hMessageList = NULLHANDLE, hMessage;
	DARRAY          *MessageList;
	WORD            MessageCount, RecipientCount;
	WORD            Msg, Rec;

	AddInSetStatus(ERR_MGATE_TRANSFERING, 0L);

	/* Search the message file for any undelivered mail messages */

	error = MailCreateMessageList(hMessageFile,
				 &hMessageList, &MessageList, &MessageCount);
	if (error)
	{
		AddInLogError(ERR_MGATE_MSGLIST, error, MGATE_MSGFILE_NAME);
		return;
	}

	/* Deliver each of the outbound messages. */

	for (Msg = 0; Msg < MessageCount; Msg++)
	{

		/* Open the outbound Domino and Notes message. */

		error = MailOpenMessage(MessageList, Msg, &hMessage);
		if (error) continue;

		/* Get the originator's name/address. */

		MailGetMessageOriginator(MessageList, Msg,
				Originator, sizeof(Originator), &StringLength);

		/* 	Deliver the message to each recipient by copying it to
			his/her directory. */

		MailGetMessageInfo(MessageList, Msg, &RecipientCount, NULL, NULL);
		for (Rec = 0; Rec < RecipientCount; Rec++)
		{

			/* Get recipient address and parse into user and domain.  The
				recipient's "user name" is actually his/her directory. */

			MailGetMessageRecipient(MessageList, Msg, Rec, RecipientName,
							sizeof(RecipientName), &RecipientNameLength);
			MailParseMailAddress(RecipientName, RecipientNameLength,
						UserName, sizeof(UserName), &UserNameLength,
						DomainName, sizeof(DomainName), &DomainNameLength);
			AddInSetStatus(ERR_MGATE_TRANSFERING_TO, (DWORD) UserName);

			/* Create a foreign mail system message file from the Domino and Notes
				message, creating it in the recipient's directory. */

			error = CreateForeignMessage(hMessage, Originator, UserName);
			if (error) 
				goto next;

			/* 	If creation was successful delete the recipient from the
				message.  Otherwise, save error text and try again next time. */

			if (!error)
			{
				MailSendDeliveryReport(MessageList, Msg, 1, &Rec);
				MailDeleteMessageRecipient(MessageList, Msg, Rec);
				AddInLogError(ERR_MGATE_SENDSUCCESS, NOERROR, RecipientName);
			}
			else
			{
				AddInFormatError(String, error);
				MailSetMessageLastError(MessageList, Msg, String);
			}

next:		AddInSetStatus(ERR_MGATE_TRANSFERING, 0L);
		}

		/* If the message has been delivered to all the recipients, */
		/* purge the message from the mail file. If the message is */
		/* not purged but delivery has timed-out, the originator will */
		/* be sent a non-delivery report for each remaining recipient. */

		MailCloseMessage(hMessage);
		MailPurgeMessage(MessageList, Msg);
	}

	/* Free the message list and close the message file */

	if (hMessageList != NULLHANDLE)
	{
		OSUnlockObject(hMessageList);
		OSMemFree(hMessageList);
	}

	AddInSetStatus(ERR_MGATE_IDLE, 0L);
}	

STATUS CreateForeignMessage(DHANDLE hMessage,
                            char *OriginatorName, char *RecipientName)

/* CreateForeignMessage - Create a foreign mail system message file.
*
*	Inputs:
*		hMessage - Open message handle.
*		OriginatorName - Originator's name.
*		RecipientName - Recipient's user name (8 or less chars).
*
*	Outputs:
*		Return value - error status.
*/
{
	STATUS   error;
	FILE     *ForeignFile, *BodyFile;
	char     String[MAXSPRINTF], FileName[_MAX_PATH], RecipientDir[_MAX_PATH];
	char     AttachmentFileName[_MAX_PATH], OriginalFileName[_MAX_PATH];
	char     BodyFileName[_MAX_PATH], *delim;
	WORD     StringLength, Length, Att;
	BLOCKID  bhAttachment;
	DWORD    BodyFileSize;
	TIMEDATE Time;
	BOOL     NonDeliveryReport;

	/* Create a foreign message file in recipient's directory. */

	strcpy(RecipientDir, MGATE_OUTBOUND_DIR);
	strcat(RecipientDir, "\\");
	strcat(RecipientDir, RecipientName);
	error = GetUniqueFileName(GatewayDrive, RecipientDir,
							  MGATE_MSG_EXT, FileName);
	if (error) 
		return(error);
	ForeignFile = fopen(FileName, "w");		
	if (ForeignFile == NULL)
	{
		AddInLogError(ERR_MGATE_RECIPCREATE, NOERROR, FileName);
		return(ERR_MGATE_RECIPCREATE);
	}

	/* Write the "Date" item line. */

	MailGetMessageItemTimeDate(hMessage, MAIL_POSTEDDATE_ITEM_NUM, &Time);
	ConvertTIMEDATEToText(NULL, NULL, &Time, String, sizeof(String) - 1, NULL);
	fprintf(ForeignFile, DateFormat, String);

	/* Write the "To" and "CC" item lines */

	error = AddToItems(ForeignFile, hMessage, MAIL_SENDTO_ITEM_NUM, ToFormat);
	if (error) 
		goto Close;
	error = AddToItems(ForeignFile, hMessage, MAIL_COPYTO_ITEM_NUM, CCFormat);
	if (error) 
		goto Close;

	/* Write the "Originator" item line. */

	fprintf(ForeignFile, OriginatorFormat, OriginatorName);

	/* Write the "Subject" string. If message is non-delivery report,
		prefix subject with "NonDelivery of:" */

	if (NonDeliveryReport = MailIsNonDeliveryReport(hMessage))
    	{
		strcpy(String, NonDeliverySubject);
		Length = strlen(NonDeliverySubject);
    	}	
    	else
		Length = 0;
	MailGetMessageItem(hMessage, MAIL_SUBJECT_ITEM_NUM,
			&String[Length], (WORD) (sizeof(String) - Length), &StringLength);
	fprintf(ForeignFile, SubjectFormat, String);

	/* If the message is a non-delivery report, add the error info. */

	if (NonDeliveryReport)
	{
		MailGetMessageItem(hMessage, MAIL_INTENDEDRECIPIENT_ITEM_NUM,
							String, sizeof(String), &StringLength);
		fprintf(ForeignFile, IntendedRecipientsFormat, String);

		MailGetMessageItem(hMessage, MAIL_FAILUREREASON_ITEM_NUM,
							String, sizeof(String), &StringLength);
		fprintf(ForeignFile, FailureReasonFormat, String);
	}

	/* Get the message body and copy each line of text to the foreign file. */

	error = GetUniqueFileName(GatewayDrive, RecipientDir, "TMP", BodyFileName);
	if (!error)
		error = MailGetMessageBodyText(hMessage,
				NULL, 	/* Use standard Body item */
				"\r\n",	/* Newline-terminate */
				80,		/* 80 chars per line */
				TRUE, 	/* Convert TABs */
				BodyFileName,
				&BodyFileSize);
	if (!error)
		BodyFile = fopen(BodyFileName, "r");
	if (error || (BodyFile == NULL))
	{
		AddInLogError(ERR_MGATE_TEXT_OPEN, error, BodyFileName);
		goto Close;
	}

	/* Copy each line of body text to the foreign file. */

	while (fgets(String, sizeof(String), BodyFile))
    	{
        	if (delim = strchr(String, '\n'))  /* Remove trailing newline */
				*delim = '\0';
		fprintf(ForeignFile, BodyFormat, String);
   	 }	
	fclose(BodyFile);
	unlink(BodyFileName);

	/* Get any attachments and copy them to files in the user's directory. */

	for (Att = 0; MailGetMessageAttachmentInfo(hMessage, Att,
				&bhAttachment, OriginalFileName,
				NULL, NULL, NULL, NULL, NULL); Att++)
	{
		error = GetUniqueFileName(GatewayDrive, RecipientDir, MGATE_ATT_EXT,
				 AttachmentFileName);
		if (!error)
			error = MailExtractMessageAttachment(hMessage, bhAttachment,
				 AttachmentFileName);
		if (!error)
			fprintf(ForeignFile, AttachmentFormat,
				AttachmentFileName, OriginalFileName);
		else
		{
			AddInLogError(ERR_MGATE_ATT_CREATE, error, AttachmentFileName);
			goto Close;
		}
	}

	/* Close the message file */

Close:
	fclose(ForeignFile);
	if (error)
		unlink(FileName);
	return(error);
}

STATUS near pascal GetUniqueFileName(char *Drive, char *Dir, char *Ext,
                         char *FileName)

/* GetUniqueFileName - Create a unique file name.
*
*	Inputs:
*		Drive - Drive letter string pointer.
*		Dir - Directory name string pointer.
*		Ext - Extension name string pointer.
*
*	Outputs:
*		*FileName - Unqiue file name.
*/
{
	int  file;
	WORD Num;
	char Name[17];

	/* Increment through numbered file names until a non-existent one found. */

	for (Num = 0; Num <= 32767; Num++)
	{
		itoa(Num, Name, 10);
		_makepath(FileName, Drive, Dir, Name, Ext);
		if ((file = open(FileName, O_BINARY | O_RDONLY, 0)) == -1)
			return(NOERROR);
		close(file);
	}
	FileName[0] = '\0';
	return(ERR_MGATE_NOUNIQUE);
}



STATUS AddToItems(FILE *ForeignFile,
		 	DHANDLE hMessage, WORD ItemNum, char *Format)

/* AddToItems - Add To/CC names to message.
*
*	Inputs:
*		ForeignFile - Foreign message file pointer.
*		hMessage - Open Domino and Notes message handle.
*		ItemNum - Domino and Notes message item number (MAIL_SENDTO_ITEM_NUM, etc.)
*		Format - Item format string pointer.
*
*	Outputs:
*		Return value - error status.
*/
{
	STATUS  error;
	BLOCKID bhValue;
	WORD    ValueType,  RecipientNameLength, n;
	DWORD   ValueLength;
	char    *Value, *RecipientName;

	error = MailGetMessageItemHandle(hMessage, ItemNum, &bhValue, &ValueType, &ValueLength);
	if (error) return(NOERROR); /* Optional field */

	Value = OSLockBlock(char, bhValue);

	for (n = 0;;n++)
	{

		/* Get the next name in the list. */

		if (ValueType == TYPE_TEXT_LIST)
		{
			if (ListGetText(Value, TRUE, n,
						&RecipientName, &RecipientNameLength) != NOERROR)
				break;
		}
		else if (ValueType == TYPE_TEXT)
		{
			if (n != 0) break; /* Only one entry */
			RecipientName = Value + sizeof(WORD);
			RecipientNameLength = (WORD) ValueLength - sizeof(WORD);
		}
		else
			break;

		/* Add the name to the foreign message file. */

		if (RecipientNameLength != 0)
			fprintf(ForeignFile, Format, RecipientNameLength, RecipientName);
	}
	
	OSUnlockBlock(bhValue);
	return(NOERROR);
}

