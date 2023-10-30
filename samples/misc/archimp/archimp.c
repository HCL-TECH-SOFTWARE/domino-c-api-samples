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

   PROGRAM:    ARCHIMP

   FILE:       archimp.c

   PURPOSE:    Demonstrates the ArchiveDocumentImport, ArchiveRestoreDocument API  

   SYNTAX:     archimp  <Input file> <Target database> [server name - optional]

   DESCRIPTION:
        This program shows how to restore the archived file back to a note in a db. 

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

BOOL  LNPUBLIC  ProcessArgs(int argc, char* argv[], char *dbPath, char *pInFileName);


/* Callback for ArchiveDocumentImport */

DWORD far PASCAL NoteImportCallback
                                   (BYTE *Buffer,
                                    DWORD MaxToRead,
                                    void *pUserCtx);

/* Callback for ArchiveRestoreDocument */

STATUS far PASCAL  AttachImportCallback
                                       (const char *FileName,
                                        DWORD FileNameLen,
                                        DWORD dwDupIdx,
                                        BYTE *Buffer,
                                        DWORD MaxToRead,
                                        void *pUserCtx);


typedef struct {
    FILE *pInFile;
    FILE *pAttachFile;
    DWORD dwBytesLeft;
} IMPORTCONTEXT;


/************************************************************************

    FUNCTION:   Main

    PURPOSE:    HCL C API for Notes/Domino subroutine

*************************************************************************/

int main(int argc, char *argv[])
{
    /* Local data declarations */

    char       	pname[MAXPATH] = "";         /* buffer to store the input path to database */
    DBHANDLE   	db_handle = NULLHANDLE;		 /* database handle */
    STATUS     	error = NOERROR;             /* error code from API calls */
    IMPORTCONTEXT Ctx;
    char achInFileName[MAXPATH+1];
    NOTEHANDLE hNote = NULLHANDLE;
    HARCHIVEDOCUMENT hDoc = NULLHANDLE;

    memset(&Ctx, 0, sizeof(IMPORTCONTEXT));

    error = NotesInitExtended (argc, argv);
    if (error)
    {
        fprintf (stderr, "\nError initializing Notes.\n");
        return (1);
    }

    if(!ProcessArgs(argc, argv, pname, achInFileName))
    {
        PRINTLOG( "\nUsage:  %s  <Input file> <Target database> [options]\n", argv[0] );
        PRINTLOG("\nOptions: -s ServerName\n"); 
        return (0);
    }
    
    PRINTLOG("Opening %s\n", pname);

    /* Create the database if it doesn't exist */
    error = NSFDbCreate(pname, DBCLASS_NOTEFILE, FALSE);
    if(error && ERR(error) != ERR_EXISTS)
    {
        PRINTERROR (error,"NSFDbCreate");
        goto cleanup;
    }
    
    /* open the db. */
    if (error = NSFDbOpen (pname, &db_handle))
    {
        PRINTERROR (error,"NSFDbOpen");
        goto cleanup;
    }

    /* open the input file. */
    Ctx.pInFile = fopen(achInFileName, "rb");

    if(Ctx.pInFile == NULL)
    {
        PRINTLOG("Error creating %s\n",achInFileName);
        goto cleanup;
    }
    else
    {
        PRINTLOG("Input file successfully opened\n");
    }

    fseek(Ctx.pInFile, 0, SEEK_END);
    Ctx.dwBytesLeft = ftell(Ctx.pInFile);
    rewind(Ctx.pInFile);

    
    /* ArchiveDocumentImport will call NoteImportCallback to read bytes from the input file.
     * This file should contain the output produced by ArchiveExportDatabase NoteExportCallback function.
     * See the archexp sample for more details */	
    
    if(error = ArchiveDocumentImport(0, NoteImportCallback, &Ctx, &hDoc))
    {
        PRINTERROR(error,"ArchiveDocumentImport");
        goto cleanup;	   
    }
    else
    {
        PRINTLOG("Successfully executed ArchiveDocumentImport API\n");
    }

    /*  The ArchiveRestoreDocument function restores a note and its related attachments from the data stream
    *   produced by ArchiveExportDatabase. The document restored will have the same UNID as the one exported.
    * 	If a document with that UNID exists, it will be replaced with the document produced by this function. 
    * 	See below for NoteImportCallback and AttachImportCallback definitions 
    *	This function optionally returns an hNote of the note that was created. */

    if(error = ArchiveRestoreDocument(db_handle, 0, hDoc,  AttachImportCallback, &Ctx, &hNote))
    {
        PRINTERROR(error,"ArchiveRestoreDocument");
        goto cleanup;	   
    }
    else
    {
        PRINTLOG("Succesfully executed ArchiveRestoreDocument API\n");
    }

    ArchiveDocumentDestroy(hDoc);
    hDoc = NULLHANDLE;

    PRINTLOG("Program completed successfully\n");

cleanup:

    if(hNote)
        NSFNoteClose(hNote);

    if(Ctx.pInFile)
        fclose(Ctx.pInFile);

    /* Close the database. */
    if(db_handle)
        NSFDbClose (db_handle);    


    /* Terminate Domino and Notes. */

    NotesTerm();

    /* End of archimp program. */

    return (0);
}


