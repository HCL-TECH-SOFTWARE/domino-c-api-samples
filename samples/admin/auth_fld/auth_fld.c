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

    PROGRAM:    auth_fld

    FILE:       auth_fld.c

    SYNTAX:     auth_fld <server> <database> <textfile1> <textfile2>

    FUNCTION:   Creates two documents containing author fields,
                a main topic and a response document.

    PURPOSE:    Demonstrates how to append author fields to documents.
                Creates items with data type "Names", "Author Names",
                and "Reader Names".

    DESCRIPTION:
 
    <server> is the HCL Domino server where the destination database
    resides. This option is required.  You may specify "" for <server> 
    to indicate the database is in the local Notes data directory.  
    If you specify a database on a HCL Domino server, then the documents 
    created by this sample will not be visible to you because you are not 
    listed in the Reader Names field.
   
    <database> must specify the file name of the Domino discussion 
    database (DISCUSS.NSF) included with this API sample program. 
    This assumes the database exists and has the design of DISCUSS.NSF.
    Also, the user running this API program must have author access or
    better to this database.

    <textfile1> must name an ASCII text file residing in the current 
    working directory. The contents of <texfile1> becomes the body of 
    the main topic document. The first line of <textfile1> is used as
    the subject of the main topic document. 

    <textfile2> must name another ASCII text file residing in the current 
    working directory. The entire contents of <texfile2> becomes the 
    body of the response document. The first line of <textfile2> is used
    as the subject of the response.

    EXAMPLE:    auth_fld "" discuss maindoc.txt respdoc.txt

    auth_fld creates two documents in the specified discussion database:
    a main topic document and a response document. 

    The two ASCII text files specified provide the body fields of the 
    main and response documents, respectively.

    auth_fld sets the author field of the main and response documents
    to the following hierarchical names:

         CN=Stephen N. Phillips/O=HCL Corportation
         CN=Shelby T. Williams/O=HCL Corportation

    auth_fld parses out the first line of each text file. It obtains the
    the "Subject" field of the main document from the first line of the 
    first file, and the subject of the response document from the first 
    line of the second file.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif
                                                              
#include <stdio.h>
#if defined(OS400) || defined (AIX64) || defined(MAC)
#include <stdlib.h>
#else
#include <malloc.h>
#endif
#include <string.h>

/* Notes API include files */

#include <global.h>                     /* STATUS, LIST, etc. */
#include <nsfdb.h>                      /* NSFDbOpen, etc */
#include <nsfnote.h>                    /* ITEM_READWRITERS, etc. */
#include <names.h>                      /* MAXPATH */
#include <stdnames.h>                   /* FIELD_FORM */
#include <osfile.h>                     /* OSPathNetConstruct */
#include <editods.h>                    /* DEFAULT_FONT_ID */
#include <easycd.h>                     /* CompoundTextCreate, etc. */
#include <ods.h>
#include <editods.h>
#if defined(OS400)
#include <misc.h>
#endif

#include <osmisc.h>
#include "../../printLog.h"
#include "auth_fld.h"                   /* Local definitions */

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/************************************************************************
                                                                        
    FUNCTION:   Main                                               
                                                                        
*************************************************************************/

