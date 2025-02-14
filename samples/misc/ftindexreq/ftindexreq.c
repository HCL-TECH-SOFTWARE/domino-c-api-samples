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

    PROGRAM:    ftindexreq

    FILE:       ftindexreq.c

    DESCRIPTION:
        This program illustrates some of the full text indexing request capabilities.

        The user specifies a server database name in the command line.

        First, the database is request for indexing the full text index,
        using the database's default options.
        
        Next, it validates the last indexed date time for the database.
        
****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* OS and C include files */

#include <windows.h>
#include <stdio.h>
#include <string.h>

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <nsfdb.h>
#include <ft.h>
#include <misc.h>
#include <osmem.h>
#include <osmisc.h>
#include <ostime.h>

#if defined(CAPI_TESTING) 
#include "printlog.h" 
#else
 #define PRINTLOG printf 
 #define PRINTERROR(api_error, api_name) {\
 char    szErrorText[256] = { 0 };\
 OSLoadString(NULLHANDLE, ERR(api_error), szErrorText, sizeof(szErrorText));\
 fprintf(stderr, "[ERROR]:%s:%d:%s - %s", __FILE__,__LINE__,api_name,szErrorText); }
#endif 

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

#define STRING_LENGTH  256

void  LNPUBLIC  ProcessArgs (int argc, char *argv[], 
                             char *db_name);
short LNPUBLIC IsRemoteDbIndexed(DBHANDLE, DWORD);


int main(int argc, char *argv[])
{

/* Local data declarations */

   char     DBName[MAXUSERNAME]={0};  /* pathname of database */
   DBHANDLE hDB;           /* database handle */
   STATUS   error=NOERROR;       /* error code from API calls */
   TIMEDATE retTime={0};       /* date that the database was last indexed */
   char szTD[MAXALPHATIMEDATE + 1] = {0};
   WORD wRetLen;


   if (error = NotesInitExtended (argc, argv))
   {
      PRINTLOG("\n Unable to initialize Notes.\n");
      return (1);
   }

   /* Get the command line parameters that the user entered. */
   ProcessArgs(argc, argv, DBName); 


   /* Open the database. */
   if (error = NSFDbOpen (DBName, &hDB))
      goto exit0;

     /* Full Text Index the remote database */
   if ( (error = FTIndexRequest(hDB)) != NOERROR )
   {      
      PRINTERROR(error, "FTIndexRequest");       
      goto exitCloseDB;
   }
   else 
   {
      PRINTLOG("FTIndexRequest is success on creating the remote server request for Indexing.\n");
   }

   if (!IsRemoteDbIndexed(hDB, 200))
   {
      PRINTLOG("Remote DB is FT indexed\n");
   }

   /* Report the date/time that this database was indexed */
   memset (szTD, '\0',MAXALPHATIMEDATE + 1);
   if (error = FTGetLastIndexTime (hDB, &retTime))
      goto exitCloseDB; 

   if (error = ConvertTIMEDATEToText (NULL, NULL, &retTime, szTD,
                                      MAXALPHATIMEDATE, &wRetLen))
      goto exitCloseDB; 
 

   PRINTLOG ("Time of Last Index:  %s\n\n", szTD);

exitCloseDB:
   NSFDbClose(hDB);

exit0:
   if (error) 
      PRINTERROR (error,"FTIndexreq");
   NotesTerm();
   return(error);
}


/************************************************************************

    FUNCTION:   IsRemoteDbIndexed - To check remote DB is indexed or not

    INPUTS:     hDB - handle to the open DB in server, 
                waitTime - time to wait to get the response 
 
    OUTPUTS:    retVal -  0 on Success
                          -1 on failure

*************************************************************************/

short IsRemoteDbIndexed(DBHANDLE hDB, DWORD waitTime)
{
    short retVal = 0;
    STATUS sErr = NOERROR;
    TIMEDATE m_tdLastTime = { 0 };
    WORD wCounter = 0;
    do
    {
      sErr = FTGetLastIndexTime(hDB, &m_tdLastTime);
      if(ERR(sErr) == NOERROR)
      {
         PRINTLOG("\nDatabase is full text indexed."); 
         retVal = -1;
      }

      if(wCounter > waitTime)
      {
          break;
      }

      PRINTLOG("\nWaiting for database to be full text indexed...");
      Sleep(1000L);
      wCounter++;
    } while (ERR(sErr) != NOERROR);

    if (ERR(sErr) != NOERROR)
    {
       PRINTERROR(sErr, "FTGetLastIndexTime");
       retVal=-1;
    }
    return retVal;
}


/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:     argc, argv - directly from the command line
 
    OUTPUTS:    db_name -     database name obtained from command line or
                              from prompt.

*************************************************************************/

void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                               char *db_name)
{
   if (argc != 2)
   {
      printf("Enter database name: "); 
      fflush (stdout);
      gets(db_name);
      printf("\n");

   }
   else
   {
       strcpy(db_name, argv[1]);
   } /* end if */
} /* ProcessArgs */
