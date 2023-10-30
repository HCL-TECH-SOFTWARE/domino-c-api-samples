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

   PROGRAM:    dirsearch

   FILE:       dirsearch.c

   PURPOSE:    Demonstrates the Directory Independence API  

   SYNTAX:     dirsearch -s servername -d domain -n username
 
   Description:
				This program is intended to illustrate the practical usage of the 
				Directory API implemented via a C API program.

				Using the Directory API functionality in the C API Toolkit for
				Notes/Domino, this program will:

				- Perform a simple search for a directory entry based on its Notes distinguished
				name, abbreviated name, common name, first name, last name, short name, group name,
				server name, or internet address, using the Directory API

				- Print search results
*************************************************************************/

/* OS and C include files */

#if defined(OS400)
#pragma convert(850)
#endif


#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* HCL C API for Notes/Domino include files */ 
#include "global.h"
#include "nsfdb.h"
#include "nsfdata.h"
#include "osmisc.h"
#include "../../printLog.h"

/* Directory Independence API files */
#include "dirctx.h"
#include "direntry.h"
#include "direntryid.h"
#include "dircoll.h"
#include "dirprint.h"

/* Local constants and data */
#define BADCH   (int)'?'
#define EMSG    ""

/*	Global data */
int     opterr = 1;             /* if error message should be printed */
int     optind = 1;             /* index into parent argv vector */
int     optopt = 1;             /* character checked for validity */

typedef struct {
                char szServerName [MAXUSERNAME+1];
                char szDomainName [MAXDOMAINNAME+1];
                char szName [MAXSPRINTF+1];
                char szItems [MAXSPRINTF+1];
                DWORD dwFlags;
                WORD wNumItems;
                BYTE formtype;
                DWORD grouptype;
                int PrintFlags;
} ARG_STRUCT;

/* Local function prototypes */
int processArgs (int nargc, const char * const *nargv, ARG_STRUCT *args);
int getOpt (int nargc, const char * const *nargv, const char *ostr, char** poptarg);

static void near PASCAL help()
{
	PRINTLOG("Find a directory entry and print requested information about it.\n\n");
	PRINTLOG(" -d			Domain Name\n");
	PRINTLOG(" -s			Server Name\n");
	PRINTLOG(" -f			flags which control the behavior, for more information please see DirCtxSetFlags in the documentation\n");
	PRINTLOG(" -n			Name of the entry to find (will match on Notes distinguished name,\n");
	PRINTLOG("            abbreviated name, common name, first name, last name, short name, \n");
	PRINTLOG("            group name, server name, or internet address).\n");
	PRINTLOG(" -i			Item{,Item} one or more items to return (omit for $$DIR_ITEMS_ALL_DOMINO) Add a plus sign '+' to display operational attributes\n");
	PRINTLOG(" -t			Ojbect Type - one of:\n\t\t\t\t"
						"anyperson\n\t\t\t\t"
						"dominoperson\n\t\t\t\t"
						"anygroup\n\t\t\t\t"
						"dominogroup\n");
	PRINTLOG(" -g			Group Type - one of:\n\t\t\t\t"
						"0 = multi-purpose groups only\n\t\t\t\t"
						"1 = mail-only groups\n\t\t\t\t"
						"2 = acl-only groups\n\t\t\t\t"
						"3 = deny-list only groups\n\t\t\t\t"
						"4 = servers-only groups\n\n");
}

void LNCALLBACK printLn(void *ctx, const char *line)
{
    FILE *f = (FILE *)ctx;
    fprintf(f, "%s\n", line);
}

