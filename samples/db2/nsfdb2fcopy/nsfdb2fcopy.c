
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

    PROGRAM:    NSFDB2FCopy

    FILE:       NSFDB2FCopy.c

    SYNTAX:     NSFDB2FCopy <DB_NAME> <TargetDB_NAME> <server_name>

    PURPOSE:    Copy the NSFDB2 database specified and reconnects the 
    copy of the DB2 data as an NSFDB2 database.

    DESCRIPTION:
    This function efficiently copies the NSFDB2 database specified by sourceNSF 
    and reconnects the copy of the DB2 data as an NSFDB2 database named  targetNSF . 
    This function will only work locally on a Domino server supporting NSFDB2 databases.

****************************************************************************/
/* OS and C include files */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <nsfdb.h>
#include <osmisc.h>
#include <nsfnote.h>
#include <osmem.h>
#include <osfile.h> 
#include <kfm.h>
#include <textlist.h>
#include <nsfdb.h>
#include "../../printLog.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif


/* local function prototypes */


int  verify(char*, char*);    /*verify the test result*/

int main(int argc, char* argv[])
{
    char            *DB_NAME;
    char            *TargetDB_NAME;
    char            *server_name;
    char            fullpath_name[100];
    STATUS          err = 0;
    char            buffer[200];
    char            Group_Name[100];
    DWORD           dwBuffer;
    
    DHANDLE         db_handle = NULL;
    DHANDLE         hNotesDLL = NULL;
    BOOL            isDB2 = 0;
    DWORD           size;
    TIMEDATE        retDataMod;
    TIMEDATE        retNonDataMod;
    int             i;

    DWORD infotype[] = {  DB2NSF_INFO_IS_DB2_BACKED,       /*1     Is database DB2-backed: 1/0*/                            DB2NSF_INFO_SCHEMA_NAME,         /*2     Group schema containing NSFDB2 data*/
                          DB2NSF_INFO_TABLESPACE_NAME,     /*3     Tablespace name*/
                          DB2NSF_INFO_TSID,                /*4     Tablespace ID*/
                          DB2NSF_INFO_USERSCHEMA_NAME,     /*5     Schema containing DAVs*/
                          DB2NSF_INFO_CLASS_DESC           /*6     NSFDB2 grouping class description*/
                       };
    char* infoString[] = {  "DB2NSF_INFO_IS_DB2_BACKED",     /*1     Is database DB2-backed: 1/0&*/
                            "DB2NSF_INFO_SCHEMA_NAME",       /*2     Group schema containing NSFDB2 data*/
                            "DB2NSF_INFO_TABLESPACE_NAME",   /*3     Tablespace name*/
                            "DB2NSF_INFO_TSID",              /*4     Tablespace ID*/
                            "DB2NSF_INFO_USERSCHEMA_NAME",   /*5     Schema containing DAVs*/
                            "DB2NSF_INFO_CLASS_DESC"         /*6     NSFDB2 grouping class description*/
                         } ;
    DWORD ServerInfo[] = {  DB2NSF_SERVINFO_SERVER_DEFAULT_TYPE,
                            DB2NSF_SERVINFO_NSFDB2_CAPABLE,
                            DB2NSF_SERVINFO_DB2_DATABASE_NAME
                         };
    char* serverInfoString[] = {  "DB2NSF_SERVINFO_SERVER_DEFAULT_TYPE",
                                  "DB2NSF_SERVINFO_NSFDB2_CAPABLE",
                                  "DB2NSF_SERVINFO_DB2_DATABASE_NAME"
                               };
                                   
    /*  Command line parameters. */                               
    if (argc != 4)
    {
        PRINTLOG ("\nUsage:  %s  <DB_NAME> <TargetDB_NAME> <server_name>  \n", argv[0]);
        return (NOERROR);
    }

    DB_NAME = argv[1];
    TargetDB_NAME = argv[2];
    server_name = argv[3];
        
        
    if (err = NotesInitExtended (argc, argv))
    {
        PRINTERROR(err,"NotesInitExtended");
        return 1;
    }
        
    PRINTLOG("======notes init successfully\n");
    /* Construct fully qualified path from components */
        
    if (strlen(server_name))
    {
        if (err = OSPathNetConstruct(NULL, server_name,
                                     DB_NAME, fullpath_name))
        {
                PRINTERROR(err,"OSPathNetConstruct");
                goto EXIT;
        }
                        PRINTLOG("======Construct os netpath successfully\n");
    }
    else
    {
        strcpy(fullpath_name, DB_NAME);
    }

    if ( err = NSFDbOpenExtended (fullpath_name,
                                  0,
                                  NULLHANDLE, 
                                  NULL,
                                  &db_handle, 
                                  &retDataMod, 
                                  &retNonDataMod))
    {
            PRINTERROR(err,"NSFDbOpenExtended");
            goto EXIT;        
    }
    
    PRINTLOG("======open %s succeessfully\n", fullpath_name)        ;

    /*to determine whether the DB is a NSF DB2 Database*/
    
    if( err = NSFDbIsDB2( db_handle, &isDB2) )        
    {
        PRINTERROR(err,"NSFDbIsDB2");
        goto EXIT;
    }
    else
    {
        PRINTLOG("\n======the database is a NSFDB2 DB\n");
    }
    
    PRINTLOG("\nbegin to get the DB2 DB's information");
    /*begin to get the DB2 DB's information*/

        
    for( i=0; i<6; i++)
    {
    
        size = sizeof(buffer);
        if( (i==0) | (i==3) )
        {
            size = sizeof(DWORD);
            if(err = NSFDB2GetInfo(db_handle, infotype[i],
                                   &dwBuffer,&size))
            {
                PRINTERROR(err,"NSFDB2GetInfo");
                goto EXIT;                                
            }
            else
            {
                PRINTLOG("\n======Get DB Info:%s successfully,return value is %d\n", infoString[i], dwBuffer);
            }
        }
        else
        {
            size = sizeof(buffer);
            if(err = NSFDB2GetInfo(db_handle, infotype[i],
                                   buffer,&size))
            {
                PRINTERROR(err,"NSFDB2GetInfo");
                goto EXIT;                                
            }
            else
            {
                if( i == 2)
                {
                    memcpy(Group_Name, buffer, sizeof(buffer));
                }
                PRINTLOG("\n======Get DB2 Info:%s, return value is %s\n", infoString[i], buffer);
            }
        }
    }

    PRINTLOG("\n======begin to get the DB2 server's information\n");
        
    /*begin to get the server's information*/
        
    for( i=0; i<3; i++ )
    {
        if( i == 1) 
        {
            size =  sizeof(DWORD);
            NSFDB2GetServerInfo(server_name,
                                ServerInfo[i],
                                &dwBuffer, &size);
            PRINTLOG("\n======Get ServerInfo %s, return value is:", serverInfoString[i]);
            PRINTLOG("%d\n", dwBuffer);
        }
        else
        {
            size = sizeof(buffer);
            if(err = NSFDB2GetServerInfo(server_name,
                                         ServerInfo[i],
                                         buffer, &size))
            {
                PRINTERROR(err,"NSFDB2GetServerInfo");
                goto EXIT;
            }
            else
            {
                PRINTLOG("\n======Get ServerInfo %s, return value is:", serverInfoString[i]);
                PRINTLOG("%d\n", dwBuffer);
            }
        }

    }

    NSFDbClose(db_handle);

   /*keep in mind that when you are using the API  NSFDB2FastCopy, you must close the sourece db, or you can not
     take it offline*/
    if(err = NSFDB2FastCopy(
                            NULL,                                   /*for federation enabled DB2 database*/
                            NULL,                                   /*ignored presnetly*/
                            DB_NAME,                                /*sourceNSF*/
                            TargetDB_NAME,                          /*targetNSF*/
                            Group_Name,                             /*groupName*/
                            NULL,                                   /*loadParameter should match the LOGPATH setting of 
                                                your database configuration,Passing NULL in this 
                                                parameter will result in the current directory being 
                                                used to place the loadCopyFiles*/
                            10000,                                  /*The amount of time in milliseconds to wait for targetNSF
                                                to be taken offline.  Requires DB2BACKUP_COPY_REPLACE to be specified in flags*/
                            DB2BACKUP_COPY_REPLACE | DB2BACKUP_COPY_ISOLATE,
                            NULL                                    /*(for future use)*/
                            ))                
    {
        PRINTERROR(err,"NSFDB2FastCopy");
        goto EXIT;
    }        
    else
    {
        PRINTLOG("\n======NSFDB2FastCopy successfully\n");        
    }
        
   /*verify the TargetDB_NAME*/
    if(verify(TargetDB_NAME, server_name))
    {
        PRINTLOG("======Not verify, NSFDB2FastCopy doesn't work\n");
        goto EXIT;
    }
    PRINTLOG("======result verifies, succeed!\n");
    NotesTerm();
    return 0;
        
EXIT:        
    NotesTerm();
    return (1);
        
}

int verify(char *targetdb, char *server_name)
{
    char        fullpath_name[200]={0};
    DHANDLE     db_handle;
    if(strlen(server_name))
    {
        OSPathNetConstruct(NULL, server_name, targetdb, fullpath_name);
    }
    else
    {
        strcpy(fullpath_name, targetdb);
    }
    if (NSFDbOpen (fullpath_name, &db_handle)) return 1;
    else return 0;
}
