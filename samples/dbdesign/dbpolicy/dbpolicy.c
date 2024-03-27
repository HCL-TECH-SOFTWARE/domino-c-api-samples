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

    PROGRAM:    dbpolicy

    FILE:       dbpolicy.c

    SYNTAX:     dbpolicy  <database name>

    PURPOSE:    Shows how to create Help-About and Help-Using documents

    DESCRIPTION:
        This program creates a new database policy document (Help-About
        Database document) and a new database help document (Help-Using
        Database document) and fills in some rich text for each.

****************************************************************************/

/* OS and C include files */

#include <stdio.h>
#include <string.h>

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <osmem.h>
#include <easycd.h>
#include <fontid.h>
#include <editdflt.h>
#include <colorid.h>
#include <nsferr.h>
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

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

#define POLICY_DOC 1
#define HELP_DOC 2

#define POLICY_STRING1 "DATABASE POLICY STATEMENT"
#define HELP_STRING1   "DATABASE HELP STATEMENT"
#define  STRING_LENGTH  256

#define POLICY_STRING2 "This is a sample Policy Document \
(Help-About Database document) for the API sample program, DBPOLICY."
#define HELP_STRING2   "This is a sample Help Document \
(Help-Using Database document) for the API sample program, DBPOLICY."

/* Function prototypes */

STATUS LNPUBLIC print_doc (DBHANDLE, NOTEID);
STATUS add_rich_text (short, NOTEHANDLE);
void  LNPUBLIC  ProcessArgs (int argc, char *argv[], char *db_name);

/************************************************************************

    FUNCTION:   Main

    PURPOSE:    HCL C API for Notes/Domino subroutine

*************************************************************************/

int main(int argc, char *argv[])
{

/* Local data declarations */

   char          path_name[STRING_LENGTH];                          /* pathname of database */
   DBHANDLE      db_handle;                                         /* database handle */
   NOTEHANDLE    note_handle;                                       /* note handle */
   STATUS        error;                                             /* return code from API calls */
   int           note_class_value;                                  /* set to NOTE_CLASS_INFO - policy doc */
   NOTEID        note_ID;                                           /* to read the policy and help docs */

        
   error = NotesInitExtended (argc, argv);
   if (error)
   {
       PRINTLOG("Error: Unable to initialize Notes.\n");
       return (1);
   }


/* Get the command line parameters that the user entered. */
   ProcessArgs(argc, argv, path_name);


/* Initialize the paragraph definition blocks used in the rich text fields
   of the database policy document and of the database help document */

/* Open the database. */

   if (error = NSFDbOpen (path_name, &db_handle))
   {
       PRINTERROR(error,"NSFDbOpen");
       NotesTerm();
       return (1);
   }

/* Exit program if either a database policy document exists or a
   database help document exists */

   error = NSFDbGetSpecialNoteID (db_handle,
                                  SPECIAL_ID_NOTE | NOTE_CLASS_INFO,
                                  &note_ID);
   if (error != ERR_SPECIAL_ID)
   {
       if (error == NOERROR)
       {
          PRINTLOG ("\n\nA database Policy Document (Help-About document) exists ");
          PRINTLOG ("in this database.\n");
          PRINTLOG ("Program will not attempt to create any documents.\n\n");
       }
       PRINTERROR(error,"NSFDbGetSpecialNoteID");
       NSFDbClose(db_handle);
       NotesTerm();
       return (1);
   }

   error = NSFDbGetSpecialNoteID (db_handle,
                                  SPECIAL_ID_NOTE | NOTE_CLASS_HELP,
                                  &note_ID);
   if (error != ERR_SPECIAL_ID)
   {
       if (error == NOERROR)
       {
          PRINTLOG ("\n\nA database Help Document (Help-Using document) exists ");
          PRINTLOG ("in this database.\n");
          PRINTLOG ("Program will not attempt to create any documents.\n\n");
       }
       PRINTERROR(error,"NSFDbGetSpecialNoteID");
       NSFDbClose(db_handle);
       NotesTerm();
       return (1);
   }

/* Create a new note for the database policy document. */

   if (error = NSFNoteCreate (db_handle, &note_handle))
   {
       PRINTERROR(error,"NSFNoteCreate");
       NSFDbClose(db_handle);
       NotesTerm();
       return (1);
   }

/* Specify in the note header that this note is a database policy document */

   note_class_value = NOTE_CLASS_INFO | NOTE_CLASS_DEFAULT;

   NSFNoteSetInfo (note_handle, _NOTE_CLASS, &note_class_value);

   error = add_rich_text (POLICY_DOC, note_handle);

   if (error != NOERROR)
   {
       PRINTERROR(error,"add_rich_text");
       NSFNoteClose (note_handle);
       NSFDbClose(db_handle);
       NotesTerm();
       return (1);
   }

/* Add the new note to the database. */

   if (error = NSFNoteUpdate (note_handle, 0))
   {
       PRINTERROR(error,"NSFNoteUpdate");
       NSFNoteClose (note_handle);
       NSFDbClose(db_handle);
       NotesTerm();
       return (1);
   }

/* Close the new note. */

   if (error = NSFNoteClose (note_handle))
   {
       PRINTERROR(error,"NSFNoteClose");
       NSFDbClose(db_handle);
       NotesTerm();
       return (1);
   }

/* Create a new note for the database help document */

   if (error = NSFNoteCreate (db_handle, &note_handle))
   {
       PRINTERROR(error,"NSFNoteCreate");
       NSFDbClose(db_handle);
       NotesTerm();
       return (1);
   }

/* Specify in the note header that this note is a database help document */

   note_class_value = NOTE_CLASS_HELP | NOTE_CLASS_DEFAULT;

   NSFNoteSetInfo (note_handle, _NOTE_CLASS, &note_class_value);

   error = add_rich_text (HELP_DOC, note_handle);

   if (error != NOERROR)
   {
       PRINTERROR(error,"add_rich_text");
       NSFNoteClose (note_handle);
       NSFDbClose(db_handle);
       NotesTerm();
       return (1);
   }

/* Add the new note to the database. */

   if (error = NSFNoteUpdate (note_handle, 0))
   {
       PRINTERROR(error,"NSFNoteUpdate");
       NSFNoteClose (note_handle);
       NSFDbClose(db_handle);
       NotesTerm();
       return (1);
   }

/* Close the new note. */

   if (error = NSFNoteClose (note_handle))
   {
       PRINTERROR(error,"NSFNoteClose");
       NSFDbClose(db_handle);
       NotesTerm();
       return (1);
   }

/* Now open and print out the database policy document and the database
   help document */

/* Use NSFDbGetSpecialNoteID to get the note ID of the policy doc */

   if (error = NSFDbGetSpecialNoteID (db_handle,
                                      SPECIAL_ID_NOTE | NOTE_CLASS_INFO,
                                      &note_ID))
   {
       PRINTERROR(error,"NSFDbGetSpecialNoteID");
       NSFDbClose(db_handle);
       NotesTerm();
       return (1);
   }

   if (error = print_doc (db_handle, note_ID))
   {
       PRINTERROR(error,"print_doc");
       NSFDbClose(db_handle);
       NotesTerm();
       return (1);
   }

/* Use NSFDbGetSpecialNoteID to get the note ID of the help doc */

   if (error = NSFDbGetSpecialNoteID (db_handle,
                                      SPECIAL_ID_NOTE | NOTE_CLASS_HELP,
                                      &note_ID))
   {
       PRINTERROR(error,"NSFDbGetSpecialNoteID");
       NSFDbClose(db_handle);
       NotesTerm();
       return (1);
   }

   if (error = print_doc (db_handle, note_ID))
   {
       PRINTERROR(error,"print_doc");
       NSFDbClose(db_handle);
       NotesTerm();
       return (1);
   }

/* Close the database */

   if (error = NSFDbClose (db_handle))
   {
       PRINTERROR(error,"NSFDbClose");
       NotesTerm();
       return (1);
   }

/* End of subroutine. */
   PRINTLOG("\nProgram completed successfully.\n");
   NotesTerm();
   return (0);
}


