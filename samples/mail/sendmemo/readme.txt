Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN
  
(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
sendmemo - Sends a Domino memo using NSF functions


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
Linux   (64-bit)
IBM i

PURPOSE
-------
This program shows how to create and send a simple memo using only the NSF 
family of API functions.  Alternative ways to access mail include the 
Domino Mail Gateway family of functions and VIM.  

This program sets the SendTo and the Recipients fields of the memo 
to the SendTo value specified.  It also sets the Subject and Body according
to the command line.  The Composed Date is set to the current date and time,
and the Form field is set to "Memo".  The Delivery Priority is set to Normal,
the Delivery Report is set to basic, and the Return Receipt is set to No.
The SentTo value is not verified in the Address book. The memo is then placed
in the "mail.box" file on the mail server specified in the NOTES.INI file.  
If no mail server is specified, the memo is placed in the local "mail.box" 
file - checking for "mail2.box" first in the event of multiple mail.box files.

Please refer to the User Guide database, Chapter 10-2
"Domino Mail Architecture", "mail.box" section for more information on 
programmatic access to the mail.box file.  Please refer to the Domino 
Administration Help for more information regarding multiple mail box files.


FILES
-----
readme.txt  - This file.

sendmemo.c  - Main C program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

linux64.mak     - Make file for Linux 64-bit.

mk_sendmemo_i.clp    - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_sendmemo_p.clp    - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.  

RUNNING sendmemo
----------------
- At the command-line the syntax for this sample is:

      sendmemo  "SendTo"  "Subject"  "Body Text"

Only one recipient name may be specified, although this may be the name 
of a group.


EXAMPLE
-------

- Under Windows, UNIX(Linux, AIX),

  sendmemo "domino admin/HCL" "Automated mail: for testing" "This is sample memo body"

- Under IBM i:
  run:  rundomcmd server(<server>) cmd(call pgm(qntcsdk/sendmemo) 
         parm('admin as40016/apiqe' 'subject'
         'body text:How are you!') ) batch(*no)  

SAMPLE OUTPUT:
--------------
successfully deposited memo 'Automated mail: for testing' in 'CN=DominoServer/O=CENTOS!!mail.box'.
