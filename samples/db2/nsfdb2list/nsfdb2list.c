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

    PROGRAM:    nsfdb2list

    FILE:       nsfdb2list.c

    PURPOSE:    Provides a list of all NSFDB2 databases.

    SYNTAX:     nsfdb2list <DB_NAME> <server_name>

*************************************************************************/

/* OS and C include files */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <nsfdb.h>
#include <osmisc.h>
#include <nsfnote.h>
#include <log.h>
#include <osmem.h>
#include <osfile.h> 
#include <kfm.h>                                             
#include <textlist.h>
#include"../../printLog.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

#define 	LINEOTEXT			256


int main(int argc, char *argv[])
{
    char                *DB_NAME;
    char                *server_name;
    char                 fullpath[100];                      /*Relative to the Domino server's data directory.  */
    STATUS              error = 0;                          /* return code from API calls */
    DBHANDLE            hDB = NULL;                         /* handle to database */
    FILE                *flog = NULL;                       /*name of log file*/
    TIMEDATE            retDataMod;                         /* used in NSFDbOpenExtended*/
    TIMEDATE            retNonDataMod;                      /*same as above*/
    DHANDLE             hDbList;                            /*Handle of the test db*/
    char                DbList[100][100];
    char                **pDbList = (char **)(DbList); 
    char                *pTlist;
    char                buffer[100];
    char                *pText;
    char                tablespaceName[100];
    DWORD               size;
    WORD                wTextSize;
    USHORT              nEntries, i; 
    char                szTempStr[LINEOTEXT];
    
    /*  Command line parameters. */
    if (argc != 3)
    {
        PRINTLOG ("\nUsage:  %s  <DB_NAME>  <server_name>  \n", argv[0]);
        return (NOERROR);
    }
    
    DB_NAME = argv[1];
    server_name = argv[2];
    
    PRINTLOG("========begin the test\n");
    error = NotesInitExtended (argc, argv);
    if (error)
    {
        PRINTLOG("========Error: Unable to initialize Notes.\n");
        return (1);
    }
        
    PRINTLOG("========begin to Construct the OSPathNet\n");

    if (strlen(server_name))
    {
        if (error = OSPathNetConstruct(NULL, server_name,
                                       DB_NAME, fullpath))
        {
            PRINTERROR(error,"OSPathNetConstruct");
            goto EXIT;
        }
    }
        
    else
    {
        strcpy(fullpath, DB_NAME);
    }

    if ( error = NSFDbOpenExtended (fullpath, 0, NULLHANDLE, NULL ,
                                    &hDB, &retDataMod, &retNonDataMod))
    {
        PRINTERROR(error,"NSFDbOpenExtended");
        goto EXIT;
    }
                
    PRINTLOG("========open DB %s successfully\n", fullpath);
        
    /*Test the api*/
    size = sizeof(buffer);
    if(error = NSFDB2GetInfo(hDB, DB2NSF_INFO_TABLESPACE_NAME,
                             buffer,&size))
    {
        PRINTERROR(error,"NSFDB2GetInfo");
        goto EXIT;                
    }
    else
    {
        PRINTLOG("========Get DB Info:%s successfully", "DB2NSF_INFO_TABLESPACE_NAME\n");
    }
    memcpy(tablespaceName, buffer, strlen(buffer));        
        
    /*use NULL for the last parameter*/
    error = NSFDB2ListNSFDB2Databases( &hDbList,
                                       (void **)pDbList,
                                       NULL);
    if( error )
    {
        PRINTERROR(error,"NSFDB2ListNSFDB2Databases");
        goto EXIT;
    }
    else 
    {
        PRINTLOG("\n========execute NSFDB2ListNSFDB2Databases successfully!!\n");
        PRINTLOG("\n========now List ALL NSFDB2 DataBases\n");                
    }
    pTlist = (char *)OSLockObject(hDbList);
    nEntries = ListGetNumEntries(pTlist, FALSE);
        
    if(nEntries) 
        
    PRINTLOG("\n========We have %d NSFDB2 Databases\n", nEntries);
    pText = buffer;
    for( i=0; i<nEntries; i++)
    {
        error = ListGetText(pTlist,
                            FALSE,
                            i,
                            &pText,
                            &wTextSize);
        if(error)
        {
            PRINTLOG("\n========error in ListGetText, i = %d\n", i);
                        PRINTERROR(error,"ListGetText");
        }
        else
        {
            strncpy(szTempStr, pText, wTextSize);  
            szTempStr[wTextSize] = '\0';
            PRINTLOG("        %d: %s\n", i, szTempStr);                                                                        
        }
    }
        
    OSUnlockObject(hDbList);
    OSMemFree(hDbList);
        
        /* Close the database.*/

    if(error = NSFDbClose (hDB))
        {
           PRINTERROR(error,"NSFDbClose");
           goto EXIT;
        }
        /* End of main routine */

        PRINTLOG("\n========Program completed successfully.\n");
        return (0);
 
 EXIT:
        PRINTLOG("========ERR: Terminating Notes...\n");
        NotesTerm();
        return (1);                
}
