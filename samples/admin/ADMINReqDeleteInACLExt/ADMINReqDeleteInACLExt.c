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

PROGRAM:    ADMINReqDeleteInACLExt

FILE:       ADMINReqDeleteInACLExt.c

PURPOSE:    This program will creates a "Delete in Access Control List" request in the Administration Requests database (admin4.nsf).

SYNTAX:     ADMINReqDeleteInACLExt  <server name> <database filename>.<mail flag>.<idvault flag>

*************************************************************************************************/
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

/* NOTE: This code MUST be the LAST file included so that ascii versions of the system APIs are used     */
#if defined(OS390) && (__STRING_CODE_SET__==ISO8859-1 /* If os390 ascii compile                          */)     
#include <_Ascii_a.h>
#endif

/* conestant types */
 #define     SERVER_NAME_LEN         80
 #define     STDIN                   stdin
 #define     FLAG_LEN                2

/* function prototypes */

void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                               char *server_name, char *db_name, char *mail_flag ,char *idvault_flag);

/* Program declaration */
int main(int argc, char *argv[])
{
    /* Local data declarations */

    DBHANDLE    db_handle = NULLHANDLE;         /* database handle */
    STATUS      error = NOERROR;                /* error code from API calls */
    char        chAuthorName[MAXUSERNAME+1] = {0};    /* author's name */
    char        chUserName[MAXUSERNAME+1] = "CN=C user/O=HCL";        /* user ACL that must be deleted */
    char        chMailServerName[MAXPATH] = {0};        /* mail server name */
    char        chMailFileName[MAXPATH] = "mail\\cuser.nsf";    /* mail file name */
    char        *chMailFileFlag = NULL;         /* "0" = Don't delete mail file */
                                                /* "1" = Delete just mail file specified in person record */
                                                /* "2" = Delete mail file specified in person record & all replicas */
                                            
    char        *chIDVaultFlag  = NULL;         /* "0" = Don't delete, just mark as inactive */
                                                /* "1" = Delete the user from the vault */
                                                /* "2" = Don't do anything with the user's ID in the vault */
                                            
    char        *chIDVaultName = "O=BlueVault";   /* IDVault Name */
    ADMINReqParams ARPptr;                      /* ADMINReqParams structure */
    int         ArgNum = 0;
    char        szServer[SERVER_NAME_LEN] = {0};      /* server names are < 80 chars */
    char        szDBName[MAXPATH] = {0};
    char        *serverName = NULL;
    char        *admin4db = NULL;
    char        mailFlag[2] = {0};
    char        idVaultFlag[2] = {0};
    char        admin4DBFullPath[MAXPATH+1] = {0};
    
    serverName = szServer;
    admin4db = szDBName;
    chMailFileFlag = mailFlag;
    chIDVaultFlag = idVaultFlag;

    
    ProcessArgs(argc, argv, serverName, admin4db, chMailFileFlag, chIDVaultFlag);

    
    /* Process input arguments */
    error = NotesInitExtended (argc, argv);
    
    /* Initialize the notes */
    if (error)
    {
        PRINTERROR(error, "NotesInitExtended");
        return(error);
    }
    
    /* Construct the path for the admin request file */
    if (error = OSPathNetConstruct(NULL, serverName, admin4db, admin4DBFullPath))
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
    
    /* get the mail server name */
    if (error = SECKFMGetUserName(chAuthorName))
    {
        PRINTERROR(error, "SECKFMGetUserName");
        NSFDbClose (db_handle);
        NotesTerm();
        return (error);
    }
    
    strncpy(chMailServerName, serverName, MAXPATH);
    memset(&ARPptr, 0x00, sizeof(ARPptr));
    ARPptr.dwDeleteInNABType = DELETE_PERSON_IN_NAB;
   
    if (error = ADMINReqDeleteInACLExt(db_handle,
                                       chAuthorName,
                                       chUserName,
                                       chMailServerName,
                                       chMailFileName,
                                       chMailFileFlag,
                                       chIDVaultFlag,
                                       chIDVaultName,
                                       &ARPptr,
                                       sizeof(ARPptr)))
    {
        PRINTERROR(error, "ADMINReqDeleteInACLExt");
        NotesTerm();
        NSFDbClose(db_handle);
        return (error);
    }
    
    PRINTLOG("\n!!! ADMINReqDeleteInACLExt Processed Sucessfully !!! \n ");

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

    OUTPUTS:    server_name - server name obtained from command line or
                              from prompt.
                db_name -     database name obtained from command line or
                              from prompt.
                mail_flag   - mailflag name obtained from command line or
                              from prompt.
                idvault_flag  - vaultflag name obtained from command line or
                              from prompt.

*************************************************************************/

void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                               char *server_name, char *db_name ,char *mail_flag ,char *idvault_flag)
{
    if (argc != 5)
    {

        printf("Enter server name: ");
        fflush (stdout);
        fgets(server_name, SERVER_NAME_LEN, STDIN);
        printf("\n");
        printf ("Enter database filename:  ");
        fflush (stdout);
        fgets(db_name, MAXPATH-1, STDIN);
        printf("\n");
        printf ("Enter mail flag [0 or 1 or 2]:  ");
        fflush (stdout);
        fgets(mail_flag, FLAG_LEN-1, STDIN);
        printf("\n");
        printf ("Enter idvault flag [0 or 1 or 2]:  ");
        fflush (stdout);
        fgets(idvault_flag, FLAG_LEN-1, STDIN);

    }
    else
    {
        memset(server_name, '\0', SERVER_NAME_LEN);    
        strncpy(server_name, argv[1], SERVER_NAME_LEN-1);
        memset(db_name, '\0', MAXPATH);    
        strncpy(db_name, argv[2], MAXPATH-1);
        memset(mail_flag, '\0', FLAG_LEN);
        strncpy(mail_flag, argv[3], FLAG_LEN-1);
        memset(idvault_flag, '\0', FLAG_LEN);
        strncpy(idvault_flag, argv[4], FLAG_LEN-1);
    } /* end if */
} /* ProcessArgs */
