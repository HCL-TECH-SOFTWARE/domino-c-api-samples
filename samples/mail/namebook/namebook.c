/**************************************************************************
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
                
    PROGRAM:    namebook

    FILE:       namebook.c

    PURPOSE:    Demonstrate the HCL C API for Notes/Domino 
                function NAMEGetAddressBooks().

    SYNTAX:     namebook  <server name>  [options]

        if options = TITLES, this prints the title of each database in
        addition to file path.

**************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif


/* OS and C include files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* HCL C API for Notes/Domino include files */
#include <global.h>
#include <intl.h>
#include <nsf.h>
#include <acl.h>
#include <misc.h>
#include <osmem.h>
#include <osenv.h>
#include <osfile.h>                     /* OSPathNetParse */
#include <ostime.h>
#include <lookup.h>
#include <stdnames.h>
#include <textlist.h>
#include <editods.h>
#include <oserr.h>
#include <osmisc.h>
#include "../../printLog.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

 
/************************************************************************

    FUNCTION:   Main

*************************************************************************/

int main (int argc, char *argv[])
{
    char    szServer[MAXUSERNAME+1];
    char*   szOptions;
    WORD    wOption;
    STATUS  error=NOERROR;
    WORD    wCount;
    WORD    wLength;
    WORD    wEntry;
    WORD    wEntryLen;
    DHANDLE  hReturn;
    char   *pszReturn;    
    char    achPort[MAXPATH];
    char    achServer[MAXPATH];
    char    achFile[MAXPATH];
    char    szNetPathName[MAXPATH];         /* Network Path of Database */
    char    szPortName[MAXPATH];         /* Port Name */

    if (argc<1 || argc>2)
    {
Syntax:
        PRINTLOG ("Syntax: namebook  [option]\n");
        PRINTLOG (
        "where [option] = TITLES or nothing.\n");
        return NOERROR;
    }

    wOption = 0;

    if (argc==2)
    {
        szOptions=argv[1];
        if (!strcmp(szOptions,"TITLES"))
            wOption = NAME_GET_AB_TITLES | NAME_DEFAULT_TITLES ;
        else
        {
            PRINTLOG ("Error: unrecognized option '%s'\n", szOptions);
            goto Syntax;
        }
    }
        

    if (error=NotesInitExtended (argc, argv))
    {
        PRINTLOG("\n Unable to initialize Notes. Error Code[0x%04x]\n", error);    
        return (1);
    }
    
    /* get the Mail Server from notes.ini */
    if (!OSGetEnvironmentString("MailServer", szServer, MAXUSERNAME))
    {
        NotesTerm();
        PRINTLOG("\nError in getting mail server name. Exiting");
        return (1);
    }
    /* get the port name from notes.ini */
    if (!OSGetEnvironmentString("Ports", szPortName, MAXPATH))
    {
        NotesTerm();
        PRINTLOG("\nError in getting port name. Exiting");
        return (1);
    }

    error = NAMEGetAddressBooks( 
                                 szServer,
                                 wOption,
                                 &wCount,
                                 &wLength,
                                 &hReturn);

    if (error)
       goto Bye;

    /* If there are no entries, return. */
    if (!wCount) 
    {
        PRINTLOG ("No entries in NAMES.\n");
        goto Bye;
    }

    /* Lock the return buffer. */

    pszReturn = OSLock(char, hReturn);

    /* Loop through the entries. */

    for (wEntry = 0; wEntry < wCount; wEntry++)
    {
         /* Get path length. */
        wEntryLen = strlen(pszReturn);
        error = OSPathNetConstruct (szPortName, (char FAR *)szServer,
                                    pszReturn, (char FAR *)szNetPathName);
        if (error != NOERROR)
            goto Bye;

        /* Parse the path into components. */
        OSPathNetParse(szNetPathName, achPort, achServer, achFile);

        /* process components */
        PRINTLOG("Entry %d\n\tPort = %s\n\tServer = %s\n\tFile = %s\n",
                (wEntry+1), achPort, achServer, achFile);

        if (wOption & NAME_GET_AB_TITLES)
        {
            /* Advance to title. */
            pszReturn += wEntryLen+1;
            wEntryLen = strlen(pszReturn);
            PRINTLOG ("\tTitle = '%s'\n", pszReturn);
        }
          
        /* Advance pointer to next entry. */
        pszReturn += wEntryLen+1;
    }

    /* Unlock and free the return buffer. */
    OSUnlock(hReturn);
    OSMemFree(hReturn);

Bye:
    if (error)
        PRINTERROR (error,"OSPathNetConstruct");  
    NotesTerm();
    return(error);
}
