Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN
  
(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.



PROGRAM
-------
dbdrive - Example of a Domino or Notes External Database Driver Program. 


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM i


FILES
-----
dbdrive.c    - Main program.

dbdrive.h    - Header file used by the main program.

mswin32.def  - Module definition file for Windows 32-bit.

mswin64.def  - Module definition file for Windows 64-bit.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

mk_dbdrive_i.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_dbdrive_p.clp - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.     

readme.txt   - This file specifies what is needed to use this example.

     
PURPOSE
-------
This program is an example of a database driver which allows data in
non-Domino databases to be accessed by the Domino Designer functions,
@DbLookup and @DbColumn. It is implemented as a Dynamic Link Library
or shared object.

This database driver operates on databases consisting of one or more
lines separated by carriage return/newlines.  Each line consists of two
strings, separated by a comma. The database is sorted alphabetically by
the first string on the line.

@DbColumn: The database driver's @DbColumn function takes as input the
       name of a database from which to read.  The function reads
       each line, parses the first string in that line, and adds it to a
       TEXT_LIST if it is not already there. This TEXT_LIST is returned.

       When the @DbColumn function is invoked using the class name
       "dbdrv" as in:

           @DbColumn("dbdrv";"dbdrive.db")

       the driver will open the database file "dbdrive.db" (located
       in the data directory), read each line from the database,
       construct a TEXT_LIST containing the unique first strings,
       and returns the TEXT_LIST.

@DbLookup: The database driver's @DbLookup function takes as input the
       name of a database from which to read and a string to use as
       a search key;  @DbLookup returns a handle to a text list
       containing the second strings from all lines whose first strings
       matched the key.

       When the @DbLookup function is invoked using the class name
       "dbdrv" as in:

           @DbLookup("dbdrv";"dbdrive.db";"North America")


       the driver will open the database file "dbdrive.db" (located
       in the Domino or Notes Data directory), read each line from
       the database, construct a TEXT_LIST containing the second
       string of each line whose first string was "North America",
       and return a handle to the TEXT_LIST.

Please see the User Guide chapter on External Database Drivers and the
database driver source code for more details on writing database drivers.


Invoking the database driver from a Notes Workstation
-----------------------------------------------------
<Not applicable on Domino Servers>
After building the DLL, copy it to your Notes Program directory.
Copy dbdriver.nsf to your Notes Data directory.
Copy dbdriver.db  to your Notes Data directory.

@DbLookup:

First look over the design of the form that uses @DbLookup
in dbdriver.nsf.  From Domino Designer, open the dbdriver.nsf 
database, choose Forms, and open the form "DbLookup Form".  
Click on the field "Animals".  The formula for this field should 
read:

    @DbLookup("dbdrv";"dbdriver.db";"North America").

Close this form.

From Notes client, compose a document using the "DbLookup Form". 
Displayed on this form is a non-editable text field.
The contents of the text field will be all of the strings in the
database that were preceded by the string "North America".

Display the contents of the file dbdriver.db, and you'll see that the
strings displayed in the text field are indeed the strings that follow
all instances of the string "North America".


@DbColumn:

From Domino Designer, open the dbdriver.nsf database, choose
Forms, and open the form "DbColumn Form".  Double click on the field,
"ContinentList", to bring up the Field Properties.  In the Properties 
Control Box, the formula for keyword choices should read:

    @DbColumn("dbdrv";"dbdriver.db").

Close this form.

From Notes client, compose a document using the "DbColumn Form". 
Displayed on this form is a keyword field, shown as
checkboxes, of the seven continents of the world.

Display the contents of the file dbdriver.db, and you'll see that the
keyword list does indeed contain the same list of strings contained in
dbdriver.db.


Invoking the database driver from a Domino Server
-------------------------------------------------
For Windows, copy ndbdbdrv.dll to your Domino Program directory.

For IBM i, add a link to the service program libdbdbdrv.srvpgm:

      ADDLNK OBJ('/QSYS.LIB/QNTCSDK.LIB/LIBDBDBDRV.SRVPGM')
             NEWLNK('/QIBM/USERDATA/LOTUS/NOTES/LIBDBDBDRV.SRVPGM')
             LNKTYPE(*SYMBOLIC) 
Copy  dbdrtest.nsf to your Domino Data directory.
Copy  dbdriver.db  to your Domino Data directory.


dbdriver.db currently contains the following strings:

        Africa, Elephant
        Africa, Zebra
        Antarctica, Penguin
        Asia, Panda
        Australia/New Zealand, Koala
        Europe, Reindeer
        North America, Bison
        North America, Grizzly Bear
        South America, Jaguar

Start the Domino server, and be sure the Agent Manager server task is
started, either automatically via the notes.ini "ServerTasks=" line, or
manually from the Domino server console prompt, as in

  > load amgr

The Domino database file, dbdrtest.nsf, contains the background agent - 
"Db Driver Test".  This agent runs automatically on the Domino Server 
every hour.  This agent uses @DbLookup to set the field called 
TEXT_LIST of all documents in dbdrtest.nsf to the values associated 
with North American in dbdriver.db.

You will need to edit this background agent in order to specify the 
name of your Domino Server.  After you copy dbdrtest.nsf to your Domino
Server data directory, use Domino Designer to select or open this
database (on the server).  From the Agents navigator, select and edit the
"Db Driver Test" agent.  Check to make sure that the agent's formula is:

     FIELD TEXT_LIST := @DbLookup("dbdrv"; "dbdriver.db"; "North America");
     SELECT @All

Click the "Schedule..." button and select the name of your Domino Server 
from the "Run on:" edit box.  Choose OK to save your changes.  The agent 
will be run at each hourly index, as scheduled.

For testing you may want to modify the scheduled frequency.  Use the
"Schedule..." button to do this.

After the server has executed the background macro for dbdrtest.nsf, open
this database.  Open one of the documents and notice that the contents
of the TEXT_LIST field has been changed to be all of the strings in the
external database that were preceded by the string "North America", 
specifically,
      Bison, Grizzly Bear

The "Reset" agent can be run to reset the documents and re-enable the 
background agent.

