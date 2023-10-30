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
 *    SEL_REP: Create a selective replication note
 *    
 *    Create a selective replication formula note in a database replica
 *    located on a workstation. The replication formula note consists of the
 *    following items:
 *
 *    $TITLE:          This is a item of type TYPE_TEXT.  It specifies the
 *                     name replication destination. In this sample it
 *                     is the network name of the local workstation.
 *
 *    $ReplVersion:    This is an item of type TYPE_TEXT.  It must contain
 *                     the character "1", followed by a null terminator.
 *
 *    $ReplSrcServers: This item is of type TYPE_TEXT_LIST. It contains the
 *                     names of servers from which replicated documents
 *                     will be received.
 *
 *    $ReplClassMasks: This item is of type TYPE_TEXT_LIST, and it defines
 *                     the types of documents that will be replicated with
 *                     the corresponding servers in the $ReplSrcServers list.
 *
 *                     The types of notes that may be replicated are defined
 *                     by the bitmasks in NOTECLASS.  The string stored in
 *                     the $ReplClassMasks text list for each source server
 *                     is a character representation of the decimal value
 *                     of the bitmask.
 *
 *    $ReplFormula:    This is an item of type TYPE_FORMULA. It defines
 *                     the selection criteria for documents that will be
 *                     replicated. There is one $ReplFormula item for
 *                     each server with which the workstation will replicate,
 *                     and they must appear in the same order as their
 *                     corresponding entries in the $ReplSrcServer item.
 *
 *    NOTE:  The user must edit the string defined by szServerName below
 *           to specify the name of the Domino server on which the database
 *           is stored.
 *************************************************************************/
 
/* C header files     */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* HCL C API for Notes/Domino Header files.  */

#include <global.h>
#include <nsfdata.h>
#include <nif.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <nsferr.h>
#include <editods.h>
#include <osmem.h>
#include <osenv.h>
#include <reg.h>
#include <stdnames.h>
#include <names.h>
#include <nsfsearc.h>
#include <osfile.h>
#include <idtable.h>  
#include "../../printLog.h"
#include <osmisc.h>  

/*
 * Local header files
 */

#include "sel_rep.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/*
 *     Main Program
 *
 */

