
(c) Copyright HCL Technologies   2018,   2025
All rights reserved.


PROGRAM
-------
 environment - Shows the os related functions such as 
	1.OSSetEnvironmentTIMEDATE-> Set a timedate into an environment variable.
	2.OSGetEnvironmentTIMEDATE-> Get a timedate from an environment variable.
	3.OSGetEnvironmentSeqNo -> Returns a sequence number that represents the number of times any environment variable has been changed.
				The sequence number changing means the notes.ini file has been changed. 
				If our variable is dynamic then we can also check if our variable has changed or not.
ENVIRONMENTS
------------
Windows (64-bit) 
Linux (64-bit) 

FILES
-----
environment.c    - Main program.

mswin64.mak     - Makefile for Windows 64-bit.

linux64.mak    - Makefile for Linux  64 bit.

readme.txt   - This file specifies the requirements for this example.


RUNNING  environment
-----------------------
- Under Windows, or Linux,
  type:
       environment
  at the system command prompt.


The program should print output that looks like this:

OSGetEnvironmentTimedate was successful for INI_TIME_TESTDATE.
Timedate value of INI_TIME_TESTDATE is 19-07-2021 11:26:00
The notes.ini file has been changed/modified.
The INI_TIME_TESTDATE environment variable has not been changed in notes.ini file.