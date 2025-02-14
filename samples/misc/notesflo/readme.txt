Licensed Materials - Property of HCL
CIR9ZEN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.



PROGRAM
--------------
notesflo 

PURPOSE
-------------
Sample OLE 2 Notes/FX and NotesFlow server.  It creates
a object to which we can change background and frontground color.

ENVIRONMENTS
-----------------------
Windows (32-bit)
Windows (64-bit)

FILES
--------
mswin32.mak  - Make file for Windows 32-bit.
mswin64.mak  - Make file for Windows 64-bit.

notesflo.h   - Application header file
notesflo.cpp - Application source
notesflo.ico - Application icon
selcolor.cpp - Color selection dialog
app_rsc.h    - Resource header file
app_rsc.rc   - Resource compiler source
ln_buff.hpp  - Notes API buffer class header
ln_buff.cpp  - Notes API buffer class source
ln_flow.hpp  - NotesFlow classes header
ln_flow.cpp  - NotesFlow classes source
nf_app.hpp   - Application object header
nf_app.cpp   - Application object source
nf_doc.hpp   - Document object header
nf_doc.cpp   - Document object source
nf_image.hpp - Image object header
nf_image.cpp - Image object source
nf_obj.hpp   - OLE object header
nf_obj.cpp   - OLE object source
nf_hatch.hpp - Hatch window handler header
nf_hatch.cpp - Hatch window handler source
nf_store.hpp - Storage object header
nf_store.cpp - Storage object source
nf_util.cpp  - Utility functions source
olecf.hpp    - OLE 2 IClassFactory header
olecf.cpp    - OLE 2 IClassFactory source
oledo.hpp    - OLE 2 IDataObject header
oledo.cpp    - OLE 2 IDataObject source
oleec.hpp    - OLE 2 IExternalConnection header
oleec.cpp    - OLE 2 IExternalConnection source
oleipao.hpp  - OLE 2 IInPlaceActiveObject header
oleipao.cpp  - OLE 2 IInPlaceActiveObject source
oleipo.hpp   - OLE 2 IInPlaceObject header
oleipo.cpp   - OLE 2 IInPlaceObject source
oleobj.hpp   - OLE 2 IOleObject header
oleobj.cpp   - OLE 2 IOleObject source
oleps.hpp    - OLE 2 IPersistStorage header
oleps.cpp    - OLE 2 IPersistStorage source
readme.txt   - This file specifies what is needed to use this example.
nf_doc.nsf   - Detailed documentation on the OLE server and NotesFlow
	       implementation in this sample program.


RUNNING notesflo
-------------------------
Under Windows 64-bit

           notesflo

Under Windows 32-bit

The notesflo program can be run in 5 different ways:

 1) Register the program in the Windows system registry.
 2) Un-register the program.
 3) Generate a registration (.reg) file.
 4) Run as a stand-alone program.
 5) Run as an embedded OLE 2 server.
 
    1:  Registering notesflo
    ----------------------------

The command to register the program in the Windows system registry is:

    notesflo -RegServer

The information necessary for an OLE container to load and run the
program is added to the Windows system registry.  This must be done
before a notesflo object can be embedded in a Notes document.


    2:  Un-registering notesflo
    ---------------------------------

The command to un-register the program is:

    notesflo -UnregServer

All the information about the notesflo program is removed from the
Windows system registry.  After this is done, the program cannot be
run as an embedded OLE object.


    3:  Generating a registration file
    ----------------------------------------

The program can generate a registration file, notesflo.reg, which the
registry editor can use to install the notesflo program.  The command
is:

    notesflo -MakeReg

The registration file is created and the program terminates.


    4:  Running as a stand-alone program
    -----------------------------------------------

The command to run the program on Windows systems is:

    notesflo

The program can also be run from the File Manager or Explorer.  The
program launches, registers itself in the Windows system registry, then
draws the image in an application window.


    5:  Running as an embedded object
    --------------------------------------------

1) Copy the database notesflo.nsf to your Notes data directory.

2) Register the application as described in "1: Registering notesflo".
    This must be done in order for Windows and Notes to locate the
    server program.

3) From Notes, open the NotesFlow Containers database (notesflo.nsf).

4) You may open an existing document:
    4a) Select a document from the "NotesFlow Containers" view.
    4b) Open the document for editing ("Actions/Edit Document")
    4c) Double-click on the graphic.  The object should become active
	in place, as shown by the hatched border.

5) You may create a new document:
    5a) Select "Create/NotesFlow Container" from the Notes menu.
    5b) Set the color and size fields as desired.
    5c) Move the cursor to the "Document" field.
    5d) From the Notes menu, select "Create/Object...".  The Windows
	"Create Object" dialog box is displayed.
    5e) Select "NotesFlow" sample.  A new in-place active object
	should appear, with the hatched border.

6) While the NotesFlow object is in-place active, the foreground and
    background colors may be changed through the "Colors" menu.  The
    size may be changed by dragging the size handles in the hatched
    border.  Changing the colors or size of the object will update
    the color and size fields in the document.

7) When the Notes document is active (the hatched border is not visible),
    the color and size of the object may be changed by updating the fields
    in the document.

For more detailed information on the NotesFlow code in this sample, please
see the database nf_doc.nsf in this sample program directory.
