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

    PROGRAM:    fontbl

    FILE:       fontbl.c

    SYNTAX:     fontbl  <database filename>

    PURPOSE:    Shows how to create rich text with fonts other than 
                FONTID_ROMAN, FONTID_SWISS, or FONTID_TYPEWRITER

    DESCRIPTION:
        This program illustrates how to add a font table to a document 
        so that rich text can use fonts other than FONTID_ROMAN, 
        FONTID_SWISS, and FONTID_TYPEWRITER.  A font table is an item
        named ITEM_NAME_FONTS ($FONTS). It is an item of TYPE_COMPOSITE
        and it contains a table of font structures. Later in the document,
        runs of text (CDTEXT records) can refer to fonts in the table
        by specifying a table entry as the font ID.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* C include files */

#include <stdio.h>
#include <string.h>

/* HCL C API for Notes/Domino include files */


#include <global.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <ods.h>
#include <editods.h>
#include <editdflt.h>
#include <fontid.h>
#include <ostime.h>
#include <nsferr.h>
#include <osmisc.h>
#include <stdnames.h>
#include <colorid.h>
#include <oserr.h>
#include <osmem.h>
#include "../../printLog.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif


void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                         char *db_filename); 

/* #defines for fonts included in the font table */
#define FTFAMILY_UNKNOWN  0xF0   /* Font family for OS/2 programs */
#define DEFAULT_PITCH     0x00
#define FIXED_PITCH       0x01
#define VARIABLE_PITCH    0x02

#define FTFACE_ID   STATIC_FONT_FACES   /* starting id number for the font
                                           face in the font table */

/************************************************************************

    FUNCTION:   Main

    PURPOSE:    HCL C API for Notes/Domino subroutine

*************************************************************************/

