/********************************************************************
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
 *  PROGRAM: XEDIT.C                                                
 *                                                                  
 *  PURPOSE: Generic ASCII Text Export DLL for Windows versions of  
 *  HCL Notes. Can be configured in EDITEXP state-                
 *  ment in Notes.ini to use memory object or temporary file as in- 
 *  termediate storage for CD Item obtained from Note.              
 *                                                                  
 *  FUNCTIONS:                                                      
 *                                                                  
 *  ExpAsciiText()- Export text from current Domino and Notes       
 *  TYPE_COMPOSITE item to a file.                                  
 *                                                                  
 ********************************************************************/

/* OS-specific includes */
#if defined (W32)
#include <windows.h>
#endif

#if defined (W32)
#include <io.h>
#endif

/* C includes */
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys\types.h>
#include <sys\stat.h>

/* Domino and Notes includes */
#include <global.h>
#include <osmisc.h>
#include <ossignal.h>
#include <osmem.h>
#include <editods.h>
#include <ixedit.h>
#include <ixport.h>
#include <oserr.h>

/* Local includes */
#include "xedit.h"

/*
 *  Local CleanUp defines.  These are used to keep track of our progress, 
 *  so that we will know what needs to be closed or freed when we exit.
 */

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

#define DO_NOTHING          0x00
#define UNLOCK_EXPCDBID     0x01
#define FREE_EXPCDBID       0x02
#define CLOSE_EXPFILE       0x04
#define DELETE_EXPFILE      0x08
#define CLOSE_CDFILE        0x10
#define FREE_EXPBUFFER      0x20
#define UNLOCK_EXPBUFFER    0x40

/* Local defines */
#define IXOKMessage(str) (*(OSSIGMSGPROC)  \
          OSGetSignalHandler(OS_SIGNAL_MESSAGE))(str, OSMESSAGETYPE_OK)

/* Globals */
char   String1[LINEOTEXT];
char   String2[LINEOTEXT];
char   String3[LINEOTEXT];
DHANDLE hLib_Instance;

/***********************************************************************/
/* Import Procedure ExpAsciiText()                                     */
/*                                                                     */
/*    Inputs:                                                          */
/*      EditorData = Pointer to data block set up by editor for this   */
/*                   routine.                                          */
/*      Flags      = Import flags (see ixport.h)                       */
/*      hModule    = handle to this module.                            */
/*      AltLibraryName = Ignored here                                  */
/*      FileName   = File Name to export (fully expanded path name     */
/*                   included)                                         */
/*                                                                     */
/*    Outputs:                                                         */
/*      The Output File specfied in EditorData block is filled with    */
/*      text exported from the Rich Text Body of the Note being edited.*/
/***********************************************************************/

