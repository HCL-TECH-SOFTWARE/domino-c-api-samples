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

    PROGRAM:    dbprops

    FILE:       dbprops.c

    PURPOSE:    Get, Set and Reset some of the database properties available
                to the Notes C API.
					
                When the Get option is chosen, this sample will display the
                current database title, the replication ID, the replication
                flags, the replication priority, the replication cut-off
                interval, the replication cut-off date and the advanced
                database properties settings.	
					
                When the Set or Reset options are chosen, this sample will
                modify the database title, some advanced database properties,
                some replication flags, the replication cut-off interval and
                the replication priority settings.					

                Note**
                This program does not illustrate how to access ALL of the
                database properties available to the C API.  While many of
                the current database properties are accessible to the C API,
                some are not.  Certain database properties may require that
                flag values be set as well as additional fields added to
                specific design notes. Since it would be impossible to
                document exactly how to access all the database properties
                that are available, this sample is only meant to lay the
                ground work in doing so by illustrating how to set some of
                the replication settings, some advanced database properties
                and the database title.  Many of the database properties are
                defined in the ICON_FLAGS section of 'stdnames.h'.

    SYNTAX:     dbprops  <action>

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif



/* C and OS include files */

#include <stdio.h>
#include <string.h>

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <stdnames.h>
#include <nsfdb.h>
#include <nsferr.h>
#include <nsfnote.h>
#include <misc.h>
#include <idtable.h>
#include <osmem.h>
#include <osmisc.h>
#include <ostime.h>
#include <ctype.h>
#include "../../printLog.h"

#define  STRING_LENGTH  256


/************************************************************************

    FUNCTION:   Main

    PURPOSE:    HCL C API for Notes/Domino subroutine

*************************************************************************/

