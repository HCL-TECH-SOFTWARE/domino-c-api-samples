/*************************************************************************
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
 *
 * FILEATT - Sample program that inserts file attachments into a
 *              document.
 *
 *
 *************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* Platform specific Header files.  */
#if defined(OS400)
#include <global.h>           /* global.h must be first for i5/OS */
#endif

#if defined(UNIX) || defined(MAC)         /* UNIX and MAC */
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include <unistd.h>
#else               /* Wintel */
#include <windows.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <io.h>
#include <stdio.h>
#endif

#include <global.h>
#include <addin.h>
#include <nsfdb.h>
#include <nif.h>
#include <nsfsearc.h>
#include <kfm.h>
#include <osmisc.h>

#include <nsfdb.h>
#include <nif.h>
#include <nsfsearc.h>
#include <kfm.h>


 /* HCL C API for Notes/Domino Header files.  */
#if !defined(OS400)
#include <global.h>
#endif
#include <nsfdata.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <editods.h>
#include <stdnames.h>
#include <osmem.h>
#include <osmisc.h>
#include <ostime.h>
#include <oserr.h>
#include <nsfobjec.h>
#include <misc.h>
#include <osfile.h>

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

#include <nif.h>
#include <textlist.h>
#include <osmem.h>
#include <misc.h>
#include <miscerr.h>
#include <editods.h>
#include "../../printLog.h"

STATUS LNPUBLIC AttachFile1(  /* using NSFNoteAttachFile */
    NOTEHANDLE hNote);

STATUS LNPUBLIC AttachFile2(  /* using low-level FILEOBJECT routines */
    DBHANDLE hDB,
    NOTEHANDLE hNote);

void  LNPUBLIC   DumpObjectItem( char * Name,
                                    WORD NameLength,
                                    char * pData);

STATUS   LNPUBLIC   DumpOneItem( WORD       Spare,
                                 WORD       ItemFlags,
                                 char far * Name,
                                 WORD       NameLength,
                                 void far * Value,
                                 DWORD      ValueLength,
                                 void far * Parameter );

STATUS LNPUBLIC DumpOneNote( void far *Param,
                 SEARCH_MATCH far *pSearchMatch,
                 ITEM_TABLE far *SummaryBuffer);


void GetFileT(
    time_t *systime,
    TIMEDATE *pTime);

#if defined(UNIX) || defined(MAC)
#define LAPI_OpenFile(f,m) open(f, O_RDONLY, 0777)
#define LAPI_FILE short
#define LAPI_CloseFile(f) close(f)
#define READPERM (_O_RDONLY | _O_BINARY)

#else
#define LAPI_OpenFile(f,m) _open(f,m)
#define LAPI_FILE HFILE
#define LAPI_CloseFile(f) _close(f)
#define READPERM (_O_RDONLY | _O_BINARY)
#endif

#define NUM_NOTE_CLASSES    13
#define LINELENGTH  68

/*
 * Global variables
 */

DWORD       dwItem;

/*
 * ObjectClassTable - used by DumpObjectItem()
 * The symbolic constant NUM_NOTE_CLASSES is defined in nsf_dump.h
 */
char szNOTE_CLASS_DOCUMENT1[]    = "NOTE_CLASS_DOCUMENT";
char szNOTE_CLASS_INFO1[]        = "NOTE_CLASS_INFO";
char szNOTE_CLASS_FORM1[]        = "NOTE_CLASS_FORM";
char szNOTE_CLASS_VIEW1[]        = "NOTE_CLASS_VIEW";
char szNOTE_CLASS_ICON1[]        = "NOTE_CLASS_ICON";
char szNOTE_CLASS_DESIGN1[]      = "NOTE_CLASS_DESIGN";
char szNOTE_CLASS_ACL1[]         = "NOTE_CLASS_ACL";
char szNOTE_CLASS_HELP_INDEX1[]  = "NOTE_CLASS_HELP_INDEX";
char szNOTE_CLASS_HELP1[]        = "NOTE_CLASS_HELP";
char szNOTE_CLASS_FILTER1[]      = "NOTE_CLASS_FILTER";
char szNOTE_CLASS_FIELD1[]       = "NOTE_CLASS_FIELD";
char szNOTE_CLASS_REPLFORMULA1[] = "NOTE_CLASS_REPLFORMULA";
char szNOTE_CLASS_PRIVATE1[]            = "NOTE_CLASS_PRIVATE";

