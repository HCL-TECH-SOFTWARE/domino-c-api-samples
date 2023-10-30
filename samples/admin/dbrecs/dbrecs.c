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

 PROGRAM:   dbrecs

 FILE:      dbrecs.c

 PURPOSE:   This program is intended to illustrate the practical usage of the
            Media Recovery and Archiving functions of the Backup and Recovery
            API.  It is not intended to show exclusive use of these functions
            or the most effective program logic when using these functions.

            ** IMPORTANT NOTE ** Great care should be taken to maintain
            "recoverable" backup files (i.e. once a backup file has been
            recovered it is no longer a "recoverable" file).  If a problem
            occurs after a database has been taken offline and deleted, an
            existing recoverable backup file that was created from that
            original DB will be the ONLY means available to restore the
            original DB.
		
            This sample is for illustrative purposes only.  Since the logic
            of this type of functionality is highly dependent on its specific
            intention, this sample is not recommended for use with production
            databases.


 SYNTAX:    dbrecs <option> [input file] [restore db]

    option: ARCHIVE - Archive system logs.   (Input file NOT REQUIRED)
            CHECK   - Check a database or backup file.  (Input file REQUIRED)
            RECOVER - Recover a backup file. (Input file REQUIRED)
            RESTORE - Restore a database.  (Input file & restore db REQUIRED)

        input file: - Path to a backup file. (If REQUIRED)
        restore db: - Path to the database to be restored. (If REQUIRED)

****************************************************************************/

/* OS and C include files */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

/* Notes API include files */
#include "global.h"
#include "osfile.h"
#include "osmem.h"
#include "osmisc.h"
#include "ostime.h"
#include "nsfdb.h"
#include "nsferr.h"
#include "oserr.h"
#include "misc.h"
#include "win32io.h"
#include "../../printLog.h"

/* define exit codes if they are not defined elsewhere */
#ifndef EXIT_SUCCESS
   #define EXIT_SUCCESS 0
#endif

#ifndef EXIT_FAILURE
   #define EXIT_FAILURE 1
#endif

#define MAXPATHLENGTH 1024

/* local callback and functions */
STATUS LNPUBLIC MyCallback(UNID * Logid, DWORD Lognumber, char * path);
STATUS LNPUBLIC NoteCallback(DWORD state_flags, void far * userParm, NOTE_RESTORE_CALLBACK_INFO far * pinfo);

static STATUS DoArchiveLogs(void);
static STATUS TakeDbsOffline(char * DbPath);
static STATUS RestoreDbs(char * BUPath, char * DbPath);
static STATUS RecoverDbs(char * BUPath, DWORD Rflags, int RNoteInfo);
static STATUS BringDbsOnline(char * DbPath);
static void EventLog(int LogFD, char * outstring);
static STATUS CheckDb(char * BUPath);

static char DirSlash[] = "\\";
static char LogDir[MAXPATHLENGTH] = "DBLogs";

int LogFD;  /* FD for logging events */

