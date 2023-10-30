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

    PROGRAM:    userreg

    FILE:       userreg.c

    SYNTAX:     userreg  <server's distinguished name>

    PURPOSE:    demonstrates the User Registration capabilities

    DESCRIPTION:
        This program demonstrates the User Registration capabilities of
        the HCL C API for Notes/Domino.

        First, a new Organization certifier, ABCorp, is created.  Then,
        a new Organization Unit certifier, Sales, is created.  This
        certifier is a descendent of the Organization certifier.  Then,
        a new server, OTTO, is registered and certified with the
        Organization certifier.  Last, a new user, Jayne Doe is registered
        and certified with the Organization Unit certifier.

        Each of these steps create ID files. If this program is run under
        Win32, these ID files are written to disk A:.  If this program is
        run under Unix, these files are written to /tmp.

        Each of these steps create entries in the Name & Address book.
        Therefore, you must have proper access to the Name & Address
        book on the server specified on the command line in order for
        this program to succeed.

        The entries for the Organization certifier, Organization Unit
        certifier, new server, and new user are then looked up in the
        Name and Address book and their Note IDs are printed out.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* OS and C include files */

#include <stdio.h>
#include <string.h>

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <names.h>
#include <nsfdb.h>
#include <reg.h>
#include <osmisc.h>
#include <misc.h>
#include <ostime.h>
#include "../../printLog.h"
#if defined(OS400)
#include "lapiplat.h"
#endif

/* symbolic constants */
char ORG_CERTNAME[] = "ABCorp";
char US[] = "US";
char PASSWORD[] = "abcorp";
char ORGUNIT_CERTNAME[] = "Sales";
char NEW_SERVNAME[] = "OTTO";
char NEW_USERNAME[] = "Jayne Doe";
char SERVER_TITLE[] = "Corporate Server";  /* server title */
char ORG_UNIT[] = "Inside Sales";          /* Org Unit - provides uniqueness to the name */
char LAST_NAME[] = "Doe";                  /* Last name */
char FIRST_NAME[] = "Jayne";               /* First name */
char CERTS_FLD[] = "$Certifiers";
char SERVERS_FLD[] = "$Servers";
char USERS_FLD[] = "$Users";

/* symbolic constants that depend on the enviroment */
#if defined(DOS) || defined(NT) /* FAT names on A: disk */
    char ORG_CERT_ID[] = "C:\\Program Files\\HCL\\Domino\\Data\\cert.id";
    char ORGUNIT_CERT_ID[] = "C:\\testorg.id";
    char SERVER_ID[] = "C:\\Program Files\\HCL\\Domino\\Data\\server.id";
    char USER_ID[] = "C:\\JDOE.ID";
    char MAILFILENAME[] =  "MAIL\\JDOE.NSF";
//#elif defined(UNIX) && !defined(OS400)        /* UNIX names in /tmp dir */
#elif defined(UNIX) || defined(MAC)        /* UNIX names in /tmp dir */
    char ORG_CERT_ID[] = "./cert.id";
    char ORGUNIT_CERT_ID[] = "./testorg.id";
    char SERVER_ID[] = "./server.id";
    char USER_ID[] = "./jdoe.id";
    char MAILFILENAME[] =  "mail/jdoe.nsf";
#elif defined (OS400)                        /* i5/OS names in default data dir */
    char ORG_CERT_ID[] = "abcorp.id";
    char ORGUNIT_CERT_ID[] = "absales.id";
    char SERVER_ID[] = "otto.id";
    char USER_ID[] = "jdoe.id";
    char MAILFILENAME[] = "mail/jdoe.nsf";
#endif

char DOMAIN_NAME[] = "ABC";
#define MIN_PASS_LEN 3

char DNAME_ORG_CERT[] = "O=ABCorp/C=US";
char DNAME_ORGUNIT_CERT[] = "OU=Sales/O=ABCorp/C=US";
char DNAME_OTTO[] = "CN=OTTO/O=ABCorp/C=US";
char DNAME_JAYNE[] =  "CN=Jayne Doe/OU=Inside Sales/OU=Sales/O=ABCorp/C=US";

char NA_NAME[] = "names.nsf";

