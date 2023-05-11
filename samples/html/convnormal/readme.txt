Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2022
All rights reserved.


PROGRAM
-------
convertmormal - Example of convertting an attachment to be HTML format


PURPOSE
-------
Demonstrates building a program which can be used to convert some normal elements
(for example, text field information, section, table, and so on )whthin an nsf document, 
which can be used in HTML format.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM i
Linux 64bit


FILES
-----
convertmormal.c      - Main C program source file.
mswin32.mak     - Make file for Windows 32-bit.
mswin64.mak     - Make file for Windows 64-bit.
linux64.mak    - Makefile for Linux 64 bit.
mk_cnvnomal_i.clp    - Make file (CL Program) for IBM i using ILE C 
                       with the source code being an IFS file.    
mk_cnvnomal_p.clp    - Make file (CL Program) for IBM i using ILE C 
                       with the source code being in a source physical file.  
readme.txt           - This file specifies what is needed to use this example.


INSTALLING AND RUNNING convertmormal
------------------------------------

This sample program will run against the Domino database normal.nsf,
provided with the HCL C API for Domino and Notes toolkit.This database is 
located under the database directory. You must copy this file to the Domino 
or Notes data directory before running this program.

- Under Windows,at the system command prompt.
  type: convertmormal

- Under IBM i type:
        RUNDOMCMD SERVER(<server name>) CMD(CALL PGM(QNTCSDK/CNVNOMAL)) 
		BATCH(*YES) ALWMLTTHD(*YES)

- The program should print "Sucess to execute this sample" if the program execute successfully.
  Or if the program print "error occur, check the log file", it means some error occurs, you 
  should go to check the output log file to check the detailed error message.

- After the program execute successfully, you will get three new files under the program directory.
- Under IBM i type:
  After the program execute successfully, you will get three new files under 
  the /home/qnotes directory.                          

  1) output.log : this file will log the convert information of the test database. It will also
     log the error information. When you find the program can not execute successfully, turn to
     this file to get the detailed information. Besides, you can get very detailed information         about the converting result of the test database.
  2) output.html: this file is a html file, whose content is converted from the test database,
     you can open this file using your internet browser to verify the test result.
