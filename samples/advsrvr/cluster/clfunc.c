/******************************************************************************
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
    
    FILE:       CLFUNC.C

    PURPOSE:    Cluster function module that is referenced by the CLUMON sample
                program. Contains routines for gathering and managing Advanced 
                Server Cluster information via the HCL C API for Notes/Domino. 
				For platform independence considerations, there are no UI specific
                references in this module.

 ******************************************************************************/

#include <malloc.h>
#include <global.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <string.h>
#include <stdlib.h>
#include <ns.h>
#include <textlist.h>
#include <srverr.h>
#include <osmem.h>
#include <osfile.h>
#include <dname.h>

#include "clumon.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/* Function prototypes */
STATUS GetServerLoad (char FAR*, DWORD FAR *);                    
STATUS GetServerCluster (char FAR*, char FAR*);	            
STATUS GetServerClusterMates (char FAR*, DWORD, DHANDLE FAR*);  
STATUS GetDBMarks (char FAR*, char FAR*, DWORD*, BOOL*);
STATUS SetDBMarks (char FAR*, char FAR*, WORD);
STATUS RemoteCommand (char FAR*, char FAR*, char FAR*, WORD*);
STATUS CreateAndCopyDB (char FAR*, char FAR*, char FAR*, char FAR*, DWORD);  

/******************************************************************************
 * GetServerLoad()
 *
 * Routine to get the load (availability) index of the specified server.  
 *
 * CALLED HCL C API for Notes/Domino:
 *        DNCanonicalize(), NSPingServer()
 *
 ******************************************************************************/

STATUS GetServerLoad ( char FAR *pServerName, /* server name */
                       DWORD FAR *dwLoadIndex /* returned availability */
                     )
{
    char    szCanonServerName[MAXUSERNAME]; /* Canonicalized Name of Server */
    STATUS  nError;

    /* Canonicalize the servername if it isn't already done.  
     * The NSPingServer should use a canonicalized servername as input
     */
    nError = DNCanonicalize( 0L, NULL, pServerName, (char FAR*)szCanonServerName, 
                             MAXUSERNAME, NULL);
    if (nError != NOERROR)
        return (nError);

    /* Call NSPingServer - only interested in load index */
    nError = NSPingServer( (char FAR *)szCanonServerName, dwLoadIndex, NULL);
    
    /* And return the status */
    return( nError);
}

	
/******************************************************************************
 * GetServerCluster()
 *
 * Routine to get name of the cluster (if any) configured for the specified
 * server. 
 *
 * CALLED HCL C API for Notes/Domino:
 *        DNCanonicalize(), NSPingServer(), ListGetText(), ListGetNumEntries(),
 *        OSLock(), OSUnlock(), OSMemFree()
 *
 *****************************************************************************/

STATUS GetServerCluster ( char FAR *pServerName,  /* server name */
                          char FAR *pClusterName  /* returned cluster name */
                         )
{
    WORD        wNumListEntries = 0;
    WORD        wBufferLen = 0;
    void FAR    *lpList;
    char        *pBuffer;
    char        achClusterName[MAXUSERNAME];
    DHANDLE     hList=NULLHANDLE;
    char        szCanonServerName[MAXUSERNAME]; /* Canonicalized Name of Server */
    STATUS      nError;
    
    /* Canonicalize the servername if it isn't already done.  
    * The NSPingServer should use a canonicalized servername as input
    */
    nError = DNCanonicalize( 0L, NULL, pServerName, (char FAR*)szCanonServerName, 
                             MAXUSERNAME, NULL);
    if (nError != NOERROR)
        return (nError);

    /* and call NSPingServer - only interested in cluster list */
    nError = NSPingServer( (char FAR *)szCanonServerName, NULL, &hList);
    
    /* If the server is unavailable, proceed - can still get its cluster name */
    if (!nError || (ERR(nError) == ERR_SERVER_UNAVAILABLE))
    {
        /* if the list handle is NULL, that indicates that this server doesn't
         * belong to a cluster
         */
        if (hList == NULLHANDLE)
            return(nError = NPNERR_NOT_CLUSTER_MEMBER);
        else
        {
            /* Lock down the list so we can use it */
            lpList = OSLock( void, hList);
            wNumListEntries = ListGetNumEntries( lpList, FALSE);
            if (wNumListEntries > 0)
            {
                /* The first entry in the list is the cluster name */
                nError = ListGetText( lpList, FALSE, 0, &pBuffer, &wBufferLen);
                if (!nError)
                {
                    strncpy( achClusterName, pBuffer, wBufferLen);
                    achClusterName[wBufferLen] = '\0';
                    OSUnlock( hList);
#ifndef UNIX
                    lstrcpy( pClusterName, (char FAR *)achClusterName);    
#else
                    strcpy( pClusterName, (char FAR *)achClusterName);    
#endif
                }
                else
                {
                    nError = NPNERR_GETTING_CLUSTER_NAME;
                    OSUnlock( hList);
                    goto Cleanup;
                }
            }
        }
    }
    else
        return(nError);

Cleanup:
    if (hList != NULLHANDLE)        
        OSMemFree( hList);

    return( nError);
}