/* local function prototypes */
void LNCALLBACK REGCallback (char far *);
STATUS GetCertCtx (char far *, HCERTIFIER *, char far *);


TIMEDATE ExpireDate;
BOOL LocalServer;

/************************************************************************

    FUNCTION:   Main

    PURPOSE:    HCL C API for Domino and Note subroutine

*************************************************************************/

int main(int argc, char *argv[])
{
    STATUS   error;              /* return code from API calls */
    char     *ServName;          /* server name entered on the command line */
    char     MailServName[MAXUSERNAME + 1]; /* mail server name */
    char     FullDBPath[MAXPATH];  /* complete pathname for Name and Address
                                     Book */
    char     ServLocation[MAXLOCATIONNAME] = "Sales LAB";
    char     WorkLocation[MAXLOCATIONNAME] = "323 West";

    HCERTIFIER hCertCtx;
    DBHANDLE hNABook;              /* handle to name and address book */
    NOTEID NoteID;

   /* if no server name on the command line, assume local machine */
   if (argc == 1)
    {
        ServName = '\0';
        LocalServer = TRUE;
    }
    else if (argc == 2) /* else we have the remote server name */
    {
        ServName = argv[1];
        strcpy(MailServName, ServName);
        LocalServer = FALSE;
    }

    FullDBPath[0] = '\0';

    error = NotesInitExtended (argc, argv);
    if (error)
    {
        PRINTLOG("Error: Unable to initialize Notes.\n");
        return (1);
    }

    PRINTLOG ("\n");
    /* Create and register new organizational certifier - ABCorp */
    error = REGNewCertifier (
                             NULLHANDLE,            /* no certifier context */
                             KFM_IDFILE_TYPE_ORG,   /* Organizational certifier */
                             ServName,              /* registration server */
                             US,                  /* country code - optional */
                             ORG_CERTNAME,          /* Organization */
                             NULL,                  /* Org Unit */
                             PASSWORD,              /* password */
                             ORG_CERT_ID,           /* ID file for new certifier */
                             NULL,                  /* location of certifier - optional */
                             NULL,                  /* comment - optional */
                             NULL,                  /* Mail Server's name - optional */
                             NULL,                  /* Mail file's name - optional */
                             NULL,                  /* no forwarding address */
                             fREGCreateIDFileNow |  /* flags - only create ID file */
                             fREGUSARequested |
                             fREGCreateAddrBookEntry|
                             fREGOkayToModifyID|
                             fREGOkayToModifyAddrbook,
                             0,                     /* key width (must be 0) */
                             MIN_PASS_LEN,          /* minimum password length */
                             &REGCallback,          /* pointer to callback function */
                             FullDBPath);           /* returned pathname of file where
                                        error occurred */

    PRINTLOG ("\n\n");
   

    if (error)
    {
        PRINTLOG("Error: unable to register new organizational certifier.\n");
        if (FullDBPath[0] != '\0')
            PRINTLOG ("Error occurred in file %s\n", FullDBPath);
        PRINTERROR(error,"REGNewCertifier");
        NotesTerm();
        return (1);
   }

    /* Prepare to call REGNewCertifier to create and register a new
       organizational unit certifier - Sales org unit of ABCorp.
       First get the Organization Certifier context for ABCorp, then pass
       this context as input to REGNewCertifier.
    */

    if (error = GetCertCtx(ORG_CERT_ID, &hCertCtx, PASSWORD))
    {
        PRINTERROR(error,"GetCertCtx");
        NotesTerm();
        return (1);
    }

    error = REGNewCertifier (
                             hCertCtx,              /* certifier context */
                             KFM_IDFILE_TYPE_ORGUNIT, /* Organizational Unit certifier */
                             ServName,              /* registration server */
                             NULL,                  /* country code - optional*/
                             NULL,                  /* Organization - Just use the above
                                        certifier context */
                             ORGUNIT_CERTNAME,      /* Org Unit */
                             PASSWORD,              /* password */
                             ORGUNIT_CERT_ID,       /* ID file for new certifier */
                             NULL,                  /* location of certifier - optional */
                             NULL,                  /* comment - optional*/
                             NULL,                  /* Mail Server's name - optional */
                             NULL,                  /* Mail file's name - optional */
                             DNAME_JAYNE,           /* forwarding address */
                             fREGCreateIDFileNow |  /* flags */
                             fREGUSARequested    |
                             fREGCreateAddrBookEntry|
                             fREGOkayToModifyID|
                             fREGOkayToModifyAddrbook,
                             0,                     /* key width (must be 0) */
                             MIN_PASS_LEN,          /* minimum password length */
                             &REGCallback,          /* pointer to callback function */
                             FullDBPath);           /* returned pathname of file where
                                        error occurred */

    PRINTLOG ("\n\n");
    if (error)
    {
        PRINTLOG("Error: unable to register new organizational unit certifier.\n");
        if (FullDBPath[0] != '\0')
            PRINTLOG ("Error occurred in file %s\n", FullDBPath);
        SECKFMFreeCertifierCtx (hCertCtx);
        PRINTERROR(error,"REGNewCertifier");
        NotesTerm();
        return (1);
    }

    /* Create and Register a new server.  Certify server with the Organization
       certifier created earlier (use the above Organization certifier
       context).
    */
#if 0
    /* 
     We have commented out code to make the sample run manually. 
     As with the previous code , 
     it was unable to register a new server using the current server ID.
    */

   //error = REGNewServer (
   //           hCertCtx,              /* certifier context */
   //           KFM_IDFILE_TYPE_DERIVED, /* derived from certifier context */
   //           ServName,              /* registration server */
   //           NULL,                  /* Org Unit - optional */
   //           NEW_SERVNAME,          /* name of this new server */
   //           PASSWORD,              /* password */
   //           SERVER_ID,             /* ID file for new server */
   //           ServLocation,          /* location of server - optional */
   //           NULL,                  /* comment - optional*/
   //           DOMAIN_NAME,           /* Domino domain of new server */
   //           NULL,                  /* network name - optional */
   //           NEW_USERNAME,          /* administrator's name - optional */
   //           SERVER_TITLE,       /* server title */
   //           fREGCreateIDFileNow |  /* flags */
   //           fREGUSARequested    |
   //           fREGCreateAddrBookEntry|
   //			  fREGOkayToModifyID|
   //				  fREGOkayToModifyAddrbook,
   //           MIN_PASS_LEN,           /* minimum password length */
   //           &REGCallback,           /* pointer to callback function */
   //           FullDBPath);            /* returned pathname of file where
   //                                      error occurred */

   //printf ("\n\n");

   /* Free the certifier context */
   //SECKFMFreeCertifierCtx (hCertCtx);
   /*
   if (error)
   {
      printf("Error: unable to register new server.  Error:%x\n", error);
      if (FullDBPath[0] != '\0')
         printf ("Error occurred in file %s\n", FullDBPath);
      PrintAPIError(error);
      NotesTerm();
      return (1);
   }
   */
#endif

  /* Prepare to call REGNewWorkstation() to create and register a new user.
     First get the Organization Unit Certifier context for ABCorp,
     Sales Unit. Then, pass this certifier context as input to
     REGNewWorkstation(). It will certify the new user with this certifier.
   */
    if (error = GetCertCtx(ORGUNIT_CERT_ID, &hCertCtx, PASSWORD))
    {
        PRINTERROR(error,"GetCertCtx");
        NotesTerm();
        return (1);
    }

    /* if local machine, need to get the User Name of this workstation */
    if (LocalServer)
    {
        if (error = SECKFMGetUserName(MailServName))
        {
            PRINTERROR(error,"SECKFMGetUserName");
            NotesTerm();
            return (1);
        }
    }

    error = REGNewWorkstation (
                               hCertCtx,              /* certifier context */
                               KFM_IDFILE_TYPE_DERIVED, /* derived from certifier context */
                               ServName,              /* Registration server */
                               ORG_UNIT,              /* Org Unit - provides uniqueness to the name */
                               LAST_NAME,             /* Last name */
                               FIRST_NAME,            /* First name */
                               NULL,                  /* no middle initial */
                               NULL,                  /* no password initially */
                               USER_ID,               /* ID file name */
                               WorkLocation,          /* location - optional */
                               NULL,                  /* comment - optional */
                               MAILSYSTEM_NOTES,      /* mail system  */
                               MailServName,          /* mail server name */
                               MAILFILENAME,          /* pathname of mail file */
                               NULL,                  /* forward address - optional */
                               fREGCreateIDFileNow |  /* flags */
                               fREGUSARequested    |
                               fREGCreateMailFileNow |
                               fREGCreateAddrBookEntry|
                               fREGOkayToModifyID|
                               fREGOkayToModifyAddrbook,
                               0,                      /* minimum password length */
                               &REGCallback,           /* pointer to callback function */
                               FullDBPath);            /* returned pathname of file where
                                         error occurred */

    PRINTLOG ("\n\n");

    /* Free the certifier context */
    SECKFMFreeCertifierCtx (hCertCtx);

    if (error)
    {
        PRINTLOG("Error: unable to register a new user. Error:%x\n", error);
        PRINTERROR(error,"REGNewWorkstation");
        if (FullDBPath[0] != '\0')
            PRINTLOG ("Error occurred in file %s\n", FullDBPath);
        NotesTerm();
        return (1);
    }


    /* Prepare to call REGReCertifyID() to recertify a new user.
      First get the Organization Unit Certifier context for ABCorp,
      Sales Unit. Then, pass this certifier context as input to
      REGReCertify(). It will recertify the new user with this certifier.
    */
    if (error = GetCertCtx(ORGUNIT_CERT_ID, &hCertCtx, PASSWORD))
    {
        PRINTERROR(error,"GetCertCtx");
        NotesTerm();
        return (1); 
    }

    PRINTLOG("Begin recertifying %s...\n", NEW_USERNAME);

    error = REGReCertifyID (
                            hCertCtx,              /* certifier context */
                            0,                     /* Spare1 - reserved */
                            ServName,              /* Registration server */
                            NULL,                  /* Org Unit name */
                            USER_ID,               /* ID file name */
                            &ExpireDate,           /* Expiration Date */
                            0,                     /* Spare2 - reserved */
                            NULL,                  /* Spare3 - reserved */
                            NULL,                  /* pointer to callback function - optional */
                            NULL);                 /* Error path name */

    if (!error)
        PRINTLOG("%s successfully recertified.\n", NEW_USERNAME);

    PRINTLOG ("\n\n");

    /* Free the certifier context */
    SECKFMFreeCertifierCtx (hCertCtx);

    if (error)
    {
        PRINTLOG("Error: unable to recertify a new user. Error:%x\n", error);
        PRINTERROR(error,"REGReCertifyID");
        if (FullDBPath[0] != '\0')
            PRINTLOG ("Error occurred in file %s\n", FullDBPath);
        NotesTerm();
        return (1);
    }


    /* Find the new entries in the Name and Address book */

    /* Construct the path for the Name and Address book */

    if (error = OSPathNetConstruct(NULL, ServName, NA_NAME, FullDBPath))
    {
        PRINTLOG("Error: unable to construct network path to N&A book.\n");
        PRINTERROR(error,"OSPathNetConstruct");
        NotesTerm();
        return (1);
    }

    /* Open the database. */

    if (error = NSFDbOpen (FullDBPath, &hNABook))
    {
        PRINTLOG("Error: unable to open N&A book '%s'.\n", FullDBPath);
        PRINTERROR(error,"NSFDbOpen");
        NotesTerm();
        return (1);
    }

    /* Look up the Organization certifier entry */
    if (error = REGFindAddressBookEntry (hNABook,
                                         CERTS_FLD,
                                         DNAME_ORG_CERT,
                                         &NoteID))
    {
        PRINTLOG("Unable to find orgainizational certifier entry in N&A book.\n");
        PRINTERROR(error,"REGFindAddressBookEntry");
        NSFDbClose(hNABook);
        NotesTerm();
        return (1);
    }

    if (NoteID)
       PRINTLOG (
                 "\nOrganization Certifier, %s, found in NA Book.\nNoteID = %#lX\n\n",
                 DNAME_ORG_CERT, NoteID);

   /* Look for the Org Unit certifier entry */
   if (error = REGFindAddressBookEntry (hNABook,
                                        CERTS_FLD,
                                        DNAME_ORGUNIT_CERT,
                                        &NoteID))
    {
        PRINTLOG("Unable to find orgainizational unit certifier entry in N&A book.\n");
        PRINTERROR(error,"REGFindAddressBookEntry");
        NSFDbClose(hNABook);
        NotesTerm();
        return (1);
    }

    if (NoteID)
        PRINTLOG (
                  "\nOrg Unit Certifier, %s, found in NA Book.\nNoteID = %#lX\n\n",
                  DNAME_ORGUNIT_CERT, NoteID);

   /* Look for the new server entry */
   /*
    We have commented out code to make the sample run manually.
    As with the previous code ,
    it was unable to register a new server using the current server ID.
    */
   /*
   if (error = REGFindAddressBookEntry (hNABook,
                                        SERVERS_FLD,
                                        DNAME_OTTO,
                                        &NoteID))
    {
        PRINTLOG("Error: unable to find server entry in N&A book.\n");
        PrintAPIError(error);
        NSFDbClose(hNABook);
        NotesTerm();
        return (1);
    }

   if (NoteID)
      printf ("\nServer, %s, found in NA Book.\nNoteID = %#lX\n\n",
              DNAME_OTTO, NoteID);
    */
   /* Look for the new user entry */

   if (error = REGFindAddressBookEntry (hNABook,
                                        USERS_FLD,
                                        DNAME_JAYNE,
                                        &NoteID))
    {
        PRINTLOG("Unable to find new user entry in N&A book.\n");
        PRINTERROR(error,"REGFindAddressBookEntry");
        NSFDbClose(hNABook);
        NotesTerm();
        return (1);
    }

    if (NoteID)
        PRINTLOG ("\nUser, %s, found in NA Book.\nNoteID = %#lX\n\n",
                  DNAME_JAYNE, NoteID);

    /* Close the database. */

    if(error = NSFDbClose (hNABook))
    {
        PRINTERROR(error,"NSFDbClose");
        NotesTerm();
        return (1);
    }
    NSFDbDelete(MAILFILENAME);
    NotesTerm();

/* End of main routine */
    PRINTLOG("\nProgram completed successfully.\n");

    return (0);
}


