Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
---------------
produce - Server Add-In program that produces events.
consume - Server Add-In program that consumes events.

PURPOSE
-------
These programs are simple examples that produce and consume user defined
server events.  The program, consume, creates an event queue in which to
receive events, and specifies the types of events that it will consume.
The program, produce, creates events of the appropriate type in the event
queue.

ENVIRONMENTS
------------
Windows 32-bit
Windows 64-bit
Linux 64-bit
IBM i


FILES
-----
consume.c     - The source file for the Event Consumer program.
consume.h     - The header file for the Event Consumer program.
consume.rc    - The resource file for the Event Consumer program.
produce.c     - The source file for the Event Producer program.
produce.h     - The header file for the Event Producer program.
produce.rc    - The resource file for the Event Producer program.
mswin32.mak     - Make file for Windows 32-bit.
mswin64.mak     - Make file for Windows 64-bit.
linux 64.mak   - Make fil for Linux 64-bit.
mk_events_i.clp  - Make file (CL Program) for IBM i using ILE C 
                       with the source code being an IFS file.                           
mk_events_p.clp - Make file (CL Program) for IBM i using ILE C 
                       with the source code being in a source physical file. 
readme.txt    - This file specifies what is needed to use this example.


RUNNING produce
----------------------

1) Make sure the Domino Data directory on the HCL Domino Server contains
the server log file (log.nsf). 

2) Start the HCL Domino Server.

The command line for produce and consume execution is  -

    produce

- Under IBM i, load the add-in tasks by typing:

        load producer
        load consumer


Note: This program should only be started from a HCL Domino Server.


CHECKING THE RESULTS
-----------------------------
Using the Notes Workstation software, display the latest entry
in the Domino Server Log database, and check that the above messages
appear.

SAMPLE OUTPUT
------------------------
[243C:0002-3F60] 06-10-2020 17:52:51   Event Producer Test: Initialization complete.
[4C7C:0002-2FC4] 06-10-2020 17:52:51   CONSUME Test: Initialization complete.
[243C:0002-3F60] 06-10-2020 17:52:57   PRODUCE Test: Produced an event!
[4C7C:0002-2FC4] 06-10-2020 17:52:57   CONSUME consumed an event at 06-10-2020 17:52:5706-10-2020 17:52:57
[243C:0002-3F60] 06-10-2020 17:53:07   PRODUCE Test: Produced an event!
[4C7C:0002-2FC4] 06-10-2020 17:53:07   CONSUME consumed an event at 06-10-2020 17:53:0706-10-2020 17:53:07
[243C:0002-3F60] 06-10-2020 17:53:17   PRODUCE Test: Produced an event!
[4C7C:0002-2FC4] 06-10-2020 17:53:17   CONSUME consumed an event at 06-10-2020 17:53:1706-10-2020 17:53:17
[243C:0002-3F60] 06-10-2020 17:53:27   PRODUCE Test: Produced an event!
[4C7C:0002-2FC4] 06-10-2020 17:53:27   CONSUME consumed an event at 06-10-2020 17:53:2706-10-2020 17:53:27
[243C:0002-3F60] 06-10-2020 17:53:37   PRODUCE Test: Produced an event!
[4C7C:0002-2FC4] 06-10-2020 17:53:37   CONSUME consumed an event at 06-10-2020 17:53:3706-10-2020 17:53:37
[4C7C:0002-2FC4] 06-10-2020 17:53:37   CONSUME Test: Termination complete.
[243C:0002-3F60] 06-10-2020 17:53:39   PRODUCE Test: Termination complete.

