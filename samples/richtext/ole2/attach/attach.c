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
 *    ATTACH:  Create a document containing an OLE2 object.
 *
 *    This program creates a note containing a Microsoft Paint OLE2 object.
 *    The following algorithm is used:
 *
 *    1. Create a root storage in a temporary structured storage file.
 *
 *    2. Create a substorage for the embedded object.
 *
 *    3. Create an object on the substorage from the input bitmap file.
 *
 *    4. Save the object to its persistent storage (structured storage file).
 *
 *    5. Open the database EMBEDOLE.NSF, create a new note.
 *
 *    6. Generate a filename to be assigned to the OLE2 object file when it
 *       is attached to the note.  This program uses the string constant
 *       "EXT12345".  But in a real application, the numeric part of this
 *       string should be a random number between 00000 and 32767.
 *
 *    7. Attach the OLE2 structured storage file to the note by calling
 *       NSFNoteAttachOLE2Object.  Please see the chapter "Embedding and
 *       Extracting OLE2 Objects" in the User Guide for information on OLE2
 *       structured storage files.
 *
 *    8. Create the rich text item containing the sequence of CD records that
 *       define an embedded OLE object. These records include:
 *
 *          a) A CDOLEBEGIN record, which contains information such as
 *             the name of the embedded file and the type of object
 *             (embedded or linked).
 *          b) A CDTEXT item which acts as a temporary representation of
 *             the OLE object. When the ojbect is opened, modified, and closed,
 *             The CDTEXT item will be replaced by sequence of bitmap records.
 *          c) A CDOLEEND record, to mark the end of the OLE object.
 *      
 *    9. Append the rich text item to the note, close the note, and
 *       close the database.
 *
 *************************************************************************/
 
#include <ole2.h>
#include <ole2ver.h>
#include "initguid.h"

/* C header files */

#include <stdio.h>

/* HCL C API for Notes/Domino Header files */

#include <global.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <editods.h>
#include <osmem.h>
#include <nsfole.h>
#include <osmisc.h>
#include "../../../printLog.h"

#include "cdrecord.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/* forward declarations */

STATUS LNPUBLIC AppendOLE2Object(
  void far *,
  DWORD);

STATUS LNPUBLIC AppendRichText(
  NOTEHANDLE hNote,
  char *szItemName,
  char *szObjectName,
  char *szClassName);
 

/*
 *  Main Program
 */

int main(int argc, char *argv[])
{

  LPMALLOC          lpMalloc    = NULL;
  LPSTORAGE         lpRootStg  = NULL;
  LPSTORAGE         lpSubStg   = NULL;
  LPOLEOBJECT       lpOleObj = NULL;
  LPPERSISTSTORAGE  lpPersistStg = NULL;
  WCHAR             szSrcFile[] = L"c:\\notesapi\\samples\\richtext\\ole2\\attach\\notes.bmp";
  CLSID             clsid;
  LPOLESTR          lpProgID;
  DWORD             dwMode = STGM_READWRITE | STGM_SHARE_EXCLUSIVE;
  STATSTG           StatStg;
  char              szStgFile[MAXPATH_OLE];
  DBHANDLE          hDb;
  NOTEHANDLE        hNote;
  char              szObjectName[] = "EXT12345";  /* should really be unique (random) number */
  char              szItemName[] = "Body";
  char              szProgID[MAXPATH_OLE];
  WORD              ClassDocument   = NOTE_CLASS_DOCUMENT;
  STATUS            sError=0;
  BOOL              bError=0;

  /* Initialize Domino and Notes */
   
  if (sError = NotesInitExtended (argc, argv))
  {
     PRINTLOG("\n Unable to initialize Notes.\n");
     return (1);
  }
  /* Initialize OLE */

  if (HIWORD(OleBuildVersion()) != rmm)
    goto exit1;

  if (FAILED(OleInitialize(NULL)))
    goto exit1;
   
  /* Obtain OLE allocator */

  CoGetMalloc(MEMCTX_TASK, &lpMalloc);

  /* Get CLSID */

  if(FAILED(GetClassFile(szSrcFile, &clsid)))
  {
    PRINTLOG ("Sample file used to create OLE2 object not found.\n");
    goto exit2;
  }

  /* Get ProgID */

  if(FAILED(ProgIDFromCLSID(&clsid, &lpProgID)))
    goto exit2;

  /* Create root storage */

  if(FAILED(StgCreateDocfile(NULL,dwMode | STGM_CREATE /*| STGM_DELETEONRELEASE*/,0,&lpRootStg)))
    goto exit3;

  /* Create substorage for embedded object */

  if(FAILED(lpRootStg->lpVtbl->CreateStorage(lpRootStg,lpProgID,dwMode,0,0,&lpSubStg)))
    goto exit4;

  /* Create object on newly created substorage */

  if(FAILED(OleCreateFromFile(&CLSID_NULL,
                              szSrcFile,
                              &IID_IOleObject,
                              OLERENDER_DRAW,
                              NULL,
                              NULL,
                              lpSubStg,
                              (void FAR* FAR*)&lpOleObj)))
    goto exit5;

  /* Ask the object for its persistent storage interface */

  if(lpOleObj->lpVtbl->QueryInterface(lpOleObj,&IID_IPersistStorage,&lpPersistStg))
    goto exit6;

  /* Save the object to the structured storage file */

  if(FAILED(OleSave(lpPersistStg, lpSubStg, TRUE)))
    goto exit7;

  lpPersistStg->lpVtbl->SaveCompleted(lpPersistStg, NULL);

  /* Get temporary storage file name */

  if(FAILED(lpRootStg->lpVtbl->Stat(lpRootStg, &StatStg, STATFLAG_DEFAULT)))
    goto exit7;

  bError = WideCharToMultiByte(CP_ACP,0,StatStg.pwcsName,-1,szStgFile,MAXPATH_OLE,NULL,NULL);

  lpMalloc->lpVtbl->Free(lpMalloc, (LPVOID)StatStg.pwcsName);

  if (bError == FALSE)
    goto exit7;

  /* Open the database */

  if (sError = NSFDbOpen("embedole.nsf", &hDb))
    goto exit7;

  /* Create a new note */

  if (sError = NSFNoteCreate(hDb, &hNote))
    goto exit8;

  /* Create the "Form" item */

  if (sError = NSFItemSetText(hNote, "Form", "Simple Form", MAXWORD))
    goto exit9;

  /* Attach the object */

  if(sError = NSFNoteAttachOLE2Object(hNote,
                                     szStgFile,
                                     szObjectName,
                                     TRUE,
                                     "",
                                     szItemName,
                                     (OLE_GUID *)&clsid,
                                     DDEFORMAT_METAFILE,
                                     FALSE,
                                     FALSE,
                                     0))
    goto exit9;

  /* Append presentation CD records */

  if(!(WideCharToMultiByte(CP_ACP,0,lpProgID,-1,szProgID,MAXPATH_OLE,NULL,NULL)))
    goto exit9;

  sError = AppendRichText(hNote,szItemName,szObjectName,szProgID);
  if (sError)
  {
    if (sError==FALSE)
       sError=0;
    goto exit9;
  }

  /* Set the note type */

  NSFNoteSetInfo(hNote, _NOTE_CLASS, &ClassDocument);

  /* Save the note */

  if (sError = NSFNoteUpdate(hNote, 0))
    goto exit9;

  PRINTLOG("Attached OLE object...\n");

exit9:
  if (sError)
      PRINTERROR (sError,"NSFNoteUpdate");  

  NSFNoteClose(hNote);

exit8:
  NSFDbClose(hDb);

exit7:
  lpPersistStg->lpVtbl->Release(lpPersistStg);

exit6:
  lpOleObj->lpVtbl->Release(lpOleObj);

exit5:
  lpSubStg->lpVtbl->Release(lpSubStg);

exit4:
  lpRootStg->lpVtbl->Release(lpRootStg);

exit3:
  lpMalloc->lpVtbl->Free(lpMalloc, lpProgID);

exit2:
  lpMalloc->lpVtbl->Release(lpMalloc);
  OleUninitialize();

exit1:
  NotesTerm();
  return (sError); 
}


