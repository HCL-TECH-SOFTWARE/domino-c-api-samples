Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN
  
(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.



PROGRAM
-------
namebook  - List all Address books identified by the NAMES variable
            in the notes.ini file.


ENVIRONMENTS
------------
Windows 32-bit
Windows 64-bit
Linux   64-bit
IBM i

PURPOSE
-------
This program demonstrates the HCL C API for Domino and Notes function
NAMEGetAddressBooks().


FILES
-----
namebook.c      - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

linux64.mak     - Make file for Linux 64-bit.

readme.txt      - This file specifies what is needed to use this example.

mk_namebook_i.clp    - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_namebook_p.clp    - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file. 


RUNNING namebook
----------------
The syntax for this sample is:

        namebook  [options]


EXAMPLE
-------
Note:  A Server's Address book is called the Domino Directory.

Print the name list also including the titles of the Address
book databases by typing:

         namebook  TITLES

- Under IBM i:
  run:  rundomcmd server(<server>) cmd(call pgm(qntcsdk/namebook)
         parm('')) batch(*yes) alwmltthd(*yes)

SAMPLE OUTPUT
-------------
Entry 1
        Port = TCPIP
        Server = CN=DominoSever/O=RHEL
        File = names.nsf
        Title = 'RHEL's Directory'

