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

    PROGRAM:    uiaddin2

    FILE:       uiaddin2.c

    SYNTAX:     called from the Notes Client Actions menu

    PURPOSE:    Demonstrates how to implement a Notes menu add-in
                program that populates the caret field of an open 
                document.

    DESCRIPTION:
        This program provides a procedure to handle each of four menu
        items added to the Actions menu in Notes.
               
        The first menu item is always enabled.  When this menu item is
        chosen, its procedure finds the system date and time and prints
        out this information in a message box.  
               
        The second menu item is enabled only if the user has a document
        opened in edit mode and is editing a field of type TIME.  When
        this menu item is chosen, its procedure imports the system date
        and time into this field.

        The third menu item is enabled only if the user has a document
        opened in edit mode and is editing a field called CompanyName.
        When this menu item is chosen, its procedure imports a hardcoded
        company name into this field.

        The fourth menu item is enabled only if the user has a document
        opened in edit mode and is editing a field of type TEXT.  When
        this menu item is chosen, its procedure imports hard coded text
        into this field.

   FUNCTIONS:
       DTMenuProc   - the addin's menu processing procedure.
       InsertText   - procedure to insert text into a document.

****************************************************************************/

#include <fcntl.h>
#include <io.h>
#include <sys\stat.h>
#include <windows.h>            /* required for all Windows apps */

/* Standard library include files */
#include <stdio.h>
#include <string.h>
#include <io.h>

/* Domino and Notes include files */

#include <global.h>
#include <addinmen.h> 
#include <ostime.h>
#include <misc.h>
#include <editdflt.h>
#include <osmem.h>
#include <easycd.h>

/* Menu Item IDs */
#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

#define IDM_DISPLAY_DATE 0
#define IDM_INSERT_DATE 1
#define IDM_INSERT_CO_NAME 2
#define IDM_INSERT_LOC 3

#define COMPANY_NAME "Acme Corporation"
#define LOCATION     "Anytown, USA"

/* create a table that contains the ID and name of the menu items */

typedef struct
{
    WORD ID;
    char *MenuString;
} MENUINFO1;

static MENUINFO1 MenuTable[] =
{
    IDM_DISPLAY_DATE,   "Displa&y Date/Time",
    IDM_INSERT_DATE,    "Insert Date&/Time",
    IDM_INSERT_CO_NAME, "Insert Company Name",
    IDM_INSERT_LOC,     "Insert Location"
};


/* local function prototype */
void InsertText (NAM_COMMAND_INFO *, char *);
void DspMessageBox (HWND, char *, char *);
int OpenOutputFile (char *);


