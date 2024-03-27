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

    PROGRAM:    dumpfont

    SYNTAX:     dumpfont

    PURPOSE:    Locate and print out the font tables in all data notes.

    DESCRIPTION:

      This program demonstrates use of ODSReadMemory to convert
      a font table from canonical format to host format before
      printing out the contents of the font table to the screen.

      A font table is a special item of TYPE_COMPOSITE named $Font.
      Domino and Notes stores a $Font item in any document that contains
      text in a font other than Roman, Swiss, or Typewriter. The $Font
      item data consists of a CDFONTTABLE structure followed by one
      or more CDFACE structures.

      When an API program reads a CDFONTTABLE or CDFACE structure
      from the NSF subsystem, these structures will be in Dmino and
      Notes Canonical format, i.e. Intel 8x86 byte ordering with no
      pads. API programs that run on other platforms (SPARC,
      etc.) must convert the Canonical-format data structures to Host-
      specific format before attempting to access the members of
      the structure. Dumpfont demonstrates how to use the API
      function ODSReadMemory() to perform this conversion before
      accessing the structure members of the data structure.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* C include files */

#include <stdio.h>
#include <string.h>

/* HCL C API for Notes/Domino include files */


#include <global.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <nsfsearc.h>
#include <osmem.h>
#include <ods.h>
#include <nsferr.h>
#include <idtable.h>            /* IDCreateTable */
#include <stdnames.h>
#include <editods.h>            /* CDFONTTABLE */

#ifndef OSMISC_DEFS
#include <osmisc.h>
#endif

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

/* Function prototypes */

STATUS LNPUBLIC AddIDUnique (void *, SEARCH_MATCH *, ITEM_TABLE *);
STATUS LNPUBLIC ProcessOneNote( void *, DWORD );
STATUS LNPUBLIC PrintFontTable( char*, WORD, DWORD, void* );

/************************************************************************

    FUNCTION:   main C API for Domino and Notes subroutine

*************************************************************************/
int main(int argc, char *argv[])
{
    char        szDBName[] = "richtext.nsf";
    DBHANDLE    hDB;
    STATUS      error = NOERROR;
    DHANDLE     hNoteIDTable;

    /*   Start by calling Notes Init.  */

    error = NotesInitExtended (argc, argv);
    if (error)
    {
        PRINTLOG("Error: Unable to initialize Notes.\n");
        return (1);
    }

    /*  Open the database. */

    if (error = NSFDbOpen( szDBName, &hDB ))
    {
        PRINTLOG ("Error: unable to open database '%s'.\n", szDBName);
        PRINTERROR(error,"NSFDbOpen");
        NotesTerm();
        return(1);
    }

    /*  Create ID table then call to NSFSearch.  NSFSearch will find
        all data notes and call the action routine, AddIDUnique, on each.
        AddIDUnique adds the ID of the data note to the ID table if and
        only if the ID is not already in the table.  The result is a table
        of Note IDs where each ID is guaranteed to appear only once.
     */

    if (error = IDCreateTable( sizeof(NOTEID), &hNoteIDTable ))
    {
        PRINTLOG( "Error: unable to create ID table.\n" );
        NSFDbClose( hDB );
        PRINTERROR(error,"IDCreateTable");
        NotesTerm();
        return(1);
    }

    if (error = NSFSearch(
                    hDB,            /* database handle */
                    NULLHANDLE,     /* selection formula (select all notes) */
                    NULL,           /* title of view in selection formula */
                    0,              /* search flags */
                    NOTE_CLASS_DOCUMENT,/* note class to find */
                    NULL,           /* starting date (unused) */
                    AddIDUnique,    /* call for each note found */
                    &hNoteIDTable,  /* argument to AddIDUnique */
                    NULL ))         /* returned ending date (unused) */

    {
        PRINTLOG( "Error: unable to search database.\n" );
        IDDestroyTable( hNoteIDTable );
        NSFDbClose( hDB );
        PRINTERROR(error,"NSFSearch");
        NotesTerm();
        return(1);
    }

    /*  Loop over note IDs in the table. Call ProcessOneNote on each. */
    if (error = IDEnumerate( hNoteIDTable,
                             ProcessOneNote, /* called for each ID */
                             &hDB ))         /* arg passed to func */
    {
        PRINTLOG( "Error: unable to enumerate documents in ID table.\n" );
        IDDestroyTable( hNoteIDTable );
        NSFDbClose( hDB );
        PRINTERROR(error,"IDEnumerate");
        NotesTerm();
        return(1);
    }

    if (error =IDDestroyTable( hNoteIDTable ))
    {
        NSFDbClose( hDB );
        PRINTERROR(error,"IDDestroyTable");
        NotesTerm();
        return(1);
    }

    /* Close the database */
    if (error = NSFDbClose(hDB))
    {
        PRINTERROR(error,"NSFDbClose");
        NotesTerm();
        return(1);
    }

    /* leave with no error */

    NotesTerm();

    PRINTLOG("\nProgram completed successfully.\n");

    return(0);

}

