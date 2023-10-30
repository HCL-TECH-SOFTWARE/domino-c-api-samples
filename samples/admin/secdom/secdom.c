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

    PROGRAM:    SECDOM

    FILE:       SECDOM.C (main program)

    PURPOSE:    C API Sample program that illustrates how to create a
                library that will, from the web, authenticate a Domino
                 user through his Operating System user account via DSAPI.

*************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/* Input and output include files */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>



/* Notes SDK include files */
#include "global.h"
#include "osmem.h"
#include "lookup.h"
#include "dsapi.h"
#include "addin.h"
#include "../../printLog.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif


#define MAX_BUF_LEN 512
#define USER_DOMAIN_SEPARATOR "@"

/*---
*      local procedure prototypes
*/

/* Notes SDK unix shared library entrypoint */
STATUS FAR PASCAL MainEntryPoint (void);

/* Routines with syntax dictated by the DSAPI interface */
unsigned int Authenticate(FilterContext* context, FilterAuthenticate* authData);

/* Retrieval of names from Notes name and address book */
int getUserNames (FilterContext* context,
                  char *userName,
                  char **pUserFullName,
                  int  *pUserFullNameLen,
                  char **pUserShortName,
                  int  *pUserShortNameLen);

int getLookupInfo (FilterContext* context,
                   char *pMatch,
                   unsigned short itemNumber,
                   char **pInfo,
                   int  *pInfoLen);

int doAuthenticate(char *userName, char *domain, char *password);

#ifdef LINUX

int unixAuthenticate(char *userName, char *password);
#else
int separateUsernameAndDomainname(char *userName,char *separator,char **user,char**domain);
int winAuthenticate(char *userName, char *domain, char *password);
#endif

/*---
*      local procedures follow
*/

STATUS FAR PASCAL MainEntryPoint (void)
{
/*
 * Description:  Provide a main entry point for the Notes API to
 *               initialize inside of this shared library.  We need
 *               this entry point because we want to be able to lookup
 *               user information in the name and address book via the
 *               Notes SDK API.
 *
 * Input:  nothing
 * Output: nothing
 * Return: NOERROR
 */
    return TRUE;
}

/*---
*      filter initialization
*/
unsigned int FilterInit(FilterInitData* filterInitData)
{
/*
 * Description:  Filter initialization is performed when the filter
 *               shared library is dynamically loaded.
 *
 * Input:  filterInitData     dsapi specification controls the format
 *                            of data
 * Output: filterInitData     several fields are filled in
 *
 * Return: kFilterHandledEvent
 */

    PRINTLOG("\nFilterInitData() is getting called.\n");
    /*Required*/
    filterInitData->appFilterVersion = kInterfaceVersion;

    /* Modify the following code to set the flags you want */
    filterInitData->eventFlags = kFilterAuthenticate;

   /* Set a short description for your filter */
    strcpy(filterInitData->filterDesc,
           "Operating System Authentication Filter");

        /* insert any global initialization code here...       */

    /* Output sent to stdout and stderr is displayed on the
     * server console, but is not written to the server log file.
     */
    PRINTLOG("\nDSAPI Authentication filter initialized\n");
    return kFilterHandledEvent;
}

/*---
*      filter termination
*/
unsigned int TerminateFilter(unsigned int reserved)
{
/*
 * Description:  Filter termination is performed when the filter
 *               shared library is unloaded.
 *
 * Input:  reserved     currently unused (dsapi spec controls the
 *                      format of data)
 * Output: none
 *
 * Return: kFilterHandledEvent
 */

    /* insert any global cleanup code here... */

    return kFilterHandledEvent;
}


/*---
*      filter notification handling
*/
unsigned int HttpFilterProc(FilterContext* context,
                            unsigned int eventType, void* eventData)
{
/*
 * Description:  This routine is called for all dsapi filter events.
 *
 * Input:  reserved     currently unused (dsapi spec controls the
 *                      format of data)
 * Output: none
 *
 * Return: kFilterNotHandled for all events that we don't customize,
 *         otherwise allow our filter routine to provide a return
 *         value.
 */

   /* Include only those events we want to handle */
    switch (eventType) {
        case kFilterAuthenticate:
            return Authenticate(context, (FilterAuthenticate *)eventData);
        default:
            break;
    }

    return kFilterNotHandled;
}


