/*************************************************************************
 *
 * Copyright HCL Technologies 1996, 2025.
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

PROGRAM:    ADMINReqChkAccessNCMoveReplica

FILE:       ADMINReqChkAccessNCMoveReplica.c

PURPOSE:    This program will creates a "Check Access for Non-cluster Move Replica Creation" request in the Administration Requests database (admin4.nsf).

SYNTAX:     ADMINReqChkAccessNCMoveReplica  <source server name> <source database name> <destination server name> <destination full path database name>

*******************************************************************************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* OS and C include files */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* HCL C API for Notes/Domino include files */

#include "global.h"
#include "nsfdb.h"
#include "nsfdata.h"
#include "osmisc.h"
#include "osfile.h"
#include "reg.h"
#include "adminp.h"

#include <osmem.h>
#include <osenv.h>
#include <nsfnote.h>
#include <editods.h>
#include <stdnames.h>
#include <mailserv.h>
#include <mail.h>
#include <textlist.h>

#if defined(CAPI_TESTING) 
#include "printlog.h" 
#else
 #define PRINTLOG printf 
 #define PRINTERROR(api_error, api_name) {\
 char    szErrorText[256] = { 0 };\
 OSLoadString(NULLHANDLE, ERR(api_error), szErrorText, sizeof(szErrorText));\
 fprintf(stderr, "[ERROR]:%s:%d:%s - %s", __FILE__,__LINE__,api_name,szErrorText); }
#endif 

/* conestant types */
 #define     SRCSERVER_NAME_LEN         80
 #define     STDIN                   stdin

/* function prototypes */

void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                               char *SrcServer_name, char *SrcDb_name ,char *DesServer_name, char *DesDb_name);

/* Program declaration */
int main(int argc, char *argv[])
{
    /* Local data declarations */

    DBHANDLE    db_handle = NULLHANDLE;         /* database handle */
    DBHANDLE    srcDB_handle = NULLHANDLE;         /* database handle */
    STATUS      error = NOERROR;                /* error code from API calls */
    char        chAuthorName[MAXUSERNAME+1] = {0};    /* author's name */
    ADMINReqParams arpPtr;                      /* ADMINReqParams structure */
    DBREPLICAINFO  Replicainfo;
    int         argNum = 0;
    char        chSourceServer[SRCSERVER_NAME_LEN] = {0};      /* server names are < 80 chars */
    char        chSourceDBName[MAXPATH] = {0};
    char        chDestinationServer[SRCSERVER_NAME_LEN] = {0};      /* server names are < 80 chars */
    char        chDestinationDBName[MAXPATH] = {0};
    char        *pchSrcServer = NULL;     /* source Server name */
    char        *pchSrcPathName = NULL;   /* source file name */
    char        *pchDesServer = NULL;     /* destination Server name */ 
    char        *pchDesPathName = NULL;   /* destination file name */
    char        admin4db[] = "admin4.nsf";
    char        chTitle[]= "ABD";      /* Title of the database being replicated */
    
    char        admin4DBFullPath[MAXPATH+1] = {0};
    char        srcDBFullPath[MAXPATH+1] = {0};
    
    pchSrcServer = chSourceServer;
    pchSrcPathName = chSourceDBName;
    pchDesServer = chDestinationServer;
    pchDesPathName = chDestinationDBName;

    
    ProcessArgs(argc, argv, pchSrcServer, pchSrcPathName, pchDesServer, pchDesPathName);

    /* Process input arguments */
    error = NotesInitExtended (argc, argv);
    
    /* Initialize the notes */
    if (error)
    {
        PRINTERROR(error, "NotesInitExtended");
        return(1);
    }
    
    /* Construct the path for the admin request file */
    if (error = OSPathNetConstruct(NULL, pchSrcServer, admin4db, admin4DBFullPath))
    {
        PRINTERROR (error,"OSPathNetConstruct");  
        NotesTerm();
        return (error);
    }
    
    /* Open the database. */
    if (error = NSFDbOpen (admin4DBFullPath, &db_handle))
    {
        PRINTERROR (error,"NSFDbOpen");
        NotesTerm();
        return (error);
    }
    
    /* Construct the path for the srcDBFullPath request file */
    if (error = OSPathNetConstruct(NULL, pchSrcServer,pchSrcPathName, srcDBFullPath))
    {
        PRINTERROR (error,"OSPathNetConstruct");
        NSFDbClose (db_handle);		
        NotesTerm();
        return (error);
    }
    
    /* Open the database. */
    if (error = NSFDbOpen (srcDBFullPath, &srcDB_handle))
    {
        PRINTERROR (error,"NSFDbOpen");
        NSFDbClose (db_handle);
        NotesTerm();
        return (error);
    }
    
    /* Copy Replica info and provide new Replica ID as required. */
    if (error = NSFDbReplicaInfoGet (srcDB_handle, &Replicainfo))
    {
        NSFDbClose (srcDB_handle);
        NSFDbClose (db_handle);
        NotesTerm();
        return (error);
    }
    
    /* get the mail server name */
    if (error = SECKFMGetUserName(chAuthorName))
    {
        PRINTERROR(error, "SECKFMGetUserName");
        NSFDbClose (srcDB_handle);
        NSFDbClose (db_handle);
        NotesTerm();
        return (error);
    }
    
    memset(&arpPtr, 0x00, sizeof(arpPtr));
    
    if (error = ADMINReqChkAccessNCMoveReplica(db_handle,
                                            chAuthorName,
                                            pchSrcServer,
                                            srcDBFullPath,
                                            chTitle,
                                            &Replicainfo,
                                            pchDesServer,
                                            pchDesPathName,
                                            &arpPtr,
                                            sizeof(arpPtr)))
    {
        PRINTERROR(error, "ADMINReqChkAccessNCMoveReplica");
        NSFDbClose (srcDB_handle);
        NSFDbClose(db_handle);
        NotesTerm();
        return (error);
    }
    
    PRINTLOG("\n!!! ADMINReqChkAccessNCMoveReplica Processed Sucessfully !!! \n ");
    
    /* Close the srcdatabase. */
    if (error = NSFDbClose (srcDB_handle))
    {
        PRINTERROR (error,"NSFDbClose");
        NSFDbClose (db_handle);
        NotesTerm();
        return (error);
    }

    /* Close the database. */
    if (error = NSFDbClose (db_handle))
    {
        PRINTERROR (error,"NSFDbClose");
        NotesTerm();
        return (error);
    }

    /* Terminate Domino and Notes. */
    NotesTerm();
    /* End of program. */
    return (error);

}

