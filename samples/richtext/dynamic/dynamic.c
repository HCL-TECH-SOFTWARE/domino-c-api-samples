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

    PROGRAM:    dynamic

    FILE:       dynamic.c

    SYNTAX:     dynamic

    PURPOSE:    Demonstrate low level creation of rich text via dynamically
                allocating memory and initializing it with CD records.

    DESCRIPTION:
        This program creates a rich text field by dynamically allocating
        a memory buffer, initializing Compound Document data structures,
        also known as CD records, and copying these CD records into the
        buffer. It calls NSFItemAppend to append this buffer to a note.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* C include files */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <ods.h>
#include <colorid.h>
#include <editods.h>
#include <editdflt.h>
#include <fontid.h>
#include <ostime.h>

#include <osmisc.h>
#include "../../printLog.h"


#define PARA_STYLE_ID 1

/************************************************************************

    FUNCTION:   main

*************************************************************************/

int main(int argc, char *argv[])
{
    char            path_name[]="richtext.nsf";  /* pathname of database */
    DBHANDLE        db_handle;  /* database handle */
    NOTEHANDLE      note_handle;/* note handle */
    TIMEDATE        timedate;   /* a Domino and Notes time/date field */
    WORD            wBuffLen;   /* required buffer length */
    BYTE           *rt_field;   /* allocated rich-text field */
    BYTE           *buff_ptr;   /* position in allocated memory */
    CDPABDEFINITION pabdef;     /* rich-text paragraph style */
    CDPARAGRAPH     para;       /* rich-text paragraph header */
    CDPABREFERENCE  ref;        /* rich-text style reference */
    CDTEXT          cdtext;     /* rich-text text header */
    char            szString1[] = "Hello world... ";
    char            szString2[] = "So long world ";
    WORD            wString1Len = strlen( szString1 );
    							/* Length of actual string */
    WORD            wString1BufLen = wString1Len + (wString1Len%2);
    							/* Space to allocate (must be an	*/
    							/* even number of bytes)			*/
    WORD            wString2Len = strlen( szString2 );
    							/* Length of actual string */
    WORD            wString2BufLen = wString2Len + (wString2Len%2);
    							/* Space to allocate (must be an	*/
    							/* even number of bytes)			*/
    FONTIDFIELDS   *pFontID;    /* font definitions in text header */
    DWORD           rt_size;    /* size of rich-text field */
    STATUS          error = NOERROR;  /* return code from API calls */

    
    if (error = NotesInitExtended (argc, argv))
    {
        PRINTLOG("\n Unable to initialize Notes.\n");
        return (1);
    }


    /* Open the database. */
    
    if (error = NSFDbOpen (path_name, &db_handle))
    {
        PRINTLOG( "Error: unable to open database `%s`\n", path_name );
        PRINTERROR (error,"NSFDbOpen");  
        NotesTerm();
        return (1);
    }
    
    
    /* Create a new data note. */
    
    if (error = NSFNoteCreate (db_handle, &note_handle))
    {
        PRINTLOG("Error: unable to create new document.\n");
        NSFDbClose (db_handle);
        PRINTERROR (error,"NSFNoteCreate");  
        NotesTerm();
        return (1);
    }
    
    /* Write a field named FORM to the note -- this field specifies the
       default form to use when the note is displayed. 
     */
    
    if (error = NSFItemSetText ( note_handle, 
                                      "FORM",
                                      "RichTextForm", 
                                      MAXWORD))
    {
        PRINTLOG("Error: unable to set text item into new document.\n");
        NSFNoteClose (note_handle);
        NSFDbClose (db_handle);
        PRINTERROR (error,"NSFItemSetText");  
        NotesTerm();
        return (1);
    }
    
    /* Get the current time/date and write it to a field named TIME_DATE. */
    
    OSCurrentTIMEDATE(&timedate);
    
    if (error = NSFItemSetTime (note_handle, "TIME_DATE", &timedate))
    {
        PRINTLOG("Error: unable to set time item into new document.\n");
        NSFNoteClose (note_handle);
        NSFDbClose (db_handle);
        PRINTERROR (error,"NSFItemSetTime");  
        NotesTerm();
        return (1);
    }
    
    /* The next several sections of code create and write a rich-text field.
       A rich-text field consists of a series of CD records. This field 
       will contain a CDPABDEFINITION, a CDPARAGRAPH, a CDPABRFERENCE, 
       and 2 CDTEXT records. A CDTEXT record consists of a CDTEXT structure
       followed by run of text.
    
       Allocate a buffer to hold this series of CD records. To calculate the 
       size of the buffer required, add up the ODS lengths of each of the CD 
       records it will contain. The length of a CDTEXT record is the ODS
       length of the CDTEXT structure, plus the number of characters in the
       text run. Force the number of characters in each text run to be even.
       If the string length is odd, add one to the string length so that the
       record length is even. You must ensure that all CD records begin on 
       even byte boundaries.
     */
    
    wBuffLen =  ODSLength( _CDPABDEFINITION ) + 
                ODSLength( _CDPARAGRAPH )     + 
                ODSLength( _CDPABREFERENCE )  +
                ODSLength( _CDTEXT )          +
                wString1BufLen                +
                ODSLength( _CDTEXT )          +
                wString2BufLen                ;
    

    rt_field = (BYTE *) malloc ( wBuffLen );

    if( rt_field == (BYTE *)NULL )
    {
        PRINTLOG("Error: unable to allocate %d bytes memory.\n", wBuffLen);
        NSFNoteClose (note_handle);
        NSFDbClose (db_handle);
        NotesTerm();
        return (0); 
    }    
              
    /* Keep a pointer to our current position in the buffer. */
              
    buff_ptr = rt_field;
              
    /* Initialize a CDPABDEFINITION structure.We use all defaults, 
       except for centered justification. 
    */

    memset(&pabdef, 0, sizeof(CDPABDEFINITION));

    pabdef.Header.Signature = SIG_CD_PABDEFINITION;
    pabdef.Header.Length = ODSLength( _CDPABDEFINITION );
    pabdef.PABID = PARA_STYLE_ID;
    pabdef.JustifyMode = JUSTIFY_CENTER;
    pabdef.LineSpacing = DEFAULT_LINE_SPACING;
    pabdef.ParagraphSpacingBefore = DEFAULT_ABOVE_PAR_SPACING;
    pabdef.ParagraphSpacingAfter = DEFAULT_BELOW_PAR_SPACING;
    pabdef.LeftMargin = DEFAULT_LEFT_MARGIN;
    pabdef.RightMargin = DEFAULT_RIGHT_MARGIN;
    pabdef.FirstLineLeftMargin = DEFAULT_FIRST_LEFT_MARGIN;
    pabdef.Tabs = DEFAULT_TABS;
    pabdef.Tab[0] = DEFAULT_TAB_INTERVAL;
    pabdef.Flags = 0;
    pabdef.TabTypes = TAB_DEFAULT;
    pabdef.Flags2 = 0;

    /* Call ODSWriteMemory to convert the CDPABDEFINITION structure to 
       Domino and Notes canonical format and write the converted structure into
       the buffer at location buff_ptr. This advances buff_ptr to the
       next byte in the buffer after the canonical format strucure.
     */
    
    ODSWriteMemory( &buff_ptr, _CDPABDEFINITION, &pabdef, 1 );

    /* Put a paragraph header in the field. */
    
    para.Header.Signature = SIG_CD_PARAGRAPH;
    para.Header.Length = (BYTE) ODSLength( _CDPARAGRAPH );

    ODSWriteMemory( &buff_ptr, _CDPARAGRAPH, &para, 1 );
    
    /* Put a paragraph reference block in the field. Specify 
       PARA_STYLE_ID so that this paragraph uses the style 
       defined above.
     */

    ref.Header.Signature = SIG_CD_PABREFERENCE;
    ref.Header.Length = (BYTE) ODSLength( _CDPABREFERENCE );
    ref.PABID = PARA_STYLE_ID;
    
    ODSWriteMemory( &buff_ptr, _CDPABREFERENCE, &ref, 1 );

    /* Add the CDTEXT record to the field. A CDTEXT record consists
       of a CDTEXT structure followed by a run of text. Initialize the
       CDTEXT structure by filling in the signature and the length. 
       The CDTEXT structure also contains the font information that
       controls how Domino and Notes displays this first run of text. 
     */

    cdtext.Header.Signature = SIG_CD_TEXT;
    cdtext.Header.Length = ODSLength( _CDTEXT ) + wString1Len ;

    pFontID = (FONTIDFIELDS *) &(cdtext.FontID);
    
    pFontID->Face = FONT_FACE_SWISS;
    pFontID->Attrib = ISBOLD;
    pFontID->Color = NOTES_COLOR_BLUE;
    pFontID->PointSize = 24;

    ODSWriteMemory( &buff_ptr, _CDTEXT, &cdtext, 1 );

    /* Write the actual characters of this first text tun to the buffer. 
       Since the run of text may contian embedded null characters, use 
       memcpy, not strcpy. No need to terminate the run of text with a 
       null because the Header.Length member of the CDTEXT structure 
       specifies the length explicitly.  Only copy the null if the
       number of characters is odd.
     */

    memcpy( (char *)buff_ptr, szString1, wString1BufLen );
    
    /* Advance the pointer to the next even-byte boundary
     */
    buff_ptr += wString1BufLen;
    

    /* Add a second run of text to the field. Use a different
       font this time.
     */

    cdtext.Header.Signature = SIG_CD_TEXT;
    cdtext.Header.Length = ODSLength( _CDTEXT ) + wString2Len ;

    pFontID = (FONTIDFIELDS *) &(cdtext.FontID);
    
    pFontID->Face = FONT_FACE_ROMAN;
    pFontID->Attrib = ISITALIC;
    pFontID->Color = NOTES_COLOR_RED;
    pFontID->PointSize = 18;

    ODSWriteMemory( &buff_ptr, _CDTEXT, &cdtext, 1 );
    memcpy( (char *)buff_ptr, szString2, wString2BufLen );
    
    /* Advance the pointer to the next even-byte boundary
     */
    buff_ptr += wString2BufLen;

    /* We are done filling the buffer with CD records. Now append the 
       buffer to the note as a rich text field. First find the size of 
       the buffer. Then add the rich-text field to the note by calling 
       NSFItemAppend. NSFItemAppend copies the data out of the buffer 
       specified by rt_field. Therfore, after calling NSFItemAppend, we 
       can free the buffer.
     */

    rt_size = (DWORD)(buff_ptr - rt_field);

    error = NSFItemAppend( note_handle,
                                0,
                                "RICH_TEXT", (WORD) strlen("RICH_TEXT"),
                                TYPE_COMPOSITE,
                                rt_field, rt_size );

    free( rt_field );
    
    
    if (error)
    {
        PRINTLOG( "Error: unable to append rich text item to the note.\n" );
        NSFNoteClose( note_handle );
        NSFDbClose( db_handle );
        PRINTERROR (error,"NSFItemAppend");  
        NotesTerm();
        return (1);
    }
 
    /* Add the entire new note (with all fields) to the database. */

    if (error = NSFNoteUpdate( note_handle, 0 ))
    {
        PRINTLOG( "Error: unable to update note to database.\n" );
        NSFNoteClose( note_handle );
        NSFDbClose( db_handle );
        PRINTERROR (error,"NSFNoteUpdate");  
        NotesTerm();
        return (1);
    }

    /* Close the new note. */

    if (error = NSFNoteClose( note_handle ))
    {
        NSFDbClose( db_handle );
        PRINTERROR (error,"NSFNoteClose");  
        NotesTerm();
        return (1);
    }

    /* Close the database */

    if (error = NSFDbClose( db_handle ))
    {
        PRINTERROR (error,"NSFDbClose");  
        NotesTerm();
        return (1);
    }
    PRINTLOG ("Document containing rich text, successfully created\n");
    PRINTLOG("\nProgram completed successfully.\n");
    NotesTerm();
    return (0); 

}
