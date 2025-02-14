Licensed Materials - Property of HCL
CIR9ZEN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
-------
mgate    - Sample HCL C API for Domino and Notes Mail Gateway Program


PURPOSE
-------
This program demonstrates the basic steps to exchanging messages between
Domino and Notes mail and another email system.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)


FILES
-----
main.c      -  Main C program file
inbound.c   -  Inbound mail task C source file
outbound.c  -  Outbound mail task C source file
mgate.h     -  Symbolic constants, function prototypes, and static variables
mgateerr.h  -  Error codes and error message definitions

mswin32.rc  -  Resource file for Windows 32-bit resource compiler
mswin64.rc  -  Resource file for Windows 64-bit resource compiler
mswin32.mak -  Makefile for Windows 32-bit.
mswin64.mak -  Makefile for Windows 64-bit.

readme.txt  -  this file


INSTALLING mgate
----------------
These instructions assume that you are working on a test HCL Domino Server.
Do not test MGATE on a production server.

1) Create directories named

		\MGATE

		\MGATE\PROGRAM

		\MGATE\INBOUND

		\MGATE\USER

		\MGATE\USER\BOB

2) Put all the files in this directory into \MGATE\PROGRAM.

3) Go to the directory \MGATE\PROGRAM.

4) Copy NMGATE.EXE into the Domino Program directory.

5) Go to your Domino data directory.

6) Edit NOTES.INI.

	Add the following lines

		MgateDrive=C:   (or other drive that contains \MGATE)

		MgateDomain=MGATE_DOMAIN

		MgateInterval=2

		MailTimeout=1

	Find the line that begins with "ServerTasks" and add ",MGATE" to the
	end of this line.

7) With the Notes workstation software, add a domain record
to the server's Domino Directory (Address book). Set the fields as follows:

		Domain type:  Foreign Domain
		Foreign domain name: MGATE_DOMAIN

		Gateway server name: <your server name>
			Note:  If <your server name> is a hierarchical
			       name, specify the full server name in
			       abbreviated format, for example,
			       myserver/Acme

		Gateway mail file name: MGATE.NSF

8) Start (or restart) the HCL Domino Server software.



SENDING AN OUTBOUND MESSAGE
---------------------------
How It Works

An outbound message is one that originates from a Notes user and is
destined for someone outside the Domino and Notes mail system.

Outbound messages first are routed through the foreign domain that you
created above. The gateway mail file holds the messages until MGATE is
ready to transfer them to the foreign users. Based on the modification
time of the gateway mail file, MGATE detects when there are pending
outbound messages.

When MGATE finds an outbound message, it delivers the message to a
directory with the same name as the foreign user. (For this reason, all
foreign user names must be 8 characters or less.) All foreign user
directories are contained within \MGATE\USER.

After delivery, the outbound message is a plain ASCII text file. Each line
of the file begins with an item code, which is one of:

	A - Attachment  (path name of attachment file, original file name)
	B - Body text line
	C - CC name
	D - Time/date
	O - Originator name
	S - Subject
	T - To name

Following the item code on each line is the contents of the item.

For example, a message from a Notes user delivered to a foreign mail user
might appear as follows in \MGATE\USER\BOB.

	D08-02-91 03:31:18 PM
	TBob @ MGATE_DOMAIN
	OChuck @ NOTES
	SHello...
	BHow are you !!!
	BI hope you are having a good day.

If a directory for the foreign user is not found, MGATE continues to look
for the directory during every outbound attempt, until the mail times out.
The time-out interval (in hours) is specified by MailTimeout. When an
outbound message times out, a failure report is sent to the mail's sender.



TESTING AN OUTBOUND MESSAGE
---------------------------
To test the outbound portion of MGATE

1) Start (or switch to) the Notes workstation software.

2) Make sure your mail server is set to the test server you are
working on.

3) Send a Domino mail message to Bob @ MGATE_DOMAIN.

4) Very quickly, the message should be delivered to \MGATE\USER\BOB.
The name of the message file should be 0.MSG.



RECEIVING AN INBOUND MESSAGE
----------------------------
How It Works

An inbound message is one that originates outside of Domino and Notes and
is destined for a Notes user.

MGATE looks for inbound messages in the \MGATE\INBOUND directory. This
directory is scanned periodically, based on the number of minutes
specified by MgateInterval. When an inbound message is found, MGATE
transforms it into a Domino mail message and passes it to the local Domino
mail router.

If Domino cannot deliver the message, MGATE returns it to the
sender's directory under \MGATE\USER.

Message files may have any name, as long as they have the extension .MSG.
Attachments files may have any name, as long as they have the extension
.ATT.

Within an inbound message file, the first character of each line is an
item code, which is one of:

	A - Attachment  (path name of attachment file, original file name)
	B - Body text line
	C - CC name
	D - Time/date
	O - Originator name
	R - Recipient (full email address)
	S - Subject
	T - To name

Following the item code on each line is the contents of the item. Items
can be repeated as many times as necessary and in any order.



TESTING AN INBOUND MESSAGE
--------------------------
To test the inbound portion of MGATE

1) Create a file named 0.MSG with the following contents

	D08-03-91 04:31:18 PM
		T<short name for user>
                R<full name of user in local Address book>
		OBob
		SHello to you too...
		BI am fine.
		BI hope you are well also.

2) Make sure the mail server for the recipient is the local test server.
(This ensures that the router will be able to deliver the mail.)

3) Copy the file to \MGATE\INBOUND.

4) Within the next 2 minutes, the message should be sent to the
designated Notes user.
