
(c) Copyright HCL Technologies   2018,   2025
All rights reserved.


PROGRAM
-------------
addqueryargs.c - To test the NSFQueryDBAddArgs API.

PURPOSE       
-------------   
NSFQueryDBAddArgs builds a lists of values to input into a DQL query, specifying 
arguments through QUEP_ARGVAL structure. Returns MEMHANDLE which consists 
of this set of QUEP_ARGVALs that were built.

ENVIRONMENTS
--------------------
Windows (64-bit)
Linux 64 bit

FILES
-----
addqueryargs.c  - Main program.

mswin64.mak     - Makefile for Windows 64-bit.

linux64.mak    - Makefile for Linux 64 bit.

readme.txt    - This file specifies the requirements for this example.
          
RUNNING addqueryargs
---------------------

- Under Windows or Linux,
  type the following at the system command prompt:

        addqueryargs

Expected OUTPUT:
---------------------
Testing NSFQueryDBAddArgs:
--------------------------
Building argument list:
Value: 'This is another line of simple text.'   Length: 38
Value: 836      Length: 3
Value: @dt('2019-01-01T16:42:00')       Length: 26
NSFQueryDBAddArgs was successful.

Program completed successfully.