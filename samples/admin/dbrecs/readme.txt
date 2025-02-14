Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------

DBRECS - Archive/check logs or recover/restore a logged database from a backup file.


PURPOSE
-------
This program is intended to illustrate the practical usage of the Media 
Recovery and Archiving functions of the Backup and Recovery API.
It is not intended to show exclusive use of these functions or the most
effective program logic when using these functions.  Since the logic of this
type of functionality is highly dependent on its specific intention, this
sample is not recommended for use with production databases.

** IMPORTANT NOTE ** Transactional system logging MUST be enabled in order
to implement the recovery of databases using the Recovery API.
Please refer to the Domino documentation for a complete description of
system logging.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)


FILES
-----
dbrecs.c     - Archive, check, recover, restore program.

win32io.c    - OS level input/output program.

win32io.h    - Header file containing symbolic constants and function prototypes.

mswin32.mak  - Make file for Windows 32-bit version.

mswin64.mak  - Make file for Windows 64-bit version.

readme.txt   - This file specifies what is needed to use this example.


IDENTIFY A TEST DATABASE
------------------------
Determine the name of a Domino database file, such as 'simple.nsf', that
resides in the Domino data directory on your system. Dbbackup will work with
any valid Domino database file, but only backup files created from logged
databases will be recoverable.


RUNNING dbrecs
--------------
Dbrecs can be copied to and run from the Domino data directory or a full
path to the backup and database files can be provided to it via the input
arguments.  Running dbrecs will cause the creation of a directory called
'DBLogs' as a subdirectory to the location where the sample was run.

This new directory will provide the location for the log file 'dbrecs.log'
and any archived logs created by executing the program with the desired
options.

** IMPORTANT NOTE ** Great care should be taken to maintain "recoverable"
backup files (i.e. once a backup file has been recovered it is no longer
a "recoverable" file).  If a problem occurs after a database has been taken
offline and deleted, an existing recoverable backup file that was created
from that original DB will be the ONLY means available to restore the
original DB.

The command line of dbrecs is  -

      dbrecs >Domino ini path> <option> [input file] [restore db]

where:

      <option> the action to be performed:
               ARCHIVE - Archive system logs.   (Input file NOT REQUIRED)
               CHECK   - Check a database or backup file.    (Input file REQUIRED)
               RECOVER - Recover a backup file. (Input file REQUIRED)
               NOTE    - Recover a backup file and get Note info. (Input file REQUIRED)
               RESTORE - Restore a database.    (Input file & restore db REQUIRED)

      [input file] path to a backup file in the Domino data directory.
      [restore db] path to the database in the Domino data directory to be
                   restored.

*********************************************************************************
EXAMPLE (ARCHIVE):

Check to see if the Domino logger, that is configured to perform archive logging,
has any files to be archived. (** NOTE ** archive logging must be enabled to use
this option.  Please refer to the Domino documentation for a complete
description of system logging.)

Under Windows, type:

      dbrecs ARCHIVE

If the program runs correctly AND:

1) There are NO transactional log extents that have exceeded the
maximum log space (see Domino documentation for details), it should display

       Checking for logs to archive ...

       No transaction log files are waiting to be archived.

      Program completed successfully.

OR:

2) There ARE transactional log extents that have exceeded the
maximum log space (see Domino documentation for details), it should display

       Checking for logs to archive ...
        Archived Logid: 7806550 -2061146175 -1798160901 965 Extent 0,
	DBLogs\118047a9\S0000000.LOG created.

       No transaction log files are waiting to be archived.

      Program completed successfully.

(*NOTE: Archived Logid, Extent, and Log file created will most likely have
different values from the above example output.)


Results:

If a transaction log extent was returned as 'ready to be archived', an archive
copy was created, in the directory specified in the sample code.

*********************************************************************************
EXAMPLE (CHECK):
Determine the state of the database or database backup file with respect to the
current log state.

Under Windows, type:

      dbrecs CHECK "c:\lotus\domino\data\simple.bak"

If the program runs correctly, it should display

       Checking file to see if new backup is needed ...

        Transactional logging type is 'ARCHIVE'.
	
        Backup file c:\lotus\notes\data\simple.bak checked : Span of log is 3

      Program completed successfully.


Results:

The span of log needed for recovery is returned (see C API Reference database
for details).

*********************************************************************************
EXAMPLE (RECOVER):

Recover a valid backup file from the transactional system logs and refresh the
DBIID value.

If the RECOVER, NOTE or RESTORE option is to be implemented, make sure a copy  of
a recoverable backup file, as created by running 'dbbackup' on a logged Domino
database, is located in the Domino data directory.  To best illustrate the
recovery functionality, modify the database that has been backed up, prior to
its recovery. Once the file is recovered, it will be brought 'up-to-date'
with the changes that were recorded as a result of the system logging.

Under Windows, copy a valid backup file (as obtained from running
dbbackup.exe) to the Domino data directory, type:

      dbrecs recover "c:\lotus\domino\data\simple.bak"

