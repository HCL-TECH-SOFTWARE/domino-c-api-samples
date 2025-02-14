Licensed Materials - Property of HCL


(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.



PROGRAM
-------
dbpolicy - Sample Notes API Program: Create a database policy document.


PURPOSE
-------
This sample creates a Help-About (policy) document in a database and
writes some text to the rich text field of this document.  It also creates
a Help-Using (help) document and writes some text to it.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)


FILES
-----
readme.txt   - This file specifies what is needed to use this example.

dbpolicy.c   - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.


RUNNING dbpolicy:
-----------------

- Under Windows:
  type the following at the system command prompt:

  dbpolicy <database>

where:

  <database> is the full pathname of the Domino database to be modified.


EXAMPLE
-------

  dbpolicy  dbpolicy

dbpolicy will create a database policy document (Help-About This Database
document) and a database help document (Help-Using This Database document)
in the database DBPOLICY.NSF.  Each document's $BODY field will contain a
couple of lines of richtext.

dbpolicy will also print out the text in the $BODY field of the database
policy document and the database help document.


CHECKING THE RESULTS
--------------------
After running the DBPOLICY program, open the database.
To display the database Policy Document that was just created, choose the
Help/About DB Policy Doc menu item.  The database policy document will
contain the strings:

                    "DATABASE POLICY STATEMENT"
"This is a sample Policy Document (Help-About Database document) for
the API sample program, DBPOLICY."

displayed in rich text.

To display the database Help Document that was just created, choose the
Help/Using DB Policy Doc menu item.  The database help document will
contain the strings:

                    "DATABASE HELP STATEMENT"
"This is a sample Help Document (Help-Using Database document) for
the API sample program, DBPOLICY."

displayed in rich text.

