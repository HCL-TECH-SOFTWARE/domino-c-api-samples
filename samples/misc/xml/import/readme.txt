Licensed Materials - Property of HCL
CIR9ZEN
CIRA2EN

(c) Copyright IBM Corporation   1996,   2013
(c) Copyright HCL Technologies   1996,   2024
All rights reserved.


PROGRAM
-------
dxlimport - Import XML (Extensible Markup Language) data into a Domino/Notes (NSF) file.


PURPOSE
-------
Dxlimport illustrates functionality available for importing XML data into Domino/Notes data.


ENVIRONMENTS
------------
Windows (32-bit)
Windows (64-bit)
IBM i
Linux 64-bit

FILES
-----
dxlimport.c   - Main program.

dxlimport.h   - Constants and function prototypes.

mswin32.mak     - Make file for Windows 32-bit.

mswin64.mak     - Make file for Windows 64-bit.

mk_import_i.clp  - Make file (CL Program) for IBM i using ILE C 
                       with the source code being an IFS file.  
                                                
mk_import_p.clp - Make file (CL Program) for IBM i using ILE C 
                       with the source code being in a source physical file.

readme.txt   - This file.


RUNNING dxlimport
----------------
- Under Windows use the following syntax at the system command prompt:

  dxlimport [ options ] dbpath

		options:
				
				-i dxlfile: DXL input file or URI
				-s server:  Domino server; omit for a local database
				-r          Replace existing database properties
				-n:         Use non-validating parser (uses validating parser otherwise)
				-nef:       No exit on first fatal error (try to continue after fatal error)
				-ui:        Ignore unknown elements and attributes
				-uw:        Log warning for unknown elements and attributes
				-ue:        Log error for unknown elements and attributes
				-uf:        Log fatal error for unknown elements and attributes
				-acli:      Ignore DXL <acl> elements (default)
				-aclri:     Replace existing ACL with DXL <acl>, else ignore
				-aclui:     Update existing ACL from DXL <acl>, else ignore
				-desc:      Create new design elements, leaving existing elements intact
				-desi:      Ignore DXL design elements (default)
				-desri:     Replace existing design elements with DXL, else ignore
				-desrc:     Replace existing design elements with DXL, else create
				-doci:      Ignore DXL <document> elements
				-docc:      Create new documents (default)
				-docri:     Replace existing documents with DXL, else ignore
				-docrc:     Replace existing documents with DXL, else create
				-docui:     Update existing documents from DXL, else ignore
				-docuc:     Update existing documents from DXL, else create
				-ftc:       Create full text index if a <fulltextsettings> element exists
				-norepl:    Replace/update do not require database and DXL be replicas
				

The database name is required and the default path is the Domino or Notes data directory.  The XML input source file
is also required.


EXAMPLES
--------

	Notes:	The DXL_IMPORT_PROPTERY values will always be set with their default values unless an option is specfied
			to change that default value.

			The sample program 'dxlexport' found in the toolkit samples directory '..\samples\misc\xml\export' can
			be used to generate an XML input source file to use with sample 'dxlimport'.


	dxlimport -i <xml-Full_filepath> <database name>
	dxlimport -i "C:\simple.xml" simple.nsf

	The above command will execute 'dxlimport' using file 'simple.xml' as the XML input source and the XML data will
	be imported into Domino/Notes database 'simple.nsf'.  Since there were no other options specfied all
	DXL_IMPORT_PROPERTY default values will be honored.


	
	dxlimport -norepl -docrc -i "C:\simple.xml" simple.nsf

	The above command will execute 'dxlimport' using file 'simple.xml' as the XML input source and the XML data will
	be imported into Domino/Notes database 'simple.nsf'.  The option 'norepl' indicates the XML data to import and
	the database to import to do not need to be replicas.  The option '-docrc' is requested which means when importing
	replace existing Domino/Notes documents and create new documents where none exist.  The Document import property has
	been assigned the DXLIMPORTOPTION_REPLACE_ELSE_CREATE.

- Under IBM i type:

    RUNDOMCMD SERVER(<server name>) CMD(CALL PGM(QNTCSDK/DXLIMPORT)
        PARM('-i' 'simple.xml' 'simple.nsf')) BATCH(*no)
or
    RUNDOMCMD SERVER(<server name>) CMD(CALL PGM(QNTCSDK/DXLIMPORT)
        PARM('-norepl' '-docrc' '-i' 'simple.xml' 'simple.nsf')) 
		BATCH(*no)
