/*************************************************************************
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

   PROGRAM:    ARCHEXP

   FILE:       archexp.c

   PURPOSE:    Demonstrates the ArchiveExportDatabase API  
   	
   SYNTAX:     archexp  <source DB>	<hex note ID> <output file> [options]
  
   DESCRIPTION:
        This program shows how to archive a document from one database to an
        output file. Otherwise the related attachments of the document will
        be archived too. And all the output file will store in the current 
        directory.

*************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* OS and C include files */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>


/* HCL C API for Notes/Domino include files */

#include "global.h"
#include "nsfdb.h"
#include "nsfdata.h"
#include "osmisc.h"
#include "osfile.h"
#include "idtable.h"
#include "oserr.h"
#include "../../printLog.h"

/* archivie services API file */

#include "archsvc.h"


/* Local function prototypes */

BOOL  LNPUBLIC  ProcessArgs(int argc, char* argv[], char *dbPath, NOTEID* pNoteID, char *pOutFileName);


/* Callbacks for ArchiveExportDatabase */

STATUS far PASCAL NoteInitCallback(NOTEHANDLE hNote, STATUS retError, void *pUserCtx );
STATUS far PASCAL AttachInitCallback(const char *szFileName, 
                                     DWORD dwFlags,
                                     DWORD dwDupIdx, 
                                     STATUS retError,
                                     void *pUserCtx, 
                                     DWORD dwUnused, 
                                     const BYTE* pUnused);
STATUS far PASCAL AttachOutputCallback(const BYTE *Buffer, DWORD BufferSize, BOOL bLastBuffer,  STATUS retError,void *pUserCtx );
STATUS far PASCAL ArchiveDocumentCallback(HARCHIVEDOCUMENT hDoc, void *pUserCtx);

/* Callback for ArchiveDocumentExport */
STATUS far PASCAL NoteExportCallback(const BYTE *Buffer, DWORD dwBufferSize, BOOL bLastBuffer,STATUS retError, void *pUserCtx);

typedef struct {
    FILE *pOutFile;
    FILE *pAttachFile;
    
} EXPORTCONTEXT;

/************************************************************************

    FUNCTION:   Main

    PURPOSE:    HCL C API for Notes/Domino subroutine

*************************************************************************/

int main(int argc, char *argv[])
{
    /* Local data declarations */

    char        pname[MAXPATH] = "";         /* buffer to store the input path to database */
    DBHANDLE        db_handle = NULLHANDLE;      /* database handle */
    STATUS        error = NOERROR;             /* error code from API calls */
    NOTEID        NoteID;
    DHANDLE        hIDTable = NULLHANDLE;
    EXPORTCONTEXT          Ctx;
    char        achOutFileName[MAXPATH+1];


    memset(&Ctx, 0, sizeof(EXPORTCONTEXT));

    
    error = NotesInitExtended (argc, argv);
    if (error)
    {
        fprintf (stderr, "\nError initializing Notes.\n");
        return (1);
    }


    if(!ProcessArgs(argc, argv, pname, &NoteID, achOutFileName))
    {
        PRINTLOG( "\nUsage:  %s  <database filename> <hex NoteID> <Output file> [options]\n", argv[0] );
        PRINTLOG("\nOptions: -s ServerName\n"); 
        return (0);

    }

    /* Open source database. */	
    
    PRINTLOG("Opening %s\n", pname);
    if (error = NSFDbOpen (pname, &db_handle))
    {
        PRINTERROR (error,"NSFDbOpen");
        goto cleanup;
    }

    /* Create an IDTable, and insert note ID into this IDTable. */

    PRINTLOG("Creating ID table for note %x\n", NoteID);	
    if(error = IDCreateTable(sizeof(NOTEID), &hIDTable))
    {
        PRINTERROR(error,"IDCreateTable");
        goto cleanup;
    }
    if(error = IDInsert(hIDTable, NoteID, NULL))
    {
        PRINTERROR(error,"IDInsert");
        goto cleanup;
    }

    /* Open the output file. */

    Ctx.pOutFile = fopen(achOutFileName, "wb");
    if(Ctx.pOutFile == NULL)
    {
        PRINTLOG("Error creating noteexp.dat\n");
        goto cleanup;
    }

    
    /*  The ArchiveExportDatabase function takes a db handle an IDTable and streams the identified documents to the user-supplied
    *   callback functions described below */
    
    if(error = ArchiveExportDatabase(db_handle, 
                                     hIDTable, 
                                     0, 
                                     NoteInitCallback,
                                     AttachInitCallback,
                                     AttachOutputCallback,
                                     ArchiveDocumentCallback,
                                     NULL, &Ctx))
    {
        PRINTERROR(error,"ArchiveExportDatabase");
        goto cleanup;	   
    }
  
cleanup:

    if(hIDTable)
        IDDestroyTable(hIDTable);

    /* close the output file. */
    if(Ctx.pOutFile)
        fclose(Ctx.pOutFile);

    /* Close the database. */
    if(db_handle)
        NSFDbClose (db_handle);    


    /* Terminate Domino and Notes. */
    NotesTerm();

    /* End of archexp program. */
    return (0);
}


