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

    PROGRAM:    xview

    FILE:       xview.c

    PURPOSE:    Sample View Level Export Library.
                Creates an ASCII file from documents selected
                in a Domino and Notes view.

    SYNTAX:     Invoked from the Notes user interface.

    DESCRIPTION:
    A Domino and Notes user opens a database from the Domino and Notes 
    user interface. From the view window, user selects one or more 
    documents by mouse clicks (check marks appear in left column of view).
    Menu command File - Export brings up the Export dialog box.
    Select Save File as Type "Sample View Export" and specify an
    output file to create. Domino and Notes loads this DLL and calls the
    function ExportView() in this file.  This displays a dialog
    box prompting user for some static text and the names of two
    TEXT items in the documents. Then it creates an ASCII file that
    contains, for each note selected in the view, the static text
    and the 2 TEXT items specified.

    One technically interesting feature of XVIEW is that it uses
    the ID table specified by the <hSelectedList> member of the
    VIEWIXDATA structure. This ID table identifies which notes in
    the view were selected. Domino and Notes passes a structure of type 
    VIEWIXDATA (see ixview.h) to the view-level export DLL. This structure 
    contains information about the view that was open before File - Export 
    was selected. See procedure ProcessView() for details.

*************************************************************************/

/* OS-specific includes */
#if defined (W32)
#include <windows.h>
#endif

/* C includes */
#include <string.h>
#include <stdio.h>
#include <sys\types.h>
#include <sys\stat.h>

/* Domino and Notes includes */
#include <global.h>
#include <ossignal.h>                   /* OSSIGMSGPROC */
#include <oserr.h>                      /* ERR_FILES, etc. */
#include <osmisc.h>                     /* OSLoadString */
#include <osfile.h>
#include <ixport.h>
#include <ixview.h>                     /* VIEWIXDATA */
#include <idtable.h>                    /* IDScan */

/* Local includes */
#include "xview.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif


/* Globals */
char         ItemBuffer[MAX_ITEM_SIZE];
char         String1[LINEOTEXT];
char         String2[LINEOTEXT];
char         String3[LINEOTEXT];
char         ErrorMessage[LINEOTEXT];
HWND         hWnd_Parent;
DHANDLE        hLib_Instance;
OSSIGMSGPROC MessageProc;           /* initialized in ExportView() */
FILE        *FStream = (__FILE *)0; /* initialized in ProcessView() */

/* Function prototypes */
STATUS LNPUBLIC ExportView(
                           VIEWIXDATA *ViewData,
                           WORD Flags,
                           HMODULE hModule,
                           char *AltLibraryName,
                           char *FileName);

STATUS LNPUBLIC ProcessView(
                            VIEWIXDATA *ViewData,
                            WORD Flags,
                            HMODULE hModule,
                            char *FileName,
                            char *szStaticText,
                            char *ItemName1,
                            char *ItemName2);

STATUS LNPUBLIC ReadNote (
                          DBHANDLE DbHandle,
                          NOTEID theNoteID,
                          HMODULE hModule,
                          char *szStaticText,
                          char *szItemName1,
                          char *szItemName2);

void Log(
    char *szBuffer);

STATUS MapError(
                WORD wDosCode);

#if defined (W32)
BOOL LNPUBLIC XViewDlgProc(
                           HWND hDlg,
                           UINT message,
                           UINT wParam,
                           LONG lParam);
#endif

/************************************************************************

    FUNCTION:   ExportView

    PURPOSE:    Pop up dialog box. Then export selected docs to file.
    
    DESCRIPTION:

        Domino and Notes calls this function when the Notes user selects the 
        file type corresponding to this DLL from the "Export" dialog box in the 
        Domino and Notes user interface. Domino and Notes doesn't know this 
        function's name. Notes just calls the first function exported by this 
        DLL. See .DEF file. This function must conform to the calling syntax of 
        Domino and Notes export DLLs.
    
    ALGORITHM:
        1.  Prompt user for Static text to print for each document, and 
            names of 2 TYPE_TEXT items in each note to export.          
        2.  Copy user input into local variables                        
        3.  Call procedure ProcessView                                  
								
    INPUTS:
    
        VIEWIXDATA *ViewData - Domino and Notes initializes this data structure
        before passing it as input to this routine. This data
        structre provides detailed information about the view the
        Domino and Notes user was in when she invoked the export command.
    								
    OUTPUTS:
        this creates an ASCII text file with the path and name
        specified by the user in the Export dialog box.
									
*************************************************************************/

