Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN
  
(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
sendEasyMM - Sends an easy MIME message using MIME functions


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
Linux   (64-bit)
IBM i

PURPOSE
-------
This program shows how to create a MIME(Multipurpose Internet Mail Extensions) 
format message and send it to a specified mail address.

This program first creates a new MEMO note in the mail.box on server, then calls the MIME
functions to append the predefined buffers(including both the headers and the body part)
 to the note, at last it adds a recipient to the note. This memo created will be sent to 
the recipient mailbox by domino mail router.  

Please refer to the User Guide database, Chapter 10-2
"Domino Mail Architecture", "mail.box" section for more information on 
programmatic access to the mail.box file.  Please refer to the Domino 
Administration Help for more information regarding multiple mail box files.



FILES
-----
readme.txt    - This file.

sendEasyMM.c  - Main C program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

linux64.mak     - Make file for Linux 64-bit.

mk_sendezmm_i.clp    - Make file (CL Program) for IBM i using ILE C 
                       with the source code being an IFS file. 
                          
mk_sendezmm_p.clp    - Make file (CL Program) for IBM i using ILE C 
                       with the source code being in a source physical file.   

RUNNING sendEasyMM
-------------------


- At the command-line the syntax for this sample is:

      sendEasyMM  "SendTo"<optional>

- Under IBM i type:

      RUNDOMCMD SERVER(<server name>) CMD(CALL PGM(QNTCSDK/SENDEZMM)
        PARM("SendTo"<optional>)) BATCH(*NO) 

Only one recipient name may be specified.


EXAMPLE
-------

     sendEasyMM  "domino admin/HCL"

- Under IBM i type:

     RUNDOMCMD SERVER(<server name>) CMD(CALL PGM(QNTCSDK/SENDEZMM) 
       PARM(admin@cn.ibm.com)) BATCH(*NO)

CHECKING THE RESULT
--------------------    
mail path :[CN=Domino/O=CENTOS].
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

