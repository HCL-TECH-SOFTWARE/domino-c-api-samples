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

    PROGRAM: reqnum

    FILE:    reqnum.c

    SYNTAX:  reqnum

    PURPOSE: create a file "reqnum.dat" containing NUMBER = 1
             for use by HCL C API for Notes/Domino sample program tracker. 

    DESCRIPTION:
   
    HCL C API for Notes/Domino  Sample program tracker requires a data file 
	with name "reqnum.dat" to manage a unique sequential request number
    service. Under Windows, this file resides in "c:\" by default.
    This program creates this file containing the number 1.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

/* C include files */
#include <stdio.h>
#include <errno.h>

/* HCL C API for Notes/Domino include files */
#include <global.h>                     /* NUMBER */
#include <names.h>                      /* MAXUSERNAME */
#include <nsfdb.h>                      /* DBHOOKVEC */
#include <nsfdata.h>                    /* LIST */
#include "../../printLog.h"

/* Application-specific include files */
#include "tracker.h"                    /* REQNUM_FILENAME */

/************************************************************************

    FUNCTION:   main

    PURPOSE:    Create the "next Request Number to Use" file reqnum.dat.
                Set the number to 1.

************************************************************************/

int main (int argc, char *argv[])
{
    FILE       *pReqNumFile;
    NUMBER      ReqNumber;
    size_t      count;

#if defined(OS400)
    pReqNumFile = fopen(REQNUM_FILENAME, "wb ccsid=819");
#else
    pReqNumFile = fopen(REQNUM_FILENAME, "w");
#endif


    if (pReqNumFile == (FILE*)NULL)
    {
        PRINTLOG("Error: unable to open file '%s': %d.\n",
                  REQNUM_FILENAME, errno);
        return 0;
    }

    ReqNumber = 1;
    count = fwrite(&ReqNumber, sizeof(NUMBER), 1, pReqNumFile);
    fclose(pReqNumFile);

    if (count != 1)
    {
        PRINTLOG("Error: error writing file '%s' : %d.\n",
                  REQNUM_FILENAME, errno);
    }

    return 0;
}