NAMRESULT LNCALLBACK DTMenuProc (WORD wMsg, void far *pParam)
{
    static WORD startingID;   /* the Notes ID for the first 
                                addin menu item */

    switch (wMsg)
    {
        case NAMM_INIT:
        {
            NAM_INIT_INFO *pInitInfo;

            /* pParam is a pointer to the NAM_INIT_INFO structure */
            pInitInfo = (NAM_INIT_INFO *)pParam;

            /* If this is the first time through, save wStartingID.
               The menu ID for this item in Windows is wStartingID + 
               the menu item ID which is stored in the MenuTable.
               The menu item ID is given to Notes via the 
               wMenuID member of the NAM_INIT_INFO
               strucure */

            if (pInitInfo->wMenuItemNumber == 0)    /* first time through */
                startingID = pInitInfo->wStartingID;

            /* Add each menu item to the Notes Actions menu */
            /* Tell Notes about the menu item id and name */

            pInitInfo->wMenuID = MenuTable[pInitInfo->wMenuItemNumber].ID;

            strcpy (pInitInfo->MenuItemName, 
                    MenuTable[pInitInfo->wMenuItemNumber].MenuString);

            /* if there are more menu items to add, request Notes to send
               NAM_INIT again so that the second menu item may be added */
            
            if ( pInitInfo->wMenuItemNumber < 
                 sizeof(MenuTable) / sizeof(MENUINFO1) - 1 )
                return (NAM_INIT_CONTINUE);
         
            return (NAM_INIT_STOP);
        }

        case NAMM_INITMENU:
        {
            NAM_INITMENU_INFO *pInitMenuInfo;
            WORD Flags;

            /* pParam is a pointer to a NAM_INITMENU_INFO structure */
            pInitMenuInfo = (NAM_INITMENU_INFO *)pParam;

            /* enable the Insert Date/Time menu item ONLY IF the user is in
               the proper context (ie:  editing a TIME field in 
               a document) */

            if ( pInitMenuInfo->Data.fCanImport &&
                (pInitMenuInfo->Data.Context == NAM_IN_EDIT_RW) &&
                (pInitMenuInfo->Data.IXData.Edit.CaretFieldType == 
                 FIELD_TYPE_TIME) )

                Flags = MF_ENABLED | MF_BYCOMMAND;
            else
                Flags = MF_GRAYED | MF_BYCOMMAND;

            EnableMenuItem( pInitMenuInfo->hMenu,
                            startingID + MenuTable[IDM_INSERT_DATE].ID,
                            Flags);

            /* enable Insert Company Name menu item ONLY IF the user is in
               the proper context (ie:  editing a field called CompanyName) */

            if ( pInitMenuInfo->Data.fCanImport &&
                 (pInitMenuInfo->Data.Context == NAM_IN_EDIT_RW) &&
                 !strcmp(pInitMenuInfo->Data.IXData.Edit.CaretFieldName, 
                 "CompanyName") )

                Flags = MF_ENABLED  | MF_BYCOMMAND;
            else
                Flags = MF_GRAYED | MF_BYCOMMAND;

            EnableMenuItem( pInitMenuInfo->hMenu,
                            startingID + MenuTable[IDM_INSERT_CO_NAME].ID,
                            Flags);

            /* enable Insert Location menu item ONLY IF the user is in
               the proper context (ie:  editing a plain text field) */

            if ( pInitMenuInfo->Data.fCanImport &&
                 (pInitMenuInfo->Data.Context == NAM_IN_EDIT_RW) &&
                 (pInitMenuInfo->Data.IXData.Edit.CaretFieldType == 
                  FIELD_TYPE_TEXT) )
            
            Flags = MF_ENABLED | MF_BYCOMMAND;
            else
                Flags = MF_GRAYED | MF_BYCOMMAND;

            EnableMenuItem( pInitMenuInfo->hMenu,
                            startingID + MenuTable[IDM_INSERT_LOC].ID,
                            Flags);
            return (NAM_NOERROR);

        }


        case NAMM_COMMAND:
        {
            NAM_COMMAND_INFO *pCmdInfo;
            STATUS apiSts;

            /* pParam is a pointer to a NAM_COMMAND_INFO structure */
            pCmdInfo = (NAM_COMMAND_INFO *)pParam;


            switch (pCmdInfo->wMenuID)
            {
                case IDM_DISPLAY_DATE:
               /* display the date and time in a message box */
                {
                    TIMEDATE td;
                    char szTD[MAXALPHATIMEDATE + 1];
                    WORD wRetLen;
                    TFMT tdFormat;

                    memset (szTD, '\0',MAXALPHATIMEDATE + 1);
                    OSCurrentTIMEDATE (&td);
                    tdFormat.Date = TDFMT_FULL4;
                    tdFormat.Time = TTFMT_FULL;
                    tdFormat.Zone = TZFMT_NEVER;
                    tdFormat.Structure = TSFMT_DATETIME;

                    if (apiSts = ConvertTIMEDATEToText (NULL, &tdFormat, &td, szTD,
                                                        MAXALPHATIMEDATE, 
                                                        &wRetLen))
                    {
                        DspMessageBox (pCmdInfo->hNotesWnd,
                                       "Error converting system date/time", 
                                       "UIADDIN2");
                        return (NAM_NOERROR);
                    }
                    DspMessageBox (pCmdInfo->hNotesWnd, szTD, "DATE AND TIME");
                    break;
                }

                case IDM_INSERT_DATE:
                /* insert the date and time in the document */
                {
                    TIMEDATE td;
                    char szTD[MAXALPHATIMEDATE + 1];
                    WORD wRetLen;
                    TFMT tdFormat;

                    memset (szTD, '\0',MAXALPHATIMEDATE + 1);
                    tdFormat.Date = TDFMT_FULL4;
                    tdFormat.Time = TTFMT_FULL;
                    tdFormat.Zone = TZFMT_NEVER;
                    tdFormat.Structure = TSFMT_DATETIME;

                    OSCurrentTIMEDATE (&td);
                    if (apiSts = ConvertTIMEDATEToText (NULL, &tdFormat, &td, szTD,
                                                        MAXALPHATIMEDATE, 
                                                        &wRetLen))
                    {
                        DspMessageBox (pCmdInfo->hNotesWnd,
                                       "Error converting system date/time", 
                                       "UIADDIN2");
                        return (NAM_NOERROR);
                    }

                    InsertText (pCmdInfo, szTD);
                    break;
                }

                case IDM_INSERT_CO_NAME:
                    InsertText (pCmdInfo, COMPANY_NAME);
                    break;

                case IDM_INSERT_LOC:
                    InsertText (pCmdInfo, LOCATION);
                    break;

                default:
                    break;
            }
         return (NAM_NOERROR);

        } /* case NAMM_COMMAND */

        case NAMM_TERM:
            return (NAM_NOERROR);

        default:                /* default processing */
            return (NAM_NOERROR);
    } /* switch wMsg */
}
/************************************************************************

   FUNCTION:  void InsertText (NAM_COMMAND_INFO *pInfo, char *szText)
 
   PURPOSE:   Opens the CD file given by pInfo and uses the 
              Compound Text routines to add the appropriate 
              textual data to the file.  Then sends the
              WM_ADDIN_IMPORT message to Notes to import the 
              data into the document.

   COMMENTS:
      Inputs:
         pInfo - pointer to NAM_COMMAND_INFO structure

 **************************************************************************/
