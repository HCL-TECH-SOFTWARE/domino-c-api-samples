Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2025
All rights reserved.


PROGRAM
-------
setprivs - HCL C API for Domino and Notes Sample Program:
           Set Document Read Access Privilege Levels and/or Person,
           Server, Group and Role Name List.


PURPOSE
-------
This program sets the read access privilege levels or read access name
list (consisting of people, servers, groups and/or roles) of all documents
in a view of a Domino database.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM i
Linux 64 bit


FILES
-----
readme.txt   - This file specifies what is needed to use this example.

setprivs.c   - Main program.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

linux64.mak    - Makefile for Linux 64 bit.

mk_setprivs_i.clp    - Make file (CL Program) for IBM i using ILE C 
                   with the source code being an IFS file.
    
mk_setprivs_p.clp    - Make file (CL Program) for IBM i using ILE C 
                   with the source code being in a source physical file.   

RUNNING setprivs:
-----------------
This sample program will run against the Domino database privs.nsf,
provided with the HCL C API for Domino and Notes toolkit.  You must
copy this file to the Domino data directory before running this program.

setprivs may be used to either set read access privilege levels or to set
a read access name list.

Read access privilege levels are set for a document by associating a
privilege mask with the document.  Privilege levels were used in
earlier versions of Notes and are still available for use.

A read access name list is set for a document by associating a text list
field with the document.  Role names are each enclosed in square brackets.

The following are the arguments used in this sample:

  <database> is the full pathname of the Domino database to be modified.

  <view> is the name of the view.
  Each document in this view will have its privilege levels modified.


  <numeric_privilege> is a decimal integer value that is the sum of the
  individual decimal privilege level values required for access to the
  documents:

      Privilege1 =  1
      Privilege2 =  2
      Privilege3 =  4
      Privilege4 =  8
      Privilege5 = 16

  For example, to allow all privilege levels to have access, use 31
  (1 + 2 + 4 + 8 + 16) as the numeric privilege.  To allow Privilege3 and
  Privilege4 to have access, use 12 as <numeric_privilege>.  To clear
  read access privilege levels, use 0 as <numeric privilege>.


  <name/role> is either one of the people, servers, or groups or one of the
  role names to be set in the name list.  Role names must be enclosed in
  square brackets.  To clear the read access list, specify "".
  If this argument contains spaces, it must be enclosed in double quotation
  marks.


- Under Windows:
  To set the read access privilege levels:

  type the following at the system command prompt:

  setprivs <database> <view> -M <numeric_privilege>


  To set the read access name list:

  type the following at the system command prompt:

  setprivs <database> <view> -L <name/role> [<name/role>...]

  If any of your arguments contain spaces, enclose them in quotes.


EXAMPLE
-------

The sample database, privs.nsf, has the following available privilege levels:

      Privilege 1
      Privilege 2
      Privilege 3
      Privilege 4
      Privilege 5

and the following available role names:

      Design Team
      Sales Team

To set the privilege levels to Privilege 3 and Privilege 4:

  setprivs privs SimpleDataView -M 12

To set the read access name list to the person, Jayne Doe, and to the
role names, Design Team and Sales Team:

  setprivs privs SimpleDataView -L "Jayne Doe" "[Design Team]" "[Sales Team]"


NOTE: Because the database is in the Domino data directory and has the
default extension (.nsf) you don't have to type the directory or
extension.  If the database were in a different directory or had another
extension you would type these.

If the program runs correctly, it should display

  "2 documents had their privileges updated"

- Under IBM i:
  run:  rundomcmd server(<server>) cmd(call pgm(qntcsdk/setprivs)
         parm('privs.nsf' 'simpledataview' '-m' '12')) batch(*no)    

CHECKING THE RESULTS
--------------------

To see if setprivs worked correctly when setting the privilege levels:

  Open privs.nsf.

  Select the view SimpleDataView.

  Highlight one of the documents in the view.

  Choose File - Document Properties... and select the "Key" tab.

This will display, under "Who can read this document" the current required
privilege levels.  For example, if the above stated example had been run
(setprivs using the -M option), then "(Privilege 3)" and "(Privilege 4)"
would be checked under "Who can read this document" in the Document InfoBox.

To see if setprivs worked correctly when setting the read access name list:

  Open privs.nsf.

  Select the view SimpleDataView.

  Highlight one of the documents in the view.

  Choose File - Document Properties... and select the "Fields" tab.

Highlight the $Readers field.  The specified names will appear in the
text list.
