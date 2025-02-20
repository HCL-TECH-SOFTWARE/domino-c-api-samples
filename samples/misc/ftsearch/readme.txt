Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN
  
(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.



PROGRAM
-------
ftsearch - Sample HCL C API for Domino and Notes Program: shows how to 
index a single database for full text searching and how to perform full 
text searches.  


PURPOSE
-------       
This demonstrates the full text search routines of the HCL C API for 
Domino and Notes for a single database.  This indexes a single database 
for full text searching, performs several full text searches, prints out 
the note id's and the relevancy scores.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM i


FILES
-----
readme.txt  -  This file specifies what is needed to use this example.

ftsearch.c  -  Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

mk_ftsearch_i.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being an IFS file.
    
mk_ftsearch_p.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being in a source physical file.     


RUNNING ftsearch 
----------------
Copy the sample database, "ftsearch.nsf", to the Domino or Notes data
directory on your system.  

- Under Windows:
  type the following at the system command prompt:

    ftsearch <database> <query>

    where:
      <database> is the filename of the Domino database to be modified.
      (query) is the query string for the search.  The syntax of this 
      query string is described in Follow the syntax rules as described in 
      the "Searching for Information" section of Notes 5 Help.
 
      The query is likely to have spaces and therefore should be 
      enclosed in quotation marks.


EXAMPLE
-------

    ftsearch  ftsearch  "documentation"
- Under IBM i type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/ftsearch)
        parm(ftsearch 'documentation')) batch(*no)


NOTE: Because the database is in the data directory and has the 
default extension (.nsf) you don't have to type the directory or 
extension. If the database were in a different directory or had another 
extension you would type these.


CHECKING THE RESULTS
--------------------
Upon executing this command, ftsearch will first print out 
information about indexing the database, then the results of 
each of the following three searches:
. Search all the documents in the database for the variants of the 
  query word,
. Refine the query using the ID Table created from the first search,
. Search all the documents in a given view for the variants of the 
  query word.

The output will be similar to:


Database:  ftsearch.nsf
------------------------------------------------

New documents added to index: 16
Revised documents re-indexed: 0
Documents deleted from index: 0
12029 bytes indexed

Time of Last Index:  09/15/98 12:30:00 PM

This is a Single Database Search...
-------------------------------------
11 document(s) found on the query:  documentation

Note IDs        Relevancy Scores
--------        ----------------
2136              67
2132              67
210A              66
20FA              64
211A              64
211E              64
212A              63
2102              62
2106              62
2126              61
2112              60

1 document(s) found on the refined query
Note IDs        Relevancy Scores
--------        ----------------
210A             100


The following document(s) were found on the query of the view,
Main Topics Only
Note IDs        Relevancy Scores
--------        ----------------
2132             67
20FA             64
211A             64
2102             62
2126             61

5 document(s) found on the query of the view, Main Topics Only


