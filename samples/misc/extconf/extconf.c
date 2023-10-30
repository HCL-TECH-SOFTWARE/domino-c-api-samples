/***************************************************************************
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
  
  PROGRAM:    extconf.dll
  
  FILE:       extconf.c
  
  This sample program demonstrates an extension manager that resolves
  replication conflicts.

  When a replication conflict between two documents occurs in the sample
  database, Conflict Handler, this extension manager examines each item 
  in the "newer" document.  If the item is the Payout field, it compares
  the value of this item with the one in the older document and saves
  the lower value.  For all other items, it determines whether there is a 
  conflict between the two documents.  If there is a conflict, the newer 
  item value is saved.  The extension manager then returns 
  CONFLICT_ACTION_MERGE to the HCL Domino Server so that the two documents 
  will be merged and a Replication or Save Conflict document will not be 
  posted.

***************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

#include "stdlib.h"
#include "stdio.h"

#include "global.h"
#include "pool.h"
#include "extmgr.h"
#include "misc.h"
#include "nsfdb.h"
#include "nsfnote.h"
#include "nsferr.h"
#include "nsfnote.h"
#include "osmem.h"
#include "stdnames.h"
#include "misc.h"
#include "osmisc.h"

/* Program specific include file */
#include "extconf.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

#define TEST_DB_TITLE   "Conflict Handler"
#define TEST_ITEM_NAME  "Payout"

HEMREGISTRATION RegEMId1 = 0; 

STATUS LNCALLBACK ExtCallback(EMRECORD FAR * pExRecord);

/****************************************************************************

    FUNCTION: MainEntryPoint

    PURPOSE:  This function is called by Domino and Notes (NSF) at 
              initialization time; it is exported as ordinal @1.


*****************************************************************************/
STATUS LNPUBLIC DLL_EXPORT_INFIX MainEntryPoint(void)
{
   STATUS err;

   err = EMRegister( EM_NSFCONFLICTHANDLER,
                     EM_REG_BEFORE,           
                     ExtCallback,
                     0, &RegEMId1);
                  
    return err;

}

/****************************************************************************

    FUNCTION: ExtCallback

    PURPOSE:  Called by extension manager when a Conflict occurs.

****************************************************************************/

