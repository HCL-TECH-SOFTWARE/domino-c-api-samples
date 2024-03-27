/****************************************************************************
 *
 * Copyright HCL Technologies 1996, 2023.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
      
    PROGRAM:    dxlimport

    FILE:			dxlimport.c

    SYNTAX:			dxlimport [ options ] dbpath

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
	 		
	DESCRIPTION:
	
		This sample illustrates how to import XML data into an NSF database based on the options 
		chosen.	
   

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* C include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <xml.h>
#include <osfile.h>
#include <nsfdb.h>
#include <osmem.h>
#include "dxlimport.h"
#if defined(CAPI_TESTING) 
#include "printlog.h" 
#else
 #define PRINTLOG printf 
 #define PRINTERROR(api_error, api_name) {\
 char    szErrorText[256] = { 0 };\
 OSLoadString(NULLHANDLE, ERR(api_error), szErrorText, sizeof(szErrorText));\
 fprintf(stderr, "[ERROR]:%s:%d:%s - %s", __FILE__,__LINE__,api_name,szErrorText); }
#endif 

#ifndef OSMISC_DEFS
#include <osmisc.h>
#endif

#ifdef UNIX
	#define STRICMP strcasecmp
#else
	#define STRICMP stricmp
#endif

#ifdef UNIX
	#define STRNICMP strncasecmp
#else
	#define STRNICMP strnicmp
#endif

int main(int argc, char *argv[])
{
	STATUS		error =  NOERROR;
	struct		ImportOptions	impOptions;
	DBHANDLE	hDB;

	char		pname[MAXPATH] = "";         /* buffer to store the input path to database */
	char		*path_name;					/* pathname of database */
	

	if (error = NotesInitExtended (argc, argv))
	{
		PRINTLOG(  "\n Unable to initialize Notes.\n");
		return (1);
	} 

	PRINTLOG(  "DXLIMPORT Utility\n" );

	memset(&impOptions, 0, sizeof(impOptions));
	error = ProcessArgs( argc, argv, &impOptions );
	
	/* ProcessArgs checks the syntax, etc. */
	if (error)
	{
		PRINTLOG( "\nInvalid command line argument(s).  Please try again. \n");
		PrintUsage();
		NotesTerm(); /* ProcessArgs already printed an error message */ 
		return (0);
    }

	path_name = impOptions.fileDatabase;		/* Assign Database to Import */

	if (impOptions.serverName != NULL)			/* Check to see if user supplied a servername */
	{
		if (error = OSPathNetConstruct( NULL, impOptions.serverName, impOptions.fileDatabase, pname))
		{
			PRINTERROR(error,"OSPathNetConstruct");
			NotesTerm();
			return(1);
		}
		path_name = pname;
	}

	
	PRINTLOG(  "dxlimport: importing '%s'\n", path_name );

	if (error = NSFDbOpen( path_name, &hDB ))	/* Open the Domino/Notes Database */
{
		PRINTLOG(  "Error: unable to open '%s.'\n", path_name );
		PRINTERROR (error,"NSFDbOpen"); 
		NotesTerm();
		return (1);
	}

	error = ImportXMLData(hDB, &impOptions);
	
	if(error)
	{
		NSFDbClose(hDB);
		PRINTERROR(error,"ImportXMLData");
		NotesTerm();
		return(1);
	}

	NSFDbClose( hDB );
	
	PRINTLOG(  "\n\nDXLIMPORT: Done.\n" );
	
	if (error == NOERROR)
	PRINTLOG("\nProgram completed successfully.\n");
	NotesTerm();
	return (0);
}

