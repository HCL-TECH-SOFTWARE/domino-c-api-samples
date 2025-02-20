Licensed Materials - Property of HCL

(c) Copyright HCL Technologies  2025
All rights reserved.


PROGRAM
-------
rfc3339totimedate - Sample shows the user to understand the usage of 
                    ConvertRFC3339DateToTIMEDATE which converts RFC3339 Date&Time 
		    to Notes TIMEDATE format.


ENVIRONMENTS
------------
windows (32-bit)
Windows (64-bit)
Linux 64bit


FILES
-----
readme.txt   - This file specifies how to use this example.

rfc3339totimedate.c   - The main program.

mswin64.mak     - Make file for Windows 64-bit.

mswin32.mak     - Make file for Windows 32-bit.

linux64.mak     - Make file for Linux 64-bit.


RUNNING rfc3339totimedate:
--------------------------
This sample uses the current date & time and formats it to an RFC-3339 formatted string then uses
ConvertRFC3339DateToTIMEDATE() API to convert the RFC-3339 formatted datetime string to Notes
TIMEDATE.
- Under Windows, type
  the following at the system command prompt:

  rfc3339totimedate.exe 
  
- Under Linux, type the following at system command prompt:
  rfc3339totimedate

EXAMPLE - output 
----------------

 RFC3339 date format [2022-04-19T12:20:56+05:30]
 Notes TIMEDATE Format [19-04-2022 12:20:56]

Program completed successfully.

UTC ZONE:


 RFC3339 date format [2022-11-03T20:01:15.16+05:30]
 Notes TIMEDATE Format [03-11-2022 20:01:15]
 RFC3339 date format [2022-11-03T20:01:15+05:30]
 Notes TIMEDATE Format [03-11-2022 20:01:15]

Program completed successfully.

