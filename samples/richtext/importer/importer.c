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

    PROGRAM:    importer

    FILE:       importer.c

    PURPOSE:    load and call a Domino and Notes import/export library to convert
                a file to Domino and Notes rich text format, then import the result
                to a new document in a Domino database.

    SYNTAX:     importer <DBname> <DLL Path> <File Path> [<Second DLL Name>]

    DESCRIPTION:
        Creates a new note in a specified database and imports
        a file into the Body field of the new note.  The file
        imported may be in one of several formats. The
        accompanying README.TXT file explains the formats
        supported and the DLLs that are needed for each.

        IMPORTER takes either three or four arguments:

        1) The filename of the database to which the note
           is to be added.  If the database is located outside
           of the local data directory, the full path must be
           specified.
        2) The full path and filename to the Import DLL
           used to import the selected file.
        3) The full path and filename to the file being
           imported.
        4) The name of an additional DLL needed to import this
           file format.  This parameter is only necessary if
           importing a word processing document. Otherwise
           This parameter is omitted.  See the accompanying
           README.TXT file for more on this parameter.

    This database to which the note is to be added must contain a form
    called "Simple Form" with a rich text field called "Body" such as
    the database "IMPORTER test database" (importer.nsf) supplied with
    the HCL C API for Notes/Domino toolkit.

    This example assumes that the rich text being imported is
    smaller than MAXONESEGSIZE in size.  If the rich text being
    imported is larger than 60Kb, it must be broken down into
    multiple rich text fields smaller than 60Kb. It is recommended
    that these fields be roughly 40Kb in size or smaller.

************************************************************************/

/* OS includes */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(UNIX)
	#include <unistd.h>
#endif
/* HCL C API for Notes/Domino includes */

#include <global.h>
#include <nsf.h>
#include <osmem.h>
#include <osmisc.h>
#include <oserr.h>
#include <odserr.h>
#include <names.h>
#include <editods.h>
#include <editdflt.h>
#include <nsfnote.h>
#include <ixport.h>
#include <ixedit.h>
#include <ods.h>
#include "../../printLog.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif


#define     CDBUFLEN    40960  /* Maximum recommended length of a
                                  rich-text field to be imported    */
/* Function specifications  */

STATUS LNPUBLIC ImportCD(char *szModulePath, char *szFileName,
                           char *szTempName, char *szDLL);
STATUS LNPUBLIC LoadCD(char *szFileName, DHANDLE *rethImpBuffer,
                         DWORD *retdwLen);
STATUS LNPUBLIC AddRichText(DHANDLE hNote, DHANDLE hBuf, DWORD dwBufLen);

/* function specification for Domino and Notes DLLs    */

STATUS (LNCALLBACKPTR ProcAddress)(VOID *IXContext, WORD Flags,
        HMODULE hModule, char *AltLibraryName, char *PathName);

/************************************************************************

    FUNCTION:   main

*************************************************************************/

