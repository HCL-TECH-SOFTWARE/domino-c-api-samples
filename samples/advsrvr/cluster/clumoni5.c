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

    PROGRAM:    CLUMON.EXE
    
    FILE:       CLUMONCM.C

    PURPOSE:    Advanced Server Cluster C API Sample program that illustrates 
                a "Cluster Monitor" application for managing Cluster server 
                information and their databases.  This program is designed 
                to run from the OS console (ie. UNIX shell).
                        
************************************************************************/
#ifdef OS400
#pragma convert(850)
#endif

/* UNIX */
#if defined(SUN)
#include <termios.h>
#else
#ifndef OS400
#include <termio.h>
#endif
#endif

/* Standard Library Include Files */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* HCL C API for Notes/Domino INCLUDE FILES */

#include <global.h>
#include <nsfdb.h>
#include <ns.h>
#include <osmem.h>
#include <osmisc.h>
#include <osfile.h>
#include <dname.h>
#include <textlist.h>
#include <srverr.h>


/* LOCAL INCLUDE FILES */
#include "clumon.h"
#include "clfunc.h"

/* Console "menus" */
void MainPrompt(void);
void InfoPrompt(void);
void AdmPrompt(void);

/* Console "dialogs" */
void GetClusterInfo(void);
void GetDBOptions(void);
void GetServerRestrict(void);
void GetThreshold(void);
void ManageDBOptions(void);
void SetServerRestrict(void);
void SetThreshold(void);
void CreateCopyDB(void);

/* Service routines */
void dspClusterInfo(char *, DWORD);
void dspDBOptions(char *, char *);
void updateDBOptions(char *, char *, int);
void dspServerRestrict(char *);
void setServerRestrict(char *, int);
void shutdownServer(char *);
void dspThresholdInfo(char *);
void setThresholdInfo(char *, int);
void createcopyDB(char *, char *, char *, char *, BOOL);

/* static values */
static HANDLE    hModule;                     /* module handle */
static char  szDBServerNameString[LINEOTEXT]; /* String to hold Name of Server */
static char  szDBNameString[LINEOTEXT];       /* String to hold Name of Database */

//struct termios cfg;

/*******************************
 * Main API Program Entry Point
 *******************************/

int main(int argc, char *argv[])
{
    STATUS  error;

    //ioctl(0, TCGETS, &cfg);

    if (error = NotesInitExtended (argc, argv))
    {
        printf("\n Unable to initialize Notes.\n");
        return (1);
    }

/* 
 * Print program intro, call main user menu interface,
 * and return when the user has decided to exit 
 */
    printf("\n%s\n\n", CLUMON_TITLE);
    MainPrompt();
    printf("Program Exit\n\n");

    NotesTerm();
    return (0); 

}	

/************************************************************************

    FUNCTION: MainPrompt()

    PURPOSE:  Main menu prompt.

***********************************************************************/

void MainPrompt(void)
{
    int iSelection=0;
    
    /* clear server and database name context strings */
    szDBServerNameString[0] = szDBNameString[0] = '\0';

    /* loop until user wishes to exit */ 
    while (iSelection != 3)
    { 
        printf("Main Menu\n");
        printf("1. Cluster Information\n");
        printf("2. Cluster Administration\n");
        printf("3. Exit\n\n");

        fflush(stdin);
        //ioctl(0, TCSETS, &cfg);
        printf("Enter a selection -> ");
#ifdef OS400
        iSelection = getcharI5(); 
#else
        iSelection = getchar();  /* get selection from user and */
#endif
        iSelection -= 0x30;      /* convert to integer value */
 
        switch (iSelection)
        {
            case 1: InfoPrompt(); break;
            case 2: AdmPrompt(); break;
            case 3: printf("\n"); break;
            
            default: printf("Invalid selection.  Try again.\n"); 
                break;
        }
    }                
}        

/************************************************************************

    FUNCTION: InfoPrompt()

    PURPOSE:  Cluster Information menu prompt.

***********************************************************************/

void InfoPrompt(void)
{
    int iSelection=0;


    /* loop until user wishes to return to Main Menu */ 
    while (iSelection != 5)
    { 
        printf("\nCluster Information Menu\n"); 
        printf("1. View Server Cluster\n");
        printf("2. View Database Mark Options\n");
        printf("3. View Server Restriction Setting\n");
        printf("4. View Server Availability Threshold\n");
        printf("5. Return to Main Menu\n\n");

        fflush(stdin);
        //ioctl(0, TCSETS, &cfg);
        printf("Enter a selection -> ");
#ifdef OS400
        iSelection = getcharI5(); 
#else
        iSelection = getchar();  /* get selection from user and */
#endif
        iSelection -= 0x30;      /* convert to integer value */

        switch (iSelection)
        {
            case 1: GetClusterInfo(); break;
            case 2: GetDBOptions(); break;
            case 3: GetServerRestrict(); break;
            case 4: GetThreshold(); break;
            case 5: printf("\n"); break;

            default: printf("Invalid selection.  Try again.\n"); 
                     break;
            
        }
    }                
}        

/************************************************************************

    FUNCTION: AdmPrompt()

    PURPOSE:  Cluster Administration menu prompt.

***********************************************************************/