int main(int argc, char *argv[])
{

/* Local constants */

#define RT_STRING1 "This is an example of System Monospaced font."
#define RT_STRING1_LEN strlen(RT_STRING1)
#define RT_STRING2 "This is an example of System Proportional font."
#define RT_STRING2_LEN strlen(RT_STRING2)
#define RT_STRING3 "This is an example of TYPEWRITER font."
#define RT_STRING3_LEN strlen(RT_STRING3)

#define LINEOTEXT   255
#define NUM_FONTS     2         /* number of fonts in the font table */


/* Local datatypes */

/* Local type definitions */


   CDFONTTABLE fntTable;
   CDFACE      fntFace[NUM_FONTS];



/* Local data declarations */

   char              path_name[MAXPATH];  /* pathname of database */
   DBHANDLE          db_handle;           /* database handle */
   NOTEHANDLE        note_handle;         /* note handle */
   TIMEDATE          timedate;            /* a time/date field */

   CDPABDEFINITION   def1;                /* paragraph style definition */
   CDPARAGRAPH       para1;               /* paragraph header */
   CDPABREFERENCE    ref1;                /* style reference for this para */
   CDTEXT            text1;               /* text header */


   CDPARAGRAPH       para2;               /* paragraph header */
   CDPABREFERENCE    ref2;                /* style reference for this para */
   CDTEXT            text2;               /* text header */


   CDPARAGRAPH       para3;               /* paragraph header */
   CDPABREFERENCE    ref3;                /* style reference for this para */
   CDTEXT            text3;               /* text header */


   FONTIDFIELDS     *font;                /* font definitions in text header */
   STATUS            error=0;             /* return code from API calls */
   char FAR         *pCDBuffer;
   char FAR         *pCDBufferStart;
   DHANDLE           hMem;

   if (error = NotesInitExtended (argc, argv))
   {
       PRINTLOG("\n Unable to initialize Notes.\n");
       return (1);
   }


/* Get the pathname of the database. */
    ProcessArgs(argc, argv, path_name); 


   if (error = OSMemAlloc(0, MAXONESEGSIZE, &hMem))
   {
       PRINTERROR (error,"OSMemAlloc");  
       NotesTerm();
       return (ERR_MEMORY);
   } 
        
   if ((pCDBuffer = (char*) OSLockObject(hMem)) == NULL)
   {
       PRINTERROR (error,"OSLockObject");  
       NotesTerm();
       return (ERR_MEMORY);
   } 
        
   memset(pCDBuffer, '\0', MAXONESEGSIZE);
   pCDBufferStart = pCDBuffer;

/* Open the database. */

   if (error = NSFDbOpen (path_name, &db_handle))
   {
       PRINTERROR (error,"NSFDbOpen");  
       NotesTerm();
       return (1);
   } 

/* Create a new data note */
   error = NSFNoteCreate (db_handle, &note_handle);
   if(error)
   {
       NSFDbClose(db_handle);
       PRINTERROR (error,"NSFNoteCreate");  
       NotesTerm();
       return (1);
   }
   
/* Write a field named FORM to the note -- this field specifies the
default form to use when the note is displayed. */

   if (error = NSFItemSetText ( note_handle, 
               "FORM",
               "RichTextForm",
               MAXWORD))
   {
      NSFNoteClose (note_handle);
      NSFDbClose (db_handle);
      PRINTERROR (error,"NSFItemSetText");  
      NotesTerm();
      return (1);
   }

/* Fill in the font table with the following fonts:
   System Monospaced, and System Proportional */

   fntTable.Header.Signature = SIG_CD_FONTTABLE;
   fntTable.Header.Length = ODSLength(_CDFONTTABLE) + (ODSLength(_CDFACE) * NUM_FONTS);
   fntTable.Fonts = NUM_FONTS;   /* number CDFACE records follow */

   ODSWriteMemory( (void far * far *)&pCDBuffer, _CDFONTTABLE, &fntTable, 1 );

/* The ID number of the font face in the font table start at FTFACE_ID
   (number 5).  ID numbers 0 - 4 are hard coded */

   fntFace[0].Face = FTFACE_ID;          /* ID number of face */

   /* Font family */
   fntFace[0].Family = FTFAMILY_UNKNOWN | FIXED_PITCH;
   strcpy (fntFace[0].Name, "System Monospaced");   /* Font name */


   fntFace[1].Face = FTFACE_ID + 1;       /* ID number of face */

   /* Font family */
   fntFace[1].Family = FTFAMILY_UNKNOWN | VARIABLE_PITCH; 
   strcpy (fntFace[1].Name, "System Proportional"); /* Font name */

   ODSWriteMemory( (void far * far *)&pCDBuffer, _CDFACE, &fntFace[0], 1 );
   ODSWriteMemory( (void far * far *)&pCDBuffer, _CDFACE, &fntFace[1], 1 );

/* Write a field named $FONTS to the note - this field specifies the 
   font table used with the note */

   if (error = NSFItemAppend ( note_handle, 
                                    0,
                                    ITEM_NAME_FONTS,
                                    (WORD) strlen(ITEM_NAME_FONTS),
                                    TYPE_COMPOSITE,
                                    pCDBufferStart,
                                    pCDBuffer-pCDBufferStart))
   {
      NSFNoteClose (note_handle);
      NSFDbClose (db_handle);
      PRINTERROR (error,"NSFItemAppend");  
      NotesTerm();
      return (1);
   }


/* Start at the beginning of this buffer again */
   pCDBuffer = pCDBufferStart;
   memset(pCDBuffer, '\0', MAXONESEGSIZE);

/* Get the current time/date and write it to a field named TIME_DATE. */

   OSCurrentTIMEDATE(&timedate);

   if (error = NSFItemSetTime (note_handle, "TIME_DATE", &timedate))
   {
      NSFNoteClose (note_handle);
      NSFDbClose (db_handle);
      PRINTERROR (error,"NSFItemSetTime");  
      NotesTerm();
      return (1);
   }

/* The next several sections of code create and write a rich-text field.
First, we create the field locally (in this program's memory). Then, we 
write the completed field to the note. */

/* Fill in the paragraph definition blocks. */

   memset(&def1, 0, sizeof(CDPABDEFINITION));

   def1.Header.Signature = SIG_CD_PABDEFINITION;
   def1.Header.Length = ODSLength(_CDPABDEFINITION);
   def1.PABID = 1;
   def1.JustifyMode = DEFAULT_JUSTIFICATION;
   def1.LineSpacing =     DEFAULT_LINE_SPACING;
   def1.ParagraphSpacingBefore = DEFAULT_ABOVE_PAR_SPACING;
   def1.ParagraphSpacingAfter = DEFAULT_BELOW_PAR_SPACING;
   def1.LeftMargin = DEFAULT_LEFT_MARGIN;
   def1.RightMargin = DEFAULT_RIGHT_MARGIN;
   def1.FirstLineLeftMargin = DEFAULT_FIRST_LEFT_MARGIN;
   def1.Tabs = DEFAULT_TABS;
   def1.Tab[0] = DEFAULT_TAB_INTERVAL;
   def1.Flags = 0;
   def1.TabTypes = TAB_DEFAULT;
   def1.Flags2 = 0;

    ODSWriteMemory( (void far * far *)&pCDBuffer, _CDPABDEFINITION, &def1, 1 );

/* Start a new paragraph. */

   para1.Header.Signature = SIG_CD_PARAGRAPH;
   para1.Header.Length = (BYTE)ODSLength(_CDPARAGRAPH);

   ODSWriteMemory( (void far * far *)&pCDBuffer, _CDPARAGRAPH, &para1, 1 );

/* Specify that this paragraph will use definition block #1 (above). */

   ref1.Header.Signature = SIG_CD_PABREFERENCE;
   ref1.Header.Length = (BYTE)ODSLength(_CDPABREFERENCE);
   ref1.PABID = 1;
   ODSWriteMemory( (void far * far *)&pCDBuffer, _CDPABREFERENCE, &ref1, 1 );

/* Start a run of text within this paragraph. */

   text1.Header.Signature = SIG_CD_TEXT;
   text1.Header.Length = ODSLength(_CDTEXT) + RT_STRING1_LEN;

/* Fill in the font information for this run of text. Note that we set a 
   pointer to the FontID data item within the CDTEXT header. Then we use the
   pointer to set the individual fields within the FontID. */

   font = ( FONTIDFIELDS * ) &text1.FontID;

   font->Face = FTFACE_ID;         /* font is in the FONT TABLE */
   font->Attrib = 0;
   font->Color = FONT_COLOR_BLUE;
   font->PointSize = 10;

   ODSWriteMemory( (void far * far *)&pCDBuffer, _CDTEXT, &text1, 1 );

/* Fill in the actual text of this text run. Do not append a null to the 
text run. */

   memcpy(pCDBuffer, RT_STRING1, RT_STRING1_LEN);
   pCDBuffer += RT_STRING1_LEN;

#if defined W64
    if (((long long)pCDBuffer) % 2)
	    pCDBuffer += 1;
#else
    if (((long)pCDBuffer) % 2)
	    pCDBuffer += 1;
#endif

/* Start a new paragraph. */

   para2.Header.Signature = SIG_CD_PARAGRAPH;
   para2.Header.Length = (BYTE)ODSLength(_CDPARAGRAPH);

   ODSWriteMemory( (void far * far *)&pCDBuffer, _CDPARAGRAPH, &para2, 1 );

/* Specify that this paragraph will use definition block #1 (above). */

   ref2.Header.Signature = SIG_CD_PABREFERENCE;
   ref2.Header.Length = (BYTE)ODSLength(_CDPABREFERENCE);
   ref2.PABID = 1;
   ODSWriteMemory( (void far * far *)&pCDBuffer, _CDPABREFERENCE, &ref2, 1 );
   
/* Start a run of text within this paragraph. */

   text2.Header.Signature = SIG_CD_TEXT;
   text2.Header.Length = (BYTE)ODSLength(_CDTEXT) + RT_STRING2_LEN;

/* Fill in the font information for this run of text. Note that we set a 
pointer to the FontID data item within the CDTEXT header. Then we use the
pointer to set the individual fields within the FontID. */

   font = ( FONTIDFIELDS * ) &text2.FontID;

   font->Face = FTFACE_ID + 1;
   font->Attrib = 0;
   font->Color = FONT_COLOR_MAGENTA;
   font->PointSize = 10;

   ODSWriteMemory( (void far * far *)&pCDBuffer, _CDTEXT, &text2, 1 );

/* Fill in the actual text of this text run. Do not append a null to the 
text run. */

   memcpy(pCDBuffer, RT_STRING2, RT_STRING2_LEN);
   pCDBuffer += RT_STRING2_LEN;

#if defined W64
    if (((long long)pCDBuffer) % 2)
	    pCDBuffer += 1;
#else
    if (((long)pCDBuffer) % 2)
	    pCDBuffer += 1;
#endif

/* Start a new paragraph. */

   para3.Header.Signature = SIG_CD_PARAGRAPH;
   para3.Header.Length = (BYTE)ODSLength(_CDPARAGRAPH);
   ODSWriteMemory( (void far * far *)&pCDBuffer, _CDPARAGRAPH, &para3, 1 );

/* Specify that this paragraph will use definition block #1 (above). */

   ref3.Header.Signature = SIG_CD_PABREFERENCE;
   ref3.Header.Length = (BYTE)ODSLength(_CDPABREFERENCE);
   ref3.PABID = 1;
   ODSWriteMemory( (void far * far *)&pCDBuffer, _CDPABREFERENCE, &ref3, 1 );

/* Start a run of text within this paragraph. */

   text3.Header.Signature = SIG_CD_TEXT;
   text3.Header.Length = ODSLength(_CDTEXT) + RT_STRING3_LEN;

/* Fill in the font information for this run of text. Note that we set a 
pointer to the FontID data item within the CDTEXT header. Then we use the
pointer to set the individual fields within the FontID. */

   font = ( FONTIDFIELDS * ) &text3.FontID;

   font->Face = FONT_FACE_TYPEWRITER;    /* This font is hardcoded and not
                                            in the font table */
   font->Attrib = 0;
   font->Color = FONT_COLOR_BLACK;
   font->PointSize = 10;

   ODSWriteMemory( (void far * far *)&pCDBuffer, _CDTEXT, &text3, 1 );

/* Fill in the actual text of this text run. Do not append a null to the 
text run. */

   memcpy(pCDBuffer, RT_STRING3, RT_STRING3_LEN);
   pCDBuffer += RT_STRING3_LEN;

/* Add the rich-text field to the note. */

   if (error = NSFItemAppend (note_handle,
                                   0,
                                   "RICH_TEXT",
                                   (WORD) strlen("RICH_TEXT"),
                                   TYPE_COMPOSITE,
                                   pCDBufferStart,
                                   pCDBuffer-pCDBufferStart))
   {
      NSFNoteClose (note_handle);
      NSFDbClose (db_handle);
      PRINTERROR (error,"NSFItemAppend");  
      NotesTerm();
      return (1);
   }


/* Add the new note to the database. */

   if (error = NSFNoteUpdate (note_handle, 0))
   {
      NSFNoteClose (note_handle);
      NSFDbClose (db_handle);
      PRINTERROR (error,"NSFNoteUpdate");  
      NotesTerm();
      return (1);
   }


/* Close the note */
   if (error = NSFNoteClose (note_handle))
   {
      NSFDbClose(db_handle);
      PRINTERROR (error,"NSFNoteClose");  
      NotesTerm();
      return (1);
   }

/* Close the database */

   if (error = NSFDbClose (db_handle))
   {
      PRINTERROR (error,"NSFDbClose");  
      NotesTerm();
      return (1);
   }

/* Let the user know it's done */
	
   PRINTLOG("\nDone.\n"); 
   fflush(stdout);

/* End of subroutine. */
   PRINTLOG("\nProgram completed successfully.\n");
   NotesTerm();
   return (0); 

}

/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:     argc, argv - directly from the command line
   
    OUTPUTS:    db_filename get data from the command line or from what 
                the user types at a prompt 

*************************************************************************/

void  LNPUBLIC  ProcessArgs (int argc, char *argv[], char *db_filename)
{ 
   if (argc != 2)  
   {
      printf("Enter name of database: ");      
      fflush(stdout);
      gets(db_filename);
   }    
   else
   {
      strcpy(db_filename, argv[1]);    
   } /* end if */
} /* ProcessArgs */