/************************************************************************

    FUNCTION:   add_rich_text

    PURPOSE:    adds rich text to the $BODY field of a database policy
                document or database help document.

*************************************************************************/

STATUS add_rich_text (short doc_type, NOTEHANDLE note_handle)
{
   DHANDLE       hCompound;         /* handle to CompoundText context */
   STATUS        error;
   char          *header;           /* Doc header text */
   char          *text;             /* Doc text */
   COMPOUNDSTYLE Style;
   DWORD         dwStyleID;
   FONTID        fid;
   FONTIDFIELDS  *fontfld;

   switch (doc_type)
   {
       case POLICY_DOC:
          header = POLICY_STRING1;
          text = POLICY_STRING2;
          break;

       case HELP_DOC:
          header = HELP_STRING1;
          text = HELP_STRING2;
          break;

       default:
          header = "DOC HEADER";
          text = "Text for the doc.";
          break;
   }

   if (error = CompoundTextCreate (note_handle, "$BODY", &hCompound))
       return (error);

   /* Add the header text for the document */

   CompoundTextInitStyle (&Style);    /* initializes Style to the defaults */

   Style.JustifyMode = JUSTIFY_CENTER;

   error = CompoundTextDefineStyle (
                                    hCompound,          /* handle to CompoundText context */
                                    "",                 /* style name - none */
                                    &Style,
                                    &dwStyleID);        /* style id */

   if (error != NOERROR)
   {
       CompoundTextDiscard (hCompound);
       return (error);
   }

   fontfld = (FONTIDFIELDS *)&fid;

   fontfld->PointSize = 14;
   fontfld->Color = NOTES_COLOR_BLUE;
   fontfld->Attrib = ISBOLD;
   fontfld->Face = FONT_FACE_SWISS;

   error = CompoundTextAddParagraphExt (
                                        hCompound,               /* handle to CompoundText context */
                                        dwStyleID,               /* style id */
                                        fid,                     /* font id */
                                        header,                  /* paragraph of text to add */
                                        (DWORD)strlen(header),   /* length of text to add */
                                        NULLHANDLE);             /* handle to CLS translation table */

   if (error != NOERROR)
   {
       CompoundTextDiscard (hCompound);
       return (error);
   }

   /* Add the text for the document */

   CompoundTextInitStyle (&Style);    /* initializes Style to the defaults */
   error = CompoundTextDefineStyle (
                                    hCompound,          /* handle to CompoundText context */
                                    "Normal",           /* style name */
                                    &Style,
                                    &dwStyleID);        /* style id */

   if (error != NOERROR)
   {
       CompoundTextDiscard (hCompound);
       return (error);
   }

   fontfld->PointSize = 12;
   fontfld->Color = NOTES_COLOR_RED;
   fontfld->Attrib = ISBOLD | ISITALIC;
   fontfld->Face = FONT_FACE_SWISS;

   error = CompoundTextAddParagraphExt (
                                        hCompound,               /* handle to CompoundText context */
                                        dwStyleID,               /* style id */
                                        fid,                     /* font id */
                                        text,                    /* paragraph of text to add */
                                        (DWORD)strlen(text),     /* length of text to add */
                                        NULLHANDLE);             /* handle to CLS translation table */

   if (error != NOERROR)
   {
       CompoundTextDiscard (hCompound);
       return (error);
   }

   /* Add the CompoundText context to the note */

   error = CompoundTextClose (
                              hCompound,            /* handle to CompoundText context */
                              NULL,
                              NULL,
                              NULL,
                              0);

   if (error != NOERROR)
       CompoundTextDiscard (hCompound);

   return (error);

}

