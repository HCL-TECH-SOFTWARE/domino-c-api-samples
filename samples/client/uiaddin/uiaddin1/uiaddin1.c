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

    PROGRAM:    uiaddin1

    FILE:       uiaddin1.c

    SYNTAX:     This program is run from the Notes Client menu

    PURPOSE:    Shows how to implement a Notes client menu add-in.

    DESCRIPTION:
        This program provides a procedure to handle the menu item
        "DOS Shell". Activate this menu-addin with the AddInMenus
        setting in the the notes.ini file. When the Notes user picks 
        "DOS Shell" from the action menu, this procedure executes the 
        DOS command shell.

****************************************************************************/

#include <windows.h>            /* required for all Windows apps */

/* Standard library include files */
#include <string.h>

/* Domino and Notes include files */
#include <global.h>
#include <addinmen.h> 

#define IDM_DOS_SHELL 0


NAMRESULT LNCALLBACK AddinMenuProc (WORD wMsg, LONG lParam)
{
    switch (wMsg)
    {
        case NAMM_INIT:
        {
            NAM_INIT_INFO *pInitInfo;

            /* lParam is a pointer to the NAM_INIT_INFO structure */
            pInitInfo = (NAM_INIT_INFO *)lParam;

            /* Add the menu item to the Notes Actions menu */
            /* Assign this menu item an id and a name */
            pInitInfo->wMenuID = IDM_DOS_SHELL;

            strcpy (pInitInfo->MenuItemName, "&DOS Shell");

            /* tell Notes that there are no more menu items to add */
            return (NAM_INIT_STOP);
        }

        case NAMM_COMMAND:
        {

            NAM_COMMAND_INFO *cmdInfo;
            WORD wSts;

            /* The addin menu item was chosen */
            /* lParam is a pointer to the NAM_COMMAND_INFO structure */
            cmdInfo = (NAM_COMMAND_INFO *)lParam;

            wSts = WinExec("command.com", SW_SHOWMAXIMIZED);
            if (wSts < 32 )
                MessageBox (cmdInfo->hNotesWnd, "Cannot execute this item", 
                            "Addin Error", MB_ICONHAND | MB_OK);
                return (NAM_NOERROR);
        }

        default:          /* default processing for all other messages */
            return (NAM_NOERROR);
    }
}