/* Program declaration */
VOID main (int argc, char *argv[])
{
   /* Local data declarations */
   STATUS   err = NOERROR;
   char     RecOpt[15]; /* input option */
   char     InFile[MAXPATHLENGTH]; /* input file */
   char     ResDb[MAXPATHLENGTH]; /* restore database */
   char     LogFile[MAXPATHLENGTH];
   int      i = 0;
   DWORD    flags = 0; /* flags for NSFRecoverDatabases */
   int      usage = 1;
   int      NoteInfo = 0;


   /* Verify program inputs */
   if (argc >= 5)
   {
      usage =0;
      strcpy(RecOpt, argv[2]);
      for (i=0; i<15; i++)
      {
         if(RecOpt[i] == '\0')
            break;
         RecOpt[i] = toupper(RecOpt[i]);
      }
      if (!strcmp(RecOpt,"ARCHIVE"))
         usage =1;
      if (argc >= 4)
      {
         strcpy(InFile, argv[3]);
         if (!strcmp(RecOpt,"RECOVER") || !strcmp(RecOpt,"NOTE"))
         {
            usage=1;													
            flags = 1;
            if (!strcmp(RecOpt,"NOTE"))
                NoteInfo = 1;
         }
         else if (!strcmp(RecOpt,"CHECK"))
         {
            usage=1;													
         }
         /* Ensure null termination */
         InFile[strlen(&InFile[0])+1] = '\0';

      }

      if (argc == 5)
      {
         strcpy(ResDb, argv[4]);
         if (!strcmp(RecOpt, "RESTORE"))
            usage=1;
      }
   }
   else
      usage = 0;

   if (!usage)
   {
      PRINTLOG( "\nUsage: dbrecs <Domino ini path> <option> [input file] [restore db]\n");
      PRINTLOG( "\noption: ARCHIVE - Archive system logs.   (Input file NOT REQUIRED)\n");
      PRINTLOG( "        CHECK   - Check a database or backup file.    (Input file REQUIRED)\n");
      PRINTLOG( "        RECOVER - Recover a backup file. (Input file REQUIRED)\n");
      PRINTLOG( "        NOTE    - Recover a backup file and get Note info. (Input file REQUIRED)\n");
      PRINTLOG( "        RESTORE - Restore a database.    (Input file & restore db REQUIRED)\n");
      PRINTLOG( "\ninput file: - Path to a backup file.   (If REQUIRED)\n");
      PRINTLOG( "\nrestore db: - Path to the database to be restored.   (If REQUIRED)\n\n");

      exit (EXIT_SUCCESS);
   }

   /* Initialize Notes */
   if(err = NotesInitExtended (argc, argv))
   {
      PRINTLOG ("\nError initializing Notes.\n");
      exit (EXIT_FAILURE);
   }

   /* Create the log file directory if it doesn't already exist*/
   sprintf(&LogFile[0], "%s%s",   LogDir, DirSlash);
   if(err = SysFileCreateDirectory(&LogFile[0]))
   {
      PRINTLOG("\nError %d creating directory %s\n", err, LogFile);
      NotesTerm();
      exit (EXIT_FAILURE);
   }

   /* Create the log file - do not overwrite it if it already exists */
   sprintf(&LogFile[0], "%s%s", &LogFile[0], "dbrecs.log");
   if(err = SysFileCreate(&LogFile[0], &LogFD))
   {
      PRINTLOG("\nError %d creating log file %s\n", err, LogFile);
      NotesTerm();
      exit (EXIT_FAILURE);
   }

   /* Move file pointer to end of file to append entries */
   if (err = SysFileSeek(LogFD, 0, 1))
   {
      PRINTLOG("Error moving file pointer in log file %s\n", LogFile);
      SysFileClose(LogFD);
      NotesTerm();
      exit (EXIT_FAILURE);
   }

   if (!strcmp(RecOpt,"ARCHIVE"))
   {
      PRINTLOG("\n Checking for logs to archive ...\n");
      if (err = DoArchiveLogs())
      {
         PRINTLOG("\nError archiving logs\n");
         SysFileClose(LogFD);
         NotesTerm();
         exit (EXIT_FAILURE);
      }
   }

   if (!strcmp(RecOpt,"CHECK"))
   {
      PRINTLOG("\n Checking file to see if new backup is needed ...\n");
      if (err = CheckDb(InFile))
      {
         PRINTLOG("\nError checking file %s ... \n", InFile);
         SysFileClose(LogFD);
         NotesTerm();
         exit (EXIT_FAILURE);
      }
   }

   if (!strcmp(RecOpt,"RECOVER") || !strcmp(RecOpt, "RESTORE") || !strcmp(RecOpt, "NOTE"))
   {
      if (!flags)
      {
         PRINTLOG("\n Taking database offline ...\n");
         if(err = TakeDbsOffline(ResDb))
         {
            PRINTLOG("\nError taking %s offline\n", ResDb);
            SysFileClose(LogFD);
            NotesTerm();
            exit (EXIT_FAILURE);
         }
      }

      PRINTLOG("\n Recovering backup file ...\n\n");

      if(err = RecoverDbs(InFile, flags, NoteInfo))
      {
         PRINTLOG("\nError recovering backup file.\n");
         SysFileClose(LogFD);
         NotesTerm();
         exit (EXIT_FAILURE);
      }
      if (!flags)
      {
         PRINTLOG("\n The recovered backup file is %s\n",InFile);
         PRINTLOG("\n The database to be restored is %s\n",ResDb);
         PRINTLOG("\n\n Restoring database from recovered backup file ...\n");
         if(err = RestoreDbs(InFile, ResDb))
         {
            PRINTLOG("\nError restoring %s \n", ResDb);
            SysFileClose(LogFD);
            NotesTerm();
            exit (EXIT_FAILURE);
         }

         PRINTLOG("\n Bringing database online ...\n");
         if(err = BringDbsOnline(ResDb))
         {
            PRINTLOG("\nError bringing %s online\n", ResDb);
            SysFileClose(LogFD);
            NotesTerm();
            exit (EXIT_FAILURE);
         }
      }
   }

   /* Close log file */
   SysFileClose(LogFD);

   /* Terminate Notes */
   NotesTerm();

   PRINTLOG ("\n\nProgram completed successfully.\n");

   exit (EXIT_SUCCESS);
}

