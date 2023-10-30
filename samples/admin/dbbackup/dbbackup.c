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

 PROGRAM:   dbbackup

 FILE:      dbbackup.c

 PURPOSE:   This program is intended to illustrate the practical usage of the
            NSFBackupxxx functions of the Backup and Recovery API.  It is not
            intended to show exclusive use of these functions or the most
            effective program logic when using these functions.

 SYNTAX:    dbbackup <database filename> <output filename>

****************************************************************************/

/* OS and C include files */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* Notes API include files */
#include "global.h"
#include "nsfdb.h"
#include "nsfdata.h"
#include "osmisc.h"
#include "osfile.h"
#include "osmem.h"
#include "win32io.h"
#include "../../printLog.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

#define BUFFER_SIZE   32768


/* define exit codes if they are not defined elsewhere */
#ifndef EXIT_SUCCESS
   #define EXIT_SUCCESS 0
#endif

#ifndef EXIT_FAILURE
   #define EXIT_FAILURE 1
#endif


/* Program declaration */
VOID main (int argc, char *argv[])
{

   /* Local data declarations */
   char     *path_name;      /* pathname of database */
   char     *backup_file;    /* pathname of backup file */
   DHANDLE    hDB;
   STATUS   err = NOERROR;
   int      srcfd, dstfd;
   DHANDLE    BackupContext;
   DWORD    FileSizeHigh, FileSizeLow;
   char     FullPath[1024];
   DWORD    SrcPositionLow = 0;
   DWORD    SrcPositionHigh = 0;
   char     *Buffer;
   DHANDLE    hBuffer;
   DWORD    BytesLeft;
   DWORD    BytesRead;
   DWORD    ReadCount;
   DWORD    InfoSizeHigh;
   DWORD    InfoSizeLow;
   DWORD    FilledSize;
   DHANDLE    ApplyInfoContext;
   BOOL     Logged;
   UNID     LogID;
   UNID     DbIID;
   DWORD    LogExtent;

   /* Get the pathname of the database from the command line. */
   if (argc != 4)
   {
      PRINTLOG( "\nUsage:  dbbackup <Domino ini path> <database filename> <output filename>\n");
      exit (EXIT_SUCCESS);
   } 

   path_name = argv[2];
   backup_file = argv[3];

   /* Initialize Notes */
   if (NotesInitExtended (argc, argv))
   {
      PRINTLOG ("\nError initializing Notes.\n");
      exit (EXIT_FAILURE);
   }

   /* Open the database we're going to backup */
   if (err = NSFDbOpen(path_name, &hDB))
   {
      PRINTERROR (err,"NSFDbOpen");
      NotesTerm();
      exit (EXIT_FAILURE);
   }

   /* And get its full path so we can open it at the OS level */
   if (err = NSFDbPathGet(hDB, NULL, FullPath))
   {
      PRINTERROR (err,"NSFDbPathGet");
      NSFDbClose(hDB);
      NotesTerm();
      exit (EXIT_FAILURE);
   }

   /* Start recording changes to the file at the NSF level */
   if (err = NSFBackupStart(hDB,
                            0L,
                            &BackupContext,
                            &FileSizeLow,
                            &FileSizeHigh))
   {
      PRINTERROR (err,"NSFBackupStart");
      NSFDbClose(hDB);
      NotesTerm();
      exit (EXIT_FAILURE);
   }

   /* Check to see if DB is being logged. */
   if (err = NSFDbGetLogInfo(hDB, 0L, &Logged, &LogID, &DbIID, &LogExtent))
   {
      PRINTERROR (err,"NSFDbGetLogInfo");
      NSFBackupEnd(hDB, BackupContext, BACKUPEND_ABORT);
      NSFDbClose(hDB);
      NotesTerm();
      exit (EXIT_FAILURE);
   }
   if(!Logged)
   {
      PRINTLOG("\n  Database '%s' is not currently logged ...\n", path_name);
      PRINTLOG("\n  Resulting backup file '%s' WILL NOT BE RECOVERABLE!!!\n", backup_file);
   }

   /* Open the database file at the OS level */
   if (err = SysFileOpenRead(FullPath, &srcfd))
   {
      PRINTERROR (err,"SysFileOpenRead");
      NSFBackupEnd(hDB, BackupContext, BACKUPEND_ABORT);
      NSFDbClose(hDB);
      NotesTerm();
      exit (EXIT_FAILURE);
   }

   /* Move file pointer to begining of file */
   if (err = SysFileSeek(srcfd, 0, 0))
   {
      PRINTERROR (err,"SysFileSeek");
      SysFileClose(srcfd);
      NSFBackupEnd(hDB, BackupContext, BACKUPEND_ABORT);
      NSFDbClose(hDB);
      NotesTerm();
      exit (EXIT_FAILURE);
   }

   /* Create the destination file -
      do not overwrite it if it already exists */
   if (err = SysFileCreate(backup_file, &dstfd))
   {
      PRINTERROR (err,"SysFileCreate");
      SysFileClose(srcfd);
      NSFBackupEnd(hDB, BackupContext, BACKUPEND_ABORT);
      NSFDbClose(hDB);
      NotesTerm();
      exit (EXIT_FAILURE);
   }

   /* Allocate a buffer to use for reading from the source and
      writing to the destination file */
   if (err = OSMemAlloc(MEM_SHARE, BUFFER_SIZE, &hBuffer))
   {
      PRINTERROR (err,"OSMemAlloc");
      SysFileClose(dstfd);
      SysFileDelete(backup_file);
      SysFileClose(srcfd);
      NSFBackupEnd(hDB, BackupContext, BACKUPEND_ABORT);
      NSFDbClose(hDB);
      NotesTerm();
      exit (EXIT_FAILURE);
   }

   Buffer = OSLock(char, hBuffer);

   /* Copy database file at the OS level */
   if ((MAXDWORD - FileSizeHigh) >= FileSizeLow)
   {
      BytesLeft = FileSizeHigh + FileSizeLow;
      FileSizeLow = 0;
   }
   else
   {
      BytesLeft = FileSizeHigh;
   }

   do
   {
      ReadCount = 0;
      while (BytesLeft)
      {
         BytesRead = min(BUFFER_SIZE, BytesLeft);

         if (err = SysFileRead(srcfd, Buffer, BytesRead))
            break;

         if ((MAXDWORD - SrcPositionLow) < BytesRead)
         {
            SrcPositionHigh++;
            SrcPositionLow = (BytesRead - (MAXDWORD - SrcPositionLow)) - 1;
         }
         else
         {
            SrcPositionLow += BytesRead;
         }

         /* Occassionally tell NSF that we've made progress to cut down on
            recording overhead */
         if ((++ReadCount % 10) == 0)
            NSFBackupSetHighWaterMark(hDB,
                                      BackupContext,
                                      SrcPositionLow,
                                      SrcPositionHigh);

         if (err = SysFileWrite(dstfd, Buffer, BytesRead))
            break;

         BytesLeft -=BytesRead;
      }

      if (FileSizeHigh > 0)
      {
         BytesLeft = MAXDWORD;
         FileSizeHigh--;
      }
      else
      {
         BytesLeft = FileSizeLow;
         FileSizeLow = 0;
      }

   } while (BytesLeft && !(err));

   /* Close our copy */
   SysFileClose(dstfd);

   /* If the copy hit a problem clean up */
   if (err)
   {
      PRINTERROR (err,"OSMemAlloc");
      OSUnlockObject(hBuffer);
      OSMemFree(hBuffer);
      SysFileDelete(backup_file);
      SysFileClose(srcfd);
      NSFBackupEnd(hDB, BackupContext, BACKUPEND_ABORT);
      NSFDbClose(hDB);
      NotesTerm();
      exit (EXIT_FAILURE);
   }

   /* Stop recording changes */
   NSFBackupStop(hDB, BackupContext);

   /* Get the change info size */
   if (err = NSFBackupGetChangeInfoSize(hDB,
                                        BackupContext,
                                        0,
                                        &InfoSizeLow,
                                        &InfoSizeHigh))
   {
      PRINTERROR (err,"NSFBackupGetChangeInfoSize");
      OSUnlockObject(hBuffer);
      OSMemFree(hBuffer);
      SysFileDelete(backup_file);
      SysFileClose(srcfd);
      NSFBackupEnd(hDB, BackupContext, BACKUPEND_ABORT);
      NSFDbClose(hDB);
      NotesTerm();
      exit (EXIT_FAILURE);
   }

   /* Initiate getting the change info */
   if (err = NSFBackupStartApplyChangeInfo(&ApplyInfoContext,
                                           backup_file,
                                           0,
                                           InfoSizeLow,
                                           InfoSizeHigh))
   {
      PRINTERROR (err,"NSFBackupStartApplyChangeInfo");
      OSUnlockObject(hBuffer);
      OSMemFree(hBuffer);
      SysFileDelete(backup_file);
      SysFileClose(srcfd);
      NSFBackupEnd(hDB, BackupContext, BACKUPEND_ABORT);
      NSFDbClose(hDB);
      NotesTerm();
      exit (EXIT_FAILURE);
   }

   /* Pull the change info and apply it to our copy */
   do
   {
      if (err = NSFBackupGetNextChangeInfo(hDB,
                                           BackupContext,
                                           0,
                                           Buffer,
                                           BUFFER_SIZE,
                                           &FilledSize))
      {
         PRINTERROR (err,"NSFBackupGetNextChangeInfo");
         NSFBackupEndApplyChangeInfo(ApplyInfoContext, APPLYEND_ABORT);
         OSUnlockObject(hBuffer);
         OSMemFree(hBuffer);
         SysFileDelete(backup_file);
         SysFileClose(srcfd);
         NSFBackupEnd(hDB, BackupContext, BACKUPEND_ABORT);
         NSFDbClose(hDB);
         NotesTerm();
         exit (EXIT_FAILURE);
      }

      if (err = NSFBackupApplyNextChangeInfo(ApplyInfoContext,
                                             0,
                                             Buffer,
                                             FilledSize))
      {
         PRINTERROR (err,"NSFBackupApplyNextChangeInfo");
         NSFBackupEndApplyChangeInfo(ApplyInfoContext, APPLYEND_ABORT);
         OSUnlockObject(hBuffer);
         OSMemFree(hBuffer);
         SysFileDelete(backup_file);
         SysFileClose(srcfd);
         NSFBackupEnd(hDB, BackupContext, BACKUPEND_ABORT);
         NSFDbClose(hDB);
         NotesTerm();
         exit (EXIT_FAILURE);
      }

   } while (FilledSize);

   /* Done with the backup */
   NSFBackupEndApplyChangeInfo(ApplyInfoContext,0);
   NSFBackupEnd(hDB, BackupContext, 0);

   /* Free up our buffer */
   OSUnlockObject(hBuffer);
   OSMemFree(hBuffer);

   /* Close our source */
   SysFileClose(srcfd);

   /* Close our database */
   NSFDbClose(hDB);

   /* Terminate Notes. */
   NotesTerm();

   PRINTLOG("\nThe backup file created is %s\n",backup_file);
   PRINTLOG ("\nProgram completed successfully\n");

   /* End of main program. */
   exit (EXIT_SUCCESS);
}
