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
readview   -  Read the specified view note, display some information about 
              the view.

ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM AIX
IBM i
Linux 64 bit
MAC OS X

FILES
-----
readview.c     - Main program.
readview.h     - Symbolic constants used by the main program.
               
mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

aix.mak          - Make file for IBM AIX.

aix64.mak        - Make file for IBM AIX64.


mk_readview_i.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being an IFS file.
    
mk_readview_p.clp - Make file (CL Program) for IBM i using ILE C 
                    with the source code being in a source physical file.     

linux.mak      - Make file for Linux.

linux64.mak    - Makefile for Linux 64 bit.

macosx.mak   - Make file for Mac.

readme.txt     - This file.


RUNNING readview
----------------
This program opens the Domino database readview.nsf. The API kit provides
this database in the notedata directory. You must copy this file to the 
Domino or Notes data directory before running readview.

- Under Windows, or UNIX (AIX, Linux ),
  type the following at the system command prompt:

       readview
- Under IBM i
  type:
    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/readview))
    batch(*no)
  
The program takes no arguments.


OUTPUT
------
The output of this program will look like the following (some details
may vary from one system to another):

View Title:  View A
Date/Time of last modification:  10/31/2001 03:00:17 PM
Time relative formulae DO NOT exist in this view.

Column 1 of 3
        Item Name:                  $0
        Title:                      "#"
        Formula:                    @DocNumber
        Sort Key:                   NO
        Category:                   NO
        Sort Order:                 Ascending
        Hidden:                     NO
        Response Column:            NO
        Hide Detail if Subtotaled:  NO
        Display Icon:               NO
        Resizable at Runtime:       resizable
        Resortable at Runtime:      NOT resortable

Column 2 of 3
        Item Name:                  Number
        Title:                      "Number Column"
        Sort Key:                   YES
        Category:                   NO
        Sort Order:                 Ascending
        Hidden:                     NO
        Response Column:            NO
        Hide Detail if Subtotaled:  NO
        Display Icon:               NO
        Resizable at Runtime:       resizable
        Resortable at Runtime:      NOT resortable

Column 3 of 3
        Item Name:                  Text
        Title:                      "Text Column"
        Sort Key:                   NO
        Category:                   NO
        Sort Order:                 Ascending
        Hidden:                     NO
        Response Column:            NO
        Hide Detail if Subtotaled:  NO
        Display Icon:               NO
        Resizable at Runtime:       resizable
        Resortable at Runtime:      NOT resortable


Selection View Formula: "SELECT @All"

Program completed successfully.
