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

    PROGRAM:      dbdrive

    FILE:         dbdrive.c

    PURPOSE:      Shows how to implement a database driver.
 
****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/*
 *  Windows header file. Include only if compiling for Windows.
 */

#if (defined(DOS) && defined(W)) || (defined(NT))
#include <windows.h>
#endif

/*
 * C header files
 */ 

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#include <memory.h>

/*
 *  Domino and Notes header files
 */
 
#include <global.h>
#include <ixport.h>
#include <dbdrv.h>
#include <dbdrverr.h>
#include <osmem.h>
#include <nsfdata.h>
#include <names.h>
#include <osfile.h>

#if defined(OS390) 
#include <osmisc.h>
#endif

/*
 *  Local header files.
 */
 
#include "dbdrive.h"

#if defined(OS390) && (__STRING_CODE_SET__==ISO8859-1 /* ascii compile */)
#include <_Ascii_a.h>   /* NOTE: must be the LAST file included */
#endif /* OS390, ascii compile */

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

#define READ_PERMISSION		"r"


/*
 *  Global Data
 */

SESSIONQUEUE   far *pSessQ;     /* Global session pointer                            */
DHANDLE          ResultHandle;    /* Handle to data to be returned to Domino and Notes */
DWORD          dwResultLength;  /* Length of data to be returned to Domino and Notes */

/****************************************************************************
 *
 *   FUNCTION: MainEntryPoint:   Initialize the database driver
 *
 *  Do per-process initialization.  This is called just after
 *  the LoadLibrary, and is the first entry point in the library.
 *  When this entry point is called, all of the other entry point
 *  vectors are filled in by this routine.
 *
 *  Inputs:
 *      drv - database driver vector
 *
 *  Outputs:
 *      (routine) - error status
 *      drv vectors to other subroutines have been filled in
 *
 ****************************************************************************/

STATUS LNPUBLIC MainEntryPoint(DBVEC *drv)

{
    SESSIONQUEUE *pSessQHead;

/*
 *  Fill in the subroutine vectors
 */

    drv->Init = MainEntryPoint;         /* Driver initialization function  */
    drv->Term = DBDTerm;                /* Driver termination function     */
    drv->Open = DBDOpen;                /* Session initialization function */
    drv->Close = DBDClose;              /* session termination function    */
    drv->Function = DBDPerformFunction; /* DbLookup and DbColumn function  */

 /*
  *   Perform one-time initializations by initializing local
  *   session queue.  (In Windows, with single-instance DS,
  *   this is a queue shared by all processes.  In all other 
  *   environments, it's process-private.
  */

    pSessQHead = (SESSIONQUEUE *) __AllocateMemory((size_t)sizeof(SESSIONQUEUE));
    memset(pSessQHead, 0, sizeof(SESSIONQUEUE));

    pSessQ = pSessQHead;
      
/*
 *  Done
 */

    return(NOERROR);
}

 
/****************************************************************************
 *
 *   FUNCTION: DBDTERM: Terminate the database driver
 *
 *  Per-process termination routine, ASSUMING that all open sessions
 *  for this context have been closed by the time this is called.  This
 *  is called just prior to the FreeLibrary.
 *
 *  Inputs:
 *      drv - database driver vector
 *
 *  Outputs:
 *      (routine) - error status
 *
 ***************************************************************************/
 
STATUS LNPUBLIC DBDTerm(DBVEC *drv)
{

    /*
     *  There's not much to do here, just __FreeMemory the memory associated
     *  with the first entry in the session queue.
     */

    __FreeMemory (pSessQ);
    
    return(NOERROR);
}

 
/****************************************************************************
 *
 *  FUNCTION: DBDOPEN: -  Open a session
 *
 *  Any databases opened as a side-effect of Functions
 *  performed on this session will gather up their context in the
 *  hSession returned by this routine.
 *
 *  Inputs:
 *      drv - database driver vector
 *
 *  Outputs:
 *      *rethSession = filled in with session handle
 *      (routine) - error status
 *
 ***************************************************************************/

