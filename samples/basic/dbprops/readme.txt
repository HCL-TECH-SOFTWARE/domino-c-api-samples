Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
dbprops.c - Get, Set and Reset some of the database properties available
            to the Notes C API.
					

            Note**
            This program does not illustrate how to access ALL of the
            database properties available to the C API.  While many of
            the current database properties are accessible to the C API,
            some are not.  Certain database properties may require that
            flag values be set as well as additional fields added to
            specific design notes. Since it would be impossible to
            document exactly how to access all the database properties
            that are available, this sample is only meant to lay the
            ground work in doing so by illustrating how to set some of
            the replication settings, some advanced database properties
	    and the database title.  Many of the database properties are
	    defined in the ICON_FLAGS section of 'stdnames.h'.


ENVIRONMENTS
------------
Windows 32-bit
Windows 64-bit
linux 64-bit
IBM i


FILES
-----
dbprops.c   - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

linux64.mak	- Make file for Linux 64-bit.

readme.txt   - This file specifies what is needed to use this example.

mk_dbprops_i.clp    - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_dbprops_p.clp    - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.  

RUNNING dbprops:
-----------------
- This program will "Get", "Set" and "Reset" some of the database properties
  available to the Notes C API.  This program opens the Domino database
  simple.nsf. The C API toolkit provides this database in the notedata
  directory. You must copy this file to the Domino or Notes data directory
  before running dbprops.

- When the Get option is chosen, this sample will display the current database
  title, the replication ID, the replication flags, the replication priority,
  the replication cut-off interval, the replication cut-off date and the
  advanced database properties settings.	
					
  When the Set or Reset options are chosen, this sample will modify the
  database title, some advanced database properties, some replication flags,
  the replication cut-off interval and the replication priority settings.					


- After the program is successfully compiled, at the system prompt type:

    dbprops get


If the program runs correctly, it should display output similar to
the following:


 *** CURRENT database settings ***

  The current title for database, "simple.nsf", is:

   "Simple Data Test"


  The following replication properties are currently set:

   Database Replica ID    = 0x852566b9 - 0x0055ab21
   Database Flags         = IGNORE_DELETES
   Database Rep Priority  = Low
   DB Rep Cutoff Interval = 90 Days
   DB Rep Cutoff Date     = 01/17/2002 11:52:10 AM


  There are no DESIGN_FLAGS currently set in the ICON note.


 Program completed successfully.

*Note the values for the current database title, Database Flags, Database Rep
Priority and DB Rep Cutoff Interval.  Also notice that no DESIGN_FLAGS are
currently set.

- Now type:

     dbprops set


If the program runs correctly, it should display output similar to
the following:


 *** MODIFIED database settings ***

  The current title for database, "simple.nsf", is:

   "Database Properties Test"


  The following replication properties are currently set:

   Database Replica ID    = 0x852566b9 - 0x0055ab21
   Database Flags         = CUTOFF_DELETE
   Database Rep Priority  = High
   DB Rep Cutoff Interval = 50 Days
   DB Rep Cutoff Date     = 01/17/2002 11:52:10 AM


  The following database properties are currently set:

   "Don't maintain unread marks"
   "Document table bitmap optimization"
   "Maintain last accessed property"
   "Allow soft deletions"
   "Don't support specialized response hierarchy"
   "Allow more fields in database"


 Program completed successfully.

*Note the values for the current database title, Database Flags, Database Rep
Priority and DB Rep Cutoff Interval have been modified.  Also notice that
several DESIGN_FLAGS have now been set.

- Now type:

     dbprops reset


If the program runs correctly, it should display output similar to
the following:


 *** MODIFIED database settings ***

  The current title for database, "simple.nsf", is:

   "Simple Data Test"


  The following replication properties are currently set:

   Database Replica ID    = 0x852566b9 - 0x0055ab21
   Database Flags         = IGNORE_DELETES
   Database Rep Priority  = Low
   DB Rep Cutoff Interval = 90 Days
   DB Rep Cutoff Date     = 01/17/2002 11:52:10 AM


  There are no DESIGN_FLAGS currently set in the ICON note.


 Program completed successfully.

*Note the values for the current database title, Database Flags, Database Rep
Priority and DB Rep Cutoff Interval have been reset back to their original
settings.  Also notice that the DESIGN_FLAGS have been cleared, as they were
originally.

The sample may be run with any of the options at any time, but it is
recommended that it be run with the "Reset" option last so as to insure
that the sample database simple.nsf has been restored to its initial
condition for use with other C API sample applications.

- Under IBM i:
  run:  rundomcmd server(<server>) cmd(call pgm(qntcsdk/dbprops)
         parm('simple.nsf' 'GET')) batch(*no)  