static  struct
{
    WORD    fNoteClass;                 /* NOTE_CLASS value */
    char   *szNoteClass;                /* descriptive string */
}   ObjectClassTable[NUM_NOTE_CLASSES] = {
        NOTE_CLASS_DOCUMENT,            szNOTE_CLASS_DOCUMENT1,
        NOTE_CLASS_INFO,                szNOTE_CLASS_INFO1,
        NOTE_CLASS_FORM,                szNOTE_CLASS_FORM1,
        NOTE_CLASS_VIEW,                szNOTE_CLASS_VIEW1,
        NOTE_CLASS_ICON,                szNOTE_CLASS_ICON1,
        NOTE_CLASS_DESIGN,              szNOTE_CLASS_DESIGN1,
        NOTE_CLASS_ACL,                 szNOTE_CLASS_ACL1,
        NOTE_CLASS_HELP_INDEX,          szNOTE_CLASS_HELP_INDEX1,
        NOTE_CLASS_HELP,                szNOTE_CLASS_HELP1,
        NOTE_CLASS_FILTER,              szNOTE_CLASS_FILTER1,
        NOTE_CLASS_FIELD,               szNOTE_CLASS_FIELD1,
        NOTE_CLASS_REPLFORMULA,         szNOTE_CLASS_REPLFORMULA1,
        NOTE_CLASS_PRIVATE,                               szNOTE_CLASS_PRIVATE1
};

#define NAME_LEN 256
char szSourceFileName[NAME_LEN];
char szDbName[]         = "embedole.nsf";
char szFormName[]       = "Simple Form";

NOTEHANDLE  hNote=NULLHANDLE;              /* handle to currently open note */

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

STATUS PrintAttachmentDetails(DBHANDLE);

STATUS CreateAttFile(const char *fileName)
{
    FILE *fptr=NULL;
    fptr = fopen(fileName, "w");

    if (fptr == NULL)
    {
        PRINTLOG("File does not exist.\n");
        return 1;
    }
    PRINTLOG("\nFile Created[%s]", fileName);
    fclose(fptr);
    return NOERROR;
}


/*
 *     Main Program
 */