STATUS LNPUBLIC DBDOpen(DBVEC *vec, HDBDSESSION *rethSession)
{

    SESSIONQUEUE  *pNewSessQ;

/*
 *  If pSessQ->NodeNum is zero, no sessions have yet been defined, so
 *  define the first entry in the queue.
 */
 
    if (pSessQ->NodeNum == 0)
    {
        pSessQ->NodeNum = 1;   /* Assign a Node number.  */
        pSessQ->Next = (SESSIONQUEUE *) NULL; /* No next session in queue. */
        pSessQ->Prev = (SESSIONQUEUE *) NULL; /* No prev. session in queue.*/
    }

/*
 *  If pSessQ->NodeNum is not zero, allocate another session block,
 *  assign it a database handle, and set the links appropriately.
 */
 
    else
    {
        pNewSessQ = (SESSIONQUEUE *) __AllocateMemory((size_t)sizeof(SESSIONQUEUE));
        memset(pNewSessQ, 0, sizeof(SESSIONQUEUE));
    
        pNewSessQ->NodeNum = pSessQ->NodeNum + 1; /* Assign Node Number.*/
    
        pNewSessQ->Prev = pSessQ;
        pNewSessQ->Next = (SESSIONQUEUE *) NULL;  /* No next session in queue.*/
        pSessQ->Next = pNewSessQ;
        pSessQ = pNewSessQ;
    }
    *rethSession = (HDBDSESSION) pSessQ;

    return(NOERROR);
}

 
/****************************************************************************
 *
 *   FUNCTION: DBDCLOSE:  -  Close an open session
 *
 *  Close a session, and as a side-effect all databases whose context
 *  has been built up in hSession.
 *
 *  Inputs:
 *      drv - database driver vector
 *      hSession - session handle to close
 *
 *  Outputs:
 *      (routine) - error status
 *
 ***************************************************************************/

STATUS LNPUBLIC DBDClose(DBVEC *vec, HDBDSESSION hSession)

{
SESSIONQUEUE   *pTempSessQ = (SESSIONQUEUE *) NULL;/* temp sess queue ptr */
SESSIONQUEUE  *pTempPrevSessQ;
SESSIONQUEUE  *pTempNextSessQ;
STATUS        sError;
BOOL          HeaderNode = FALSE;

/*
 *  First, locate the session that corresponds to the session handle. If
 *  the session is found, return a pointer to it.  If there is no session
 *  with that handle, return an error.
 */
    pTempSessQ = (SESSIONQUEUE*) hSession;

/*
 *  Now deallocate all databases in the session's database queue.
 */

    if (sError = CloseDatabases(pTempSessQ->pDatabaseQ))
        return(ERR(sError));
    
/*
 *  Now remove the specified session from the session queue.
 */

    pTempPrevSessQ = pTempSessQ->Prev;
    pTempNextSessQ = pTempSessQ->Next;

    if (pTempPrevSessQ != (SESSIONQUEUE *)NULL)
        pTempPrevSessQ->Next = pTempNextSessQ;
    else
        HeaderNode = TRUE;
     
    if (pTempNextSessQ != (SESSIONQUEUE *)NULL)
        pTempNextSessQ->Prev = pTempPrevSessQ;

/*
 *  If not the first session, free it and reassign current pointer
 */
     
    if (!HeaderNode)
    {
        __FreeMemory(pTempSessQ);
        pSessQ = pTempPrevSessQ;
    }
    
    return(NOERROR);
}

/****************************************************************************
 *  FUNCTION: DBDPerformFunction: - Perform a database function
 *
 *  Perform a function on a session.  If any databases must be opened
 *  as a side-effect of this function, gather context into hSession
 *  so that it may be later deallocated/closed in Close.
 *
 *  Depending on whether DB_LOOKUP or DB_COLUMN is being called, there
 *  may be one argument (for DB_COLUMN) or two arguments (for DB_LOOKUP)
 *  passed in. The first parameter (for both functions) is an item of
 *  TYPE_TEXT_LIST with one entry - the filename of the database on
 *  which to operate. For DB_LOOKUP, the second argument will be another item
 *  of TYPE_TEXT_LIST with one entry - a key value for which to search.
 *
 *  Inputs:
 *      drv - database driver vector
 *      hSession - Session handle
 *      Function - function ID - either DB_LOOKUP or DB_COLUMN
 *      argc - number of arguments
 *      argl - array of argument lengths
 *      argv - array of argument pointers
 *
 *  Outputs:
 *      *rethResult = filled in with result handle
 *      *retResultLength = filled in with result buffer length
 *      (routine) - error status
 *
 ***************************************************************************/

