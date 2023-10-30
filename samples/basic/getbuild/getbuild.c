/************************************************************************
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
 
   PROGRAM:    getbuild

   FILE:       getbuild.c

   PURPOSE:    Get the Domino and Notes major build number and print it 
               to the screen.

   SYNTAX:     getbuild
     
   COMMENTS:
       This program gets the Domino and Notes major build number using the 
       HCL C API for Notes/Domino function NSFDbGetBuildVersion(). 
       This opens the specified database, calls NSFDbGetBuildVersion, 
       and prints the information on the screen. 

**************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* OS and C include files */

#include <stdio.h>
#include <string.h>

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <nsfdb.h>
#include <nsfdata.h>
#include <osmisc.h>
#include "../../printLog.h"

/* Local function prototypes */ 
void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                         char *db_filename); 
 
                         
#define  STRING_LENGTH  256

/* HCL C API for Notes/Domino subroutine */

int main(int argc, char *argv[])
{
    
    char		*db_filename;               /* pathname of source database */
    DBHANDLE		db_handle;              /* database handle */
    WORD		wbuild;
    STATUS		error = NOERROR;            /* error code from C API for Domino and Notes calls */
    char		database_name[STRING_LENGTH];
    db_filename = database_name;
    ProcessArgs(argc, argv, db_filename);

    if (error = NotesInitExtended (argc, argv))
    {
        PRINTLOG("\n Unable to initialize Notes.\n");
        return (1);
    }
    
    /* Open the database. */
    
    error = NSFDbOpen (db_filename, &db_handle);
    if (NOERROR != error)
    {
        PRINTLOG("Error: unable to open database '%s'.\n", db_filename);
        PRINTERROR (error,"NSFDbOpen");  
        NotesTerm();
        return (1); 
    }
    
    
    /* Get the major build number. */
    
    error = NSFDbGetBuildVersion (db_handle, &wbuild);
    if (NOERROR != error)
    {
        NSFDbClose (db_handle);
        PRINTERROR (error,"NSFDbGetBuildVersion");  
        NotesTerm();
        return (1); 
    }
    
    PRINTLOG ("\nThe major build number is: %d\n", wbuild);
    
    /* Close the database. */
    
    error = NSFDbClose (db_handle);
    if (NOERROR != error)
    {
        PRINTERROR (error,"NSFDbClose");  
        NotesTerm();
        return (1); 
    }
    
    /* End of subroutine. */

    NotesTerm();
    return (0); 
}

/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:     argc, argv - directly from the command line
   
    OUTPUTS:    db_filename get data from the command line or from what 
                the user types at a prompt 

 *************************************************************************/

void  LNPUBLIC  ProcessArgs (int argc, char *argv[], char *db_filename)
{ 
    if (argc != 2)  
    {

        printf("Enter name of database: ");      
        fflush(stdout);
        gets(db_filename);
        
    }    
    else
    {
        strcpy(db_filename, argv[1]);    
    } /* end if */
} /* ProcessArgs */
