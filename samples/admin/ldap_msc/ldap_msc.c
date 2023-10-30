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

   PROGRAM:   ldap_msc

   FILE:      ldap_msc.c

   PURPOSE:   This program is intended to illustrate the practical usage
              of the Lightweight Directory Assistance Protocol (LDAP) as
	           implemented via	a C API program.
				
              Using the LDAP functionality of the C API Toolkit for
              Notes/Domino, this program will:
				
              - Add an entry to an LDAP directory.
              - Modify the Relative Distinguished Name (RDN) of the new
                entry (deleting an identical entry if one exists.)
              - Perform a search of the directory based on a filter.
              - Perform a comparison of attribute values.
				
              NOTE**
              A THOROUGH understanding of the open industry standard LDAP as
              well as experience with result messages returned from LDAP is
              required prior to successfully running this sample.  Please
              refer to the Domino Administration Guide for information
              regarding the configuration and implementation of accessing an
              LDAP directory with Notes Users and Domino servers.				
				 		
              This sample is for illustrative purposes only.
				
   SYNTAX:    ldap_msc

*************************************************************************/

#include "global.h"
#include "misc.h"
#include "osmisc.h"
#include "ldap.h"
#include "ldap_msc.h"
#include "../../printLog.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define	NMODS	5

unsigned long	global_counter = 0;

static void free_mods( LDAPMod **mods );

void LNPUBLIC ProcessArgs (int argc,char *argv[],char *HOST,char *SEARCHBASE,char *DN,char *PASSWORD);

#define  STRING_LENGTH  256