STATUS LNPUBLIC ExpAsciiText(
EDITEXPORTDATA *EditorData,
WORD Flags,
HMODULE hModule,
char *AltLibraryName,
char *FileName)
{
    STATUS      Error_Status = NOERROR, CleanUp_Status = NOERROR;
    WORD        CleanUp_State = DO_NOTHING;
    int         Dialog_Return = 0;
    char        Tmp_String[LINEOTEXT], Fmt_String[LINEOTEXT];
    char        LineDelimiter[3], *pTmpCD, *pExpBuffer;
    int         CharsPerLine;
    DHANDLE       hExpBuffer;
    DWORD       ExpBufferLength, TmpCDLen;
    BOOL        fStripTabs;
    BLOCKID     bidTmpCD;
    int         hExpFile, hTmpCDFile;
    struct _stat TmpStats;
    long        count;
#if defined (W32)
    FARPROC   lpfnDlgProc;
#endif


  /* Prompt user for Line Delimiter, Characters per line, and    */
  /* whether or not to strip tabs.                               */

    strcpy(String1, "Output line delimiter(s) {LF, CR, CRLF, NUL}:");
    strcpy(String2, "Output line length:");
    strcpy(String3, "Strip Tabs {YES, NO}:");

#if defined (W32)

    lpfnDlgProc = MakeProcInstance(XEditDlgProc, hLib_Instance);
    DialogBox (hLib_Instance,
               MAKEINTRESOURCE(IDD_XEDIT),
               GetFocus(),
               (DLGPROC)lpfnDlgProc);
    FreeProcInstance ((DLGPROC)lpfnDlgProc);

#endif

    /* Get the line delimiter */

    if (!_stricmp("LF", String1))      /*  User selected <linefeed>     */
        strcpy(LineDelimiter, "\n");  
    else if(!_stricmp("CR", String1))  /*  User selected <return>       */
        strcpy(LineDelimiter, "\r");
    else if(!_stricmp("NUL", String1)) /*  User selected null value       */
        strcpy(LineDelimiter, "");
    else if(!_stricmp("CRLF", String1)) /* User selected <return><linefeed> */
        strcpy(LineDelimiter, "\r\n"); /*   (Most DOS text files) */
    else
    {
        /* User chose an invalid line delimiter  */
        if(!OSLoadString(hModule, ERR_XEDIT_BADDELIM,
                         Fmt_String, LINEOTEXT-1))
            strcpy(Fmt_String, "OSLoadString() ERROR: %s");

    String1[(LINEOTEXT-1) - (strlen(Fmt_String)-2)]='\0';
    wsprintf(Tmp_String, Fmt_String, String1);
    IXOKMessage(Tmp_String);
    goto Exit;
    }

    /* Get the Output line length */
    CharsPerLine = (WORD) atoi(String2);

    /* check for 0 line length and negative numbers */
    if ((CharsPerLine <= 0) || (String2[0] == '-'))
    {
        if(!OSLoadString(hModule, ERR_XEDIT_BADLEN,
                         Fmt_String, LINEOTEXT-1))
            strcpy(Fmt_String, "OSLoadString() ERROR: %s");

        String2[(LINEOTEXT-1) - (strlen(Fmt_String)-2)]='\0';
        wsprintf(Tmp_String, Fmt_String, String2);
        IXOKMessage(Tmp_String);
        goto Exit;
    }

    /* See if tabs should be stripped out or not. */
    if (!_stricmp("YES", String3))
        fStripTabs = TRUE;
    else if(!_stricmp("NO", String3))
        fStripTabs = FALSE;
    else
    { /* Error is String3 is not either "YES" or "NO"  */
        if(!OSLoadString(hModule, ERR_XEDIT_BADSTRIP,
                         Fmt_String, LINEOTEXT-1))
            strcpy(Fmt_String, "OSLoadString() ERROR: %s");

        String3[(LINEOTEXT-1) - (strlen(Fmt_String)-2)]='\0';
        wsprintf(Tmp_String, Fmt_String, String3);
        IXOKMessage(Tmp_String);
        goto Exit;
    }

    /* Init String(s) for future use */
    String1[0] = String2[0] = String3[0] = '\0';

    /* Check if temp file or temp buffer used for CD export */
    if (!(IXFLAG_DOC_EXPORT_USE_CD_FILE & Flags))
    {
        bidTmpCD.pool  = EditorData->hCompBuffer;
        bidTmpCD.block = NULLBLOCK;
        if ((TmpCDLen = EditorData->CompLength) > MAXONESEGSIZE)
        {
            if(!OSLoadString(hModule, ERR_XEDIT_CDTOOBIG,
                             Tmp_String, LINEOTEXT-1))
                strcpy(Tmp_String, "OSLoadString() ERROR");
            IXOKMessage(Tmp_String);
            goto Exit;
        }
    }
    else
    {
        /*
         * Display a string on the Domino and Notes workstation application's 
         * title bar to tell user that the conversion of Rich Text has
         * begun.  A delay is programmed to give the user time to read 
         * the message before the application writes over it.
         */
        IXPostMessage("Beginning conversion of Rich Text...");
        for (count = 0; count < 1048576; count++)
            ; /* Give user time to read message. */


            /* Open the Temp file that contains the Compound Document records */
            hTmpCDFile = _open(EditorData->InputFileName, O_BINARY | O_RDONLY);

        if (hTmpCDFile == -1)        /* If MS-DOS error during open */
        {
            Error_Status = MapError(errno);
            goto Exit;
        }
        else
            CleanUp_State += CLOSE_CDFILE;  /* Flag our progress so far. */

        /* Determine CD Tmp file size */
        if (-1 == _fstat(hTmpCDFile, &TmpStats))
        {
            Error_Status = MapError(errno);
            goto Exit;
        }

        /* If temp file got larger than the max size allowed, */
        /* load error message and exit.                       */
        if ((TmpCDLen = TmpStats.st_size) > MAXONESEGSIZE)
        {
            if(!OSLoadString(hModule, ERR_XEDIT_CDTOOBIG,
                             Tmp_String, LINEOTEXT-1))
                strcpy(Tmp_String, "OSLoadString() ERROR");
            IXOKMessage(Tmp_String);
            goto Exit;
        }

        /* Build a BLOCKID.  First, allocate a mem block. */
        if (Error_Status = OSMemAlloc(0, TmpCDLen, &bidTmpCD.pool))
            goto Exit;
        else
            CleanUp_State += FREE_EXPCDBID;  /* Flag our progress so far. */

        bidTmpCD.block = NULLBLOCK;

        /* Lock the memory block to be used for BLOCKID.  */
        if ((pTmpCD = OSLockBlock(char, bidTmpCD)) == NULL)
        {
            IXOKMessage("Unable to lock bidTmpCD");
            goto Exit;
        }
        else
            CleanUp_State += UNLOCK_EXPCDBID;  /* Flag our progress so far. */

        /* Read the TmpCDFile into Domino and Notes memory object */
        if(-1 == _read(hTmpCDFile, pTmpCD,(WORD) TmpCDLen))
        {
            Error_Status = MapError(errno);
            goto Exit;
        }

        /* Close the TmpCDFile */
        if(-1 == _close(hTmpCDFile))
        {
            Error_Status = MapError(errno);
            goto Exit;
        }
        else
            CleanUp_State -= CLOSE_CDFILE;  /* Flag our progress so far. */

        /* Unlock the bidTmpCD */
        CleanUp_State -= UNLOCK_EXPCDBID;  /* Flag our progress so far. */
        if(!OSUnlockBlock(bidTmpCD))
        {
            Error_Status = ERR_OS_ALREADY_UNLOCKED;
            goto Exit;
        }
    }

    /* Convert the Object associated with the BID to a plain text  */
    /* buffer. ConvertItemToText throws away CD Records that don't */
    /* map to text: Document Links, DDE Links, Graphics, etc.      */

    if (Error_Status = ConvertItemToText(bidTmpCD, TmpCDLen,
                                         LineDelimiter, (WORD) CharsPerLine,
                                         &hExpBuffer, &ExpBufferLength,
                                         fStripTabs))
    {
        goto Exit;
    }
    else
        CleanUp_State += FREE_EXPBUFFER;  /* Flag our progress so far. */

    /* Free bidTmpCD's memory */
    CleanUp_State -= FREE_EXPCDBID;  /* Flag our progress so far. */
    if(Error_Status = OSMemFree(bidTmpCD.pool))
        goto Exit;

    /* Lock Down the Buffer returned by ConvertItemToText() */
    if ((pExpBuffer = OSLock(char, hExpBuffer)) == NULL)
    {
        IXOKMessage("Unable to lock ExpBuffer");
        goto Exit;
    }
    else
        CleanUp_State += UNLOCK_EXPBUFFER;  /* Flag our progress so far. */

    /*
     * Display a string on the Domino and Notes workstation application's 
     * title bar to tell user that the Export File is being written.
     * A delay is programmed to give the user time to read 
     * the message before the application writes over it.
     */
    IXPostMessage("Writing export file...");
    for (count = 0; count < 1048576; count++)
        ; /* Give user time to read message. */

    /* Open the Export file */
    hExpFile = _open( FileName, O_CREAT | O_RDWR | O_EXCL, 
                      _S_IREAD | _S_IWRITE);

    if (hExpFile == -1)        /* If MS-DOS error during open */
    {
        Error_Status = MapError(errno);
        goto Exit;
    }
    else
    {
        CleanUp_State += DELETE_EXPFILE;  /* Flag our progress so far. */
        CleanUp_State += CLOSE_EXPFILE;
    }

    /* Write the newly created plain text to the Export File */
    if(-1 == _write(hExpFile, pExpBuffer, (WORD) ExpBufferLength))
    {
        Error_Status = MapError(errno);
        goto Exit;
    }
    else
        CleanUp_State -= DELETE_EXPFILE;  /* Flag our progress so far. */

    /* Close the Export File */
    if(-1 == _close(hExpFile))  /* if MS-DOS error during close, */
    {
        Error_Status = MapError(errno);
        goto Exit;
    }
    else
        CleanUp_State -= CLOSE_EXPFILE;  /* Flag our progress so far. */

    /* Unlock the hExpBuffer  and Free it */
    CleanUp_State -= UNLOCK_EXPBUFFER;  /* Flag our progress so far. */
    if(!OSUnlock(hExpBuffer))           
    {
        Error_Status = ERR_OS_ALREADY_UNLOCKED;
        goto Exit;
    }
    CleanUp_State -= FREE_EXPBUFFER;  /* Flag our progress so far. */
    if(Error_Status = OSMemFree(hExpBuffer))
        goto Exit;

Exit:
    /*  Clean up as needed, using the flags set in CleanUp_State  */
    /*  to tell what needs to be closed or freed.                 */
    if (CleanUp_State)
    {
        /* Unlock the CD Temp Buffer and Free it */
        if (CleanUp_State & UNLOCK_EXPCDBID)
        if(!OSUnlockBlock(bidTmpCD))
        {
            CleanUp_Status = ERR_OS_ALREADY_UNLOCKED;

            /* 1st arg must be zero (0) to access default string table */
            CleanupTrouble(0, CleanUp_Status);
        }

        if (CleanUp_State & FREE_EXPCDBID)
        if(CleanUp_Status = OSMemFree(bidTmpCD.pool))
        {
            CleanupTrouble(0, CleanUp_Status);
        }

        /* Unlock the hExpBuffer  and Free it */
        if (CleanUp_State & UNLOCK_EXPBUFFER)
        if(!OSUnlock(hExpBuffer))
        {
            CleanUp_Status = ERR_OS_ALREADY_UNLOCKED;
            CleanupTrouble(0, CleanUp_Status);
        }

        if (CleanUp_State & FREE_EXPBUFFER)
        if(CleanUp_Status = OSMemFree(hExpBuffer))
        {
            CleanupTrouble(0, CleanUp_Status);
        }

        /* Close Temporary CD File. Domino and Notes will Delete the Temp file */
        if (CleanUp_State & CLOSE_CDFILE)
        if(-1 == _close(hTmpCDFile))
        {
            CleanUp_Status = MapError(errno);
            CleanupTrouble(0, CleanUp_Status);
        }

        /* Close the Export File */
        if (CleanUp_State & CLOSE_EXPFILE)
        if(-1 == _close(hExpFile))
        {
            CleanUp_Status = MapError(errno);
            CleanupTrouble(0, CleanUp_Status);
        }

        /* Delete the Export File */
        if (CleanUp_State & DELETE_EXPFILE)
        if(-1 == remove(FileName))
        {   /* If MS-DOS error during delete */
            CleanUp_Status = MapError(errno);
            CleanupTrouble(0, CleanUp_Status);
        }
    }
    return(Error_Status);
}

