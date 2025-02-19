
(c) Copyright HCL Technologies   2018,   2025
All rights reserved.


PROGRAM
-------------
nsfquery.c - To test the NSFQueryDBExt2 API.

PURPOSE       
-------------   
This program writes the queries to a file, extracts the query from 
the file to query the database using NSFQueryDBExt2 and then deletes the file.

NSFQueryDBExt2 runs Domino Query Language (DQL) queries.  Its inputs are a 
database handle, the query text, length of query text, and a set of flags. 
Returns explanation of output and extra error output. The query text must be translated 
into LMBCS prior to execution.

ENVIRONMENTS
--------------------
Windows (64-bit)
Linux 64 bit

FILES
-----
nsfquery.c  - Main program.

mswin64.mak     - Makefile for Windows 64-bit.

linux64.mak    - Makefile for Linux 64 bit.

readme.txt    - This file specifies the requirements for this example.
          
RUNNING nsfquery
---------------------
This sample program can be run against the Domino database simple.nsf,
provided with the HCL C API for Domino and Notes toolkit.  Copy this file
to the Domino Data directory before running this program.

- Under Windows or UNIX (AIX, Linux),
  type the following at the system command prompt:

        nsfquery <database filename>
                 or
        nsfquery

  where:

  <database filename> is the filename of the Domino database.
  If no command line arguments are provided, the program will prompt for them.

EXAMPLE:
--------------
	nsfquery simple.nsf

Expected OUTPUT:
---------------------
Testing NSFQueryDBExt2:
-----------------------


Query Processed:
[PLAIN_TEXT = 'This is another line of simple text.']

0.PLAIN_TEXT = 'This is another line of simple text.' NSF document search estimated cost = 50
        Prep 0.99 msecs, Exec 3.150 msecs, ScannedDocs 3, Entries 0, FoundDocs 1

******************



Query Processed:
[NUMBER = 836]

0.NUMBER = 836 NSF document search estimated cost = 50
        Prep 0.26 msecs, Exec 0.200 msecs, ScannedDocs 3, Entries 0, FoundDocs 1

******************



Query Processed:
[TIME_DATE = @dt('2019-01-01T16:42:00')]

0.TIME_DATE = 2019-01-01T16:42:00.00Z NSF document search estimated cost = 50
        Prep 0.22 msecs, Exec 0.186 msecs, ScannedDocs 3, Entries 0, FoundDocs 0

******************

NSFQueryDBExt2 was successful.

Program completed successfully.