/*************************************************************************

 AppendRichText() - Create the rich text item, append to the note.

 This routine creates an OLE ojbect in a rich text field, and then
 appends the item to the note. The items needed to define 

   OLEBEGIN
   Text
   OLEEND

 Inputs:     hNote        - handle to the new note being created.
             szItemName   - pointer to name of richtext item.
             szObjectName - pointer to name of object file.

*************************************************************************/

STATUS LNPUBLIC AppendRichText(
  NOTEHANDLE hNote,
  char *szItemName,
  char *szObjectName,
  char *szClassName)
{

  DHANDLE  hBuffer;
  BYTE    *pBuffer;
  STATUS   sError=0;
  BOOL     bError=0;

  DWORD    dwBytesLeft = MAXONESEGSIZE;  
  DWORD    dwItemLength = 0; /* Length of current item.  */

  char     nonZeroString[] = "Double-click here *** to activate the object.";

  /*
   *  First, allocate and lock a buffer.
   */
 
  if(sError = OSMemAlloc(0, MAXONESEGSIZE, &hBuffer))
    return (sError);

  if ((pBuffer = OSLockObject(hBuffer)) == NULL)
  {
    OSMemFree (hBuffer);
    PRINTLOG("Error: Can not do OSLockObject on the buffer handle\n");
    return (FALSE);
  }

  /*
   *  Insert a CDOLEBEGIN record.
   */

  bError = PutOLEBegin(pBuffer,
                       szObjectName,
                       szClassName,
                       dwBytesLeft - dwItemLength, 
                       &dwItemLength);

  if (bError == FALSE)
  {
    OSUnlockObject(hBuffer);
    OSMemFree(hBuffer);
    return(FALSE);
  }

  /*
   *  Insert a non-null string.
   */

  bError = PutText(pBuffer,
                   (LPSTR)&nonZeroString,
                   dwBytesLeft - dwItemLength,
                   &dwItemLength);

  if (bError == FALSE)
  {
    OSUnlockObject(hBuffer);
    OSMemFree(hBuffer);
    return(FALSE);
  }

  /*
   *  Insert a CDOLEEND record.
   */

  bError = PutOLEEnd(pBuffer,
                     dwBytesLeft - dwItemLength,
                     &dwItemLength);

  if (bError == FALSE)
  {
    OSUnlockObject (hBuffer);
    OSMemFree (hBuffer);
    return (FALSE);
  }
 
  /*
   *  Append rich text item to the note.
   */

  if(sError = NSFItemAppend(hNote,
                            0,
                            szItemName,
                            (WORD) strlen(szItemName),
                            TYPE_COMPOSITE,
                            (void far *) pBuffer,
                            dwItemLength))
                               
  {
    OSUnlockObject (hBuffer);
    OSMemFree (hBuffer);
    return (sError);
  }
    
  /*
   *  Return.
   */

  OSUnlockObject(hBuffer);
  OSMemFree(hBuffer);
  return(NOERROR);
}
 