int main(int argc, char *argv[])
{
    char        achTempName[MAXPATH], *szModPath, *szFilePath, *szNSFFile, *szDLL;
    char        szCompanyName[] = "Acme Computing, Inc"; /* Text field */

    DHANDLE     hImpBuffer, hNewNote;
    DBHANDLE    hDbNSFFile;
    DWORD       dwImpBufLen;
    STATUS      error;    /* Return status from Domino and Notes and OS calls */

    /* Check arguments */
		
    if (argc != 4 && argc != 5)
    {
        PRINTLOG("\nThe proper syntax is: \n\tIMPORTER.EXE <NSF Filename>");
        PRINTLOG(" <Import/Export DLL Path> <Import File Path> \n");
        PRINTLOG(" <Second DLL Name> (if necessary)\n");
        return NOERROR;
    }
    szNSFFile    = argv[1];
    szModPath    = argv[2];
    szFilePath   = argv[3];


    /*  If a 2nd DLL name was entered, set szDLL to it.  Otherwise, set */
    /*  szDLL to null.                                                  */

    if (argc == 5)
    {
        szDLL = argv[4];
    }
    else
        szDLL = 0;


    /* Start by calling Notes Init. */

    error = NotesInitExtended (argc, argv);
    if (error)
    {
        PRINTLOG("Error: Unable to initialize Notes.\n");
        return (1);
    }

    /* Call the appropriate Import/Export DLL with the appropriate  */
    /* source file to create a char buffer containing the data      */
    /* that will be appended to a CD buffer.                        */

    if (error = ImportCD(szModPath, szFilePath, achTempName, szDLL))
    {
        PRINTLOG("\nUnable to create temporary file. Terminating...\n");
        PRINTERROR(error,"ImportCD");
        goto Done;
    }

    /* Open the temp file created by the Import DLL, read the contents  */
    /* into memory, and return a handle to the object.                  */

    if (error = LoadCD(achTempName, &hImpBuffer, &dwImpBufLen))
    {
        PRINTLOG("\nUnable to process temp file properly. Terminating...\n");
        PRINTERROR(error,"LoadCD");
        goto Done;
    }

    /* Open the Database, assuming it's been created with the   */
    /* template shipped with this demo.                         */

    if (error = NSFDbOpen(szNSFFile, &hDbNSFFile))
    {
        PRINTLOG("\nCannot open database '%s'. Terminating...\n", szNSFFile);
        PRINTERROR(error,"NSFDbOpen");
        goto Done;
    }

    /* Create a Note */

    if (error = NSFNoteCreate(hDbNSFFile, &hNewNote))
    {
        PRINTLOG("\nCannot create new note. Terminating...\n");
        NSFDbClose(hDbNSFFile);    /* ERROR - Close database before exit.*/
        PRINTERROR(error,"NSFNoteCreate");
        goto Done;
    }

    /* Add the Form name */

    if (error = NSFItemSetText(hNewNote,
                                 "Form",
                                 "Simple Form",
                                 (WORD) strlen("Simple Form")))
    {
        PRINTLOG("\nError while setting Form name. Terminating...\n");

        NSFNoteClose(hNewNote);    /* ERROR - Close note before exit.    */
        NSFDbClose(hDbNSFFile);    /* ERROR - Close database before exit.*/
        PRINTERROR(error,"NSFItemSetText");
        goto Done;
    }

    /*  Now add the imported data to the note. */

    if (error = AddRichText(hNewNote, hImpBuffer, dwImpBufLen))
    {
        PRINTLOG("\nError while adding rich text.  Terminating...\n");
        NSFNoteClose(hNewNote);    /* ERROR - Close note before exit.    */
        NSFDbClose(hDbNSFFile);    /* ERROR - Close database before exit.*/
        OSMemFree(hImpBuffer);     /* ERROR - Free up the Import buffer. */
        PRINTERROR(error,"AddRichText");
        goto Done;
    }

    /* write the note */

    if (error = NSFNoteUpdate(hNewNote, 0))
    {
        PRINTLOG("\nError writing new Note. Terminating..\n");
        NSFNoteClose(hNewNote);    /* ERROR - Close note before exit.    */
        NSFDbClose(hDbNSFFile);    /* ERROR - Close database before exit.*/
        OSMemFree(hImpBuffer);     /* Free up the Import buffer.         */
        PRINTERROR(error,"NSFNoteUpdate");
        goto Done;
    }

    /* We got this far without error.  Now free up the Import buffer. */

    OSMemFree(hImpBuffer);

    /* Start cleaning up.  First, Close the Note */

    if (error = NSFNoteClose(hNewNote))
    {
        PRINTLOG("\nUnable to close Note.  Terminating..\n");
        PRINTERROR(error,"NSFNoteClose");
        goto Done;
    }


    /* Now close the database. */

    if (error = NSFDbClose(hDbNSFFile))
    {
        PRINTLOG("\nError closing database '%s'. Terminating...\n, szNSFFile");
        PRINTERROR(error,"NSFDbClose");
        goto Done;
    }

    /* delete the temp file */

    if (unlink(achTempName))
    {
        PRINTLOG("\nError deleting temporary file %s\n", achTempName);
        goto Done;
    }

    PRINTLOG("\nAll Done!\n");

    NotesTerm();

    PRINTLOG("\nProgram completed successfully.\n");

    return(0);

Done:
    NotesTerm();
    return (1);        /* Return Domino and Notes error code. */
}

/************************************************************************

    FUNCTION:   ImportCD

    PURPOSE:    Loads the specified DLL.

    ALGORITHM:
        Sets up an EDITIMPORTDATA structure and invokes the DLL, passing
        this structure as input.  The DLL converts the file to be
        imported to an intermediate format and stores it in a temporary
        file. This function then returns the name of the temp file in
        szTempName.

    INPUTS:
        szModulePath - name of import DLL
        szFileName   - name of file to be imported

    OUTPUTS:
        szTempName   - Name of temp file containing imported data

*************************************************************************/

