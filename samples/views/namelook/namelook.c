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

    PROGRAM:    namelook

    FILE:       namelook.c

    SYNTAX:     namelook <ViewNames> <Names> <Items> [ServerName]

    PURPOSE:    Demonstrates the Name Lookup C API functions

    DESCRIPTION:
        This program demonstrates the C API function NAMELookup2 and
        the associated look up buffer access functions.

        NAMELOOK looks up names in the Name & Address book. It retrieves
        item values from each document it finds that matches a name. 
        NAMELOOK converts each item to text and prints it on the screen.

        The command line arguments specify the view(s) to search, the 
        name(s) to searh for, the item(s) to retrieve, and, optionally, 
        the name of the server where the lookup should occur.

        NAMELOOK tests the C API function NAMELookup2 directly. It 
        also exercises routines that parse the buffer returned from 
        NAMELookup. These buffer-parsing routines include 
        NAMELocateNextName2, NAMELocateNextMatch2, NAMELocateItem2, 
        and NAMEGetTextItem2.

        NAMELOOK handles multiple views, multiple names, and multiple 
        items. Specify multiple views, names, or items by separating the 
        views, names, or items with semi-colons. See the readme.txt file for
        a list of examples to follow.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

#include <stdio.h>
#include <string.h>

#include <global.h>
#include <nsfdata.h>                    /* TYPE_TEXT, etc.*/
#include <osmem.h>
#include <misc.h>
#include <niferr.h>                     /* ERR_NO_MORE_MEMBERS */
#include <lookup.h>
#include <osmisc.h>
#include "../../printLog.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif


/* function prototypes */
STATUS  LNPUBLIC  DisplayItem(char  *pMatch, 
                                  char  *szItemName, 
                                  WORD   wItemNum,
                                  char  *pItemValue, 
                                  WORD   DataType, 
                                  WORD   Size);

void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                char *view_names, char *names, char *items, char *servername); 

/* constants */
#define MAX_ARG_LEN 512
#define MAX_ITEM_TEXT 512
#define DATATYPE_SIZE sizeof(USHORT)
#define ITEM_LENGTH_SIZE sizeof(USHORT)
#define NUMERIC_SIZE sizeof(NUMBER)
#define TIME_SIZE sizeof(TIMEDATE)

/************************************************************************

    FUNCTION:   Main

*************************************************************************/