/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:     argc, argv - directly from the command line

    OUTPUTS:    source db path(include db name), note ID and the output
	            FileName, inputted by user

 *************************************************************************/

BOOL LNPUBLIC  ProcessArgs(int argc, char* argv[], char *dbPath, NOTEID* pNoteID, char *pOutFileName)
{
    int curarg = 1;
    char* pDBName;
    char* pServerName = NULL;
    STATUS Error = NOERROR;

    if(argc < 4)
        return FALSE;

    if(argv[curarg][0] == '=')
        curarg++;

    pDBName = argv[curarg];

    curarg++;

    if(curarg == argc)
    {
        PRINTLOG("Missing required note ID argument\n");
        return FALSE;
    }
    
    if(sscanf(argv[curarg], "%x", pNoteID) != 1)
    {
        PRINTLOG("Unable to convert %s to a NoteID\n",argv[curarg]); 
        return 0;
    }

    curarg++;

    if(curarg == argc)
    {
        PRINTLOG("Missing required output file argument\n");
        return FALSE;
    }

    strncpy(pOutFileName, argv[curarg], MAXPATH);

    curarg++;

    while(curarg < argc)
    {
        if(argv[curarg][0] == '-' &&  argv[curarg][1] == 's')
        {
            curarg++;
            if(curarg == argc)
            {
                PRINTLOG("ERROR: Missing server argument after -s\n");
                return FALSE;
            }
            pServerName = argv[curarg];					
        }
        curarg++;
    }

    if (Error = OSPathNetConstruct( NULL, pServerName, pDBName, dbPath))
    {
        PRINTERROR (Error,"OSPathNetConstruct");
        return FALSE;
    }

    return TRUE;  
} /* ProcessArgs */


/************************************************************************

    CALLBACK:   NoteInitCallback

    PURPOSE:    Make initialization of a note export.

    DESCRIPTION:
				Called by ArchiveExportDatabase when a note begin to be exported.
*************************************************************************/

STATUS far PASCAL NoteInitCallback(NOTEHANDLE hNote,STATUS retError, void *pUserCtx )
{
    STATUS Error = NOERROR;
    
    if(retError)
    {
        if(hNote)
            NSFNoteClose(hNote);
        Error= retError;
        goto cleanup;
    }
    
    
cleanup:					

    return Error;
}

/************************************************************************

    CALLBACK:   AttachInitCallback

    PURPOSE:    Make initialization of attachment data export, such as open 
                the attachment output file.

    DESCRIPTION:
        Called by ArchiveExportDatabase when attachment data begin 
        to be exported.
*************************************************************************/