void AdmPrompt(void)
{
    int iSelection=0;

    /* loop until user wishes to return to Main Menu */ 
    while (iSelection != 5)
    { 
        printf("\nCluster Administration Menu\n"); 
        printf("1. Modify Database Mark Options\n");
        printf("2. Restrict Server\n");
        printf("3. Modify Server Availability\n");
        printf("4. Database Replicate/Copy\n");
        printf("5. Return to Main Menu\n\n");

        fflush(stdin);
        //ioctl(0, TCSETS, &cfg);
        printf("Enter a selection -> ");
#ifdef OS400
        iSelection = getcharI5(); 
#else
        iSelection = getchar();  /* get selection from user and */
#endif
        iSelection -= 0x30;      /* convert to integer value */
        
        switch (iSelection)
        {
            case 1: ManageDBOptions(); break;
            case 2: SetServerRestrict(); break;
            case 3: SetThreshold(); break;
            case 4: CreateCopyDB(); break;
            case 5: printf("\n"); break;
            default: printf("Invalid selection.  Try again.\n"); 
                     break;
        }
    }                
}        

/************************************************************************

    FUNCTION: GetClusterInfo()

    PURPOSE:  User interface for displaying server cluster information

***********************************************************************/

void GetClusterInfo (void)
{
    STATUS  nError = NOERROR;

    /* prompt for target server */
    printf("Cluster Information\n");
    fflush(stdin);
    //ioctl(0, TCSETS, &cfg);
    printf("Enter the Server/Org name (or RETURN to exit)-> ");
    (void) gets(szDBServerNameString);

    if (!strcmp(szDBServerNameString, ""))
    {
        printf("\n"); 
        return;
    }
    else
        /* display cluster information */
        dspClusterInfo(szDBServerNameString, NPN_CLU_SHOW_ALL);

    fflush(stdin);
    //ioctl(0, TCSETS, &cfg);
    printf("\nEnter 'c' to continue ");
#ifdef OS400
    while (getcharI5() != 'c');
#else
    while (getchar() != 'c');
#endif

    return;
}        

/************************************************************************

    FUNCTION: GetDBOptions()

    PURPOSE:  User interface for displaying clustered database mark options

***********************************************************************/

void GetDBOptions (void)
{
    STATUS  nError = NOERROR;

    /* prompt for target server */
    printf("DB Mark Options Information\n");
    fflush(stdin);
    //ioctl(0, TCSETS, &cfg);
    printf("Enter the Server/Org name (or RETURN to cancel)-> ");
    (void) gets(szDBServerNameString);
    if (!strcmp(szDBServerNameString, ""))
    {
        printf("\n"); 
        return;
    }

    /* prompt for target database */
    fflush(stdin);
    //ioctl(0, TCSETS, &cfg);
    printf("Enter the Database name (or RETURN to cancel)-> ");
    (void) gets(szDBNameString);
    if (!strcmp(szDBNameString, ""))
    {
        printf("\n"); 
        return;
    }
    else
        /* display database mark options */
        dspDBOptions (szDBServerNameString, szDBNameString);

    fflush(stdin);
    //ioctl(0, TCSETS, &cfg);
    printf("\nEnter 'c' to continue ");
#ifdef OS400
    while (getcharI5() != 'c');
#else
    while (getchar() != 'c');
#endif

    return;
}

/************************************************************************

    FUNCTION: GetServerRestrict()

    PURPOSE:  User interface for displaying server restriction setting

***********************************************************************/

void GetServerRestrict (void)
{
    STATUS  nError = NOERROR;

    /* prompt for target server */
    printf("Server Restriction Information\n");
    fflush(stdin);
    //ioctl(0, TCSETS, &cfg);
    printf("Enter the Server/Org name (or RETURN to cancel)-> ");
    (void) gets(szDBServerNameString);
    if (!strcmp(szDBServerNameString, ""))
    {
        printf("\n"); 
        return;
    }

    /* display server restriction setting */
    dspServerRestrict (szDBServerNameString);

    fflush(stdin);
    //ioctl(0, TCSETS, &cfg);
    printf("\nEnter 'c' to continue ");
#ifdef OS400
    while (getcharI5() != 'c');
#else
    while (getchar() != 'c');
#endif

    return;
}

/************************************************************************

    FUNCTION: GetThreshold()

    PURPOSE:  User interface for displaying server availability information

***********************************************************************/

void GetThreshold (void)
{
    STATUS  nError = NOERROR;

    /* prompt for target server */
    printf("Server Availability Information\n");
    fflush(stdin);
    //ioctl(0, TCSETS, &cfg);
    printf("Enter the Server/Org name (or RETURN to cancel)-> ");
    (void) gets(szDBServerNameString);
    if (!strcmp(szDBServerNameString, ""))
    {
        printf("\n"); 
        return;
    }

    /* display availability and threshold */
    dspThresholdInfo (szDBServerNameString);

    fflush(stdin);
    //ioctl(0, TCSETS, &cfg);
    printf("\nEnter 'c' to continue ");
#ifdef OS400
    while (getcharI5() != 'c');
#else
    while (getchar() != 'c');
#endif

    return;
}
    
