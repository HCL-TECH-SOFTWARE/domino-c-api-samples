Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN
  
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.



PROGRAM
-------
ftindexreq - Sample HCL C API for Domino and Notes Program: shows how to 
index a single database for full text indexing for remote DB and how to
validate its last indexed date time.  


PURPOSE
-------       
This demonstrates the full text indexing request routines of the HCL C API for 
Domino and Notes for a single database.  This indexes a single database 
for full text indexing for remote DB, checks its last indexed date and time.


ENVIRONMENTS
------------
Windows (64-bit)


FILES
-----
readme.txt  -  This file specifies what is needed to use this example.

ftindexreq.c  -  Main program.

mswin64.mak     - Make file for Windows 64-bit.



RUNNING ftindexreq 
------------------
Copy the sample database, "ftsearch.nsf", to the Domino data
directory on your system. 

- Under Windows:
  type the following at the system command prompt:

    ftsearch <ServerName!!database> 

    where:
      <database> is the filename of the Domino database to be modified.
 
EXAMPLE
-------

    ftindexreq  DominoServer!!ftsearch 


NOTE: Because the database is in the data directory and has the 
default extension (.nsf) you don't have to type the directory or 
extension. If the database were in a different directory or had another 
extension you would type these.


CHECKING THE RESULTS
--------------------