int main(int argc, char *argv[])
{
   char        path_name[] = "test.nsf";          /* pathname of database */
   char        old_title[] = "TEST";              /* original title of database */
   char        new_title[] = "Database Properties Test"; /* new title of database */
   char        current_title[50] = "";            /* current title of database */
   char        db_title[50] = "";                 /* title of database */
   char        db_flags[100] = "";                /* icon note flags */
   char        set_db_flags[100] = "";            /* modified icon note flags */
   char        db_info[NSF_INFO_SIZE];            /* database info buffer */
   char        action[15] = "";                   /* input acttion */
   int         usage = 1;
   int         i = 0;
   int         rset = 0;
   DBHANDLE    db_handle;                         /* handle of input database */
   NOTEHANDLE  hIconNote;                         /* handle to the icon note */
   STATUS      error = NOERROR;                   /* return status from API calls */

   DBREPLICAINFO    dbrepInfo;                    /* replication info for DB */
   WORD             wRepFlags;                    /* replication flags */
   WORD             rset_wRepFlags;               /* replication flags to be set  */
   WORD             wPriority;                    /* replication priority	*/
   WORD             rset_wCutoffInterval;			/* rep cutoff interval to be set */
   char             szTimedate[MAXALPHATIMEDATE+1];
   WORD             wLen;


/* Verify program inputs */
   if (argc == 2)
   {
      usage =0;
      strcpy(action, argv[1]);
      for (i=0; i<15; i++)
      {
           if(action[i] == '\0')
               break;
           action[i] = toupper(action[i]);
      }
      if (!strcmp(action,"SET"))
      {
         usage = 1;
         rset = 1;
         strcpy (db_title, new_title);
         set_db_flags[0] = CHFLAG_NOUNREAD_MARKS;         /*Don't maintain unread marks*/
         set_db_flags[1] = CHFLAG_FORM_BUCKET_OPT;        /*Document table bitmap optimization*/
         set_db_flags[2] = CHFLAG_MAINTAIN_LAST_ACCESSED; /*Maintain last accessed property*/
         set_db_flags[3] = CHFLAG_DELETES_ARE_SOFT;       /*Allow soft deletions*/
         set_db_flags[4] = CHFLAG_DISABLE_RESPONSE_INFO;  /*Don't support specialized response hierarchy*/
         set_db_flags[5] = CHFLAG_LARGE_UNKTABLE;         /*Allow more fields in database*/
         set_db_flags[6] = '\0';
         rset_wRepFlags = REPLFLG_CUTOFF_DELETE | REPLFLG_PRIORITY_HI;
         rset_wCutoffInterval = 50;
      }
      else if (!strcmp(action,"RESET"))
      {
          usage = 1;
          rset = 1;
          strcpy (db_title, old_title);
          db_flags[0] = '\0';
          rset_wRepFlags = REPLFLG_IGNORE_DELETES | REPLFLG_PRIORITY_LOW;
          rset_wCutoffInterval = 90;
      }
      else if (!strcmp(action,"GET"))
      {
          usage = 1;
      }
      else
      {
          usage = 0;
      }
   }
   else
      usage = 0;

   if (!usage)
   {
      PRINTLOG( "\n Usage: dbprops <action> \n");
      PRINTLOG( "\n action: SET   - Set new database title and properties\n");
      PRINTLOG( "         RESET - Reset old database title and propertis\n");
      PRINTLOG( "         GET   - Get current database title and properties\n");

      return (0);
   }

   if (error = NotesInitExtended (argc, argv))
   {
      PRINTLOG("\n Unable to initialize Notes.\n");
      return (1);
   }


/* Open the input database */
   if (error = NSFDbOpen (path_name, &db_handle))
   {
      PRINTERROR (error,"NSFDbOpen");
      NotesTerm();
      return (1);
   }

/* Get the replication info */
   if (error = NSFDbReplicaInfoGet( db_handle, &dbrepInfo ))
   {
      NSFDbClose (db_handle);
      PRINTERROR (error,"NSFDbReplicaInfoGet");
      NotesTerm();
      return (1);
   }

/* We can change the title of the output database by following these steps:

   - Get the info buffer of the database (NSFDbInfoGet);
   - Modify the title information in the buffer (NSFDbInfoModify);
   - Write the modified info buffer into the database (NSFDbInfoSet);
   - If necessary, update the ICON note with the updated database
     information buffer
*/

/* Clear out the database information buffer */
   db_info[0] = '\0';

   if (rset)
   {
      PRINTLOG("\n\n *** MODIFIED database settings ***");
   }
   else
   {
      PRINTLOG("\n\n *** CURRENT database settings ***");
   }

/* Get the output database information buffer */
   if (error = NSFDbInfoGet (db_handle, db_info))
   {
      NSFDbClose (db_handle);
      PRINTERROR (error,"NSFDbInfoGet");
      NotesTerm();
      return (1);
   }

/* Add the database title to the database information buffer */
   if(rset)
   {
      NSFDbInfoModify (db_info, INFOPARSE_TITLE, db_title);
      if (error = NSFDbInfoSet (db_handle, db_info))
      {
          NSFDbClose (db_handle);
          PRINTERROR (error,"NSFDbInfoSet");
          NotesTerm();
          return (1);
      }
      dbrepInfo.Flags = rset_wRepFlags;
      dbrepInfo.CutoffInterval = rset_wCutoffInterval;
      if ( error = NSFDbReplicaInfoSet(db_handle, &dbrepInfo))
      {
          NSFDbClose (db_handle);
          PRINTERROR (error,"NSFDbReplicaInfoSet");
          NotesTerm();
          return (1);
      }
   }

/* Get the current title of the database */
   NSFDbInfoParse (db_info, INFOPARSE_TITLE, current_title, NSF_INFO_SIZE - 1);
   PRINTLOG ("\n\n  The current title for database, \"%s\", is:\n\n   \"%s\"\n", path_name, current_title);

/* Display the current replication settings */
   PRINTLOG("\n\n  The following replication properties are currently set:\n\n");
   PRINTLOG("   Database Replica ID    = %#010lx - %#010lx\n", dbrepInfo.ID.Innards[1], dbrepInfo.ID.Innards[0] );

   wRepFlags = dbrepInfo.Flags & REPLFLG_PRIORITY_INVMASK;
   wPriority = REPL_GET_PRIORITY(dbrepInfo.Flags);

   PRINTLOG("   Database Flags         = " );

   if (wRepFlags & REPLFLG_DISABLE)
       PRINTLOG("DISABLE ");
   if (wRepFlags & REPLFLG_IGNORE_DELETES)
       PRINTLOG("IGNORE_DELETES ");
   if (wRepFlags & REPLFLG_HIDDEN_DESIGN)
       PRINTLOG("HIDDEN_DESIGN ");
   if (wRepFlags & REPLFLG_DO_NOT_CATALOG)
       PRINTLOG("DO_NOT_CATALOG ");
   if (wRepFlags & REPLFLG_CUTOFF_DELETE)
       PRINTLOG("CUTOFF_DELETE ");
   if (wRepFlags & REPLFLG_NEVER_REPLICATE)
       PRINTLOG("NEVER_REPLICATE ");
   if (wRepFlags & REPLFLG_ABSTRACT)
       PRINTLOG("ABSTRACT ");
   if (wRepFlags & REPLFLG_DO_NOT_BROWSE)
       PRINTLOG("DO_NOT_BROWSE ");
   if (wRepFlags & REPLFLG_NO_CHRONOS)
       PRINTLOG("NO_CHRONOS ");

   PRINTLOG("\n   Database Rep Priority  = %s\n",
   wPriority == 0 ? "Low"     : (wPriority == 1 ? "Medium"  :
                                (wPriority == 2 ? "High"    : "Unknown")));

   PRINTLOG("   DB Rep Cutoff Interval = %d Days\n", dbrepInfo.CutoffInterval );

   ConvertTIMEDATEToText( NULL,
                          NULL,
                          &(dbrepInfo.Cutoff),
                          szTimedate,
                          MAXALPHATIMEDATE,
                          &wLen );
   szTimedate[wLen] = '\0';

   PRINTLOG("   DB Rep Cutoff Date     = %s\n\n", szTimedate );


/* In order to change the database title or any other component
   of the database information buffer, you also need to update
   this information in the ICON note after updating it in the
   database information buffer
*/

   if (!NSFNoteOpen(db_handle, NOTE_ID_SPECIAL+NOTE_CLASS_ICON, 0, &hIconNote))
   {

/* Update the FIELD_TITLE ("$TITLE") field if present */
      if (rset)
      {
         if (NSFItemIsPresent (hIconNote, FIELD_TITLE, (WORD) strlen (FIELD_TITLE)) )
         {
             NSFItemSetText(hIconNote, FIELD_TITLE, db_info, MAXWORD);
             NSFNoteUpdate(hIconNote, 0);
         }

/* Set the DESIGN_FLAGS ($Flags) field  */
         if (error = NSFItemSetText ( hIconNote,
                                      DESIGN_FLAGS,
                                      set_db_flags,
                                      MAXWORD))
         {
             PRINTERROR (error,"NSFItemSetText");
             NSFNoteClose (hIconNote);
             NSFDbClose (db_handle);
             NotesTerm();
             return (1);
         }

/* Update the note in the database */
         if (error = NSFNoteUpdate (hIconNote, 0))
         {
             PRINTERROR (error,"NSFNoteUpdate");
             NSFNoteClose (hIconNote);
             NSFDbClose (db_handle);
             NotesTerm();
             return (1);
         }
      }

/* Get the DESIGN_FLAGS ($Flags) values */
      if (NSFItemGetText(hIconNote, DESIGN_FLAGS, db_flags, sizeof (db_flags)))
      {
         PRINTLOG("\n  The following database properties are currently set:\n\n");

         i=0;
         while (db_flags[i] != '\0')
         {
            switch(db_flags[i])
            {
               case CHFLAG_NOUNREAD_MARKS:
               {
                  PRINTLOG("   \"Don't maintain unread marks\"\n");
                  break;
               }
               case CHFLAG_FORM_BUCKET_OPT:
               {
                  PRINTLOG("   \"Document table bitmap optimization\"\n");
                  break;
               }
               case CHFLAG_MAINTAIN_LAST_ACCESSED:
               {
                  PRINTLOG("   \"Maintain last accessed property\"\n");
                  break;
               }
               case CHFLAG_DELETES_ARE_SOFT:
               {
                  PRINTLOG("   \"Allow soft deletions\"\n");
                  break;
               }
               case CHFLAG_DISABLE_RESPONSE_INFO:
               {
                  PRINTLOG("   \"Don't support specialized response hierarchy\"\n");
                  break;
               }
               case CHFLAG_LARGE_UNKTABLE:
               {
                  PRINTLOG("   \"Allow more fields in database\"\n");
                  break;
               }
               default:
               {
                  PRINTLOG("   UNKNOWN database property\n");
                  break;
               }
            }
            i++;
         }
      }
      else
      {
         PRINTLOG("\n  There are no DESIGN_FLAGS currently set in the ICON note.\n");
      }

      NSFNoteClose(hIconNote);
   }
   else
   {
      PRINTLOG("\n  Unable to open ICON note.\n");
   }

/* if there was no ICON note, do nothing */

/* Close the database */
   if (error = NSFDbClose (db_handle))
   {
       PRINTERROR (error,"NSFDbClose");
       NotesTerm();
       return (1);
   }

   fflush(stdout);

/* End of program */
   PRINTLOG("\n\n Program completed successfully.\n");
   NotesTerm();
   return (0);
}
