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

    PROGRAM:    encrypt

    FILE:       encrypt.c

    SYNTAX:     encrypt  <database filename> <encryption key>

    PURPOSE:    Shows how to encrypt, decrypt, and encryption-enable data.

    DESCRIPTION:
        This program creates a document with a field that is encryption 
        enabled. It then creates a second document, encrypting the 
        encryption enabled field. The contents of the encryption enabled 
        field of each newly created document is then displayed (the 
        encrypted note is first decrypted).

****************************************************************************/

/* OS and C include files */

#include <stdio.h>
#include <string.h>

/* HCL C API for Notes/Domino include files */


#include <global.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <ostime.h>
#include <stdnames.h>
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

STATUS DecryptAndReadNoteItem (DBHANDLE hDB, NOTEID dwNoteID, char *ItemName);
void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                               char *dbpath_name, char *ekey, char *user1, char *user2); 

#define ENCRYPTED_ITEM "SECRET"

/************************************************************************

    FUNCTION:   Main

    PURPOSE:    Main Entry Point

    ALGORITHM:
      This creates a document and marks one of the fields in the
      document as encryption enabled by setting the ITEM_SEAL field flag.
      The data in this field is not encrypted.

      A second document is created similarly to the first document with the
      addition that the data in the encryption enabled field is encrypted
      with the secret key supplied on the command line and with the public
      keys of USER1 and USER2. 

      The note id's for each of the two newly created documents are then
      displayed.  If the document has been encrypted, it is then decrypted. 
      The contents of the encryption enabled field is then displayed.

*************************************************************************/

