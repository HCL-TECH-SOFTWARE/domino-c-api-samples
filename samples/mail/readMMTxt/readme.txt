Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.



PROGRAM
-------
readMMTxt - Get the text contents of all MIME messages in a NSF Database.

ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
Linux   (64-bit)
IBM i

PURPOSE
-------
This program shows how to use the MIME functions to extract the text part from the MIME messages(Multipurpose 
Internet Mail Extensions) in a NSF database.

This program first scans all the notes in the specified Dabase. For those messages which have multi
MIME part, it extract the text parts of the messages, both the plain text and also the html text, and output them
to an output file. However,this programme doesn't scan all the part of the messages, and doesn't the expand the
attachments even they are plain text.


FILES
-----
readme.txt    - This file.

readMMTxt.c  - Main C program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

linux64.mak     - Make file for Linux 64-bit.

mk_readmtxt_i.clp     - Make file (CL Program) for IBM i using ILE C 
                       with the source code being an IFS file. 
                          
mk_readmtxt_p.clp     - Make file (CL Program) for IBM i using ILE C 
                       with the source code being in a source physical file. 

RUNNING readMMTxt
-------------------

- This sample provides a sample Database(MIMEMsg.nsf) in the notedata folder.
  If needed, copy it to you HCL Notes data directory.

- At the command-line the syntax for this sample is:

      readMMTxt 
         OR
      readMMTxt  <NSF DB>

EXAMPLE
-------

     readMMTxt  MIMEmsg.nsf
 
CHECKING THE RESULT
--------------------    
     
     After the sample returns successfully, there is an output in the console screen. 

Sample Output
--------------
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Note ID is: [8F6].
============================================================
This is a multipart message:

Header:
        [From : admin/test@PROD.DOMAIN.COM]
        [To : admin@cn.domain.com]
        [Subject : MIME message(plain text)]
        [CopyTo : (null)]
        [MIME_VERSION : 1.0]

Body:

Here is the plain text:
charset = US-ASCII
***********text begin ***************
[This is a test message.
]
***********text end   ***************

Here is the HTML text:
charset = US-ASCII
***********text begin ***************
[
<br><font size=2 face="sans-serif">This is a test </font><font size=2 color=blue face="sans-serif"><b>message.</b></font>
]
***********text end   ***************
============================================================


+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Note ID is: [8FA].
============================================================
This is a multipart message:

Header:
        [From : admin/TEST@PROD.DOMAIN.COM]
        [To : admin@cn.DOMAIN.com]
        [Subject : MIME message(with txt attachment)]
        [CopyTo : (null)]
        [MIME_VERSION : 1.0]

Body:

Here is the plain text:
charset = US-ASCII
***********text begin ***************
[This is a text message, with a txt file attached.


]
***********text end   ***************

Here is the HTML text:
charset = US-ASCII
***********text begin ***************
[
<br><font size=2 face="sans-serif">This is a text message, with a txt file
attached.</font>
<br>
<br>
]
***********text end   ***************
============================================================


+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
Note ID is: [8FE].
============================================================
This is a multipart message:

Header:
        [From : admin/TEST@PROD.DOMAIN.COM]
        [To : admin@cn.DOMAIN.com]
        [Subject : MIME message with attachmetn(Txt/BMP/Jpg) and inline images]
        [CopyTo : (null)]
        [MIME_VERSION : 1.0]

Body:

Here is the plain text:
charset = ISO-8859-1
***********text begin ***************
[You can get two types of Help in Notes:
Context-sensitive Help, specific to the task you're performing
Complete Help on all areas of the Notes product you're using
Context-sensitive Help is available in most areas of Notes including
dialog boxes and properties boxes. In this release of Notes, many parts of
your mail, Personal Address Book, and bookmarks (for example, many tabs in
dialog boxes and forms) include improved context-sensitive help. To see
it, press F1 (Macintosh users, Command-?), choose Help - Context Help, or
click the question mark button at the top right corner of any dialog box
or properties box.



To see all the Help available, choose Help - Help Topics from the main
menu. Once you open Help, you can use the Contents, the Index, or the
(full-text) Search to locate topics. You can also open Help for other
Notes products (for example, Help for IBM Lotus Domino Designer« 6 while
in Notes) by using the Actions menu in the Help window.



]
***********text end   ***************

Here is the HTML text:
charset = ISO-8859-1
***********text begin ***************
[
<br><font size=1 face="sans-serif">You can get two types of Help in Notes:</font>
<ul>
<li><font size=1 face="sans-serif">Context-sensitive Help, specific to
the task you're performing </font>
<li><font size=1 face="sans-serif">Complete Help on all areas of the Notes
product you're using</font></ul><font size=1 face="sans-serif">Context-sensitive
Help is available in most areas of Notes including dialog boxes and properties
boxes. In this release of Notes, many parts of your mail, Personal Address
Book, and bookmarks (for example, many tabs in dialog boxes and forms)
include improved context-sensitive help. To see it, press F1 (Macintosh
users, Command-?), choose Help - Context Help, or click the question mark
button at the top right corner of any dialog box or properties box.</font>
<p><img src=cid:_1_08ADF3C0017F482400322A9B482571C7>
<p>
<p><img src=cid:_1_08ADFA1408ADF7EC00322A9B482571C7>
<p><font size=1 face="sans-serif">To see all the Help available, choose
Help - Help Topics from the main menu. Once you open Help, you can use
the Contents, the Index, or the (full-text) Search to locate topics. You
can also open Help for other Notes products (for example, Help for IBM
Lotus Domino Designer« 6 while in Notes) by using the Actions menu in the
Help window. </font>
<p><img src=cid:_1_08AE01C408ADFF9C00322A9B482571C7>
<p>
<p>
]
***********text end   ***************
============================================================
Attached file item [Plain.txt] is a MIME part in [$FILE].
succeed return.