STATUS LNPUBLIC MyCallback(UNID * LogId, DWORD LogNumber, char * path)
/************************************************************************

   FUNCTION:  MyCallback

   PURPOSE:   Callback for media recovery.

   INPUTS:    Logid      - Pointer to the logid.
              LogNumber  - The log number for the desired log.
              path       - Where to put the log file.

   RETURNS:   Returns status of the call.

************************************************************************/
{
   char FilePath[MAXPATHLENGTH];

   /* determine where the log is */
   sprintf(&FilePath[0],
           "%s%s%08x%sS%07x.TXN",
           LogDir,
           DirSlash,
           LogId->File.Innards[0] ^
           LogId->File.Innards[1] ^
           LogId->Note.Innards[0] ^
           LogId->Note.Innards[1],
           DirSlash,
           LogNumber);

   /* put the log where requested */
   return (!SysFileCopy(&FilePath[0], path, TRUE));
}

STATUS LNPUBLIC NoteCallback(DWORD state_flags, void far *userParm, NOTE_RESTORE_CALLBACK_INFO far * pinfo)
/************************************************************************

   FUNCTION:  NoteCallback

   PURPOSE:   Callback for NSFRecoverDatabasesWithCallback.

   INPUTS:    state_flags - State flags.
              userParm    - Pointer to user parameters.
              info        - Pointer to NOTES_RESTORE_CALLBACK_INFO structure.

   RETURNS:   Returns status of the call.

************************************************************************/
{

   STATUS   err= NOERROR;
   NOTE_RESTORE_CALLBACK_INFO info;
   char     timebuffer[MAXALPHATIMEDATE + 1];
   WORD     timelength;
   char     EventString[MAXPATHLENGTH + 100];
   char     note_action[25]="";


   memcpy( (char*)&info, (char*)pinfo, sizeof(NOTE_RESTORE_CALLBACK_INFO) );

   err = ConvertTIMEDATEToText(NULL, NULL, &info.TranTime, &timebuffer[0], MAXALPHATIMEDATE, &timelength);
   timebuffer[timelength] = '\0';

   if(!err)
   {
      switch (state_flags)
      {
         case MediaCallback_NoteInsert:
              strcpy(note_action, "Addition");
              break;

         case MediaCallback_NoteDelete:
              strcpy(note_action, "Deletion");
              break;

         case MediaCallback_CLR:
              strcpy(note_action, "Undo");
              break;

         default:
              strcpy(note_action, "Unknown");
      }

      sprintf(EventString, "\n\tNote information for backup file %s\n\t\tStarted at: %s\n\t\tInfoSize: %d\n\t\tNoteID: %lX\n\t\tNoteHandle: %d\n\t\tUser Name: %s\n\t\tAction: %s\n\t\tFinished at",
              info.PathName,
              timebuffer,
              info.InfoSize,
              info.NoteId,
              info.hNote,
              info.UserName,
              note_action);

      EventLog(LogFD, EventString);
   }
   return err;
}


