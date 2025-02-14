Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.

PROGRAM
-------
DBBACKUP - Create a backup file of a database,illustrate the practical usage of the Backup,
Media Recovery and Archiving functions of the Backup and RecoveryAPI.

PURPOSE
-------
This program is intended to illustrate the practical usage of the Backup,
Media Recovery and Archiving functions of the Backup and  Recovery API.
It is not intended to show exclusive use of these functions or the most
effective program logic when using these functions.  Since the logic of this
type of functionality is highly dependent on its specific intention, this
sample is not recommended for use with production databases.

** IMPORTANT NOTE ** Transactional system logging MUST be enabled in order
to implement the recovery of databases using the Backup and Recovery API.
Please refer to the Domino documentation for a complete description of
system logging.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)


FILES
-----
dbbackup.c   - Backup program.

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


RUNNING dbbackup
----------------
Dbbackup can be copied to and run from the Domino data directory or a full
path to the database and backup files can be provided to it via the input
arguments.


The command line of dbbackup is  -

      dbbackup <database filepath> <output filepath>

where:

      <database filepath> is the path of the Domino database to backup.

      <output filepath> is the path of the backup file created.


EXAMPLE:

Under Windows, type:

      dbbackup.EXE "C:\Program Files\HCL\Domino\Data\simple.nsf" "C:\Program Files\HCL\Domino\Data\simple.bak"

If the program runs correctly, it should display

      Program completed successfully, 'simple.bak' backup file created.


TO CHECK THE RESULT
-------------------
To see if 'dbbackup' worked correctly:

      Verify the creation of backup file at OS level.

SAMPLE OUTPUT
------------------------

int: 4
handle: 8
dhandle: 4
HFILE: 4
HFILE_ERROR = : -1

Program completed successfully, 'C:\Program Files\HCL\Domino\Data\simple.bak' backup file created.