STATUS LNPUBLIC DBDPerformFunction(DBVEC *vec, HDBDSESSION hSession,
                                   WORD Function,
                                   WORD argc, DWORD *argl, void **argv,
                                   DHANDLE *rethResult, DWORD *retResultLength)


{

    SESSIONQUEUE   *pTempSessQ;  /* Pointer to session information             */
    STATUS     sError = NOERROR;
    DBQUEUE    *pDBQ;            /* Pointer to database context information    */
    char       *pBuffer1;        /* Pointers used to construct function output.*/
    char       *pBuffer2;
    char       *pTempList;
    char       *pListHead;
    char       *pData;
    char       *pText;
    char       *pOldString;
       
    WORD       wStringCount = 0;   /* Variables for building function output. */
    WORD       wLenString = 0;
    WORD       wDirNameLen = 0;
    DWORD      dwBufLen = 0;                       

    DHANDLE      hTemp;
    char       *pFileName;             /* Pointer to database filename          */
    DWORD      FileNameLen;            /* Length of packed filename string.     */
    char       *pKey;                  /* Pointer to key string (for DB_LOOKUP) */
#if defined(OS390) && (__STRING_CODE_SET__!=ISO8859-1 /* ebcdic compile */)
    char       tempKey[MAX_STRING_LEN];/* Translation buffer for key data.      */
#endif /* OS390, ebcdic compile */

    DWORD      KeyLen;                 /* Length of packed key string           */
    char       *pFullPathName;         /* Full pathname for database file       */
    char       String[MAX_STRING_LEN]; /* String var. for reading from file.    */
    char       *pString;               /* Pointer to help parse input from file.*/
		    
/*
 *  Find the session with the specified session handle.
 */
      
    pTempSessQ = (SESSIONQUEUE*)hSession;

/*
 *  The filename of the database to work with is stored in the
 *  item of TYPE_TEXT_LIST stored in argv[0].  If the item passed in
 *  is not of TYPE_TEXT_LIST, an error has occurred, so exit.
 */
 
    pData  = (char *)argv[0];             /* Get pointer to data    */
    if (*(WORD *)pData != TYPE_TEXT_LIST) /* If not TYPE_TEXT_LIST  */
        return (ERR_DBD_DATATYPE);        /*  return an error code. */

/*
 *  Advance the pointer past the datatype word and past the LIST header,
 *  allocate a buffer to hold the filename member of the DATABASEQ 
 *  structure, then copy the filename to that buffer and null-
 *  terminate it. This will then be passed to the file open routine.
 */

    pData += sizeof(WORD);         /* Step past the datatype word         */
    pData += sizeof(LIST);         /*   and step past the list header.    */

    FileNameLen = *(WORD *)pData;  /* Save length of filename.            */
    pData += sizeof(WORD);         /* Step past filename length           */
    pFileName = pData;             /* Save pointer to database file name. */

    if (argc > 1)                  /* If there's more than 1 parameter,   */
    {
        pData = (char *) argv[1];  /* Then the 2nd one should be the key  */
                                   /*   on which to search;               */
        pData += sizeof(WORD);     /* Step past the datatype word         */
        pData += sizeof(LIST);     /*   and step past the list header.    */
	
        KeyLen = *(WORD *)pData;   /* Save length of key.                 */
        pData += sizeof(WORD);     /* Step past key length                */
#if defined(OS390) && (__STRING_CODE_SET__!=ISO8859-1 /* ebcdic compile */)
        OSTranslate(OS_TRANSLATE_LMBCS_TO_NATIVE, pData, KeyLen, tempKey, MAX_STRING_LEN);                
        pKey = tempKey;
#else
        pKey = pData;              /* Save pointer to key.                */
#endif /* OS390, ebcdic compile */
    }

/*
 *  Allocate a buffer in which to store the filename, and construct
 *  the full path to the database file.
 */
 
    pFullPathName = (char *) __AllocateMemory((size_t) MAXPATH); /* Allocate buffer */
    memset(pFullPathName, 0, MAXPATH);                 /* Init to zero    */
    
    wDirNameLen = OSGetDataDirectory(pFullPathName); 
#if defined(OS390) && (__STRING_CODE_SET__!=ISO8859-1 /* ebcdic compile */)
    OSTranslate(OS_TRANSLATE_LMBCS_TO_NATIVE, pFullPathName, wDirNameLen, pFullPathName, MAXPATH);
#endif /* OS390, ebcdic compile */
#if defined(UNIX)
    strcat(pFullPathName, "/");
#else
    strcat(pFullPathName, "\\");
#endif
    wDirNameLen = strlen(pFullPathName);
#if defined(OS390) && (__STRING_CODE_SET__!=ISO8859-1 /* ebcdic compile */)
    OSTranslate(OS_TRANSLATE_LMBCS_TO_NATIVE, pFileName, (size_t)FileNameLen, pFullPathName+wDirNameLen, (MAXPATH - (size_t)wDirNameLen));
#else
    memmove(pFullPathName+wDirNameLen,
        pFileName, (size_t)FileNameLen);     /* Copy filename to buffer.*/
#endif /* OS390, ebcdic compile */

/*
 *  Scan the session's database queue. If the database is not
 *  already open, open it and add an entry to the database queue.
 *  If the database is already open, we don't need to reopen it.
 */

     if (sError = OpenDatabase(pTempSessQ,
                               (char *)pFullPathName,
                               &pDBQ) != NOERROR)
                               return (ERR(sError));
    
/*
 * dispatch based on function
 */

    switch (Function)
    {
        case DB_LOOKUP:

            /*
             *  Allocate 2 buffers; The TYPE_TEXT_LIST header will be built in one
             *  of them, and the list of text strings will be placed in the
             *  other. They will be combined to create the TEXT_LIST that is
             *  returned by this function.
             *  Remember that an item of TYPE_TEXT_LIST consists of:
             *
             *      a WORD specifying the datatype as TYPE_TEXT_LIST;
             *      a LIST specifying how many strings are in the list
             *          (call it N);
             *      a sequence of N words, each specifying the length of
             *           the corresponding string in the list;
             *      the packed strings themselves;
             */
  
            if (sError = OSMemAlloc(0, MAX_BUFF_LEN, rethResult))
                return (ERR(sError));
  
            pBuffer1 = (char *)OSLockObject(*rethResult);       
            memset (pBuffer1, 0, MAX_BUFF_LEN);
  
            if (sError = OSMemAlloc(0, MAX_BUFF_LEN, &hTemp))
            {
                OSUnlockObject(*rethResult);
                OSMemFree(*rethResult);
                return (ERR(sError));
            }
  
            pBuffer2 = (char *)OSLockObject(hTemp);
            memset (pBuffer2, 0, MAX_BUFF_LEN);
        
            /*
             *  pListHead points to the LIST member of the Text List.
             *  pText will always point to the location to write the next string.
             *  wBuffLen contains the cumulative lengths of the packed strings
             *  in the text list; pTempList points to the current position in the
             *  text list header.
             */
     
            *((WORD *) pBuffer1) = TYPE_TEXT_LIST; /* set the datatype word. */
            pListHead = pBuffer1 + sizeof(WORD);   /* Set ptr to start of textlist.*/
            pText = pBuffer2;
            dwBufLen = 0;
            pTempList = pListHead + sizeof(WORD); /* Point to location to     */
                                                  /*   store length of string */
  
            /*
             *  Now perform the function on the specified database.
             *  Each line in the database consists of two strings, seperated by
             *  a comma.  For each line in the database, Read the first string.
             *  If it matches the "key" string (ignoring case), then add the
             *  second string from that line to the text list that will be
             *  returned.
             */
  
            /* Reset file ptr to start of file.  */
            __FileSeek(pDBQ->pFile, 0L, SEEK_SET);
        
            /* Read line from file   */
            while ( __ReadLine( String, MAX_STRING_LEN-1, pDBQ->pFile) )
            {
                pString = strchr(String, ',');
                wLenString = (WORD)(pString - String);
                /*
                 *  If the key value of this line does not match the
                 *  key passed in (ignoring case), break out of this
                 *  iteration of the loop and get the next line from the file.
                 */
	
                if (memcmp(String, pKey, (size_t) wLenString) != 0)
                    continue; 
	
                while (!isalnum(*pString))        /* Skip chars between key */
                                                  /*   and string.          */
                {
                    pString += sizeof(char);
                }
                wLenString = strlen(pString)-1;
                dwBufLen += wLenString;         /* Add to total textlist len.*/
#if defined(OS390) && (__STRING_CODE_SET__!=ISO8859-1 /* ebcdic compile */)
                OSTranslate (OS_TRANSLATE_NATIVE_TO_LMBCS, pString, wLenString, pText, (MAX_BUFF_LEN - dwBufLen));
#else
                memmove(pText,
                        pString,
                        wLenString);            /* Copy to TEXT_LIST buffer. */
#endif /* OS390, ebcdic compile */

                wStringCount++;     /* Increment count of strings in text list.*/
                *((WORD *) pTempList) = (wLenString); /* Set size of this string. */
                pTempList += sizeof(WORD);  /* Increment ptr to string lengths. */
                pText +=(wLenString);       /* Increment ptr to strings. */
            }
  
            /*
             * All strings have been read into the string buffer. Set the
             * count of entries in the textlist, then append the string
             * buffer to the textlist header buffer.
             */
  
            *((WORD *) pListHead) = wStringCount;
            memmove(pTempList, pBuffer2, (pText - pBuffer2));
            dwBufLen += (pTempList - pBuffer1); /* Add lengths of both buffers */
            *retResultLength = dwBufLen;        /* Return combined length. */
           
            /*
             *  Unlock the two buffers, and __FreeMemory the temp buffer.
             */
  
            OSUnlockObject(*rethResult);
            OSUnlockObject(hTemp);
            OSMemFree(hTemp);
  
            return(NOERROR);
            break;
      

        case DB_COLUMN:

            /*
             *  Allocate 2 buffers; The TYPE_TEXT_LIST header will be built in one
             *  of them, and the list of text strings will be placed in the
             *  other. They will be combined to create the TEXT_LIST that is
             *  returned by this function.
             *  Remember that an item of TYPE_TEXT_LIST consists of:
             *
             *      a WORD specifying the datatype as TYPE_TEXT_LIST;
             *      a LIST specifying how many strings are in the list
             *          (call it N);
             *      a sequence of N words, each specifying the length of
             *          the corresponding string in the list;
             *      the packed strings themselves;
             */
  
            if (sError = OSMemAlloc(0, MAX_BUFF_LEN, rethResult))
                return (ERR(sError));
  
            pBuffer1 = (char *)OSLockObject(*rethResult);       
            memset (pBuffer1, 0, MAX_BUFF_LEN);
  
            if (sError = OSMemAlloc(0, MAX_BUFF_LEN, &hTemp))
            {
                OSUnlockObject(*rethResult);
                OSMemFree(*rethResult);
                return (ERR(sError));
            }
  
            pBuffer2 =(char *) OSLockObject(hTemp);
            memset (pBuffer2, 0, MAX_BUFF_LEN);
         
            /*
             *  pListHead points to the LIST member of the Text List.
             *  pText will always point to the location to write the next string.
             *  wBuffLen contains the cumulative lengths of the packed strings
             *  in the text list; pTempList points to the current position in the
             *  text list header.
             */
            *((WORD *) pBuffer1) = TYPE_TEXT_LIST; /* Set the datatype word. */
            pListHead = pBuffer1 + sizeof(WORD);   /* Point to start of textlist. */
            pText = pBuffer2;
            dwBufLen = 0;
            pTempList = pListHead + sizeof(WORD); /* Point to place to store */
                                                  /*   length of 1st string. */
  
            /*
             *  Now perform the function on the specified database.
             *  Each line in the database consists of two strings, seperated by
             *  a comma.  For each line in the database, get the first string in
             *  the line and add the string to the text list that will be
             *  returned if that string is not already in the text list.
             *
             *  Note:  the lines in the database are sorted alphabetically
             *  by the first string on the line.
             */

            pOldString = "";
  
            /* Reset file ptr to start of file.  */
            __FileSeek(pDBQ->pFile, 0L, SEEK_SET);
           
            /* Read line from file   */
            while ( __ReadLine( String, MAX_STRING_LEN-1, pDBQ->pFile) )
            {
                pString = strchr(String, ',');
                wLenString = (WORD)(pString - String);
                dwBufLen += wLenString;     /* Add to total size of list. */
	
                /*
                 *  If the string just read from the file matches the last one
                 *  written to the buffer, then dond't write it again.
                 */
             
                if (memcmp(String, pOldString, (size_t) wLenString) == 0)
                    continue;
	
                memmove(pText,
                        String,
                        wLenString);        /* Copy to TEXT_LIST buffer.  */
                        
                        wStringCount++;     /* Increment count of strings in list. */
                        *((WORD *) pTempList) = (wLenString);  /* Set size of this string. */
                        pTempList += sizeof(WORD);  /* Increment ptr to string lengths. */
                        pOldString = pText;         /* Save ptr to most recent string. */
                        pText +=(wLenString);       /* Increment ptr to strings.    */
            }
  
            /*
             * All strings have been read into the string buffer. Set the
             * count of entries in the textlist, then append the string
             * buffer to the textlist header buffer.
             */
  
            *((WORD *) pListHead) = wStringCount;
            memmove(pTempList, pBuffer2, (pText - pBuffer2));
            dwBufLen += (pTempList - pBuffer1); /* Add lengths of both buffers */
            *retResultLength = dwBufLen;        /* Return combined length.     */
           
            /*
             *  Unlock the two buffers, and __FreeMemory the temp buffer.
             */
  
            OSUnlockObject(*rethResult);
            OSUnlockObject(hTemp);
            OSMemFree(hTemp);
  
            return(NOERROR);
            break;
      
        default:
            /*
             *  The function request was neither DB_LOOKUP nor DB_COLUMN, so
             *  return an error code.
             */

            return (ERR_DBD_FUNCTION);
            break;
    }

    return (NOERROR);
}
 
 
/****************************************************************************
 *   FUNCTION: OpenDatabase - open a database, and add its context to the
 *                            session's database queue.
 *
 *   Given a session and a database name, scan the session's database queue
 *   and see if the session already has the file open.  Return a pointer to
 *   its database queue entry if so.  If not, open the database, add it
 *   to the session's database queue, and return a pointer to the
 *   database queue entry.
 *
 ***************************************************************************/

   
