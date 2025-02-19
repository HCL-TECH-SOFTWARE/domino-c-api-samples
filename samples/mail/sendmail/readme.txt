Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN
  
(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.



PROGRAM
--------
sendmail -  Send a Notes Mail Message using the Notes Mail Gateway API 
            functions.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
Linux   (64-bit)
IBM i

FILES
-----
readme.txt    - This file.

sendmail.c    - Main program.

sendmail.h    - Program defines, control IDs, and function prototypes.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

linux64.mak  - Make file for Linux 64-bit.

mk_sendmail_i.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being an IFS file.
     
mk_sendmail_p.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being in a source physical file.     


RUNNING sendmail
----------------
- Under Windows, Linux type:

       sendmail
           or
       sendmail "MailfileName" "SendTo" "CopyTo" "Subject" "Body"

  If the running command given is sendmail at the system command prompt.  The program will prompt for the
  following inputs.  For each input below, enter a string and
  press carriage return.
  
  SendTo <User Name to send Mail to, User Name to send Mail to, etc.>
  CopyTo <User Name to Copy to, User Name to Copy to, etc.>
  Subject <Mail Message Subject Name>
  Body    <Body of Mail Message - Maximum length 1024 characters>
  MailfileName  <Mail File name from which you want to send mail>

  Note: Enter a comma to separate multiple User names.

- Under IBM i type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/sendmail)
        parm('<User Name to send Mail to>' 'User Name to copy Mail to'
             '<Mail Message Subject Name>' '<Body of Mail Message>') 
        batch(*no)
  or
    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/sendmail))
        batch(*no)

  Note: For IBM i, you must provide the MailFile name in the notes.ini file, 
  for example 
        MailFile=mail/jdoe.nsf

EXAMPLE:
--------

- Under Windows,Linux, AIX type:
  
  sendmail "mail\dadmin.nsf" "domino admin/HCL" " " "This is a testing mail" "This is sample mail body"

SAMPLE OUTPUT:
--------------

Send To mailIDs:RHEL User1/RHEL
Copy To mailIDs:
Subject:This is a testing mail
Priority:N
Body:This is sample mail body
SendMail successful.
Mail file contains 1 message(s).

Message #1:
        Originator = 'CN=L2Support/O=RHEL'
        Number of Recipients = 2.
                Recipient 1 = 'RHEL User1/RHEL'  Domain = ''
                Recipient 2 = ''         Domain = ''
	To: RHEL User1/RHEL
        Cc:
        From: CN=L2Support/O=RHEL
        Date: 10/05/2020 11:43:57 AM
        Subject: This is a testing mail
        Body:
                This is sample mail body

Program completed successfully.