/******************************************************************************
 * GetServerClusterMates()
 *
 * Routine to get the cluster mates (if any), as configured for the specified
 * server. This routine returns a pointer to a handle of a text list which
 * contains the members of the cluster.	The caller is responsible for freeing up
 * the allocated text list.
 *
 * CALLED HCL C API for Notes/Domino:
 *        DNCanonicalize(), NSGetServerClusterMates(), OSMemFree()
 *
 ******************************************************************************/

STATUS GetServerClusterMates ( char FAR *pServerName, /* server name */ 
                               DWORD dwLookupFlags,   /* lookup flags */
                               DHANDLE *hRetList       /* returned clustermates */    
                              )
{
    char    szCanonServerName[MAXUSERNAME]; /* Canonicalized Name of Server */
    STATUS  nError;
    
    /* Canonicalize the servername if it isn't already done.  
     * The NSGetServerClusterMates requires a fully canonicalized servername
     * as input
     */
    nError = DNCanonicalize( 0L, NULL, pServerName, (char FAR*)szCanonServerName, 
                             MAXUSERNAME, NULL);
    if (nError != NOERROR)
        return (nError);

    /* And call NSGetServerClusterMates */
    nError = NSGetServerClusterMates( (char FAR *)szCanonServerName, 
                                       dwLookupFlags, hRetList);
    
    /* cleanup if error */	        
    if (nError != NOERROR)
    {
        if (*hRetList != NULLHANDLE)        
        {
            OSMemFree( *hRetList);
            *hRetList = NULLHANDLE;
        }
    }
    return( nError);
}


/******************************************************************************
 * GetDBMarks()
 *
 * Routine to get the open database option flags of the specified database file.
 * This routines opens the database (with failover support), determines if 
 * failover occurs, gets the options, closes the database, and returns the 
 * option flag and failover indication to the calling routine.
 *
 * CALLED HCL C API for Notes/Domino:
 *        DNCanonicalize(), DNAbbreviate(),
 *        OSPathNetConstruct(), OSPathNetParse(),
 *        NSFDbPathGet(), NSFDbOpenExtended(), NSFDbGetOptions(), NSFDbClose()
 *
 ******************************************************************************/

STATUS GetDBMarks ( char FAR *pServerName,  /* database server name */ 
                    char FAR *pDBName,      /* database file name */ 
                    DWORD    *dwOptionMask, /* open option flags */
                    BOOL     *bFailover     /* TRUE if server failover */         
                   )
{
    STATUS        nError;
    DHANDLE       hDb;            /* NSFDbOpenExtended parameters */
    TIMEDATE      dataNoteMod;
    TIMEDATE      nonDataNoteMod;

    char    szCanonServerName[MAXUSERNAME]; /* Canonicalized Name of Server */
    char    szNetPathName[MAXPATH];         /* Network Path of Database */

    char    szFailoverServerName[MAXUSERNAME]; /* Failover Server Name */
    char    szFailoverDBName[MAXUSERNAME];     /* Failover DB Name */
    char    szFailoverPathName[MAXPATH];       /* Expanded Failover Path Name */ 

    /* Canonicalize the Servername */
    nError = DNCanonicalize( 0L, NULL, pServerName, 
                                (char FAR *)szCanonServerName, MAXUSERNAME, NULL);
    if (nError != NOERROR)
        return (nError);
            
    /* Get the DB Options */  
    /* 1) Construct NetPath */
    nError = OSPathNetConstruct (NULL, (char FAR *)szCanonServerName, 
                                 pDBName, (char FAR *)szNetPathName);
    if (nError != NOERROR)       
        return nError;

    /* 2) Open the Database via NSFDbOpenExtended (support server failover)*/
    nError = NSFDbOpenExtended ((char FAR *)szNetPathName, 
                                DBOPEN_CLUSTER_FAILOVER, NULLHANDLE, NULL,
                                &hDb, &dataNoteMod, &nonDataNoteMod );
    if (nError != NOERROR )
        return nError;

    /* 3) Check for clustered server failover by getting the path name of the
          opened database */  
    nError = NSFDbPathGet(hDb, (char FAR *)szFailoverPathName, NULL);
    if (nError != NOERROR )
    {
        (void) NSFDbClose (hDb);
        return nError;
    }

    /* parsing out the server and database names */
    nError = OSPathNetParse ((char FAR *)szFailoverPathName, NULL, 
                             (char FAR *)szFailoverServerName, 
                             (char FAR *)szFailoverDBName);
    if (nError != NOERROR )
    {
        (void) NSFDbClose (hDb);
        return nError;
    }

    /* and comparing them to the specified parameters */
#ifndef UNIX
    if (lstrcmpi((char FAR *)szCanonServerName, (char FAR *)szFailoverServerName))
#else
    if (strcmp((char FAR *)szCanonServerName, (char FAR *)szFailoverServerName))
#endif
    {
        /* Failover occured -> return new abbreviated server/db name */
        *bFailover = TRUE;
        nError = DNAbbreviate (0L, NULL, (char FAR *)szFailoverServerName,
                               pServerName, MAXUSERNAME, NULL);
#ifndef UNIX
        lstrcpy (pDBName, (char FAR *)szFailoverDBName);
#else
        strcpy (pDBName, (char FAR *)szFailoverDBName);
#endif
        if (nError != NOERROR )
        {
            (void) NSFDbClose (hDb);
            return nError;
        }
    }
    else
        *bFailover = FALSE;

    /* 4) Get the Database options via NSFDBGetOptions */
    nError = NSFDbGetOptions (hDb, dwOptionMask);
    if (nError != NOERROR )
    {
        (void) NSFDbClose (hDb);
        return nError;
    }

    /* 5) Close the Database and return */
    nError = NSFDbClose (hDb);
    return nError;
}

