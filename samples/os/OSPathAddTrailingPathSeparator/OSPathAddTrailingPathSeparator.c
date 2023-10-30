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
    PROGRAM:    OSPAthAddTrailingPathSeparator

    FILE:       OSPAthAddTrailingPathSeparator.c

    SYNTAX:     OSPAthAddTrailingPathSeparator

    PURPOSE:    Adds a trailing path separator if the last character in the path is not a separator.

    DESCRIPTION:
                This program adds a trailing path separator if needed (platform specific).


****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif


/* OS and C include files */

#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <osfile.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <nsfsearc.h>
#include <nsferr.h>
#include <osmisc.h>
#include "../../printLog.h"


/************************************************************************

    FUNCTION:   main

    PURPOSE:    Main HCL C API for Notes/Domino subroutine

		This sample creates file path manually and
		compares with file path obtained from OSPathAddTrailingPathSeparator API
		and finally checks physical existance of the path

*************************************************************************/
#if defined(UNIX)
#define NormalPathSepString		"/"
#else
#define NormalPathSepString		"\\"
#endif
#define DEBUG_LOGDIR_DEFAULT "IBM_TECHNICAL_SUPPORT"


int main(int argc, char *argv[])
{

   /* Local data declarations */
   STATUS      error = NOERROR;            /* return status from API calls */
   char szDataDir[MAXPATH] = {0};
   char szFullPath[MAXPATH] = {0};
   char szFullPathManual[MAXPATH] = {0};
   struct stat info = {0};

   PRINTLOG("\n initializing Notes.\n");
     
   if (error = NotesInitExtended (argc, argv))
   {
       PRINTLOG("\n Unable to initialize Notes.\n");
       PRINTERROR(error,"NotesInitExtended");
       return (1);
   }

   /* Reading Domino data directory */
   OSGetDataDirectory(szDataDir);

   /* preparing the path manually*/
   strncat(szFullPathManual, szDataDir, sizeof(szFullPathManual) -1 - strlen(szFullPathManual));
   strncat(szFullPathManual, NormalPathSepString, sizeof(szFullPathManual) - 1 - strlen(szFullPathManual));
   strncat(szFullPathManual, DEBUG_LOGDIR_DEFAULT, sizeof(szFullPathManual) - 1 - strlen(szFullPathManual));
   strncat(szFullPathManual, NormalPathSepString, sizeof(szFullPathManual) - 1 - strlen(szFullPathManual));

   PRINTLOG("Manual path : %s\n", szFullPathManual);
   strncat(szFullPath, szDataDir, sizeof(szFullPath) - 1 - strlen(szFullPath));

    /* Appending trailing seprator */
   if (error = OSPathAddTrailingPathSeparator(szFullPath, sizeof(szFullPath)))
   {
       PRINTERROR(error,"OSPathAddTrailingPathSeparator");
       NotesTerm();
       return(1);
   }
      
   strncat(szFullPath, DEBUG_LOGDIR_DEFAULT, sizeof(szFullPath) - 1 - strlen(szFullPath));

   /* Appending trailing seprator */
   if (error = OSPathAddTrailingPathSeparator(szFullPath, sizeof(szFullPath)))
   {
       PRINTERROR(error,"OSPathAddTrailingPathSeparator");
       NotesTerm();
       return(1);
   }

   PRINTLOG("Full path : %s\n", szFullPath);

   /* verifies Manaully prepared path is same as path obtained by using OSPathAddTrailingPathSeparator API */

   if ((strncmp(szFullPath, szFullPathManual, (sizeof(szFullPath) - 1))) == 0)
   {
       PRINTLOG("\n Paths are equal.\n");
   }
   else
   {
       PRINTLOG("\n Error : Paths are not equal.\n");
   }

   int statRC = stat(szFullPath, &info);

   /* verifying physically the domino log directory exists */
   if ((statRC == 0) && (info.st_mode & S_IFDIR))
   {
       PRINTLOG("\n Domino Log Directory %s Exists.", szFullPath);
   }
   else
   {
       PRINTLOG("\n Domino Log Directory %s Doesn't Exists.", szFullPath);
   }
 
   PRINTLOG("\n\n Program completed successfully.\n");
   
   /* End of main routine. */
   NotesTerm();
   return (0); 

}
