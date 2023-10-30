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

    PROGRAM:    cdfread

    FILE:       cdfread.c

    SYNTAX:     cdfread

    PURPOSE:    Demonstrate function CompoundTextAssimilateFile
                and CompoundTextAssimilateBuffer

    DESCRIPTION:
        This creates a new data note in the database, richtext.nsf 
        and fills in the rich text field of the note with the 
        compound text from a CD file, cd_recs.inp using the
        CompoundTextAssimilateFile API. Also, it reads the CD file
        contents in a buffer and then adds the contents of the buffer
        to the compound text context using the CompoundTextAssimilateBuffer
        API and then update the notes document with this data.
        
        A CD file is a file containing Domino and Notes rich text data. 
        This data must be in Domino and Notes Canonical format. Use the
        companion program "cdfwrite" to create this CD file.
        
        The database used in this program has a form named 
        "RichTextForm".  This form contains a time/date field named,
        TIME_DATE, and a rich text field named, RICH_TEXT.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* OS and C include files */

#include <stdio.h>
#include <string.h>

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <editods.h>
#include <editdflt.h>
#include <stdnames.h>
#include <ostime.h>
#include <easycd.h>
#include <fontid.h>
#include <osmem.h>
#include <nif.h>
#include "../../printLog.h"

#include <osmisc.h>

/* local include files */

#include "cdfile.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

#define BUFF_SIZE 1024

/************************************************************************

    FUNCTION:   main

*************************************************************************/