/* Program declaration */
int main(int argc, char *argv[])
{
	STATUS			error = NOERROR;	
	DIRCTX			hCtx = NULLDIRCTX;
	DIRCOLLECTION	hCollection = NULLDIRCOLLECTION;
	ARG_STRUCT		args = {0};

	memset (&args, 0, sizeof (args));
	error = NotesInitExtended (argc, argv);
	
	if ( error)
	{
	   fprintf (stderr, "\nError initializing Notes.\n");
	   NotesTerm();
	   return (1);
	}

	if (processArgs (argc, (const char * const *)argv, &args))
	{
	   NotesTerm();
	   return 1;
	}
	/* Create the specified directory context for directory operations */
	if (( error = DirCtxAlloc2(args.szServerName, args.szDomainName, &hCtx)) != NOERROR)
	{
	   fprintf(stderr, "DirCtxAlloc failure - %d:", error);
	   PRINTERROR(error,"DirCtxAlloc2");
	   goto Done;
	}

	/* Set the input flags that control behavior. */
	if (( error = DirCtxSetFlags( hCtx, args.dwFlags)) != NOERROR )
	goto Done;

	/* Search for the Entry via name */
	if ( args.formtype == (0x01 | 0x02) )		/* any type of person */
	    error = DirCtxSearchPersonsByName(hCtx, args.szName, args.szItems, args.wNumItems, &hCollection);
	else if ( args.formtype == 0x02 )			/* domino people only */
	    error = DirCtxSearchDominoPersonsByName(hCtx, args.szName, args.szItems, args.wNumItems, &hCollection);
	else if ( args.formtype == (0x10 | 0x20) )	/* any type of group */
	    error = DirCtxSearchGroupsByName(hCtx, args.szName, args.szItems, args.wNumItems, &hCollection);
	else if ( args.formtype == 0x20 )			/* domino groups only */
	    error = DirCtxSearchDominoGroupsByName(hCtx, args.szName, args.szItems, args.wNumItems, args.grouptype, &hCollection);
	else										/* any type of doc from the $Users view */
	    error = DirCtxSearchByName(hCtx, args.szName, args.szItems, args.wNumItems, &hCollection);

	if ( error != NOERROR )
	{
	   fprintf(stderr, "SearchByName failure - %d:", error);
	   PRINTERROR(error,"DirCtxSetFlags");
	   goto Done;
	}	

	/* Print search summary */
	if (DirCtxGetDomain(hCtx, args.szDomainName) == NOERROR &&
	    DirCtxGetDirectoryServer(hCtx, args.szServerName) == NOERROR)
	{
	   PRINTLOG("Found '%lu' matches for '%s' in domain '%s' through server '%s'.\n\n",
	             DirCollectionGetNumEntries(hCollection), args.szName, args.szDomainName, args.szServerName);									 
	}

	/* Print complete results */
	DirCollectionPrint(hCollection, args.PrintFlags, printLn, stdout);
	

Done:	
	/* Free the directory collection of entries and context */
	DirCollectionFree(hCollection);
	DirCtxFree(hCtx);
		    
	if( error != NOERROR )
	{
	   fprintf(stderr, "SearchByName failure - %d:", error);
	   PRINTERROR(error,"DirCtxSetFlags");

	   NotesTerm();
	   return (1);
	}
	NotesTerm();
	return (0);
}