int main (int argc, char *argv[])
{
    char        szServerName[STRING_LENGTH];   
                                /* name of HCL Domino Server where DB resides */
    char        szDbPathName[STRING_LENGTH];   
                                /* pathname of database, e.g. "discuss" */
    char        szMainFileName[STRING_LENGTH]; 
                                /* ASCII file containing Main Doc body */
    char        szRespFileName[STRING_LENGTH]; 
                                /* ASCII file containing Reponse body */
    DBHANDLE    hDb;            /* handle to database specified */
    char      * szMainSubject;  /* subject of Main Topic document */
    char      * szRespSubject;  /* subject of Response document */
    OID         MainNoteOID;    /* OID of Main Topic document */
    STATUS      error=NOERROR;  /* return code from API calls */


   /* Process Arguments */
    ProcessArgs(argc, argv, szServerName, szDbPathName, szMainFileName, szRespFileName);

    if (error = NotesInitExtended (argc, argv))
    {
       PRINTLOG("\n Unable to initialize Notes.\n");
       return (1);
    }

    /* Open Domino DB. Sets hDb */
    if (error = OpenNotesDB (szServerName, szDbPathName, &hDb))
        goto Exit0;

    /*  Get first line from first input file to use as subject of main topic
        document. If successfull, szMainSubject is allocated and initialized. 
    */
    if (error = GetFileFirstLine (szMainFileName, &szMainSubject))
        goto Exit1;

    /* Get first line from second file to use as subject of response
       document. If successfull, szRespSubject is allocated and initialized.
    */
    if (error = GetFileFirstLine (szRespFileName, &szRespSubject))
        goto Exit2;

    /* Create main topic document. If successfull, this returns the OID
       of the main doc, which we need when creating the response doc.
    */
    if (error = CreateMainTopic (hDb, szMainSubject, szMainFileName, 
                                 &MainNoteOID))
    {
        goto Exit3;
    }

    /* Create response document. Specify the OID of the main document
       so that this can set the $REF item to point back to the main doc.
     */
    if (error = CreateResponse (hDb, szMainSubject,
                                szRespSubject, 
                                szRespFileName,
                                MainNoteOID))
    {
        goto Exit3;
    }

    PRINTLOG ("auth_fld: added 2 documents to %s.\n", szDbPathName);

Exit3:
    free (szRespSubject);

Exit2:
    free (szMainSubject);
                        
Exit1:
    NSFDbClose (hDb);

Exit0:
    if (error)
       PRINTERROR (error,"OpenNotesDB");

    NotesTerm();

    return(error);
}                     

/************************************************************************
                                                                        
     FUNCTION:  OpenNotesDB()                                           
                                                                        
     PURPOSE:   Open the Domino Db (nsf file) on the specified server.   
                                                                        
*************************************************************************/

STATUS LNPUBLIC   OpenNotesDB(char * pServer, char * pNsfName, 
                                DBHANDLE * phDb)
{
    STATUS  error ;
    char    fullpath_name[MAXPATH];


    if (strlen (pServer) != 0) /* remote server specified */
    {
        error = OSPathNetConstruct (NULL, pServer, pNsfName, fullpath_name);
        if (error != NOERROR)
        {
            PRINTLOG("Error: unable to construct network path to database.\n");
            return(error);
        }
    }
    else                                /* DB is LOCAL: just use nsf name */
        strcpy(fullpath_name, pNsfName);               

    /* Check user input in case DB full path name too long */
    if (strlen(fullpath_name) > MAXPATH)
    {
        PRINTLOG("Error: Database full path name: '%s'\n is longer than %i\n",
                fullpath_name, MAXPATH);
        return(ERR_AUTH_FLD_DBOPEN);
    }

    if (error = NSFDbOpen (fullpath_name, phDb))
    {
        PRINTLOG("Error: unable to open database '%s'.\n", fullpath_name);
        return (error);
    }                    
    return (NOERROR);
}


/************************************************************************

    FUNCTION:   GetFileFirstLine (char * szFileName, char ** szFirstLine)

    PURPOSE:    Open file and read the first line

*************************************************************************/

