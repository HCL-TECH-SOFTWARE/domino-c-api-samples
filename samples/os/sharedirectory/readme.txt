
(c) Copyright HCL Technologies   2018,   2025
All rights reserved.


PROGRAM
-------
sharedirectory.c - To test OSGetSharedDataDirectory routine.

PURPOSE
-----------
This program prints the path of the shared directory of a multi-user Notes client.

ENVIRONMENTS
------------
Windows (32-bit)  

FILES
-----
sharedirectory.c    - Main program.

mswin32.mak     - Makefile for Windows 32-bit.

readme.txt   - This file specifies the requirements for this sample.


RUNNING  environment
-----------------------
Note:
-----
Before running this sample, install the Notes client as multi-user.


- Under Windows, 
  type the following at the system command prompt:
       sharedirectory

Expected OUTPUT:
------------------------
The shared data directory of the multi-user Notes client is "C:\ProgramData\IBM\Notes\Data\Shared".
Program completed successfully.