int main (int argc, char *argv[])
{
    STATUS   error = NOERROR;
    char     argNameSpaces[MAX_ARG_LEN];
    char     argNames[MAX_ARG_LEN];
    char     argItems[MAX_ARG_LEN];
    char     ServerName[MAX_ARG_LEN];
    char     *ptr;
    char     *pItemName;
    WORD     NumNameSpaces, NumNames, NumNameRecords, NumItems;
    WORD     Length; /* NumMatches; */
    DWORD    NumMatches;
    WORD     i, j ;
    WORD     usNameRecord, usMatch, usItem ;
    char     NameSpaces[MAX_ARG_LEN];
    char     Names[MAX_ARG_LEN];
    char     Items[MAX_ARG_LEN];
    DHANDLE  hLookup;
    char     *pLookup;
    BYTE     *pNameRecord;
    char     *pView;
    char     *pName;
    char     *pMatch;
    WORD     DataType, Size;
    char     *pItemValue;


    memset(argNameSpaces, '\0', MAX_ARG_LEN);
    memset(argNames, '\0', MAX_ARG_LEN);
    memset(argItems, '\0', MAX_ARG_LEN);
    memset(ServerName, '\0', MAX_ARG_LEN);
    ProcessArgs(argc, argv, argNameSpaces, argNames, argItems, ServerName); 
        

    /*  Parse argNameSpaces to obtain list of view names */

    for (NumNameSpaces=0, Length=0, ptr = strtok(argNameSpaces, ";");
         ptr != NULL;
         NumNameSpaces++, Length += strlen(ptr)+1, ptr = strtok(NULL, ";"))
    {
         strncpy(NameSpaces+Length, ptr, (MAX_ARG_LEN-1)-Length);
    }
    if (NumNameSpaces == 0)
    {
        NumNameSpaces = 1;        /* Allow null string for view name */
        NameSpaces[0] = '\0';
    }

    /*  Parse argNames to obtain list of names to look up */

    for (NumNames=0, Length=0, ptr = strtok(argNames, ";");
         ptr != NULL;
         NumNames++, Length += strlen(ptr)+1, ptr = strtok(NULL, ";"))
    {
         strncpy(Names+Length, ptr, (MAX_ARG_LEN-1)-Length);
    }
    if (NumNames == 0)
    {
        NumNames = 1;            /* Allow null string for name */
        Names[0] = '\0';
    }

    /*  Parse argItems for list of items to retrieve for each name */

    for (NumItems=0, Length=0, ptr = strtok(argItems, ";");
         ptr != NULL;
         NumItems++, Length += strlen(ptr)+1, ptr = strtok(NULL, ";"))
    {
        strncpy(Items+Length, ptr, (MAX_ARG_LEN-1)-Length);
    }
    if (NumItems == 0)
    {
        PRINTLOG("Error: no items requested.\n");
        error=1;
        goto End1;
    }

    PRINTLOG("Looking up %d item(s) for %d name(s) in %d view(s).\n",
                          NumItems, NumNames, NumNameSpaces);

    /*   Start by calling Notes Init.  */

    if (error = NotesInitExtended (argc, argv))
    {
        PRINTLOG("\n Unable to initialize Notes. Error Code[0x%04x]\n", error);
        goto End1;
    }
   
    if (error = NAMELookup2(ServerName, 0,
                            NumNameSpaces, NameSpaces,
                            NumNames, Names,
                            NumItems, Items,
                            &hLookup))
    {
        PRINTLOG("Error: unable to look up names in N&A books.\n");
        goto End;
    }

    pLookup = (char *) OSLockObject(hLookup);

    /* Loop over name records */

    pNameRecord = NULL;
    NumNameRecords = NumNames * NumNameSpaces ;

    for(usNameRecord = 0; usNameRecord < NumNameRecords ; usNameRecord++) 
    {
        pNameRecord = (BYTE *) NAMELocateNextName2(pLookup, pNameRecord, &NumMatches);

        /* set pView to the name of the view we found this name record in */
        for(pView = NameSpaces, 
            i=0; i < (usNameRecord%NumNameSpaces); i++)
        {
            pView += (strlen(pView)+1);
        }

        /* set pName to the name that this name record matches */ 
        for(pName = Names, 
            j=0; j < (usNameRecord%NumNames); j++)
        {
            pName += (strlen(pName)+1);
        }

        PRINTLOG("Found %d match(es) in record for name '%s' in view '%s'.\n",
                          NumMatches, pName, pView);

        /* Loop over matches in this name record */

        for(pMatch = NULL, 
            usMatch = 0; usMatch < NumMatches; usMatch++)
        {
            pMatch = (char *) NAMELocateNextMatch2(pLookup, pNameRecord, pMatch);

            PRINTLOG("Item(s) in match # %d:\n", usMatch+1);

            /* For each requested item, get information and display. */

            for(pItemName = NULL, 
                usItem = 0; usItem < NumItems; usItem++)
            {                    
                if (pItemName==NULL) 
                    pItemName = Items;
                else
                    pItemName += (strlen(pItemName) + 1);

                pItemValue = (char *) NAMELocateItem2(pMatch, usItem,
                                 &DataType, &Size);

                if (pItemValue == NULL) /* if item not present in this match */
                    continue;           /* go on to next match */

                if (error = DisplayItem(pMatch, pItemName, usItem,
                                        pItemValue, DataType,
                                        Size))
                {
                    goto CleanUp;
                }
            } /* end of loop over items in one match */
        } /* end of loop over matches in one name record */
    } /* end of loop over name records */

CleanUp:
    OSUnlockObject(hLookup);
    OSMemFree(hLookup);

End:
    if (error == NOERROR)
    {
        PRINTLOG("\nProgram completed successfully.\n");
    }
    else
    {
        PRINTERROR (error,"NAMELookup2"); 
    }

    NotesTerm();
End1:
    return (error); 
}

/************************************************************************

    FUNCTION:   DisplayItem

*************************************************************************/