/* Program declaration */
int main(int argc, char *argv[])
{
    LDAP          *ld;
    char          *dn, *ndn, *nrdn;
    int           i;
    int           rc;
    LDAPMod       **mods;

    char          *a, *sdn;
    char          **vals;
    int           j;
    LDAPMessage   *result, *e;
    BerElement    *ber;

    char          *compAttribute, *compValue;
    struct        berval bvalue;

    /* Values used in creating the entry */
    char *objectclass_values[] = { "top", "person", "organizationalPerson",
                                   "inetOrgPerson", "dominoPerson", NULL };
    char *cn_values[] = { "John Public", NULL };
    char *sn_values[] = { "Public", NULL };
    char *givenname_values[] = { "John", NULL };
    char *telephonenumber_values[] = { "+1 888 555 1212", NULL };

    /* Option to have DEBUG information written to screen.
	
       NOTE: "DEBUG_OUTFILE=" and "LDAPDEBUG=1" parameters must also be
       defined in the notes.ini to enable this option.
	
       Uncomment the following lines for DEBUG information to display */
    /*
    WORD		ldap_debug_level = 0;

    ldap_debug_level = LDAP_DEBUG_API;
    ldap_set_option (NULL, LDAP_OPT_DEBUG_LEVEL, &ldap_debug_level);
    */

    char HOST[STRING_LENGTH];
    char SEARCHBASE[STRING_LENGTH];
    char DN[STRING_LENGTH];
    char PASSWORD[STRING_LENGTH];

    ProcessArgs (argc, argv, HOST, SEARCHBASE, DN, PASSWORD);

    /* Specify the DN we're adding */
    dn = MOD_DN;
    /* the destination DN */
    ndn = MOD_NDN;
    /* the new RDN */
    nrdn = MOD_NRDN;


    if (NotesInitExtended (argc, argv))
    {
        fprintf (stderr, "\nError initializing Notes.\n");
        return (1);
    }

    /* get a handle to an LDAP connection */
    if ( (ld = ldap_init( HOST, PORT )) == NULL )
    {
        perror( "ldap_init" );
        NotesTerm();
        return( 1 );
    }
    /* authenticate to the directory as administrator */
    if ( ldap_simple_bind_s( ld, DN, PASSWORD ) != LDAP_SUCCESS )
    {
        ldap_perror( ld, "ldap_simple_bind_s" );
        NotesTerm();
        return( 1 );
    }

    if (( mods = ( LDAPMod ** ) malloc(( NMODS + 1 ) * sizeof( LDAPMod *)))
	    == NULL )
    {
        fprintf( stderr, "Cannot allocate memory for mods array\n" );
        NotesTerm();
        return( 1 );
    }
    /* Construct the array of values to add */
    for ( i = 0; i < NMODS; i++ )
    {
        if (( mods[ i ] = ( LDAPMod * ) malloc( sizeof( LDAPMod ))) == NULL )
        {
            fprintf( stderr, "Cannot allocate memory for mods element\n" );
            NotesTerm();
            return( 1 );
        }
    }
    mods[ 0 ]->mod_op = 0;
    mods[ 0 ]->mod_type = "objectclass";
    mods[ 0 ]->mod_values = objectclass_values;
    mods[ 1 ]->mod_op = 0;
    mods[ 1 ]->mod_type = "cn";
    mods[ 1 ]->mod_values = cn_values;
    mods[ 2 ]->mod_op = 0;
    mods[ 2 ]->mod_type = "sn";
    mods[ 2 ]->mod_values = sn_values;
    mods[ 3 ]->mod_op = 0;
    mods[ 3 ]->mod_type = "givenname";
    mods[ 3 ]->mod_values = givenname_values;
    mods[ 4 ]->mod_op = 0;
    mods[ 4 ]->mod_type = "telephonenumber";
    mods[ 4 ]->mod_values = telephonenumber_values;
    mods[ 5 ] = NULL;

	
    PRINTLOG("\n  Modifcation results:\n\n");

    /* Add the entry */
    if (( rc = ldap_add_s( ld, dn, mods )) != LDAP_SUCCESS )
    {
        /* If entry exists already, fine.  Ignore this error. */
        if ( rc == LDAP_ALREADY_EXISTS )
        {
            printf( "Entry \"%s is already in the directory.\n", dn );
        }
        else
        {
            ldap_perror( ld, "ldap_add_s" );
            free_mods( mods );
            NotesTerm();
            return( 1 );
        }
    }
    else
    {
         PRINTLOG( "\tAdded entry \"%s\".\n", dn );
    }
    free_mods( mods );

    /* Delete the destination entry, for this example */
    if (( rc = ldap_delete_s( ld, ndn )) != LDAP_SUCCESS )
    {
        /* If entry does not exist, fine.  Ignore this error. */
        if ( rc == LDAP_NO_SUCH_OBJECT )
        {
            PRINTLOG( "\tEntry \"%s\" is not in the directory.  "
             "\n\t  No need to delete.\n", ndn );
        }
        else
        {
            ldap_perror( ld, "ldap_delete_s" );
            NotesTerm();
            return( 1 );
        }
    }
    else
    {
        PRINTLOG( "\tDeleted entry \"%s\".\n", ndn );
    }

    /* Do the RDN modification operation */
    if ( ldap_modrdn2_s( ld, dn, nrdn, 0 ) != LDAP_SUCCESS )
    {
        ldap_perror( ld, "ldap_modrdn2_s" );
        NotesTerm();
        return( 1 );
    }

    PRINTLOG( "\tThe RDN modification operation was successful.\n \tEntry...\n"
              "\t  \"%s\" \n\tHas been changed to...\n"
              "\t  \"%s\" \n", dn, ndn );


    /* Perform search */
    PRINTLOG("\n  Search results:\n\n");

    if ( ldap_search_s( ld, SEARCHBASE, LDAP_SCOPE_SUBTREE,
                        FILTER, NULL, 0, &result ) != LDAP_SUCCESS )
    {
        ldap_perror( ld, "ldap_search_s" );
        if ( result == NULL )
        {
            ldap_unbind_s( ld );
            NotesTerm();
            return( 1 );
        }
    }

    /* for each entry print out name, attrs and values */
    for ( e = ldap_first_entry( ld, result ); e != NULL;
          e = ldap_next_entry( ld, e ) )
    {
        if ( (sdn = ldap_get_dn( ld, e )) != NULL )
        {
            PRINTLOG( "\tdn: %s\n", sdn );
            ldap_memfree( sdn );
        }
        for ( a = ldap_first_attribute( ld, e, &ber );
              a != NULL; a = ldap_next_attribute( ld, e, ber ) )
        {
            if ((vals = ldap_get_values( ld, e, a)) != NULL )
            {
                for ( j = 0; vals[j] != NULL; j++ )
                {
                    PRINTLOG( "\t%s: %s\n", a, vals[j] );
                }
                ldap_value_free( vals );
             }
             ldap_memfree(a);
         }
         ber_free(ber, 0);
    }
    ldap_msgfree( result );

    /* compare the value "person" against the objectclass attribute */
    PRINTLOG("\n  Comparison results:\n\n");

    compAttribute = "objectclass";
    compValue = "person";
    bvalue.bv_val = compValue;
    bvalue.bv_len = strlen(compValue);

    rc = ldap_compare_ext_s( ld, DN, compAttribute, &bvalue, NULL, NULL );
    switch ( rc )
    {
        case LDAP_COMPARE_TRUE:
            PRINTLOG( "\tThe value \"person\" is contained in the objectclass "
                     "attribute.\n" );
            break;
        case LDAP_COMPARE_FALSE:
            PRINTLOG( "\tThe value \"person\" is not contained in the objectclass "
                     "attribute.\n" );
            break;
        default:
            ldap_perror( ld, "ldap_compare_s" );
    }

    /* compare the value "xyzzy" against the objectclass attribute */
    compValue = "xyzzy";
    bvalue.bv_val = compValue;
    bvalue.bv_len = strlen(compValue);

    rc = ldap_compare_ext_s( ld, DN, compAttribute, &bvalue, NULL, NULL );
    switch ( rc )
    {
        case LDAP_COMPARE_TRUE:				
            PRINTLOG( "\tThe value \"xyzzy\" is contained in the objectclass "
                      "attribute.\n" );
            break;
        case LDAP_COMPARE_FALSE:
            PRINTLOG( "\tThe value \"xyzzy\" is not contained in the objectclass "
                      "attribute.\n" );
            break;
        default:
            ldap_perror( ld, "ldap_compare_s" );
    }

    ldap_unbind_s( ld );
    NotesTerm();
    PRINTLOG ("\nProgram completed successfully.\n");
    return 0;
}



