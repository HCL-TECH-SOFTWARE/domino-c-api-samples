Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
-------
find_dbs - HCL C API for Domino and Notes Sample Program: List all Domino 
           Databases in a directory.  Also lists sub directories.


PURPOSE
-------
This program finds all databases and subdirectories contained in a 
specified directory, and prints out the summary information for each 
database. 


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM i
Linux 64 bit


FILES
-----
readme.txt     - This file specifies what is needed to use this example.

find_dbs.c     - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

linux64.mak    - Makefile for Linux 64 bit.

mk_find_dbs_i.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being an IFS file.
    
mk_find_dbs_p.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being in a source physical file.     


RUNNING find_dbs
----------------
- Under Windows:
  type the following at the system command prompt:

  find_dbs <server> <directory>

- Under IBM i:
  run:  rundomcmd server(<server>) cmd(call pgm(qntcsdk/find_dbs)
	        parm('' <data directory>)) batch(*no)

where:

  <server> is the name of the HCL Domino Server to be searched.
  To specify that the subdirectory to be searched is on the local machine,
  type two double quotes ("") in place of a server name.

  <directory> is the subdirectory to be searched.
  To specify that the Domino or Notes default data directory is to be searched,
  type two double quotes ("") in place of a directory name. 


EXAMPLE
-------

  find_dbs server1 notedata

Find_dbs finds all databases and subdirectories of the "notedata" 
directory on the HCL Domino Server "server1", and prints the summary 
information for each.


SAMPLE OUTPUT
------------------------

$TITLE:  Data
$Path:  notedata\Data
$Type:  $DIR
$Modified:  28-09-2020 11:22:53
$Length:  -1.7806e+33
$Properties:  Directory
$LinkInfo:
$PHYSICALPATH:  C:\Program Files\HCL\Domino\Data\notedata\Data

Program completed successfully.