int processArgs (int nargc, const char * const *nargv, ARG_STRUCT *args)
{
	int nswitch = 0;
	int c = 0;
	char *optarg = NULL;

	/*  Initialize optional args */
	strcpy(args->szItems, DIR_ITEMS_ALL_DOMINO);
	args->wNumItems = 1;

	while (( c = getOpt( nargc, nargv, "t:g:s:d:f:i:n:b", &optarg )) != -1 )
	{
		switch ( c ) 
		{
		    case 't':
		        if (strcmp("anyperson", optarg) == 0) { args->formtype = (0x01 | 0x02); }
		        else if (strcmp("dominoperson", optarg) == 0) { args->formtype = 0x02; }
		        else if (strcmp("anygroup", optarg) == 0) { args->formtype = (0x10 | 0x20); }
		        else if (strcmp("dominogroup", optarg) == 0) { args->formtype = 0x20; }
		        break;
		    case 'g':
		        args->grouptype = (DWORD)atol(optarg);
		        break;
		    case 's':
		        strncpy (args->szServerName, optarg, strlen(optarg));
		        break;
		    case 'd':
		        strncpy (args->szDomainName, optarg, strlen(optarg));
		        break;
		    case 'f':
		        args->dwFlags = atoi( optarg);
		        break;
		    case 'i':
		        args->szItems[0] = '\0';
		        args->wNumItems = 0;
		        {
		            int avail_len = sizeof(args->szItems) - 1;
		            char *ptr = args->szItems;
		            char *token = strtok(optarg, ",");
		            while (token != NULL)
		            {
		                const int len = strlen(token);
		                if (avail_len < len)
		                {
		                    PRINTLOG("Sorry, too many -i Items.\n");
		                    return 1;
		                }

		                if (*token == '+')
		                    args->PrintFlags |= DIRPRINT_FLAG_EXTENDED_ENTRY;
		                else
		                {
		                    strncpy(ptr, token, avail_len);
		                    args->wNumItems++;
		                    avail_len -= (len + 1);
		                    ptr += (len + 1);
		                }

						
		                token = strtok(NULL, ",");
		            }

		            if ((args->PrintFlags & DIRPRINT_FLAG_EXTENDED_ENTRY) && args->wNumItems == 0)
		            {
		                args->szItems[0] = '\0';
		                strcpy(args->szItems, DIR_ITEMS_ALL_DOMINO);
		                args->wNumItems = 1;
		            }

		        }
		        break;
		    case 'b':
		        break;
		    case 'n':
		        nswitch = 1;
		        strncpy (args->szName, optarg, strlen(optarg));

		        break;
		    default:
		        help();
		        return 1;
		}

	}

	if (nswitch == 0)
	{
	   help();
	   return 1;
	}
		
	return 0;
}

int getOpt (int nargc, const char * const *nargv, const char *ostr, char** poptarg)
{
	static const char *place = EMSG;      /* option letter processing */
	char *oli;                     /* option letter list index */
	const char *p = NULL;

	if (!*place) {                          /* update scanning pointer */
	    if (optind >= nargc || *(place = nargv[optind]) != '-') {
	        place = EMSG;
	        return(EOF);
	    }
	    if (place[1] && *++place == '-') {      /* found "--" */
	        ++optind;
	        place = EMSG;
	        return(EOF);
	    }
	}                                       /* option letter okay? */
	if ((optopt = (int)*place++) == (int)':' ||
	    (oli = strchr((char *)ostr, optopt)) == 0) {
	    /*
	     * if the user didn't specify '-' as an option,
	     * assume it means EOF.
	     */
	    if (optopt == (int)'-')
	        return(EOF);
	    if (!*place)
	        ++optind;
	    if (opterr) {
	        if ((p = strrchr(*nargv, '/')) != NULL)
	            p = *nargv;
	        if (p != NULL)
	            PRINTLOG("%s: illegal option -- %c\n", p, optopt);
	    }
	    return(BADCH);
	}
	if (*++oli != ':') {                    /* don't need argument */
	    *poptarg = NULL;
	    if (!*place)
	        ++optind;
	}
	else {                                  /* need an argument */
	    if (*place)                     /* no white space */
	        *poptarg = (char*)place;
	    else if (nargc <= ++optind) {   /* no arg */
	        place = EMSG;
	        if ((p = strrchr(*nargv, '/')) != NULL)
	            p = *nargv;
	        if (opterr && p != NULL)
	            PRINTLOG("%s: option requires an argument -- %c\n", p, optopt);
	        return(BADCH);
	    }
	    else                            /* white space */
	        *poptarg = (char*)nargv[optind];
	    place = EMSG;
	    ++optind;
	}
	return(optopt);                         /* dump back option letter */
}