STATUS LNCALLBACK ExtCallback(EMRECORD FAR * pExRecord)
{
    STATUS   error = NOERROR;

    switch (pExRecord->EId)
    {
        case EM_NSFCONFLICTHANDLER:
        {
            DHANDLE hDb;
            DHANDLE hOldNote;
            DHANDLE hNewNote;
            DWORD *pConflictAction;
            VARARG_PTR ap;
    
            char dbinfo [NSF_INFO_SIZE];
            char title [MAXSPRINTF];

            TIMEDATE tdDiverge;
            BLOCKID  bhOldItem;
            BLOCKID  bhNewItem;
            BLOCKID bhOldValue;
            BLOCKID bhNewValue;
            TIMEDATE tdOldItem;
            TIMEDATE tdNewItem;
            double OldValue;
            double NewValue;
      
            BLOCKID bhNewItemDeletes [10];
            BLOCKID bhOldItemDeletes [10];
            WORD  NumNewItemDeletes = 0;
            WORD  NumOldItemDeletes = 0;

            ap = pExRecord->Ap;
            hDb = VARARG_GET(ap,DHANDLE);
            hOldNote = VARARG_GET(ap,DHANDLE);
            hNewNote = VARARG_GET(ap,DHANDLE);
            pConflictAction = VARARG_GET(ap,DWORD *);

            if (pExRecord->NotificationType != EM_BEFORE)
                break;

            /* Only do this for our sample DB */
  
            if (error = NSFDbInfoGet (hDb, dbinfo))
                break;

            NSFDbInfoParse(dbinfo, INFOPARSE_TITLE, title, sizeof(title));
            if (strcmp(title, TEST_DB_TITLE))
                break;

            /* Check the parameters */

            if (!hOldNote || !hNewNote || !pConflictAction)
                break;
      
            /* Find the point of divergence.  Items later than this time 
               are possibly in conflict */
                  
            if (error = NSFNoteFindDivergenceTime (hOldNote, hNewNote, 
                                                   0L, &tdDiverge))
                break;

            /* Loop thru all the items in the new note. 
               NOTE: we are assuming that there are no unique items in 
               either note.  */
      
            error = NSFItemInfo(hNewNote, NULL, 0, &bhNewItem, NULL, 
                                &bhNewValue, NULL);
            while (!error)
            {
                char  NewName [MAXSPRINTF];
                WORD  NewNameLen;
                BOOL  fPayout = FALSE;
            
                /* Get the new item's modified time */       

                if (error = NSFItemGetModifiedTimeByBLOCKID (hNewNote, 
                                                             bhNewItem,
                                                             0L, &tdNewItem))
                    goto Done;
            
                /* Grab the item name */
            
                NSFItemQuery(hNewNote, bhNewItem, NewName, sizeof(NewName),
                             &NewNameLen, NULL, NULL, NULL, NULL);
                NewName [NewNameLen] = '\0';
               
                /* Skip over the an $ special items */
            
                if (NewName [0] == '$')
                    goto NextItem;
               
                /* If it is our special item then grab it's value. */
            
                if (strcmp(NewName, TEST_ITEM_NAME) == 0)
                {
                    fPayout = TRUE;
                    NewValue = *((double*)(OSLockBlock(WORD, bhNewValue)+1));
                    OSUnlockBlock(bhNewValue);
                }
               
                /* Find the matching item in the old note and get 
                   its time. */

                if (error = NSFNoteFindMatchingItem (hNewNote, bhNewItem, 
                                                     hOldNote, 0L, 
                                                     &bhOldItem))
                    goto Done;

                if (error = NSFItemGetModifiedTimeByBLOCKID (hOldNote, 
                                                             bhOldItem,
                                                             0L, &tdOldItem))
                    goto Done;

                /* If this is the payout item then get the old value and 
                   set both note's items to the lowest of the two values */
               
                if (fPayout)
                {
                    NSFItemQuery(hOldNote, bhOldItem, NULL, NULL, NULL,
                                 NULL, NULL, &bhOldValue, NULL);
                    OldValue = *((double*)(OSLockBlock(WORD, bhOldValue)+1));
                    OSUnlockBlock(bhOldValue);
               
                    if (NewValue < OldValue)
                        bhOldItemDeletes [NumOldItemDeletes++] = bhOldItem;                  
                    else if (OldValue < NewValue)
                        bhNewItemDeletes [NumNewItemDeletes++] = bhNewItem;                  
                }  
                else
                {
                    int   compare;
               
                    /* If we have a conflict take the new value. */
                           
                    if (TimeDateCompare (&tdDiverge, &tdNewItem) == -1 ||
                        TimeDateCompare (&tdDiverge, &tdOldItem) == -1)
                    {
                        /* We have a conflict */
                  
                        compare = TimeDateCompare (&tdNewItem, &tdOldItem);
                        if (compare == 1)
                            bhOldItemDeletes [NumOldItemDeletes++] = bhOldItem;                  
                        if (compare == -1)
                            bhNewItemDeletes [NumNewItemDeletes++] = bhNewItem;                  
                    }
                }
            
                /* Get the next item on the new note */
NextItem:            
                error  = NSFItemInfoNext(hNewNote, bhNewItem, NULL, 0,
                                         &bhNewItem, NULL, &bhNewValue, NULL);
            }

            if (error == ERR_ITEM_NOT_FOUND)
                error = NOERROR;

            if (error == NOERROR)
            {
                WORD  i;
            
                /* Do the old item deletes */
            
                for (i = 0; i < NumOldItemDeletes; i++)
                    if (error = NSFItemDeleteByBLOCKID(hOldNote, 
                                                       bhOldItemDeletes [i]))
                        goto Done;
            
                /* Do the new item deletes */
            
                for (i = 0; i < NumNewItemDeletes; i++)
                    if (error = NSFItemDeleteByBLOCKID(hNewNote, 
                                                       bhNewItemDeletes [i]))
                        goto Done;
            }
                        
            /* Tell the replicator to merge the rest of the note. */

            *pConflictAction = CONFLICT_ACTION_MERGE;
            break;
        }
    }

Done:
         
    if (!error)
        error = ERR_EM_CONTINUE;
    return error;
}


STATUS LNPUBLIC DLL_EXPORT_INFIX ExtClear (void) 
{
    STATUS      status;

    if (0 != RegEMId1)
        status = EMDeregister (RegEMId1);
    else
        status = NOERROR;

    return (status);
}

