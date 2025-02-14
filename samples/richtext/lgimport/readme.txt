Licensed Materials - Property of HCL
CIR9ZEN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
lgimport  - Load and call a Domino or Notes import library with a large
            input file


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)


FILES
-----
readme.txt    -  This file.

lgimport.c    -  Main program.
cdrecord.c    -  Contains the functions that create the various CD records.
lgimport.h    -  Function prototypes and symbolic constants
tiger.tif     -  A sample TIFF image file that the user may import
		 to test this program.

mswin32.mak  -  Makefile for Windows 32-bit.

mswin64.mak  -  Makefile for Windows 32-bit.


PURPOSE
-------
This program demonstrates how to import several different kinds of
files into a rich text field of a Domino document.  The program has
been tested with the following file format:

      TIFF image files

Also included is an explanation on how to import a WordPro document.

File imports are accomplished through the use of import DLLs that are
specific to each format. The information about the DLLs used to import
files is contained in the NOTES.INI file.  Following are two sample
entries from NOTES.INI, and an explanation of the fields in each:

   EDITIMP6=TIFF 5.0 Image,0,_ITIFF,,.TIF,,18,

The fields in this entry are:

    EDITIMP6         - Specifies that this is an EDIT-level import
		       entry.

    TIFF 5.0 IMAGE   - This entry refers to Tagged Image File Format images.
		       This is the string that appears in the list box in the
		       Notes file import dialog.

    0                - For Imports, this parameter is always zero.

    _ITIFF           - The DLL to be used to import this file.

    <blank>          - This field is left blank for graphics file import.

    .TIF             - The default file extension for files of this
		       type. (This field is not important for this
		       example)


EDITIMP21=WordPro 96/97,0,_IW4W,W4W12F/V0,.LWP,,2,

The fields in this entry are:

    EDITIMP21        - Specifies that this is an EDIT-level import
		       entry.

    WordPro 96/97    - This entry refers to WordPro 96/97 documents.
		       This is the string that appears in the list box in the
		       Notes file import dialog.

    0                - For Imports, this parameter is always zero.

    _IW4W            - The DLL to be used to import this file.

    W4W12F/V0        - This is the name of an additional DLL used by _IW4W
		       to import this word processing file.  This
		       parameter is set to blank for non-word processing
                       files.

    .LWP             - The default file extension for files of this
		       type. (This field is not important for this
		       example)


NOTE:  Depending on the platform, Domino and Notes use slightly different
       naming conventions for their import DLLs.  The first character of a
       Windows DLL filename is 'n'.  But the entries in NOTES.INI always
       use '_' for the first character.


RUNNING lgimport
----------------
The LGIMPORT program takes either three or four parameters:

   1)   The filename of the database to which the document is to be
	added.  This file should reside in the Domino or Notes data directory.

   2)   The fully qualified path name of the import DLL appropriate
	to the format of the file being imported.  This information is
	found in the NOTES.INI file.  If the Domino or Notes directory is on
	your search path, then you can specify just the file name.

   3)   The filename of the file to be imported.  If the file is not in
	the current directory, the full path to the file must be supplied.
	
   4)   If importing a word processing file, the name of the additional DLL
	specified in the NOTES.INI entry. If not importing a word
	processing file, this parameter is omitted.


A sample command line to invoke this program might look like
the following under Windows.  Filenames and paths would need to be changed
to suit the user's specific environment.


Import a TIFF 5.0 Image:
-----------------------
lgimport lgimport.nsf c:\lotus\notes\nitiff.dll tiger.tif
	  ^            ^                         ^         ^
	  |            |                         |         |
	  |            |                         |         No 2nd DLL name
	  |            |                         |
	  |            |                    Name of TIFF Image file to import
	  |            |
	  |            Import DLL for TIFF Image file
	  |
	  Name of database


Import a WordPro file:
----------------------
lgimport lgimport.nsf c:\lotus\notes\niw4w.dll filename.lwp w4w12f/v0
	  ^            ^                       ^            ^
	  |            |                       |            |
	  |            |                       |            2nd DLL needed
	  |            |                       |            for WordPro files
	  |            |                       |
	  |            |                       Name of WordPro file to import
	  |            |
	  |            Import DLL for WordPro file
	  |
	  Name of database