STATUS LNPUBLIC  ExportView(
VIEWIXDATA *ViewData,
WORD        Flags,
HMODULE     hModule,
char       *AltLibraryName,
char       *FileName)
{
    STATUS    error = NOERROR;
    char      szStaticText[LINEOTEXT];
    char      szItemName1[LINEOTEXT];
    char      szItemName2[LINEOTEXT];
#if defined (W32)
    FARPROC   lpfnDlgProc;
#endif

    /* Initialize global data */
    MessageProc = (OSSIGMSGPROC) OSGetSignalHandler(OS_SIGNAL_MESSAGE);

  /* Prompt user for Static text to print for each document, and names of 
     2 TYPE_TEXT items in each note to export. */
    strcpy(String1, "Static Text for each exported note:");
    strcpy(String2, "First Text Field Name:");
    strcpy(String3, "Second Text Field Name:");

#if defined (W32)

    lpfnDlgProc = MakeProcInstance(XViewDlgProc, hLib_Instance);
    DialogBox (hLib_Instance,
               MAKEINTRESOURCE(IDD_XVIEW),
               GetFocus(),
               (DLGPROC)lpfnDlgProc);
    FreeProcInstance ((DLGPROC)lpfnDlgProc);

#endif

    /* Copy user input into local variables szStaticText, etc. */
    strcpy(szStaticText, String1);
    strcpy(szItemName1,  String2);
    strcpy(szItemName2,  String3);

    if (strlen(szItemName1) == 0)
    {
        if(!OSLoadString(hModule, ERR_XVIEW_NULLITEMNAME,
           ErrorMessage, LINEOTEXT-1))
            sprintf(ErrorMessage, "OSLoadString() ERROR\n");

        (*MessageProc) (ErrorMessage, OSMESSAGETYPE_OK);

        return (error);
    }

    /* Call ProcessView to extract data and create output file. */
    if (error = ProcessView(ViewData,
                            Flags,
                            hModule, 
                            FileName,
                            szStaticText,
                            szItemName1,
                            szItemName2))
    {
        return(error);
    }
    return(NOERROR);
}

/************************************************************************

    FUNCTION:   ProcessView

    PURPOSE:
    Open the ASCII output file. Loop through all Note IDs listed
    in the hSelectedList of the VIEWIXDATA. For each ID, call
    procedure ReadNote to output the specified items from each
    note to the ASCII output file. Close the ASCII output file and
    return.

*************************************************************************/
 
STATUS LNPUBLIC ProcessView(
VIEWIXDATA *ViewData,
WORD        Flags,
HMODULE     hModule,
char       *FileName, 
char       *szStaticText,
char       *szItemName1,
char       *szItemName2)
{                    
    STATUS   error = NOERROR;
    DHANDLE    hSelectedList;  /* handle to ID table of Selected notes */
    DBHANDLE DbHandle;
    DWORD    notes_scanned;  /* used in procedure IDScan() */
    NOTEID   note_id;        /* gets NOTEIDs stored in ID table */
    
    /* Open ASCII export file */
    FStream = __OpenFile(FileName,
                         ((Flags & IXFLAG_APPEND)?FMODE_APPEND:FMODE_WRITE));
    if (!FStream)  /* error opening file */
    {
        if(!OSLoadString(hModule, ERR_XVIEW_OPENFILE,
           ErrorMessage, LINEOTEXT-1))
            sprintf(ErrorMessage, "OSLoadString() ERROR\n");

    (*MessageProc) (ErrorMessage, OSMESSAGETYPE_OK);

    return (ERR_XVIEW_OPENFILE);
    }
    
    hSelectedList = ViewData->hSelectedList;
    if (hSelectedList == NULLHANDLE)
    {
        if (!OSLoadString(hModule, ERR_XVIEW_NOSELECTED,
            ErrorMessage, LINEOTEXT-1))
            sprintf(ErrorMessage, "OSLoadString() ERROR\n");

        (*MessageProc) (ErrorMessage, OSMESSAGETYPE_OK);

        return (NOERROR);
    }

    DbHandle = ViewData->hNoteFile;
    if (DbHandle == NULLHANDLE)
    {
        if (!OSLoadString(hModule, ERR_XVIEW_NULLDBHANDLE,
            ErrorMessage, LINEOTEXT-1))
            sprintf(ErrorMessage, "OSLoadString() ERROR\n");

        (*MessageProc) (ErrorMessage, OSMESSAGETYPE_OK);

        return (NOERROR);
    }

    /* Loop through all Note IDs stored in the hSelectedList ID table 
       and call procedure ReadNote to print out each one to the output file. 
     */
    notes_scanned = 0L;
    while(IDScan(hSelectedList, (FLAG)(notes_scanned++==0L), &note_id))
    {
        if (error = ReadNote(DbHandle, note_id, hModule, szStaticText,
                             szItemName1, szItemName2))
            return(error);
    }

    /* Done exporting selected notes to ASCII file. Close ASCII file. */
    if (FStream)
    {
        __CloseFile(FStream);
        FStream = (__FILE *)0;
    }

    return(error);
}