/************************************************************************

    FUNCTION: ManageDBOptions()

    PURPOSE:  User interface for modifying database mark option

***********************************************************************/

void ManageDBOptions (void)
{
    char    szMark[LINEOTEXT];
    int     iMark;
    STATUS  nError = NOERROR;

    /* prompt for target server */
    printf("DB Mark Options Management\n");
    fflush(stdin);
    //ioctl(0, TCSETS, &cfg);
    printf("Enter the Server/Org name (or RETURN to cancel)-> ");
    (void) gets(szDBServerNameString);
    if (!strcmp(szDBServerNameString, ""))
    {
        printf("\n"); 
        return;
    }

    /* prompt for target database */
    fflush(stdin);
    //ioctl(0, TCSETS, &cfg);
    printf("Enter the Database name (or RETURN to cancel)-> ");
    (void) gets(szDBNameString);
    if (!strcmp(szDBNameString, ""))
    {
        printf("\n"); 
        return;
    }

    /* and desired mark option */
    fflush(stdin);
    //ioctl(0, TCSETS, &cfg);
    printf("Enter New Database Mark Option, where\n");
    printf("\t0 = Mark In Service\n");
    printf("\t1 = Mark Out Of Service\n");
    printf("\t2 = Mark For Deletion\n");
    printf("(or RETURN to cancel)-> ");
    (void) gets(szMark);
    if (!strcmp(szMark, ""))
    {
        printf("\n"); 
        return;
    }                                                     

    iMark = atoi(szMark);
    if ((iMark<0)||(iMark>2))
    {
        printf("Invalid selection.  Cancelling operation.\n"); 
        return;
    }                                                     
    else
        /* modify database mark options */
        updateDBOptions (szDBServerNameString, szDBNameString, iMark);

    fflush(stdin);
    //ioctl(0, TCSETS, &cfg);
    printf("\nEnter 'c' to continue ");
#ifdef OS400
    while (getcharI5() != 'c');
#else
    while (getchar() != 'c');
#endif

    return;
}

/************************************************************************

    FUNCTION: SetServerRestrict()

    PURPOSE:  User interface for modifying server restrinction setting

***********************************************************************/

void SetServerRestrict (void)
{
    STATUS  nError = NOERROR;
    char    szRestrict[LINEOTEXT];
    int     iRestrict;

    /* prompt for target server */
    printf("Server Restriction Management\n");
    fflush(stdin);
    //ioctl(0, TCSETS, &cfg);
    printf("Enter the Server/Org name (or RETURN to cancel)-> ");
    (void) gets(szDBServerNameString);
    if (!strcmp(szDBServerNameString, ""))
    {
        printf("\n"); 
        return;
    }
    
    /* and desired restriction */
    fflush(stdin);
    //ioctl(0, TCSETS, &cfg);
    printf("Enter Server Restriction Action, where\n");
    printf("\t0 = Unrestrict Server\n");
    printf("\t1 = Restrict Server\n");
    printf("\t2 = Shutdown Server\n");
    printf("(or RETURN to cancel)-> ");
    (void) gets(szRestrict);
    if (!strcmp(szRestrict, ""))
    {
        printf("\n"); 
        return;
    }                                                     

    iRestrict = atoi(szRestrict);
    switch (iRestrict) {
        /* modify server restriction setting */
        case 0:
        case 1: setServerRestrict (szDBServerNameString, iRestrict);
            break;

        /* shutdown server */
        case 2: shutdownServer (szDBServerNameString);
            break;

        default: printf("Invalid selection.  Cancelling operation.\n"); 
            return;
    }

    fflush(stdin);
    //ioctl(0, TCSETS, &cfg);
    printf("\nEnter 'c' to continue ");
#ifdef OS400
    while (getcharI5() != 'c');
#else
    while (getchar() != 'c');
#endif

    return;
}

/************************************************************************

    FUNCTION: SetThreshold()

    PURPOSE:  User interface for modifying server availability threshold

***********************************************************************/

void SetThreshold (void)
{
    STATUS  nError = NOERROR;
    char    szThreshold[LINEOTEXT];
    int     iThreshold;

    /* prompt for target server */
    printf("Availability Threshold Management\n");
    fflush(stdin);
    //ioctl(0, TCSETS, &cfg);
    printf("Enter the Server/Org name (or RETURN to cancel)-> ");
    (void) gets(szDBServerNameString);
    if (!strcmp(szDBServerNameString, ""))
    {
        printf("\n"); 
        return;
    }
    
    /* and desired threshold */
    fflush(stdin);
    //ioctl(0, TCSETS, &cfg);
    printf("Enter New Server Availability Threshold (or RETURN to cancel)-> ");
    (void) gets(szThreshold);
    if (!strcmp(szThreshold, ""))
    {
        printf("\n"); 
        return;
    }                                                     

    /* modify the availability threshold */
    iThreshold = atoi(szThreshold);
    setThresholdInfo (szDBServerNameString, iThreshold);

    fflush(stdin);
    //ioctl(0, TCSETS, &cfg);
    printf("\nEnter 'c' to continue ");
#ifdef OS400
    while (getcharI5() != 'c');
#else
    while (getchar() != 'c');
#endif

    return;
}