/******************************************************************************
 * SetDBMarks()
 *
 * Routine to set the open database option flags of the specified database file.
 * This routines calls the NSFDbMarkxxx API functions to mark the database either 
 * in service, out of service, or for deletion, as specified by the passed flag.
 *
 * CALLED HCL C API for Notes/Domino:
 *        DNCanonicalize(), OSPathNetConstruct(), 
 *        NSFDbMarkInService(), NSFDbMarkOutOfService(), NSFDbMarkForDelete()
 *
 ******************************************************************************/

STATUS SetDBMarks ( char FAR *pServerName, /* database server name */ 
                    char FAR *pDBName,     /* database file name */ 
                    WORD wMarkFlag         /* mark flag */
                  )
{
    STATUS  nError;

    char    szCanonServerName[MAXUSERNAME];  /* Canonicalized Name of Server */
    char    szNetPathName[MAXPATH];          /* Network Path of Database */

    /* Canonicalize the Servername */
    nError = DNCanonicalize( 0L, NULL, pServerName, 
                                (char FAR *)szCanonServerName, MAXUSERNAME, NULL);
    if (nError != NOERROR)
        return nError;

    /* Construct NetPath */
    nError = OSPathNetConstruct (NULL, (char FAR *)szCanonServerName, 
                                pDBName, (char FAR *)szNetPathName);

    /* And call relevent NSFMark functions */
    /* NSFDbMarkInService() */
    if ( wMarkFlag & MARK_IN_SERVICE )
        nError = NSFDbMarkInService ( (char FAR *)szNetPathName );
    if (nError != NOERROR )
        return nError;
   
    /* NSFDbMarkOutOfService() */
    if ( wMarkFlag & MARK_OUT_SERVICE )
        nError = NSFDbMarkOutOfService ( (char FAR *)szNetPathName );
    if (nError != NOERROR )
        return nError;
   
    /* NSFDbMarkForDelete() */
    if ( wMarkFlag & MARK_DELETE )
        nError = NSFDbMarkForDelete ( (char FAR *)szNetPathName );

    return nError;
}


/******************************************************************************
 * RemoteCommand()
 *
 * Routine to issue a remote server console command and solicit a response via
 * the HCL C API for Notes/Domino function NSFRemoteConsole.  This routine 
 * is used to modify the specified server's configuration where there are no 
 * supporting C API for Domino and Notes.
 *
 * This routine is responsible for locking and freeing the response buffer 
 * returned by Domino and Notes. The response is truncated to fit in the caller's 
 * buffer as specified by the "wResponseLen" argument.   The actual response length
 * is then returned in the "wRepsonseLen" argument. 
 *
 * CALLED HCL C API for Notes/Domino:
 *        DNCanonicalize(), NSFRemoteConsole()
 *        OSLock(), OSUnlock(), OSMemFree(), 
 *
 ******************************************************************************/

