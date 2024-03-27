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

    PROGRAM:    nsf_dump

    FILE:       nsf_dump.c

    SYNTAX: nsf_dump  <database name>  [options]

      options =
           none    - dump everything, non-data
                     notes first, then data notes.
           -d      - dump just data notes
           -n      - dump just non-data notes
           -b      - dump binary data (like bitmaps)
           -f      - with fourth arg, a formula, dump
                     just data notes that match
                     the specified formula.
           -c      - file is a CD file, not NSF
           -s      - file is a CD storage file (not NSF)
           -o      - <file name> Dump output to file name.
           -p      - dump profile information, (Environment variable,
                     PROFILE, must be included in your makefile)
           -e      - dump deletion stubs only
           -m      - leave TYPE_MIME_PART and TYPE_RCF822_TEXT in native format
           -l      - dump folder object items

   DESCRIPTION:
   Print out the entire contents of a Domino database (NSF file) or a
   Compound Text (CD) file to the standard output using HCL C API for
   Domino and Notes functions.  This shows how to enumerate the contents of a
   Domino database including database header data, note header data, all
   notes, and all items. NSF_DUMP is a valuable tool for finding out
   how Domino and Notes stores different kinds of data in Domino databases.

   Note: #ifdef APITESTMODE is for internal use only.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* C include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(UNIX) || defined(MAC)
    #include <sys/types.h>
#endif

/* HCL C API for Notes/Domino include files */

#ifdef OS390
#include <_Ascii_a.h>    /* NOTE: must be the LAST file included */
#endif

#include <global.h>
#include <intl.h>
#include <nsf.h>
#include <acl.h>
#include <misc.h>
#include <nsfnote.h>
#include <osmem.h>
#include <ostime.h>
#include <stdnames.h>
#include <textlist.h>
#include <ods.h>
#include <editods.h>
#include <oserr.h>
#include <odserr.h>
#include <oleods.h>
#include <idtable.h>
#include <osmisc.h>
#if defined(CAPI_TESTING) 
#include "printlog.h" 
#else
 #define PRINTLOG printf 
 #define PRINTERROR(api_error, api_name) {\
 char    szErrorText[256] = { 0 };\
 OSLoadString(NULLHANDLE, ERR(api_error), szErrorText, sizeof(szErrorText));\
 fprintf(stderr, "[ERROR]:%s:%d:%s - %s", __FILE__,__LINE__,api_name,szErrorText); }
#endif 


/* NSF_DUMP - specific include file */
#include "nsf_dump.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/* Global Variables */
DBHANDLE    hDB;                /* handle to the database being dumped */
NOTEHANDLE  hNote;              /* handle to currently open note */
FILE       *dumpfile;           /* output file */
DWORD       dwItem;
DWORD        OFlags = 0;          /* Flags for NSFNoteOpen */

DWORD       dwCDRecord;
DWORD           dwActionRecord;
DWORD           dwNavigateRecord;
DWORD       nl, nw, nc;
BOOL        ACLDigestItemFlag = FALSE;

/* static globals (this file only) */
static  BOOL  bDumpData     = TRUE;
static  BOOL  bDumpNonData  = TRUE;
static  BOOL  bDumpCDFile   = FALSE;
static  BOOL  bDumpCDStorage = FALSE;
static  BOOL  bDumpBinary   = FALSE;
static  BOOL  bDumpDelStubs = FALSE;

#if defined (PROFILE)
static  BOOL  bDumpProfile  = FALSE;
#endif

#ifdef APITESTMODE
static  BOOL  bDumpNoTitle  = FALSE;
#endif

char szNOTE_CLASS_DOCUMENT[]    = "Document";
char szNOTE_CLASS_INFO[]        = "Help-About";
char szNOTE_CLASS_FORM[]        = "Form";
char szNOTE_CLASS_VIEW[]        = "View";
char szNOTE_CLASS_ICON[]        = "Icon";
char szNOTE_CLASS_DESIGN[]      = "Design Collection";
char szNOTE_CLASS_ACL[]         = "ACL";
char szNOTE_CLASS_HELP_INDEX[]  = "Help index";
char szNOTE_CLASS_HELP[]        = "Help-Using";
char szNOTE_CLASS_FILTER[]      = "Macro";
char szNOTE_CLASS_FIELD[]       = "Shared Field";
char szNOTE_CLASS_REPLFORMULA[] = "Replication Formula";
char szNOTE_CLASS_PRIVATE[]     = "Private Note";

static  struct
{
    WORD    fNoteClass;                 /* NOTE_CLASS value */
    char   *szNoteClass;                /* descriptive string */
    DWORD   dwDumpCount;                /* how many of that class dumped */
}   NoteClassTable[NUM_NOTE_CLASSES] =
    {
      NOTE_CLASS_DOCUMENT,                 szNOTE_CLASS_DOCUMENT,     0,
      NOTE_CLASS_INFO,                     szNOTE_CLASS_INFO,         0,
      NOTE_CLASS_FORM,                     szNOTE_CLASS_FORM,         0,
      NOTE_CLASS_VIEW,                     szNOTE_CLASS_VIEW,         0,
      NOTE_CLASS_ICON,                     szNOTE_CLASS_ICON,         0,
      NOTE_CLASS_DESIGN,                   szNOTE_CLASS_DESIGN,       0,
      NOTE_CLASS_ACL,                      szNOTE_CLASS_ACL,          0,
      NOTE_CLASS_HELP_INDEX,               szNOTE_CLASS_HELP_INDEX,   0,
      NOTE_CLASS_HELP,                     szNOTE_CLASS_HELP,         0,
      NOTE_CLASS_FILTER,                   szNOTE_CLASS_FILTER,       0,
      NOTE_CLASS_FIELD,                    szNOTE_CLASS_FIELD,        0,
      NOTE_CLASS_REPLFORMULA,              szNOTE_CLASS_REPLFORMULA,  0,
      NOTE_CLASS_PRIVATE,                  szNOTE_CLASS_PRIVATE,      0
    };

static  DWORD   dwUnknownClassCount = 0;

int main(int argc, char *argv[])
{
    STATUS      Error =  NOERROR;
    char        szFilename[128];
    FORMULAHANDLE  hCompForm;
    TIMEDATE    tdSearchSince;
    DWORD           DelStubsCt;

	  if (Error = NotesInitExtended (argc, argv))
	 {
     PRINTLOG("\n Unable to initialize Notes.\n");
     return (1);
	 }

    PRINTLOG( "NSF_DUMP Utility\n" );

    dumpfile = stdout;

#ifdef OS390
	/* Need to convert args from EBCDIC (Native z/OS) to ASCII (how this sample is compiled) */
	__argvtoascii_a(argc,argv);
#endif
    Error = ProcessArgs( argc, argv, &hCompForm );
    if (Error == NOERROR)
      strcpy( szFilename, argv[1]);

    /* ProcessArgs checks the syntax and sets globals bDumpNonData, etc. */
    if (Error)
    {
     NotesTerm(); /* ProcessArgs already printed an error message */
     return (0);
    }

    fprintf( dumpfile, "nsf_dump: dumping '%s'\n", szFilename );

    /* If -c specified, dump CD file (not NSF file) and return. */
    if (bDumpCDFile)
    {
     DumpCDFile( szFilename );
     NotesTerm();
     return (0);
    }

    /* If -s specified, dump CD storage file (not NSF file) and return. */
    if (bDumpCDStorage)
    {
     DumpCDStorageFile( szFilename );
     NotesTerm();
     return (0);
    }

    if (Error = NSFDbOpen( szFilename, &hDB ))
    {
      fprintf( stderr, "Error: unable to open '%s.'\n", szFilename );
	  PRINTERROR (Error,"NSFDbOpen");
      NotesTerm();
      return (1);
    }

    if (Error = DumpDBHeaderData( hDB, szFilename ))
    {
      NSFDbClose( hDB );
      PRINTERROR (Error,"DumpDBHeaderData");
      NotesTerm();
      return (1);
    }

    tdSearchSince.Innards[1] = ANYDAY;
    tdSearchSince.Innards[0] = ALLDAY;

    if (bDumpNonData)
    {
      if (Error = NSFSearch( hDB, NULLHANDLE, NULL,
                             SEARCH_SUMMARY,
                             NOTE_CLASS_ALLNONDATA,
                             &tdSearchSince,
                             DumpOneNote,
                             NULL, NULL ))
      {
       fprintf( stderr,"Error encountered searching for design notes.\n" );
       PRINTERROR (Error,"NSFSearch");
       NotesTerm();
       return (1);
      }
    }

    if (bDumpData)
    {
      if (Error = NSFSearch( hDB, hCompForm, NULL, 0,
                             NOTE_CLASS_DOCUMENT,
                             &tdSearchSince,
                             DumpOneNote,
                             NULL, NULL ))
      {
       fprintf( stderr,"Error encountered searching for data notes.\n" );
       PRINTERROR (Error,"NSFSearch");
       NotesTerm();
       return (1);
      }
    }

    if (bDumpDelStubs)
    {
      Error = DumpDelStub( hDB, &DelStubsCt );
      if ((Error == ERR_NO_MODIFIED_NOTES) || (DelStubsCt == 0))
        fprintf( stderr, "\nThis database does not contain any deletion stubs.\n");
      else if (Error != NOERROR)
      {
        fprintf( stderr, "Error encountered searching for deleted notes.\n");
        NSFDbClose(hDB);
        PRINTERROR (Error,"DumpDelStub");
        NotesTerm();
        return (1);
      }
    }

#if defined (PROFILE)
    if (bDumpProfile)
    {

    /*
     * Enumerate all Profile Documents for this Database.
     * Action Routine DumpProfile() will be called for each Profile
     * document found.
     */
      if (Error = NSFProfileEnum( hDB, NULL, 0, DumpProfile, NULL, 0 ))
      {
        fprintf( stderr, "Error encountered searching for profile information.\n");
        NSFDbClose(hDB);
        PRINTERROR (Error,"NSFProfileEnum");
        NotesTerm();
        return (1);
      }
    }
#endif
    NSFDbClose( hDB );

    fflush(dumpfile);

    PRINTLOG( "\n\nNSF_DUMP: Done.\n" );
    if ( stdout != dumpfile )
    fclose(dumpfile);
    if (Error == NOERROR)
      PRINTLOG("\nProgram completed successfully.\n");
      NotesTerm();
      return (0);
}