STATUS LNPUBLIC ImportCD(char *szModulePath, char *szFileName,
                           char *szTempName, char *szDLL)
{
    EDITIMPORTDATA EditImportData;            /* Import DLL data structure                 */
    HMODULE        hmod;                      /* module handle                             */
    STATUS         error;                     /* Return status from Domino and Notes calls */
    char          *FileName;                  /* File name to be imported                  */
    char           TempName[] = "DEFAULT.CD"; /* Temp Filename for import.                 */
    char          *ModuleName;                /* pointer to DLL module name                */
    char          *DLLName;                   /* 2nd DLL for import of                     */
                                              /*  word-processing docs.                    */
    ModuleName = szModulePath;
    FileName   = szFileName;
    DLLName   = szDLL;

    /* Use OSLoadLibrary to load the import DLL and return a pointer to */
    /* the main entry point.                                             */

    if (error = OSLoadLibrary (ModuleName, (DWORD)0, &hmod, &ProcAddress))
    {
        PRINTLOG ("OSLoadLibrary failed.\n");
        goto Done;
    }

    /* Set up the EditImportData data structure with a Default          */
    /* filename for the library to do its report to.  If the import     */
    /* succeeds, this file will contain a set of compound document      */
    /* data strucures formatted in the  same manner as they would be    */
    /* if they were loaded in memory; the records start on even byte    */
    /* boundaries and contain a signature, length, the signature-       */
    /* specific data structure (if any), then the  signature-specific   */
    /* data.                                                            */

      strcpy (EditImportData.OutputFileName, TempName);
      PRINTLOG ("Temp filename is %s.\n", EditImportData.OutputFileName);

    /* Assign the default fontid */

      EditImportData.FontID = DEFAULT_FONT_ID;

    /* Call the ProcAddress() function whose address we located within  */
    /* the DLL loaded above.  This is the SAME argument set that would  */
    /* be used by an import/export DLL written to handle a non-standard */
    /* file format.                                                     */
    /*                                                                  */
    /* The ProcAddress() function can import multiple files.  Since     */
    /* only one file is being imported in this example, the flags will  */
    /* indicate that the file is the first (and also last) file to be   */
    /* imported.                                                        */

    if (error = (*ProcAddress) (&EditImportData,
                                IXFLAG_FIRST | IXFLAG_LAST,         /* Both 1st and last import */
                                0,                                  /* Use default hmodule      */
                                DLLName,                            /* 2nd DLL, if needed.      */
                                FileName))                          /* File to import.          */
    {
        PRINTLOG ("Call to DLL Entry point failed.\n");
        goto Done;
    }


    /* return the temp filename to calling routine */

    strcpy (szTempName, EditImportData.OutputFileName);

Done:
    /* Free the DLL and return */
    OSFreeLibrary(hmod);
    return error;
}

/************************************************************************

    FUNCTION:   LoadCD

    PURPOSE:    read the contents of the intermediate file into memory

    ALGORITHM:
        Opens the intermediate file created by the DLL provided
        on the IMPORTER.EXE command line, reads the contents of
        the file into memory and passes a handle to the object
        back to the calling routine.

    INPUTS:
        szFileName    - Name of temp file holding data to import.

    OUTPUTS:
        rethImpBuffer - handle to data buffer.

    RETURNS:
        retdwLen      - Number of bytes of data in buffer.

*************************************************************************/