STATUS  LNPUBLIC  GetFileFirstLine (char * szFileName, char ** szFirstLine)
{
    FILE   *pFile;

    if (NULL == (pFile = fopen (szFileName, "r")))
    {
        PRINTLOG ("Error: unable to open input file '%s'.\n", szFileName);
        return(ERR_AUTH_FLD_FILEOPEN);
    }

    *szFirstLine = (char *) malloc ( (size_t)FIRST_LINE_MAX_SIZE );

    if (*szFirstLine == NULL)
    {
        PRINTLOG ("Error: unable to allocate %lu bytes memory.\n", 
                                            (DWORD)FIRST_LINE_MAX_SIZE);
        fclose (pFile);
        return(ERR_AUTH_FLD_MALLOC);
    }
 
    if (fgets (*szFirstLine, FIRST_LINE_MAX_SIZE, pFile) == NULL)
    {
        if (feof (pFile))               /* input file empty */
        {
            PRINTLOG ("Error: input file '%s' is empty.\n", szFileName);
            fclose (pFile);
            return (ERR_AUTH_FLD_EOF);
        }
        else                    /* some other file read error encountered */
        {
            PRINTLOG ("Error: unable to read input file '%s'.\n", szFileName);
            fclose (pFile);
            return (ERR_AUTH_FLD_READ_ERROR);
        }
    }

    (*szFirstLine)[strlen (*szFirstLine) - 1 ] = '\0'; /* trim off \n */

    fclose (pFile);
    
    return NOERROR;
}


/************************************************************************

    FUNCTION:   CreateMainTopic

    PURPOSE:    Create a Main Topic document in the database. Return the
                Originator ID (OID) of the main document to the caller.

    INPUTS:     DBHANDLE    hDb             handle to open database
                char       *szMainSubject   subject of Main Topic
                char       *szMainFileName  text file for Body field

    OUTPUTS:    OID        *poidMainNote    gets OID of Main Topic note

    RETURNS:    status code or NOERROR

*************************************************************************/

STATUS  LNPUBLIC  CreateMainTopic (DBHANDLE hDb, 
                                   char * szMainSubject, 
                                   char * szMainFileName, 
                                   OID  * poidMainNote)
{
    NOTEHANDLE  hNote;
    STATUS      error;

    /* Create a new data note. */
    if (error = NSFNoteCreate (hDb, &hNote))
    {
        PRINTLOG ("Error: unable to create main topic document.\n");
        goto Exit0;
    }

    /* Append Form item to the note: "MainTopic" */
    if (error = AppendFormItem (hNote, szMAINTOPIC_FORM))  goto Exit1;

    /* Append Author item to the note. */
    if (error = AppendAuthorItem (hNote, szMAINTOPIC_AUTHOR))  goto Exit1;

    /* Editors */
    if (error = AppendEditorsItem (hNote))  goto Exit1;

    /* Readers */
    if (error = AppendReadersItem (hNote))  goto Exit1;

    /* Categories */
    if (error = AppendCategoriesItem (hNote)) goto Exit1;

    /* Body: text from the first input file */
    if (error = AppendBodyItem (hNote, szMainFileName)) goto Exit1;

    /* Subject: first line of first input file */
    if (error = AppendSubjectItem (hNote, szMainSubject))  goto Exit1;

    /* Append $UpdatedBy item to the note. */
    if (error = AppendUpdatedByItem (hNote, szMAINTOPIC_AUTHOR))  goto Exit1;

    /* Update the Main Topic document to the database. */
    if (error = NSFNoteUpdate (hNote, 0))
    {
        PRINTLOG ("Error: unable to update main document to disk.\n");
        goto Exit1;
    }
        
    /* Get the Note OID of the Main Topic document. Do this after updating.*/
    NSFNoteGetInfo (hNote, _NOTE_OID, poidMainNote);

Exit1:
    NSFNoteClose (hNote);

Exit0:
    return (error);
}

/************************************************************************

    FUNCTION:   CreateResponse

    PURPOSE:    Create a Response document in the database. Doc is a 
                response to the note specified by the Originator ID.

    INPUTS:     DBHANDLE    hDb             handle to open database
                char       *szMainSubject   subject of Main Topic
                char       *szRespSubject   subject of Response
                char       *szRespFileName  text file for Body field
                OID         oidMainNote     gets OID of Main Topic note

    RETURNS:    status code or NOERROR

*************************************************************************/