/************************************************************************

    FUNCTION: CreateCopyDB()

    PURPOSE:  User interface for creating new or replica databases

***********************************************************************/

void CreateCopyDB (void)
{
    STATUS  nError = NOERROR;
    char    szDBCopyServer[LINEOTEXT];  /* Copy database server name */
    char    szDBCopyFile[LINEOTEXT];    /* Copy database file name */
    BOOL    bReplFlag;                  /* TRUE for repl copy, FALSE for new */
    char    szReplNew[LINEOTEXT];       /* Replicate or New Copy input */

    /* prompt for source server */
    printf("DB Copy Management\n");
    fflush(stdin);
    //ioctl(0, TCSETS, &cfg);
    printf("Enter the Source Server/Org name (or RETURN to cancel)-> ");
    (void) gets(szDBServerNameString);
    if (!strcmp(szDBServerNameString, ""))
    {
        printf("\n"); 
        return;
    }

    /* prompt for source database */
    fflush(stdin);
    //ioctl(0, TCSETS, &cfg);
    printf("Enter the Source Database name (or RETURN to cancel)-> ");
    (void) gets(szDBNameString);
    if (!strcmp(szDBNameString, ""))
    {
        printf("\n"); 
        return;
    }

    /* prompt for target server */
    fflush(stdin);
    //ioctl(0, TCSETS, &cfg);
    printf("Enter the Destination Server/Org name (or RETURN to cancel)-> ");
    (void) gets(szDBCopyServer);
    if (!strcmp(szDBCopyServer, ""))
    {
        printf("\n"); 
        return;
    }

    /* prompt for target database copy */
    fflush(stdin);
    //ioctl(0, TCSETS, &cfg);
    printf("Enter the Destination Database name (or RETURN to cancel)-> ");
    (void) gets(szDBCopyFile);
    if (!strcmp(szDBCopyFile, ""))
    {
        printf("\n"); 
        return;
    }

    /* and specify replica or new copy */
    fflush(stdin);
    //ioctl(0, TCSETS, &cfg);
    printf("Specify Replica or New Copy (R/N) (or RETURN to cancel) -> ");
    (void) gets(szReplNew);
    if (!strcmp(szReplNew, ""))
    {
        printf("\n"); 
        return;
    }                                                     
    
    if (!strcmp(szReplNew, "R"))
        bReplFlag = TRUE;
    else if (!strcmp(szReplNew, "N"))
        bReplFlag = FALSE;
    else
    {
        printf("Invalid selection.  Cancelling operation.\n"); 
        return;
    }

    /* copy database */
    createcopyDB (szDBServerNameString, szDBNameString,
                  szDBCopyServer, szDBCopyFile, bReplFlag);

    fflush(stdin);
    //ioctl(0, TCSETS, &cfg);
    printf("\nEnter 'c' to continue ");
#ifdef OS400
    while (getcharI5() != 'c');
#else
    while (getchar() != 'c');
#endif

    return;
}

/************************************************************************
 
    FUNCTION: dspClusterInfo(char *, DWORD)
  
    PURPOSE:  Retrieves and displays the Advanced Server Cluster
              information for the specified server. The GetServerCluster, 
              GetServerLoad, and GetServerClusterMates routines (clfunc.c) 
              are called to perform the relevant HCL C API for Domino and 
              Notes functions.

    COMMENTS:

          Although designed to retrieve each piece separately,
          this routine currently is called by the GetClusterInfo() 
          function passing a NPN_CLU_SHOW_ALL flag.  This enables 
          displaying all the three server cluster information
          elements. 

          Any errors are processed and displayed to the user as 
          appropriate.
            
 ************************************************************************/