STATUS LNPUBLIC LoadCD(char *szFileName, DHANDLE *rethImpBuffer,
                         DWORD *retdwLen)
{
    DHANDLE  hBuffer;       /* handle for memory buffer for CD file read            */
    FILE    *pCDFile;       /* File pointer to CD file                              */
    STATUS   error;         /* Return status from Domino and Notes and OS calls     */
    char    *FileName;      /* File name to be imported                             */
    char    *pBuffer;       /* pointer to memory buffer for CD file read            */
    DWORD    CDFileLength;  /* Size of CD file created during import                */

    error = NOERROR;
    FileName = szFileName;

    /* This file must be read into memory then directly appended to     */
    /* existing compound document (TYPE_COMPOUND) buffer or appended    */
    /* to a note as a new item (TYPE_COMPOUND).                         */

    if ( (pCDFile = fopen(FileName, "r+b")) == NULL)
    {
        PRINTLOG ("Open of '%s' failed.\n", FileName);
        error = ERR_NOEXIST;
        goto Done;
    }

    /* Get the size of the file */
	
	fseek (pCDFile, 0L, SEEK_END);
	CDFileLength = (DWORD) ftell(pCDFile);
	fseek (pCDFile, 0L, SEEK_SET);
	
    /* In this example imports must be < MAXONESGESIZE; error otherwise. */

    if (CDFileLength > MAXONESEGSIZE)
    {
        char NumberString[33];

        //ltoa(CDFileLength, NumberString, 10); /* Convert dword value to ASCII */
		sprintf(NumberString, "%d", CDFileLength);
		
        PRINTLOG ("Error - temp file '%s' is too large: %s bytes.\n",
                 FileName, NumberString);

        PRINTLOG("This program can't import files larger than %u bytes.\n",
                MAXONESEGSIZE);
        fclose (pCDFile);
        error = ERR_ODS_TEXT_TOOBIG;
        goto Done;
    }

    /* Create a file buffer to read CD file into and process it. */

    if (error = OSMemAlloc (0, CDFileLength, &hBuffer))
    {
        PRINTLOG ("Error allocating memory for file read.  Terminating...\n");
        fclose(pCDFile);
        goto Done;
    }

    /* Get the address of the buffer by locking it */

    pBuffer = (char *)OSLockObject (hBuffer);

    /* Read the entire file into a single buffer */

    if ( fread (pBuffer, (size_t)CDFileLength, (size_t)1, pCDFile) < 1 )
    {
        PRINTLOG ("error on file %s read.\n", FileName);
        OSUnlockObject (hBuffer);
        OSMemFree(hBuffer);
        fclose(pCDFile);
        error = ERR_IOERROR;
        goto Done;
    }


    /* The object should not be locked any longer than it has to be.  */

    OSUnlockObject (hBuffer);

    /* return handle to the CD Buffer containing the Import item, and its length */

    *rethImpBuffer = hBuffer;
    *retdwLen = CDFileLength;

    /* close the file. */

    fclose(pCDFile);

Done:
    return error;
}

/************************************************************************

    FUNCTION:   AddRichText

    PURPOSE:    Adds a Rich Text item to the note that is being created.

    ALGORITHM:
        First, generate the Rich Text field:

               - Allocate a buffer,
               - Create a CDPARAGRAPH structure in that buffer,
               - Append a CDPABDEFINITION structure to the buffer,
               - Append a CDPABREFERENCE structure to the buffer,
               - Append the contents of the import buffer.

               Then, add the Rich Text item to the Note by calling
               NSFItemAppend.

   Inputs:     hNote      - handle to the new note being created.
               hImpBuffer - handle to buffer containing imported data
               dwBufLen   - length of import buffer.
   Outputs:    None.

*************************************************************************/

STATUS LNPUBLIC AddRichText (DHANDLE hNote, DHANDLE hImpBuffer,
                               DWORD dwBufLen)