STATUS RecoverDbs(char * BUPath, DWORD Rflags, int RNoteInfo)
/************************************************************************

   FUNCTION:  RecoverDbs

   PURPOSE:   Recover specified backup file.

   INPUTS:    BUPath - Path to backup file.
              Rflags - Recovery flags.

   RETURNS:   Returns status of the call.

************************************************************************/
{
   STATUS   err= NOERROR;
   char     EventString[MAXPATHLENGTH + 100];
   unsigned short int index = 0;
   unsigned short int errindex = 0;


   /* If the "recover" option has been chosen, we'll refresh the DBIID of
      the recovered file, allowing access to the original DB as well as
      this recovered version.  The "recover" option creates a new
      "instance" of the original database.  This new instance of the
      original database will be a new database that will require its own
      backup file created and maintained if it is ever to be recovered.

      If the "restore" option was chosen, we do not want to refresh the
      original DB's DBIID, as we do not want to create a new "instance".
      In this case the original DB will be deleted when it is taken
      offline then replaced with its recovered backup file. */

   if(Rflags)
      Rflags = DBRECOVER_ZAP_ID;

   if(RNoteInfo)
   {
      sprintf(EventString, "\nRecovering backup file %s with CallBack", BUPath);
      EventLog(LogFD, EventString);
      err = NSFRecoverDatabasesWithCallback(BUPath, MyCallback, Rflags, &index, NULL, NoteCallback, 0);
   }
   else
   {
      if (!Rflags)
          sprintf(EventString, "Recovering backup file %s", BUPath);
      else
          sprintf(EventString, "\nRecovering backup file %s", BUPath);
      EventLog(LogFD, EventString);
      err = NSFRecoverDatabases(BUPath, MyCallback, Rflags, &index, NULL);
   }

   if (!err)
   {
      /* Although this sample was designed to pass a 'list' containing
         only one database name via the BUPath parameter of
         NSFRecoverDatabases, the following loops are included to
         illustrate how to handle a 'list' containing multiple database
         names along with the required imbedded NULLs. */

      while (BUPath[0] != '\0')
      {
          PRINTLOG("\n Backup file recovered.\n");
          sprintf(EventString, "Backup file %s recovery complete", BUPath);
          EventLog(LogFD, EventString);
          BUPath += strlen(BUPath) + 1;
      }
   }
   else
   {
      while (BUPath[0] != '\0')
      {
          errindex++;
          if(errindex ==index)
          {
             PRINTLOG("\nError recovering backup file %s\n", BUPath);
             sprintf(EventString, " *** ERROR recovering backup file %s *** (%s)",
                     BUPath,
                     print_api_error(err));
                     EventLog(LogFD, EventString);
          }
          BUPath += strlen(BUPath) + 1;
      }
   }
   return err;
}

STATUS TakeDbsOffline(char * DbPath)
/************************************************************************

   FUNCTION:  TakeDbsOffline

   PURPOSE:   Take a specified databases offline.

   INPUTS:    DbPath - Path to database.

   RETURNS:   Returns status of the call.

************************************************************************/
{
   STATUS   err = NOERROR;
   DWORD    WaitTime = 0;
   char     EventString[MAXPATHLENGTH + 100];

	/* In this example, since we will be replacing the database once its
      backup has been recovered, we will delete the database when it's
      taken offline and we will not refresh the DBIID during its recovery.
      Domino will not allow two active databases to have the same DBIID.
		
      **NOTE:Great care	should be taken to maintain "recoverable"
      backup files (i.e. once a backup file has been recovered it is no
      longer a "recoverable" file).  If a problem occurs after a
      database has been taken offline and deleted, an existing
      recoverable backup file that was created from that original DB
      will be the ONLY means available to restore the original DB.*/

   if (!(err = NSFTakeDatabaseOffline(DbPath, WaitTime, 0)))
   {
      sprintf(EventString, "\nDatabase file %s taken offline", DbPath);
      EventLog(LogFD, EventString);
   }
   else
   {
      sprintf(EventString,
              " *** ERROR taking database %s offline *** (%s)",
              DbPath,
              print_api_error(err));
      EventLog(LogFD, EventString);
   }

   return err;
}

STATUS RestoreDbs(char * BUPath, char * DbPath)
/************************************************************************

   FUNCTION:  RestoreDbs

   PURPOSE:   Restore a specific database from a specific backup file.

   INPUTS:    BUPath - Path to recovered backup file.
              DbPath - Path to database to restore.

   RETURNS:   Returns status of the call.

************************************************************************/
{
   STATUS   err = NOERROR;
   char     EventString[(2*MAXPATHLENGTH) + 100];

   if (SysFileCopy(BUPath, DbPath, FALSE))
   {
      sprintf(EventString,
              "Database file %s restored from %s",
              DbPath,
              BUPath);
      EventLog(LogFD, EventString);
      return NOERROR;
   }
   else
   {
      sprintf(EventString,
              " ***** ERROR ***** restoring database %s from backup file %s",
              DbPath,
              BUPath);
      EventLog(LogFD, EventString);
      return 1;
   }
}