void dspClusterInfo(char *szServerName, DWORD dwClusterFlags)
{
    STATUS nError;
    char szErrorString[LINEOTEXT];      /* Error Message String */
    char szClusterName[MAXUSERNAME];    /* Name of Cluster */
    char szClusterInfo[LINEOTEXT];      /* Returned Cluster Info */

    DWORD dwLoadIndex = 0;
    HANDLE hClusterList = NULLHANDLE;	 /* list of clustermates */
    void FAR *lpClusterList = NULL;		 /* locked-down cluster list pointer */
      
    WORD wNumListEntries = 0;
    WORD wBufferLen = 0;
    char *pBuffer;
    int i;

    printf("\nCluster Information For %s\n", szServerName);

    /* If the Show_Server's_Cluster menu item was selected */
    if (dwClusterFlags & NPN_CLU_SHOW_CLUNAME )
    {
        nError = GetServerCluster(szServerName, szClusterName );
       
        /* return if error */
        if (nError != NOERROR)
            goto Cleanup;
        else
        {
            /* Print out cluster name */
            printf("\nCluster Name: %s", szClusterName);
        }
    }

    /* If the Show_Server's_Load menu item was selected */
    if ( dwClusterFlags & NPN_CLU_SHOW_LOAD )
    {
        nError = GetServerLoad(szServerName, &dwLoadIndex);
      
        if (nError)
            goto Cleanup;

        else
        /* Print out Availability index */
         printf("\nAvailability: %ld\n", dwLoadIndex);
    }
    
    /* If the Show_Server's_ClusterMates menu item was selected, get the
    * Cluster mates.  To ensure the latest information, the Cluster mate
    * list will be looked up via server NameLookup using by specifiying
    * the CLUSTER_LOOKUP_NOCACHE flag.
    */
    if ( dwClusterFlags & NPN_CLU_SHOW_CLUMATES )
    {

        /* Call the routine to get the cluster list of the specified server.  
         * If hClusterList != NULLHANDLE. then it must be freed in this 
         * block of code.
         */
        nError = GetServerClusterMates(szServerName, 
                                       (DWORD)CLUSTER_LOOKUP_NOCACHE,
                                       &hClusterList);
    
        /* If the server is in a restricted or unavailable state then we can
        * still continue because it will still return the cluster info to us.
        * If it's another error then quit but make sure that the list is freed
        */
        if (nError)
        {
            if ( !(( ERR(nError) == ERR_SERVER_UNAVAILABLE) || 
                ( ERR(nError) == ERR_SERVER_RESTRICTED)) )
                goto Cleanup;
        }
        nError = NOERROR;
        lpClusterList = OSLock( void, hClusterList);
        wNumListEntries = ListGetNumEntries( lpClusterList, FALSE);
    
        /* Display the member count */
        printf("Cluster Mates: %d\n", wNumListEntries);

        /* Get the members from the list */
        for (i = 0; i < (int) wNumListEntries; i++)
        {
            nError = ListGetText( lpClusterList, FALSE, (WORD) i, 
                                  FAR &pBuffer, &wBufferLen );
            if (!nError)
            {
                /* Update the Cluster Mates dialog list box */
                memcpy(szClusterInfo, pBuffer, wBufferLen);
                szClusterInfo[wBufferLen] = '\0';
                printf("\t%s\n", szClusterInfo);
            }
            else
            {
                OSUnlock( hClusterList);
                goto Cleanup;
            }
        } /* end of FOR */
        
        /* No longer need this memory - Freeing done in "Cleanup:" below */
        OSUnlock( hClusterList);
    }  

Cleanup:
   if (hClusterList != NULLHANDLE)        
       OSMemFree( hClusterList);

   /* Result Processing */
   switch (nError)
   {
        case NPNERR_NOT_CLUSTER_MEMBER:
           sprintf (szErrorString, 
                    "Server %s is not a member of any cluster!\n",
                    szDBServerNameString);
           printf("\n%s", szErrorString);
           break;
   
        case NOERROR:
           break;
   
        default:  /* HCL C API for Notes/Domino Error */
           OSLoadString(hModule, ERR(nError),
                        szErrorString, LINEOTEXT-1);
           printf("\n%s", szErrorString);
           break;
   }                 
   return;
}   

/************************************************************************

    FUNCTION: dspDBOptions(char *, char *)

    PURPOSE:  Retrieves and displays the Database Mark Options for the
              specified server and database names.  The
              GetDBMarks routine (clfunc.c) is called to perform the
              relevant HCL C API for Notes/Domino functions.
              
    COMMENTS:

          This routine passes the input Server and Database names to the
          GetDBMarks routine which opens the database and returns its Option
          flag mask. If successful, the mark option is displayed.

          Note that clusetered server failover is supported by the called
          GetDBMarks routine.  If failover occurs, a message is displayed to
          inform the user.

          Any errors are processed and displayed to the user as appropriate.

***********************************************************************/

void dspDBOptions (char *szServerName, char *szDBName )
{
    DWORD   dwOptionMask;
    STATUS  nError;
    char    szErrorString[LINEOTEXT];   /* Error Message String */
    BOOL    bFailover;                  /* Failover flag */

    printf("\nDatabase Mark Options For %s!!%s\n", szServerName, szDBName);
    
    /* Call routine to get the Mark options */
    nError = GetDBMarks (szServerName, szDBName, &dwOptionMask, &bFailover);
    
    /* Return if error getting options */
    if (nError != NOERROR)
        goto Cleanup;

    /* Report clustered server failover and update server/DB name Edit boxes */
    if (bFailover)
    {
        sprintf (szErrorString, 
                 "Clustered Database Server Failover Occurred\nOpened DB: %s!!%s",
                 szServerName, szDBName);
        printf ("\n%s\n", szErrorString);
    }

    /* Report Mark Options */
    if (dwOptionMask & DBOPTION_OUT_OF_SERVICE)        
        printf("\tMarked Out Of Service\n");
    else
        printf("\tMarked In Service\n");
    
    if (dwOptionMask & DBOPTION_MARKED_FOR_DELETE)
        printf("\tMarked For Deletion\n");

Cleanup:
    /* Result Processing */
    if ( nError != NOERROR)
    {
        OSLoadString(hModule, ERR(nError),
                     szErrorString, LINEOTEXT-1);
        printf("%s\n", szErrorString);
    }
   return;
}