void InsertText (NAM_COMMAND_INFO *pInfo, char *szText)
{
    NAM_CONTEXT_DATA *pData = &(pInfo->Data);
    int hCDFile;
    unsigned int wError;
    STATUS nError;
    DHANDLE hCompound, hReturnCD;     /* handle to CompoundText, and 
                                       returned CD buffer */
    COMPOUNDSTYLE Style;
    DWORD  dwStyleID;
    DWORD  dwReturnCDSize;
    char   szReturnCDFile[MAXPATH];  /* returned CD filename if the data
                                        cannot be stored in memory */
    void  *pCD;

    nError = CompoundTextCreate (
                                 NULLHANDLE,     /* create a standalone CompoundText context */
                                 NULL,
                                 &hCompound);

    if (nError != NOERROR)
    {
        DspMessageBox (pInfo->hNotesWnd, "Unable to create compound text", 
                       "UIADDIN2");
        return;
    }

    /* Add the compound text (the Location) */
    CompoundTextInitStyle (&Style);
    nError = CompoundTextDefineStyle (
                                      hCompound,          /* handle to compound text */
                                      "Normal",           /* style name */
                                      &Style,
                                      &dwStyleID);        /* style id */
 
    if (nError)
    {
        DspMessageBox (pInfo->hNotesWnd, "Unable to define CompoundText style",
                       "UIADDIN2");
        CompoundTextDiscard (hCompound);
        return;
    }
    nError = CompoundTextAddParagraphExt (
                                          hCompound,            /* handle to compound text */
                                          dwStyleID,            /* style id */
                                          pData->IXData.Edit.Import.FontID,       /* font id */
                                          szText,               /* text to add */
                                          (DWORD)strlen(szText),  /* length of text to add */
                                          NULL);          /* CLS translation table */

    if (nError)
    {
        DspMessageBox (pInfo->hNotesWnd, 
                       "Unable to add CompoundText paragraph", "UIADDIN2");
        CompoundTextDiscard (hCompound);
        return;
    }

    nError = CompoundTextClose (
                                hCompound,          /* handle to compound text */
                                &hReturnCD,         /* handle to CD buffer */
                                &dwReturnCDSize,    /* size of returned buffer */
                                szReturnCDFile,     /* file, if CD can't fit into a buffer */
                                MAXPATH - 1);       /* size of szReturnCDFile */
    if (nError != NOERROR)
    {
        CompoundTextDiscard (hCompound);
        DspMessageBox (pInfo->hNotesWnd, "Unable to process compound text", 
                       "UIADDIN2");
        return;
    }

    if (hReturnCD != NULLHANDLE)
    {
        /* CompositeText is in a buffer - write it to the import file */
        /* Create the import file */

        hCDFile = OpenOutputFile (pData->IXData.Edit.Import.OutputFileName);

        if (hCDFile == -1)
        {
            DspMessageBox (pInfo->hNotesWnd, "Unable to create import file", 
                           "UIADDIN2");
            return;
        }

        /* write the Composite Text to the file */

        pCD = OSLockObject (hReturnCD);
        if (pCD == NULL)
        {
            DspMessageBox (pInfo->hNotesWnd, "Unable to access CompoundText", 
                           "UIADDIN2");
            OSMemFree (hReturnCD);    /* free the buffer */
            _close(hCDFile);
            return;
        }

        /* Since the compound text is in a buffer, it is known that this
           buffer must be < 64K and we can therefore cast dwReturnCDSize
           to an unsigned int */

        wError = _write (hCDFile, pCD, (unsigned int)dwReturnCDSize );
        if (wError == -1)
        {
            DspMessageBox (pInfo->hNotesWnd, "Unable to write to import file", 
                           "UIADDIN2");
            OSUnlockObject (hReturnCD);
            OSMemFree (hReturnCD);    /* free the buffer */
            _close(hCDFile);
            return;
        }
        OSUnlockObject(hReturnCD);
        OSMemFree (hReturnCD);    /* free the buffer */
 
        _close(hCDFile);
    }
    else
    {
        /* CompoundText is in a file, rename the file to the import
           file name */
        wError = rename (szReturnCDFile,
                       pData->IXData.Edit.Import.OutputFileName);
        if (wError)
        {
            DspMessageBox (pInfo->hNotesWnd, "Unable to write to import file", 
                           "UIADDIN2");
            return;
        }
    }

    /* Send Notes the message to import the file */
    SendMessage(pInfo->hNotesWnd, WM_ADDIN_IMPORT, 0, (LONG)pInfo);
}


/*************************************************************************

    FUNCTION: DspMessageBox

    PURPOSE:  Displays a message box

    INPUT:
        szString = message to print out
        szCaption = caption for message box

*************************************************************************/
void DspMessageBox (HWND hWndParent, char *szString, char *szCaption)
{
    MessageBox (hWndParent, szString, szCaption, MB_OK);
}

/*************************************************************************

   FUNCTION: OpenOutputFile (char *CDFile)

   PURPOSE:  Returns the file handle of a newly created output file.

*************************************************************************/
int OpenOutputFile (char *CDFile)
{
    return (_open (CDFile, O_CREAT | O_WRONLY | O_BINARY, S_IREAD | S_IWRITE));
}

