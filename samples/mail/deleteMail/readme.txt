Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.



PROGRAM
-------
deleteMail- Get the text contents of all MIME messages in a NSF Database.

NOTE: This is only used for CAPI samples automation testing, not an API testing.

ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
Linux   (64-bit)

PURPOSE
-------
This program shows how to delete mails in the given mail NSF database.

This program first scans all the notes in the specified Database. Deletes those mails/docs using NSFDbDelete().


FILES
-----
readme.txt      - This file.

deleteMail.c    - Main C program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.
 
linux64.mak     - Make file for Linux 64-bit.


RUNNING deleteMail
-------------------

- If no server name provided as argument then program looks for MAILServer env in NOTES.INI.

- At the command-line the syntax for this sample is:

      deleteMail <server name> <mail DB File Name> 

      or 

      deleteMail <mail DB File Name> 

EXAMPLE
-------

     deleteMail  "mail\dadmin.nsf"
 
CHECKING THE RESULT
--------------------    
     
     After the sample returns successfully message along with list of noteid deleted.