STATUS BringDbsOnline(char * DbPath)
/************************************************************************

   FUNCTION:  BringDbsOnline

   PURPOSE:   Bring a specified database online.

   INPUTS:    DbPath - Path to database.

   RETURNS:   Returns status of the call.

************************************************************************/
{
   STATUS   err = NOERROR;
   char     EventString[MAXPATHLENGTH + 100];

   if (!(err = NSFBringDatabaseOnline(DbPath, 0)))
   {
      sprintf(EventString, "Database file %s brought online", DbPath);
      EventLog(LogFD, EventString);
   }
   else
   {
      sprintf(EventString,
              " *** ERROR bringing database %s online *** (%s)",
              DbPath,
              print_api_error(err));
      EventLog(LogFD, EventString);
   }
   return err;
}

STATUS DoArchiveLogs(void)
/************************************************************************

   FUNCTION:  DoArchiveLogs
	
   PURPOSE:   Archive any extents that are ready to be archived.

   RETURNS:   Returns status of the call.

************************************************************************/
{
   STATUS err = NOERROR;
   char   EventString[MAXPATHLENGTH + 100];
   int    FirstPass = 1;       /* Flag for initial pass through loop */
   char   LogPath[MAXPATHLENGTH];
   char   ArchiveDir[MAXPATHLENGTH];
   char   ArchivePath[MAXPATHLENGTH];
   DWORD  LogNumber;
   UNID   LogId;
   WORD   LogType;

   if ( err = NSFGetTransLogStyle (&LogType))
      PRINTERROR(err,"NSFGetTransLogStyle");

   if (LogType == TRANSLOG_STYLE_CIRCULAR || LogType == TRANSLOG_STYLE_LINEAR)
   {
      switch(LogType)
      {
         case TRANSLOG_STYLE_CIRCULAR:
              PRINTLOG("\n  Transactional logging is 'CIRCULAR'.\n");
              break;

         case TRANSLOG_STYLE_LINEAR:
              PRINTLOG("\n  Transactional logging is 'LINEAR'.\n");
              break;
      }
      return 1;
   }

   if (err = NSFBeginArchivingLogs ())
   {
      sprintf(EventString,
              " *** ERROR archiving logs *** (%s)",
              print_api_error(err));
      EventLog(LogFD, EventString);
      return 1;
   }

   while (!err)
   {

      /* Be sure on the first pass through to get the first archive log.
         This ensures that the list gets reset in case this didn't leave
         cleanly and the logger believes we are off archiving a log handed
         to us in an earlier invocation. */
      if (FirstPass)
      {
         FirstPass--;
         err = NSFGetFirstLogToArchive(&LogId, &LogNumber, &LogPath[0]);
      }
      else
         err = NSFGetNextLogToArchive(&LogId, &LogNumber, &LogPath[0]);

      /* if we have a log to archive copy it off */
      if (err == NOERROR)
      {
         PRINTLOG("\n  Creating archive log ...\n");

         /* Create the destination file/directory -
         overwrite it if it already exists */
         sprintf(&ArchiveDir[0], "%s%s",   LogDir, DirSlash);
         if (err = SysFileCreateDirectory(&ArchiveDir[0]))
         {
            PRINTLOG("\nError %d creating directory %s\n", err, ArchiveDir);
            break;
         }

         sprintf(&ArchiveDir[0],
                 "%s%08x%s",
                 &ArchiveDir[0],
                 LogId.File.Innards[0] ^
                 LogId.File.Innards[1] ^
                 LogId.Note.Innards[0] ^
                 LogId.Note.Innards[1],
                 DirSlash);
         if (err = SysFileCreateDirectory(&ArchiveDir[0]))
         {
            PRINTLOG("\nError %d creating directory %s\n", err, ArchiveDir);
            break;
         }

         sprintf(&ArchivePath[0], "%sS%07d.TXN", &ArchiveDir[0], LogNumber);

         /* copy the log off */
         if (!SysFileCopy(&LogPath[0], &ArchivePath[0], TRUE))
         {
            PRINTLOG("\nError copying files \n");
            err = 1;
            break;
         }

         if (err = NSFDoneArchivingLog(&LogId, &LogNumber))
         {
            PRINTERROR(err,"NSFDoneArchivingLog");
            break;
         }

         sprintf(EventString,
                 "\nArchived Logid: %d %d %d %d Extent %d to %s",
                 LogId.File.Innards[0],
                 LogId.File.Innards[1],
                 LogId.Note.Innards[0],
                 LogId.Note.Innards[1],
                 LogNumber,
                 ArchivePath);

         EventLog(LogFD, EventString);

         PRINTLOG("   Archived Logid: %d %d %d %d Extent %d,\n   %s created.\n",
                LogId.File.Innards[0],
                LogId.File.Innards[1],
                LogId.Note.Innards[0],
                LogId.Note.Innards[1],
                LogNumber,
                ArchivePath);
      }
      else if (err == ERR_NO_TRANSLOGS_TO_ARCHIVE)
      {
         sprintf(EventString,
                 "\n *** Log status *** (%s)",
                 print_api_error(err));
         EventLog(LogFD, EventString);
         err = NOERROR;
         break;
      }
      else
      {
         sprintf(EventString,
                 " *** ERROR archiving logs *** (%s)",
                 print_api_error(err));
         EventLog(LogFD, EventString);
         break;
      }
   }
   if (err = NSFEndArchivingLogs ())
   {
      sprintf(EventString,
              " *** ERROR archiving logs *** (%s)",
              print_api_error(err));
      EventLog(LogFD, EventString);
   }
   return err;
}