int main(int argc, char *argv[])
{
    STATUS      nErr = NOERROR; /* return code from API calls */
    char        szPathName[] = "richtext.nsf";     /* pathname of database */
    char        szItemName_RICH_TEXT[] = "RICH_TEXT";
    DBHANDLE    hDB;            /* database handle */
    NOTEHANDLE  hNote;          /* note handle */
    TIMEDATE    timedate;       /* a time/date field */
    DHANDLE       hCompound;      /* handle to CompoundText context */
    FILE        *pCDFile;       /* file stream */
    char        *pszCDFileName = CD_FILENAME;   /* pointer to CD file */
    char        szBuffer[BUFF_SIZE] = {0};        /* buffer */
    char        *pszBuffer = szBuffer;              /* pointer to buffer */
    SIZE_T      sRetVal;                        /* return value from fread() */

    if (nErr = NotesInitExtended (argc, argv))
    {
        PRINTLOG("\n Unable to initialize Notes.\n");
        return (1);
    }


    /* Open the database. */

    if ( (nErr = NSFDbOpen (szPathName, &hDB)) != NOERROR )
    {
        PRINTLOG( "Error: unable to open database '%s'.\n", szPathName );
        PRINTERROR (nErr,"NSFDbOpen");  
        NotesTerm();
        return (1);
    }
   
    /* Create a new data note. */

    if ( (nErr = NSFNoteCreate (hDB, &hNote)) != NOERROR )
    {
        PRINTLOG( "Error: unable to create new document in database.\n" );
        NSFDbClose (hDB);
        PRINTERROR (nErr,"NSFNoteCreate");  
        NotesTerm();
        return (1);
    }

    /* Write a field named FORM to the note -- this field specifies the
       default form to use when the note is displayed. */

    nErr = NSFItemSetText( hNote, FIELD_FORM, "RichTextForm", MAXWORD );

    if (nErr != NOERROR)
    {
        PRINTLOG( "Error: unable to set text in item '%s'.\n", FIELD_FORM );
        NSFNoteClose (hNote);
        NSFDbClose (hDB);
        PRINTERROR (nErr,"NSFItemSetText");  
        NotesTerm();
        return (1);
    }

    /* Get the current time/date and write it to a field named TIME_DATE. */

    OSCurrentTIMEDATE(&timedate);

    if ( (nErr = NSFItemSetTime (hNote, "TIME_DATE", &timedate)) != NOERROR )
    {
        PRINTLOG( "Error: unable to set time in item 'TIME_DATE'.\n" );
        NSFNoteClose (hNote);
        NSFDbClose (hDB);
        PRINTERROR (nErr,"NSFItemSetTime");  
        NotesTerm();
        return (1);
    }

    /* Create the CompoundText context. We get an Item Context (as opposed
       to a stand-alone context) by specifying the handle to an open note 
       and the name of a rich text field.
    */

    nErr = CompoundTextCreate (
                hNote,        /* handle of note */
                szItemName_RICH_TEXT,  /* item name */
                &hCompound);  /* returned handle to CompoundText context */

    if (nErr != NOERROR)
    {
        PRINTLOG( "Error: unable to create CompoundText context for 'RICH_TEXT'.\n" );
        NSFNoteClose (hNote);
        NSFDbClose (hDB);
        PRINTERROR (nErr,"CompoundTextCreate");  
        NotesTerm();
        return (1);
    }

    /* Open the CD file. */

    pCDFile = fopen( pszCDFileName, "r" );

    if (pCDFile == NULL)
    {
        PRINTLOG( "Error: unable to open file '%s'.\n", pszCDFileName );
        CompoundTextDiscard (hCompound);
        NSFNoteClose (hNote);
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
    }

    /* Read the contents of the file in a buffer. */

    sRetVal = fread( szBuffer, sizeof(char), sizeof(szBuffer)-1, pCDFile );

    if (ferror(pCDFile))
    {
        PRINTLOG("Error: unable to read file '%s'.\n", pszCDFileName );
        fclose (pCDFile);
        CompoundTextDiscard (hCompound);
        NSFNoteClose (hNote);
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
    }
    else if(!sRetVal)
    {
        PRINTLOG("Error: There is no data in the file '%s'.\n", pszCDFileName );
        fclose (pCDFile);
        CompoundTextDiscard (hCompound);
        NSFNoteClose (hNote);
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
    }

    /* Close the file. */

    if (fclose( pCDFile ))
    {
        PRINTLOG("Error: unable to close file '%s'.\n", pszCDFileName);
        CompoundTextDiscard (hCompound);
        NSFNoteClose (hNote);
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
    }

    /* Add the data from the buffer to compound text context. */

    nErr = CompoundTextAssimilateBuffer(hCompound,(void *) pszBuffer,(DWORD) sizeof(szBuffer));

    if (nErr != NOERROR)
    {
        PRINTLOG( "Error: unable to add compound text from Buffer to CompoundText context .\n" );
        CompoundTextDiscard (hCompound);
        NSFNoteClose (hNote);
        NSFDbClose (hDB);
        PRINTERROR (nErr,"CompoundTextAssimilateBuffer");
        NotesTerm();
        return (1);
    }

    /* Add the data from the CD file to the CompoundText context.*/

    nErr = CompoundTextAssimilateFile (
                hCompound,          /* handle to CompoundText context */
                CD_FILENAME,       /* file of CD Records */
                0L);                /* reserved */

    if (nErr != NOERROR)
    {
        PRINTLOG("Error: unable to assimilate CD record from file '%s'.\n",
                    CD_FILENAME);
        CompoundTextDiscard (hCompound);
        NSFNoteClose (hNote);
        NSFDbClose (hDB);
        PRINTERROR (nErr,"CompoundTextAssimilateFile");  
        NotesTerm();
        return (1);
    }

    /* Add the CompoundText context to the note. Since this is an Item 
       Context, (associated with the newly created note), only specify 
       hCompound parameter. 
    */

    nErr = CompoundTextClose (
                hCompound,            /* handle to CompoundText context */
                NULL,                    
                NULL,
                NULL,
                0);

    if (nErr != NOERROR)
    {
        PRINTLOG("Error: unable to close compound text context.\n");
        CompoundTextDiscard (hCompound);
        NSFNoteClose (hNote);
        NSFDbClose (hDB);
        PRINTERROR (nErr,"CompoundTextClose");  
        NotesTerm();
        return (1);
    }

    /* Add the new note to the database. */

    if ( (nErr = NSFNoteUpdate (hNote, 0)) != NOERROR )
    {
        PRINTLOG("Error: unable to update new note to the database.\n");
        NSFNoteClose (hNote);
        NSFDbClose (hDB);
        PRINTERROR (nErr,"NSFNoteUpdate");  
        NotesTerm();
        return (1);
    }

    /* Close the new note. */

    if ( (nErr = NSFNoteClose (hNote)) != NOERROR )
    {
        NSFDbClose (hDB);
        PRINTERROR (nErr,"NSFNoteClose");  
        NotesTerm();
        return (1);
    }

    PRINTLOG("New note successfully created.\n");

    /* Close the database. */

    if ( (nErr = NSFDbClose (hDB)) != NOERROR )
	{
        PRINTERROR (nErr, NSFDbClose);
        NotesTerm();
        return (1);
	}

    /* End of subroutine. */

    PRINTLOG("\nProgram completed successfully\n");
    NotesTerm();
    return (0); 

}
