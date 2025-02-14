Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN
  
(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.



PROGRAM
-------
sendAttMM - Sends a MIME message with an embedded bitmap using MIME functions


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
Linux   (64-bit)
IBM i


PURPOSE
-------
This program shows how to create a MIME(Multipurpose Internet Mail Extensions) 
format message with inline attachment and send it to a specified mail address.

This program first creates a new MEMO note in the mail.box on server, then calls
 the MIME functions to append all needed items (RFC822 header and MIME Body Parts)
to it. At last, the program adds a recipient to the message, this message will be 
routed to the recipient.

Please refer to the User Guide database, Chapter 10-2
"Domino Mail Architecture", "mail.box" section for more information on 
programmatic access to the mail.box file.  Please refer to the Domino 
Administration Help for more information regarding multiple mail box files.



FILES
-----
readme.txt    - This file.

sendAttMM.c   - Main C program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

linux64.mak     - Make file for Linux 64-bit.

mk_sndattmm_i.clp    - Make file (CL Program) for IBM i using ILE C 
                       with the source code being an IFS file. 
                          
mk_sndattmm_p.clp    - Make file (CL Program) for IBM i using ILE C 
                       with the source code being in a source physical file.   

RUNNING sendAttMM
-----------------

- Before running the sample in Linux you need to copy "New.bmp" file from notedata folder in CAPIToolkit to domino server /local/notesdata.

- Server name is fetched from the NOTES.INI file.

- At the command-line the syntax for this sample is:

      sendAttMM  <recipient>[Optional]

Only one recipient name may be specified.

- Under IBM i type:
      RUNDOMCMD SERVER(<server name>) CMD(CALL PGM(QNTCSDK/SENDATTMM)) 
	  BATCH(*NO)                           

EXAMPLE
-------

     sendAttMM  "domino admin/HCL"
                                                   
- Under IBM i type:

      RUNDOMCMD SERVER(<server name>) CMD(CALL PGM(QNTCSDK/SENDATTMM) 
          PARM(ADMIN@CN.IBM.COM)) BATCH(*NO)                                                       
 
CHECKING THE RESULT
--------------------    

mail path :[CN=LotusNotes/O=CENTOS].
 Append Body 1.
 Append Body 2.
 Append Body 3.
 Append Body 4.
 Append Body 5.
 Append Body 6.
 Append Body 7.
successfully deposited memo
Mail file contains 1 message(s).

Message #1:
        Originator = 'Dizzy Dean <ddean@cardinals.com>'
        Number of Recipients = 1.
                Recipient 1 = 'Centos User1/CENTOS'      Domain = ''
        To: administrator <admin@cn.ibm.com>
        Cc:
        From: Dizzy Dean <ddean@cardinals.com>
        Date: 06/02/2006 12:21:37 PM
        Subject: Test message: mp/alt with text/plain, text/html

Program completed successfully.

