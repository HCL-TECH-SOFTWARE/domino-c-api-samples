Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN
  
(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
dxlexport - Export NSF data into XML format.


PURPOSE
-------
DXLEXPORT is a program which allows the export of Domino/Notes (NSF) data into
an Extensible Markup Language (XML) format.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM i
Linux 64bit

FILES
-----
dxlexport.c  - Main program.
dxlexport.h  - Constants and function prototypes.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

mk_export_i.clp  - Make file (CL Program) for IBM i using ILE C 
                       with the source code being an IFS file.  
                                                
mk_export_p.clp - Make file (CL Program) for IBM i using ILE C 
                       with the source code being in a source physical file.

readme.txt   - This file.


RUNNING dxlexport
----------------
- Under Windows use the following syntax at the system command prompt:

	dxlexport [options] dbpath [notespec]

		dbpath		- is relative to the Domino data directory
		notespec	- is either a design element name, or a hex noteid (ie. 0x211a)
	
		options
	
			-a			- Write all notes in the database (including acl and design elements)
			-c <number>	- Write all notes with the given note class(es)
			-d <file>	- Write a DOCTYPE declaration using the given file
			-l			- Write acl elements only
			-n			- Write all notes as <note>
			-o <file>	- Write the DXL to the file (default is standard out)
			-s <server>	- Use the database on the specified Domino server
			-r			- Write rich text items as uninterpreted data (<itemdata>)

The database name is required and the default path is the Domino or Notes
data directory.

The notespec provides a way to specify just one Note for exporting into XML format.

Or one of the following options can be set to export notes data into an XML format:

1.	Select option -a to export all notes in a database into XML format.
2.	Select option -l to only export the ACL elements of a notes database into XML format.
3.	Select option -c <number> to export all notes with the given note class(es) into XML format.

All other switches are optional. If the -o option is not specified, this program exports
the specified contents of a Domino database into XML format to the standard output.
It does not handle signals, which means it does not stop on Ctl+C or Ctl+BREAK.
Therefore, run this program on small databases or specify the -o option to send the
output to a file.

This sample does output a DXL Banner Comment within the XML data.  The DXL Banner Comment
is a property within the DXL_EXPORT_PROPERTY definition.

EXAMPLE
-------

	To export the entire contents of an NSF file into XML format and output the results to
	an output file.

		dxlexport -a -o "Simple.XML" simple
	
	This exports NSF file simple.nsf into XML format and puts the exported data into the output
	file Simple.XML.

	To export a particular noteID within simple.nsf and put the exported data into an output file.

		dxlexport -o "Simple.XML" simple 0x8F6

	This will export NoteID 0x8F6 from simple.nsf into XML format and output the results to
	file Simple.XML.

- Under IBM i type:

    RUNDOMCMD SERVER(<server name>) CMD(CALL PGM(QNTCSDK/DXLEXPORT) 
         PARM('-a' '-o' 'simple.xml' 'simple')) BATCH(*no)
or
    RUNDOMCMD SERVER(<server name>) CMD(CALL PGM(QNTCSDK/DXLEXPORT) 
         PARM('-o' 'simple.xml' 'simple' '0x8F6')) BATCH(*no)