/************************************************************************

    FUNCTION:    REGCallback

    PURPOSE:     callback function - displays status during registration

    INPUT:       msg   - message to be displayed

*************************************************************************/

void LNCALLBACK REGCallback (char far *msg)
{
    PRINTLOG("%s\n",msg);
}

/************************************************************************

    FUNCTION:    GetCertCtx

    PURPOSE:     returns a handle to the certifier context.

    INPUTS:
        pCertFilePath - pointer to full path spec of certifier id file
        szPassword - password

    OUTPUTS:
        phCertCtx     - pointer to handle to Certifier context

    RETURNS:    status code from API Call

*************************************************************************/

STATUS GetCertCtx (char far *pCertFile, HCERTIFIER *phCertCtx,
                   char far *szPassword)
{
    TIMEDATE ExpDate;
    STATUS error;
    char CertName[MAXSPRINTF];    /* Note:  Org names may be up to 64 chars
                                   The CertName in this example is an org
                                   name plus a  country code. */
    BOOL IsHierarchical;
    WORD FileVersion;
    KFM_PASSWORD KFMPassword, *pKFMPassword;    /* encoded password */

    OSCurrentTIMEDATE(&ExpDate);

    /* set the expiration date to two years from today (Domino and Notes default) */
    error = TimeDateAdjust(&ExpDate, 0, 0, 0, 0, 0, 2);
    ExpireDate = ExpDate;

    if (error)
        return (error);

   /* get the encoded password */
    if (szPassword == NULL)
        pKFMPassword = NULL;
    else
    {
        pKFMPassword = &KFMPassword;
        SECKFMCreatePassword (szPassword, pKFMPassword);
    }

   /* get the certifier context */
    error = SECKFMGetCertifierCtx (
                                   pCertFile,       /* Certifier file path spec */
                                   pKFMPassword,    /* encoded password */
                                   NULL,            /* no certifier log */
                                   &ExpDate,        /* expiration date */
                                   CertName,        /* returned name of certifier */
                                   phCertCtx,       /* returned handle to certifier context */
                                   &IsHierarchical, /* returned TRUE if certifier is
                                   hierarchical */
                                   &FileVersion);   /* returned file version */

    return (error);
}
#ifdef __cplusplus
}
#endif