/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:     argc, argv - directly from the command line

    OUTPUTS:    *phCompForm - gets handle to compiled formula

*************************************************************************/

STATUS  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                               FORMULAHANDLE * phCompForm)
{
    char    myFormula[MAX_FORMULA_LEN+1];
    int     iArgCount;

    *phCompForm = NULLHANDLE;
    strcpy( myFormula, DEFAULT_FORMULA );

    if (argc == 1)
    {
      PrintUsage();
      return (PKG_ADDIN);
    }

    /* if options, read and intepret the arg list */
    if (argc > 2)  /* simple case: dump database */
    {
      for (iArgCount = 2; iArgCount < argc; iArgCount++)
      {
        if ((argv[iArgCount][0] == '-') || (argv[iArgCount][0] == '/'))
        {
          if ((argv[iArgCount][1] == 'd') || (argv[iArgCount][1] == 'D'))
            bDumpNonData = FALSE;   /* data notes only - no stats*/

          else if ((argv[iArgCount][1] == 'n') || (argv[iArgCount][1] == 'N'))
            bDumpData = FALSE;       /* non-data (design) notes only */

          else if ((argv[iArgCount][1] == 'c') || (argv[iArgCount][1] == 'C'))
          {
            bDumpCDFile  = TRUE;
            bDumpNonData = FALSE;
            bDumpData    = FALSE;
          }

          else if ((argv[iArgCount][1] == 's') || (argv[iArgCount][1] == 'S'))
          {
            /* Request to dump a CD storage file */
            bDumpCDStorage  = TRUE;
            bDumpNonData = FALSE;
            bDumpData    = FALSE;
          }

          else if ((argv[iArgCount][1] == 'b') || (argv[iArgCount][1] == 'B'))
            bDumpBinary = TRUE;       /* Include dump of any binary data in output */

#ifdef APITESTMODE
          else if ((argv[iArgCount][1] == 't') || (argv[iArgCount][1] == 'T'))
            bDumpNoTitle = TRUE;
#endif

          else if ((argv[iArgCount][1] == 'f') || (argv[iArgCount][1] == 'F'))
          {
            /* -f option specified: next arg is selection formula */

            /* make sure formula is supplied */
            if (argc < iArgCount + 2)
            {
              fprintf( stderr,
                       "Error: Must supply a formula.\n");
              return (PKG_ADDIN);
            }

            /* make sure formula will fit in myFormula[] array */
            if (strlen( argv[iArgCount+1] ) > MAX_FORMULA_LEN)
            {
              fprintf( stderr,
                       "Error: formula must not exceed %d characters.\n",
                       MAX_FORMULA_LEN);
              return (PKG_ADDIN);
             }                /* dump only data notes that match formula */
               strcpy (myFormula, argv[iArgCount+1]);
               bDumpNonData = FALSE;
               iArgCount++;
          }

          else if ((argv[iArgCount][1] == 'o') || (argv[iArgCount][1] == 'O'))
          {
            /* -o option specified: next arg is filename */

            /* make sure file name is supplied */
            if (argc < iArgCount + 2)
            {
              fprintf( stderr,
                       "Error: Must supply a file name.\n");
              return (PKG_ADDIN);
            }

            /* open the file and assign to dump file pointer */
            if ((dumpfile = fopen(argv[iArgCount+1],"w+")) == NULL)
            {
              fprintf( stderr,
                       "Error: dump output file could not be opened.\n");
              return (PKG_ADDIN);
            }                /* dump only data notes that match formula */
            iArgCount++;
          }
          else if ((argv[iArgCount][1] == 'e') || (argv[iArgCount][1] == 'E'))
          {
            /* -e option specified, dump deletion stubs only */
            bDumpDelStubs = TRUE;
            bDumpNonData = FALSE;
            bDumpData = FALSE;
          }

#if defined (PROFILE)
          else if ((argv[iArgCount][1] == 'p') || (argv[iArgCount][1] == 'P'))
          {
            /* -p option dump profile information only */
            bDumpProfile = TRUE;
            bDumpNonData = FALSE;
            bDumpData = FALSE;
          }
#endif
          else if ((argv[iArgCount][1] == 'm') || (argv[iArgCount][1] == 'M'))
          {
            /* -m option specified, leave TYPE_MIME_PART and TYPE_RCF822_TEXT in native format */
            OFlags |= OPEN_RAW_MIME;
          }
          else if ((argv[iArgCount][1] == 'l') || (argv[iArgCount][1] == 'L'))
            /* -l option specified, open the note with the OPEN_WITH_FOLDERS flag */
            OFlags |= OPEN_WITH_FOLDERS;

          else /* syntax error */
          {
            PrintUsage();
            return (PKG_ADDIN);
          }
        }
        else /* syntax error */
        {
          PrintUsage();
          return (PKG_ADDIN);
        }
      }
    }
    return( ProcessFormula( myFormula, phCompForm ) );
}

/************************************************************************

    FUNCTION:   PrintUsage

*************************************************************************/
void    LNPUBLIC  PrintUsage()
{
    fprintf( stderr, "\nUSAGE : nsf_dump <database name> [option]\n" );
    fprintf( stderr, "\noption : -d Dump data Notes ONLY.\n");
    fprintf( stderr, "         -n Dump non-data Notes ONLY.\n");
    fprintf( stderr, "         -b Include binary data in output.\n");
    fprintf( stderr,
    "         -f <formula> Selects which data Notes to dump.\n");
    fprintf( stderr, "         -c File is a CD file (not NSF).\n");
    fprintf( stderr, "         -s File is a CD storage file (not NSF).\n");
    fprintf( stderr, "         -o <file name> Dump output to file name.\n");
    fprintf( stderr, "         -e Dump deletion stubs only.\n");
#if defined (PROFILE)
    fprintf( stderr, "         -p Dump profile information ONLY. \n");
#endif
    fprintf( stderr,
     "         -m Leave TYPE_MIME_PART and TYPE_RCF822_TEXT in native format.\n");
    fprintf( stderr,
     "         -l Dump folder object items.\n");

    return;
}