/*
 * Free a mods array.
 */
static void
free_mods( LDAPMod **mods )
{
    int i;

    for ( i = 0; i < NMODS; i++ )
    {
        free( mods[ i ] );
    }
    free( mods );
}


void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                             char *HOST, 
                             char *SEARCHBASE, 
                             char *DN,
                             char *PASSWORD)
{ 
    if (argc != 5)
    {       
      
        printf("Enter Hostname: ");   
        fflush(stdout);
        fgets(HOST, STRING_LENGTH, stdin);

        printf("\n");
        printf("Enter Organisation: ");
        fflush(stdout);
        fgets(SEARCHBASE, STRING_LENGTH, stdin);

        printf("\n");
        printf("Enter DN: ");      
        fflush(stdout);
        fgets(DN, STRING_LENGTH, stdin);

        printf("\n");
        printf("Enter PASSWORD: ");      
        fflush(stdout);
        fgets(PASSWORD, STRING_LENGTH, stdin);

    }  
    else
    {
        memset(HOST,'\0',STRING_LENGTH);
        strncpy(HOST, argv[1], STRING_LENGTH);
        memset(SEARCHBASE,'\0',STRING_LENGTH);
        strncpy(SEARCHBASE, argv[2], STRING_LENGTH);
        memset(DN,'\0',STRING_LENGTH);
        strncpy(DN, argv[3], STRING_LENGTH);
        memset(PASSWORD,'\0',STRING_LENGTH);
        strncpy(PASSWORD, argv[4], STRING_LENGTH);
    }
}
