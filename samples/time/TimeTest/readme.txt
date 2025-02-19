Licensed Materials - Property of HCL

(c) Copyright HCL Technologies   1996,   2025

All rights reserved.


PROGRAM
-------
TimeTest - Shows the basic time related functions such as
         1. TimeDateCollate -> Compares two binary TIMEDATE values.
         2. TimeExtractJulianDate -> Extracts the Julian date from a TIMEDATE value.
         3. TimeDateDifferenceFloat -> Floating-point difference between two TIMEDATE values.
         and prints their output to the screen.

ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
Linux (64-bit) 


FILES
-----
TimeTest.c    	- Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.
    
linux64.mak  	- Makefile for Linux  64 bit.

readme.txt	- This file specifies what is needed to use this example.


RUNNING TimeTest
-------------------
This sample program doesn't required any input file.
Domino/Notes should up and running.


- Under Windows, or UNIX 
  type:
       TimeTest
  at the system command prompt.


The program should print output that looks like this:

Test of Time functions TimeDateCollate, TimeExtractJulianDate, TimeDateDifferenceFloat
Get Now current timedate
Now time/date is 05-03-2021 11:30:22 AM
Get Future current timedate
Future time/date is 05-03-2021 11:31:22 AM
Date converted to Julian date using TimeExtractJulianDate is 2459279
Difference between Now and Future date using TimeDateDifferenceFloat is 60.000000
The TimeDateCollate function has Passed as 'Future' date is bigger than 'Now' date