int main(int argc, char *argv[])
{
    DBHANDLE hDB;
    NOTEHANDLE lhNote;
    WORD ClassDocument = NOTE_CLASS_DOCUMENT;
    STATUS sError = NOERROR;

    if (argc == 2)
    {
        memset(szSourceFileName, '\0', NAME_LEN);
        strncpy(szSourceFileName, argv[1], NAME_LEN-1);
    }
    else
    {
        PRINTLOG("\n Usage: %s <full path soruce file name>\n", argv[0]);
        PRINTLOG("\n Exp: %s D:\\notesapi\\samples\\misc\\fileatt\\readme.txt", argv[0]);
        return 1;
    }

    if (sError = NotesInitExtended (argc, argv))
    {
        PRINTLOG("\n Unable to initialize Notes. Error Code[0x%04x]\n", sError);
        return (1);
    }


    /*
     * Open database
     */

    if (sError = NSFDbOpen(szDbName, &hDB))
    {
        PRINTERROR (sError,"NSFDbOpen");  
        NotesTerm();
        return (1);
    }

    /*
     * Now create a note in database
     */

    if (sError = NSFNoteCreate(hDB, &lhNote))
    {
        NSFDbClose(hDB);
        PRINTERROR (sError,"NSFNoteCreate");  
        NotesTerm();
        return (1);
    }

    NSFNoteSetInfo(lhNote, _NOTE_CLASS, &ClassDocument);

    /*
     *  Set the form name. ('Form' field)
     */

    if (sError = NSFItemSetText(lhNote,
                                "Form",
                                szFormName,
                                (WORD)strlen(szFormName)))
    {
        NSFNoteClose(lhNote);
        NSFDbClose(hDB);
        PRINTERROR (sError,"NSFItemSetText");  
        NotesTerm();
        return (1);
    }

    if (sError = CreateAttFile("attach1.txt"))
    {
        NSFNoteClose(lhNote);
        NSFDbClose(hDB);
        PRINTERROR (sError,"CreateAttFile");  
        NotesTerm();
        return (1);
    }

    if (sError = CreateAttFile("attach2.txt"))
    {
        NSFNoteClose(lhNote);
        NSFDbClose(hDB);
        PRINTERROR (sError,"CreateAttFile");  
        NotesTerm();
        return (1);
    }

    if (sError = AttachFile1(lhNote))
    {
        NSFNoteClose(lhNote);
        NSFDbClose(hDB);
        PRINTERROR (sError,"AttachFile1");
        NotesTerm();
        return (1);
    }

    if (sError = AttachFile2(hDB, lhNote))
    {
        NSFNoteClose(lhNote);
        NSFDbClose(hDB);
        PRINTERROR (sError,"AttachFile2");  
        NotesTerm();
        return (1);
    }

    /*
     *  Save the changes
     */

    if (sError = NSFNoteUpdate(lhNote, 0))
    {
        NSFNoteClose(lhNote);
        NSFDbClose(hDB);
        PRINTERROR (sError,"NSFNoteUpdate");  
        NotesTerm();
        return (1);
    }

    /*
     * Close the note
     */

    if (sError = NSFNoteClose(lhNote))
    {
        NSFDbClose(hDB);
        PRINTERROR (sError,"NSFNoteClose");  
        NotesTerm();
        return (1);
    }

    if(sError=PrintAttachmentDetails(hDB))
    {
        NSFDbClose(hDB);
        PRINTERROR (sError,"PrintAttachmentDetails");
        NotesTerm();
        return (1);
    }

    /*
     * Close the database
     */

    if (sError = NSFDbClose(hDB))
    {
        PRINTERROR (sError,"NSFDbClose");  
        NotesTerm();
        return (1);
    }


    NotesTerm();
    PRINTLOG ("\nProgram completed successfully.\n");
    return (0); 

}

STATUS PrintAttachmentDetails(DBHANDLE hDB)
{
    STATUS error=NOERROR;

/* Get the note id of the view we want. */

    if (error = NSFSearch( hDB, NULL, 
                          "New View", 0,
                          NOTE_CLASS_DOCUMENT,
                          NULL,
                          DumpOneNote,
                          &hDB, NULL ))
    {
        fprintf( stderr,"Error encountered searching for data notes.\n" );
        PRINTERROR (error,"NSFSearch");
        return (1);
    }
}

STATUS LNPUBLIC DumpOneNote( void far *Param,
                             SEARCH_MATCH far *pSearchMatch,
                             ITEM_TABLE far *SummaryBuffer)
 
{
        STATUS error=NOERROR;
        DBHANDLE hDB=*(DBHANDLE *)Param;
        SEARCH_MATCH SearchMatch;

    memcpy( (char*)&SearchMatch, (char*)pSearchMatch, sizeof(SEARCH_MATCH) );

    if (error = NSFNoteOpenExt( hDB, SearchMatch.ID.NoteID, 0, &hNote ))
    {
        fprintf( stderr, "Error: unable to open %0x note.\n", SearchMatch.ID.NoteID );
        return (error);
    }

    PRINTLOG("\n Note id: [%0x]\n", SearchMatch.ID.NoteID);

    dwItem = 0;
    NSFItemScan( hNote, DumpOneItem, NULL );

    if (error=NSFNoteClose(hNote))
    {
        return error;
    }
    return (NOERROR);
}

