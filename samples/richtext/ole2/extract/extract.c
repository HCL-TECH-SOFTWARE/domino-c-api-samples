/*
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
*/

#include <ole2.h>
#include <ole2ver.h>
#include "initguid.h"

/* C header files */

#include <stdio.h>
#include <string.h>

/* HCL C API for Notes/Domino Header files */

#include <global.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <nsferr.h>
#include <nsfsearc.h>
#include <osmem.h>
#include <nsfole.h>
#include <osmisc.h>
#include "../../../printLog.h"


/* Forward declarations */

STATUS LNPUBLIC ProcessOneNote(
  void far *phDB,
  SEARCH_MATCH far *pSearchMatch,
  ITEM_TABLE far *Unused);

STATUS LNPUBLIC ProcessOneCDRecord(
  char far *pRecord,
  WORD wRecordType,
  DWORD dwRecordLength,
  void far *phNote);

/*
 *  Main Program
 */

int main(int argc, char *argv[])
{

  DBHANDLE hDb;
  char * formula= "@IsAvailable($OLEOBJINFO)";
  FORMULAHANDLE hFormula;
  WORD wdc;  /* "we don't care" return args for NSFFormulaCompile */
  STATUS error=NOERROR;


  if (error = NotesInitExtended (argc, argv))
  {
     PRINTLOG("\n Unable to initialize Notes.\n");
     return (1);
  }


  if (error = NSFDbOpen("embedole.nsf", &hDb))
  {
    PRINTLOG("NSFDbOpen failed...\n");
    goto Done2;
  }

  /* Compile the selection formula. */

  if (error = NSFFormulaCompile(NULL,
                                0,
                                formula,
                                (WORD)strlen(formula),
                                &hFormula,
                                &wdc,
                                &wdc,
                                &wdc,
                                &wdc,
                                &wdc,
                                &wdc))
     goto Done1;

  /* Call ProcessOneNote for each note containing an embedded OLE2 object. */

  if (error = NSFSearch(hDb,
                        hFormula,
                        NULL,
                        0,
                        NOTE_CLASS_DOCUMENT,
                        NULL,
                        ProcessOneNote,
                        &hDb,
                        NULL))
     goto Done0;

Done0:
  /* Free the memory allocated to the compiled formula. */

  OSMemFree(hFormula);

Done1:
  /* Close the database. */

  NSFDbClose(hDb) ;

Done2:
   if (error)
      PRINTERROR (error,"NSFDbOpen");

   NotesTerm();

  /* End of main routine. */
   return (error);

}


/************************************************************************

    FUNCTION:   ProcessOneNote

    PURPOSE:    Action routine called for each note containing an embedded
	        OLE2 object. In turn, calls action routine for each CD
                record in the $OLEOBJINFO item.

*************************************************************************/

STATUS LNPUBLIC ProcessOneNote(
  void far *phDB,
  SEARCH_MATCH far *pSearchMatch,
  ITEM_TABLE far *Unused )
{

  SEARCH_MATCH SearchMatch;
  DBHANDLE     hDB;
  NOTEHANDLE   hNote;
  WORD         wDataType;
  BLOCKID      bidValue;
  DWORD        dwValueLen;
  STATUS       error=NOERROR;

  memcpy((char *)&SearchMatch, (char *)pSearchMatch, sizeof(SEARCH_MATCH));

  if (!(SearchMatch.SERetFlags & SE_FMATCH))
    return (NOERROR);

  hDB = *((DBHANDLE far *)phDB);

  if (error = NSFNoteOpen(hDB, SearchMatch.ID.NoteID, 0, &hNote))
  {
    PRINTLOG("NSFNoteOpen failed...\n");
    return (error);
  }

  /*
   * Find the first $OLEOBJINFO item
   */

  if (error = NSFItemInfo(hNote,
                          OLE_OBJECT_ITEM,
                          (WORD)strlen(OLE_OBJECT_ITEM),
                          NULL,
                          &wDataType,
                          &bidValue,
                          &dwValueLen))
  {
    NSFNoteClose(hNote);
    PRINTLOG("NSFItemInfo failed...\n");
    return (error);
  }

  if (wDataType != TYPE_COMPOSITE)
  {
    PRINTLOG("$OLEOBJINFO item is not Rich Text...\n");
    return (NOERROR);
  }

  EnumCompositeBuffer(bidValue, dwValueLen, ProcessOneCDRecord, &hNote);

  NSFNoteClose(hNote);

  return (error);

}


/************************************************************************

    FUNCTION:   ProcessOneCDRecord

    PURPOSE:    Action routine called for each embedded OLE2 object.
	        Extracts OLE2 object to Structured Storage file.

*************************************************************************/