/********************************************************************/
/*  FUNCTION: void CleanupTrouble(HMODULE mod_handle, STATUS error) */
/********************************************************************/
void CleanupTrouble(
HMODULE mod_handle,
STATUS error)
{
    char error_str[LINEOTEXT], tmp_str[LINEOTEXT];

    OSLoadString(mod_handle, error, error_str, LINEOTEXT-1);
    wsprintf(tmp_str, "\nCleanup Error: %s\n", error_str);
    IXOKMessage(tmp_str);
}

/************************************************************************

    FUNCTION:   MapError

    PURPOSE:    Map MS-DOS Extended Error Codes to notes GLOBERR.H file
                errors

*************************************************************************/

STATUS MapError(
int wDosCode)
{
    STATUS  usNotesErr = NOERROR;

    switch (wDosCode)
    {
        case ENOENT: usNotesErr = ERR_NOEXIST;               break;
        case EMFILE: usNotesErr = ERR_FILES;                 break;
        case EACCES: usNotesErr = ERR_LOCK;                  break;
        case EEXIST: usNotesErr = ERR_EXISTS;                break;
        case EBADF:  usNotesErr = ERR_FD;                    break;
        case ENOSPC: usNotesErr = ERR_DISK;                  break;

        default:   usNotesErr = ERR_UNKNOWN;                 break;
    }
    return(usNotesErr);
}

#if defined (W32)
/************************************************************************

    FUNCTION: XEditDlgProc(HWND, UINT, UINT, LONG)

    PURPOSE:  Processes messages for "XEdit" dialog box

    MESSAGES:

      WM_INITDIALOG - initialize dialog box
      WM_COMMAND    - Input received

*************************************************************************/

BOOL LNPUBLIC XEditDlgProc(
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