/************************************************************************

    FUNCTION:   DumpDBHeaderData

*************************************************************************/
STATUS  LNPUBLIC  DumpDBHeaderData( DBHANDLE hDB, char * szFilename )
{
    STATUS      Error;
    USHORT      usMode;                 /* DB_DIRECTORY or DB_LOADED */
    DBREPLICAINFO    dbrepInfo;         /* replication info for DB */
    DBID        dbid;
    char        buffer[NSF_INFO_SIZE];
    char        szDBInfo[NSF_INFO_SIZE];
    WORD        wRepFlags;
    WORD        wPriority;
    char        szTimedate[MAXALPHATIMEDATE+1];
    WORD        retlevel;


    if (Error = NSFDbModeGet (hDB, &usMode))
    {
      fprintf (stderr, "Error: unable to get database mode.\n");
      return (ERR(Error));
    }
    if (usMode & DB_LOADED) /* hDB refers to a normal database file */
    {
      ;              /* this is what we expect: no output needed */
    }
    else if (usMode & DB_DIRECTORY) /* hDB refers to a directory  */
    {
      fprintf (stderr, "Error: '%s' is a directory, not a file.\n",
               szFilename);
      return (ERR(Error));
    }

    /* Now we know it's a Notes database file, get the other header data. */
    if (Error = NSFDbInfoGet( hDB, buffer ))
    {
      fprintf (stderr, "Error: unable to get title of %s\n.", szFilename );
      return (ERR(Error));
    }
    NSFDbInfoParse( buffer, INFOPARSE_TITLE, szDBInfo, NSF_INFO_SIZE-1 );

#ifdef APITESTMODE
    if (!bDumpNoTitle)
      fprintf( dumpfile, "Database Title        = '%s'\n", szDBInfo );
#else
      fprintf( dumpfile, "Database Title        = '%s'\n", szDBInfo );
#endif

    NSFDbInfoParse( buffer, INFOPARSE_CATEGORIES, szDBInfo, NSF_INFO_SIZE-1 );
    fprintf( dumpfile, "Database Categories   = '%s'\n", szDBInfo );

    NSFDbInfoParse( buffer, INFOPARSE_CLASS, szDBInfo, NSF_INFO_SIZE-1 );
    fprintf( dumpfile, "Database Class        = '%s'\n", szDBInfo );

    NSFDbInfoParse( buffer, INFOPARSE_DESIGN_CLASS, szDBInfo,
                    NSF_INFO_SIZE-1 );
    fprintf( dumpfile, "Database Design Class = '%s'\n", szDBInfo );

    NSFDbIDGet( hDB, &dbid );

#ifndef APITESTMODE
    fprintf( dumpfile, "Database ID           = %#010lx - %#010lx\n",
             dbid.Innards[1],
             dbid.Innards[0]);
#endif

    if (Error = NSFDbReplicaInfoGet( hDB, &dbrepInfo ))
    {
      fprintf( stderr, "Error: unable to get replica info for %s\n.",
               szFilename );
      return (ERR(Error));
    }

#ifndef APITESTMODE
    fprintf( dumpfile, "Database Replica ID   = %#010lx - %#010lx\n",
             dbrepInfo.ID.Innards[1],
         dbrepInfo.ID.Innards[0] );
#endif

    wRepFlags = dbrepInfo.Flags & REPLFLG_PRIORITY_INVMASK;
    wPriority = REPL_GET_PRIORITY(dbrepInfo.Flags);

    fprintf( dumpfile, "Database Flags        = " );

    if (wRepFlags & REPLFLG_DISABLE)
      fprintf(dumpfile, "DISABLE ");
    if (wRepFlags & REPLFLG_IGNORE_DELETES)
      fprintf(dumpfile, "IGNORE_DELETES ");
    if (wRepFlags & REPLFLG_HIDDEN_DESIGN)
      fprintf(dumpfile, "HIDDEN_DESIGN ");
    if (wRepFlags & REPLFLG_DO_NOT_CATALOG)
      fprintf(dumpfile, "DO_NOT_CATALOG ");
    if (wRepFlags & REPLFLG_CUTOFF_DELETE)
      fprintf(dumpfile, "CUTOFF_DELETE ");
    if (wRepFlags & REPLFLG_NEVER_REPLICATE)
      fprintf(dumpfile, "NEVER_REPLICATE ");
    if (wRepFlags & REPLFLG_ABSTRACT)
      fprintf(dumpfile, "ABSTRACT ");
    if (wRepFlags & REPLFLG_DO_NOT_BROWSE)
      fprintf(dumpfile, "DO_NOT_BROWSE ");
    if (wRepFlags & REPLFLG_NO_CHRONOS)
      fprintf(dumpfile, "NO_CHRONOS ");

    fprintf( dumpfile, "\nDatabase Rep Priority = %s\n",
    wPriority == 0 ? "Low"     :
                                (wPriority == 1 ? "Medium"  :
                                (wPriority == 2 ? "High"    : "Unknown")));

    fprintf( dumpfile, "DB Rep Cutoff Interval= %d Days\n",
             dbrepInfo.CutoffInterval );
    GetTDString( &(dbrepInfo.Cutoff), szTimedate );

#ifndef APITESTMODE /* only print datetime if we are not in test mode */
    fprintf( dumpfile, "DB Rep Cutoff Date    = %s\n", szTimedate );
#endif


    if(Error = NSFGetMaxPasswordAccess( hDB, &retlevel))
    {
      fprintf( stderr, "Error: unable to get maximum internet name and password access %s\n.",
               szFilename );
      return (ERR(Error));
    }
      fprintf(dumpfile, "Maximum Internet name and password access is:");
      switch (retlevel)
      {
        case ACL_LEVEL_MANAGER:
          fprintf(dumpfile, " (Manager)\n");
          break;
        case ACL_LEVEL_DESIGNER:
          fprintf(dumpfile, "  (Designer)\n");
          break;
        case ACL_LEVEL_EDITOR:
          fprintf(dumpfile, "  (Editor)\n");
          break;
        case ACL_LEVEL_AUTHOR:
          fprintf(dumpfile, "  (Author)\n");
          break;
        case ACL_LEVEL_READER:
          fprintf(dumpfile, "  (Reader)\n");
          break;
        case ACL_LEVEL_DEPOSITOR:
          fprintf(dumpfile, "  (Depositor)\n");
          break;
        case ACL_LEVEL_NOACCESS:
          fprintf(dumpfile, "  (No Access)\n");
          break;
        default:
          fprintf(dumpfile, "  (Not Set)\n");
          break;
      }

    return NOERROR;
}

/************************************************************************

    FUNCTION:   ProcessFormula

*************************************************************************/
STATUS  LNPUBLIC  ProcessFormula( char *myFormula,
                                  FORMULAHANDLE  *phCompForm )
{
    FORMULAHANDLE   hCompForm;
    WORD            myFormLen;
    STATUS          Error;
    WORD            compFormLen;
    STATUS          compErrCode;
    WORD            errLine, errCol, errOffset, errLength;

    myFormLen = strlen( myFormula );
    Error = NSFFormulaCompile( NULL, 0, myFormula, myFormLen,
                               &hCompForm, &compFormLen,
                               &compErrCode,
                               &errLine, &errCol, &errOffset, &errLength );

    if (Error == NOERROR)
    {
      *phCompForm = hCompForm;
      return (NOERROR);
    }

    if (Error == ERR_FORMULA_COMPILATION)
    {
      fprintf( stderr,
               "Error: Formula compilation error 0x%04x detected.\n",
               compErrCode );
      fprintf( stderr, "     Line: %d\n", errLine );
      fprintf( stderr, "      Col: %d\n", errCol );
      fprintf( stderr, "   Length: %d\n", errLength );
      myFormula[errOffset+errLength] = '0';
      fprintf( stderr, "    Token: '%s'\n", &myFormula[errOffset] );
    }
    else
      fprintf (stderr, "Error: unable to compile formula '%s'.\n",
               myFormula );

    *phCompForm = NULLHANDLE;
    return (Error);
}

/************************************************************************

    FUNCTION:   DumpOneNote

    PURPOSE:    Print the contents of one note -- of any class -- to
                the dumpfile.

    DESCRIPTION: This is the action routine specified to NSFSearch.

    INPUTS:
      Param         - not used
      pSearchMatch  - information about the note found
      SummaryBuffer - not used

*************************************************************************/

