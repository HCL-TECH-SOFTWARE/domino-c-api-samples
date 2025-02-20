Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN
  
(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
-------
idtables - Sample HCL C API for Domino and Notes Program:
	       Delete, Touch, Replace and Set Categories


PURPOSE
-------
This program shows how to use ID Tables to delete, touch, replace, archive, and
categorize documents in Domino databases.


ENVIRONMENT
-----------
Windows (32-bit)
Windows (64-bit)
IBM i


FILES
-----
readme.txt    - This file specifies what is needed to use this example.

idtables.c    - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

mk_idtables_i.clp    - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_idtables_p.clp    - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.  


RUNNING idtables
----------------
Copy the sample databases, "id.nsf" and "id_arc.nsf", to the Domino data
directory on your system.  From the Notes user interface, add the database's
icon to your Notes workspace.

- Under Windows:
  type the following at the system command prompt:


    idtables  -Sdb_name -action [-Bbegin (n) days ago]
		    [-Ncategory] [-Aarchive_name]

	action:
		T  touch selected document's last modified date.
		D  delete selected documents.
		C  stamp selected documents with a given category value.
		K  check the ID exists in the table
		F  Find the difference between two tables
		R  Replace destination table with source table

	options:
		B  begin document selection (n) days ago.
		N  new category string. Use with 'C' action.
		A  database to act as archive for deleted documents.


Use the -S parameter to specify the database. It should take the form:

-Sdb_name        - name of the Domino database to be modified


Specify only one of the three actions: -T (Touching documents),
-D (Delete documents) & -C (set a Category).

Touching documents, the -T option

	idtables -Sdb_name -T [-Bbegin days ago]


Deleting documents, the -D option

	idtables -Sdb_name -D [-Bbegin days ago] [-Aarchive_name]


Setting a category in a document, the -C option

	idtables -Sdb_name -C [-Bbegin days ago] [-Ncategory]

Checking ID is in the table
	idtables -Sdb_name -K
	
Checking the difference of two tables
	idtables -Sdb_name -F	

Replacing a table with another table
    idtables -Sdb_name -R

Creates new table with deleted NoteIds and removes Deleted NoteIDs from original table.
    idtables -Sdb_name -X


Specifying Options:

There are three optional parameters. They are -B (Begin selecting n
days ago), -A (Archive deleted documents to this database) and -N (New
category to set).

-Bbegin days ago - number of days back from today to start selecting
                   documents.  This option may be used on any of the
                   three actions. If not specified then all documents
                   are selected.

-Aarchive_name   - database to put all deleted documents into.

-Ncategory       - category to set in each selected document.


EXAMPLES
========

EXAMPLE Using the -T option
---------------------------

	idtables -Sid.nsf -T -B365

Touch the Modified Date & Time field in each document of the id.nsf
database that has been modified since 365 days ago.


Here is an example of what idtables.exe may display:
----------------------------------------------------
You have selected documents in id.nsf from:
	Begin date: 02/03/2001 01:02:42 PM EST through
	End date  : 02/03/2002 01:02:42 PM EST

ID Table Flags = 0001
ID Table size: 23
Number of entries in ID Table = 3 will be considered for touching.
Modified Time/Date = 02/03/2002 01:02:42 PM EST

Analysis of documents between begin & end:
	# of entries in ID Table  = 3
	# of deletion stubs      = 0
	# of touched documents   = 3

All Done!

Program completed successfully.
------------------------------------------

- Under IBM i:
  run:  rundomcmd server(<server>) cmd(call pgm(qntcsdk/idtables) 
           parm('-Sid.nsf''-T' '-B2125') ) batch(*no)     


CHECKING THE RESULTS
--------------------
Check to see if the displayed results from idtables.exe indicate that
any documents were touched (see the value on the line that reads "# of
Touched Documents = ").  If no documents were touched, try running
idtables.exe with no -B option.

In Notes, open id.nsf.  The Modified column in the All Docs view should
reflect the date and time for newly touched documents.



EXAMPLE Using the -C Option
---------------------------

	idtables -Sid.nsf -C -B1 -NEarth

Set the Category field to Earth of the documents of id.nsf that have
been modified in the past 1 day. Also set the Modified Date & Time
field in each selected document. If no field named Category exists, add
a new field called Category and set it to Earth.

Here is an example of what idtables.exe may display:
--------------------------------------------------
You have selected documents in id.nsf from:
	Begin date: 02/02/2002 01:13:48 PM EST through
	End date  : 02/03/2002 01:13:48 PM EST

ID Table Flags = 0001
ID Table size: 23
Number of entries in ID Table = 3 will be considered for stamping.

Analysis of documents between begin & end:
	# of entries in ID Table  = 3
	# of deletion stubs       = 0
	# of stamped documents    = 3

All Done!

Program completed successfully.
--------------------------------------------------


CHECKING THE RESULTS
--------------------
Check to see if the displayed results from idtables.exe indicate that any
documents were Stamped (see the line that reads "# of Stamped
documents = ").  If no documents were stamped, try running idtables.exe
with no -B option.

In Notes, open id.nsf.  The Category column of the All Docs view should
have the value "Earth", for the documents that have been stamped, since
that is the parameter we supplied on the command line.  In addition, the
Modified column in the All Docs view should reflect the date and time
these documents were stamped.



EXAMPLE Using The -D Option
---------------------------

	idtables -Sid.nsf -D -B23 -Aid_arc.nsf

Delete all the documents of the id.nsf database that have been modified in the
past 23 days. Copy the documents to be deleted into the id_arc.nsf database.


Here is an example of what idtables.exe may display:
----------------------------------------------
You have selected documents in id.nsf from:
	Begin date: 01/11/2002 01:18:19 PM EST through
	End date  : 02/03/2002 01:18:19 PM EST

ID Table Flags = 0001
ID Table size: 23
Number of entries in ID Table = 3 will be considered for deletion

Analysis of documents between begin & end:
	# of entries in ID Table  = 3
	# of entries in arc table = 3
	# of deletion stubs       = 0
	# of invalid documents    = 0

Archived 3 documents in id_arc.nsf

Deleted 3 documents in id.nsf

All Done!

Program completed successfully.
----------------------------------------------

CHECKING THE RESULTS
--------------------
Check to see if the displayed results from idtables.exe indicate that any
documents were deleted (see the line that reads "Deleted: xxx documents in
id.nsf").  If no documents were deleted, try running idtables.exe with no
-B option.

In Notes, open id.nsf.  There should be as many documents missing as are
indicated as having been deleted.

In Notes, open id_arc.nsf (the Archive database).  There should be as many
documents added as are indicated as having been archived.

After deleting records from a test database, it is a good idea to refresh the
database to its original state by using Notes to do a File - Database -
New Copy of the original database to the Domino data directory.  Likewise, to
rerun idtables.exe with the -A option again, it may be a good idea to delete
the documents in the archive database.


EXAMPLE Using The -F Option
---------------------------

	idtables -Sid.nsf -F
	
	checks the two tables are same or different.and says what operation (addition ,deletion)  would make both the table same.
	
Here is an example of what idtables.exe may display:
---------------------------------------------------
	
	You have selected documents in id.nsf from:
        Begin date: Beginning of Cosmos through
        End date  : 05/11/2021 05:31:59 PM GMT

	ID Table Flags = 0001
	ID Table size:  23
	Number of entries in ID Table = 3
	Testing Table differences

	There are additions to make table1 same as table2
	No of added entries  = 7
	Table difference test is successful.

	All Done!

Program completed successfully.

	
EXAMPLE Using The -K Option
---------------------------

	idtables -Sid.nsf -K 	
	
	checks whether the given id is present in the table and prints the position of the id in the table.

	Here is an example of what idtables.exe may display:
-------------------------------------------------------

You have selected documents in id.nsf from:
        Begin date: Beginning of Cosmos through
        End date  : 05/11/2021 05:31:32 PM GMT

	ID Table Flags = 0001
	ID Table size:  23
	Number of entries in ID Table = 3
	Testing Scan Back
	ID = 72

	Release 12.0|May 14, 2021

	Release 12.0|May 14, 2021
	ID present after 68
    
EXAMPLE Using The -R Option
---------------------------

	idtables -Sid.nsf -R 	
	
You have selected documents in id.nsf from:
        Begin date: Beginning of Cosmos through
        End date  : 04/06/2022 04:27:00 PM ZE5B

ID Table Flags = 0001
ID Table size:  23
Number of entries in ID Table = 3

Testing IDTableReplaceExt API
=================================

Before IDTableReplaceExt API called

Tables are not equal.


After IDTableReplaceExt API called

Tables are equal.


All Done!

Program completed successfully.


EXAMPLE Using The -X Option
---------------------------    
You have selected documents in id.nsf from:
        Begin date: Beginning of Cosmos through
        End date  : 02/04/2022 09:20:23 PM ZE5B

ID Table Flags = 0001
ID Table size:  23
Number of entries in ID Table = 3

Testing IDTableReplaceExt API
===========================
Number of entries in ID Table after creation = 0
Number of entries in ID Table after inserting a range of IDs = 10

Before IDTableReplaceExt API called

Tables are not equal.


After IDTableReplaceExt API called

Tables are equal.


Testing IDTableSplitDeleted API
===================================

Before Split
-------------

NoteID from Source Table 8110
NoteID from Source Table 9110
NoteID from Source Table 80008110
NoteID from Source Table 80009110

No. of entries in Source Table : 4


No. of entries in Deleted Table : 0

After Split
-------------

NoteID from Source Table 8110
NoteID from Source Table 9110

No. of entries in Source Table : 2

NoteID from Deleted Table 80008110
NoteID from Deleted Table 80009110

No. of entries in Deleted Table : 2

All Done!

Program completed successfully.

