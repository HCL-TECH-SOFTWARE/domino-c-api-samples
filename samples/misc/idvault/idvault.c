/*************************************************************************
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

   PROGRAM:    idvault

   FILE:       idvault.c

   PURPOSE:    This programe upload id file to ID vault,gets id file from ID vault,
               synch id file contents with the ID vault



*************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* OS and C include files */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* HCL C API for Notes/Domino include files */

#include "global.h"
#include "idvault.h"
#include "nsfdb.h"
#include "osfile.h"

#ifndef OSMISC_DEFS
#include <osmisc.h>
#endif

#if defined(CAPI_TESTING) 
#include "printlog.h" 
#else
 #define PRINTLOG printf 
 #define PRINTERROR(api_error, api_name) {\
 char    szErrorText[256] = { 0 };\
 OSLoadString(NULLHANDLE, ERR(api_error), szErrorText, sizeof(szErrorText));\
 fprintf(stderr, "[ERROR]:%s:%d:%s - %s", __FILE__,__LINE__,api_name,szErrorText); }
#endif 

/* Program declaration */
int main(int argc, char *argv[])
{
	/* Local data declarations */
         
	STATUS     error = NOERROR;             /* error code from API calls */
	KFHANDLE	hKFC = NULLKFHANDLE;
	KFHANDLE	hKFC2 = NULLKFHANDLE;
	DBHANDLE  hDB;

	char    *ServerName;
	char    *DomainName;
	char	DBName[]="names.nsf";
	char    DBPath[MAXPATH+1];
#ifdef OS400
	char	UserS1ID[] = "/DominoServer/LGCToolkit/people/s1.id";
	char	UserS2ID[] = "/DominoServer/LGCToolkit/people/s2.id";
	char	UserS3ID[] = "/DominoServer/LGCToolkit/people/s3.id";
	char	UserSSID[] = "/DominoServer/LGCToolkit/people/ss.id";
#else
	char	UserS1ID[] = "s1.id";
	char	UserS2ID[] = "s2.id";
	char	UserS3ID[] = "s3.id";
	char	UserSSID[] = "ss.id";
#endif
	char	UserNameS1[MAXUSERNAME] = "CN=s1/O=";
	char	UserNameS2[MAXUSERNAME] = "CN=s2/O=";
	char	UserNameS3[MAXUSERNAME] = "CN=s3/O=";
	char	UserNameSS[MAXUSERNAME] = "CN=ss/O=";
	char	PASSWORD[]="123456";
	DWORD	VaultFlags = 0;  
	WORD    wFlags = 0;
	char    UserName[MAXUSERNAME] = { 0 };
	LICENSEID licenseID;
    
	if(argc != 3)
	{
		PRINTLOG( "\nUsage:  %s  <server name> <domain>\n", argv[0] );
		return (0);
	}

	/* init ... */
	ServerName = argv[1];
	DomainName = argv[2];	
	strcat(UserNameS1,DomainName);
	strcat(UserNameS2,DomainName);
	strcat(UserNameS3,DomainName);
	strcat(UserNameSS,DomainName);


        if (error = NotesInitExtended (argc, argv))
        {
           fprintf (stderr, "\nError initializing Notes.\n");
           return (1);
        }

	if ( error = OSPathNetConstruct( NULL, ServerName,DBName,DBPath) ){
	    PRINTERROR (error,"OSPathNetConstruct");
	    NotesTerm();
	    return (1);
	}

	/*Must be call NSFDbOpen() for init before execute other id vault functions.*/	
	if ( error = NSFDbOpen (DBPath, &hDB) ){
	    PRINTERROR (error,"NSFDbOpen");
	    NotesTerm();
	    return (1);
	}

	error = SECKFMOpen (&hKFC, UserS1ID, PASSWORD, SECKFM_open_All, 0, NULL);
	if ( error ){
	     PRINTERROR (error,"SECKFMOpen");
             NSFDbClose(hDB);
             NotesTerm();
             return (1);
	}

        /*
	 * Get ID flags information associated to keyfile context.
	 * SECKFMGetIDFlags - This routine provides access to ID flags
	 * associated with the keyfile
	 * context that is passed in. This API can return below flags,
	 * fIDFH_Password    0x0001 which indicates that file is password protected.
	 * fIDFH_PWShareable 0x0008 which indicates that password may be shared by all process.
	 * Information on these flags can be seen in kfm.h file.
	 *
	 */
         error = SECKFMGetIDFlags(hKFC, &wFlags);
         if ( error ){
              PRINTERROR (error,"SECKFMGetIDFlags");
         }
         else {
             PRINTLOG("SECKFMGetIDFlags returned ID flags %d.\n", wFlags);
         }

        if ( fIDFH_Password & wFlags ){
             PRINTLOG("SECKFMGetIDFlags returned ID as password protected.\n");
        }
  
       /* Get user info using SECKFMGetUserInfo API */
       error = SECKFMGetUserInfo(hKFC, UserName, &licenseID);
       if ( NOERROR != error ){
            PRINTERROR (error, "SECKFMGetUserInfo");
       }
       else {
            PRINTLOG("SECKFMGetUserInfo returned username %s.\n", UserName);
       }

	/* Put ID file using the hKFC */
	error = SECidfPut (UserNameS1, 
			   PASSWORD,
			   UserS1ID,
			   &hKFC, 
			   ServerName,
			   0,
			   0,
			   NULL);
	
	if ( error ){
	     PRINTERROR (error,"SECidfPut");
	     SECKFMClose (&hKFC, SECKFM_close_WriteIdFile, 0, NULL);
             NSFDbClose(hDB);
             NotesTerm();
             return (1);

	}
	PRINTLOG("Put ID file using the hKFC...OK\n");

	SECKFMClose (&hKFC, SECKFM_close_WriteIdFile, 0, NULL);

        error = SECKFMOpen (&hKFC, UserS2ID, PASSWORD, SECKFM_open_All, 0, NULL);
	if ( error ){
             PRINTERROR (error,"SECKFMOpen");
             NSFDbClose(hDB);
             NotesTerm();
             return (1);

	}
	/* Put ID file using local file name and hKFC - only hKFC should be used */
 	error = SECidfPut (UserNameS2, 
			   PASSWORD,
			   NULL, 
			   &hKFC, 
			   ServerName, 
			   0, 
			   0,
			   NULL);
	if ( error ){
             PRINTERROR (error,"SECidfPut");
	     SECKFMClose (&hKFC, SECKFM_close_WriteIdFile, 0, NULL);
             NSFDbClose(hDB);
             NotesTerm();
             return (1);

	}

	PRINTLOG("Put ID file using local file name and hKFC - only hKFC should be used...OK\n");
	SECKFMClose (&hKFC, SECKFM_close_WriteIdFile, 0, NULL);

        error = SECKFMOpen (&hKFC, UserS3ID, PASSWORD, SECKFM_open_All, 0, NULL);
	if ( error ){
             PRINTERROR (error,"SECKFMOpen");
             NSFDbClose(hDB);
             NotesTerm();
             return (1);

	}
	
	/* Put ID file using local file name */
	error = SECidfPut (UserNameS3, 
			   PASSWORD,
			   UserS3ID, 
			   NULL, 
			   ServerName, 
			   0, 
			   0,
			   NULL);

	if ( error ){
             PRINTERROR (error,"SECidfPut");
             SECKFMClose (&hKFC, SECKFM_close_WriteIdFile, 0, NULL);
             NSFDbClose(hDB);
             NotesTerm();
             return (1);

	}

	PRINTLOG("Put ID file using local file name...OK\n");
	SECKFMClose (&hKFC, SECKFM_close_WriteIdFile, 0, NULL);

	/* Get id file to local file name and hKFC */
	error = SECidfGet (UserNameS1, 
			PASSWORD,
			UserS1ID, 
			&hKFC2, 
			ServerName, 
			0,
			0,
			NULL);
	if ( error ){
             PRINTERROR (error,"SECidfGet");
             NSFDbClose(hDB);
             NotesTerm();
             return (1);

	}
	PRINTLOG("Get id file to local file name and hKFC...OK\n");


	SECKFMClose (&hKFC2, SECKFM_close_WriteIdFile, 0, NULL);


	/* Get id file to just hKFC*/
	error = SECidfGet (UserNameS2, 
			PASSWORD,
			NULL, 
			&hKFC2, 
			ServerName, 
			0,
			0,
			NULL);
	if ( error ){
             PRINTERROR (error,"SECidfGet");
             NSFDbClose(hDB);
             NotesTerm();
             return (1);

	}

	PRINTLOG("Get id file to just hKFC...OK\n");
	SECKFMClose (&hKFC2, SECKFM_close_WriteIdFile, 0, NULL);



	/* Get id file to just local file name */
	error = SECidfGet (UserNameS3, 
			PASSWORD,
			UserS3ID,  
			NULL, 
			ServerName, 
			0,
			0,
			NULL);
	if ( error ){
             PRINTERROR (error,"SECidfGet");
             NSFDbClose(hDB);
             NotesTerm();
             return (1);

	}

	PRINTLOG("Get id file to just local file name...OK\n");

	error = SECKFMOpen (&hKFC2, UserS1ID, PASSWORD, SECKFM_open_All, 0, NULL);
	if ( error ){
             PRINTERROR (error,"SECKFMOpen");
             NSFDbClose(hDB);
             NotesTerm();
             return (1);

	}

	/* Sync the hKFC with the ID file in the Vault and write new hKFC */
	error = SECidfSync (UserNameS1, PASSWORD,
			NULL,
			&hKFC2, 
			ServerName, 
			0, 
			0,
			NULL, 
			&VaultFlags);
	if ( error ){
             PRINTERROR (error,"SECidfSync");
             SECKFMClose (&hKFC2, SECKFM_close_WriteIdFile, 0, NULL);
             NSFDbClose(hDB);
             NotesTerm();
             return (1);

	}
	PRINTLOG("Sync the hKFC with the ID file in the Vault and write new hKFC...OK\n");

	/* Sync the ID file with the ID file in the Vault */
	error = SECidfSync (UserNameS1, PASSWORD,
			UserS1ID, 
			NULL,
			ServerName, 
			0,
			0, 
			NULL, 
			&VaultFlags);
        if ( error ){
             PRINTERROR (error,"SECidfSync");
             SECKFMClose (&hKFC2, SECKFM_close_WriteIdFile, 0, NULL);
             NSFDbClose(hDB);
             NotesTerm();
             return (1);

        }
	PRINTLOG("Sync the ID file with the ID file in the Vault ...OK\n");

	/* Sync the hKFC with the ID file in the Vault and hKFC */
        error = SECidfSync (UserNameS1, PASSWORD,
			UserS1ID, 
			&hKFC2, 
			ServerName, 
			0,
			0,
			NULL, 
			&VaultFlags);
        if ( error ){
             PRINTERROR (error,"SECidfSync");
             SECKFMClose (&hKFC2, SECKFM_close_WriteIdFile, 0, NULL);
             NSFDbClose(hDB);
             NotesTerm();
             return (1);

        }
        PRINTLOG("Sync the hKFC with the ID file in the Vault and hKFC...OK\n");

        SECKFMClose (&hKFC2, SECKFM_close_WriteIdFile, 0, NULL);


        /* Terminate Domino and Notes. */



        NSFDbClose(hDB);

        NotesTerm();

        /* End of intro program. */
        PRINTLOG("\nProgram completed successfully.\n");
        return (0);
}

