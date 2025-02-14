Licensed Materials - Property of HCL
CIR9ZEN
CIRA0EN
CIRA2EN
CIRA1EN
 
(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.



PROGRAM
-------
readmail    -  print the contents of a message (mail) file


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM AIX (32-bit)
IBM AIX (64-bit)
Linux (64 bit)
MAC OS X
IBM i


FILES
-----
readmail.c     - C source file for the program

mswin32.mak    - Make file for Windows 32-bit.

mswin64.mak    - Make file for Windows 64-bit.

linux64.mak    - Makefile for Linux 64 bit.

macosx.mak     - Make file for Mac.

aix.mak	       - Make file for AIX 32-bit.

aix64.mak      - Make file for AIX 64-bit.

mk_readmail_i.clp - Make file (CL Program) for IBM i using ILE C
                    with the source code being an IFS file.

mk_readmail_p.clp - Make file (CL Program) for IBM i using ILE C
                    with the source code being in a source physical file.

readme.txt     - This file. Documents READMAIL and provides example.


RUNNING readmail
----------------
  This sample program prints the entire contents of a Domino message file 
  to the screen.  Therefore, it might be desirable to test it on the small 
  message file hfinn.nsf included in the API toolkit. You must copy this 
  file to the Domino or Notes data directory before running this
  program.
   
   - Under Windows or UNIX (AIX, Linux ),
    type the following at the system command prompt:

	 readmail <serverName> <database>  
			  or
	 readmail
	 NOTE: Without args program will prompt for server name and DB name.

   - Under IBM i, type:

	rundomcmd server(<server name>) cmd(call pgm(qntcsdk/readmail)
	parm(server database)) batch(*no)

  where:

  <server> is the name of the Domino mail server 

  <database> is the filename of the Domino mail database

  If no command line arguments are typed, then you will be prompted
  to enter them.  If entering arguments at the prompts instead of at the
  command line, do not enclose your responses in quotation marks.


EXAMPLE:

    Under Windows, Unix(Linux, AIX),
    readmail  "dominoserver/IBM"  "hfinn"

    Under IBM i, type:

	rundomcmd server(<server name>) cmd(call pgm(qntcsdk/readmail)
	parm('' hfinn.nsf)) batch(*no)
    
    The empty quotes signify that the database resides on the local 
    machine in the Domino or Notes data directory.


CHECKING THE RESULT
--------------------        
    Note: All messages within a User's mail file will be returned 
	  (this includes inbox messages, sent messages, draft messages, 
	  etc.).  Only messages which were actually sent will have a 
	  recipient list.

    If you run the program against the hfinn.nsf database, it should 
    display something similar to the following:

    Mail file contains 1 message(s).

    Message #1: 
	    Originator = 'Martin Cox'
	    Number of Recipients = 1.
		    Recipient 1 = 'Charles Connell'  Domain = 'Lotus'
	    To: Charles Connell
	    Cc: Donna Amaral,Mark Yuska,Mark Wallace
	    From: Martin Cox
	    Date: 04/12/1992 04:31:17 PM
	    Subject: Re: APISUPP.NSF Stub attached
	    Body:
		    Attached please find the replica stub o
		    f the Notes API Support database 

		    (APISUPP.NSF).
	    Attachment 1 = 'APISUPP.NSF'

    Program completed successfully.