int main(int argc, char *argv[])
{


    /* Local data declarations */

    char        PathName[MAXPATH]; /* pathname of database */
    char        EncryptKey[100];   /* name of secret encryption key */
    DBHANDLE    hDB;               /* database handle */
    NOTEHANDLE  hNote;             /* note handle */
    char        TextField[100];    /* contents of a text field */
    NUMBER      NumField = 1;      /* contents of a numeric field */
    TIMEDATE    TimeField;         /* contents of a time/date field */
    NOTEID      NoteID1, NoteID2;  /* note id's of the created documents */
    NOTEHANDLE  hEncryptedNote;    /* note handle of encrypted note */
    STATUS      error = NOERROR;   /* return code from API calls */
    char        user1[MAXPATH];
    char        user2[MAXPATH];


    
    error = NotesInitExtended (argc, argv);
    if (error)
    {
        PRINTLOG("Error: Unable to initialize Notes.\n");
        return (1);
    }

    /* Get the pathname of the database. 
       Also get the secret encryption key name with which to encrypt the 
       document */

    ProcessArgs(argc, argv, PathName, EncryptKey, user1, user2);

    /* Open the database. */

    if (error = NSFDbOpen (PathName, &hDB))
    {
        PRINTERROR(error,"NSFDbOpen");
        NotesTerm();
        return (1);
    }

   /* Create the first data note. */

    if (error = NSFNoteCreate (hDB, &hNote))
    {
        PRINTERROR(error,"NSFNoteCreate");
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
    }

    /* Write a field named FORM to the note -- this field specifies the
       form to use when displaying the note. */

    if (error = NSFItemSetText (hNote, 
                                "FORM",
                                "SimpleDataForm", 
                                 MAXWORD))
    {
        PRINTERROR(error,"NSFItemSetText");
        NSFNoteClose (hNote);
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
    }

    /* Write a text field named PLAIN_TEXT to the note. 
       Set the ITEM_SUMMARY flag so the field can be seen in a view. */

    strcpy (TextField, "This is an example of an encryption enabled document.");

    if (error = NSFItemAppend (hNote,
                               ITEM_SUMMARY,
                               "PLAIN_TEXT", (WORD)strlen("PLAIN_TEXT"),
                               TYPE_TEXT,
                               TextField, (DWORD)strlen(TextField)))
    {
        PRINTERROR(error,"NSFItemAppend");
        NSFNoteClose (hNote);
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
    }

    /* Write a numeric field named NUMBER to the note. 
       Set the ITEM_SUMMARY flag so the field can be seen in a view. */

    if (error = NSFItemAppend (hNote,
                               ITEM_SUMMARY,
                               "NUMBER", (WORD)strlen("NUMBER"),
                               TYPE_NUMBER,
                               &NumField, (DWORD)sizeof(NumField)))
    {
        PRINTERROR(error,"NSFItemAppend");
        NSFNoteClose (hNote);
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
    }


    /* Write a time/date field named TIME_DATE to the note. 
       Set the ITEM_SUMMARY flag so the field can be seen in a view. */

    OSCurrentTIMEDATE(&TimeField);

    if (error = NSFItemAppend (hNote,
                               ITEM_SUMMARY,
                               "TIME_DATE", (WORD)strlen("TIME_DATE"),
                               TYPE_TIME,
                               &TimeField, (DWORD)sizeof(TimeField)))
    {
        PRINTERROR(error,"NSFItemAppend");
        NSFNoteClose (hNote);
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
    }

    /* Write a text field named, ENCRYPTED_ITEM, to the note. Set the 
       ITEM_SEAL flag so the field is encryption enabled.  */

    strcpy (TextField, "This is an encryption enabled field.  ");
    strcat (TextField, "The data, however, is not encrypted.");

    if (error = NSFItemAppend (hNote,
                               ITEM_SUMMARY | ITEM_SEAL,
                               ENCRYPTED_ITEM, (WORD)strlen(ENCRYPTED_ITEM),
                               TYPE_TEXT,
                               TextField, (DWORD)strlen(TextField)))
    {
        PRINTERROR(error,"NSFItemAppend");
        NSFNoteClose (hNote);
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
    }

    /* Add the entire new note (with all fields) to the database. */

    if (error = NSFNoteUpdate (hNote, 0))
    {
        PRINTERROR(error,"NSFNoteUpdate");
        NSFNoteClose (hNote);
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
    }

    /* Store the NOTEID.  It will be used later to read the encrypted
       enabled field. */

    NSFNoteGetInfo (hNote, _NOTE_ID, &NoteID1);

    if (error = NSFNoteClose (hNote))
    {
        PRINTERROR(error,"NSFNoteClose");
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
    }

    /* Create a second note */

    if (error = NSFNoteCreate (hDB, &hNote))
    {
        PRINTERROR(error,"NSFNoteCreate");
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
    }

    /* Write a field named FORM to the note -- this field specifies the
       form to use when displaying the note. */

    if (error = NSFItemSetText (hNote, 
                                "FORM",
                                "SimpleDataForm", 
                                 MAXWORD))
    {
        PRINTERROR(error,"NSFItemSetText");
        NSFNoteClose (hNote);
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
    }

    /* Write a text field named PLAIN_TEXT to the note. 
       Set the ITEM_SUMMARY flag so the field can be seen in a view. */

    strcpy (TextField, "This is an example of an encrypted document.");

    if (error = NSFItemAppend (hNote,
                               ITEM_SUMMARY,
                               "PLAIN_TEXT", (WORD)strlen("PLAIN_TEXT"),
                               TYPE_TEXT,
                               TextField, (DWORD)strlen(TextField)))
    {
        PRINTERROR(error,"NSFItemAppend");
        NSFNoteClose (hNote);
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
    }

    /* Write a numeric field named NUMBER to the note. 
       Set the ITEM_SUMMARY flag so the field can be seen in a view. */

    ++NumField;

    if (error = NSFItemAppend (hNote,
                               ITEM_SUMMARY,
                               "NUMBER", (WORD)strlen("NUMBER"),
                               TYPE_NUMBER,
                               &NumField, (DWORD)sizeof(NumField)))
    {
        PRINTERROR(error,"NSFItemAppend");
        NSFNoteClose (hNote);
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
    }

    /* Write a time/date field named TIME_DATE to the note. 
       Set the ITEM_SUMMARY flag so the field can be seen in a view. */

    OSCurrentTIMEDATE(&TimeField);

    if (error = NSFItemAppend (hNote,
                               ITEM_SUMMARY,
                               "TIME_DATE", (WORD)strlen("TIME_DATE"),
                               TYPE_TIME,
                               &TimeField, (DWORD)sizeof(TimeField)))
    {
        PRINTERROR(error,"NSFItemAppend");
        NSFNoteClose (hNote);
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
    }

    /* Write a text field named, ENCRYPTED_ITEM to the note. Set the 
       ITEM_SEAL flag so the field is encryption enabled.  */

    strcpy (TextField, "This is an encryption enabled field.  ");
    strcat (TextField, "The data is encrypted.");

    if (error = NSFItemAppend (hNote,
                               ITEM_SUMMARY | ITEM_SEAL,
                               ENCRYPTED_ITEM, (WORD)strlen(ENCRYPTED_ITEM),
                               TYPE_TEXT,
                               TextField, (DWORD)strlen(TextField)))
    {
        PRINTERROR(error,"NSFItemAppend");
        NSFNoteClose (hNote);
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
    }
  
    /* Write an item, ITEM_NAME_NOTE_SEALNAMES, to the note with the 
       secret encryption key name. This is an item of TYPE_TEXT
       with the ITEM_SUMMARY flag set. Users who have this secret
       encryption key will be able to decrypt this item. */

    if (error = NSFItemAppend (hNote,
                               ITEM_SUMMARY,
                               ITEM_NAME_NOTE_SEALNAMES, (WORD)strlen(ITEM_NAME_NOTE_SEALNAMES),
                               TYPE_TEXT,
                               EncryptKey, (DWORD)strlen(EncryptKey)))
    {
        PRINTERROR(error,"NSFItemAppend");
        NSFNoteClose (hNote);
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
    }

    /* New for Notes/Domino 6 */
    /* Write an item, ITEM_NAME_NOTE_SEALUSERS, to the note with a set of user
       names.  For these users, the note will be encrypted with the user's public key.
       These users will be able to decrypt this item with their private keys */

    if (error = NSFItemCreateTextList (hNote, 
                                       ITEM_NAME_NOTE_SEALUSERS, 
                                       user1,
                                       MAXWORD))
    {
        PRINTERROR(error,"NSFItemCreateTextList");
        NSFNoteClose (hNote);
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
    }

    /* Add another user to the ITEM_NAME_NOTE_SEALUSERS */
    
    if (error = NSFItemAppendTextList (hNote, 
                                       ITEM_NAME_NOTE_SEALUSERS, 
                                       user2,
                                       MAXWORD, 
                                       FALSE)) 
    {
        PRINTERROR(error,"NSFItemAppendTextList");
        NSFNoteClose (hNote);
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
    }
  
  
   
   /* Encrypt the note */

    if (error = NSFNoteCopyAndEncrypt (hNote, 0, &hEncryptedNote))
    {
        PRINTERROR(error,"NSFNoteCopyAndEncrypt");
        NSFNoteClose (hNote);
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
    }

    if (error = NSFNoteUpdate (hEncryptedNote, 0))
    {
        PRINTERROR(error,"NSFNoteUpdate");
        NSFNoteClose (hNote);
        NSFNoteClose (hEncryptedNote);
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
   }

    /* Store the NOTEID of this document.  It will be used later to
       decrypt and read the document. */

    NSFNoteGetInfo (hEncryptedNote, _NOTE_ID, &NoteID2);

    /* Close the notes. (Remove its structure from memory.) */

    if (error = NSFNoteClose (hNote))
    {
        PRINTERROR(error,"NSFNoteClose");
        NSFNoteClose (hEncryptedNote);
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
    }

    if (error = NSFNoteClose (hEncryptedNote))
    {
        PRINTERROR(error,"NSFNoteClose");
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
    }

    /* Demonstrate reading encrypted fields. */

    if (error = DecryptAndReadNoteItem (hDB, NoteID1, ENCRYPTED_ITEM))
    {
        PRINTERROR(error,"DecryptAndReadNoteItem");
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
    }

    if (error = DecryptAndReadNoteItem (hDB, NoteID2, ENCRYPTED_ITEM))
    {
        PRINTERROR(error,"DecryptAndReadNoteItem");
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
    }

    /* Close the database */

    if (error = NSFDbClose (hDB))
    {
        PRINTERROR(error,"NSFDbClose");
        NSFDbClose (hDB);
        NotesTerm();
        return (1);
    }

/* End of subroutine. */
    PRINTLOG("\nProgram completed successfully.\n");
    return (0);
}

