/*
* Copyright HCL Technologies 1996, 2023.
*
* Licensed under the Apache License, Version 2.0 (the "License");
*you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
*WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

/* C header files */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Notes API Header files */
#include <global.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <nsferr.h>
#include <nsfsearc.h>
#include <osmem.h>
#include <nsfasc.h>
#include <osmisc.h>
#include "../../../printLog.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/* Forward declarations */
STATUS LNPUBLIC ProcessOneNote(
  void far *         phDB,
  SEARCH_MATCH far * pSearchMatch,
  ITEM_TABLE far *   Unused);

/* Global variables */
char szDbName[]  = "embedole.nsf";
char szObjName[] = "OLEobject";

/*
 *  Main Program
 */

int main(int argc, char *argv[])
{

  DBHANDLE      hDb;
  char *        formula = "@IsAvailable($OLEOBJINFO)";
  FORMULAHANDLE hFormula;
  WORD          wdc;  /* "we don't care" return args for NSFFormulaCompile */
  STATUS        rc = NOERROR;

  if (NotesInitExtended(argc, argv))
  {
    PRINTLOG("NotesInitExtended failed...\n");
    return(1);
  }

  if (rc = NSFDbOpen(szDbName, &hDb))
  {
    PRINTLOG("NSFDbOpen failed...\n");
    goto Done2;
  }

  /* Compile the selection formula. */
  if (rc = NSFFormulaCompile(NULL,
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
  {
    PRINTLOG("NSFFormulaCompile failed...\n");
    goto Done1;
  }

  /* Call ProcessOneNote for each note containing an embedded OLE2 object. */
  if (rc = NSFSearch(hDb,
                     hFormula,
                     NULL,
                     0,
                     NOTE_CLASS_DOCUMENT,
                     NULL,
                     ProcessOneNote,
                     &hDb,
                     NULL))
  {
    goto Done0;
  }

Done0:
  /* Free the memory allocated to the compiled formula. */

  OSMemFree(hFormula);

Done1:
  /* Close the database. */

  NSFDbClose(hDb) ;

Done2:
  if (rc)
     PRINTERROR (rc,"NSFDbOpen");

  NotesTerm();

  /* End of main routine. */

  return(rc);

}


/************************************************************************

    FUNCTION:   ProcessOneNote

    PURPOSE:    Action routine called for each note containing an embedded
                OLE2 object.

*************************************************************************/

STATUS LNPUBLIC ProcessOneNote(void far *         phDB,
                               SEARCH_MATCH far * pSearchMatch,
                               ITEM_TABLE far *   Unused )
{

  SEARCH_MATCH SearchMatch;
  DBHANDLE     hDB;
  NOTEHANDLE   hNote;
  STATUS       rc=NOERROR;
  DWORD        dwId = 0;	
  DHANDLE      hszAttachmentName = NULLHANDLE;
  char *       pszAttachmentName = NULL;

  memcpy((char *)&SearchMatch, (char *)pSearchMatch, sizeof(SEARCH_MATCH));

  if (!(SearchMatch.SERetFlags & SE_FMATCH))
    return (NOERROR);

  hDB = *((DBHANDLE far *)phDB);

  if (rc = NSFNoteOpen(hDB, SearchMatch.ID.NoteID, 0, &hNote))
  {
    PRINTLOG("NSFNoteOpen failed...\n");
    return (rc);
  }

  /*
   * Get associated file attachments from OLE object
   */

  while (NOERROR == (rc = NSFNoteOLEGetAssociateFileOnObject(hNote,
                                                             szObjName,
                                                             &hszAttachmentName,
                                                             dwId)))
  {
    pszAttachmentName = (char *)OSLockObject(hszAttachmentName);
    PRINTLOG("NSFNoteOLEGetAssociateFileOnObject dwId = %d Attachment Name = %s\n", dwId, pszAttachmentName);
    dwId++;
    OSUnlockObject(hszAttachmentName);
    OSMemFree(hszAttachmentName);
  }

  if (rc != ERR_ARGS_INVALID)
  {
    PRINTLOG("NSFNoteOLEGetAssociateFileOnObject failed...\n");
  }
  else
  {
    rc = NOERROR;
  }

  NSFNoteClose(hNote);

  return (rc);

}