/************************************************************************

    FUNCTION:   ImportXMLData

    INPUTS:     DBHANDLE hDB, struct ImportOptions *impOptions
   

*************************************************************************/
STATUS LNPUBLIC ImportXMLData( DBHANDLE hDB, struct ImportOptions *impOptions)
{
	STATUS				error = NOERROR;
	DXL_IMPORT_PROPERTY		propValue;			/* DXL Import Property Values */
	DXLLOGOPTION			logAction;			/* DXL Log Option Values */
	Xml_Validation_Option		valOption;			/* XML Validation Options */
	DXLIMPORTHANDLE			hDXLImport;			/* DXL Import Handle */
	DXLIMPORTOPTION			setGetValue;		/* DXL Import Option Values */
	MEMHANDLE			hMem;			/* Memory Handle needed to view the log for any errors */
	char				*pData;			/* Pointer to log information if any */
	struct ImportContext		impCtx;		/* Information needed to pass to the reader function */
	
	/* Assign the DXLIMPORTHANDLE for this Import Session */
	if(error = DXLCreateImporter (&hDXLImport))
		return (error);
	
	/* 
	 * Check options passed in from the command line and set the DXLIMPORTPROPERTY
	 * values to values requested.
	 */
	setGetValue = DXLIMPORTOPTION_IGNORE;
	/*
	 * iACLImportOption -	Default is DXLIMPORTOPTION_IGNORE
	 *						Change this option if not equal to default.
	 */
	if(impOptions->Properties.aclOpt != setGetValue)
	{
		propValue = iACLImportOption;
		if(error = DXLSetImporterProperty(hDXLImport, propValue, &impOptions->Properties.aclOpt))
		{
			DXLDeleteImporter(hDXLImport);
			return(error);
		}
	}

	/*
	 * iDesignImportOption -	Default is DXLIMPORTOPTION_IGNORE
	 *							Change this option if not equal to default.
	 */
	if(impOptions->Properties.designOpt != setGetValue)
	{
		propValue = iDesignImportOption;
		if(error = DXLSetImporterProperty(hDXLImport, propValue, &impOptions->Properties.designOpt))
		{
			DXLDeleteImporter(hDXLImport);
			return(error);
		}
	}
	
	/* 
	 * iDocumentsImportOption -	Default is DXLIMPORTOPTION_CREATE
	 *							Change this option if not equal to default.
	 */
	setGetValue = DXLIMPORTOPTION_CREATE;
	if(impOptions->Properties.documentsOpt != setGetValue)
	{
		propValue = iDocumentsImportOption;
		if(error = DXLSetImporterProperty(hDXLImport, propValue, &impOptions->Properties.documentsOpt))
		{
			DXLDeleteImporter(hDXLImport);
			return(error);
		}
	}

	/*
	 * iInputValidationOption -	Default is Xml_Validate_Auto
	 *							Change this option if not equal to default.
	 */
	valOption = Xml_Validate_Auto;
	if(impOptions->Properties.inputValOpt != valOption)
	{
		propValue = iInputValidationOption;
		if(error = DXLSetImporterProperty(hDXLImport, propValue, &impOptions->Properties.inputValOpt))
		{
			DXLDeleteImporter(hDXLImport);
			return(error);
		}
	}

	/*
	 * iUnknownTokenLogOption -	Default is DXLLOGOPTION_FATALERROR
	 *							Change this option if not equal to default.
	 */
	logAction = DXLLOGOPTION_FATALERROR;
	if(impOptions->Properties.tokenLogOpt != logAction)
	{
		propValue = iUnknownTokenLogOption;
		if(error = DXLSetImporterProperty(hDXLImport, propValue, &impOptions->Properties.tokenLogOpt))
		{
			DXLDeleteImporter(hDXLImport);
			return(error);
		}
	}
	
	/*
	 * iCreateFullTextIndex -	Default is FALSE, change option value if set to TRUE 
	 */
	if(impOptions->Properties.createFullText)
	{
		propValue = iCreateFullTextIndex;
		if(error = DXLSetImporterProperty(hDXLImport, propValue, &impOptions->Properties.createFullText))
		{
			DXLDeleteImporter(hDXLImport);
			return(error);
		}
	}

	/*
	 * iReplaceDbProperties -	Default is FALSE, change option value if set to TRUE
	 */
	if(impOptions->Properties.replaceDbProp)
	{
		propValue = iReplaceDbProperties;
		if(error = DXLSetImporterProperty(hDXLImport, propValue, &impOptions->Properties.replaceDbProp))
		{
			DXLDeleteImporter(hDXLImport);
			return(error);
		}
	}

	/*
	 * iExitOnFirstFatalError -	Default is TURE, change option value if set to FALSE
	 */
	if(impOptions->Properties.exitOnFirstFatalError == FALSE)
	{
		propValue = iExitOnFirstFatalError;
		if(error = DXLSetImporterProperty(hDXLImport, propValue, &impOptions->Properties.exitOnFirstFatalError))
		{
			DXLDeleteImporter(hDXLImport);
			return(error);
		}
	}

	/*
	 * iReplicaRequiredForReplaceOrUpdate -	Default is TRUE, change option value if set to FALSE
	 */
	if(impOptions->Properties.replicaRequiredForReplaceOrUpdate == FALSE)
	{
		propValue = iReplicaRequiredForReplaceOrUpdate;
		if(error = DXLSetImporterProperty(hDXLImport, propValue, &impOptions->Properties.replicaRequiredForReplaceOrUpdate))
		{
			DXLDeleteImporter(hDXLImport);
			return(error);
		}
	}

	/*
	 * Options are all set.  Time to Import.
	 */
	memset(&impCtx, 0, sizeof(impCtx));
	
	/* Assign DXL File Name to read if any */
	impCtx.dxlFileName = impOptions->pInputFile;
		
	/* Start the Import of the XML Data */
	error = DXLImport( hDXLImport, DXLReaderFunc, hDB, (void far *)&impCtx );

	/*
	 * Check to find out if there were any error messages generated with this import.
	 */
	if(DXLImportWasErrorLogged(hDXLImport))
	{
		/* iDxlResultLog */
		propValue = iResultLog;
		if(DXLGetImporterProperty(hDXLImport, propValue, &hMem))
		PRINTLOG( "\nUnable to Get Error Log Information...\n");
		pData = (char *)OSMemoryLock(hMem);
		PRINTLOG( "\nResultLog = %s\n", pData);
		OSMemoryUnlock(hMem);
		OSMemoryFree(hMem);
	}
	DXLDeleteImporter(hDXLImport);
	return(error);
}