STATUS  LNPUBLIC  CreateResponse (DBHANDLE hDb, 
                                char * szMainSubject,
                                char * szRespSubject, 
                                char * szRespFileName, 
                                OID    oidMainNote)
{
    NOTEHANDLE  hNote;
    STATUS        error = NOERROR;

    /* Create a new data note. */
    if (error = NSFNoteCreate (hDb, &hNote))
    {
        PRINTLOG ("Error: unable to create response note.\n");
        goto Exit0;
    }

    /* Append Form item to the note: "Response" */
    if (error = AppendFormItem (hNote, szRESPONSE_FORM))  goto Exit1;

    /* Append $REF item to the note. */
    if (error = AppendRefItem (hNote, oidMainNote))  goto Exit1;

    /* Append Author item to the note. */
    if (error = AppendAuthorItem (hNote, szRESPONSE_AUTHOR))  goto Exit1;

    /* Editors */
    if (error = AppendEditorsItem (hNote))  goto Exit1;

    /* Readers */
    if (error = AppendReadersItem (hNote))  goto Exit1;

    /* Body: text from the first input file */
    if (error = AppendBodyItem (hNote, szRespFileName)) goto Exit1;

    /* OriginalSubject */
    if (error = AppendOriginalSubjectItem (hNote, szMainSubject)) goto Exit1;

    /* Subject */
    if (error = AppendSubjectItem (hNote, szRespSubject))  goto Exit1;

    /* Append $UpdatedBy item to the note. */
    if (error = AppendUpdatedByItem (hNote, szRESPONSE_AUTHOR))  goto Exit1;

    /* Update the Response document to the database. */
    if (error = NSFNoteUpdate (hNote, 0))
    {
        PRINTLOG ("Error: unable to update response document to disk.\n");
    }
        
Exit1:
    /* Close the new note. */
    NSFNoteClose (hNote);

Exit0:
    return (error);
}

/************************************************************************
                                                                        
    FUNCTION:   AppendFormItem                                          
                                                                        
*************************************************************************/

STATUS  LNPUBLIC  AppendFormItem (NOTEHANDLE  hNote, char * szForm)
{
    STATUS  error;

    if (error = NSFItemSetText (hNote, 
                                DISCUSS_ITEM_FORM,  /* "Form" */
                                szForm, 
                                MAXWORD))
    {
        PRINTLOG ("Error: unable to set text in Form field.\n");
        return (error);
    }
    return(NOERROR);
}

/************************************************************************
                                                                        
    FUNCTION:  AppendAuthorItem

    PURPOSE:   Append item named "Author" to the open note. 
    
    DESCRIPTION:

    To mimic the behavior of the Notes 3.0 data type "Author Names",
    this appends the item using TYPE_TEXT and sets the item flags 
    ITEM_SUMMARY, ITEM_READWRITERS, and ITEM_NAMES. This requires use 
    of NSFItemAppend rather than NSFItemSetText.
                                                                        
*************************************************************************/

STATUS  LNPUBLIC  AppendAuthorItem (NOTEHANDLE  hNote, char * szAuthor)
{
    STATUS  error;

    if (error = NSFItemAppend ( hNote, 
                                ITEM_SUMMARY | ITEM_READWRITERS | ITEM_NAMES,
                                DISCUSS_ITEM_AUTHOR,  /* "Author" */
                                (WORD) strlen(DISCUSS_ITEM_AUTHOR),
                                TYPE_TEXT,
                                szAuthor,
                                strlen(szAuthor)))
    {
        PRINTLOG ("Error: unable to append Author field.\n");
    }
    return (error);
}

/************************************************************************
                                                                        
    FUNCTION:   AppendEditorsItem

    PURPOSE:    Create and append the Editors item to the note. 
    
    DESCRIPTION:
    
    The Editors item is a text list with two values: EDITORS1 and 
    EDITORS2. To mimic the Notes 3.0 "Author Names" data type, this
    creates an item of TYPE_TEXT_LIST with the following item flags 
    set: ITEM_SUMMARY, ITEM_READWRITERS, and ITEM_NAMES. Note that
    we can't use NSFItemCreateTextList and NSFItemAppendTextList 
    because these functions do not expose the ability to set the
    necessary item flags. Therefore, this creates the text list from 
    scratch and calls NSFItemAppend.

    The format of a two-element item of TYPE_TEXT_LIST is as follows:
                    
                LIST        * list header *
                USHORT      * length of first text string *
                USHORT      * length of second text string *
                text        * text of first string: not terminated *
                text        * text of second string: not terminated *
                                                                        
*************************************************************************/