STATUS LNPUBLIC ProcessOneCDRecord(
  char far *pRecord,
  WORD wRecordType,
  DWORD dwRecordLength,
  void far *phNote)
{

  char far       *pData = pRecord;
  NOTEHANDLE      hNote;
  CDOLEOBJ_INFO   cdOleObjInfo;
  ENCRYPTION_KEY  EncryptKey;
  ENCRYPTION_KEY *pEncryptKey;
  char            szFileName[MAXPATH];
  char            szObjectName[MAXPATH];
  STATUS          error = NOERROR;

  LPMALLOC        lpMalloc    = NULL;
  LPSTORAGE       lpRootStg  = NULL;
  LPSTORAGE       lpSubStg   = NULL;
  LPSTREAM        lpStream    = NULL;
  WCHAR           wcsStgFile[MAXPATH_OLE];
  WCHAR           wcsStream[]    = L"Contents";
  LPOLESTR        lpProgID;
  DWORD           dwMode         = STGM_READWRITE | STGM_SHARE_EXCLUSIVE;
  char            szProgID[MAXPATH_OLE];
  char            szContents[1024];
  ULONG           cbRead;

  if (wRecordType != SIG_CD_OLEOBJINFO)
    goto exit1;

  ODSReadMemory(&pData, _CDOLEOBJ_INFO, &cdOleObjInfo, 1);

  strncpy(szObjectName, pData, cdOleObjInfo.FileObjNameLength);
  szObjectName[cdOleObjInfo.FileObjNameLength] = '\0';

  strcpy(szFileName, szObjectName);
  strcat(szFileName, ".OLE");

  hNote = *((NOTEHANDLE far *)phNote);

  error = NSFNoteDecrypt(hNote, 0, &EncryptKey);
  if(error)
  {
    if(ERR(error) == ERR_NOTE_NOT_SEALED)
      pEncryptKey = NULL;
    else
    {
      PRINTLOG("NSFNoteDecrypt failed...\n");
      goto exit1;
    }
  }

  if (error = NSFNoteExtractOLE2Object(hNote,
                                       szObjectName,
                                       szFileName,
                                       pEncryptKey,
                                       TRUE,
                                       0))
  {
    PRINTLOG("NSFNoteExtractOLE2Object failed...\n");
    goto exit1;
  }

  PRINTLOG("Extracted OLE object to file %s\n",szFileName);

  /* Initialize OLE */

  if (HIWORD(OleBuildVersion()) != rmm)
    goto exit1;

  if (FAILED(OleInitialize(NULL)))
    goto exit1;

  /* Obtain OLE allocator */

  CoGetMalloc(MEMCTX_TASK, &lpMalloc);

  /* Open storage */

  if (!MultiByteToWideChar(CP_ACP,0,szFileName,-1,wcsStgFile,MAXPATH_OLE))
    goto exit2;
  if (FAILED(StgOpenStorage(wcsStgFile,NULL,dwMode,NULL,0,&lpRootStg)))
    goto exit2;
  PRINTLOG("Opened root storage from file %s\n",szFileName);

  /* Get ProgID */

  if(FAILED(ProgIDFromCLSID((REFCLSID)&cdOleObjInfo.OleObjClass, &lpProgID)))
    goto exit3;
  if(!(WideCharToMultiByte(CP_ACP,0,lpProgID,-1,szProgID,MAXPATH_OLE,NULL,NULL)))
    goto exit3;

  /* Open substorage */

  if(FAILED(lpRootStg->lpVtbl->OpenStorage(lpRootStg,lpProgID,NULL,dwMode,NULL,0,&lpSubStg)))
    goto exit3;
  PRINTLOG("Opened substorage '%s'\n",szProgID);

  /* Open 'Contents' stream */

  if(FAILED(lpSubStg->lpVtbl->OpenStream(lpSubStg,wcsStream,NULL,dwMode,0,&lpStream)))
  {
    PRINTLOG("No 'Contents' stream in substorage '%s'\n",szProgID);
    goto exit4;
  }
  PRINTLOG("Opened 'Contents' stream:\n");

  if(FAILED(lpStream->lpVtbl->Read(lpStream,(void *)szContents,1024,&cbRead)))
    goto exit5;
  else
    PRINTLOG("%s\n\n",szContents);

exit5:
  lpStream->lpVtbl->Release(lpStream);

exit4:
  lpSubStg->lpVtbl->Release(lpSubStg);

exit3:
  lpRootStg->lpVtbl->Release(lpRootStg);

exit2:
  lpMalloc->lpVtbl->Release(lpMalloc);
  OleUninitialize();

exit1:
  return (error);

}