/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:     argc, argv - directly from the command line

    OUTPUTS:    SrcServer_name - source server name obtained from command line or
                              from prompt.
                SrcDb_name -  source database name obtained from command line or
                              from prompt.
                DesServer_name   - destination server name obtained from command line or
                              from prompt. 
                DesDb_name  - destination database name obtained from command line or
                              from prompt.
*************************************************************************/

void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                               char *SrcServer_name, char *SrcDb_name ,char *DesServer_name, char *DesDb_name)
{
    if (argc != 5)
    {

        printf("Enter Source Server name: ");
        fflush (stdout);
        fgets(SrcServer_name, SRCSERVER_NAME_LEN, STDIN);
        printf("\n");
        printf ("Enter source file name:  ");
        fflush (stdout);
        fgets(SrcDb_name, MAXPATH-1, STDIN);
        printf("\n");
        printf ("Enter destination Server name:  ");
        fgets(DesServer_name, MAXPATH-1, STDIN);
        printf("\n");
        printf ("Enter destination fullPath file name:  ");
        fgets(DesDb_name, MAXPATH-1, STDIN);
        printf("\n");
    }
    else
    {
        memset(SrcServer_name, '\0', SRCSERVER_NAME_LEN);    
        strncpy(SrcServer_name, argv[1], SRCSERVER_NAME_LEN-1);
        memset(SrcDb_name, '\0', MAXPATH);    
        strncpy(SrcDb_name, argv[2], MAXPATH-1);
        memset(DesServer_name, '\0', SRCSERVER_NAME_LEN);    
        strncpy(DesServer_name, argv[3], SRCSERVER_NAME_LEN-1);
        memset(DesDb_name, '\0', MAXPATH);    
        strncpy(DesDb_name, argv[4], MAXPATH-1);
    } /* end if */
} /* ProcessArgs */
