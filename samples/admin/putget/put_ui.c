/*
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
* DESCRIPTION

This is the front end for PUTNOTE.

*/


/* C include files */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Notes API include files */

#include <global.h>

/* Local include files */

#include "constant.h"
#include "putget.h"

/* Main program */

int main (int argc, char *argv[])
{

/* Local data declarations */

    char		server[MAX_PARAM];
    char		database[MAX_PARAM];
    char		text_path[MAX_PARAM];
    char		log_path[MAX_PARAM];
    char		record_separator;
    char		field_separator;
    char		list_separator;
    char		control_string[CONTROL_STRING_LEN];
    int			error;
    int			i;


/* Set default values for the parameters. */

    strcpy (server, "");
    strcpy (database, DEFAULT_PUTNOTE_DATABASE);
    strcpy (text_path, DEFAULT_PUTNOTE_TEXTFILE);
    strcpy (log_path, DEFAULT_PUTNOTE_LOG);
    record_separator = DEFAULT_RECORD_SEPARATOR;
    field_separator = DEFAULT_FIELD_SEPARATOR;
    list_separator = DEFAULT_LIST_SEPARATOR;

/* Get each parameter from the command line. */

    for (i=1; i < argc; i++)

    {
        if (strncmp (argv[i], "/S=", 3) == 0)
        {
            strcpy (server, argv[i]+3);
            continue;
        }

        if (strncmp (argv[i], "/D=", 3) == 0)
        {
            strcpy (database, argv[i]+3);
            continue;
        }

        if (strncmp (argv[i], "/T=", 3) == 0)
        {
            strcpy (text_path, argv[i]+3);
            continue;
        }

        if (strncmp (argv[i], "/L=", 3) == 0)
        {
            strcpy (log_path, argv[i]+3);
            continue;
        }

        if (strncmp (argv[i], "/rs=", 4) == 0)
        {
            record_separator = (char) atoi(argv[i]+4);
            continue;
        }

        if (strncmp (argv[i], "/fs=", 4) == 0)
        {
            field_separator = (char) atoi(argv[i]+4);
            continue;
        }

        if (strncmp (argv[i], "/ls=", 4) == 0)
        {
            list_separator = (char) atoi (argv[i]+4);
            continue;
        }

        /* If argument is not recognized, just skip it */

    }

/* Construct the control string that we will pass to PUTNOTE. */

    control_string[RECORD_SEPARATOR_CONTROL] = record_separator;
    control_string[FIELD_SEPARATOR_CONTROL] = field_separator;
    control_string[LIST_SEPARATOR_CONTROL] = list_separator;

/* Call PUTNOTE. */

    error = putnote (argc,
                     argv,
                     server,
                     database,
                     text_path,
                     log_path,
                     control_string);

    if (error) exit (EXIT_FAILURE);

/* End of program. */

    exit (EXIT_SUCCESS);
}