/************************************************************************

    FUNCTION:   ReadNote

    PURPOSE:    Output staic text and 2 field of data from one note
		to ASCII file.

    ALGORITHM:  Open note. Print static text to ASCII output file.
		Get the TYPE_TEXT items from the note.  Print items to
		the ASCII output file. Close the note.

*************************************************************************/

STATUS LNPUBLIC ReadNote (
DBHANDLE    DbHandle,
NOTEID      theNoteID,
HMODULE     hModule,
char       *szStaticText,
char       *szItemName1,
char       *szItemName2)
{
    NOTEHANDLE NoteHandle;
    STATUS     error = NOERROR;
    WORD       item_len;
    
    /* Open note */
    if (error = NSFNoteOpen (DbHandle, theNoteID, 0, &NoteHandle))
    {   
        if (!OSLoadString(hModule, ERR_XVIEW_NSFNOTEOPEN,
            ErrorMessage, LINEOTEXT-1))
            sprintf(ErrorMessage, "OSLoadString() ERROR\n");

        (*MessageProc) (ErrorMessage, OSMESSAGETYPE_OK);

        return(error);
    }

    /*  Format line of static text and write it to the ASCII output file  */
    sprintf(ItemBuffer, "\n%s\n", szStaticText);
    Log(ItemBuffer);

    /* Get item #1 from note assuming it is of TYPE_TEXT.
       Format it and write to the ASCII output file.
    */
    if (NSFItemIsPresent(NoteHandle, szItemName1, (WORD)strlen(szItemName1)))
    {
        item_len = NSFItemGetText(NoteHandle, szItemName1, ItemBuffer,
                                  MAX_ITEM_SIZE-1);
        strcat(ItemBuffer, "\n");
    }
    else    /* item not found */
        sprintf(ItemBuffer, "Item '%s' not found in note.\n", szItemName1);

    Log(ItemBuffer);

    /* Get item #2 from note assuming it is of TYPE_TEXT.
       Format it and write to the ASCII output file.
    */
    if (NSFItemIsPresent(NoteHandle, szItemName2, (WORD)strlen(szItemName2)))
    {
        item_len = NSFItemGetText(NoteHandle, szItemName2, ItemBuffer,
                   MAX_ITEM_SIZE-1);
        strcat(ItemBuffer, "\n");
    }
    else    /* item not found */
        sprintf(ItemBuffer, "Item '%s' not found in note.\n", szItemName2);

    Log(ItemBuffer);

    /* Close note */

    if (error = NSFNoteClose (NoteHandle))
    {
        if (!OSLoadString(hModule, ERR_XVIEW_NSFNOTECLOSE,
            ErrorMessage, LINEOTEXT-1))
            sprintf(ErrorMessage, "OSLoadString() ERROR\n");

        (*MessageProc) (ErrorMessage, OSMESSAGETYPE_OK);

        return(error);
    }

    return(NOERROR);
}

