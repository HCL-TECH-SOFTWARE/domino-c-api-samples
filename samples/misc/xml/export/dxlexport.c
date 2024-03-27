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
      
    PROGRAM:    dxlexport

    FILE:			dxlexport.c

    SYNTAX:			dxlexport [options] dbpath [notespec]

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

	
	DESCRIPTION:
	
		This sample illustrates how to export (NSF) data into XML format based on the options chosen.	
   

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* C include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(UNIX)
    #include <sys/types.h>
#endif

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <xml.h>
#include <nsfsearc.h>
#include <nsfdb.h>
#include <osfile.h>
#include <nif.h>
#include <osmem.h>
#include <idtable.h>
#include "dxlexport.h"

#if defined(CAPI_TESTING) 
#include "printlog.h" 
#else
 #define PRINTLOG printf 
 #define PRINTERROR(api_error, api_name) {\
 char    szErrorText[256] = { 0 };\
 OSLoadString(NULLHANDLE, ERR(api_error), szErrorText, sizeof(szErrorText));\
 fprintf(stderr, "[ERROR]:%s:%d:%s - %s", __FILE__,__LINE__,api_name,szErrorText); }
#endif 

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

#ifndef OSMISC_DEFS
#include <osmisc.h>
#endif


int main(int argc, char *argv[])
{
    STATUS		error =  NOERROR;
    struct		ExportOptions	expOptions;
    DBHANDLE	hDB;

    char		pname[MAXPATH] = "";         /* buffer to store the input path to database */
    char		*path_name;					/* pathname of database */


    if (error = NotesInitExtended (argc, argv))
    {
        PRINTLOG( "\n Unable to initialize Notes.\n");
        return (1);
    } 

    PRINTLOG(  "DXLEXPORT Utility\n" );

    expOptions.pOutputFile = stdout;

    memset(&expOptions, 0, sizeof(expOptions));
    error = ProcessArgs( argc, argv, &expOptions );
   
    /* ProcessArgs checks the syntax, etc. */
    if (error)
    {
        NotesTerm(); /* ProcessArgs already printed an error message */ 
        return (0);
    }

    path_name = expOptions.fileDatabase;		/* Assign Database to Export */

    if (expOptions.serverName != NULL)			/* Check to see if user supplied a servername */
    {
        if (error = OSPathNetConstruct( NULL, expOptions.serverName, expOptions.fileDatabase, pname))
        {
            PRINTERROR(error,"OSPathNetConstruct");
            NotesTerm();
            return(1);
        }
        path_name = pname;
    }

    if(expOptions.pOutputFile == NULL)			/* Assign pOutputFile to stdout if not specified. */
        expOptions.pOutputFile = stdout;

    PRINTLOG( "dxlexport: exporting '%s'\n", path_name );

    if (error = NSFDbOpen( path_name, &hDB ))	/* Open the Domino/Notes Database */
    {
        PRINTLOG(  "Error: unable to open '%s.'\n", path_name );
        PRINTERROR (error,"NSFDbOpen"); 
        NotesTerm();
        return (1);
    }

    if(expOptions.noteClassMask != 0)			/* Check to see if exporting a class of Notes */
        error = ExportSetofNotes(&expOptions, hDB);
    else
        error = ExportData(&expOptions, hDB);	/* Otherwise exporting, Database, ACL or specific note */

    if(error)
    {
        NSFDbClose(hDB);
        PRINTERROR(error,"NSFDbOpen");
        NotesTerm();
         return(1);
    }

    NSFDbClose( hDB );

    fflush(expOptions.pOutputFile);

    PRINTLOG(  "\n\nDXLEXPORT: Done.\n" );
    if ( stdout != expOptions.pOutputFile ) 
        fclose(expOptions.pOutputFile);
    if (error == NOERROR)
        PRINTLOG("\nProgram completed successfully.\n");
        NotesTerm();
        return (0);
}