/************************************************************************

    FUNCTION:   DXLReaderFunc

    INPUTS:     unsigned char *pBuffer, const DWORD MaxToRead, void far *impCtx
   

*************************************************************************/
DWORD LNCALLBACK DXLReaderFunc(unsigned char *pBuffer, const DWORD MaxToRead, void far *impCtx )
{
	DWORD bytesReturned = 0;
	DWORD numread = 0;
	DWORD bytesToCopy = 0;
    
	struct ImportContext *tmpCtx = (struct ImportContext*)impCtx;

	/* Check to see if first time through */
	if(!tmpCtx->dxlInputFile)
	{
		/* open the XML input file to import */
		tmpCtx->dxlInputFile = fopen(tmpCtx->dxlFileName, "r+b");

		if (!tmpCtx->dxlInputFile)
		{
			PRINTLOG( "Error: Unable to open XML Input file:%s.\n", tmpCtx->dxlFileName);
			return(0);
		}	
    
		/* get a handle to the file */
		tmpCtx->InputFDHandle = fileno(tmpCtx->dxlInputFile);

		/* get the file length of the file */
		if((fstat(tmpCtx->InputFDHandle, &tmpCtx->buf) == -1))
		{
			PRINTLOG( "Error: Unable to obtain file size.\n");
			return(0);
		}
		tmpCtx->InputFDSize = tmpCtx->buf.st_size;
		tmpCtx->bytesRemaining = (DWORD)tmpCtx->InputFDSize;
	}
	/* Now ready to pass data to the parser */
	if( tmpCtx->bytesRemaining == 0 )
	{
		/* All done processing this input source */
		fclose(tmpCtx->dxlInputFile);
		return(tmpCtx->bytesRemaining);
	}
	else if( tmpCtx->bytesRemaining > MaxToRead )
		bytesToCopy = MaxToRead;
	else 
		bytesToCopy = tmpCtx->bytesRemaining;

	numread = fread((unsigned char *)pBuffer, 1, bytesToCopy, tmpCtx->dxlInputFile);
	if(ferror(tmpCtx->dxlInputFile))
	{
		PRINTLOG( "Error: Unable to continue reading import file %s.\n Terminating..\n", "ExpNotes.xml");
		fclose(tmpCtx->dxlInputFile);
		bytesReturned = 0;
	}
	if(feof(tmpCtx->dxlInputFile) && (!numread))
	{
		/* End of File Reached earlier than expected */
		fclose(tmpCtx->dxlInputFile);
		bytesReturned = 0;
	}
	else if (feof(tmpCtx->dxlInputFile) && (numread > 0))
	{
		/* End of file reached in this run */
		bytesReturned = numread;
	}
	else if (numread == (unsigned int)bytesToCopy)
	{
		bytesReturned = numread;
		if(bytesToCopy >= tmpCtx->bytesRemaining)
			tmpCtx->bytesRemaining = bytesToCopy - tmpCtx->bytesRemaining;
		else
			tmpCtx->bytesRemaining = tmpCtx->bytesRemaining - bytesToCopy;
	}
	else
	{
		/* Unknown error, close up and return zero */
		fclose(tmpCtx->dxlInputFile);
		bytesReturned = 0;
	}

	return(bytesReturned);
}