int main(int argc, char *argv[])
{
    NOTEHANDLE hNote; 
    STATUS sError = NOERROR;
    DBHANDLE hDB, hOrigDB;

    char *szServerName = NULL;
    char szFileName[] = "test.nsf";
    char szFullPathName[MAXPATH];
    char szReplVersion[] = "1";
    
    WORD   wdc;           /* "We Don't Care" - Used to receive data that */
                          /* we don't care about that is returned by     */
                          /* NSFFormulaCompile.                          */

    WORD wItemLength;
    WORD wTextBufferLength = TEXTLIST_BUFFER_LENGTH;
    
    BYTE *pBuffer, *pItem;
    char IDFileSpec[MAXENVVALUE+1];
    char UserName[MAXUSERNAME+1];
    WORD retUserNameLen;
    char SrcServerName[] = "-";
    char szSelFormula[] = "SELECT (numberfield < 100)";
    char *pSelFormula;

    DHANDLE hSelFormula;

    WORD wSelFormulaLen;

    int  iClasses;
    char ClassMaskString[CLASS_STRING_LENGTH];
    
    DHANDLE hMem;
    WORD ClassReplFormula = NOTE_CLASS_REPLFORMULA;

    DBREPLICAINFO    ReplicaInfo;  /* replication info for the databases */
    TIMEDATE         StartTime;    /* time and date used to control what
                                      notes we copy */
    TIMEDATE         LastTime;          /* returned from 
                                      NSFDbGetModifiedNoteTable */
    DBID             OrigDBID;          /* dbid of input database */
    DBID             RepDBID;           /* dbid of output database */
    DHANDLE          pIDTable;          /* handle to id table */
    DWORD            NumScanned, NumEntries;
    NOTEID           nid;

    if (argc != 2)
    {
        PRINTLOG("\nUsage:  %s <server name>\n", argv[0]);
        return (0);
    }
    else
    {
        szServerName = argv[1];
    }

/*
 *   Start by calling Notes InitExtended.
 */

    if (sError = NotesInitExtended (argc, argv))
    {
        PRINTLOG("\n Unable to initialize Notes.\n");
        return (1);
    }


/*
 *  Build the network path to the server copy of the database
 *  we wish to replicate.
 */

    if (sError = OSPathNetConstruct(NULL,
                 szServerName,
                 szFileName,
                 szFullPathName))
    {
        PRINTERROR (sError,"OSPathNetConstruct");  
        NotesTerm();
        return (1);
    }
 
/*
 * Open the original database.
 */

    if (sError = NSFDbOpen(szFullPathName, &hOrigDB))
    {
        PRINTERROR (sError,"NSFDbOpen");  
        NotesTerm();
        return (1);
    }

    PRINTLOG ("\nCreating replica database.\n");

/*
 *  Create and open the local (replica) database.
 */

    if (sError = NSFDbCreate(szFileName,
                 DBCLASS_NOTEFILE,
                 TRUE))

    {
        NSFDbClose(hOrigDB);
        PRINTERROR (sError,"NSFDbCreate");  
        NotesTerm();
        return (1);
    }

    if (sError = NSFDbOpen(szFileName, &hDB))
    {
        NSFDbClose(hOrigDB);
        PRINTERROR (sError,"NSFDbOpen");  
        NotesTerm();
        return (1);
    }


/*  Copy the replication settings (not the replication history) from
    the input database to the output database. The replication settings
    include the database replica ID. This makes the destination database
    a replica copy of the source database.
 */

    if (sError = NSFDbReplicaInfoGet (hOrigDB, &ReplicaInfo))
    {
        NSFDbClose (hOrigDB);
        NSFDbClose (hDB);
        PRINTERROR (sError,"NSFDbReplicaInfoGet");  
        NotesTerm();
        return (1);
    }

/*
 *  Set the replication flags so that deletes are not propagated when
 *  replicating
 */        

    ReplicaInfo.Flags |= REPLFLG_IGNORE_DELETES;
   
    if (sError = NSFDbReplicaInfoSet (hDB, &ReplicaInfo))
    {
        NSFDbClose (hOrigDB);
        NSFDbClose (hDB);
        PRINTERROR (sError,"NSFDbReplicaInfoSet");  
        NotesTerm();
        return (1);
    }

/*
 *  Copy the ACL from the input database to the output database.
 */

    if (sError = NSFDbCopyACL (hOrigDB, hDB))
    {
        NSFDbClose (hOrigDB);
        NSFDbClose (hDB);
        PRINTERROR (sError,"NSFDbCopyACL");  
        NotesTerm();
        return (1);
    }

/*
 *  Set a time/date structure that will determine the date of the earliest
 *  note copied in the next call.  Use TimeConstant with TIMEDATE_WILDCARD
 *  specified to indicate that we do not want any cutoff date.
 */

    TimeConstant (TIMEDATE_WILDCARD, &StartTime);

/* Do not use NSFDbCopy to copy all notes in the input database to the 
   output database. Such copies are not guaranteed to be replicas of 
   the original notes.  Instead get an IDTABLE of all notes in the 
   database, use IDScan to obtain each NOTEID, and then call 
   NSFDbCopyNote to copy each note from one database to the other.
*/

    NSFDbIDGet (hOrigDB, &OrigDBID);
    NSFDbIDGet (hDB, &RepDBID);

/* Get the NoteID table for all notes in the input database */
    if (sError = NSFDbGetModifiedNoteTable (hOrigDB,
                                            NOTE_CLASS_ALL,
                                            StartTime, &LastTime,
                                            &pIDTable) )
        if (sError != ERR_NO_MODIFIED_NOTES)
        {
            IDDestroyTable (pIDTable);
            NSFDbClose (hOrigDB);
            NSFDbClose (hDB);
            PRINTERROR (sError,"NSFDbGetModifiedNoteTable");  
            NotesTerm();
            return (1);
        }

    NumScanned = 0L;
    NumEntries = IDEntries (pIDTable);
    if (NumEntries)
        while (IDScan (pIDTable, (FLAG)(NumScanned++ == 0), &nid) )
            if (sError = NSFDbCopyNote (hOrigDB, NULL,
                                        &ReplicaInfo.ID, nid,
                                        hDB, &RepDBID,
                                        &ReplicaInfo.ID, NULL,
                                        NULL) )
            {
                IDDestroyTable (pIDTable);
                NSFDbClose (hOrigDB);
                NSFDbClose (hDB);
                PRINTERROR (sError,"NSFDbCopyNote");  
                NotesTerm();
                return (1);
            }
    IDDestroyTable (pIDTable);
                       
/*
 *  Close the original database - we're done with it.
 */

    NSFDbClose (hOrigDB);

   PRINTLOG("\nReplica database created.\n"); 
   PRINTLOG ("\nCreating selective replication formula.\n");
/*
 *  We now have a replica of the original database. Create a
 *  replication formula note now.  First, create a note in database.
 */

    if (sError = NSFNoteCreate(hDB, &hNote))
    {
        NSFDbClose(hDB);            /* Close database            */
        PRINTERROR (sError,"NSFNoteCreate");  
        NotesTerm();
        return (1);
    }

/*
 *  Set the NOTE_CLASS appropriately.
 */

    NSFNoteSetInfo(hNote, _NOTE_CLASS, &ClassReplFormula);
 
    

/*
 *  Set the name of the destination for replicated notes. ($TITLE item).
 *  In this case, the destination is the local workstation.
 *  First, get the name of the workstation's user.
 */
    if (!OSGetEnvironmentString ("KeyFileName", IDFileSpec, 
                                          MAXENVVALUE))

    {
        NSFDbClose(hDB);            /* Close database            */
        PRINTERROR (sError,"NSFNoteCreate");  
        NotesTerm();
        return (1);
    }
    if (sError = REGGetIDInfo (IDFileSpec, REGIDGetName, UserName, 
                               MAXUSERNAME+1, &retUserNameLen))
    {
        NSFDbClose(hDB);            /* Close database            */
        PRINTERROR (sError,"REGGetIDInfo");  
        NotesTerm();
        return (1);
    }

/*
 *  Now set the $TITLE item.
 */
    
    if (sError = NSFItemSetText(hNote,
                                REPLFORMULA_SERVER_ITEM,
                                UserName,
                                MAXWORD))
    {
        NSFDbClose(hDB);            /* Close database            */
        PRINTERROR (sError,"NSFItemSetText");  
        NotesTerm();
        return (1);
    }

    
/*
 *  Set the REPLFORMULA_VERSION_ITEM.
 */

    if (sError = NSFItemSetText(hNote,
                                REPLFORMULA_VERSION_ITEM,
                                szReplVersion,
                                MAXWORD))
    {
        NSFDbClose(hDB);            /* Close database            */
        PRINTERROR (sError,"NSFItemSetText");  
        NotesTerm();
        return (1);
    }

/*
 *  Create the $ReplSrcServers. This is a text list containing the
 *  names of all source servers.
 *
 *  First, allocate a buffer in which the TEXT_LIST will be built.
 */

    if (sError = OSMemAlloc (0, wTextBufferLength, &hMem))
    {
        PRINTERROR (sError,"OSMemAlloc");  
        NotesTerm();
        return (FALSE);
    }

   
/*
 *  Lock global buffer.
 */
    
    if ((pBuffer = (BYTE*)OSLockObject(hMem)) == NULL)
    {
        OSMemFree (hMem);
        NotesTerm();
        return (FALSE);
    }

/*
 *  Set start length to zero, and point the item pointer to the
 *  start of the buffer.
 */           

    wItemLength = 0;
    pItem = pBuffer;

/*
 *  Now, fill in text list of server names containing database replicas that
 *  documents could be pulled from.  In this example, there is only
 *  one entry, the default "- Any Server -", represented by a single dash
 *  ("-").  This is the required shorthand for specifying "- Any Server -".
 *  This means that when replicating from any server, use the
 *  information in this note.
 *
 *  If different replication criteria were desired for documents being pulled
 *  from a specific server, that server's name would be added as another entry
 *  in the text list.
 */
 
    *((USHORT *) pItem) = 1;       /*  Set number of entries.  */
    
    pItem += sizeof(WORD);
    wItemLength += sizeof(WORD);

 /*
  *  Set length of first source server.
  */

    *((WORD *) pItem) = sizeof(SrcServerName)-1; 
    pItem += sizeof(WORD);
    wItemLength += sizeof(WORD);

 /*
  *  First source server name.
  */
    
    memcpy(pItem, SrcServerName, sizeof(SrcServerName) -1);
    pItem += (sizeof(SrcServerName) - 1);
    wItemLength += (sizeof(SrcServerName) - 1);

/*
 *  Append the $ReplSrcServers item to the note.
 */

    if (sError = NSFItemAppend(hNote,
                               ITEM_SUMMARY,
                               REPLFORMULA_SOURCE_SERVERS,
                               sizeof (REPLFORMULA_SOURCE_SERVERS)-1,
                               TYPE_TEXT_LIST,
                               pBuffer,
                               wItemLength))
    {
        OSUnlockObject (hMem);
        OSMemFree (hMem);
        NSFDbClose(hDB);             /* Close database            */
        PRINTERROR (sError,"NSFItemAppend");  
        NotesTerm();
        return (1);
    }

/*
 *  Create the $ReplClassMask item. There is only one entry in the text
 *  list, since in this example the replication criteria are the same
 *  regardless of which server the documents are being pulled from.  If
 *  more than one server were specified in $ReplSrcServers item,
 *  character strings representing the classes of documents to be replicated
 *  from the corresponding servers would need to be added to the text list.
 */

/*
 *  Reset start length to zero, and point the item pointer to the
 *  start of the buffer.
 */           

    wItemLength = 0;
    pItem = pBuffer;

    *((USHORT *) pItem) = 1;              /*  Set number of entries.  */
    
    pItem += sizeof(WORD);
    wItemLength += sizeof(WORD);

/*
 *  Convert the note class mask to a character string.
 */
    
    iClasses = MY_NOTE_CLASSES;
    sprintf (ClassMaskString, "%i", iClasses); /* _itoa (iClasses, ClassMaskString, 10); */

/*
 *  Set length of mask string.
 */

    *((WORD *) pItem) = strlen(ClassMaskString);
    pItem += sizeof(WORD);
    wItemLength += sizeof(WORD);

 /*
  *  Copy mask string.
  */
  
    memcpy(pItem, ClassMaskString, strlen(ClassMaskString));
    pItem += (strlen(ClassMaskString));
    wItemLength += (strlen(ClassMaskString));

/*
 *  Append the $ReplClassMasks item to the note.
 */

    if (sError = NSFItemAppend(hNote,
                               ITEM_SUMMARY,
                               REPLFORMULA_NOTECLASS_ITEM,
                               sizeof (REPLFORMULA_NOTECLASS_ITEM)-1,
                               TYPE_TEXT_LIST,
                               pBuffer,
                               wItemLength))
    {
        OSUnlockObject (hMem);
        OSMemFree (hMem);
        NSFDbClose(hDB);             /* Close database            */
        PRINTERROR (sError,"NSFItemAppend");  
        NotesTerm();
        return (1);
    }

/*
 *  Create a $ReplFormula item.  If more than one entry was specified for
 *  The $ReplSrcServers item, a $ReplFormula item would be needed for each
 *  server, and the order of the $ReplFormula items appeared would correspond
 *  to the order of server names in the text list.
 *
 *  Compile the selection formula, get a pointer to it,and append
 *  it to the note.
 */
     
    if (sError = NSFFormulaCompile(NULL,
                                   0,
                                   szSelFormula,
                                   (WORD)strlen(szSelFormula),
                                   &hSelFormula,
                                   &wSelFormulaLen,
                                   &wdc, &wdc, &wdc,
                                   &wdc, &wdc))
    {
        OSUnlockObject (hMem);
        OSMemFree (hMem);
        NSFDbClose(hDB);             /* Close database            */
        PRINTERROR (sError,"NSFFormulaCompile");  
        NotesTerm();
        return (1);
    }

    pSelFormula = (char*) OSLockObject(hSelFormula);
    
/*
 *  Append the $ReplFormula item to the note.
 */

    if (sError = NSFItemAppend(hNote,
                               ITEM_SUMMARY,
                               REPLFORMULA_FORMULA_ITEM,
                               sizeof (REPLFORMULA_FORMULA_ITEM)-1,
                               TYPE_FORMULA,
                               pSelFormula,
                               wSelFormulaLen))
    {
        OSUnlockObject (hMem);
        OSMemFree (hMem);
        NSFDbClose(hDB);             /* Close database            */
        PRINTERROR (sError,"NSFItemAppend");  
        NotesTerm();
        return (1);
    }

/*
 *  Unlock and free memory associated with this formula.
 */

    OSUnlockObject(hSelFormula);
    OSMemFree(hSelFormula);

/*
 *  All items for this note have been defined, so update the note.
 */

    if (sError = NSFNoteUpdate(hNote, 0))
    {
        OSUnlockObject (hMem);
        OSMemFree (hMem);
        NSFDbClose(hDB);             /* Close database            */
        PRINTERROR (sError,"NSFNoteUpdate");  
        NotesTerm();
        return (1);
    }

/* 
 * Close the note
 */

    if (sError = NSFNoteClose(hNote))
    {
        OSUnlockObject (hMem);
        OSMemFree (hMem);
        NSFDbClose(hDB);             /* Close database            */
        PRINTERROR (sError,"NSFNoteClose");  
        NotesTerm();
        return (1);
    }

/*
 * Close the database
 */

    if (sError = NSFDbClose(hDB))
    {
        OSUnlockObject (hMem);
        OSMemFree (hMem);
        PRINTERROR (sError,"NSFDbClose");  
        NotesTerm();
        return (1);
    }

/*
 * leave with no error
 */

    OSUnlockObject (hMem);
    OSMemFree (hMem);
    PRINTLOG ("\nSelective replication formula created.\n");
    PRINTLOG ("\nDone.\n");
    NotesTerm();
    return (0); 
}