/************************************************************************
                                                                        
	FUNCTION:   ExportData     
	
	This function exports either a full Database, the ACL of a Database,
	or a specific Note.
                                                                        
*************************************************************************/
STATUS LNPUBLIC ExportData( struct ExportOptions *expOptions, DBHANDLE hDB )
{

    STATUS				error = NOERROR;
    DXLEXPORTHANDLE			hDXLExport;			/* Handle need for all DXL Export functions */
    NOTEHANDLE			noteHandle;			/* Handle of specific note going to export */


    /* Assign the DXLEXPORTHANDLE for this Export Session */
    if(error = DXLCreateExporter (&hDXLExport))
        return(error);

    /* Check our DXL_EXPORT_PROPERTY options and set appropriately */
    if(error = SetExportOptions(hDXLExport, expOptions))
        return(error);

    /* 
     * Find out what is to be exported.
     */
    /*
     * Check to see if user wants to Export the Database ACL
     */
    if(expOptions->whatToExport == EXP_ACL)
        error = DXLExportACL( hDXLExport, DXLExportStreamFunc, hDB, (void far *)expOptions);

    /*
     * Check to see if the user wants to export a full Database 
     */
    else if(expOptions->whatToExport == EXP_DATABASE)
        error = DXLExportDatabase( hDXLExport, DXLExportStreamFunc, hDB, (void far *)expOptions);

    /*
     * Check to see if there is a specific note to be exported via the NoteID or a Note Design Name
     */
    else if((expOptions->noteID != 0) || (expOptions->noteSpec != NULL))
    {
        if(expOptions->noteSpec != NULL)
        {
            /*
             * Want to export a design note, see if it can be found
             */
            if(error = NIFFindDesignNote(hDB, expOptions->noteSpec, NOTE_CLASS_ALL, &expOptions->noteID))
            {
                DXLDeleteExporter(hDXLExport);
                return(error);
            }
        }
        /*
         * See if the NoteID wanted to export can be found 
         */
        if(error = NSFNoteOpenExt(hDB, expOptions->noteID, 0, &noteHandle))
        {
            DXLDeleteExporter(hDXLExport);
            return(error);
        }
        error = DXLExportNote( hDXLExport, DXLExportStreamFunc, noteHandle, (void far *)expOptions);
        NSFNoteClose(noteHandle);
    }
    else
        /* Should not get here, this is an invalid request for what to export */
        PRINTLOG(  "\nInvalid request on what to Export\n");

    /*
     * Find out if there are any errors to report.
     */
    CheckDXLErrorLog(hDXLExport);

    /* Free and Unlock the DXLEXPORTHANDLE */
    DXLDeleteExporter(hDXLExport);
    return (error);
}
/************************************************************************
                                                                        
	FUNCTION:   ExportSetofNotes                                 

	This function exports a set of notes based on the Note Class(es)
	specified.  (NOTE_CLASS_xxx)
                                                                        
*************************************************************************/
STATUS LNPUBLIC ExportSetofNotes( struct ExportOptions *expOptions, DBHANDLE hDB )
{
    STATUS			error = NOERROR;
    DXLEXPORTHANDLE		hDXLExport;
    DHANDLE			hNoteIDTable;   /* table of Note IDs to modify */

    /*
     * Need to set up an ID Table which will contain a list of Note IDs that are to be exported.
     */
    if (error = IDCreateTable(sizeof(NOTEID), &hNoteIDTable))
        return(error);

    /* 
     * Call NSFSearch to find the notes that match the note classes we would like to export. 
     * For each note found, the routine AddIdUnique is called.
     */

    if (error = NSFSearch (
                           hDB,						/* database handle */
                           NULLHANDLE,				/* selection formula */
                           NULL,					/* title of view in selection formula */
                           0,						/* search flags */
                           expOptions->noteClassMask,    /* note class to find */
                           NULL,					/* starting date (unused) */
                           AddIDUnique,				/* call for each note found */
                           &hNoteIDTable,			/* argument to AddIDUnique */
                           NULL))					/* returned ending date (unused) */

    {
        IDDestroyTable(hNoteIDTable);
        return(error);
    }

    /* 
     * Get our DXLEXPORTHANDLE, our session started 
     */
    if(error = DXLCreateExporter (&hDXLExport))
    {
        IDDestroyTable(hNoteIDTable);
        return(error);
    }

    /* 
     * Check our DXL_EXPORT_PROPERTY options and set appropriately 
     */
    if(error = SetExportOptions(hDXLExport, expOptions))
    {
        IDDestroyTable(hNoteIDTable);
        return(error);
    }

    /*
     * Export the set of notes, based on noteClassMask that are to be exported.
     */
    if( error = DXLExportIDTable(hDXLExport, DXLExportStreamFunc, hDB, hNoteIDTable, (void far *)expOptions))
    {	
        CheckDXLErrorLog(hDXLExport);
        DXLDeleteExporter(hDXLExport);
        IDDestroyTable(hNoteIDTable);
        return(error);
    }

    /*
     * Find out if there are any errors to report.
     */
    CheckDXLErrorLog(hDXLExport);
    
    /* 
     * Free and Unlock the DXLEXPORTHANDLE 
     */
    DXLDeleteExporter(hDXLExport);

    /* 
     * Free the memory containing the ID table
     */
    IDDestroyTable(hNoteIDTable);

    return (error);
}
/************************************************************************
                                                                        
    FUNCTION:   SetExportOptions                            
                                                                        
*************************************************************************/
STATUS LNPUBLIC SetExportOptions(DXLEXPORTHANDLE hDXLExport, struct ExportOptions *expOptions)
{
	STATUS				error = NOERROR;
	DXL_EXPORT_PROPERTY		propValue;
	BOOL				setGetValue;
	DXL_RICHTEXT_OPTION		richOpt;
	char				bannerComment[100];

	
	/*
	 * First set up a Banner Comment
	 */
	memset(bannerComment, 0, sizeof(bannerComment));
	propValue = eDXLBannerComments;
	if(error = DXLSetExporterProperty(hDXLExport, propValue, &bannerComment))
	{
		DXLDeleteExporter(hDXLExport);
		return(error);
	}

	/* 
	 * User has requested to specify a DTD to use
	 */
	if(expOptions->docDTD != NULL)
	{
		propValue = eDoctypeSYSTEM;
		if(error = DXLSetExporterProperty(hDXLExport, propValue, expOptions->docDTD))
		{
			DXLDeleteExporter(hDXLExport);
			return(error);
		}
	}

	/* 
	 * User has requested to TURN OFF the forcing of Notes Format, but rather
	 * requests a higher level of data abstraction.
	 */
	if(expOptions->forceNoteFormat)
	{
		propValue = eForceNoteFormat;
		setGetValue = FALSE;
		if(error = DXLSetExporterProperty(hDXLExport, propValue, &setGetValue))
		{
			DXLDeleteExporter(hDXLExport);
			return(error);
		}
	}

	/* 
	 * User has requested to output richtext as uninterpretted (base64'ed) item data.
	 */
	if(expOptions->richTextOption)
	{
		propValue = eDxlRichtextOption;
		richOpt = eRichtextAsItemdata;
		if(error = DXLSetExporterProperty(hDXLExport, propValue, &richOpt))
		{
			DXLDeleteExporter(hDXLExport);
			return(error);
		}
	}
return(error);
}
/************************************************************************
                                                                        
    FUNCTION:   DXLExportStreamFunc                                       
                                                                        
*************************************************************************/
void LNCALLBACK DXLExportStreamFunc ( const BYTE *bytes, DWORD length, void far *expCtx )
{

	struct ExportOptions *tmpCtx = (struct ExportOptions*)expCtx;
	DWORD numberWritten = 0;

	
	numberWritten = fwrite( bytes, sizeof(BYTE), length, tmpCtx->pOutputFile);
	if( numberWritten != length )
	{
		PRINTLOG(  "Error: Did not write specified bytes required %d\n", length );
	}
	else
	{
		tmpCtx->totalBytesWritten += numberWritten;
	}


	return;
}

