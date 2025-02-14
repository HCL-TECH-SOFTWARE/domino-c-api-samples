Licensed Materials - Property of HCL

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
GMT - convert time to GMT,extracting ticks , datetime, timezone and DST from current time and
      convertTIMEDATEtoRFC3339Date - converts Notes TIMEDATE to RFC339 TIMEDATE(e.g. 2022-05-03T06:48:54.92Z)

ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
Linux (64-bit) 


FILES
-----
GMT.c    		- Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.
    
linux64.mak  	- Makefile for Linux  64 bit.

readme.txt		- This file specifies what is needed to use this example.


RUNNING GMT
---------------
This sample program doesn't required any input file.
Domino/Notes should up and running.


- Under Windows, or UNIX 
  type:
       GMT
  at the system command prompt.


The program should print output that looks like this:

Test of TimeExtractDate
Calling OSCurrentTimeZone to get the time zone
Calling OSCurrentTIMEDATE to put something in tdNowTime
Calling TimeExtractTicks to get the Time part
Calling TimeExtractDate to get the Date part
TimeExtractDate returned with 0x65258693
Comparing OS DST to returned DST
Daylight Savings Times match - VERIFIED
Comparing OS Time Zone to returned Time Zone
Time Zones match - VERIFIED
Calling TimeConstruct with Date = 0x258693 Time = 0x18019d
Comparing original TIMEDATE to returned TIMEDATE
TIMEDATEs match - VERIFIED
hour : 9 minute : 52 second : 12
 hour : 4 minute : 22 second : 12
 hour : 9 minute : 52 second : 12
Test of ConvertTIMEDATEtoRFC3339Date
Notes TIMEDATE = 04-05-2022 12:12:08 PM
RFC339 TIMEDATE = 2022-05-04T06:42:08.67Z
Successfully converted Notes TIMEDATE to RFC339 TIMEDATE.