STATUS  LNPUBLIC  DisplayItem(char   *pMatch,
                                  char   *szItemName,
                                  WORD    wItemNum,
                                  char   *pItemValue, 
                                  WORD    DataType, 
                                  WORD    Size)
{
    STATUS        error = NOERROR;
    char          szItemText[MAX_ITEM_TEXT+1]; /* text of an item */
    WORD          wItemTextLen = 0;            /* length of str in szItemText[] */
    char          *szDataType;                 /* printable version of DataType */
    WORD          wElement;                    /* number of element of list */
    char          szElement[MAX_ITEM_TEXT+1];  /* text of element of list */
    NUMBER        NumericItem;                 /* numeric item value */
    TIMEDATE      TimeItem;                    /* time/date item value */
    TIMEDATE_PAIR TimeDatePair;                /* time/date pair item value */
    WORD          TimeStringLen;               /* length of ASCII time/date */
    RANGE         Range;                       /* time list or number list header */

    PRINTLOG("item %d:", wItemNum+1);
    PRINTLOG("\tName   = '%s'\n", szItemName);
    PRINTLOG("\tLength = %d\n", (Size-DATATYPE_SIZE));

    if (Size <= DATATYPE_SIZE)
    {
        PRINTLOG("\tItem has no value.\n");
        return (NOERROR);
    }

    switch(DataType)
    {
        case TYPE_TEXT:
            szDataType = "TYPE_TEXT";
            wItemTextLen = Size-DATATYPE_SIZE;
            memcpy (szItemText, pItemValue+DATATYPE_SIZE, wItemTextLen);
            szItemText[wItemTextLen] = '\0';
            break;

        case TYPE_TEXT_LIST:
            szDataType = "TYPE_TEXT_LIST";
            szItemText[0] = '\0';
            for (wElement = 0; ; wElement++)
            {
                if (error = NAMEGetTextItem2(pMatch, wItemNum, 
                                wElement, szElement, MAX_ITEM_TEXT))
                {
                    if (ERR(error) == ERR_NO_MORE_MEMBERS)
                    {   /* there are no more items in the text list */
                        error = NOERROR;
                        break;
                    }
                    return(error);  
                }

                /* szElement now contains a zero-terminated string */

                if(wElement>0) /* if this is the 2nd, 3rd, etc. element */
                {              /* append semicolon-space as a separator */
                    strncat( szItemText, "; ", MAX_ITEM_TEXT-wItemTextLen );
                    wItemTextLen = strlen(szItemText);
                }     
                strncat( szItemText, szElement, MAX_ITEM_TEXT-wItemTextLen );
                wItemTextLen = strlen(szItemText);
            }
            break;

        case TYPE_NUMBER:
            szDataType = "TYPE_NUMBER";
            memcpy (&NumericItem, pItemValue+DATATYPE_SIZE, NUMERIC_SIZE);
            sprintf (szItemText, "%g", NumericItem);
            break;

        case TYPE_NUMBER_RANGE: 
            szDataType = "TYPE_NUMBER_RANGE";
            szItemText[0] = '\0';
            pItemValue += sizeof(WORD);    /* step over data type word */
            Range = *((RANGE*)pItemValue); /* pick off the RANGE */
            pItemValue += sizeof(RANGE);   /* step over RANGE header */
            for (wElement=0; wElement<Range.ListEntries; wElement++)
            {
                memcpy (&NumericItem, pItemValue, NUMERIC_SIZE);
                sprintf (szElement, "%g", NumericItem);
                if(wElement>0) /* if this is the 2nd, 3rd, etc. element */
                {              /* append semicolon-space as a separator */
                    strncat( szItemText, "; ", MAX_ITEM_TEXT-wItemTextLen );
                    wItemTextLen = strlen(szItemText);
                }     
                strncat (szItemText, szElement, MAX_ITEM_TEXT-wItemTextLen);
                wItemTextLen = strlen(szItemText);
                pItemValue += sizeof(NUMBER);   /* step to next NUMBER */
            }
            break;


        case TYPE_TIME: 
            szDataType = "TYPE_TIME";
            memcpy (&TimeItem, pItemValue+DATATYPE_SIZE, TIME_SIZE);
            if (error = ConvertTIMEDATEToText (NULL,NULL,&TimeItem,
                           szItemText,MAXALPHATIMEDATE,&TimeStringLen))
            {
                return (error);
            }
            szItemText[TimeStringLen] = '\0';
            break;

        case TYPE_TIME_RANGE: 
            szDataType = "TYPE_TIME_RANGE";
            szItemText[0] = '\0';
            pItemValue += sizeof(WORD);     /* step over data type word */
            Range = *((RANGE*)pItemValue);  /* pick off the RANGE */
            pItemValue += sizeof(RANGE);    /* step over RANGE header */

            /* Loop over TIMEDATE entries */

            for (wElement=0; wElement<Range.ListEntries; wElement++)
            {
                memcpy (&TimeItem, pItemValue, TIME_SIZE);
                if (error = ConvertTIMEDATEToText (NULL,NULL,&TimeItem,
                               szElement,MAXALPHATIMEDATE,&TimeStringLen))
                {
                    return (error);
                }
                szElement[TimeStringLen] = '\0';
                if(wElement>0) /* if this is the 2nd, 3rd, etc. element */
                {              /* append semicolon-space as a separator */
                    strncat (szItemText, "; ", MAX_ITEM_TEXT-wItemTextLen);
                    wItemTextLen = strlen(szItemText);
                }     
                strncat( szItemText, szElement, MAX_ITEM_TEXT-wItemTextLen );
                wItemTextLen = strlen(szItemText);
            }

            /* Loop over TIMEDATE_PAIR entries */

            for (wElement=0; wElement<Range.RangeEntries; wElement++)
            {
                memcpy (&TimeDatePair, pItemValue, sizeof(TIMEDATE_PAIR));
                if (error = ConvertTIMEDATEToText (NULL,NULL,
                            &(TimeDatePair.Lower), szElement,
                            MAXALPHATIMEDATE, &TimeStringLen))
                {
                    return (error);
                }
                szElement[TimeStringLen] = '\0';
                if (wElement>0)
                {
                    strncat (szItemText, "; ", MAX_ITEM_TEXT-wItemTextLen);
                    wItemTextLen = strlen(szItemText);
                }
                strncat (szItemText, szElement, MAX_ITEM_TEXT-wItemTextLen);
                wItemTextLen = strlen(szItemText);
                strncat (szItemText, " - ", MAX_ITEM_TEXT-wItemTextLen);
                wItemTextLen = strlen(szItemText);
                if (error = ConvertTIMEDATEToText (NULL,NULL,
                            &(TimeDatePair.Upper), szElement,
                            MAXALPHATIMEDATE, &TimeStringLen))
                {
                    return (error);
                }
                szElement[TimeStringLen] = '\0';
                strncat (szItemText, szElement, MAX_ITEM_TEXT-wItemTextLen);
                wItemTextLen = strlen(szItemText);
                pItemValue += sizeof(TIMEDATE_PAIR); /* next TIMEDATE_PAIR */
            }
            break;

        case TYPE_USERID:
            szDataType = "TYPE_USERID";
            szItemText[0] = '\0';
            break;

        default:
            szDataType = "Unrecognized data type";
            szItemText[0] = '\0';
    }

    PRINTLOG("\tType   = %s\n", szDataType);
    PRINTLOG("\tValue  = '%s'\n", szItemText);

    return (NOERROR);
}