STATUS LNPUBLIC OpenDatabase(SESSIONQUEUE *pSessionQ,
                             char *pFileName,
                             DBQUEUE **pDatabaseQ)
{
	
DBQUEUE    *pDBQ;     /* pointers used to traverse and build database queue. */
DBQUEUE    *pDBQ2;
DBQUEUE    *pTempDBQ;
BOOL       DbFound = FALSE;

/*
 *  Scan the database queue to see if the specified file is
 *  already open. If so, no need to reopen it.
 */

    pDBQ = pDBQ2 = pSessionQ->pDatabaseQ;  /* ptr to 1st db queue entry */
    while ((pDBQ != (DBQUEUE *) NULL) && DbFound == FALSE)
    {
        if (strcmp (pDBQ->DbName, pFileName)== 0)
            DbFound = TRUE;
        else
        {
            pDBQ2 = pDBQ;
            pDBQ = pDBQ->Next;
        }
    }
  
  
/*
 *  If the database isn't already open and in the queue,
 *  allocate memory for a database queue entry, open the file,
 *  and add it to the queue. 
 */
  
    if (!DbFound)
    {
        pTempDBQ = (DBQUEUE *) __AllocateMemory((size_t) sizeof(DBQUEUE));
        memset (pTempDBQ, 0, sizeof(DBQUEUE));

        /*
         *  Open the database file in read mode.
         */
     
        pTempDBQ->pFile = __OpenFile( pFileName, READ_PERMISSION );
        if ( pTempDBQ->pFile == NULL )
        {
            __FreeMemory(pTempDBQ);
            return (ERR_OPENING_FILE); /* Couldn't open the file. */
        }

        /*
         *  Set the name of the database file in the database queue entry.
         */
     
        pTempDBQ->DbName = pFileName;

        pDBQ = pTempDBQ;

        /*
         *  If this is the first database in this session's queue,
         *  place the new entry in the head of the queue. 
         *  Otherwise, this new database queue entry is added to the end
         *  of the database queue. 
         */
     
        if (pSessionQ->pDatabaseQ == (DBQUEUE *) NULL)
        {
            pSessionQ->pDatabaseQ = pDBQ;
        }
        else
        {
            pDBQ2->Next = pDBQ;
            pDBQ->Prev = pDBQ2;
        }
    }
    *pDatabaseQ = (DBQUEUE *)pDBQ;
    return (NOERROR);
}