/************************************************************************

    FUNCTION: dspThresholdInfo(char *)

    PURPOSE:  Retrieves and displays the Server availability and threshold
              information for the specified server name.   The GetServerLoad
              and RemoteCommand routines (clfunc.c) are called to perform
              the relevant HCL C API for Notes/Domino functions.  
              RemoteCommand programmatically uses Domino and Notes remote 
              console commands to get the server availability threshold 
              information. The referenced commands are defined in clumon.h.
              
    COMMENTS:

          This routine passes the input Servername parameter to the 
          GetServerLoad to retrieve the current availability (load)
          index.  If successful, the value is displayed.

          Next, the input Servername parameter is passed with the proper 
          console command to the RemoteCommand routine, which retrieves the 
          current server availability threshold configuration. If successful,
          the returned status (console response) is interpreted and displayed.

          Any errors are processed and displayed to the user as appropriate.
            
***********************************************************************/

void dspThresholdInfo (char *szServerName)
{
    STATUS  nError;
    char    szErrorString[LINEOTEXT];           /* Error Message String */
    DWORD   dwAvailability;                     /* server availability value*/
    char    szInfo[LINEOTEXT];                  /* info string*/
    char    szServerResponse[LINEOTEXT];        /* response buffer */
    WORD    wResponseLen = LINEOTEXT;           /* and length      */
    int     i;

    printf("\nCurrent Availability And Threshold For %s\n", szServerName);

    /* First retrieve and display the server availability (if noerror) */
    nError = GetServerLoad (szServerName, &dwAvailability);
    if (nError != NOERROR)
        goto Cleanup;

    sprintf (szInfo, "%lu", dwAvailability);
    printf("Availability: %s\n", szInfo);

    /* Then call routine to get the current Threshold configuration */
    nError = RemoteCommand (szServerName, GET_THRESHOLD, 
                            szServerResponse, &wResponseLen);
    
    /* Return if error with remote command */
    if (nError != NOERROR)
        goto Cleanup;

    /* Parse out Threshold setting from response string */
    /* first strip out CR/LF termination of Notes.ini string */
    i = strlen(szServerResponse) - 2;
    szServerResponse[i] = '\0';

    /* if never configured, the threshold is zero */
    if (!strcmp(szServerResponse, CON_ERROR))
        sprintf(szInfo, "%d", 0); 
   
    /* else read in the value assigned to the config variable */
    else
    {
        i = strlen(THRESHOLD); 
        strcpy(szInfo, "");
        while (szServerResponse[i] != '\0')  
               sprintf(szInfo, "%s%c", szInfo, szServerResponse[i++]);
    }

    /* and display to the dialog box */
    printf("Availability Threshold: %s\n", szInfo);

Cleanup:
    /* Result Processing */
    if ( nError != NOERROR)
    {
        OSLoadString(hModule, ERR(nError),
                     szErrorString, LINEOTEXT-1);
        printf("%s\n", szErrorString);
    }
    return;
}

/************************************************************************

    FUNCTION: updateDBOptions(char *, char *, int)

    PURPOSE:  Updates the Database Options for the specified server, database
              and options. The SetDBMarks routine (clfunc.c) is called to
              peform the relevant HCL C API for Notes/Domino functions.

    COMMENTS:

          This routine passes the specified database option marks along with
          the input server and database names to the SetDBMarks routine to 
          mark the database as appropriate.

          If the user has specified to mark a database for delete, a YES-NO
          warning message is displayed.  A status message is displayed after
          the database has been marked.
          
          Any errors are processed and displayed to the user as appropriate.
            
***********************************************************************/

void updateDBOptions (char *szServerName, char *szDBName, int iMark )
{
    STATUS  nError;
    WORD    wMarkFlag = 0;
    char    szErrorString[LINEOTEXT];        /* Error Message String */
    char    cYesNo;
    
    printf("\nUpdating Database Mark Options For %s!!%s\n", szServerName, szDBName);

    /* Read desired Marks;  Error if both Mark In and Mark Out of Service */
    if ( iMark == 0 )
        wMarkFlag |= MARK_IN_SERVICE;

    if ( iMark == 1 )
        wMarkFlag |= MARK_OUT_SERVICE;

    if ( iMark == 2 )
    {    
        fflush(stdin);
        //ioctl(0, TCSETS, &cfg);
        sprintf (szErrorString, "%s\n%s",
         "Marking a database for deletion can not be undone.",
         "Do you wish to continue (Y/N)?->" );
        printf("%s", szErrorString);
#ifdef OS400
        cYesNo = getcharI5();
#else
        cYesNo = getchar();
#endif
        if ((cYesNo=='y')||(cYesNo=='Y'))
            wMarkFlag |= MARK_DELETE;
        else
        { 
            nError = NPNERR_CANCELLED;
            goto Cleanup;
        }
    }

    /* Call routine to set the database marks based on the mark flag */
    nError = SetDBMarks (szServerName, szDBName, wMarkFlag);

Cleanup:
    /* Result Processing */
    switch (nError)
    {
        case NPNERR_CANCELLED:
            printf("Database Mark Cancelled\n");
            break;

        case NOERROR:
            printf("Database Mark Successful\n");
            break;

        default:
            OSLoadString(hModule, ERR(nError),
                         szErrorString, LINEOTEXT-1);
            printf("%s\n", szErrorString);
            break;
    }
    return;
}