STATUS RemoteCommand(char FAR *pServerName,     /* server name */
                     char FAR *pConsoleCommand, /* remote console command */
                     char FAR *pServerResponse, /* response buffer */
                     WORD *wResponseLen)        /* desired/actual response length */
{
    STATUS nError;
    char szCanonServerName[MAXUSERNAME];   
    DHANDLE hServerResponse = NULLHANDLE;
    WORD i;
    char *pBuffer;
    WORD wBufferSize;
    
    /* Canonicalize the Servername */
    nError = DNCanonicalize( 0L, NULL, pServerName, (char FAR *)szCanonServerName, 
                                MAXUSERNAME, NULL);
    if (nError != NOERROR)
        return nError;

    /* and execute the remote console command to the specified server */
    nError = NSFRemoteConsole( szCanonServerName, pConsoleCommand, 
                               &hServerResponse);
    
    /* if no error, then package the returned response buffer */
    if (nError == NOERROR)
    {
        pBuffer = OSLock( char, hServerResponse);
        wBufferSize = strlen( pBuffer);
        
        for (i = 0; i < wBufferSize; i++ )
        {
            /* Fillin the caller's buffer with the response */
            if( i < *wResponseLen)
            {
                /* If we've reached a NULL in the buffer returned by the server,
                 * then we're done
                 */
                if (pBuffer[i] == '\0')
                    break;
                else    
                    pServerResponse[i] = pBuffer[i];
            }
            else
                break;    
        }
        pServerResponse[i] = '\0';
        *wResponseLen = wBufferSize;  
        OSUnlock( hServerResponse);
    }
    
    if (hServerResponse != NULLHANDLE)
        OSMemFree( hServerResponse);
        
    return(nError);
}    

/******************************************************************************
 * CreateAndCopyDB()
 *
 * Routine to create and copy a specified database file to a specified 
 * destination via the C API for Domino and Notes function NSFDbCreateAndCopy(). 
 *
 * CALLED HCL C API for Notes/Domino:
 *        DNCanonicalize(), OSPathNetConstruct(), NSFDbCreateAndCopy()
 *
 ******************************************************************************/

STATUS CreateAndCopyDB ( char FAR *pServerName, /* source server name */
                         char FAR *pDBName,     /* source database file name */ 
                         char FAR *pServerDest, /* destination server name */
                         char FAR *pDBDest,     /* destination database name */
                         DWORD    dwCopyFlag    /* copy flag */
                        )
{
    STATUS  nError;
    char    szCanonServerName[MAXUSERNAME]; /* Canonicalized Source Name */
    char    szCanonServerDest[MAXUSERNAME]; /* Canonicalized Dest. Name  */
    char    szNetPathName[MAXPATH];         /* Network Path of Source Db */
    char    szNetPathDest[MAXPATH];         /* Network Path of Dest. Db */
    DBHANDLE hCopyDb = NULLHANDLE;          /* returned DB handle */

    /* Canonicalize the source and destination servernames
     * The NSFDbCreateAndCopy should use canonicalized servernames as input
     */
    nError = DNCanonicalize( 0L, NULL, pServerName, (char FAR*)szCanonServerName, 
                             MAXUSERNAME, NULL);
    if (nError != NOERROR)
        return (nError);

    nError = DNCanonicalize( 0L, NULL, pServerDest, (char FAR*)szCanonServerDest, 
                             MAXUSERNAME, NULL);
    if (nError != NOERROR)
        return (nError);

    /* Construct the network paths for the source and destination files */
    nError = OSPathNetConstruct (NULL, (char FAR *)szCanonServerName, 
                                 pDBName, (char FAR *)szNetPathName);
    if (nError != NOERROR)       
        return nError;

    nError = OSPathNetConstruct (NULL, (char FAR *)szCanonServerDest, 
                                 pDBDest, (char FAR *)szNetPathDest);
    if (nError != NOERROR)       
        return nError;

    /* Call NSFDbCreateAndCopy - using no size limit and copying all data */
    nError = NSFDbCreateAndCopy ((char FAR *)szNetPathName,
                                 (char FAR *)szNetPathDest,
                                 NOTE_CLASS_ALL,           /* copy all data */
                                 (WORD)0,                  /* no size limit */
                                 dwCopyFlag,               
                                 &hCopyDb);

    if (nError != NOERROR)       
        return nError;

    /* Close the returned open copy handle - since we do not use it */
    if (hCopyDb != NULLHANDLE)
        nError = NSFDbClose (hCopyDb);

    /* return the status */
    return( nError);
}
