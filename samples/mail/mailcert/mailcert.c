/************************************************************************
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

    PROGRAM:    mail_cert

    FILE:       mail_cert.c

    PURPOSE:   demonstrates the SECGetSignerInfoFromMail function 

    SYNTAX:     mail_cert <maildb>

    DESCRIPTION:
        This program demonstrates the SECGetSignerInfoFromMail function, 
        get certificate from the s-MIME mail documents

*************************************************************************/

/* C include files */

#include <stdio.h>
#include <string.h>
#if defined(NT) || defined (UNIX)
   #include <ctype.h>
#endif

/* HCL C API for Notes/Domino include files */
#include <global.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <nsfsearc.h>
#include <osmem.h>
#include <nsferr.h>
#include <idtable.h>            /* IDCreateTable */
#include <osmisc.h>
#include <kfm.h>
#include <mail.h>
#include <mailserv.h>
#include "../../printLog.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif


#define     ITEM_NAME_CATEGORIES    "Categories"
#define     CATEGORIES_VALUE_P      "p"
#define     CATEGORIES_VALUE_Q      "q"
#define     CATEGORIES_VALUE_R      "r"


/* Function prototypes */

STATUS LNPUBLIC AddIDUnique (void far *, SEARCH_MATCH far *, ITEM_TABLE far *);
STATUS LNPUBLIC GetCertificate(void far *, DWORD);
void  LNPUBLIC  ProcessArgs (char *DBFileName);
SECGETSIGNERINFOPROC VerifyCertCallBack (void *, void *, DWORD, WORD, WORD);

/************************************************************************

   FUNCTION:   Main

   PURPOSE:    Main Notes HCL C API for Notes/Domino routine

   DESCRIPTION:
         

*************************************************************************/
int main (int argc, char *argv[])
{
    char        *szDBName;
    DBHANDLE    hDB;
    STATUS      error;
    DHANDLE     hNoteIDTable;

    char       pname[MAXPATH] = "";         /* buffer to store the input path to database */
    char       *path_name;                  /* pathname of database */
    char       *db_name;                    /* name of database */


/* Initialize pointers to point to "" */
    path_name = pname;
    db_name = pname;


/*  Command line parameters. */
    if (argc != 2)
    {
        PRINTLOG ("\nUsage:  %s  <db filename>  \n", argv[0]);
        return (NOERROR);
    }

    szDBName = argv[1];

/* Initialize Notes Session*/
    if (error = NotesInitExtended (argc, argv))
    {
        PRINTLOG("\n Unable to initialize Notes. Error Code[0x%04x]\n", error);
        return ERR(error);
    }


/*  Open the database. */
    if (error = NSFDbOpen (szDBName, &hDB))
    {
        PRINTLOG ("Error: unable to open database '%s'.\n", szDBName);
        PRINTERROR (error,"NSFDbOpen");
        return (ERR(error));
    }

/*  Create ID table then call to NSFSearch.  NSFSearch will find
    all data notes and call the action routine, AddIDUnique, on each.
    AddIDUnique adds the ID of the data note to the ID table if and
    only if the ID is not already in the table.  The result is a table
    of Note IDs where each ID is guaranteed to appear only once.
*/
    if (error = IDCreateTable(sizeof(NOTEID), &hNoteIDTable))
    {
        PRINTLOG ("Error: unable to create ID table.\n");
        PRINTERROR(error,"IDCreateTable");
        NSFDbClose (hDB);
        return (ERR(error));
    }

    if (error = NSFSearch (
        hDB,            /* database handle */
        NULLHANDLE,     /* selection formula (select all notes) */
        NULL,           /* title of view in selection formula */
        0,              /* search flags */
        NOTE_CLASS_DOCUMENT,/* note class to find */
        NULL,           /* starting date (unused) */
        AddIDUnique,    /* call for each note found */
        &hNoteIDTable,  /* argument to AddIDUnique */
        NULL))          /* returned ending date (unused) */
    {
        PRINTLOG ("Error: unable to search database.\n");
        PRINTERROR(error,"NSFSearch");
        IDDestroyTable(hNoteIDTable);
        NSFDbClose (hDB);
        return ERR(error);
    }

    /*  Loop over note IDs in the table. Call ChangeCategory on each. */
    if (error = IDEnumerate(hNoteIDTable,
                            GetCertificate, /* called for each ID */
                            &hDB))          /* arg passed to func */
    {
        PRINTLOG ("Error: unable to enumerate documents in ID table.\n");
        PRINTERROR(error,"IDEnumerate");
    }

    IDDestroyTable(hNoteIDTable);

    NSFDbClose (hDB);

    if (error == NOERROR)
        PRINTLOG("\nProgram completed successfully.\n");

    NotesTerm();
    return (0);
}