STATUS  LNPUBLIC  DumpOneNote( void far *Param,
                 SEARCH_MATCH far *pSearchMatch,
                 ITEM_TABLE far *SummaryBuffer )
{
    STATUS      Error;
    SEARCH_MATCH SearchMatch;
    char        szClass[LINELENGTH+1];
    DWORD       dwDumpCount;

    memcpy( (char*)&SearchMatch, (char*)pSearchMatch, sizeof(SEARCH_MATCH) );

    if (!(SearchMatch.SERetFlags & SE_FMATCH))
    return (NOERROR);

    if (Error = GetNoteClassAndDumpCount( hDB,
                                          SearchMatch.ID.NoteID,
                                          szClass,
                                          LINELENGTH,
                                          &dwDumpCount ))
      return (Error);

    if (Error = NSFNoteOpenExt( hDB, SearchMatch.ID.NoteID, OFlags, &hNote ))
    {
      fprintf( stderr, "Error: unable to open %s note.\n", szClass );
      return (Error);
    }

    fprintf( dumpfile, "\n%s Note : %3ld\n", szClass, dwDumpCount );

    if (Error = DumpNoteHeaderData( hNote ))
    {
      NSFNoteClose( hNote );
      return( Error );
    }

    dwItem = 0;
    NSFItemScan( hNote, DumpOneItem, NULL );

    Error = NSFNoteClose( hNote );
    if (Error != NOERROR)
      fprintf( stderr, "Error: unable to close note.\n" );
    return(Error);
}

/************************************************************************

    FUNCTION:   DumpDelStub

    PURPOSE:    Print the remaining contents of a deletion stub.

*************************************************************************/

STATUS  LNPUBLIC  DumpDelStub( DBHANDLE hDB, DWORD *num_deletions )
{
    STATUS      Error;
    char        szClass[LINELENGTH+1];
    DWORD       dwRetCount, notes_scanned;
    TIMEDATE        begin_td, end_td;
    DHANDLE           idtable_handle;
    NOTEID          note_id, delNote;
    OID         oidNote;
    TIMEDATE    tdModified;
    WORD        wClass;
    WORD        i;
    char        szTimedate[MAXALPHATIMEDATE+1];



    /* Get the beggining of time, which will be from conception of this database. */
    TimeConstant (TIMEDATE_WILDCARD, &begin_td);

    /* Get the end time of current time and date */
    OSCurrentTIMEDATE(&end_td);

    dwRetCount = *num_deletions = notes_scanned = 0L;

    if (Error = NSFDbGetModifiedNoteTable( hDB, NOTE_CLASS_ALL, begin_td,
                                           &end_td, &idtable_handle))
      return(Error); /* Possibly no deletion stubs in this database */

    while(IDScan(idtable_handle, (FLAG)(notes_scanned++==0L), &note_id))
    {
      if (RRV_DELETED & note_id)
      {
        *num_deletions += 1;
        /*
         * Remove the RRV_DELETED bit before calling NSFDbGetNoteInfo() otherwise
         * we will get the error ERR_INVALID_NOTE.
         */
        delNote = note_id ^ RRV_DELETED;
        Error = NSFDbGetNoteInfo( hDB, delNote, &oidNote, &tdModified, &wClass );
        if( Error == ERR_NOTE_DELETED ) /* We know this is a deletion stub */
        {
          Error = NOERROR;
          for (i=0;i<NUM_NOTE_CLASSES;i++)
          {
            if (wClass & NoteClassTable[i].fNoteClass)
            {
              strncpy( szClass, NoteClassTable[i].szNoteClass, LINELENGTH );
              dwRetCount = ++(NoteClassTable[i].dwDumpCount);
              break;
            }
          }
          if (i==NUM_NOTE_CLASSES)
          {
            strncpy( szClass, "Unrecognized Class", LINELENGTH );
            dwRetCount = ++dwUnknownClassCount;
          }
          fprintf( dumpfile, "\n%s Note, Deletion Stub : %3ld\n", szClass, dwRetCount );
          fprintf( dumpfile, "  Header Data: \n" );
          fprintf( dumpfile, "\tDBID             = %#010lx - %#010lx\n", oidNote.File.Innards[1], oidNote.File.Innards[0] );
          fprintf( dumpfile, "\tNote ID          = %#010lx\n", delNote );
          PrintOid( &oidNote );
          PrintNoteClass( wClass );
          GetTDString( &tdModified, szTimedate );
          fprintf( dumpfile, "\tModified Time    = %s\n", szTimedate );
        }
        else
        {
          fprintf ( dumpfile, "Error: unable to get info re: note %#010lx\n",
                    delNote );
        }
      }
    }
    IDDestroyTable(idtable_handle);
    return (Error);
}
/************************************************************************

    FUNCTION:   DumpProfile

    PURPOSE:

    DESCRIPTION: This is the action routine specified to NSFProfileEnum

*************************************************************************/
#if defined (PROFILE)
STATUS  LNPUBLIC  DumpProfile( DBHANDLE hDB, void far *Ctx, char *ProfileName,
                               WORD ProfileNameLength, char *UserName,
                               WORD UserNameLength,  NOTEID ProfileNoteID )
{
    STATUS Error;
    NOTEHANDLE hNote;
    char        szClass[LINELENGTH+1];
    DWORD       dwDumpCount;

    if (Error = GetNoteClassAndDumpCount( hDB,
                                          ProfileNoteID,
                                          szClass,
                                          LINELENGTH,
                                          &dwDumpCount ))
      return (Error);

    if (Error = NSFNoteOpen( hDB, ProfileNoteID, 0, &hNote ))
    {
      fprintf( stderr, "Error: unable to open %s note.\n", szClass );
      return (Error);
    }

    fprintf( dumpfile, "\n%s Note : %3ld\n", szClass, dwDumpCount );

    if (Error = DumpNoteHeaderData( hNote ))
    {
      NSFNoteClose( hNote );
      return( Error );
    }

    dwItem = 0;
    NSFItemScan( hNote, DumpOneItem, NULL );

    Error = NSFNoteClose( hNote );
    if (Error != NOERROR)
    fprintf( stderr, "Error: unable to close note.\n" );
    return(Error);
}
#endif

/************************************************************************

    FUNCTION:   GetNoteClassAndDumpCount

    PURPOSE:  Look up the note class, copy a descriptive string
              to the return string, increment the count of notes
              of that class dumped so far, and return that count.

*************************************************************************/
STATUS  LNPUBLIC  GetNoteClassAndDumpCount( DBHANDLE    hDB,
                                            NOTEID      nid,
                                            char *      szRetString,
                                            WORD        wRetStrLen,
                                            DWORD      *pdwRetCount )
{
    STATUS      Error;
    OID         oidNote;
    TIMEDATE    tdModified;
    WORD        wClass;
    WORD        i;

    if (Error = NSFDbGetNoteInfo( hDB, nid, &oidNote, &tdModified, &wClass ))
    {
      fprintf ( dumpfile, "Error: unable to get info re: note %#010lx\n",
                nid );
      return(Error);
    }

    for (i=0;i<NUM_NOTE_CLASSES;i++)
    {
      if (wClass & NoteClassTable[i].fNoteClass)
      {
        strncpy( szRetString, NoteClassTable[i].szNoteClass, wRetStrLen );
        *pdwRetCount = ++(NoteClassTable[i].dwDumpCount);
        break;
      }
    }
    if (i==NUM_NOTE_CLASSES)
    {
      strncpy( szRetString, "Unrecognized Class", wRetStrLen );
      *pdwRetCount = ++dwUnknownClassCount;
    }
    return NOERROR;
}


/************************************************************************

    FUNCTION:   DumpNoteHeaderData

*************************************************************************/