/************************************************************************

    FUNCTION: dspServerRestrict(char *)

    PURPOSE:  Retrieves and displays the Server Restriction state for the
              specified server.   The RemoteCommand routine (clfunc.c) is
              called to perform the relevant HCL C API for Domino and 
              Notes functions. Specifically, this routine programmatically 
              uses Domino and Notes remote console commands to retrieve 
              server restriction status. The referenced commands are 
              defined in clumon.h.
              
    COMMENTS:

          This routine passes the input Servername parameter along with 
          the proper console command to the RemoteCommand routine, which
          retrieves the current server restriction status. If successful,
          the returned status (console response) is interpreted and displayed.

          Any errors are processed and displayed to the user as appropriate.
            
***********************************************************************/

void dspServerRestrict (char *szServerName)
{
    STATUS  nError;
    char    szErrorString[LINEOTEXT];           /* Error Message String */
    char    szServerResponse[LINEOTEXT];        /* response buffer */
    WORD    wResponseLen = strlen(RESTRICTED);  /* and length      */

    printf("\nCurrent Restriction For %s\n", szServerName);

    /* Call routine to get the Mark options */
    nError = RemoteCommand (szServerName, GET_RESTRICT, 
                            szServerResponse, &wResponseLen);
    
    /* Return if error with remote command */
    if (nError != NOERROR)
        goto Cleanup;

    /* and check Server Restricted checkbox if SERVER_RESTRICT=1 
     * response is returned
     */
    if (!strcmp(szServerResponse, RESTRICTED))        
        printf("\tServer Restricted\n");
    else
        printf("\tServer Not Restricted\n");

Cleanup:
    /* Result Processing */
    if ( nError != NOERROR)
    {
        OSLoadString(hModule, ERR(nError),
                     szErrorString, LINEOTEXT-1);
        printf("%s\n", szErrorString);
    }

    return;
}

/************************************************************************

    FUNCTION: setServerRestrict(char *, int)

    PURPOSE:  Sets and displays the Server Restriction state for the
              specified server.  The RemoteCommand routine (clfunc.c) is 
              called to perform the relevant HCL C API for Domino and 
              Notes functions.  Specifically, this routine programmatically 
              uses Notes and Domino remote console commands to set/unset 
              the server restriction configuration. The referenced commands 
              are defined in clumon.h.

    COMMENTS:

          This routine builds the appropriate console command for the specified
          action, and passes the input Servername parameter amd console command 
          to the RemoteCommand routine, which sets the current server 
          restriction status as desired.  If successful, a message is 
          displayed.

          Any errors are processed and displayed to the user as appropriate.
            
***********************************************************************/

void setServerRestrict (char *szServerName, int iRestrict)
{
    STATUS  nError;
    char    szErrorString[LINEOTEXT];       /* Error Message String */
    char    szCommand[LINEOTEXT];           /* remote command */
    char    szServerResponse[LINEOTEXT];    /* response buffer */
    WORD    wResponseLen = 0;               /* and length      */
    BOOL    bRestrictFlag;                  /* TRUE=restrict, FALSE=unrestrict */

    printf("\nSetting Restriction For %s\n", szServerName);

    /* Determine whether server is being restricted or unrestricted, and
     * build up the appropriate console command 
     */
    if ( iRestrict == 0 )
    {
        sprintf (szCommand, "%s", RESTRICT_SERVER);
        bRestrictFlag = FALSE;
    }
    else
    {
        sprintf (szCommand, "%s%d", RESTRICT_SERVER, 1);
        bRestrictFlag = TRUE;
    }

    /* Call routine to set the Server restriction */
    nError = RemoteCommand (szServerName, szCommand, 
                            szServerResponse, &wResponseLen);

    /* Result Processing */
    if ( nError != NOERROR)
    {
        OSLoadString(hModule, ERR(nError),
                     szErrorString, LINEOTEXT-1);
        printf("%s\n", szErrorString);
    }
    else
    {
        if ( bRestrictFlag )
            printf("Server Restricted\n");
        else
            printf("Server Unrestricted\n");
    }
    return;
}


/************************************************************************

    FUNCTION: shutdownServer(char *)

    PURPOSE:  Shuts down (quits) the server specified. The RemoteCommand 
              routine (clfunc.c) is called to perform the relevant Lotus 
              C API for Domino and Notes functions.  Specifically, this 
              routine programmatically uses Domino and Notes remote 
              console commands to shutdown the server. The referenced 
              commands are defined in clumon.h.
              
    COMMENTS:

          This routine first displays a YES-NO message to the user,
          warning them and asking them if they wish to continue.  If yes,
          the the input Servername parameter along with the proper console
          command to the RemoteCommand routine, which shuts down the server. 
          
          If successful, a message is displayed.   Note, that since the
          server is being shutdown, "error" messages may be displayed
          by the remote console command (e.g. Server unavailable).  These
          messages can be expected, but are not interpreted by this routine.
                 
          Any errors are processed and displayed to the user as appropriate.
            
***********************************************************************/