/************************************************************************

   FUNCTION:   AddIDUnique

   PURPOSE:    This is the action routine called for each note found
               by NSFSearch.

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

    if (!(SearchMatch.SERetFlags & SE_FMATCH))
        return (NOERROR);


    hNoteIDTable = *((DHANDLE far *)phNoteIDTable);

    if (error = IDInsert(hNoteIDTable, SearchMatch.ID.NoteID, &flagOK))
    {
        PRINTLOG ("Error: unable to insert note ID into table.\n");
        PRINTERROR(error,"IDInsert");
        return ERR(error);
    }

    if (flagOK == TRUE)
    {
        PRINTLOG ("\tInserted note %lX into table.\n", SearchMatch.ID.NoteID);
    }
    else
    {
        PRINTLOG ("\tNote %lX is already in table.\n", SearchMatch.ID.NoteID);
    }

    return (NOERROR);
}

/************************************************************************

    FUNCTION:   GetCertificate

*************************************************************************/

STATUS LNPUBLIC GetCertificate (void far * phDB, DWORD NoteID)
{
    DBHANDLE    hDB;
    STATUS      error;
    NOTEHANDLE  hNote;
    char        String[255];
    WORD        StringLength;
    
    void *pCallCtx = NULL;

    PRINTLOG ("\tProcessing note %lX.\n", NoteID);
    hDB = *( (DBHANDLE far *)phDB );

    if (error = NSFNoteOpenExt (
                                hDB,
                                NoteID,
                                OPEN_RAW_MIME,
                                &hNote))
    {
        PRINTLOG ("Error: unable to open note.\n");
        PRINTERROR(error,"NSFNoteOpenExt");
        return ERR(error);
    }

    if (NSFNoteIsSignedOrSealed (hNote, (BOOL far *) NULL, (BOOL far *) NULL) )
    {
        PRINTLOG("this mail was signed\n"); 
        MailGetMessageItem (hNote, MAIL_SUBJECT_ITEM_NUM, String,
                                      MAXSPRINTF, &StringLength);
        String[StringLength] = '\0';
        PRINTLOG("\nSUBJECT:%s\n", String);
        PRINTLOG(" Calling SECGetSignerInfoFromMail ...\n");
        if (error = SECGetSignerInfoFromMail(hNote, (SECGETSIGNERINFOPROC) &VerifyCertCallBack, 
                                  pCallCtx, 0, 0))
        {
            PRINTLOG("  Unable to get signerInfoFrom Mail.\n");
            PRINTERROR(error,"SECGetSignerInfoFromMail");
            return ERR(error);
        }
        else
        {
            PRINTLOG("  SECGetSignerInfoFromMail had no errors.\n\n"); 
        }
    }

    NSFNoteClose(hNote);
   
    return (NOERROR);
  
}

/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:

    OUTPUTS:    DBFileName, inputted by user

 *************************************************************************/

void  LNPUBLIC  ProcessArgs (char *DBFileName)
{
    PRINTLOG("Enter name of database: ");
    fflush(stdout);
    gets(DBFileName);

} /* ProcessArgs */

/************************************************************************

    FUNCTION:    VerifyCertCallBack

    PURPOSE:     callback function - displays CertSize

    INPUT:

*************************************************************************/

SECGETSIGNERINFOPROC VerifyCertCallBack (void *pCallCtx, void *pCert, DWORD CertSize, WORD Res1, WORD Res2)
{
    PRINTLOG("  Inside VerifyCertCallBack...\n");
    PRINTLOG("  Internet certificate found - CertSize = %ld\n\n", CertSize);
    return (NOERROR);
}