/************************************************************************

    FUNCTION:   DumpOneItem

    PURPOSE:    Print one item from a note to the .

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
    char  * pData;
    WORD    wDataType;


    pData = (char *) Value;

    memcpy( (char*)&wDataType, pData, sizeof(WORD) );
    pData += sizeof(WORD);


    switch (wDataType)
    {
        case TYPE_ERROR:
            break;

        case TYPE_UNAVAILABLE:
            break;

        case TYPE_OBJECT:
            DumpObjectItem( Name, NameLength, pData );
            break;

    }
    return NOERROR;
}


void  LNPUBLIC   DumpObjectItem( char * Name,
                                 WORD NameLength,
                                 char * pData
                                )
{
    OBJECT_DESCRIPTOR   ObjectDesc;
    WORD                wObjectType;
    WORD                wObjectFlags;
    DBHANDLE            hDB;
    DWORD               dwObjectSize;
    WORD                wClass, wPrivs, i;
    FILEOBJECT          FileObject;
    WORD                wFileNameLen;
    WORD                wCompress;
    char                szFileCreated[MAXALPHATIMEDATE+1];
    char                szFileModified[MAXALPHATIMEDATE+1];
    char               *szFileName;
    FILEOBJECT_MACEXT   MACFileObject;
    FILEOBJECT_HPFSEXT  HPFSFileObject;
    STATUS              error;
    char               *pObject;

    /* pData points to data in Domino and Notes canonical format. The first
       structure in the object item is an OBJECT_DESCRIPTOR. We want
       to convert the OBJECT_DESCRIPTOR to host format and store the
       result in ObjectDesc.

       Objects of type OBJECT_FILE consist of an OBJECT_DESCRIPTOR,
       followed by some additional information. To read the OBJECT_FILE
       properly, we need to read the entire object, starting with the
       OBJECT_DESCRIPTOR that we've already read. Since ODSReadMemory
       resets the buffer pointer to the next unread byte, we'll make a copy
       of the pointer to use in one of the calls to ODSReadMemory.
       In earlier versions of this sample program we didn't do this, and
       we weren't processing file objects properly.
    */

    pObject = pData;

    ODSReadMemory( &pObject, _OBJECT_DESCRIPTOR, &ObjectDesc, 1 );

    wObjectType = ObjectDesc.ObjectType;

    if (wObjectType != OBJECT_UNKNOWN)
    {
        /*  OBJECT_NO_COPY and OBJECT_PRESERVE are flags that may be bitwise
             OR-ed into the object type word. If they are, store them in a
             separate word, then mask them out of the object type word.
        */
        wObjectFlags = ( wObjectType & (OBJECT_NO_COPY | OBJECT_PRESERVE) );
        if (wObjectFlags)
        {
            wObjectType &= ~(OBJECT_NO_COPY | OBJECT_PRESERVE);
        }

        if ( wObjectType == OBJECT_FILE )
        {
            PRINTLOG( "  Object Type = OBJECT_FILE.\n" );
        }
        else if (wObjectType == OBJECT_FILTER_LEFTTODO)
        {
            PRINTLOG(  "  Object Type = OBJECT_FILTER_LEFTTODO.\n");
        }
        else
        {
            PRINTLOG(  "  Object Type %#x not recognized.\n",
                        wObjectType);
        }

        if ( wObjectFlags & OBJECT_NO_COPY )
        {
            PRINTLOG(  "  ObjectFlag OBJECT_NO_COPY is set.\n");
        }

        if ( wObjectFlags & OBJECT_PRESERVE )
        {
            PRINTLOG(  "  ObjectFlag OBJECT_PRESERVE is set.\n");
        }
    }
    else
    {
        PRINTLOG(  "  Object Type = OBJECT_UNKNOWN.\n");
    }

    NSFNoteGetInfo( hNote, _NOTE_DB, &hDB );
    if (error = NSFDbGetObjectSize(
                                   hDB,
                                   ObjectDesc.RRV,
                                   ObjectDesc.ObjectType,
                                   &dwObjectSize,
                                   &wClass,
                                   &wPrivs ))
    {
        PRINTLOG(  "   Error: unable to get object size.\n" );
        PRINTLOG(  "   Error code = %#x.\n", error );
    }
    else
    {

        for (i=0; i<NUM_NOTE_CLASSES; i++)
        {
            if (wClass & ObjectClassTable[i].fNoteClass)
            {
                PRINTLOG(  "  Object Class = %s\n",
                            ObjectClassTable[i].szNoteClass );
                break;
            }
        }

        if (i==NUM_NOTE_CLASSES)
        {
            PRINTLOG(  "  Object Class Unrecognized.\n" );
        }

    }

    if ( wObjectType != OBJECT_FILE )
    {
        return;
    }
    /* FROM HERE ON WE ARE DUMPING FILE OBJECTS ONLY */

   /* pData points to the start of the FILEOBJECT structure in
      canonical format. Convert it to host format, storing the
      result in FileObject. This advances pData to point to the
      next byte after the FILEOBJECT structure in the buffer,
      which is the statr of the original relative file path name.
    */
    ODSReadMemory( &pData, _FILEOBJECT, &FileObject, 1 );
    /* file name */
    wFileNameLen = FileObject.FileNameLength;
    if (szFileName = (char *) malloc(wFileNameLen+1))
    {
        memcpy( szFileName, pData, wFileNameLen );
        szFileName[wFileNameLen] = '\0';
        PRINTLOG(  "  File Name = '%s'.\n", szFileName );
        free(szFileName);
    }
    pData += wFileNameLen;

    /* compression technique */
    wCompress = (WORD)(FileObject.CompressionType & COMPRESS_MASK);
    if (wCompress == COMPRESS_NONE)
    {
        PRINTLOG(  "  Compression Type = NONE.\n" );
    }
    else if (wCompress == COMPRESS_HUFF)
    {
        PRINTLOG(  "  Compression Type = Huffman Encoding.\n" );
    }

}