void shutdownServer (char *szServerName)
{
    STATUS  nError;
    char    szErrorString[LINEOTEXT];
    char    szServerResponse[LINEOTEXT];    /* response buffer */
    WORD    wResponseLen = 0;               /* and length      */
    char    cYesNo;

    printf("\nShuttting Down Server %s\n", szServerName);

    /* First warn the user */
    fflush(stdin);
    //ioctl(0, TCSETS, &cfg);
    sprintf (szErrorString, "%s\n%s", 
        "A server can not be restarted from a remote console.",
        "Do you wish to continue with the shutdown (Y/N)?-> " );
    printf("%s", szErrorString);

#ifdef OS400
        cYesNo = getcharI5();
#else
        cYesNo = getchar();
#endif
    if ((cYesNo=='y')||(cYesNo=='Y'))

    /* Call routine to shutdown the server */
        nError = RemoteCommand (szServerName, SHUTDOWN_SERVER, 
                                szServerResponse, &wResponseLen);
    else
        nError = NPNERR_CANCELLED;

    /* Result Processing */
    switch ( nError )
    {
        case NPNERR_CANCELLED:
            printf("Server Shutdown Cancelled\n");
            break;

        case NOERROR:
            printf("Server Shutdown Successful\n");
            break;

        default:
            OSLoadString(hModule, ERR(nError),
                         szErrorString, LINEOTEXT-1);
            printf("%s\n", szErrorString);
            break;
    }
    return;
}

/************************************************************************

    FUNCTION: setThresholdInfo(char *, int)

    PURPOSE:  Sets and displays the Server availability and threshold
              information for the specified server.  The RemoteCommand 
              routine (clfunc.c) is called to perform the relevant Lotus 
              C API for Domino and Notes functions.  Specifically, this 
              routine programmatically uses Domino and Notes remote 
              console commands to set the server availability threshold 
              configuration. The referenced commands are defined in clumon.h.

    COMMENTS:

          This routine builds the appropriate console command for the specified
          threshold value, and passes the input Servername parameter amd console
          command to the RemoteCommand routine, which sets the current server
          availability threshold as desired. If successful, a message is 
          displayed.

          Any errors are processed and displayed to the user as appropriate.
            
***********************************************************************/

void setThresholdInfo (char *szServerName, int iThreshold)
{
    STATUS  nError;
    char    szErrorString[LINEOTEXT];       /* Error Message String */
    char    szCommand[LINEOTEXT];           /* remote command */
    char    szServerResponse[LINEOTEXT];    /* response buffer */
    WORD    wResponseLen = 0;               /* and length      */

    /* Check for input errors -> Value must be between 0 and 100 */
    if ( (iThreshold < 0) || (iThreshold > 100) ) 
    {
        nError = NPNERR_BAD_INPUT;
        goto Cleanup;
    }
    
    /* No errors, build console command string and send command */
    sprintf (szCommand, "%s%d", SET_THRESHOLD, iThreshold);
    nError = RemoteCommand (szServerName, szCommand, 
                            szServerResponse, &wResponseLen);
    
    /* Return if error with remote command */
    if (nError != NOERROR)
        goto Cleanup;

Cleanup:
    /* Result Processing */
    switch (nError)
    {
        case NPNERR_BAD_INPUT:
            strcpy (szErrorString,
                    "Invalid Threshold Value Entered\nValid Range: 0 - 100"); 
            printf("%s\n", szErrorString);
            break;

        case NOERROR:
            printf("Availability Threshold Updated\n");
            break;

        default:
            OSLoadString(hModule, ERR(nError),
                         szErrorString, LINEOTEXT-1);
            printf("%s\n", szErrorString);
            break;
    }
    return;
}

/************************************************************************

    FUNCTION: createcopyDB(char *, char *, chare *, char *, BOOL)

    PURPOSE:  Creates and Copies a database file using the specified
              sources and destinations. The CreateAndCopyDB
              routine (clfunc.c) is called to perform the relevant
              HCL C API for Notes/Domino functions.

    COMMENTS:

          This routine passes the input source and copy Server and 
          Database names along with the Replica copy flag value to the
          CreateAndCopyDB routine, which creates and copies the
          whole database (with ACL) as desired.
          
          If successful, a status message is displayed. 

          Any errors are processed and displayed to the user as appropriate.
            
***********************************************************************/

void createcopyDB (char *szServerName, char *szDBName, 
                   char *szCopyServer, char *szCopyDB, BOOL bReplFlag )
{
    STATUS  nError;
    char    szErrorString[LINEOTEXT];       /* Error Message String */
    DWORD   dwCopyFlag;
    
    /* Determine and set Replication Flag */
    if (bReplFlag)
        dwCopyFlag = DBCOPY_REPLICA;
    else
        dwCopyFlag = (DWORD)0;
          
    /* Pass the input parameters to the CreateCopy routine */
    nError = CreateAndCopyDB (szServerName, 
                              szDBName,
                              szCopyServer, 
                              szCopyDB,
                              dwCopyFlag);

    /* Result Processing */
    if ( nError != NOERROR)
    {
        OSLoadString(hModule, ERR(nError),
                     szErrorString, LINEOTEXT-1);
        printf("%s\n", szErrorString);
    }

    /* No errors ->  Change Window Title Context */
    else
        printf("Database Copy Successful\n");

    return;
}

int getcharI5(void)
{
    char Key1[256];
    memset(Key1, 0, 256);
    fflush(stdout);
    gets(Key1);
    return Key1[0];
}