/****************************************************************************
 *
 *   FUNCTION: CloseDatabases - Close all databases in a session's
 *                              database queue.
 *
 ***************************************************************************/


STATUS LNPUBLIC CloseDatabases(DBQUEUE *pDatabaseQ)
{

    DBQUEUE *pTempDBQ;

/*
 *  First, check to see if there are any entries in the queue.  If not,
 *  there are no databases to close.
 */

    if (pDatabaseQ == NULL)
        return (NOERROR);

/*
 * For each entry in the queue, close the database, clear the database's
 * file pointer, get a pointer to the next entry in the queue,
 * and __FreeMemory the current database queue entry.
 */
	
    while ( pDatabaseQ->DbName )
    {
        pTempDBQ = pDatabaseQ;						/* Save ptr to this queue entry  */
	
        __CloseFile(pDatabaseQ->pFile);				/* Close the database associated */
        pDatabaseQ->pFile = (TDBQueueFile) NULL;	/* Clear the file pointer.       */
                                                	/*   with this queue entry.      */
        __FreeMemory(pDatabaseQ->DbName);           /* __FreeMemory filename buffer.         */
        pDatabaseQ->DbName = (char *) NULL; 		/* clear the database filename.  */ 

        /*
         *  If there is a next entry in the database queue, get a pointer to it.
         *  Otherwise we're done.
         */
     
        if (pDatabaseQ->Next)
        {
            pDatabaseQ = pDatabaseQ->Next;
            __FreeMemory(pTempDBQ);      /* free up old queue entry. */
        }
    }
    __FreeMemory(pTempDBQ);              /* free up last queue entry. */

   return (NOERROR);
}

/*============================================================================
            W I N D O W S  S P E C I F I C  F U N C T I O N S 
============================================================================*/
/*
* LibMain and WEP only need to be defined under Windows.
*/
 
#if defined(W) && defined(DOS)

/****************************************************************************
   FUNCTION: LibMain

   PURPOSE:  Standard Windows 3.X DLL entry point.
	   
*******************************************************************************/
int LNPUBLIC LibMain(hModule, wDataSeg, wHeapSize, lpszCmdLine)
DHANDLE  hModule;
WORD    wDataSeg;
WORD    wHeapSize;
LPSTR   lpszCmdLine;
{
    if (wHeapSize > 0)
        UnlockData(0);
    return 1;
}

/****************************************************************************
    FUNCTION:  WEP(int)

    PURPOSE:  Performs cleanup tasks when the DLL is unloaded.  WEP() is
	      called automatically by Windows when the DLL is unloaded (no
	      remaining tasks still have the DLL loaded).  It is strongly
	      recommended that a DLL have a WEP() function, even if it does
	      nothing but returns success (1), as in this example.

*******************************************************************************/
int LNPUBLIC WEP (bSystemExit)
int  bSystemExit;
{
    return(1);
}

#endif  