/************************************************************************
                                                                        
    FUNCTION:	CheckDXLErrorLog                             
                                                                        
*************************************************************************/
void LNCALLBACK CheckDXLErrorLog( DXLEXPORTHANDLE hDXLExport)
{

	DXL_EXPORT_PROPERTY		propValue;
	MEMHANDLE			hMem;
	char				*pData;

	if(DXLExportWasErrorLogged(hDXLExport))
	{
		/* eDxlExportResultLog */
		propValue = eDxlExportResultLog;
		if(DXLGetExporterProperty(hDXLExport, propValue, &hMem))
		{
			PRINTLOG( "\nNot Able to Retrieve Error Log Information!!!\n");
			return;
		}
		pData = (char *)OSMemoryLock(hMem);
		PRINTLOG( "\nResultLog = %s\n", pData);
		OSMemoryUnlock(hMem);
		OSMemoryFree(hMem);
	}
	return;
}

/************************************************************************
                                                                        
    FUNCTION:   AddIDUnique                                             
                                                                        
*************************************************************************/
STATUS LNPUBLIC AddIDUnique    
                           (void far * phNoteIDTable,
                            SEARCH_MATCH far *pSearchMatch,
                            ITEM_TABLE far *summary_info)
{
   SEARCH_MATCH SearchMatch;
   DHANDLE      hNoteIDTable;
   STATUS       error;
   BOOL         flagOK;

    memcpy( (char*)&SearchMatch, (char*)pSearchMatch, sizeof(SEARCH_MATCH) );

   /*  if (SearchMatch.MatchesFormula != SE_FMATCH)      V3 */
   if (!(SearchMatch.SERetFlags & SE_FMATCH))     /* V4 */
      return (NOERROR);

   hNoteIDTable = *((DHANDLE far *)phNoteIDTable);

   error = IDInsert(hNoteIDTable, SearchMatch.ID.NoteID, &flagOK);

   return (ERR(error));
}