STATUS  LNPUBLIC  AppendEditorsItem (NOTEHANDLE  hNote)
{
    STATUS      error = NOERROR;
    DWORD       dwValueLen;
    void far   *pvoidItemValue;
    LIST       *pList;
    USHORT     *pusLength;
    USHORT      usLen1, usLen2;
    char       *pchStr;

    usLen1 = strlen(EDITORS1);
    usLen2 = strlen(EDITORS2);

    dwValueLen = sizeof(LIST) + (EDITORS_COUNT * sizeof(USHORT)) +
                 usLen1 + usLen2 ;

    pvoidItemValue = (void far *) malloc ((size_t)dwValueLen);

    if (pvoidItemValue == NULL)
    {
        PRINTLOG ("Error: unable to allocate %lu bytes memory.\n", dwValueLen);
        return(ERR_AUTH_FLD_MALLOC);
    }

    pList = (LIST*)pvoidItemValue;
    pList->ListEntries = EDITORS_COUNT;
    pList++;
    pusLength = (USHORT*)pList;
    *pusLength = usLen1;
    pusLength++;
    *pusLength = usLen2;
    pusLength++;
    pchStr = (char*)pusLength;
    memcpy (pchStr, EDITORS1, usLen1);
    pchStr += usLen1;
    memcpy (pchStr, EDITORS2, usLen2);

    if (error = NSFItemAppend ( hNote, 
                                ITEM_SUMMARY | ITEM_READWRITERS | ITEM_NAMES,
                                DISCUSS_ITEM_EDITORS,  /* "Editors" */
                                (WORD) strlen(DISCUSS_ITEM_EDITORS),
                                TYPE_TEXT_LIST,
                                pvoidItemValue,
                                dwValueLen))
    {
        PRINTLOG ("Error: unable to append Editors field.\n");
    }
    free (pvoidItemValue);
    return (error);
}

/************************************************************************
                                                                        
    FUNCTION:   AppendReadersItem

    PURPOSE:    Create and append the Readers item to the note. 
                                                                        
*************************************************************************/

STATUS  LNPUBLIC  AppendReadersItem (NOTEHANDLE  hNote)
{
    STATUS      error = NOERROR;
    DWORD       dwValueLen;
    void far   *pvoidItemValue;
    LIST       *pList;
    USHORT     *pusLength;
    USHORT      usLen1, usLen2;
    char       *pchStr;

    usLen1 = strlen(READERS1);
    usLen2 = strlen(READERS2);

    dwValueLen = sizeof(LIST) + (READERS_COUNT * sizeof(USHORT)) +
                 usLen1 + usLen2 ;

    pvoidItemValue = (void far *) malloc ((size_t)dwValueLen);
    if (pvoidItemValue == NULL)
    {
        PRINTLOG ("Error: unable to allocate %lu bytes memory.\n", dwValueLen);
        return(ERR_AUTH_FLD_MALLOC);
    }

    pList = (LIST*)pvoidItemValue;
    pList->ListEntries = READERS_COUNT;
    pList++;
    pusLength = (USHORT*)pList;
    *pusLength = usLen1;
    pusLength++;
    *pusLength = usLen2;
    pusLength++;
    pchStr = (char*)pusLength;
    memcpy (pchStr, READERS1, usLen1);
    pchStr += usLen1;
    memcpy (pchStr, READERS2, usLen2);

    if (error = NSFItemAppend ( hNote, 
                                ITEM_SUMMARY | ITEM_READERS | ITEM_NAMES,
                                DISCUSS_ITEM_READERS,  /* "Readers" */
                                (WORD) strlen(DISCUSS_ITEM_READERS),
                                TYPE_TEXT_LIST,
                                pvoidItemValue,
                                dwValueLen))
    {
        PRINTLOG ("Error: unable to append Readers field.\n");
    }
    free (pvoidItemValue);
    return (error);
}