/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:     argc, argv - directly from the command line
   
    OUTPUTS:    server_name - server name obtained from command line or
                              from prompt.
                db_name -     database name obtained from command line or
                              from prompt.

*************************************************************************/

void  LNPUBLIC  ProcessArgs (int argc, char *argv[], 
                char *view_names, char *names, char *items, char *servername)
{ 
    if ( (argc != 4) && (argc != 5) ) 
    {
        printf("Enter View names separated by semi-colons: ");      
        fflush (stdout);
        fgets(view_names, MAX_ARG_LEN-1, stdin);
        printf("\n");
        printf ("Enter names separated by semi-colons:  ");
        fflush (stdout);
        fgets(names, MAX_ARG_LEN-1, stdin);
        printf("\n");
        printf ("Enter items separated by semi-colons:  ");
        fflush (stdout);
        fgets(items, MAX_ARG_LEN-1, stdin);
        printf("\n");
        printf ("Enter server name or hit enter:  ");
        fflush (stdout);
        fgets(servername, MAX_ARG_LEN-1, stdin);

    }    
    else
    {
        strncpy(view_names, argv[1], MAX_ARG_LEN-1);    
        strncpy(names, argv[2], MAX_ARG_LEN-1);   
        strncpy(items, argv[3], MAX_ARG_LEN-1);    
        if (argc == 5)
           strncpy(servername, argv[4], MAX_ARG_LEN-1);    
        else
           strcpy(servername, "");        /*local server*/

    } /* end if */
        
} /* ProcessArgs */


