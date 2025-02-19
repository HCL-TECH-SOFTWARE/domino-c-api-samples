Licensed Materials - Property of HCL
CIR9ZEN
CIRA0EN
CIRA2EN
CIRA1EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
---------------
nsf_dump

PURPOSE
--------------
Nsf_dump prints out the entire contents of a Domino database (NSF file) 
or a Compound Text (CD) file to the standard output using HCL C API 
for Domino and Notes functions. Nsf_dump recognizes almost every structure, 
value, and flag exposed by the API and prints everything it recognizes. 
Use this program to find out how Domino stores different kinds of data in 
Domino databases. This example also shows how to enumerate the contents of a 
Domino database, including database header data, note header data, all
classes of notes, and all types of data items. 


ENVIRONMENTS
------------------------
Windows (32-bit)
Windows (64-bit)
IBM AIX
Linux (64-bit)
MAC OS X
IBM i


FILES
--------
nsf_dump.c   - Main program.
dumpitem.c   - Routines that print the contents of one item.
dumpcd.c     - Routines that print contents of one CD record.
cdfile.c     - Routines that process the contents of a CD file.
agents.c     - Routines that dump Agent records.
navigate.c   - Routines that dump Navigator records.
mime.c       - Routines that dump MIME records.

nsf_dump.h   - Constants and function prototypes.

mswin32.mak     - Make file for Windows 32-bit.
mswin64.mak     - Make file for Windows 64-bit.

aix.mak      - Make file for AIX.

aix64.mak      - Make file for AIX64.

linux64.mak    - Makefile for Linux 64 bit.

mk_nsf_dump_i.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being an IFS file.
    
mk_nsf_dump_p.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being in a source physical file.    

macosx.mak   - Make file for Mac. 

readme.txt   - This file specifies, what is needed to use in this example.


RUNNING nsf_dump
---------------------------
- Under Windows, or UNIX (AIX and Linux),
  use the following syntax at the system command prompt:

  nsf_dump  <database name>  [options]

  where [options] =     
                      not given    - dump everything, non-data 
                                     notes first, then data notes.
                      -d           - dump just data notes.
                      -n           - dump just non-data notes.
                      -b           - dump binary data (like bitmaps).
                      -f <formula> - dump just data notes that match 
                                     the specified formula.
                      -c           - file is a CD file, not NSF.
                      -s           - file is a CD storage file (not NSF)
                      -o <file>    - dump output to specified file. 
                      -p           - dump profile information, (Environment 
                                     variable, PROFILE, must be included
                                     in your makefile)
                      -e           - dump deletion stubs only
                      -m           - leave TYPE_MIME_PART and TYPE_RCF822_TEXT 
                                     in native format
                      -l           - dump folder object items
                                
The database name is required and the default path is the Domino or Notes
data directory.

All other switches are optional. If the -c option is specified the default 
path for the CD file is the current directory.  If the -o option is not 
specified, this program prints the entire contents of the specified 
Domino database to the standard output.  It does not handle signals, which 
means it does not stop on Ctl+C or Ctl+BREAK.  Therefore, run this program 
on small databases or specify the -o option to send the output to a file.

nsf_dump will run against any Domino database. 

EXAMPLE
--------------
  nsf_dump simple

- Under IBM i type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/nsf_dump)
        parm(<database name>  [options])) batch(*no)

  For example,
    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/nsf_dump)
        parm(simple.nsf '-d')) batch(*no)

SAMPLE OUTPUT 
------------------------
nsf_dump simple -p


NSF_DUMP Utility

USAGE : nsf_dump <database name> [option]

option : -d Dump data Notes ONLY.
         -n Dump non-data Notes ONLY.
         -b Include binary data in output.
         -f <formula> Selects which data Notes to dump.
         -c File is a CD file (not NSF).
         -s File is a CD storage file (not NSF).
         -o <file name> Dump output to file name.
         -e Dump deletion stubs only.
         -m Leave TYPE_MIME_PART and TYPE_RCF822_TEXT in native format.
         -l Dump folder object items.