STATUS LNPUBLIC AttachFile1(  /* using NSFNoteAttachFile */
    NOTEHANDLE hNote)
{
    char szAttachFileName[] = "attach1.txt"; /* UI-visible attachment name */
    STATUS sError;

    PRINTLOG("\n Attaching attachment 1 \n");
    if (sError = NSFNoteAttachFile( hNote,
                                    ITEM_NAME_ATTACHMENT,
                                    (WORD) strlen(ITEM_NAME_ATTACHMENT),
                                    szSourceFileName,
                                    szAttachFileName,
                                    HOST_LOCAL | COMPRESS_NONE))
    {
        return(ERR(sError));
    }

    return(NOERROR);
}

STATUS LNPUBLIC AttachFile2(  /* using low-level FILEOBJECT routines */
    DBHANDLE hDB,
    NOTEHANDLE lhNote)
{
    char szAttachFileName[] = "attach2.txt"; /* UI-visible attachment name */
    BYTE   *pObjectItemBuffer;
    DHANDLE hObjectItemBuffer;
    DWORD  dwObjectID;
    LAPI_FILE hAttachFile;
    WORD *pw;
    char *pchar;
    DWORD dwItemSize;
    FILEOBJECT objFile;
    BLOCKID    bidFile;
    int        i;

#if defined(UNIX) || defined(MAC)
    struct  stat AttachStatBuf;
#else
    struct  _stat AttachStatBuf;
#endif

    TIMEDATE MyTimeDate;
    STATUS sError;
    PRINTLOG("\n Attaching attachment 2 \n");

    hAttachFile = LAPI_OpenFile(szSourceFileName, READPERM);

    if (hAttachFile == -1)
        return (ERR_NOEXIST);

#if defined(UNIX) || defined(MAC)
    i = fstat(hAttachFile, &AttachStatBuf);
    if (i == -1)
        return (ERR_FD);
    objFile.FileSize = AttachStatBuf.st_size;
#else
    i = _fstat(hAttachFile, &AttachStatBuf);
    if (i == -1)
        return (ERR_FD);
    objFile.FileSize = AttachStatBuf.st_size;
#endif

/*
 *  Allocate memory for the object item.
 */


    if (sError = NSFDbAllocObject(hDB,
                                  objFile.FileSize,
                                  NOTE_CLASS_DOCUMENT,
                                  0,
                                  &dwObjectID))

    {
        return (ERR(sError));
    }

    objFile.Header.ObjectType = OBJECT_FILE;
    objFile.Header.RRV = dwObjectID;
    objFile.FileNameLength = strlen(szAttachFileName);
    objFile.HostType = HOST_LOCAL;
    objFile.CompressionType = COMPRESS_NONE;
    objFile.FileAttributes = 0;
    objFile.Flags = 0;

    GetFileT(&AttachStatBuf.st_mtime, &MyTimeDate);

    objFile.FileCreated = MyTimeDate;
    objFile.FileModified = MyTimeDate;

    dwItemSize = (DWORD)(sizeof(WORD) + ODSLength(_FILEOBJECT) + strlen(szAttachFileName));

    if (sError = OSMemAlloc(0, dwItemSize, &bidFile.pool))
    {
        NSFDbFreeObject(hDB, dwObjectID);
        return (ERR(sError));
    }

    bidFile.block = NULLBLOCK;

    pw = OSLockBlock(WORD, bidFile);
    *pw = TYPE_OBJECT;
    pw++;

    pchar = (char *) pw;
    ODSWriteMemory (&pw, _FILEOBJECT, &objFile, 1);
    pchar += ODSLength(_FILEOBJECT);
    memcpy(pchar, szAttachFileName, strlen(szAttachFileName)); 

    OSUnlockBlock(bidFile);

    if (sError = NSFItemAppendObject(lhNote,
                                     ITEM_SIGN | ITEM_SEAL | ITEM_SUMMARY,
                                     ITEM_NAME_ATTACHMENT,
                                     (WORD)strlen(ITEM_NAME_ATTACHMENT),
                                     bidFile,
                                     dwItemSize,
                                     TRUE))
    {
        OSMemFree(bidFile.pool);
        return (ERR(sError));
    }

    OSMemAlloc(0, objFile.FileSize, &hObjectItemBuffer);
    pObjectItemBuffer = (BYTE *) OSLockObject(hObjectItemBuffer);

    pchar = OSLock(char, hObjectItemBuffer);

#if defined (UNIX) || defined(MAC)
    read(hAttachFile, pchar, (UINT)objFile.FileSize);
#else
    _read(hAttachFile, pchar, (UINT)objFile.FileSize);
#endif

    OSUnlockObject(hObjectItemBuffer);
    sError = NSFDbWriteObject(hDB,
                              dwObjectID,
                              hObjectItemBuffer,
                              0,
                              objFile.FileSize);

    OSUnlockObject(hObjectItemBuffer);
    OSMemFree(hObjectItemBuffer);

    if (sError = NSFNoteUpdate(lhNote, 0))
    {
        return (ERR(sError));
    }
    LAPI_CloseFile(hAttachFile);

    return(NOERROR);
}