/************************************************************************

    FUNCTION:   Log

    PURPOSE:    Write a line of text to global FStream.

*************************************************************************/
void Log(
char *szBuffer)
{
    if (FStream)
    {
        __WriteFile(szBuffer, FStream);
        if (__FileError(FStream))
        {
            __CloseFile(FStream);
            FStream = (__FILE *)0;
        }
    }
}

/************************************************************************

    FUNCTION:   MapError

    PURPOSE:    Map MS-DOS Extended Error Codes to Domino and Notes 
	            GLOBERR.H file errors

*************************************************************************/

STATUS        MapError(WORD wDosCode)
{
  STATUS  NotesErr = NOERROR;

    switch (wDosCode)
    {
        case 0x02: NotesErr = ERR_NOEXIST;                 break;
        case 0x03: NotesErr = ERR_NO_MORE_FILES;           break;
        case 0x04: NotesErr = ERR_FILES;                   break;
        case 0x05: NotesErr = ERR_LOCK;                    break;
        case 0x06: NotesErr = ERR_FD;                      break;
        case 0x0F: NotesErr = ERR_INVALID_DRIVE;           break;
        case 0x13: NotesErr = ERR_PROTECTED;               break;
        case 0x1D: NotesErr = ERR_IOERROR;                 break;
        case 0x1E: NotesErr = ERR_IOERROR;                 break;
        case 0x20: NotesErr = ERR_LOCK; /* same as 0x05 */ break;
        case 0x21: NotesErr = ERR_LOCKS;                   break;
        default:   NotesErr = ERR_UNKNOWN;                 break;
    }
    return(NotesErr);
}

#if defined (W32)
/************************************************************************

    FUNCTION: XViewDlgProc(HWND, UINT, UINT, LONG)

    PURPOSE:  Processes messages for "XView" dialog box

    MESSAGES:

      WM_INITDIALOG - initialize dialog box
      WM_COMMAND    - Input received

*************************************************************************/

BOOL LNPUBLIC XViewDlgProc(
HWND hDlg,
UINT message,
UINT wParam,
LONG lParam)
{
    switch (message)
    {
        case WM_INITDIALOG:
            SetDlgItemText(hDlg, IDD_PROMPT3, String3);
            SendDlgItemMessage(hDlg, IDD_STRING3, EM_LIMITTEXT, LINEOTEXT-1,0L);

            SetDlgItemText(hDlg, IDD_PROMPT2, String2);
            SendDlgItemMessage(hDlg, IDD_STRING2, EM_LIMITTEXT, LINEOTEXT-1,0L);

            SetDlgItemText(hDlg, IDD_PROMPT1, String1);
            SendDlgItemMessage(hDlg, IDD_STRING1, EM_LIMITTEXT, LINEOTEXT-1,0L);

            SetFocus(GetDlgItem(hDlg, IDD_STRING1));

            return (FALSE);

        case WM_COMMAND:
            switch (wParam)
            {
                case IDD_OK:
                    GetDlgItemText(hDlg, IDD_STRING1, String1, LINEOTEXT);
                    GetDlgItemText(hDlg, IDD_STRING2, String2, LINEOTEXT);
                    GetDlgItemText(hDlg, IDD_STRING3, String3, LINEOTEXT);
                    EndDialog(hDlg, wParam);
                    return (TRUE);

                case IDD_CANCEL:
                    String1[0] = '\0';
                    String2[0] = '\0';
                    String3[0] = '\0';
                    EndDialog(hDlg, wParam);
                    return (TRUE);
            }
            break;

        default:
        break;

    }
    return (FALSE);                /* Didn't process a message */
}

#endif

#if defined (W32)
/****************************************************************************
*
*    FUNCTION: DllMain(PVOID, ULONG, PCONTEXT)
*
*    PURPOSE: Win 32 Initialization DLL
*
****************************************************************************/

BOOL WINAPI DllMain(
IN PVOID hmod,
IN ULONG ulReason,
IN PCONTEXT pctx OPTIONAL)
{
  hLib_Instance = hmod;

    return TRUE;

    UNREFERENCED_PARAMETER(ulReason);
    UNREFERENCED_PARAMETER(pctx);
}
#endif