/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:     argc, argv - directly from the command line
   

*************************************************************************/

STATUS  LNPUBLIC  ProcessArgs (int argc, char *argv[], struct ExportOptions *expOptions)
{
    if (argc == 1)   
    {
        PrintUsage();
        return (ERR_INVALID_ARGS);
    }
	
    argc--, argv++;
    for (; argc > 0; argc--, argv++) {
        if (argv[0][0] == '-' || argv[0][0] == '/')
        {
            /*  Process options. */

            switch (argv[0][1])
            {
			case 'a':	/* 	Export all notes, not just data notes. */
				expOptions->whatToExport = EXP_DATABASE;
				break;

			case 'c':	/*	Specify a note class	*/
				argc--, argv++;
				expOptions->noteClassMask = (WORD) atoi(argv[0]);
				break;

			case 'd':	/*  The filename of the DTD */
				argc--, argv++;
				expOptions->docDTD = argv[0];
				break;
			
			case 'l':	/*	Output acl entry */
				expOptions->whatToExport = EXP_ACL;
				break;

			case 'n':	/*	Output as <note> */
				expOptions->forceNoteFormat = (TRUE);
				break;

			case 'o':	/*	Output to file */
				argc--, argv++;
				/* open the file and assign to dump file pointer */
				if ((expOptions->pOutputFile = fopen(argv[0],"w+")) == NULL)
				{
					PRINTLOG(  
                       "Error: dump output file could not be opened.\n");
					return (ERR_INVALID_ARGS);
				}                
				break;

			case 'r':	/*	Write rich text items as uninterpreted data (<itemdata>) */
				expOptions->richTextOption = TRUE;
				break;

			case 's':	/*	Specify server */
				argc--, argv++;
				expOptions->serverName = argv[0];
				break;
			
            default:    /* Invalid Option Specified */
                PRINTLOG ( "Didn't understand option %s\n", argv[0]);
				PrintUsage();
				return(ERR_INVALID_ARGS);
            }
        }
        else
        {
			if (expOptions->fileDatabase == NULL)
				expOptions->fileDatabase = argv[0];
			else if (expOptions->noteSpec == NULL)
			{
				expOptions->noteSpec = argv[0];
				/* Preliminary check to see if Note ID was passed in */
				if(strncmp(expOptions->noteSpec, "0x", 2) == 0)
				{
					if(sscanf(expOptions->noteSpec+2, "%x", &expOptions->noteID) != 1 || expOptions->noteID == 0)
					{
						PRINTLOG( "\nCouldn't parse noteid '%s'\n", expOptions->noteSpec);
						PrintUsage();
						return(ERR_INVALID_ARGS);
					}
					/* Since it looks like we have a noteID and not a design note, clear out noteSpec */
					expOptions->noteSpec = NULL;
				}
			}
			else
			{
				PRINTLOG ( "Too many args: %s\n", argv[0]);
				PrintUsage();
				return(ERR_INVALID_ARGS);
			}
        }
    }
	if(expOptions->fileDatabase == NULL)
	{
		PRINTLOG( "\nYou must specify a Database Name to export from. \n");
		PrintUsage();
		return(ERR_INVALID_ARGS);
	}
	return(NOERROR);
}


/************************************************************************

    FUNCTION:   PrintUsage

*************************************************************************/
void    LNPUBLIC  PrintUsage()
{
    PRINTLOG(  "\nUSAGE : dxlexport [options] dbpath [notespec]\n" );
	PRINTLOG( "\ndbpath    - is relative to the Domino data directory\n");
	PRINTLOG( "notespec  - is either a design element name, or a hex noteid (ie. 0x211a)\n");
	PRINTLOG( "\noptions : \n");
	PRINTLOG( "         -a Write all notes in the database (including acl and design elements)\n");
	PRINTLOG( "         -c <number> Write all notes with the given note class(es).\n");
	PRINTLOG( "         -d <file> Write a DOCTYPE declaration using the given file\n");
	PRINTLOG( "         -l Write acl elements only.\n");
	PRINTLOG( "         -n Write all notes as <note>.\n");
	PRINTLOG( "         -o <file> Write the DXL to the file (default is standard out)\n");
	PRINTLOG( "         -s <server> Use the database on the specified Domino server\n");
	PRINTLOG( "         -r Write rich text items as uninterpreted data (<itemdata>) \n");
   
    return;
}