STATUS  LNPUBLIC  DumpNoteHeaderData( NOTEHANDLE hNote )
{
    DBHANDLE    hDB;
    DBID        dbid;
    NOTEID      noteid;
    OID         oid;
    WORD        wclass;
    TIMEDATE    tdModified;
    WORD        wPreV3Privileges;
    WORD        wFlags;
    TIMEDATE    tdLastAccessed;
    char        szTimedate[MAXALPHATIMEDATE+1];

    fprintf( dumpfile, "  Header Data: \n" );

    NSFNoteGetInfo(hNote, _NOTE_DB, &hDB);
    NSFNoteGetInfo(hNote, _NOTE_ID, &noteid);
    NSFNoteGetInfo(hNote, _NOTE_OID, &oid);
    NSFNoteGetInfo(hNote, _NOTE_CLASS, &wclass);
    NSFNoteGetInfo(hNote, _NOTE_MODIFIED, &tdModified);
    NSFNoteGetInfo(hNote, _NOTE_PRIVILEGES, &wPreV3Privileges);
    NSFNoteGetInfo(hNote, _NOTE_FLAGS, &wFlags);
    NSFNoteGetInfo(hNote, _NOTE_ACCESSED, &tdLastAccessed);

    NSFDbIDGet( hDB, &dbid );

#ifndef APITESTMODE
    fprintf( dumpfile, "\tDBID             = %#010lx - %#010lx\n",
             dbid.Innards[1],
             dbid.Innards[0] );

    fprintf( dumpfile, "\tNote ID          = %#010lx\n", noteid );
#endif

    PrintOid( &oid );
    PrintNoteClass( wclass );
    GetTDString( &tdModified, szTimedate );

#ifndef APITESTMODE
    fprintf( dumpfile, "\tModified Time    = %s\n", szTimedate );
#endif

    PrintNotePrivileges( wPreV3Privileges );
    PrintNoteFlags( wFlags );
    GetTDString( &tdLastAccessed, szTimedate );

#ifndef APITESTMODE
    fprintf( dumpfile, "\tLast Accessed    = %s\n", szTimedate );
#endif

    return( NOERROR );
}

/************************************************************************

    FUNCTION:   PrintOID

*************************************************************************/

void  LNPUBLIC   PrintOid( OID * poidNote )
{
    char        szTimedate[MAXALPHATIMEDATE+1];

    if (poidNote == (OID*)NULL)
    {
      fprintf( dumpfile, "\tUnable to obtain Note OID.\n");
      return;
    }

#ifndef APITESTMODE
    fprintf( dumpfile, "\tOID.File         = %#010lx - %#010lx\n",
             poidNote->File.Innards[1],
             poidNote->File.Innards[0] );
#endif

    GetTDString( &(poidNote->Note), szTimedate );

#ifndef APITESTMODE
    fprintf( dumpfile, "\tOID.Note         = %s\n", szTimedate );

    fprintf( dumpfile, "\tOID.Sequence     = %#lx\n", poidNote->Sequence );
#endif

    GetTDString( &(poidNote->SequenceTime), szTimedate );

#ifndef APITESTMODE
    fprintf( dumpfile, "\tOID.SequenceTime = %s\n", szTimedate );
#endif

    return;
}

/************************************************************************

   FUNCTION:   PrintNoteClass

*************************************************************************/

void LNPUBLIC PrintNoteClass( WORD wNoteClass )
{
    fprintf( dumpfile, "\tNote Class : " );
    if (wNoteClass &  NOTE_CLASS_DOCUMENT)
      fprintf( dumpfile, "NOTE_CLASS_DOCUMENT " );
    if (wNoteClass &  NOTE_CLASS_INFO)
      fprintf( dumpfile, "NOTE_CLASS_INFO " );
    if (wNoteClass &  NOTE_CLASS_FORM)
      fprintf( dumpfile, "NOTE_CLASS_FORM " );
    if (wNoteClass &  NOTE_CLASS_VIEW)
      fprintf( dumpfile, "NOTE_CLASS_VIEW " );
    if (wNoteClass &  NOTE_CLASS_ICON)
      fprintf( dumpfile, "NOTE_CLASS_ICON " );
    if (wNoteClass &  NOTE_CLASS_DESIGN)
      fprintf( dumpfile, "NOTE_CLASS_DESIGN " );
    if (wNoteClass &  NOTE_CLASS_ACL)
      fprintf( dumpfile, "NOTE_CLASS_ACL " );
    if (wNoteClass &  NOTE_CLASS_HELP_INDEX)
      fprintf( dumpfile, "NOTE_CLASS_HELP_INDEX " );
    if (wNoteClass &  NOTE_CLASS_HELP)
      fprintf( dumpfile, "NOTE_CLASS_HELP " );
    if (wNoteClass &  NOTE_CLASS_FILTER)
      fprintf( dumpfile, "NOTE_CLASS_FILTER " );
    if (wNoteClass &  NOTE_CLASS_FIELD)
      fprintf( dumpfile, "NOTE_CLASS_FIELD " );
    if (wNoteClass &  NOTE_CLASS_REPLFORMULA)
      fprintf( dumpfile, "NOTE_CLASS_REPLFORMULA " );
    if (wNoteClass &  NOTE_CLASS_DEFAULT)
      fprintf( dumpfile, "NOTE_CLASS_DEFAULT " );
    if (wNoteClass &  NOTE_CLASS_PRIVATE)
      fprintf( dumpfile, "NOTE_CLASS_PRIVATE " );
    fprintf( dumpfile, "\n" );
    return;
}

/************************************************************************

   FUNCTION:   PrintNotePrivileges

*************************************************************************/

void LNPUBLIC PrintNotePrivileges( WORD wPreV3Privileges )
{
    fprintf( dumpfile, "\tPre-V3 Privileges mask = %#x\n",
             (ACL_BITPRIVS & wPreV3Privileges) );
    return;
}

/************************************************************************

   FUNCTION:   PrintNoteFlags

*************************************************************************/

void LNPUBLIC PrintNoteFlags( WORD wNoteFlags )

{
    fprintf( dumpfile, "\tNote Flags : " );
    if (wNoteFlags &  NOTE_FLAG_READONLY)
      fprintf( dumpfile, "NOTE_FLAG_READONLY " );
    if (wNoteFlags &  NOTE_FLAG_ABSTRACTED)
      fprintf( dumpfile, "NOTE_FLAG_ABSTRACTED " );
    fprintf( dumpfile, "\n" );
    return;
}

/************************************************************************

    FUNCTION:   DumpOneItem

    PURPOSE:    Print one item from a note to the dumpfile.

    ALGORITHM:  This is the action routine specified to NSFItemScan.

*************************************************************************/

