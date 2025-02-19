Licensed Materials - Property of HCL
CIR9ZEN
CIRA0EN
CIRA2EN
CIRA1EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
-------
itemTime - Read Item Values and Print the values and Comparision results.


PURPOSE:      
-------       
This program is an introductory HCL C API for Domino and Notes program.
IT reads itemvalue with data type of number and datetime in all the noteid
present in the DB. and print the values. also it compares the item values with
already defined values and print the result.



ENVIRONMENTS
------------
Windows (64-bit)
Linux  64 bit

FILES
-----
itemTime.c    - Main program.

mswin64.mak     - Make file for Windows 64-bit.

linux64.mak    - Makefile for Linux 64 bit.

readme.txt   - This file specifies what is needed to use this example.


RUNNING itemTime:
----------------
This sample program is designed to run against the Domino database, 
simple.nsf, provided with the HCL C API for Domino and Notes toolkit.  
You must copy this file to the Domino or Notes data directory before
running this program.


- Under Windows, or UNIX (Linux)
  type the following at the system command prompt:
  
        itemTime simple.nsf
               or
        itemTime


EXAMPLE:

      itemTime  simple.nsf

If the program runs correctly, it should display something similar to the
following:

----------------------------------------------------------------------------

Note ID is: 8F6.
NUMBER field is: 1
NUMBER field comparision Success with Positive result.
TIME_DATE field is: 01/15/1988
Item timedate 01/15/1988 is older.

Note ID is: 8FA.
NUMBER field is: 837
NUMBER field comparision Success with Negative result.
TIME_DATE field is: 07/04/1953 04:42:00 PM
Item timedate 07/04/1953 04:42:00 PM is older.

Note ID is: 8FE.
NUMBER field is: 126
NUMBER field comparision Success with Negative result.
TIME_DATE field is: 07/15/1993 12:44:44 AM
Item timedate 07/15/1993 12:44:44 AM is older.

Note ID is: 90A.
NUMBER field is: 1
NUMBER field comparision Success with Positive result.
TIME_DATE field not found.

Program completed successfully.
-------------------------------------------------------------------------------