/************************************************************************

    FUNCTION:   AddIDUnique

*************************************************************************/

STATUS LNPUBLIC AddIDUnique( void * phNoteIDTable,
                                SEARCH_MATCH far *pSearchMatch,
                                ITEM_TABLE *summary_info )
{
    SEARCH_MATCH SearchMatch;
    DHANDLE        hNoteIDTable;
    STATUS       error;
    BOOL         flagOK;

    memcpy( (char*)&SearchMatch, (char*)pSearchMatch, sizeof(SEARCH_MATCH) );

    if (!(SearchMatch.SERetFlags & SE_FMATCH))
    {
        return NOERROR;
    }

    hNoteIDTable = *((DHANDLE *)phNoteIDTable);

    if (error = IDInsert( hNoteIDTable, SearchMatch.ID.NoteID, &flagOK ))
    {
        PRINTLOG( "Error: unable to insert note ID into table.\n" );
        return(error);
    }
    /* If flagOK == TRUE then we inserted note ID into table. Else, the
       note ID is already in the table. Return success in either case.
    */
    return NOERROR;
}

/************************************************************************

    FUNCTION:   ProcessOneNote

*************************************************************************/

STATUS LNPUBLIC ProcessOneNote( void * phDB, DWORD NoteID )
{
    DBHANDLE    hDB;
    STATUS      error;
    NOTEHANDLE  hNote;
    BLOCKID bhThisItem;
    WORD    wType;
    BLOCKID bhValue;
    DWORD   dwLength;

    PRINTLOG ("Processing note %lX.\n", NoteID);

    hDB = *( (DBHANDLE *)phDB );

    if (error = NSFNoteOpen( hDB, NoteID, 0, &hNote))
    {
        PRINTLOG( "Error: unable to open note.\n" );
        return(error);
    }

    /*  Look for the "$Fonts" field within this note. */

    error = NSFItemInfo( hNote, ITEM_NAME_FONTS,
                                (WORD) strlen (ITEM_NAME_FONTS),
                                &bhThisItem,  /* Item BLOCKID */
                                &wType,       /* Type         */
                                &bhValue,     /* Value BLOCKID */
                                &dwLength);   /* Value length */

    /* If no font table, just return.*/

    if (ERR(error) == ERR_ITEM_NOT_FOUND)
    {
        NSFNoteClose( hNote );
        return NOERROR;
    }
    else if (error)
    {
        PRINTLOG( "Error: unable to access item %d.\n", ITEM_NAME_FONTS );
        NSFNoteClose( hNote );
        return(error);
    }

    EnumCompositeBuffer( bhValue, dwLength, PrintFontTable, NULL);

    NSFNoteClose( hNote );

    return NOERROR;
}

/************************************************************************

    FUNCTION:   PrintFontTable

*************************************************************************/

STATUS LNPUBLIC PrintFontTable( char  *RecordPtr,
                                  WORD   RecordType,
                                  DWORD  RecordLength,
                                  void  *Unused )
{
    void           *pItemValue;
    CDFONTTABLE     cdFontTab;
    CDFACE          cdFace;
    WORD            wIndex;

    if (RecordType != SIG_CD_FONTTABLE)
    {
        /* EnumCompositeBuffer found a CD record in the "$Fonts" field
           that is not of type SIG_CD_FONTTABLE.  Unusual, but not fatal.
        */
        return NOERROR;
    }

    /* RecordPtr points to the item value in canonical format after the
       datatype word. The item value starts with a CDFONTTABLE structure.
       Call ODSReadMemory() to convert this CDFONTTABLE to host format and
       store it in cdFontTab. ODSReadMemory() increments pItemValue to
       point to the next byte in the input buffer after the CDFONTTABLE.
    */

    pItemValue = RecordPtr;
    ODSReadMemory( &pItemValue, _CDFONTTABLE, &cdFontTab, 1 );

    for (wIndex = 0; wIndex < cdFontTab.Fonts; wIndex++)
    {
        ODSReadMemory( &pItemValue, _CDFACE, &cdFace, 1 );
        PRINTLOG( "    Font %d:\n", wIndex );
        PRINTLOG( "       Face    = %d\n", cdFace.Face );
        PRINTLOG( "       Family  = %d\n", cdFace.Family );
        PRINTLOG( "       Name    = %s\n", cdFace.Name );
    }

    return (NOERROR);
}
#ifdef __cplusplus
}
#endif
