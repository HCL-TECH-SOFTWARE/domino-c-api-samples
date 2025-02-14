Licensed Materials - Property of HCL
CIR9ZEN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
editfax - Sample HCL C API for Notes Program:
          Demonstrate the usage of the Editfax APIs.


ENVIRONMENTS
------------
Windows (32-bit)


PURPOSE
-------
This program shows how to render a Notes document to a Fax bitmap,
and how to attach a FAX bitmap to a Notes document.


FILES
-----
editfax.c      - Main program.

mswin32.mak     - Make file for Windows 32-bit.

readme.txt     - This file specifies what is needed to use this
                 example.


RUNNING editfax:
----------------
This sample program requires a source and a target Domino database.
The faxsrc.nsf and makeform.nsf, provided with the HCL C API for
Domino and Notes toolkit, are specified as the source and the target
Domino database, correspondingly.  You must copy these two
files to the Notes data directory before running this program.


- Under Windows 32-bit, at the system command prompt, type:

        editfax <NoteID-of-the-source-document>

           where:

           NoteID-of-the-source-document: Optional. NoteID of a
                                         document in the source database,
                                         indicating that this is the
                                         only document to be rendered
                                         to the FAX bitmap.

        for example:

           editfax 90A




EXAMINING THE DOMINO DATABASE TO CHECK THE RESULT:
--------------------------------------------------

To see if 'editfax' worked correctly:

      Use Notes to open the makeform.nsf and the faxsrc.nsf databases.

      If you specified a specific NoteID-of-the-source-document when
      executing this sample, you should see the corresponding "FAX copy"
      document in the makeform.nsf.

      If you did not specify a NoteID-of-the-source document when
      executing this sample, for each source document in the faxsrc
      database, you should see a corresponding "FAX copy" document
      in the makeform.nsf.