If the program runs correctly, it should display

       Recovering backup file ...

       Backup file c:\lotus\domino\data\simple.bak recovered.

      Program completed successfully.

Results:

All transactions recorded in the Domino transaction logs (additions, deletions and
modifications), since the creation of the database backup file via dbbackup, have
been applied to the database backup file (i.e. simple.bak).

At this point in time, a new backup of the database should now be created
as the recovery process invalidates it as a recoverable backup file.

Attempting to recover a backup file that has previously been recovered or
opened via Domino, will result in the following error:

       Recovering backup file c:\lotus\notes\data\simple.bak ...

       Recovery Manager: Recovery only supported for Backup Files.

      Error recovering backup file simple.bak

*********************************************************************************
EXAMPLE (NOTE):
* This option is the same as the 'RECOVER' option, except this option retrieves
information specific to the notes being recovered and writes this data to the
event log file "dbrecs.log" (See the "TO CHECK THE RESULT" section below.)

Recover a valid backup file from the transactional system logs, while retrieving
information specific to the notes being recovered, and refresh the DBIID value.

If the RECOVER, NOTE or RESTORE option is to be implemented, make sure a copy  of
a recoverable backup file, as created by running 'dbbackup' on a logged Domino
database, is located in the Domino data directory.  To best illustrate the
recovery functionality, modify the database that has been backed up, prior to
its recovery. Once the file is recovered, it will be brought 'up-to-date'
with the changes that were recorded as a result of the system logging.

Under Windows, copy a valid backup file (as obtained from running
dbbackup.exe) to the Domino data directory, type:

      dbrecs note "c:\lotus\notes\data\simple.bak"

If the program runs correctly, it should display

       Recovering backup file ...

       Backup file c:\lotus\notes\data\simple.bak recovered.

      Program completed successfully.

Results:

All transactions recorded in the Domino transaction logs (additions, deletions and
modifications), since the creation of the database backup file via dbbackup, have
been applied to the database backup file (i.e. simple.bak).

At this point in time, a new backup of the database should now be created
as the recovery process invalidates it as a recoverable backup file.


*********************************************************************************
EXAMPLE (RESTORE):
Recover a valid backup file and restore it to an existing database.

If the RECOVER, NOTE or RESTORE option is to be implemented, make sure a copy  of
a recoverable backup file, as created by running 'dbbackup' on a logged Domino
database, is located in the Domino data directory.  To best illustrate the
recovery functionality, modify the database that has been backed up, prior to
its recovery. Once the file is recovered, it will be brought 'up-to-date'
with the changes that were recorded as a result of the system logging.

Under Windows, copy a valid backup file (as obtained from running
dbbackup.exe) to the Domino data directory, type:

      dbrecs "=C:\lotus\domino\data\notes.ini" restore "c:\lotus\domino\data\simple.bak" "c:\lotus\domino\data\simple.nsf"

If the program runs correctly, it should display

       Taking database c:\lotus\domino\data\simple.nsf offline ...

       Recovering backup file ...

       Backup file c:\lotus\domino\data\simple.bak recovered.

       Restoring database c:\lotus\domino\data\simple.nsf
        from recovered backup file c:\lotus\domino\data\simple.bak ...

       Bringing database c:\lotus\domino\data\simple.nsf online ...

      Program completed successfully.


Results:

1) The Domino database (i.e. simple.nsf) was taken off line and deleted.

2) All transactions recorded in the Domino transaction logs (additions, deletions
and modifications), since the creation of the database backup file via dbbackup,
have been applied to the database backup file (i.e. simple.bak).

3) The recovered backup file was copied (restored) to the original Domino database.

4) The restored Domino database was brought back on line.

At this point in time, a new backup of the database should now be created
as the recovery process invalidates it as a recoverable backup file.

Attempting to recover a backup file that has previously been recovered or
opened via Domino, will result in the following error:

       Recovering backup file c:\lotus\notes\data\simple.bak ...

       Recovery Manager: Recovery only supported for Backup Files.

      Error recovering backup file simple.bak

*********************************************************************************


TO CHECK THE RESULT
-------------------
To see if 'dbrecs' worked correctly:

      As 'dbrecs' performs its actions it keeps a running record of events
      in a file called 'dbrecs.log'.  This file resides in a directory
      called 'DBLogs', created as a subdirectory to the location where the
      sample was run.  These log entries will describe the action performed,
      as well as any Domino errors, along with the date and time that the
      action occurred.


SAMPLE OUTPUT
------------------------
Taking database C:\Program Files\HCL\Domino\Data\test.nsf offline ...

 Recovering backup file ...


 Backup file C:\Program Files\HCL\Domino\Data\test.bak recovered.

 Restoring database C:\Program Files\HCL\Domino\Data\test.nsf
  from recovered backup file C:\Program Files\HCL\Domino\Data\test.bak ...

 Bringing database C:\Program Files\HCL\Domino\Data\test.nsf online ...


Program completed successfully.
