Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN
  
(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2023
All rights reserved.

PROGRAM
---------------
add_res

PURPOSE
--------------
This is a sample HCL Domino Resource-based server add-in program.

It creates a status line and sets its status, then performs some operations periodically
and writes that to a Database.

ENVIRONMENTS
------------------------
Windows (32-bit)
Windows (64-bit)
Linux (64-bit)
IBM i

FILES
--------
add_res.c    - Main program.

add_res.h    - A local include file, used by both the compiler and the
                        resource compiler.

add_res.rc    - The control file for the Windows resource compiler.

mswin32.mak    - Make file for Windows 32-bit.

mswin64.mak    - Make file for Windows 64-bit.

linux64.mak    - Make file for Linux 64-bit.

mk_addres_i.clp    - Make file (CL Program) for IBM i using ILE C 
                                 with the source code being an IFS file. 
                          
mk_addres_p.clp    - Make file (CL Program) for IBM i using ILE C 
                                 with the source code being in a source physical file. 
                       
readme.txt    - This file specifies what is needed to use this example.

OPERATING ENVIRONMENT REQUIREMENTS
----------------------------------
The file addintst.nsf, the database in which add_res will create notes,
must reside in the Domino data directory.

RUNNING add_res
---------------
1) Copy the executable file for the add-in task to the Domino Program
directory.
- Under IBM i, add the Symbolic Link to the executable file:
    ADDLNK OBJ('/QSYS.LIB/QNTCSDK.LIB/ADD_RES.PGM')            
        NEWLNK('/QIBM/USERDATA/LOTUS/NOTES/ADD_RES.PGM')       
        LNKTYPE(*SYMBOLIC) 
		
2) If the Notes workstation software is running, exit from it.

3) Start the HCL Domino Server software.

4) Wait until all server tasks have started - about 30 seconds.

5) Press ENTER to get the server console prompt, >.

6) Load the add-in task by typing

        load add_res

7) The task should immediately print "Add-in Test: Initialization
Complete" to the server console and also to the Domino and Notes log.

8) Confirm that the add-in task is running by typing "show tasks" at the
server console.  You should see the "Add-in Test" task displayed.

EXAMPLE
--------------
To run add_res automatically under Windows, Linux  64-bit

           add_res auto

- Under IBM i, add the Symbolic Link to the executable file:
    ADDLNK OBJ('/QSYS.LIB/QNTCSDK.LIB/ADD_RES.PGM')            
        NEWLNK('/QIBM/USERDATA/LOTUS/NOTES/ADD_RES.PGM')       
        LNKTYPE(*SYMBOLIC) 
		

SAMPLE OUTPUT
-------------------------
[15B4:0002-58DC] 23-10-2020 12:41:25   Add-in Test: Initialization complete.
[15B4:0002-58DC] 23-10-2020 12:41:29   Add-in Test: 20-second job complete.
[15B4:0002-58DC] 23-10-2020 12:41:50   Add-in Test: 20-second job complete.
[15B4:0002-58DC] 23-10-2020 12:42:09   Add-in Test: 20-second job complete.
[15B4:0002-58DC] 23-10-2020 12:42:09   Add-in Test: Termination complete.

