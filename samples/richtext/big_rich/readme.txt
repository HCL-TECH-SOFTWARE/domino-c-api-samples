Licensed Materials - Property of HCL
CIR9ZEN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
big_rich - Reads ASCII text files into a Domino database.


PURPOSE
-------
Big_rich shows how to create rich text fields from ASCII text
files using Compound Text functions.  It reads all the ASCII .txt
files in the source directory and stores them in the rich text fields
of consecutive documents created in the destination database.  The
text files may be of any size up to 5 Megabytes.  The utility program
big_file creates ASCII text files of any size to use when testing
big_rich.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
Linux (64-bit)

FILES
-----
readme.txt   - This file specifies what is needed to use this example.

big_rich.c   - Main program.


mswin32.mak - Makefile for Windows 32-bit.

mswin64.mak - Makefile for Windows 64-bit.

linux64.mak - Makefile for Linux 64-bit.


RUNNING big_rich
----------------
The syntax for the utility program big_file is:

  big_file <file> <count>

where:

  <file> is the fully-qualified name of an output ASCII text file.
  <count> is the number of ASCII characters to be written to <file>.

The syntax for this big_rich is:

  big_rich <server> <database> <directory> <file> <count>

where:

  <server> is the HCL Domino Server where the <database> resides.
  You may specify "" for <server> to indicate the database is in the
  local data directory.

  <database> is the filename of a Domino database with a design similar
  to the example database "big_rich.nsf" included with the C API toolkit.
  The database must exist and you must have at least author access.

  <directory> is a path specification to a directory containing any number
  of ASCII files with a file extension of ".txt".  These ASCII files may
  be of arbitrary length.	
  
  <file> is the fully-qualified name of an output ASCII text file.
  <count> is the number of ASCII characters to be written to <file>.

Create one or more example text files.  You may use any ASCII text files
to test big_rich but the file name extension must be ".txt".  You may also
use the utility program "big_file", included with this sample, to create
ASCII text files of any size for testing purposes.

For example, to create a file containing 50K bytes of text, type:

In Windows
---------------
  big_file c:\temp\f50k.txt 51200

Then type:

  big_rich "" big_rich temp f50k.txt 51200

In Linux
----------
 big_file /temp/f50k.txt 51200

Then type:

  big_rich "" big_rich temp f50k.txt 51200


If successful, big_rich will print the following:

  big_rich: created note from file 'f50k.txt'.
  big_rich: successfully processed 1 files.


CHECKING THE RESULTS
--------------------
From Notes, open the target database and press <escape>, if necessary,
to see the main view.  Open the document with Subject = f50k.txt.

big_rich reads all the files ending in ".txt" found in the source
directory, and creates a document in the destination database
for each file.

The "From" field of each document gets the name of the user who
is running this program (obtained from the ID file).  The "Category"
field gets the directory name specified.  The "Body" field gets the
complete text from the file, converted to rich text format.  The
"Subject" field of each document gets the file name.