/***************************************************************************
 *
 *  GetFileT( time_t *systime, TIMEDATE *pTime)
 *
 *  Convert a time stored in a time_t variable to a Domino and Notes TIMEDATE
 *  variable.
 *
 *  Inputs:   systime   - a pointer to a time_t variable containing the
 *                        time to be converted.
 *            pTimeDate - a pointer to a Domino and Notes TIMEDATE variable
 *                        in which to store the converted time.
 *
 *  Outputs:  None
 *
 ****************************************************************************/

void GetFileT(time_t *systime, TIMEDATE *pTime)
{
   
    TIME   NotesTime;
    struct tm *newtime;

    newtime = localtime(&(*systime)); /* Convert time_t to tm structure,    */
                                      /* corrected for the local time zone. */
/*
 * Convert time from the tm structure to a Domino and Notes TIME structure.
 */

    NotesTime.year = (newtime->tm_year) + 1900; /* newtime->tm_year is current year - 1900 */
    NotesTime.month = (newtime->tm_mon) + 1;
    NotesTime.day = newtime->tm_mday;
    NotesTime.weekday = newtime->tm_wday + 1;
    NotesTime.hour = newtime->tm_hour;
    NotesTime.minute = newtime->tm_min;
    NotesTime.second = newtime->tm_sec;
    NotesTime.hundredth = 0;

/*
 *  Set time zone and Daylight Savings Time indicator.
 */

    OSCurrentTimeZone((int *)&NotesTime.zone, (int *) &NotesTime.dst);

/*
 * Use the integer members of the TIME structure to construct the TIMEDATE
 * member of the TIME structure.
 */

    TimeLocalToGM(&NotesTime);
    *pTime = NotesTime.GM;
    return;
}