void EventLog(int LogFD, char * outstring)
/************************************************************************

   FUNCTION:  EventLog
	
   PURPOSE:   Write a record to the log file.

************************************************************************/
{
   TIMEDATE now;
   char     timebuffer[MAXALPHATIMEDATE + 1];
   char     Buffer[MAXPATHLENGTH];
   DWORD    BuffLen;
   WORD     timelength;

   OSCurrentTIMEDATE(&now);
   ConvertTIMEDATEToText(NULL, NULL, &now, &timebuffer[0], MAXALPHATIMEDATE, &timelength);
   timebuffer[timelength] = '\0';

   sprintf(Buffer, "%s: %s\n", outstring, timebuffer);
   BuffLen = strlen(Buffer);

   if (SysFileWrite(LogFD, Buffer, BuffLen))
   {
      PRINTLOG("Error writing to Log File\n");
      SysFileClose(LogFD);
   }
   return;
}

STATUS CheckDb(char * BUPath)
/************************************************************************

   FUNCTION:  CheckDb

	PURPOSE:   Check to see if a new backup file is needed.

   INPUTS:    BUPath - Path to backup file.

   RETURNS:   Returns status of the call.

************************************************************************/
{
   STATUS   err= NOERROR;
   char     EventString[MAXPATHLENGTH + 100];
   char     EventString2[MAXPATHLENGTH + 100];
   DWORD    ComfortSpan = 0;  /* For 'CIRCULAR' or 'ARCHIVE' type logging. */
/*   DWORD    ComfortSpan = 65536;  For 'CIRCULAR' type logging only */
   DWORD    BackupNeeded;
   WORD     LogType;
   int      unkown = 0;


   /* If the "check" option has been chosen, we'll determine the
      state of the database or database backup file with respect
      to the current log state.*/


   if ( err = NSFGetTransLogStyle (&LogType))
      PRINTERROR(err,"NSFGetTransLogStyle");

   switch (LogType)
   {
      case TRANSLOG_STYLE_ARCHIVE:
           PRINTLOG("\n  Transactional logging type is 'ARCHIVE'.\n");
           if (ComfortSpan)
              PRINTLOG("\n   For 'ARCHIVE' type logging only a ComfortSpan of '0' is supported.\n");
              break;

      case TRANSLOG_STYLE_CIRCULAR:
           PRINTLOG("\n  Transactional logging type is 'CIRCULAR'.\n");
           break;

      case TRANSLOG_STYLE_LINEAR:
           PRINTLOG("\n  Transactional logging type is 'LINEAR'.\n");
           break;

      default:
           unkown = 1;
           PRINTLOG("\n  Transactional logging type is 'UNKOWN'.\n");
           break;
   }

   if(unkown)
      return (1);

   err = NSFIsNewBackupNeeded(BUPath, ComfortSpan, &BackupNeeded);

   if (!err)
   {
      sprintf(EventString, "\nBackup file %s checked - ", BUPath);

      if (ComfortSpan)
      {
         if (BackupNeeded)
             strcat(EventString, "New Backup is needed");
         else
             strcat(EventString, "New Backup is NOT needed");
      }
      else
      {
         sprintf(EventString2, "Span of log is %d", BackupNeeded);
         strcat(EventString, EventString2);
      }

      EventLog(LogFD, EventString);
      PRINTLOG("\n  %s\n", EventString);
   }
   else
   {
      sprintf(EventString,
              " *** ERROR checking backup file %s *** (%s)",
              BUPath,
              print_api_error(err));
      EventLog(LogFD, EventString);
   }
   return err;
}