/*---
*      handle user authentication
*/
unsigned int Authenticate(FilterContext* context,
                          FilterAuthenticate* authData)
{
/*
 * Description:  This routine is called on a dsapi kFilterAuthUser
 *               event.
 *
 * Input:  context     dsapi specification controls the format of data
 *
 *         authData    password field contains the password to use for
 *                     authentication
 *                     userName field contains the name
 *                     to authenticate
 *                     foundInCache field is TRUE if
 *                     user has been found in the cache and can be
 *                     authenticated on that basis.
 *
 * Output: authData    authName field is filled with user's
 *                     distinguished name
 *                     authType filed is filled
 *                     with kNotAuthentic if we can't authenticate the
 *                     user kAuthenticBasic if we can
 *                     authenticate the user.
 *
 * Return: kFilterNotHandled if we do not understand the input data,
 *                     or if the user has been found in the cache, or
 *                     if we find that the user to be authenticated is
 *                     not known to OS.
 *         kFilterHandledEvent if the user is known to OS.
 */

   char string1[] = "\n user is found in the cache \n";

   /* If the user is found in the cache, then we don't need to do
    * anything further.
    */

   if (!authData || authData->foundInCache) {
        AddInLogMessageText (string1, NOERROR);
        return kFilterNotHandled;
   }

   /* Attempt to verify the user's password.
    */
   if (authData->userName && authData->password) {
        char *fullName = NULL;
        int fullNameLen = 0;
        char *shortName = NULL;
        int shortNameLen = 0;
        char *user = NULL;
        char *domain = NULL;

#if defined SOLARIS || AIX || LINUX
        user=(char*)authData->userName;
#else
        separateUsernameAndDomainname(authData->userName,USER_DOMAIN_SEPARATOR,&user,&domain);
#endif

        /* Lookup the user in the Name and Address book.  Get
         * the user's short name (which we expect is the OS
         * user name), and get the user's fullname (which we
         * expect will be in the format to pass back to
         * dsapi).
         */
        if (NOERROR == getUserNames (context,
                                     user,
                                     &fullName,
                                     &fullNameLen,
                                     &shortName,
                                     &shortNameLen) )
        {

         /* Authenticate the username/pswd with OS */

            if (NOERROR != doAuthenticate(shortName, domain,
                                           (char *)authData->password))
            {
                return kFilterNotHandled;
            }
            else
            {
                /* Copy the canonical name for this user that
                 * dsapi requires.  */
                strncpy ((char *)authData->authName, fullName,
                         authData->authNameSize);
                authData->authType = kAuthenticBasic;
                authData->foundInCache = TRUE;
            }
            return kFilterHandledEvent;
        }
    }
    return kFilterNotHandled;
}


int getUserNames (FilterContext* context,
                  char *userName,
                  char **pUserFullName,
                  int  *pUserFullNameLen,
                  char **pUserShortName,
                  int  *pUserShortNameLen) {
/*
 * Description:  Lookup the user and return the user's full name and
 *               short name.
 *
 * Input:  context            context we'll use for allocating memory
 *         userName           the name of the user to lookup
 * Output: pUserFullName      location of the user's full name
 *         pUserFullNameLen   location to store the length of fullname
 *         pUserShortName     location of the user's shortname
 *         pUserShortNameLen  location to store the length of
 *                            shortname
 *
 * Return: -1 on error, 0 on success
 */
    STATUS   error = NOERROR;
    DHANDLE    hLookup = NULLHANDLE;
    DWORD   Matches = 0;
    char   *pLookup;
    char   *pName = NULL;
    char   *pMatch = NULL;
    int     rc = -1;

    char    string2[] = "full name=%s,length=%d\n";
    char    string3[] = "short name=%s,length=%d\n";

    /* Initialize output */
    *pUserFullName = NULL;
    *pUserFullNameLen = 0;
    *pUserShortName = NULL;
    *pUserShortNameLen = 0;

    /* do the name lookup
    */
    error = NAMELookup2(NULL, /* NULL means look locally */
                           0,   /* flags */
                           1,   /* number of namespaces */
                           "$Users", /* namespace list */
                           1,   /* number of names to lookup */
                           userName, /* list of names to lookup */
                           2, /* number of items to return */
                           "FullName\0ShortName", /* list of items to
                                                  * return */
                           &hLookup); /* place to receive handle of
                                    * return buffer */

    if (error || (NULLHANDLE == hLookup))
        goto NoUnlockExit;

    pLookup = (char *) OSLockObject(hLookup);

    /*   Get a pointer to our entry.
    */
    pName = (char *)NAMELocateNextName2(pLookup, /* name lookup
                                                 * buffer */
                                        NULL, /* start at beginning of
                                               * lookup buffer */
                                        &Matches); /* Receives number
                                               * of times we
                                               * found the entry
                                               * (should be 1) */

   /* If we didn't find the entry, then quit */
    if ((pName == NULL) || (Matches <= 0)) {
        goto Exit;
    }

    pMatch = (char *)NAMELocateNextMatch2(pLookup,  /* name lookup
                                                     * buffer */
                                          pName, /* entry that we found */
                                          NULL); /* no previous match */
    if (NULL == pMatch) {
        goto Exit;
    }

   /* Get the full name from the info we got back */

    if ( getLookupInfo (context,
                        pMatch,
                        0,
                        pUserFullName,
                        pUserFullNameLen) )
        goto Exit;

    AddInLogMessageText (string2, 0,*pUserFullName,*pUserFullNameLen);

   /* Get the short name from the info we got back */
    if ( getLookupInfo (context,
                        pMatch,
                        1,
                        pUserShortName,
                        pUserShortNameLen) )
        goto Exit;
    else
        rc = 0;

    AddInLogMessageText (string3, 0,*pUserShortName,*pUserShortNameLen);
Exit:
    if ( pLookup && hLookup )
        OSUnlock(hLookup);
NoUnlockExit:
    if (NULLHANDLE != hLookup)
        OSMemFree(hLookup);
    return rc;
}


