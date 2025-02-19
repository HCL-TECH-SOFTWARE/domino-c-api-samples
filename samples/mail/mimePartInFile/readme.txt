Licensed Materials - Property of HCL

(c) Copyright HCL Technologies   2025
All rights reserved.



PROGRAM
-------
mimePartInFile - Sends MIME message part as file content
                 and uses NSFMimePartInFile to check the same.

ENVIRONMENTS
------------
Windows (64-bit)
Linux   (64-bit)

PURPOSE
-------
This program shows how to send the MIME messages(Multipurpose Internet Mail Extensions) part 
as a file content and uses NSFMimePartInFile to check message is in file content.

Note: This program does not output the content of the MIME message.


FILES
-----
readme.txt   	  - This file.

mimePartInFile.c  - Main C program.

mswin64.mak       - Make file for Windows 64-bit.

linux64.mak       - Make file for Linux 64-bit.

New.bmp		  - This is a sample bit map file to attach in the message.

RUNNING mimePartInFile
-------------------

At the command-line the syntax for this sample is:

Windows:
      mimePartInFile.exe  <Domino user name>

Linux:
      mimePartInFile <Domino user name>
	  
EXAMPLE
-------

     mimePartInFile.exe  "dadmin@sample.com"
 
CHECKING THE RESULT
--------------------    
mail path :[CN=DOMINO-SERVER/O=SAMPLE].
 Append Body 1.
 Append Body 2.

MIME part content is in file.
File Name from MIME part [$MIME_PART_0000EF2000004102]

 Noteid[2774]
Successfully created MIME EML File [test.eml].

Successfully completed!!
    