STATUS   LNPUBLIC   DumpOneItem( WORD       Spare,
                                 WORD       ItemFlags,
                                 char far * Name,
                                 WORD       NameLength,
                                 void far * Value,
                                 DWORD      ValueLength,
                                 void far * Parameter )
{
    DWORD   i;
    char  * pData;
    WORD    length;
    WORD    wDataType;
    char   *szDataType;
    char   *szClass;
    char   *szItemFlags;
    char   TestItemName[81];

    fprintf( dumpfile, "Item : %ld\n  Item Name   = '", ++dwItem );
    strcpy(TestItemName,"\0");

    for (i=0; i<NameLength; i++)
    {
      fprintf( dumpfile, "%c", Name[i] );
      TestItemName[i] = Name[i];
    }

    TestItemName[i]='\0';

    /* if this item is $ACLDigest set flag... */
    if (strcmp(TestItemName, "$ACLDigest")==0)
      ACLDigestItemFlag = TRUE;

    fprintf( dumpfile, "'\n" );

    pData = (char *) Value;
    length = (WORD) ValueLength; /* includes the 2 byte data type */

    memcpy( (char*)&wDataType, pData, sizeof(WORD) );
    pData += sizeof(WORD);
    length -= sizeof(WORD);

    szDataType = GetLiteralString( TypeStringTablePtr, wDataType );
    fprintf( dumpfile, "  Data Type   = %s\n", szDataType );

#ifndef APITESTMODE
    fprintf( dumpfile, "  Data Length = %ld\n", ValueLength );
#endif

    szClass = GetLiteralString( ClassStringTablePtr, (CLASS_MASK & wDataType) );
    fprintf( dumpfile, "  Type Class  = %s\n", szClass );

    GetFieldFlagsString( ItemFlags, &szItemFlags );
    fprintf( dumpfile, "  Field Flags = %s\n", szItemFlags );

    switch (wDataType)
    {
      case TYPE_ERROR:
        break;

      case TYPE_UNAVAILABLE:
        break;

      case TYPE_TEXT:
#ifndef APITESTMODE
        fprintf( dumpfile, "  Text data   = " );
        if (length)
          PrintText( pData, (DWORD)length );
        else
          fprintf (dumpfile, "\n");
#else
        if ((strcmp(TestItemName,"$UpdatedBy")==0) || (strcmp(TestItemName,"$MachineName")==0))
          fprintf( dumpfile, "  Text data   =\n" );
#endif
        break;

      case TYPE_TEXT_LIST:

#ifndef APITESTMODE
        DumpTextlistItem( pData, length );
#else
        if ((strcmp(TestItemName,"$UpdatedBy")==0) || (strcmp(TestItemName,"$MachineName")==0))
          break;
        else
          DumpTextlistItem( pData, length );
#endif
        break;

      case TYPE_NUMBER:
        DumpNumberItem( pData, length );
        break;

      case TYPE_NUMBER_RANGE:
        DumpNumberRangeItem( pData, length );
        break;

      case TYPE_TIME:
        DumpTimeItem( pData, length );
        break;

      case TYPE_TIME_RANGE:
        DumpTimeRangeItem( pData, length );
        break;

      case TYPE_FORMULA:
        PrintFormula( pData, length );
        break;

      case TYPE_USERID:
        DumpUseridItem( pData, length );
        break;

      case TYPE_INVALID_OR_UNKNOWN:
        break;

      case TYPE_COMPOSITE:
        DumpCompoundTextItem( Name, NameLength, pData, length );
        break;

      case TYPE_COLLATION:
        break;

      case TYPE_OBJECT:
        DumpObjectItem( Name, NameLength, pData, length );
        break;

      case TYPE_NOTEREF_LIST:
        break;

      case TYPE_VIEW_FORMAT:
        break;

      case TYPE_ICON:
        break;

      case TYPE_NOTELINK_LIST:
        DumpNotelinkListItem( pData, length );
        break;

      case TYPE_SIGNATURE:
        break;

      case TYPE_SEAL:
        break;

      case TYPE_SEALDATA:
        break;

      case TYPE_SEAL_LIST:
        break;

      case TYPE_HIGHLIGHTS:
        break;

      case TYPE_WORKSHEET_DATA:
        break;

      case TYPE_USERDATA:
        DumpUserDataItem( pData, length );
        break;

      /* New item types in Release 4.0 */
      case TYPE_QUERY:
        DumpAgentQueryItem ( Name, NameLength, pData, length );
        break;

      case TYPE_ACTION:
        DumpActionItem ( Name, NameLength, pData, length );
        break;

      case TYPE_ASSISTANT_INFO:
        DumpAgentInfoItem ( Name, NameLength, pData, length );
        break;

      case TYPE_VIEWMAP_DATASET:
        DumpViewmapDatasetItem ( Name, NameLength, pData, length );
        break;

      case TYPE_VIEWMAP_LAYOUT:
        DumpViewmapLayoutItem ( Name, NameLength, pData, length );
        break;

      case TYPE_LSOBJECT:
        DumpLotusScriptObjectItem ( Name, NameLength, pData, length );
        break;

      case TYPE_HTML:
        DumpHTMLItem ( Name, NameLength, pData, length );
        break;

     /* New item types in Release 5.0.7 */
      case TYPE_MIME_PART:
        DumpMimePart( pData, length );
        break;
      case TYPE_RFC822_TEXT:
        DumpRFC822Item( pData, length );
        break;

      default:
        fprintf( dumpfile, "  unknown data type = %x\n", wDataType );
        break;
    }
    return( NOERROR );
}


/************************************************************************

    FUNCTION:   PrintText

    PURPOSE:    Print a block of ascii text

    INPUTS:
      char   *pPtr;                   pointer to start of data
      DWORD   dwCharacters;           number of characters to print

*************************************************************************/

void    LNPUBLIC  PrintText( char far *pPtr, DWORD dwCharacters )
{
    DWORD   i;
    char    buf[LINELENGTH + 1];
    int             bufPos;

    bufPos = 0;
    for (i = 0; i < dwCharacters; i++)
    {
      if ((*pPtr == '\0') || (*pPtr == '\n')
           || (*pPtr == '\r') || (*pPtr == '\f'))
      {
        buf[bufPos++] = '\0';
        pPtr++;
        fprintf( dumpfile, "%s\n", buf);
        bufPos = 0;
      }
      else
        buf[bufPos++] = *pPtr++;

      if (bufPos >= LINELENGTH)
      {
        buf[LINELENGTH] = '\0';
        fprintf( dumpfile, "%s\n", buf);
        bufPos = 0;
      }
    }

    if (0 != bufPos)
    {
      buf[bufPos++] = '\0';
      fprintf( dumpfile, "%s\n", buf);
    }

    return;
}

/************************************************************************

    FUNCTION:   NSFDDUMPDumpMemory

    PURPOSE:    Print memory in HEX side-by-side with ASCII representation

    INPUTS:     char *   pData           pointer to start of memory to dump
                WORD     wSize           size in bytes of memory to dump

*************************************************************************/

void    LNPUBLIC  NSFDDUMPDumpMemory( char * pData, WORD wSize )
{
    WORD    wCount;             /* How many bytes have been printed */
    WORD    wDumped;            /* Where we are on this line */
    WORD    wOffset;            /* Offset to first byte */
    char    chASCII[17];
    char    *pLocal;

    if (!bDumpBinary)
    {
      fprintf (dumpfile, "  (Binary data not printed.  "
               "Use the -b switch to print binary data.)\n");
      return;
    }

    pLocal = pData;
    wCount = wDumped = wOffset = 0;
    if (wSize < 1)
    {
      fprintf(dumpfile, "Error: NSFDUMPDumpMemory: Size less than one (1)\n");
      return;
    }

    fprintf (dumpfile, "\n");   /* seperate output with blank lines */

    while (wCount < wSize)
    {
      wOffset = LOWORD(pLocal) & 0x000F;
      wDumped = 0;

#if defined (DOSW16)
      /* Print the address in segmented format */
      fprintf (dumpfile, "%04X:%04X  ", HIWORD(pLocal), LOWORD(pLocal));
#else
      /* Print the address in flat format */
      fprintf (dumpfile, "%08X  ", pLocal);
#endif

      /* Fill in any leading blanks */
      for ( ; wDumped < wOffset; wDumped++)
      /* Start with current value of wDumped */
      {
        fprintf(dumpfile, "   ");
        chASCII[wDumped] = ' ';
      }

      /* Print any data on this line */
      for ( ; (wDumped < 16) && (wCount < wSize); wDumped++)
      /* Start with current value of wDumped */
      {
        fprintf (dumpfile, "%02X", (BYTE) *pLocal);
        /* exclude non-printing ASCII */
        chASCII[wDumped] = (char) ((*pLocal < 0x20 || *pLocal > 0x7E)
                                                    ? 0x2E : *pLocal);

        if (7 == wDumped)
          fputc ('-', dumpfile);
        else
          fputc (' ', dumpfile);

        pLocal++;
        wCount++;
      }

      /* Fill in any trailing blanks */
      for ( ; wDumped < 16; wDumped++)
      {
        fprintf (dumpfile, "   ");
        chASCII[wDumped] = ' ';
      }

      chASCII[wDumped] = '\0';
      fprintf(dumpfile, "%s\n", chASCII);
    }

    fprintf (dumpfile, "\n");
    return;
}

/*************************************************************************

    FUNCTION:   WordCount

    PURPOSE:    Count characters, words, and lines in a text buffer.

**************************************************************************/

void    LNPUBLIC  WordCount( char * pText, WORD wTextLen )
{
    int     c;
    BOOL    inword;
    WORD    wPos=0;

    nw = 0;
    nc = 0;

    inword = FALSE;
    while(wPos < wTextLen)
    {
      c = pText[wPos++]; nc++;

      /* Line counting done in PrintText() - - it does Fill Justify
         to 80 columns.
       */

      if (c == ' ' || c == '\n' || c == '\t')
        inword = FALSE;
      else if(inword == FALSE)
      {
        if((c >= 'a' && c <= 'z') ||  /* lower case? */
           (c >= 'A' && c <= 'Z'))    /* upper case? */
        {
          inword = TRUE;
          nw++;
        }
      }
      else if((c >= 'a' && c <= 'z') ||   /* lower case? */
              (c >= 'A' && c <= 'Z') ||   /* upper case? */
              (c >= '0' && c <= '9') ||   /* or digit? */
               c == '\'')                 /* or apostrophe? */
        ; /* within a word, no counting! */
      else
        inword = FALSE;
    }
    return;
}