/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:     argc, argv	- directly from the command line
				impOptions	- needed to keep track of user requests
   

*************************************************************************/
STATUS  LNPUBLIC  ProcessArgs (int argc, char *argv[], struct ImportOptions *impOptions)
{
	
	DXLIMPORTOPTION			impAction;
	DXLLOGOPTION			logAction;
	Xml_Validation_Option	xmlValOpt;
	
	/* Check for empty command line. */
	if (argc == 1)
		return DXLI_RET_NO_ARGS;
	
	
	/* Set defaults in the ImportProperties structure */
	
	impAction = DXLIMPORTOPTION_IGNORE;
	impOptions->Properties.aclOpt = impAction;			/* ACL */
	impOptions->Properties.designOpt = impAction;		/* Design */
	impAction = DXLIMPORTOPTION_CREATE;
	impOptions->Properties.documentsOpt = impAction;	/* Documents */
	xmlValOpt = Xml_Validate_Auto;
	impOptions->Properties.inputValOpt = xmlValOpt;		/* Input Validation */
	impOptions->Properties.replicaRequiredForReplaceOrUpdate = TRUE;
	impOptions->Properties.exitOnFirstFatalError = TRUE;
	logAction = DXLLOGOPTION_FATALERROR;
	impOptions->Properties.tokenLogOpt = logAction;

	argc--, argv++;
	/* Process each argument. */
	for (; argc > 0; argc--, argv++)
	{	
		/* Check for option. */

		if(argv[0][0] == '-' || argv[0][0] == '/')
		{
						
			switch(argv[0][1])
			{
				case 'a':	/* ACL modification */
				case 'A':
						if (STRICMP(argv[0], "-acli") == 0)
							impAction = DXLIMPORTOPTION_IGNORE;
						else if (STRICMP(argv[0], "-aclri") == 0)
							impAction = DXLIMPORTOPTION_REPLACE_ELSE_IGNORE;
						else if (STRICMP(argv[0], "-aclui") == 0)
							impAction = DXLIMPORTOPTION_UPDATE_ELSE_IGNORE;
						else
							return DXLI_RET_INVALID_ARGS;
						impOptions->Properties.aclOpt = impAction;
					break;

				
				case 'd':	/* design/document modification */
				case 'D':
						/* Check for modification options first. */
						if (STRICMP(argv[0], "-desc") == 0)
							impAction = DXLIMPORTOPTION_CREATE;
						else if (STRICMP(argv[0], "-desi") == 0)
							impAction = DXLIMPORTOPTION_IGNORE;
						else if (STRICMP(argv[0], "-desri") == 0)
							impAction = DXLIMPORTOPTION_REPLACE_ELSE_IGNORE;
						else if (STRICMP(argv[0], "-desrc") == 0)
							impAction = DXLIMPORTOPTION_REPLACE_ELSE_CREATE;

						else if (STRICMP(argv[0], "-docc") == 0)
							impAction = DXLIMPORTOPTION_CREATE;
						else if (STRICMP(argv[0], "-doci") == 0)
							impAction = DXLIMPORTOPTION_IGNORE;
						else if (STRICMP(argv[0], "-docri") == 0)
							impAction = DXLIMPORTOPTION_REPLACE_ELSE_IGNORE;
						else if (STRICMP(argv[0], "-docrc") == 0)
							impAction = DXLIMPORTOPTION_REPLACE_ELSE_CREATE;
						else if (STRICMP(argv[0], "-docui") == 0)
							impAction = DXLIMPORTOPTION_UPDATE_ELSE_IGNORE;
						else if (STRICMP(argv[0], "-docuc") == 0)
							impAction = DXLIMPORTOPTION_UPDATE_ELSE_CREATE;
						else
							return DXLI_RET_INVALID_ARGS;
					
						/* finish setting up if either design or document */
						if (STRNICMP(argv[0], "-des", 4) == 0)
							impOptions->Properties.designOpt = impAction;
						else if (STRNICMP(argv[0], "-doc", 4) == 0)
							impOptions->Properties.documentsOpt = impAction;
					break;

				case 'f':
				case 'F':
						if (STRICMP(argv[0], "-ftc") == 0)
						{
							impOptions->Properties.createFullText = TRUE;
						}
					break;

				case 'i':	/* DXL input source */
				case 'I':
						argc--, argv++;
						impOptions->pInputFile = argv[0];
					break;

				case 'n':	/* use non-validating parser */
				case 'N':
						if (STRICMP(argv[0], "-n") == 0)
						{
  							xmlValOpt = Xml_Validate_Never;
							impOptions->Properties.inputValOpt = xmlValOpt;
						}
						else if (STRICMP(argv[0], "-norepl") == 0)
							impOptions->Properties.replicaRequiredForReplaceOrUpdate = FALSE;
						else if (STRICMP(argv[0], "-nef") == 0)
							impOptions->Properties.exitOnFirstFatalError = FALSE;
						else
							return DXLI_RET_INVALID_ARGS;
					break;

				case 's':	/* Notes server name */
				case 'S':
						argc--, argv++;
						impOptions->serverName = argv[0];
					break;

				case 'r':	/* replace database properties */
				case 'R':
				        impOptions->Properties.replaceDbProp = TRUE;
					break;

				case 'u':	/* logoptions for unknown tokens */
				case 'U':
						if (STRICMP(argv[0], "-ui") == 0)
							logAction = DXLLOGOPTION_IGNORE;
						else if (STRICMP(argv[0], "-uw") == 0)
							logAction = DXLLOGOPTION_WARNING;
						else if (STRICMP(argv[0], "-ue") == 0)
							logAction = DXLLOGOPTION_ERROR;
						else if (STRICMP(argv[0], "-uf") == 0)
							logAction = DXLLOGOPTION_FATALERROR;
						else
							return DXLI_RET_INVALID_ARGS;
						impOptions->Properties.tokenLogOpt = logAction;
					break;

				default:
					return DXLI_RET_INVALID_ARGS;	/* invalid option */
			}
		}
		else
		{
			if (impOptions->fileDatabase == NULL)
				impOptions->fileDatabase = argv[0];
			else
				return DXLI_RET_INVALID_ARGS;
		}
	}

	/* 
	 * Make sure we got at least the DXL input source, and that we weren't
	 * expecting any additional filenames on the command line.
	 */

	if ((impOptions->pInputFile == NULL) || (impOptions->fileDatabase == NULL))
		return DXLI_RET_INVALID_ARGS;

	return DXLI_RET_NOERROR;
}