/************************************************************************

    FUNCTION:   print_doc

    PURPOSE:    prints out the policy and help documents

*************************************************************************/

STATUS LNPUBLIC print_doc (
                           DBHANDLE db_handle,
                           NOTEID note_ID)
{


/* Local data declarations. */

   NOTEHANDLE  note_handle;
   char        field_text[500];
   BLOCKID     field_block;
   DWORD       field_length, text_length;
   WORD        field_type;
   DHANDLE     text_buffer;
   char        *text_ptr;
   STATUS      error;


/* Print the note ID. */

   PRINTLOG ("\n\n       ************* Note ID is: %lX. *************\n",
              note_ID);

/* Open the note. */

   if (error = NSFNoteOpen (
                            db_handle,              /* database handle */
                            note_ID,                /* note ID */
                            0,                      /* open flags */
                            &note_handle))          /* note handle (return) */

       return (error);

/* Look for the $BODY field within the note. This function tells us
whether the field is there, and if present, its location (BLOCKID)
within Domino and Notes' memory. Check the return code for "field not
found" versus a real error. */

   error = NSFItemInfo (
                        note_handle,             /* note handle */
                        "$BODY",                 /* field we want */
                        (WORD) strlen("$BODY"),  /* length of above */
                        NULL,                    /* full field (return) */
                        &field_type,             /* field type (return) */
                        &field_block,            /* field contents (return) */
                        &field_length);          /* field length (return) */

   if (error)
   {
       if (ERR(error) == ERR_ITEM_NOT_FOUND)
           PRINTLOG ("\n Error:  $BODY field not found \n");
       NSFNoteClose (note_handle);
       return (error);
   }

/* Extract only the text from the rich-text field into an ASCII string. */

   if (error = ConvertItemToText (
                                  field_block,   /* BLOCKID of field */
                                  field_length,  /* length of field */
                                  "\n",          /* line separator for output */
                                  60,            /* line length in output */
                                  &text_buffer,  /* output buffer */
                                  &text_length,  /* output length */
                                  TRUE))         /* strip tabs */
   {
       NSFNoteClose (note_handle);
       return (error);
   }

/* Lock the memory allocated for the text buffer. Cast the resulting
pointer to the type we need. */

   text_ptr = (char *) OSLockObject (text_buffer);

/* Move the text from the text buffer into a character array and
append a null to it. (We do this so that we now have a regular C
text string.) */

   memcpy (field_text, text_ptr, (short) text_length);
   field_text[text_length] = '\0';

/* Print the text of the $BODY field. */

   PRINTLOG ("\nThe text in this document is:\n\n%s\n", field_text);

/* Unlock and free the text buffer. */

   OSUnlockObject (text_buffer);
   OSMemFree (text_buffer);

/* Close the note. */

   if (error = NSFNoteClose (note_handle))
       return (error);

/* End of subroutine. */

   return (NOERROR);

}


/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:     argc, argv - directly from the command line

    OUTPUTS:    db_name -     database name obtained from command line or
                              from prompt.

*************************************************************************/

void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                             char *db_name)
{
   if (argc != 2)
   {

       printf("Enter database name: ");
       fflush (stdout);
       gets(db_name);
       printf("\n");

   }
   else
   {
       strcpy(db_name, argv[1]);
   } /* end if */

} /* ProcessArgs */