/************************************************************************
                                                                        
    FUNCTION:   AppendCategoriesItem                                    
                                                                        
*************************************************************************/

STATUS  LNPUBLIC  AppendCategoriesItem (NOTEHANDLE hNote)
{
    STATUS      error;

    if (error = NSFItemSetText ( hNote, 
                        DISCUSS_ITEM_CATEGORIES, /* "Categories" */
                        CATEGORIES,              /* "Using Notes" */ 
                        MAXWORD))
    {
        PRINTLOG ("Error: unable to set Categories field.\n");
    }
    return (error);
}

/************************************************************************

    FUNCTION:   AppendBodyItem

    PURPOSE:    Appends a compound (rich) text item "Body" to a note
                containing the text from the specified file.

    INPUTS:      
        NOTEHANDLE  hNote;            open note to append Body field to
        char        *szFileName;      file name of ASCII text file

    RETURNS:   
        NOERROR or status returned from a HCL C API for Domino and 
        Notes function call.

*************************************************************************/

STATUS  LNPUBLIC  AppendBodyItem (NOTEHANDLE hNote, char * szFileName)
{
    STATUS          error;
    DHANDLE         hCompound;      /* Compound Text context */
    COMPOUNDSTYLE   Style;
    DWORD           dwStyleID;
    char            szBlank[] = "";
    char            szNewlineDelimiter[] = "\r\n";

    /* Create compound text context. This initializes hCompound. */
    if (error = CompoundTextCreate (hNote, 
                    szDISCUSS_ITEM_BODY,      /* "Body" */
                    &hCompound)) 
    {
        PRINTLOG ("Error: unable to create Compound Text context.\n");
        return (error);
    }

    /* Define a default compound text style. */
    CompoundTextInitStyle (&Style);
    if (error = CompoundTextDefineStyle (hCompound, szBlank, &Style, 
                                         &dwStyleID))
    {
        PRINTLOG ("Error: unable to define Compound Text style.\n");
        CompoundTextDiscard (hCompound);
        return (error);
    }
    
    /* Add text from the file to the compound text context */
    if (error = CompoundTextAddTextExt (
                        hCompound,              /* context */
                        dwStyleID,              /* style  */
                        DEFAULT_FONT_ID,        /* font ID */
                        szFileName,             /* text file name */
                        (DWORD) strlen (szFileName),/* name length */
                        szNewlineDelimiter,     /* newline delimiter */
                        COMP_FROM_FILE |        /* take input from file */
                        COMP_PRESERVE_LINES,    /* preserve line breaks */
                        NULLHANDLE))            /* CLS translation table */
    {
        PRINTLOG("Error: unable to add text from file '%s' to Compound Text.\n",
                                 szFileName);
        CompoundTextDiscard (hCompound);
        return (error);
    }

    /* Close the compound text context. This adds the CD text to the note. */
    if (error = CompoundTextClose (hCompound, 0, 0L, NULL, 0))
    {
        PRINTLOG("Error: unable to add Compound Text to document.\n");
        CompoundTextDiscard (hCompound);
    }
    return (error);
}

/************************************************************************
                                                                        
    FUNCTION:   AppendSubjectItem                                       
                                                                        
*************************************************************************/

STATUS LNPUBLIC AppendSubjectItem(NOTEHANDLE hNote, char * szSubject)
{
    STATUS  error;

    if (error = NSFItemSetText ( hNote, 
                    DISCUSS_ITEM_SUBJECT,   /* "Subject" */
                    szSubject, 
                    MAXWORD))
    {
        PRINTLOG ("Error: unable to set Subject field in note.\n");
    }
    return (error);
}

/************************************************************************

    FUNCTION:   AppendUpdatedByItem

*************************************************************************/