/************************************************************************

    FUNCTION:   DecryptAndReadNoteItem

    PURPOSE:    Decrypt the note and print the note ID and a text field.
    
    ALGORITHM:
    This function opens the note with the specified NOTEID.  It tests 
    whether the note is encrypted.  If the note is encrypted, it decrypts 
    the note.  It then prints out the note id and the contents of the 
    specified text field.

    INPUTS:
        DBHANDLE    - Handle to the open database.
        dwNoteID    - Note id.
        ItemName    - Name of the text item to be read.

    RETURNS:
        status returned from an API call

*************************************************************************/

STATUS DecryptAndReadNoteItem (DBHANDLE hDB, NOTEID dwNoteID, char *ItemName)

{
    NOTEHANDLE hNote;           /* note handle */
    STATUS     error;           /* return code from API calls */
    FLAG       fSealed;         /* Is not encrypted? */
    BOOL       FieldFound;
    WORD          len;
    char          ItemText[500];


    if (error = NSFNoteOpen (
                             hDB,           /* database handle */
                             dwNoteID,      /* note id */
                             0,             /* open flags */
                             &hNote))        /* note handle (return) */
      
      return (error);

   /* If the note is encrypted, decrypt it. */

    if (NSFNoteIsSignedOrSealed (hNote, (BOOL far *) NULL, (BOOL far *) &fSealed) )
        if (fSealed)
            if (error = NSFNoteDecrypt (
                                         hNote,    /* note handle */
                                         0,        /* reserved */
                                         NULL))    /* Key for attachments - not
                                                  needed */
            {
                NSFNoteClose (hNote);
                return (error);
            }

    PRINTLOG("\n\n\nNote ID:  %lX\n\n", dwNoteID);

    FieldFound = NSFItemIsPresent (hNote,
                                   ItemName,
                                   (WORD)strlen (ItemName));

    if (FieldFound)
    {

        len = NSFItemGetText ( 
                               hNote, 
                               ItemName,
                               ItemText,
                               sizeof (ItemText));

        if (fSealed)
        {
            PRINTLOG ("The %s field has been decrypted.\n", ItemName);
        }
        else
        {
            PRINTLOG ("The %s field is not encrypted.\n", ItemName);
        }

        PRINTLOG ("Contents of %s field:\n\n%s\n", ItemName, ItemText);

    }

    /* If the specified field is not there, print a message. */

    else
    {
        PRINTLOG ("%s field not found.\n", ItemName);
    }

    return (NOERROR);
                     
}


/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:     argc, argv - directly from the command line
   
    OUTPUTS:    dbname - server name obtained from command line or
                              from prompt.
                ekey -        secret encryption key
                              from prompt.

*************************************************************************/

void  LNPUBLIC  ProcessArgs (int argc, char *argv[], 
                               char *dbname, char *ekey, char *user1, char *user2)
{ 
    if (argc != 5)
    {
        printf("Enter database name: ");      
        fflush (stdout);
        gets(dbname);
        printf("\n");
        
        printf ("Enter encryption key:  ");
        fflush (stdout);
        gets(ekey);

        printf ("Enter user1 (e.g. CN=test user/O=testorg):  ");
        fflush (stdout);
        gets(user1);

        printf ("Enter user2 (e.g. CN=test user1/O=testorg): ");
        fflush (stdout);
        gets(user2);
    }    
    else
    {
        strcpy(dbname, argv[1]);    
        strcpy(ekey, argv[2]);
        strcpy(user1, argv[3]);
        strcpy(user2, argv[4]);
    } /* end if */

} /* ProcessArgs */
