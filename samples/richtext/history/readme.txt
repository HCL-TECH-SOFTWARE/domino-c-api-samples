Licensed Materials - Property of HCL
CIR9ZEN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
history - Sample HCL C API for Domino and Notes program.
          Collect all response documents into the rich text field of the
          main document.


PURPOSE
-------
This program reads through a view of a service request tracking database,
finds all the responses to each main document, and appends the data from
the response documents to the "history" field (a rich text field) of
the corresponding main document. It preserves all elements of the rich
text of the response documents including bit maps, tables, and document
links. After processing the response documents, it deletes them from the
database.

History is a valuable sample for the following reasons:

     1. It demonstrates how to navigate a view and find all the
        responses to each main document. (See functions CollectHistory()
        and SkipToMainDocument().)

     2. It shows how to collect data of various types from multiple
        response documents into a rich text field. (See functions
        ProcessOneResponse() and ReplaceHistory().)

     3. It shows how to work around a known limitation in the
        function CompoundTextAssimilateItem(). The problem is that
        CompoundTextAssimilateItem() does not preserve Document Links.
        (See functions ExportLinks(), ProcessOneBodyItem(), and
        ProcessOneCDRecord() for code that converts CDLINK2-type doc
        Links to CDLINKEXPORT2 doc links.)

     4. It shows how to populate a Windows combo list box with the
        titles and view note IDs of all the views available in a DB.
        (See the function InitViewTitleList().)


ENVIRONMENTS
------------
Windows (32-bit)

Note: Currently not support 64-bit UI application on Windows 64-bit

FILES
-----
readme.txt   - This file specifies what is needed to use this example.

history.c    - Main program C source code file.

history.h    - Symbolic constants. Data structures. Menu and
               dialog box control IDs. Function prototypes.

histerr.h    - Program-specific error codes, used as string IDs.

histwin.rc   - Resource compiler source for Windows 32-bit.

histwin.dlg  - Dialog box definition file for Windows 32-bit.

histwin.ico  - Icon for Windows 32-bit.

mswin32.mak     - Make file for Windows 32-bit.



INSTALLING THE TARGET DATABASE
------------------------------
The history program assumes that the target database has the design of
the 'history.nsf' database, provided with this toolkit.

1. Copy the target database 'history.nsf' to the Domino or Notes data
   directory.  Note: Running 'history' modifies the database significantly,
   so you may wish to keep an original copy in another directory.

2. In Notes, open the 'history.nsf' database and examine the main view.
   Make sure there are some main documents and some response documents
   visible.  You may compose additional main or response documents to test
   the software.  (In the database provided with the Toolkit, some documents
   already have information in the history field.)


RUNNING history
---------------
1. Under Windows, from the command prompt type:
      history
   The main program window appears. There is only one menu: File.

2. Pull down the File menu. Select "Collect responses into History".
   The "Response Collector" dialog box appears.
   Make sure the selected database is "Service Requests (HISTORY)"
   and the "View to Process" is "1. Date Received (Main View)".

3. Click the "Collect responses into history" button.  The cursor turns to
   an hourglass while the program is processing the database.  When it is
   finished, a message box displays "Successfully collected responses into
   history".
   If history encounters an unexpected condition, it displays an appropriate
   message.


CHECKING THE RESULTS
--------------------
1. From Notes, open the target database and press <escape>, if necessary,
   to see the main view.

2. Notice that there are no response documents.  History deletes them after
   collecting the data into the main documents.

3. Open any main document.  Examine the history field.  Notice that all the
   data from the original response documents has been appended to the history
   field - including rich text elements like bit maps, tables, and document
   links.  Any information that was already in the history field is preserved.