{
    char    *pImpBuffer;
    STATUS   rError;                   /* Error code to be returned    */
    DWORD    dwItemBufLen;             /* length of rich text block    */
    DWORD    dwCDMaxBuf = CDBUFLEN;    /* Maximum length of block      */
    char    *pCDBuffer;                /* Pointer to rich text buffer. */
    char    *pCurrent;                 /* Pointer to the current location in the buffer */

    DHANDLE  hCDBuffer;                /* handle to dynamic mem.       */
    DWORD    dwCDBufLen = 0;
    WORD     wPara = 1;

    CDPARAGRAPH         CDPara;
    CDPABREFERENCE      CDPabRef;
    CDPABDEFINITION     CDPabDef;

    /* Dynamically allocate memory for CD buffer. */

    if (rError = OSMemAlloc (0, CDBUFLEN, &hCDBuffer))
    {
        PRINTLOG("Cannot allocate memory for CD buffer.  Terminating...\n");
        goto Done;
    }

    pCDBuffer = (char *) OSLockObject(hCDBuffer);


    /* Keep a pointer to our current position in the buffer */

	 pCurrent = pCDBuffer;


    /* Place Paragraph structure in Composite Document buffer */

    CDPara.Header.Signature = SIG_CD_PARAGRAPH;
    CDPara.Header.Length = (BYTE) ODSLength(_CDPARAGRAPH);
    dwCDBufLen += ODSLength(_CDPARAGRAPH);
    if (dwCDBufLen % 2)                 /* if len is odd,    */
        dwCDBufLen++;                   /*  make it even     */

    ODSWriteMemory( &pCurrent, _CDPARAGRAPH, &CDPara, 1 );


    /* Place Paragraph Definition structure in CD buffer */

    if ((dwCDBufLen + ODSLength(_CDPABDEFINITION)) > dwCDMaxBuf)
    {
        PRINTLOG("\nCD Buffer not big enough to add CDPABDEFINITION. ");
        PRINTLOG("Terminating...\n");
        OSUnlockObject(hCDBuffer);       /* Unlock  CD buffer.     */
        OSMemFree(hCDBuffer);            /* Free up the CD buffer. */
        goto Done;
    }

    memset(&CDPabDef, 0, sizeof(CDPABDEFINITION));

    CDPabDef.PABID = wPara;
    CDPabDef.Flags = 0;
    CDPabDef.Header.Signature = SIG_CD_PABDEFINITION;
    CDPabDef.Header.Length    = ODSLength(_CDPABDEFINITION);

    CDPabDef.JustifyMode =    JUSTIFY_LEFT;
    CDPabDef.LineSpacing =    DEFAULT_LINE_SPACING;
    CDPabDef.ParagraphSpacingBefore = DEFAULT_ABOVE_PAR_SPACING;
    CDPabDef.ParagraphSpacingAfter = DEFAULT_BELOW_PAR_SPACING;
    CDPabDef.LeftMargin  =    DEFAULT_LEFT_MARGIN;
    CDPabDef.RightMargin =    DEFAULT_RIGHT_MARGIN;
    CDPabDef.FirstLineLeftMargin= DEFAULT_FIRST_LEFT_MARGIN;
    CDPabDef.Tabs = DEFAULT_TABS;
    CDPabDef.Tab[0] = DEFAULT_TAB_INTERVAL;
    CDPabDef.TabTypes = TAB_DEFAULT;
    CDPabDef.Flags2 = 0;

    dwCDBufLen += ODSLength(_CDPABDEFINITION);
    if (dwCDBufLen % 2)                     /* if len is odd,   */
        dwCDBufLen++;                       /* make it even     */

    ODSWriteMemory( &pCurrent, _CDPABDEFINITION, &CDPabDef, 1 );


    /* Place Paragraph Reference structure in CD buffer */

    if ((dwCDBufLen + ODSLength(_CDPABREFERENCE)) > dwCDMaxBuf)
    {
        PRINTLOG("\nCD Buffer not big enough to add CDPABREFERENCE. ");
        PRINTLOG("Terminating...\n");
        OSUnlockObject(hCDBuffer);  /* Unlock  CD buffer.     */
        OSMemFree(hCDBuffer);       /* Free up the CD buffer. */
        goto Done;
    }
    CDPabRef.Header.Signature = SIG_CD_PABREFERENCE;
    CDPabRef.Header.Length = (BYTE) ODSLength(_CDPABREFERENCE);
    CDPabRef.PABID = wPara;
    dwCDBufLen += ODSLength(_CDPABREFERENCE);
    if (dwCDBufLen % 2)                 /* if len is odd,   */
        dwCDBufLen++;                   /*   make it even   */

    ODSWriteMemory( &pCurrent, _CDPABREFERENCE, &CDPabRef, 1 );

    /* Lock down the Import buffer and get a pointer to it. */

    pImpBuffer = (char *) OSLockObject(hImpBuffer) + sizeof(WORD);
    dwItemBufLen = dwBufLen - sizeof(WORD);
    if ((dwCDBufLen + dwItemBufLen) > dwCDMaxBuf)
    {
        PRINTLOG("\nCD Buffer not big enough for import file.  ");
        PRINTLOG("Terminating...\n");
        OSUnlockObject(hImpBuffer); /* Unlock Import buffer.   */
        OSUnlockObject(hCDBuffer);  /* Unlock  CD buffer.      */
        OSMemFree(hCDBuffer);       /* Free up the CD buffer.  */
        goto Done;
    }

    /* Move the imported data into CD buffer  */

    memmove(&pCDBuffer[dwCDBufLen], pImpBuffer, (WORD) dwItemBufLen);
    dwCDBufLen += dwItemBufLen;
    if (dwCDBufLen % 2)             /* if len is odd,   */
        dwCDBufLen++;               /*   make it even   */

    /* Append the "Body" item to the note */

    if (rError = NSFItemAppend(hNote, (WORD)0,
                               "Body", (WORD)strlen("Body"),
                               TYPE_COMPOSITE,
                               pCDBuffer, dwCDBufLen))
    {
        PRINTLOG("\nError adding 'Body' item. Terminating...\n");
        OSUnlockObject(hImpBuffer); /* Unlock Import buffer.     */
        OSUnlockObject(hCDBuffer);  /* Unlock  CD buffer.        */
        OSMemFree(hCDBuffer);       /* Free up the CD buffer.    */
        goto Done;
    }

    /*  Unlock memory that was previously locked, free the CD buffer, */
    /*  and return.                                                   */


    OSUnlockObject(hImpBuffer); /* Unlock Import buffer.  */
    OSUnlockObject(hCDBuffer);  /* Unlock  CD buffer.     */
    OSMemFree(hCDBuffer);       /* Free up the CD buffer. */

Done:
    return (rError);

}
