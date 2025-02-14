Licensed Materials - Property of HCL
CIRA0EN
CIRA2EN
CIRA1EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
addnames, index, mixed_1, mixed_2 - Notes Performance Test Suite


PURPOSE
-------
This directory contains a suite of programs that test the performance of 
Domino and Notes.  Some of the programs measure raw data I/O speed, while other 
programs test indexing performance.  By using these programs as a set, or 
individually, you can accurately test (and compare) the performance of 
Domino and Notes on various hardware and software platforms. 

All of the tests operate on a Domino Address book.  This convention was 
chosen because an Address book is a typical Domino database, and is familiar
to all users.


OVERVIEW OF THE TESTS
---------------------

ADDNAMES -- Adds unique names to a N&A book. You control how many 
names are added. This test measures the raw data I/O speed to Notes. 

INDEX -- Performs one update of a specified index in a specified
database. This test measures the raw indexing speed of Notes.

MIXED_1 -- Performs any number of "transactions" on a N&A book. For n 
transactions, the test performs 1/3 n additions of new records, 1/3 n 
modifications to existing records, and 1/3 n deletions of whole records.

The program uses optimal indexing -- the index is only updated twice in 
the entire test run. The test, therefore, measures a combination of 
data I/O, index usage and a few index updates. 

MIXED_2 -- Performs any number of transactions on a N&A book, just like 
the test above, but it uses "worst case" indexing. The index is updated 
before every transaction. The test, therefore, measures primarily the 
speed of many index updates, where each update has only one change.


ENVIRONMENTS
------------
IBM AIX
Linux 64 bit
IBM i
Windows 32-bit
Windows 64-bit


FILES
-----
readme.txt   - This file specifies what is needed to use this example.

addnames.c   - Main program.
index.c      - Main program.
mixed_1.c    - Main program.
mixed_2.c    - Main program.

perform.cmd  - Runtime command file for OS/2 (32-bit).

aix.mak          - Make file for IBM AIX.

aix64.mak        - Make file for IBM AIX64.

mswin32.mak  - Make file for Windows-32 bit.

mswin64.mak  - Make file for Windows-64 bit.

linux64.mak    - Makefile for Linux 64 bit.

perform.sh   - Shell script for all UNIX platforms to run the test.

mk_perform_i.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_perform_p.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.  

macosx.mak   - Make file for Mac.   


RUNNING THE TESTS
-----------------

- Under UNIX (AIX, Linux ):

  The best way to run the tests is to use the perform.sh Shell script, 
  or a variation of it.  This script always starts with a fresh N&A 
  book, so test results are reproducible.  It then runs all the tests 
  in a coherent order and reports the time taken by each test.

  Note:  perform.sh contain explicit paths for the target and destination 
         locations of na_test.nsf.  If these locations are different on 
         your system, you will need to modify these command files 
         accordingly.

        *Since the perform.sh shell script carries out operating system
	 level copies to the server's data directory, it is recommended
	 that this sample be run while the Domino server is NOT running.
	 If this sample is run multiple times while the server is also
	 running, a caching issue may result on the server.

  Type:

    sh perform.sh

      or

    to save the results of a test run, redirect the output of the perform.sh
    to a log file as follows:

    sh perform.sh > perform.log
- Under IBM i type:
    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/addnames)
        parm(na_test.nsf '200')) batch(*no)
    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/index)
        parm(na_test.nsf Contacts)) batch(*no)
    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/mixed_1)
        parm(na_test.nsf '300')) batch(*no)
    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/mixed_2)
        parm(na_test.nsf '300')) batch(*no)

-Under Windows at the system command prompt:
  Type:

	perform
 
 If the program runs correctly, it should display a message with the following similar output:


Loading database with initial set of names...

[11:08:29.48]ADDNAMES  NA_TEST  50

***** ADDNAMES Test Begins ******
It took 0 seconds to add 50 names


Indexing the new names...

[11:08:29.60]INDEX  NA_TEST  Contacts

***** INDEX Test Begins ******
It took 0 seconds to index


Performing mixed transactions with optimal indexing...

[11:08:29.73]MIXED_1  NA_TEST  40

***** MIXED_1 Test Begins ******
It took 0 seconds to add 13 names with no index usage at all
It took 0 seconds to index Contacts view
It took 0 seconds to modify 13 records
It took 0 seconds to rebuild the index
It took 0 seconds to delete 13 records


Performing mixed transactions with worst-case indexing...

[11:08:29.89]MIXED_2  NA_TEST  40

***** MIXED_2 Test Begins ******
It took 0 seconds to add 13 names with no index usage at all
It took 0 seconds to index Contacts view
It took 0 seconds to modify 13 records
It took 0 seconds to delete 13 records



[11:08:30.26]echo Done
Done

