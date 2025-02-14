Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
tracker   -  HCL C API for Domino and Notes Sample Program: 
             Example NSF hook driver


ENVIRONMENTS
------------
Windows (32-bit)
IBM i


FILES
-----
tracker.c    - Main program.
tracker.h    - Constants and function prototypes.
reqnum.c     - Utility program.
mswin32.mak     - Make file for Windows 32-bit.
mswin32.def  - Windows 32-bit definition file for this example.
mk_tracker_i.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
mk_tracker_p.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.     
readme.txt   - This file specifies what is needed to use this example.


RUNNING tracker:
----------------
Tracker creates "reqnum.dat", "instance.dat" and multiple tracker.00x files, 
whose file path names are hard-coded via symbolic constants TRACE_FILENAME, 
REQNUM_FILENAME, and INSTANCE_FILENAME in tracker.h.  Under Windows, the files 
reside in c:\ by default. Under IBM i, the files reside in the data directory
of your server.

To handle multiple processes calling into tracker, multiple trace files are 
created and have the naming convention "tracker.00x" (for example 
instance 1 = tracker.001, instance 2 = tracker.002).  Maximum instances 
allowed in this sample is 99.  The file "instance.dat" contains the current 
instance number and you can erase this file so tracker will create a new 
one or overwrite an existing one starting from instance 1.


                -------------- Warning --------------

    If these defaults are invalid or inappropriate in the environment in
    which you will be testing tracker, then you must modify the file,
    tracker.h, and set the symbolic constants TRACE_FILENAME, 
    REQNUM_FILENAME, and INSTANCE_FILENAME as necessary.  Then rebuild 
    both the tracker program library and the reqnum utility program before 
    you start testing.  Then, adapt the following instructions accordingly.


To install this program:

    1.  Copy tracker.nsf and trashcan.nsf to the Notes data directory.
        These databases are provided in the "notedata" directory of
        the API toolkit.  Use a Notes client to add the icons for these 
        databases to the Notes desktop.

    2.  Quit Notes if it is running.

    3.  Copy the tracker library into the Notes program directory.

            Under Windows, this file is NTRACKER.DLL
		Under IBM i, the file is the service program LIBTRACKER in library QNTCSDK.
		Add the Symbolic Link to the service program:
  
      		ADDLNK OBJ('/QSYS.LIB/QNTCSDK.LIB/LIBTRACKER.SRVPGM')
 		            NEWLNK('/QIBM/USERDATA/LOTUS/NOTES/LIBTRACKER.SRVPGM')
    				LNKTYPE(*SYMBOLIC) 
        
    4.  Edit the notes.ini file, normally found in your Notes data 
        directory, and add the lines:

            NSF_HOOKS=TRACKER
            TRACKER_TARGET=TRACKER
            TRACKER_TRASHCAN=TRASHCAN


    5.  Run the reqnum utility program to create file reqnum.dat.
        The file reqnum.dat is a binary file that contains a NUMBER
        (a double) to use as the next service request number.

        type: 
        reqnum 
        at the system command prompt.
	- Under IBM i type:

    		rundomcmd server(<server name>) cmd(call pgm(qntcsdk/reqnum))
        		batch(*no)

        Reqnum will print nothing if it succeeds.  You may check that reqnum
        succeeded by verifying that file reqnum.dat exists.  Tracker requires
        that reqnum.dat exists before it allows new service requests to be
        created in the Service Request Tracking database.

To run this program:

    Tracker is an example NSF hook driver. After tracker is
    installed, Notes calls functions defined in Tracker automatically
    whenever the Notes system initializes, terminates, opens a note,
    updates a note, or categorizes a set of notes.

    To exercise tracker, start the Notes client.  Open the database,
    Service Requests (TRACKER). Create a "Service Request" document and 
    enter data into the "Requester", "Assigned to" and "Request type" fields.
    Leave the "Request Number" field empty.  Press <Escape> to Save and exit 
    the document.  Then, quit Notes and, remove the tracker 
    statements you had added to notes.ini.  This will remove tracker from 
    the Notes system.  You may now bring Notes back up.
    

    View the error trace files, tracker.001, tracker.002, etc. If tracker 
    ran, the trace file will exist.

    
CHECKING THE RESULTS
--------------------

    Tracker has four side effects. 

    (#1) Tracker keeps a log of all activity in the Service Requests
    database.  This log consists of a "Tracker Log" document in the 
    Service Requests database.  You may view this log in Notes by opening
    the database, changing to the "Tracker Logs" view, and opening any
    document you find there.

    (#2) Tracker provides a "unique sequential request number" service
    to the Service Request Tracking application.  Each time a Notes user
    saves a newly composed service request document, tracker assigns a
    request number to the "ReqNumber" field of the new document. You may
    view the tracker-assigned request number by opening any service
    request document composed while tracker was loaded.

    (#3) Tracker provides a "trash can" service to the Service Request
    Tracking application.  Whenever a user deletes a service request
    document from the Service Request tracking database, Tracker copies
    the document to a separate "trash can" database before the document
    is actually deleted by Notes.  This trash can help avoid loss of data
    through accidental deletion of documents.  You may check this by
    deleting a service request document from the Service Request tracking
    database, then opening the "Service Req Trash Can" database and
    verifying that the deleted document resides there.

    (#4) Tracker keeps a detailed "error trace" of everything it does
    in the ASCII files tracker.001, tracker.002, etc.  The trace files 
    record everything tracker did from initialization to termination.  It
    records every time Notes calls a tracker function, regardless of what
    database or note is being accessed.  View the error trace file using 
    any text editor, or by typing it to the screen.