int getLookupInfo (FilterContext* context,
                   char *pMatch,
                   unsigned short itemNumber,
                   char **pInfo,
                   int  *pInfoLen) {
/*
 * Description:  Get the info from the lookup buffer
 *
 * Input:  context            context we'll use for allocating memory
 *         pMatch             the name of the lookup buffer
 *         itemNumber         where the info is stored in the lookup
 *                            buffer
 * Output: pInfo              location of the info buffer
 *         pInfoLen           location to store the info length
 *
 * Return: -1 on error, 0 on success
 */

    unsigned int reserved = 0;
    unsigned int errID;
    char     *ValuePtr = NULL;
    WORD     ValueLength, DataType;
    STATUS   error;
    void     *newSpace = NULL;

    /* Initialize output */
    *pInfo = NULL;
    *pInfoLen = 0;

    /* Check the type and length of the info */

    ValuePtr = (char *)NAMELocateItem2(pMatch,
                                       itemNumber,
                                       &DataType,
                                       &ValueLength);

    if (NULL == ValuePtr || ValueLength == 0) {

        return -1;
    }
    ValueLength -= sizeof(WORD);

    /* check the value DataType */
    switch (DataType) {
        case TYPE_TEXT_LIST:
            break;

        case TYPE_TEXT:
            break;

        default:
            return -1;
    }

   /* Allocate space for the info.  This memory will be freed
    * automatically when the thread terminates.
    */

    newSpace = (context->AllocMem)(context, ValueLength+1,
                                   reserved, &errID);
    *pInfo = (char *) newSpace;
    if (NULL == *pInfo) {
        PRINTLOG ("Out of memory\n");
        return -1;
    }

   /* Get the info */
   error = NAMEGetTextItem2(pMatch, /* match that we found */
                            itemNumber, /* item # in order of item
                                             * on lookup */
                            0,      /* Member # of item in text
                                         * lists */
                            *pInfo, /* buffer to copy result
                                         * into */
                            MAX_BUF_LEN);   /* Length of buffer */
    if (!error) {
        *pInfoLen = strlen(*pInfo)+1;
        return 0;
    }

    return -1;
}


int doAuthenticate(char *userName, char *domain, char *password) {
/*
 * Description:  See if the user is known to the operating system, and if
 *               so, check that the password can be verified.
 *
 * Input:  userName           user name
 *         domain             domain name (NULL for UNIX)
 *         password           password
 *
 * Return: -1 on error, 0 on success
 */

    char   string4[] = "\nERROR: User must be specified\n";


    if (!userName) {

        AddInLogMessageText (string4, NOERROR);
        return -1;
    }

#if defined SOLARIS || AIX || LINUX
    PRINTLOG("\nin doAuthenticate()\n");
    return(unixAuthenticate(userName, password));
#else
    if (!domain) {
        AddInLogMessageText ("\nERROR: Domain must be specified. Use username@domainname format\n", NOERROR);
        return -1;
    }
    return(winAuthenticate(userName, domain, password));
#endif


}

#ifdef __cplusplus
}
#endif
/* ----- end of secdom.c */
