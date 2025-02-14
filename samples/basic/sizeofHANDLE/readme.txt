Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN


(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
sizeofHANDLE - Introductory HCL C API for Domino and Notes program that returns
        length of some handles.


ENVIRONMENTS
------------
Windows 32-bit
IBM i

FILES
-----
sizeofHANDLE.c      - Main program.

mswin32.mak     - Make file for Windows 32-bit.

readme.txt   - This file specifies what is needed to use this example.

mk_sfhle_i.clp   - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_sfhle_p.clp   - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.   


RUNNING sizeofHANDLE:
-------------

The command line of intro is  -

    sizeofHANDLE

Under IBM i,
1. Compile the makefile (CLP):

   WRKMBRPDM FILE(QNTCSDK/sfhle)
 select the CLP "MK_sfhle" and apply option 14=Compile.
 
2. Compile the sample:

   CALL PGM(QNTCSDK/MK_sfhle)
           
3. Run the sample:
for local access type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/sfhle)) 
    batch(*no)

for remote access type:

    rundomcmd server(<server name>) cmd(call pgm(qntcsdk/sfhle)
    parm(valid_server_name)) batch(*no)

If the program runs correctly, it should display a message with the
the length of some handles.