STATUS far PASCAL AttachInitCallback(const char *szFileName, 
                                     DWORD dwFlags, 
                                     DWORD dwDupIdx, 
                                     STATUS retError,
                                     void *pUserCtx,
                                     DWORD dwUnused, 
                                     const BYTE* pUnused)
{
    STATUS Error = NOERROR;
    EXPORTCONTEXT *pCtx = (EXPORTCONTEXT *)pUserCtx;	
    PRINTLOG("AttachInitCallback called\n");

    if(retError)
    {
        Error= retError;
        goto cleanup;
    }

/* open the file, later the attachment will be exported there. */

    pCtx->pAttachFile = fopen(szFileName, "wb");
    
    if (pCtx->pAttachFile == NULL)
    {
        Error = ERR_CREATE;
        goto cleanup;
    }
        
cleanup:

    return Error;
}

/************************************************************************

    CALLBACK:   AttachOutputCallback

    PURPOSE:    Export the attachment data.

    DESCRIPTION:
        Called by ArchiveExportDatabase when attachment data is 
        being exported.
*************************************************************************/

STATUS far PASCAL AttachOutputCallback(const BYTE *Buffer, DWORD BufferSize, BOOL bLastBuffer,  STATUS retError,void *pUserCtx)
{
    STATUS Error = NOERROR;	
    EXPORTCONTEXT *pCtx = (EXPORTCONTEXT *)pUserCtx;
    PRINTLOG("AttachOutputCallback called\n");

    if(retError)
    {
        Error=retError;
        goto cleanup; 
    }

/* export attachment data into attachment output file. */
    if(pCtx->pAttachFile)
        fwrite(Buffer, BufferSize, 1, pCtx->pAttachFile);		

cleanup:
    if (Error ||bLastBuffer && pCtx->pAttachFile)
    {
        fclose(pCtx->pAttachFile);
        pCtx->pAttachFile = NULL;
    }
    return Error;
}


/************************************************************************

    CALLBACK:   ArchiveDocumentCallback

    PURPOSE:    Export the document data.

    DESCRIPTION:
		ArchiveExportDatabase passes a serialized notes document via this function. 

*************************************************************************/

STATUS far PASCAL ArchiveDocumentCallback(HARCHIVEDOCUMENT hDoc, void *pUserCtx)
{
    STATUS Error = NOERROR;
    EXPORTCONTEXT *pExp = (EXPORTCONTEXT *)pUserCtx;

    if(Error = ArchiveDocumentExport(hDoc, NoteExportCallback, pExp))
    {
        goto cleanup;
    }

cleanup:

    if(hDoc)
        ArchiveDocumentDestroy(hDoc);
    
    return Error;
}

/************************************************************************

    CALLBACK:   NoteExportCallback

    PURPOSE:    Export the document data.

    DESCRIPTION:
        ArchiveExportDatabase passes a serialized notes document via this function. 
        The contents of Buffer are opaque and intended to be used as input to the 
        ArchiveImportDocument function. This function can be called multiple times 
        for a note. bLastBuffer will be TRUE on the last call for a note.
*************************************************************************/

STATUS far PASCAL NoteExportCallback(
                                     const BYTE *Buffer,
                                     DWORD dwBufferSize,
                                     BOOL bLastBuffer, 
                                     STATUS retError,
                                     void *pUserCtx
                                     )
    {
        STATUS Error = NOERROR;
        EXPORTCONTEXT *pExp = (EXPORTCONTEXT *)pUserCtx;

    if(retError)
    {
        Error= retError;
        goto cleanup;
    }

    if(pExp->pOutFile == NULL)
    {
        PRINTLOG("ERROR: ArchSvcCrlExporter::NoteExportCallback: Cannot open output file\n");
        goto cleanup;
    } 

    fwrite(Buffer,dwBufferSize,1,pExp->pOutFile);
    

cleanup:

    
    if(bLastBuffer && pExp->pOutFile != NULL)
    {
        fclose(pExp->pOutFile);
        pExp->pOutFile=NULL;
    }

    return Error;
}
