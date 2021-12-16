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

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/* Forward declarations */
STATUS LNPUBLIC ProcessOneNote(
  void far *         phDB,
  SEARCH_MATCH far * pSearchMatch,
  ITEM_TABLE far *   Unused);

void PrintAPIError (STATUS);

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
    printf("NotesInitExtended failed...\n");
    return(1);
  }

  if (rc = NSFDbOpen(szDbName, &hDb))
  {
    printf("NSFDbOpen failed...\n");
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
    printf("NSFFormulaCompile failed...\n");
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
     PrintAPIError (rc);

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
  DHANDLE        hszAttachmentName = NULLHANDLE;
  char *       pszAttachmentName = NULL;

  memcpy((char *)&SearchMatch, (char *)pSearchMatch, sizeof(SEARCH_MATCH));

  if (!(SearchMatch.SERetFlags & SE_FMATCH))
    return (NOERROR);

  hDB = *((DBHANDLE far *)phDB);

  if (rc = NSFNoteOpen(hDB, SearchMatch.ID.NoteID, 0, &hNote))
  {
    printf("NSFNoteOpen failed...\n");
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
    printf("NSFNoteOLEGetAssociateFileOnObject dwId = %d Attachment Name = %s\n", dwId, pszAttachmentName);
    dwId++;
    OSUnlockObject(hszAttachmentName);
    OSMemFree(hszAttachmentName);
  }

  if (rc != ERR_ARGS_INVALID)
    printf("NSFNoteOLEGetAssociateFileOnObject failed...\n");
  else
    rc = NOERROR;

  NSFNoteClose(hNote);

  return (rc);

}


/*************************************************************************

    FUNCTION:   PrintAPIError

    PURPOSE:    This function prints the HCL C API for Notes/Domino
		error message associated with an error code.

**************************************************************************/

void PrintAPIError (STATUS api_error)

{
    STATUS  string_id = ERR(api_error);
    char    error_text[200];
    WORD    text_len;

    /* Get the message for this HCL C API for Notes/Domino error code
       from the resource string table. */

    text_len = OSLoadString (NULLHANDLE,
                             string_id,
                             error_text,
                             sizeof(error_text));

    /* Print it. */
    fprintf (stderr, "\n%s\n", error_text);

}