/*************************************************************************

    FUNCTION:   PrintUNID

    PURPOSE:    Write the UNID to the output file.

**************************************************************************/

void LNPUBLIC PrintUNID ( char *          pIndent,
                          UNID *          pUnid )
{
    char        szTimedate[MAXALPHATIMEDATE+1];

    fprintf( dumpfile, "%sUNID.File = %#010lx - %#010lx\n", pIndent,
                pUnid->File.Innards[1],
                pUnid->File.Innards[0] );

    GetTDString( &(pUnid->Note), szTimedate );

#ifndef APITESTMODE
    fprintf( dumpfile, "%sUNID.Note = %s\n", pIndent, szTimedate );
#endif

}

/*************************************************************************

    FUNCTION:   PrintNOTELINK

    PURPOSE:    Write the NOTELINK components to the output file.

**************************************************************************/

void LNPUBLIC PrintNOTELINK ( char *          pIndent,
                              NOTELINK *      pLink )
{
    char            IndentBuf [64];

    fprintf( dumpfile, "%sLink Replica ID = %#010lx - %#010lx\n", pIndent,
      pLink->File.Innards[1], pLink->File.Innards[0] );

    strcpy (IndentBuf, pIndent);
    strcat (IndentBuf, "Link ");
    PrintUNID( IndentBuf, &(pLink->View));
    PrintUNID( IndentBuf, &(pLink->Note));
}

/*************************************************************************

    FUNCTION:   PrintFormula

    PURPOSE:    Decompile the formula and write to output file.

**************************************************************************/

void LNPUBLIC PrintFormula ( char far *      pFormula,
                             WORD            formulaLen )
{
    STATUS          error;
    DHANDLE           hFormulaText;
    WORD            wFormulaTextLength;
    char far *      pFormulaText;

    fprintf (dumpfile, "  <Beginning of formula>\n");

    if( formulaLen == 0 )
      fprintf( dumpfile, "    Formula Length is 0.\n" );
    else if (error = NSFFormulaDecompile (pFormula, FALSE, &hFormulaText,
      &wFormulaTextLength ))
    {
      fprintf( dumpfile, "    Error: unable to decompile formula.\n" );
      fprintf( dumpfile, "    Error code = %#x.\n", error );
    }
    else
    {
      pFormulaText = (char*) OSLockObject( hFormulaText );
      PrintText( pFormulaText, wFormulaTextLength );
      OSUnlockObject( hFormulaText );
      OSMemFree( hFormulaText );
    }

    fprintf (dumpfile, "  <End of formula>\n");
}

/*************************************************************************

    FUNCTION:   PrintScriptSource

    PURPOSE:    Write text form of script to output file.

**************************************************************************/

void LNPUBLIC PrintScriptSource ( char far *      pScript,
                                  DWORD           scriptLen )
{
    fprintf (dumpfile, "  <Begin Lotus Script source>\n");

    PrintText (pScript, (WORD) scriptLen);

    fprintf (dumpfile, "  <End Lotus Script source>\n");
}

/*************************************************************************

    FUNCTION:   PrintScriptObject

    PURPOSE:    Dump Lotus Script object code.

**************************************************************************/

void LNPUBLIC PrintScriptObject ( char far *      pScript,
                                  DWORD           scriptLen)
{
    fprintf (dumpfile, "  <Begin Lotus Script data>\n");

    NSFDDUMPDumpMemory (pScript, (WORD) scriptLen);

    fprintf (dumpfile, "  <End Lotus Script data>\n");
}

/************************************************************************

    FUNCTION:   DumpFontIDFields

    PURPOSE:        Print the individual fields of a FONTID.

*************************************************************************/

void    LNPUBLIC  DumpFontIDFields( FONTIDFIELDS * pFontID )
{
    fprintf( dumpfile,"Face=%d Attrib=%d Color=%d Size=%d\n",
             pFontID->Face,
             pFontID->Attrib,
             pFontID->Color,
             pFontID->PointSize );
    return;
}
/*************************************************************************

    FUNCTION:   DumpColorValue

    PURPOSE:    Print out the individual components of COLOR_VALUE

**************************************************************************/
void    LNPUBLIC DumpColorValue( COLOR_VALUE pColor )
{
    BOOL hasGradient = FALSE;
    COLOR_VALUE        pColorGradient;

    fprintf( dumpfile, "    Flags = 0x%X: ", pColor.Flags );
    if(pColor.Flags & COLOR_VALUE_FLAGS_ISRGB)
      fprintf(dumpfile, "ISRGB ");
    if(pColor.Flags & COLOR_VALUE_FLAGS_NOCOLOR)
      fprintf(dumpfile, "NOCOLOR ");
    if(pColor.Flags & COLOR_VALUE_FLAGS_SYSTEMCOLOR)
      fprintf(dumpfile, "SYSTEMCOLOR ");

    /* if this flag is set, prepare for reading the second COLOR_VALUE structure for gradients */
    if(pColor.Flags & COLOR_VALUE_FLAGS_HASGRADIENT)
    {
      fprintf(dumpfile, "HASGRADIENT ");
      memcpy(&pColorGradient,&pColor + sizeof(COLOR_VALUE), sizeof(COLOR_VALUE));
      hasGradient = TRUE;
    }

    if(pColor.Flags & COLOR_VALUE_FLAGS_APPLICATION_MASK)
      fprintf(dumpfile, "APPLICATION_MASK ");
    if(pColor.Flags & COLOR_VALUE_FLAGS_RESERVED1)
      fprintf(dumpfile, "RESERVED1 ");
    if(pColor.Flags & COLOR_VALUE_FLAGS_RESERVED2)
      fprintf(dumpfile, "RESERVED2 ");
    if(pColor.Flags & COLOR_VALUE_FLAGS_RESERVED3)
      fprintf(dumpfile, "RESERVED3 ");
    if(pColor.Flags & COLOR_VALUE_FLAGS_RESERVED4)
      fprintf(dumpfile, "RESERVED4 ");

    fprintf(dumpfile, "\n    Component1: Color Red = %d\n", pColor.Component1);
    fprintf(dumpfile, "    Component2: Color Green = %d\n", pColor.Component2);
    fprintf(dumpfile, "    Component3: Color Blue  = %d\n", pColor.Component3);
    fprintf(dumpfile, "    Component4: Not Used    = %d\n", pColor.Component4);

    /* if HASGRADIENT flag set, print out the next COLOR_VALUE structure,
     * but don't check HASGRADIENT Flag for this next structure, this is ignored by Core.
     */
    if(hasGradient)
    {
      fprintf( dumpfile, "    Flags = 0x%X: ", pColorGradient.Flags );
      if(pColorGradient.Flags & COLOR_VALUE_FLAGS_ISRGB)
        fprintf(dumpfile, "ISRGB ");
      if(pColorGradient.Flags & COLOR_VALUE_FLAGS_NOCOLOR)
        fprintf(dumpfile, "NOCOLOR ");
      if(pColorGradient.Flags & COLOR_VALUE_FLAGS_SYSTEMCOLOR)
        fprintf(dumpfile, "SYSTEMCOLOR ");
      if(pColorGradient.Flags & COLOR_VALUE_FLAGS_APPLICATION_MASK)
        fprintf(dumpfile, "APPLICATION_MASK ");
      if(pColorGradient.Flags & COLOR_VALUE_FLAGS_RESERVED1)
        fprintf(dumpfile, "RESERVED1 ");
      if(pColorGradient.Flags & COLOR_VALUE_FLAGS_RESERVED2)
        fprintf(dumpfile, "RESERVED2 ");
      if(pColorGradient.Flags & COLOR_VALUE_FLAGS_RESERVED3)
        fprintf(dumpfile, "RESERVED3 ");
      if(pColorGradient.Flags & COLOR_VALUE_FLAGS_RESERVED4)
        fprintf(dumpfile, "RESERVED4 ");

      fprintf(dumpfile, "\n    Component1: Color Red = %d\n", pColorGradient.Component1);
      fprintf(dumpfile, "    Component2: Color Green = %d\n", pColorGradient.Component2);
      fprintf(dumpfile, "    Component3: Color Blue  = %d\n", pColorGradient.Component3);
      fprintf(dumpfile, "    Component4: Not Used    = %d\n", pColorGradient.Component4);
    }

    return;
}