/************************************************************************

    FUNCTION:   PrintUsage

*************************************************************************/
void    LNPUBLIC  PrintUsage()
{
	PRINTLOG( "\nUSAGE : dxlimport [ options ]\n");

	PRINTLOG( "\noptions: \n");
	PRINTLOG( "         -i dxlfile: DXL input file or URI; use -i with no name for standard input\n");
	PRINTLOG( "         -s server:  Domino server; omit for a local database\n");
	PRINTLOG( "         -r          Replace existing database properties\n");
	PRINTLOG( "         -n:         Use non-validating parser (uses validating parser otherwise)\n");
	PRINTLOG( "         -nef:       No exit on first fatal error (try to continue after fatal error)\n");
	PRINTLOG( "         -ui:        Ignore unknown elements and attributes\n");
	PRINTLOG( "         -uw:        Log warning for unknown elements and attributes\n");
	PRINTLOG( "         -ue:        Log error for unknown elements and attributes\n");
	PRINTLOG( "         -uf:        Log fatal error for unknown elements and attributes\n\n");
	PRINTLOG( "         -acli:      Ignore DXL <acl> elements (default)\n");
	PRINTLOG( "         -aclri:     Replace existing ACL with DXL <acl>, else ignore\n");
	PRINTLOG( "         -aclui:     Update existing ACL from DXL <acl>, else ignore\n");
	PRINTLOG( "         -desc:      Create new design elements, leaving existing elements intact\n");
	PRINTLOG( "         -desi:      Ignore DXL design elements (default)\n");
	PRINTLOG( "         -desri:     Replace existing design elements with DXL, else ignore\n");
	PRINTLOG( "         -desrc:     Replace existing design elements with DXL, else create\n\n");
	PRINTLOG( "         -doci:      Ignore DXL <document> elements\n");
	PRINTLOG( "         -docc:      Create new documents (default)\n");
	PRINTLOG( "         -docri:     Replace existing documents with DXL, else ignore\n");
	PRINTLOG( "         -docrc:     Replace existing documents with DXL, else create\n");
	PRINTLOG( "         -docui:     Update existing documents from DXL, else ignore\n");
	PRINTLOG( "         -docuc:     Update existing documents from DXL, else create\n\n");
	PRINTLOG( "         -ftc:       Create full text index if a <fulltextsettings> element exists\n\n");
	PRINTLOG( "         -norepl:    Replace/update do not require database and DXL be replicas\n\n");
	
    return;
}