STATUS LNPUBLIC AppendUpdatedByItem (NOTEHANDLE hNote, char * szAuthor)
{
    STATUS  error;

    if (error = NSFItemAppend ( hNote, 
                                ITEM_SUMMARY | ITEM_NAMES,
                                DISCUSS_ITEM_UPDATEDBY,  /* "$UpdatedBy" */
                                (WORD) strlen(DISCUSS_ITEM_UPDATEDBY),
                                TYPE_TEXT,
                                szAuthor,
                                strlen(szAuthor)))
    {
        PRINTLOG ("Error: unable to append $UpdatedBy field.\n");
    }
    return (error);
}

/************************************************************************

    FUNCTION:   AppendRefItem
 
*************************************************************************/

STATUS  LNPUBLIC  AppendRefItem( NOTEHANDLE hRespNote, OID oidMainNote)
{
    STATUS      error = NOERROR;
    DWORD       dwValueLen;
    char       *buf;
    LIST       ListHdr;
    UNID       NoteUNID;

    dwValueLen = sizeof(LIST) + sizeof(UNID) ;

    buf =  (char *)malloc(sizeof(LIST) + sizeof(UNID));

    if (buf == NULL)
    {
        PRINTLOG ("Error: unable to allocate %lu bytes memory.\n", dwValueLen);
        return(ERR_AUTH_FLD_MALLOC);
    }
 
   /* Initialize the LIST header part of the response reference list */
    ListHdr.ListEntries = (USHORT) 1;
 
    /* Initialize the UNID part of the response reference list */
    NoteUNID.File = oidMainNote.File; /* user-unique identifier */
    NoteUNID.Note = oidMainNote.Note; /* time/date when the note was created */
 
    /* Pack the LIST and the UNID members of the Noteref list into
       a memory block.
    */
    memcpy(buf, (char*)&ListHdr, sizeof(LIST));
    memcpy((buf+sizeof(LIST)), (char*)&NoteUNID, sizeof(UNID));
 
    if (error = NSFItemAppend ( hRespNote, 
                                ITEM_SUMMARY,
                                FIELD_LINK,             /* "$REF" */
                                (WORD) strlen(FIELD_LINK),     
                                TYPE_NOTEREF_LIST,      /* $REF data type */
                                buf,
                                dwValueLen))
    {
        PRINTLOG ("Error: unable to append Note Reference List to Response.\n");
    }
    free (buf);
    return (error);
}

/************************************************************************

    FUNCTION:   AppendOriginalSubjectItem
 
*************************************************************************/

STATUS  LNPUBLIC  AppendOriginalSubjectItem (NOTEHANDLE hNote, 
                                        char * szMainSubject)
{
    STATUS  error;

    if (error = NSFItemSetText ( hNote, 
                    DISCUSS_ITEM_ORIGINALSUBJECT,   /* "OriginalSubject" */
                    szMainSubject, 
                    MAXWORD))
    {
        PRINTLOG ("Error: unable to set OriginalSubject field in note.\n");
    }
    return (error);
}

 
/************************************************************************
 
    FUNCTION:   ProcessArgs
 
    INPUTS:     argc, argv - directly from the command line
 
    OUTPUTS:    ServerName, DbPathName,
                MainFileName, RespFileName - data from the command line or
                from what the user types when prompted.
 
*************************************************************************/
 
void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                               char *ServerName, char *DbPathName,
                               char *MainFileName, char *RespFileName)
{
    if (argc != 5)
    {
        printf("Enter name of HCL Domino Server where DB resides: ");
        fflush(stdout);
        gets(ServerName);

        printf("Enter name of database: ");
        fflush(stdout);
        gets(DbPathName);
 
        printf("Enter name of an ASCII file containing Main Doc body: ");
        fflush(stdout);
        gets(MainFileName);
 
        printf("Enter name of an ASCII file containing Reponse body: ");
        fflush(stdout);
        gets(RespFileName);
 
    }
    else
    {
         strcpy(ServerName, argv[1]);
         strcpy(DbPathName, argv[2]);
         strcpy(MainFileName, argv[3]);
         strcpy(RespFileName, argv[4]);
 
    } /* end if */
} /* ProcessArgs */