/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:     argc, argv - directly from the command line

    OUTPUTS:    DBFileName, input file name, inputted by user

 *************************************************************************/

BOOL LNPUBLIC  ProcessArgs(int argc, char* argv[], char *dbPath, char *pInFileName)
{
    int curarg = 1;
    char* pDBName;
    char* pServerName = NULL;
    STATUS Error = NOERROR;

    if(argc < 3)
        return FALSE;

    if(argv[curarg][0] == '=')
        curarg++;

    strncpy(pInFileName, argv[curarg], MAXPATH);

    curarg++;

    if(curarg == argc)
    {
        PRINTLOG("Missing required target database\n");
        return FALSE;
    }
    
    pDBName = argv[curarg];	
    
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

    CALLBACK:   NoteImportCallback

    PURPOSE:    This is the callback that ArchiveDocumentImport uses to retrieve 
				data from the note.

    DESCRIPTION:
        It will be called repeatedly until the amount returned is less than the
        MaxToRead argument. In some cases, that means that it will be called 
        with MaxToRead == 0
*************************************************************************/

DWORD far PASCAL NoteImportCallback
                                   (BYTE *Buffer,
                                    DWORD MaxToRead,
                                    void *pUserCtx)
{
    IMPORTCONTEXT *pCtx = (IMPORTCONTEXT *)pUserCtx;
    DWORD dwBytesToRead =  MaxToRead <= pCtx->dwBytesLeft ? MaxToRead : pCtx->dwBytesLeft; 
    
    fread(Buffer, dwBytesToRead, 1, pCtx->pInFile);
    
    pCtx->dwBytesLeft -= dwBytesToRead;
    return dwBytesToRead;
}

/************************************************************************

    CALLBACK:   AttachImportCallback

    PURPOSE:    This is the callback that ArchiveRestoreDocument uses to retrieve 
                data from attachment.

    DESCRIPTION:
        It will be called repeatedly until the amount returned is less than the
        MaxToRead argument. In some cases, that means that it will be called 
        with MaxToRead == 0
*************************************************************************/

STATUS far PASCAL AttachImportCallback
                                      (const char *FileName,
                                       DWORD FileNameLen,
                                       DWORD dwDupIdx,
                                       BYTE *Buffer,
                                       DWORD MaxToRead,
                                       void *pUserCtx)
{
    IMPORTCONTEXT *pCtx = (IMPORTCONTEXT *)pUserCtx;
    DWORD dwBytesToRead = 0;
    STATUS Error = NOERROR;


    if (pCtx->pAttachFile == NULL)
    {

        /* We'll read bytes from the input file and pass back to ArchiveDocumentImport */
        /* This file should contain the output produced by ArchiveExportDatabase NoteExportCallback function */
        /* See the archexp sample for more details */
        pCtx->pAttachFile = fopen(FileName, "rb");

        if(pCtx->pAttachFile == NULL)
        {
            PRINTLOG("Error opening %s\n", FileName);
            goto cleanup;
        }

        fseek(pCtx->pAttachFile, 0, SEEK_END);
        pCtx->dwBytesLeft = ftell(pCtx->pAttachFile);
        rewind(pCtx->pAttachFile);
    }

    dwBytesToRead =  MaxToRead <= pCtx->dwBytesLeft ? MaxToRead : pCtx->dwBytesLeft; 
    
    fread(Buffer, dwBytesToRead, 1, pCtx->pAttachFile);
    
    pCtx->dwBytesLeft -= dwBytesToRead;

    if (pCtx->dwBytesLeft == 0)
    {
        fclose(pCtx->pAttachFile);
        pCtx->pAttachFile = NULL;
    }

cleanup:

    return Error;

}



