Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.



PROGRAM
-------
parseMMMsg - Parse the structure of all MIME messages in a NSF Database.

ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
Linux   (64-bit)
IBM i

PURPOSE
-------
This program shows how to parse the MIME messages(Multipurpose Internet Mail Extensions) 
in a NSF database.

This program first scans all the notes in the specified Dabase. For those notes which has 
MIME part, it recursively scans all the MIME notes and output the strcuture of the notes to
an output file. 

Note: This programme does not output the content of the MIME message.


FILES
-----
readme.txt    - This file.

parseMMMsg.c  - Main C program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

linux64.mak     - Make file for Linux 64-bit.

mk_prsmmmsg_i.clp   - Make file (CL Program) for IBM i using ILE C 
                       with the source code being an IFS file. 
                          
mk_prsmmmsg_p.clp   - Make file (CL Program) for IBM i using ILE C 
                       with the source code being in a source physical file.   


RUNNING parseMMMsg
-------------------

- The sample provides a sample Database(MIMEMsg.nsf) in the notedata folder.
  If needed, Copy it to you HCL Notes data directory.

- At the command-line the syntax for this sample is:

      parseMMMsg  
         OR
      parseMMMsg  <DB Name>

- Under IBM i type:
      RUNDOMCMD SERVER(<server name>) CMD(CALL PGM(QNTCSDK/PARSEMMMSG)) 
	    BATCH(*NO)

	  or 
	  
      RUNDOMCMD SERVER(<server name>) CMD(CALL PGM(QNTCSDK/PARSEMMMSG) 
        PARM(<DB name>)) BATCH(*NO)
	  
EXAMPLE
-------

     parseMMMsg  MIMEmsg.nsf
 
- Under IBM i type:

      RUNDOMCMD SERVER(<server name>) CMD(CALL PGM(QNTCSDK/PARSEMMMSG) 
        PARM(MIMEmsg.nsf)) BATCH(*NO)
CHECKING THE RESULT
--------------------    
     
     - After the sample returns successfully, output can be seen in executed console.
     
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Note ID is: [8F6].
============================================================
This is a multipart message:

Header:
        [From : admin/ibm%RHEL%com]
        [To : admin@cn.ibm.com]
        [Subject : MIME message(plain text)]

Body:
Content-Type: mulipart/alternative
        Content-Type: text/plain
        Content-Type: text/html
============================================================


+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Note ID is: [8FA].
============================================================
This is a multipart message:

Header:
        [From : admin/ibm%RHEL%com]
        [To : admin@cn.ibm.com]
        [Subject : MIME message(with txt attachment)]

Body:
Content-Type: mulipart/mixed
        Content-Type: mulipart/alternative
                Content-Type: text/plain
                Content-Type: text/html
        Content-Type: text/plain
        ********Have an attachment :[New Text Document.txt]
============================================================
                    
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Note ID is: [8FE].
============================================================
This is a multipart message:

Header:
        [From : admin/ibm%RHEL%com]
        [To : admin@cn.ibm.com]
        [Subject : MIME message with attachmetn(Txt/BMP/Jpg) and inline images]

Body:
Content-Type: mulipart/mixed
        Content-Type: mulipart/related
                Content-Type: mulipart/alternative
                        Content-Type: text/plain
                        Content-Type: text/html
                Content-Type: image/gif
                Content-Type: image/gif
                Content-Type: image/gif
        Content-Type: text/plain
        ********Have an attachment :[Plain.txt]
        Content-Type: image/jpeg
        ********Have an attachment :[Sample.jpg]
        Content-Type: application/octet-stream
        ********Have an attachment :[sampleBMP.bmp]
        Content-Type: application/octet-stream
        ********Have an attachment :[mswin32.mak]
============================================================

 succeed return.