/****************************************************************************************

    FUNCTION:   DumpLengthValue

    PURPOSE:    Print out the individual components of LENGTH_VALUE

*****************************************************************************************/
void    LNPUBLIC DumpLengthValue( LENGTH_VALUE pLength )
{
	fprintf( dumpfile, "    Flags = 0x%X ", pLength.Flags );
	if(pLength.Flags & CDLENGTH_FLAGS_AUTO)
		fprintf(dumpfile, "AUTO ");
	if(pLength.Flags & CDLENGTH_FLAGS_INHERIT)
		fprintf(dumpfile, "INHERIT ");
	fprintf( dumpfile, "\n");
	fprintf( dumpfile, "    Length = %f\n", pLength.Length );
	fprintf( dumpfile, "    Units = %u", pLength.Units );
	if( CDLENGTH_UNITS_UNKNOWN == pLength.Units)
		fprintf( dumpfile, "     UNKNOWN" );
	if( CDLENGTH_UNITS_TWIPS == pLength.Units)
		fprintf( dumpfile, "     TWIPS" );
	if( CDLENGTH_UNITS_PIXELS == pLength.Units)
		fprintf( dumpfile, "     PIXELS" );
	if( CDLENGTH_UNITS_PERCENT == pLength.Units)
		fprintf( dumpfile, "     PERCENT" );
	if( CDLENGTH_UNITS_EMS == pLength.Units)
		fprintf( dumpfile, "     EMS" );
	if( CDLENGTH_UNITS_EXS == pLength.Units)
		fprintf( dumpfile, "     EXS" );
	if( CDLENGTH_UNITS_CHARS == pLength.Units)
		fprintf( dumpfile, "     CHARS" );
	fprintf( dumpfile, "\n");
	fprintf( dumpfile, "    Reserved = %u\n", pLength.Reserved );
      return;
}


/*************************************************************************

    FUNCTION:   PrintCompositeBuffer

    PURPOSE:    For each CD record in the buffer, call the action routine.
                This function is similar to EnumCompositeBuffer(), but is
                not limited to items of TYPE_TEXT or TYPE_COMPOSITE.  This
                routine also takes a pointer as an argument, rather than
                a BLOCKID.  Just like EnumCompositeBuffer(), this routine
                will quit if a non-zero status code is returned from the
                callback.  If the status code ERR_ODS_ENUM_COMPLETE is
                returned, this routine will return NOERROR to the caller.

**************************************************************************/
STATUS LNPUBLIC PrintCompositeBuffer ( char FAR *              pBuffer,
                                       DWORD                   ItemValueLength,
                                       ActionRoutinePtr        ActionRoutine,
                                       void FAR *              pContext)
{
    STATUS                  error = NOERROR;
    char FAR *              Endptr;

    /* Loop through the composite buffer. */
    Endptr = pBuffer + ItemValueLength;

    while (pBuffer < Endptr)
    {
      char    *RecordStart = pBuffer;
      WORD    RecordType, Length,FixedSize;
      DWORD   RecordLength;

      /*      Get the signature of the record.  The first byte is the SIG_xxx
              value, and the second byte is a code which indicates the number
              of bits necessary to hold the record's length. */
      ODSReadMemory(&pBuffer, _WORD, &RecordType, 1);
      switch (RecordType & 0xFF00)
      {
        case LONGRECORDLENGTH:
          ODSReadMemory(&pBuffer, _DWORD, &RecordLength, 1);
          FixedSize = sizeof(LSIG);
          break;

        case WORDRECORDLENGTH:
          ODSReadMemory(&pBuffer, _WORD, &Length, 1);
          RecordLength = (DWORD) Length;
          FixedSize = sizeof(WSIG);
          break;

        default:
          RecordLength = (DWORD) ((RecordType >> 8) & 0x00FF);
          RecordType &= 0x00FF; /* Length not part of signature */
          FixedSize = sizeof(BSIG);
          break;
      }

      /*      Call the action routine with the pointer to the record, the
              type, length, and any context. */

      if (error = (*ActionRoutine)(RecordStart, RecordType, RecordLength, pContext))
        break;

      /*      Advance to the next record. */

      if (RecordLength)
        pBuffer = RecordStart + RecordLength;
      else
        pBuffer = RecordStart + FixedSize;

      #if defined W64
        if (1 & ((long long)pBuffer))      /* pad record length if odd */
          pBuffer++;                              /* (all records are word aligned) */
      #else
        if (1 & ((long)pBuffer))      /* pad record length if odd */
          pBuffer++;                              /* (all records are word aligned) */
      #endif

    } /* end while */

    if (ERR_ODS_ENUM_COMPLETE == error)
      error = NOERROR;

    return(error);
}

/*************************************************************************

    FUNCTION:   GetLiteralString

    PURPOSE:    Search the table for a constant that matches the
                dwValue argument, and return the matching string
                pointer.  The last entry in the table must have
                the value LITERAL_TABLE_END.

**************************************************************************/

char * LNPUBLIC GetLiteralString ( LITERAL_NAME_ENTRY *pTable,
                                   DWORD dwValue)
{
    /* Scan the table for a match or the end marker */
    while ((LITERAL_TABLE_END != pTable->dwValue)
            && (dwValue != pTable->dwValue))
      pTable++;

    /* Return the pointer to the string */
    return (pTable->pName);
}

/*************************************************************************

    FUNCTION:   PrintCanonicalValue

    PURPOSE:    Print an arbitrary Notes computable data type, given
                a pointer to the data, the type, and the length.

**************************************************************************/

void LNPUBLIC PrintCanonicalValue ( char *          pValue,
                                    WORD            wType,
                                    WORD            wValueLen)
{
    switch (wType)
    {
      case TYPE_TEXT:
        PrintText (pValue, wValueLen);
        break;

      case TYPE_TEXT_LIST:
        DumpTextlistItem (pValue, wValueLen);
        break;

      case TYPE_NUMBER:
        DumpNumberItem (pValue, wValueLen);
        break;

      case TYPE_NUMBER_RANGE:
        DumpNumberRangeItem (pValue, wValueLen);
        break;

      case TYPE_TIME:
        DumpTimeItem (pValue, wValueLen);
        break;

      case TYPE_TIME_RANGE:
        DumpTimeRangeItem (pValue, wValueLen);
        break;

      default:
        fprintf (dumpfile, "* * * Not a computable data type * * *\n");
        break;
    }

    return;
}

/*************************************************************************

    FUNCTION:   PrintOleGuid

    PURPOSE:    Print an OLE Global Unique ID in conventional format.
                Includes a newline at the end of the output string.

**************************************************************************/

void    LNPUBLIC  PrintOleGuid ( void *pData )
{
    OLE_GUID *      pOleGuid = (OLE_GUID *) pData;
    int                     index;

    fprintf (dumpfile, "%08lx-%04hx-%04hx-%02x%02x-",
             pOleGuid->Data1, pOleGuid->Data2, pOleGuid->Data3,
             (int) (pOleGuid->Data4[0]), (int) (pOleGuid->Data4[1]));
    for (index = 2; index < 8; index++)
      fprintf (dumpfile, "%02x", (int) (pOleGuid->Data4[index]));
    fprintf (dumpfile, "\n");
}

/*************************************************************************

    FUNCTION:   PrintNotesActions

    PURPOSE:    Print the Notes action records in the specified buffer.

**************************************************************************/

void LNPUBLIC PrintNotesActions ( char FAR *      pBuffer,
                                  DWORD           BufferLen )
{
    WORD    actionType;

    /* Check the data type */
    ODSReadMemory (&pBuffer, _WORD, &actionType, 1);
    /* Core bug in Notes - the action type word is not */
    /* always being written in canonical order!        */
    /*      if (TYPE_ACTION == actionType)                                  */
    /*      {                                                                                               */
              /* Print the action records */
    DumpActionItem (NULL, 0, pBuffer, ((WORD) (BufferLen - ODSLength (_WORD))));
    /*      }                                                                                               */
    /*      else                                                                                    */
    /*      {                                                                                               */
    /*              fprintf (dumpfile, "    Unknown action type code: %d (0x%X)\n", */
    /*                      actionType, actionType);                                */
    /*      }                                                                                               */

    return;
}
#ifdef __cplusplus
}
#endif
