/****************************************************************************
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

DBDRIVE.H

Function prototypes, structure definitions, and #defines for the
DBDRIVE sample program.

	
****************************************************************************/
#ifndef  __DBDRV
#define  __DBDRV

/*
 *  Some equates and error codes used in this sample
 */

#define MAX_BUFF_LEN   8*1024      /* Max size of TEXT_LIST */
#define MAX_STRING_LEN 256         /* Max len of string read from file. */


#define  ERR_SESSION_NOT_FOUND  PKG_DBD + 190 /* couldn't find session */
#define  ERR_OPENING_FILE       PKG_DBD + 191 /* couldn't open file */


/*
 *  Database driver functions
 */
STATUS LNPUBLIC MainEntryPoint(DBVEC *drv);
STATUS LNPUBLIC DBDTerm(DBVEC *drv);
STATUS LNPUBLIC DBDOpen(DBVEC *vec, HDBDSESSION *rethSession);
STATUS LNPUBLIC DBDClose(DBVEC *vec, HDBDSESSION hSession);
STATUS LNPUBLIC DBDPerformFunction(DBVEC *vec, HDBDSESSION hSession,
                            WORD Function,
                            WORD argc, DWORD *argl, void **argv,
                            HANDLE *rethResult, DWORD *retResultLength);

/*
 *  Database queue entry data structure.
 */

typedef FILE   *	TDBQueueFile;

typedef struct _dbqueue_tag
{
	TDBQueueFile	pFile;        /* Database file pointer. */
    char   *DbName;             /* Name of file. */
    struct _dbqueue_tag *Next;  /* Ptr to next database in queue. */
    struct _dbqueue_tag *Prev;  /* Ptr to previous database in queue. */
} DBQUEUE;

/*
 *  Session queue entry data structure.
 */
                            
typedef struct _sessionqueue_tag
{
    WORD NodeNum;                    /* Node number */
    struct _sessionqueue_tag *Next;  /* Ptr to next entry in queue */
    struct _sessionqueue_tag *Prev;  /* Ptr to previous entry in queue */
    DBQUEUE *pDatabaseQ;             /* Ptr to Queue of db's for this session. */
} SESSIONQUEUE;


/*
 *  Misc. functions used by this sample program.
 */
 
STATUS LNPUBLIC FindSession(HDBDSESSION hSession,
                              SESSIONQUEUE **pSessionQ);
   
STATUS LNPUBLIC OpenDatabase(SESSIONQUEUE *pSessionQ,
                               char *pFileName,
                               DBQUEUE **pDatabaseQ);

STATUS LNPUBLIC CloseDatabases(DBQUEUE *pDatabaseQ);

/*---------------------------------------------------------
	PLATFORM SPECIFIC MACROS FOR FILE AND MEMORY FUNCTIONS
-----------------------------------------------------------*/

/* Memory Allocation */
#define __AllocateMemory(__sz)					malloc( __sz )
#define __FreeMemory(__ptr)						free( __ptr )

/* File Managment */
#define __OpenFile( _fn, _perm )				fopen(_fn, _perm )
#define __CloseFile( _file )					fclose( _file )
#define __FileSeek( _file, _offset, _whence )	fseek( _file, _offset, _whence )
#define __ReadLine( _buf, _len, _f )			fgets( _buf, _len, _f )


#endif /* __DBDRV */
