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

    PROGRAM:    makenav

    FILE:       makenav.c

    SYNTAX:     makenav

    PURPOSE:    Create a navigator view note via the API.  

      This program creates a navigator view note in the Make Navigator
      sample database. Each time the program is run, a new navigator 
      layout item is added to the navigator view.  Each layout item
      contains an instance of an ellipse that acts as a doclink to
      the (first) document that was created in the database today.

      This program attempts to service the scenario where a document 
      is added (by someone or some program) to the database once a
      day.  If run at the end of the day, this program will add a 
      a new navigator graphic, which links to   the new document, to 
      navigator view.

    DESCRIPTION:
   
      The database file, makenav.nsf, is opened and a sample navigator 
      view note is created if it does not already exist.  The view's
      NOTE_CLASS is set to NOTE_CLASS_VIEW, the title is set to 
      "Sample Navigator", and the design flags are set to 
      DESIGN_FLAG_VIEWMAP | DESIGN_FLAG_HIDE_FROM_V3.

      The MainView of the database is searched for the first document that
      was created on the current date.  The program exits if there is no
      match, else the NOTELINK information for the view and document is 
      determined.

      The navigator view note is scanned to determine the number of 
      navigator layout items exist, if any.  This is done to intelligently
      locate the new layout item graphic in a 'free' area, and to give it 
      a unique instance and label name.
      
      The navigator layout item, $ViewMapLayout, is added to the navigator
      view note by first writing the appropriate information for the following
      three CD records that make up the navigator instance;
         VIEWMAP_HEADER_RECORD - only one per navigator view 
         VIEWMAP_RECT_RECORD   - unique for each navigator graphic (ellipse)  
         VIEWMAP_ACTION_RECORD - and action (doclink to today's document)

      Then, the complete item is appended to the navigator view note, and the 
      note is updated (stored in the database) and the database is closed.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

#ifdef __cplusplus
extern "C" {
#endif
/*
 * C header files
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
 * HCL C API for Notes/Domino Header files.
 */


#include <global.h>
#include <stdnames.h>
#include <osmem.h>
#include <ostime.h>
#include <nsfdb.h>
#include <nsfsearc.h>
#include <nsferr.h>
#include <nif.h>
#include <vmods.h>
#include <viewfmt.h>
#include <colorid.h>
#include <osmisc.h>
#include "../../printLog.h"

#ifdef OS390
#include <_Ascii_a.h>  /* NOTE: must be the LAST file included */
#include <lapicinc.h>
#endif

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/* 
 * Navigator Layout item count.
 */
WORD        wLayoutCount;

/* 
 * Navigator Action doclink information.
 */
NOTEID         DocNoteID;
ORIGINATORID   DocNoteOID;
WORD           bDocNoteFound;

/*
 * NSFSEARCHPROC and NSFITEMSCANPROC. 
 */
STATUS LNPUBLIC FindFirstNote(void *, SEARCH_MATCH *, ITEM_TABLE * );
STATUS LNPUBLIC FindLayoutCount(WORD, WORD, char *, WORD, VOID *, DWORD, VOID * );

/************************************************************************

    FUNCTION:   main

*************************************************************************/
int main(int argc, char *argv[])
{
    STATUS        sError = NOERROR;
    WORD          ClassView = NOTE_CLASS_VIEW;

    DBHANDLE      hDB;           /* sample database handle */
    NOTEHANDLE    hViewNote;     /* sample navigator view handle */
    NOTEID        ViewNoteID;    /* and note id                  */

    char          szFileName[] = "makenav.nsf";         /* sample database to be used */
    char          szMainView[] = "MainView";            /* title of view to be used */
    char          szNavViewName[] = "Sample Navigator"; /* title of navigator view */

/*
 * Variables pertaining to the Navigator Layout ($ViewMapLayout) item and 
 * relevant structures.
 */
    VIEWMAP_HEADER_RECORD  NavHeader;
    VIEWMAP_RECT_RECORD    NavRect;
    VIEWMAP_ACTION_RECORD  NavAction;

    WORD          wNavLayoutBufLen;
    DHANDLE       hNavLayoutBuffer;
    char          *pNavLayoutBuffer;
    char          *pNLBuf;

    char          szNavName[]  = "DocLink(MM/DD/YYYY)"; /* Navigator name placeholder */
    char          szNavLabel[] = "MM/DD/YYYY";          /* Navigator label placeholder */
    WORD          wNavNameLen  = strlen(szNavName);
    WORD          wNavLabelLen = strlen(szNavLabel);

    TIMEDATE      CurrTimeDate;  /* Current TimeDate */
    TIMEDATE      YesterTimeDate;   /* Current TimeDate less 1 day */
    TIMEDATE      CurrDate;      /* Current (extracted) date */
    TFMT          DateFormat;
    FONTIDFIELDS  *font;

/*
 * NOTELINK variables.
 */
    DBREPLICAINFO DBReplica;
    NOTEHANDLE    hMainView;  
    NOTEID        MainViewID; 
    ORIGINATORID  ViewOID;
    UNID          ViewUNID;
    UNID          DocNoteUNID;

/* 
 * Design Flags ($Flags) item variable.
 */
   char           szDFlags[3];

/* 
 **************************************************************************
 * Start program.
 **************************************************************************
 */         
    PRINTLOG("HCL Notes API\nmakenav sample program\n");

    if (sError = NotesInitExtended (argc, argv))
    {
       PRINTLOG("\n Unable to initialize Notes.\n");
       return (1);
    }

   
/*
 * Open database.
 */
    if (sError = NSFDbOpen( szFileName, &hDB ))
    {
       PRINTLOG("Error: unable to open database '%s'.\n", szFileName );
       goto Exit1;
    }

/*
 * Check to see if the Navigator view already exits.
 * If so, get a handle to the view.
 * If not, create a new note and append design flags 
 */
    if ((sError = NIFFindView(hDB, szNavViewName, &ViewNoteID )) == NOERROR)
    {
       if (sError = NSFNoteOpen(hDB, ViewNoteID, 0, &hViewNote ))
       {
           PRINTLOG("Error: unable to open navigator view note in database.\n");
           goto Exit2;
       }
    }

/*
 * Navigator does not exist, create the note in database.
 */
    else
    {
       sError = NSFNoteCreate( hDB, &hViewNote );
       if (sError) 
       {
           PRINTLOG("Error: unable to create note in database.\n" );
           goto Exit2;
       } 
    
/*
 * Set the NOTE_CLASS to NOTE_CLASS_VIEW.
 */
       NSFNoteSetInfo( hViewNote, _NOTE_CLASS, &ClassView );

/*
 * Set the view name. 
 */
       sError = NSFItemSetText( hViewNote, VIEW_TITLE_ITEM,
                                szNavViewName, MAXWORD );
       if (sError)
       {
           PRINTLOG("Error: unable to set text item '%s' in view note.\n",
                    VIEW_TITLE_ITEM );
           goto Exit3;
       }

/*
 * Append Design flags for the Navigator view.
 */
       szDFlags[0] = DESIGN_FLAG_VIEWMAP;
       szDFlags[1] = DESIGN_FLAG_HIDE_FROM_V3;
       szDFlags[2] = '\0';
       sError = NSFItemAppend(  hViewNote,
                                ITEM_SUMMARY,
                                DESIGN_FLAGS,
                                (WORD)strlen(DESIGN_FLAGS),
                                TYPE_TEXT,
                                szDFlags,
                                (DWORD)strlen(szDFlags ));
       if (sError) 
       {
           PRINTLOG("Error: unable to append item '%s' to view note.\n",
                    DESIGN_FLAGS );
           goto Exit3;
       }
    }

/* 
 * Get current TIMEDATE, extract of the DATE portion (the navigator label), and adjust 
 * date to yesterday for efficient (subsequent) search of data notes.
 */
    OSCurrentTIMEDATE(&CurrTimeDate );

    TimeExtractLocal(&CurrTimeDate, FALSE, &CurrDate );
    DateFormat.Date = TDFMT_FULL;
    DateFormat.Structure = TSFMT_DATE;
    ConvertTIMEDATEToText(NULL, &DateFormat, &CurrDate, szNavLabel, 
                          wNavLabelLen, &wNavLabelLen);
    szNavLabel[wNavLabelLen] = '\0';                         

    YesterTimeDate = CurrTimeDate;
    TimeDateAdjust(&YesterTimeDate, 0, 0, 0, -1, 0, 0);

/*
 * Search the view for today's first document note, and set the global ID as appropriate
 */ 
    bDocNoteFound = FALSE;

    if (sError = NSFSearch (
                            hDB,            /* database handle */
                            NULLHANDLE,        /* selection formula (all) */
                            szMainView,        /* title of view in selection formula */
                            0,                 /* search flags */
                            NOTE_CLASS_DOCUMENT,   /* note class to find */
                            &YesterTimeDate,   /* starting date (unused) */
                            FindFirstNote,      /* action routine for notes found */
                            &hDB,           /* argument to action routine */
                            NULL))              /* returned ending date (unused) */
    {
       PRINTLOG("Error: database search failed.\n");
       goto Exit3;
    }

/*
 * Exit if no document notes were available for today
 */
    if (bDocNoteFound == FALSE)
    {
       PRINTLOG("No new documents today. Navigator creation cancelled.\n" );
       goto Exit3;
    }

/* 
 * Retrieve the DBID, and the Note OIDs for both the view and document notes, 
 * and create the UNID for the notes.
 * These will be used by the NOTELINK for the Navigator action item.
 */
    if (sError = NSFDbReplicaInfoGet (hDB, &DBReplica))
    {
       PRINTLOG("Error: could not retrieve DBID.\n");
       goto Exit3;
    }

    if ((sError = NIFFindView(hDB, szMainView, &MainViewID )) == NOERROR)
    {
       if (sError = NSFNoteOpen(hDB, MainViewID, 0, &hMainView ))
       {
           PRINTLOG("Error: unable to open MainView note in database.\n");
           goto Exit3;
       }
    }
    else
    {
       PRINTLOG("Error: unable to find MainView note in database.\n");
       goto Exit3;
    }
    NSFNoteGetInfo (hMainView, _NOTE_OID, &ViewOID);
    ViewUNID.File = ViewOID.File;
    ViewUNID.Note = ViewOID.Note;

    DocNoteUNID.File = DocNoteOID.File;    /* DocNoteOID set by SEARCHPROC */
    DocNoteUNID.Note = DocNoteOID.Note;

/*
 * Scan navigator and determine if/how many layout items exist.  This value
 * is used to programatically place the new draw object in a "free" UI area.
 */
    wLayoutCount = 0;
    sError = NSFItemScan(hViewNote, FindLayoutCount, NULL );
    if ((sError) && (sError != ERR_ITEM_NOT_FOUND )) 
    {
       PRINTLOG("Error: unable to scan '%s' view note.\n", VIEWMAP_LAYOUT_ITEM );
       goto Exit3;
    }

/*
 * Create the ViewMap (Navigator) Layout item. The Navigator Layout item is an
 * item of TYPE_COMPOSITE with name VIEWMAP_LAYOUT_ITEM ($ViewMapLayout). 
 *
 * The Navigator Layout item for this view will consist of the following 
 * series of structures converted to Domino and Notes canonical format and  
 * packed together:
 *
 *          VIEWMAP_HEADER_RECORD     ---> only one per navigator view
 *          VIEWMAP_RECT_RECORD       ---\ paired for each navigator
 *          VIEWMAP_ACTION_RECORD     ---/ object within the view
 *
 * First, allocate a buffer, pNavLayoutBuffer, that will contain the 
 * entire Navigator Layout (VIEWMAP_LAYOUT_ITEM).  
 *
 * The Navigator type record (in this case an ellipse) packs the name and 
 * label strings and the Navigator action record (in this case a doclink)
 * packs a relevant name. Since the Navigator action for this sample is a
 * doclink, an action name does not apply since the NOTELINK of the doc is
 * contained within the record itself.
 */
   wNavNameLen += (wNavNameLen%2);     /* even byte boundaries */
   wNavLabelLen += (wNavLabelLen%2);   /* for packed data */

 /* 
  * If first navigator layout, then include header record in buffer length. 
  */
    if (wLayoutCount == 0 )
       wNavLayoutBufLen =  ODSLength( _VIEWMAP_HEADER_RECORD ) +
                           ODSLength( _VIEWMAP_RECT_RECORD )   +
                           wNavNameLen                         +
                           wNavLabelLen                        +
                           ODSLength( _VIEWMAP_ACTION_RECORD );   
    else
       wNavLayoutBufLen =  ODSLength( _VIEWMAP_RECT_RECORD )   +
                           wNavNameLen                         +
                           wNavLabelLen                        +
                           ODSLength( _VIEWMAP_ACTION_RECORD );   

/* 
 * Allocate and initialize the CD buffer for the entire $ViewMapLayout item.
 */
    if (sError = OSMemAlloc( 0, wNavLayoutBufLen, &hNavLayoutBuffer ))
    {
       PRINTLOG("Error: unable to allocate %d bytes memory.\n", 
                wNavLayoutBufLen);
       goto Exit3;
   }

   pNavLayoutBuffer = (char*) OSLockObject( hNavLayoutBuffer );
   memset( pNavLayoutBuffer, 0, (size_t) wNavLayoutBufLen );

/* 
 * Initialize pNLBuf. pNavLayoutBuffer will remain pointing to the top 
 * of the buffer. pNLBuf will move to point to the next available byte.
 */
    pNLBuf = pNavLayoutBuffer;

/*
 * If first navigator layout, then create the VIEWMAP_HEADER_RECORD structure.
 */
    if (wLayoutCount == 0 )
    {
       NavHeader.Header.Signature = SIG_CD_VMHEADER;
       NavHeader.Header.Length = (BYTE) ODSLength(_VIEWMAP_HEADER_RECORD );
       NavHeader.Version = VIEWMAP_VERSION;
       NavHeader.NameLen = 0;

       /*
        * Call ODSWriteMemory to convert the VIEWMAP_HEADER_RECORD structure from
        * host-specific format to Domino and Notes canonical format, and copy it into the 
        * buffer at location pNLBuf. ODSWriteMemory increments pNLBuf to point
        * to the next byte in the buffer after the written data structure.
        */
       ODSWriteMemory( &pNLBuf, _VIEWMAP_HEADER_RECORD, &NavHeader, 1 );

   }
          
/*
 *  Create the VIEWMAP_RECT_RECORD structure.
 */
    memset(&NavRect, 0, (size_t) sizeof(VIEWMAP_RECT_RECORD));
    NavRect.DRobj.Header.Signature = SIG_CD_VMELLIPSE;
    NavRect.DRobj.Header.Length = (BYTE) (ODSLength(_VIEWMAP_RECT_RECORD) + 
                           wNavNameLen + wNavLabelLen );

 /* Draw ellipse in 'free' area based on existing count. */
    NavRect.DRobj.ObjRect.top = (65L*wLayoutCount ) + 35L;        
    NavRect.DRobj.ObjRect.bottom = (65L*wLayoutCount) + 116L;
    NavRect.DRobj.ObjRect.left = 30L;
    NavRect.DRobj.ObjRect.right = 122L;
    NavRect.DRobj.flags = VM_DROBJ_FLAGS_VISIBLE | VM_DROBJ_FLAGS_SELECTABLE;                            
    NavRect.DRobj.NameLen = wNavNameLen;
    NavRect.DRobj.LabelLen = wNavLabelLen;
    font = (FONTIDFIELDS *) &(NavRect.DRobj.FontID);
    font->Face = FONT_FACE_SWISS;
    font->Attrib = 0;
    font->Color = NOTES_COLOR_BLACK;
    font->PointSize = 10;
    NavRect.DRobj.TextColor = NOTES_COLOR_BLACK;
    NavRect.DRobj.Alignment = 0;
    NavRect.DRobj.bWrap = FALSE;

    NavRect.LineColor = NOTES_COLOR_BLACK;
    NavRect.FillFGColor = NOTES_COLOR_CYAN;
    NavRect.FillBGColor = NOTES_COLOR_CYAN;
    NavRect.LineStyle = VM_LINE_SOLID;
    NavRect.LineWidth = 1;
    NavRect.FillStyle = VM_FILL_SOLID;

/*
 * Convert the Navigator Rectangle record structure to Domino and Notes canonical
 * format and append it to the buffer. This increments pNLBuf to point to
 * the next byte in the buffer after the Navigator Rectangle.
 */
    ODSWriteMemory( &pNLBuf, _VIEWMAP_RECT_RECORD, &NavRect, 1 );

/* 
 * Now append the Ellipse instance and label names to the buffer.
 */
    sprintf(szNavName, "DocLink(%s)", szNavLabel);             
    memcpy(pNLBuf, (void *)szNavName, (size_t)wNavNameLen );
    pNLBuf += wNavNameLen;
    memcpy(pNLBuf, (void *)szNavLabel, (size_t)wNavLabelLen ); 
    pNLBuf += wNavLabelLen;

/*
 *  Create the VIEWMAP_ACTION_RECORD structure.
 */
    memset(&NavAction, 0, (size_t) sizeof(VIEWMAP_ACTION_RECORD));
    NavAction.Header.Signature = (WORD)SIG_CD_VMACTION;
    NavAction.Header.Length = ODSLength(_VIEWMAP_ACTION_RECORD);
    NavAction.bHighlightTouch = FALSE;
    NavAction.bHighlightCurrent = TRUE;
    NavAction.HLOutlineColor = NOTES_COLOR_RED;
    NavAction.HLFillColor = NOTES_COLOR_YELLOW;
    NavAction.ClickAction = VM_ACTION_GOTO_LINK;
    NavAction.ActionStringLen = 0;           /* N/A for link action */
    NavAction.HLOutlineWidth = 2;
    NavAction.HLOutlineStyle= VM_LINE_SOLID;

    NavAction.LinkInfo.File = DBReplica.ID;
    memmove(&NavAction.LinkInfo.View, &ViewUNID, sizeof(UNID));
    memmove(&NavAction.LinkInfo.Note, &DocNoteUNID, sizeof(UNID));

    NavAction.ExtDataLen = 0;                   /* N/A for link action */
    NavAction.ActionDataDesignType = 0;          /* N/A for link action */

/*
 * Convert the Navigator Action record structure to Domino and Notes canonical
 * format and append it to the buffer. This increments pNLBuf to point to
 * the next byte in the buffer after the Navigator Action.
 */
    ODSWriteMemory( &pNLBuf, _VIEWMAP_ACTION_RECORD, &NavAction, 1 );

/*
 * Now append the Navigator Layout item to the note.
 */
    sError = NSFItemAppend( hViewNote,
                            ITEM_SUMMARY,
                            VIEWMAP_LAYOUT_ITEM,
                            (WORD)strlen(VIEWMAP_LAYOUT_ITEM),
                            TYPE_VIEWMAP_LAYOUT,
                            pNavLayoutBuffer,
                            (DWORD)wNavLayoutBufLen );
    OSUnlockObject( hNavLayoutBuffer );
    OSMemFree( hNavLayoutBuffer );
    if (sError) 
    {
        PRINTLOG("Error: unable to append item '%s' to view note.\n", 
                VIEWMAP_LAYOUT_ITEM );
        goto Exit3;
    }

/*
 *  Done constructing the view note. Now store the view note
 *  in the database.
 */
    sError = NSFNoteUpdate( hViewNote, 0 );
                   
    if (sError)
    {
        PRINTLOG("Error: unable to update view note in database.\n" );
    }
    else
    {
        PRINTLOG("Successfully created view note in database.\n" );
    }


Exit3:
    
    NSFNoteClose( hViewNote );

Exit2:    

    NSFDbClose( hDB );

Exit1:

    PRINTERROR (sError,"NSFNoteUpdate");
    NotesTerm();
    return (sError); 

}

/************************************************************************

    FUNCTION:   FindFirstNote

    PURPOSE:    This routine is called by NSFSearch for each note that 
                matches the selection criteria (in this case any new data 
                created over the last day).  

            The routine finds the first document that was created today
            and assigns its NOTEID to the global DocNoteID.

    INPUTS:
        The first argument to this function is the optional argument
        that we supplied when we called NSFSearch.

        The second argument is supplied by NSFSearch.    It is
        a structure of information about the note that was found.

        The third argument is also supplied by NSFSearch and is
        the summary buffer for this note. 


*************************************************************************/

STATUS LNPUBLIC FindFirstNote ( VOID *hDB,
                                SEARCH_MATCH *search_info,
                                ITEM_TABLE *summary_info)
{

/* Local data declarations. */
    SEARCH_MATCH    SearchMatch;             /* local copy of search match */  
    STATUS          error;
    NOTEHANDLE      hNoteHandle;
    TIMEDATE        CurDate;
    TIMEDATE        ModDate;

    memcpy ((char*)(&SearchMatch), (char *)search_info, sizeof(SEARCH_MATCH));

    /*
     * Skip this note if it does not really match the search criteria (it is
     * now deleted or modified), or if a matching note has been found.
     */
    if (!(SearchMatch.SERetFlags & SE_FMATCH ))
       return (NOERROR);

    if (bDocNoteFound ) 
       return (NOERROR);
      
    /* Open the note. */
    if (error = NSFNoteOpen(
                            *(DBHANDLE *)hDB,            /* database handle */
                            SearchMatch.ID.NoteID,       /* note ID */
                            0,                           /* open flags */
                            &hNoteHandle) )              /* note handle (return) */
       
        return (ERR(error));

    /* Get Modified time and compare it to today's date. */
    OSCurrentTIMEDATE(&CurDate );
    NSFNoteGetInfo(hNoteHandle, _NOTE_MODIFIED, &ModDate );

    /* Extract out date only. */
    TimeExtractLocal(&CurDate, FALSE, &CurDate );
    TimeExtractLocal(&ModDate, FALSE, &ModDate );
    
    /* If date compares, then set global flag, note id, and note OID. */
    if (TimeDateEqual(&CurDate, &ModDate ))
    {
       bDocNoteFound = TRUE;
       DocNoteID = SearchMatch.ID.NoteID;
       NSFNoteGetInfo(hNoteHandle, _NOTE_OID, &DocNoteOID );
    }

    /* Close the note. */
    if (error = NSFNoteClose(hNoteHandle ))
       return (ERR(error));

    /* End of subroutine. */
    return (NOERROR);
}

/************************************************************************

    FUNCTION:   FindLayoutCount

    PURPOSE:    This is the action routine that is called by NSFItemScan 
                for each field in the note.

            The routine determines the number of Navigator Layout items
            are contained by the navigator view note.

*************************************************************************/
STATUS LNPUBLIC FindLayoutCount (   WORD  wUnused,
                           WORD  wItemFlags,
                              char  *NamePtr,
                              WORD  wNameLen,
                              VOID  *ItemValue,
                              DWORD dwItemValueLen,
                              VOID  *Unused )
{
    char    szItemName[32];      /* local copy of item name. */

/* Copy the null-terminated item name into our local data area. */
    strncpy(szItemName, NamePtr, wNameLen );
    szItemName[wNameLen] = '\0';

/* 
 * Determine if item is a Navigator Layout (VIEWMAP_LAYOUT_ITEM);
 * if it is, then increment global count. 
 */
    if (!strcmp(szItemName, VIEWMAP_LAYOUT_ITEM ))
       wLayoutCount++;

   return (NOERROR);
}
#ifdef __cplusplus
}
#endif
