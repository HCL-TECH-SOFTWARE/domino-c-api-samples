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

    PROGRAM:    summary

    FILE:       summary.c

    SYNTAX:     summary  <db filename>
                          or
                summary

    PURPOSE:    Finds and prints all summary data of each data note in
                a database. Works on any database.

    DESCRIPTION:
        This program uses NSFSearch() to find all data notes in the
        database. The action routine, ReadSummaryData(), reads and
        parses the summary buffer of each note without opening the
        note. It prints out all the summary data.

        ReadSummaryData() does not know, in advance, the names of the
        items in the summary buffer. Therefore, it parses the summary
        buffer explicitly. Other applications that know the item names
        in advance should use NSFGetSummaryValue() or NSFLocateSummaryValue()
        instead.

        Note: #ifdef APITESTMODE is for internal use only.

****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* OS and C include files */

#include <stdio.h>
#include <string.h>
#include <memory.h>

#if defined(OS400) || defined(MAC)
#include <stdlib.h>
#else
#include <malloc.h>
#endif

/* Notes API include files */

#include <global.h>
#include <nsfdb.h>
#include <nsfnote.h>
#include <nsfsearc.h>
#include <nsferr.h>
#include <ods.h>
#include <textlist.h>
#include "../../printLog.h"
#include <osmisc.h>

void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                             char *db_filename);

#define  STRING_LENGTH  256

/* Function prototypes */

STATUS LNPUBLIC ReadSummaryData (     /* called for every document */
                                  VOID far *,
                                  SEARCH_MATCH far *,
                                  ITEM_TABLE far *);
STATUS PrintSummary (char *);
STATUS ExtractTextList (char *, char *);

/* Constants */
/* Notes imposes a 32K max summary buffer total size. Therefore,
   34K will safely store the printable text rendering of any single
   item.
*/
#define  MAX_ITEM_LEN       34816

/* The maximum number of items in the summary buffer of a single note
   is limited to the number of ITEM structures that will fit in 32K.
*/
#define  MAX_ITEMS          32768/sizeof(ITEM)

#define  MAX_ITEM_NAME_LEN  DESIGN_NAME_MAX


/* Global variables */
ITEM    Items[MAX_ITEMS];           /* Stores the array of ITEMs */
char    ItemText[MAX_ITEM_LEN];     /* Text rendering of item value */
char    ItemName[MAX_ITEM_NAME_LEN];/* Zero terminated item name */

/************************************************************************

    FUNCTION:   main

*************************************************************************/

int main(int argc, char *argv[])
{
    char       *db_filename;  /* pathname of source database */
    DBHANDLE    db_handle;    /* handle of source database */
    STATUS      error = NOERROR;   /* return status from API calls */
    char        database_name[STRING_LENGTH];

    db_filename = database_name;
    ProcessArgs(argc, argv, db_filename);

    if (error = NotesInitExtended (argc, argv))
    {
        PRINTLOG("\n Unable to initialize Notes.\n");
        return (1);
    }

    /* Open the database. */

    if (error = NSFDbOpen (db_filename, &db_handle))
    {
        PRINTLOG ("Error: unable to open database '%s'\n", db_filename);
        PRINTERROR (error,"NSFDbOpen");
        NotesTerm();
        return (1);
    }

    /*  Call NSFSearch to find all data notes in the database.
        Specify search flag SEARCH_SUMMARY so that the action
        routine gets passed the summary buffer as input.
    */

    if (error = NSFSearch (
                           db_handle,          /* database handle */
                           NULLHANDLE,         /* selection formula */
                           NULL,               /* title of view in selection formula */
                           SEARCH_SUMMARY,     /* search flags: get summary data! */
                           NOTE_CLASS_DATA,    /* note class to find */
                           NULL,               /* starting date (unused) */
                           ReadSummaryData,    /* action routine for notes found */
                           NULL,               /* argument to action routine */
                           NULL))              /* returned ending date (unused) */

    {
        PRINTLOG ("Error: unable to search database.\n");
        NSFDbClose (db_handle);
        PRINTERROR (error,"NSFSearch");
        NotesTerm();
        return (1);
    }

    /* Close the database. */

    if (error = NSFDbClose (db_handle))
    {
        PRINTLOG ("Error: unable to close database '%s'\n", db_filename);
        PRINTERROR (error,"NSFDbClose");
        NotesTerm();
        return (1);
    }

    /* End of main routine. */
    PRINTLOG("\nProgram completed successfully.\n");
    NotesTerm();
    return (0);
}


/************************************************************************

    FUNCTION:   ReadSummaryData

    PURPOSE:    If the note matches the search criteria, this prints
                the note ID and calls PrintSummary() to print out the
                summary data for the note.

    DESCRIPTION: This is the action routine called by NSFSearch for
                 each note that matches the selection criteria.

    INPUTS:
        optional_param - the optional argument that we supplied when
                         we called NSFSearch. Not used in this example.

        search_info -   supplied by NSFSearch. This is a structure of
                        information about the note that was found.

        summary_info -  The summary buffer for this note.

*************************************************************************/

STATUS LNPUBLIC ReadSummaryData
                               (VOID far *optional_param,
                                SEARCH_MATCH far *search_info,
                                ITEM_TABLE far *summary_info)
{
    SEARCH_MATCH  SearchMatch;
    STATUS        error;

    memcpy ((char*)(&SearchMatch), (char *)search_info, sizeof(SEARCH_MATCH));

    if (!(SearchMatch.SERetFlags & SE_FMATCH))
        return (NOERROR);

    /* Print the note ID. */

    PRINTLOG ("\nNote ID is: %#010lx.\n", SearchMatch.ID.NoteID);

    /* Print the summary data. */

    if (error = PrintSummary( (char*)summary_info ))
        return (error);

    return (NOERROR);
}

/************************************************************************

    FUNCTION:   PrintSummary

    PURPOSE:    Print all the items in a summary buffer.

    INPUTS:
        pSummary - a pointer to the summary buffer for one note.

    DESCRIPTION:
        The information in a summary buffer consists of an ITEM_TABLE
        structure, followed by an array of ITEM structures, followed by
        a packed sequence of field names and item values.

        ITEM_TABLE
        ITEM 1
        ITEM 2
        ...
        ITEM N
        name of item 1
        value of item 1
        name of item 2
        value of item 2
        ...
        name of item N
        value if item N

        Item names are not zero terminated. Each item value starts with
        a data type word. The lengths of the item names and item values
        will vary. The NameLength member of the ITEM structure specifies
        the length of the corresponding name, and the ValueLength member
        of the ITEM structure specifies the length of the item value.
        The ValueLength includes the length of the data type word.

        Note that this summary buffer differs in format from the buffer
        parsed by sample program VIEWSUMM. VIEWSUMM calls NIFReadEntries
        specifying READ_MASK_SUMMARYVALUES, which yields a buffer with
        format specified by ITEM_VALUE_TABLE rather than ITEM_TABLE.

        This function can parse the summary buffer of any data note
        because it does not assume the note contains items with any
        particular names. If you know the item name in advance, use
        either NSFGetSummaryValue() or NSFLocateSummaryValue().

*************************************************************************/


STATUS PrintSummary (char *pSummary)
{
    char       *pSummaryPos;        /* current position in pSummary */
    ITEM_TABLE  ItemTable;          /* header at start of pSummary */
    USHORT      ItemCount;          /* number of items in pSummary */
    USHORT      NameLength;         /* length of item name w/out terminator*/
    USHORT      ValueLength;        /* length of item value, incl. type */
    WORD        DataType;           /* item data type word */
    char       *szDataType;         /* printable data type name */
    USHORT      TextLen;            /* length of printable item text */
    NUMBER      NumericItem;        /* an item of type TYPE_NUMBER */
    NUMBER_PAIR NumberPair;         /* part of item of TYPE_NUMBER_RANGE */
    RANGE       Range;              /* part of item of TYPE_NUMBER_RANGE */
    TIMEDATE    TimeItem;           /* a time/date item */
    TIMEDATE_PAIR   TimePairItem;   /* part of time/date list or range */
    WORD        TimeStringLen;      /* length of ASCII time/date */
    STATUS      error;              /* return code from API calls */
    USHORT      i;                  /* counter for loop over items */
    USHORT      j;                  /* " " " " multi-valued items */
    char        Text[] = "TYPE_TEXT";
    char        Text_List[] = "TYPE_TEXT_LIST";
    char        Number[] = "TYPE_NUMBER";
    char        Number_Range[] = "TYPE_NUMBER_RANGE";
    char        Time[] = "TYPE_TIME";
    char        Time_Range[] = "TYPE_TIME_RANGE";
    char        Unrecognized[] = "Unrecognized";

   /* Initialize pSummaryPos to the position of the beginning of
      the summary buffer. Keep pSummary unmodified. Modify pSummaryPos.
    */

    pSummaryPos = pSummary;

   /* Copy the ITEM_TABLE header at the beginning of the summary buffer
      to a local variable. Advance pSummaryPos to point to the next
      byte in the summary buffer after the ITEM_TABLE.
    */
    memcpy ((char*)(&ItemTable), pSummaryPos, sizeof(ITEM_TABLE));
    pSummaryPos += sizeof(ItemTable);

   /* pSummaryPos now points to the first ITEM in an array of ITEM
      structures. Copy this array of ITEM structures into the global
      Items[] array.
    */

    ItemCount = ItemTable.Items;

    for (i=0; i < ItemCount; i++)
    {
        memcpy((char*)(&Items[i]), pSummaryPos, sizeof(ITEM));
        pSummaryPos += sizeof(ITEM);
    }

   /* pSummaryPos now points to the first item name. Loop over each
      item, copying the item name into the ItemName variable and
      converting the item value to printable text in ItemText.
    */

    for (i=0; i < ItemCount; i++)
    {
       /* zero out two global arrays that we re-use for each item */
        memset (ItemText, 0, MAX_ITEM_LEN);
        memset (ItemName, 0, MAX_ITEM_NAME_LEN);

        NameLength = Items[i].NameLength;
        memcpy (ItemName, pSummaryPos, NameLength);
        ItemName[NameLength] = '\0';
        pSummaryPos += NameLength;

        /*  Handle the case of a zero length item  */

        if (Items[i].ValueLength == 0)
        {
            strcpy(ItemName, "  *  ");
            ItemText[0] = '\0';
            goto PrintItem;
        }

        /* pSummaryPos now points to the item value. First get the
           data type. Then step over the data type word to the data
           value and convert the value to printable text. Store the
           text in ItemText.
        */

        memcpy ((char*)(&DataType), pSummaryPos, sizeof(WORD));
        pSummaryPos += sizeof(WORD);

        ValueLength = Items[i].ValueLength - sizeof(WORD);

        /* The way to extract an item depends on the data type. */

        switch (DataType)
        {
            case TYPE_TEXT:
                szDataType = Text;
                memcpy (ItemText, pSummaryPos, ValueLength);
                ItemText[ValueLength] = '\0';
                pSummaryPos += ValueLength;
                break;

            case TYPE_TEXT_LIST:
                szDataType = Text_List;
                if (error = ExtractTextList (pSummaryPos, ItemText))
                {
                    PRINTLOG ("Error: unable to extract text list.\n");
                    ItemText[0] = '\0';
                }
                pSummaryPos += ValueLength;
                break;

            case TYPE_NUMBER:
                szDataType = Number;
                memcpy ((char *)(&NumericItem), pSummaryPos, sizeof(NUMBER));
                  sprintf (ItemText, "%f", NumericItem);
                pSummaryPos += sizeof(NUMBER);
                break;

            case TYPE_NUMBER_RANGE:
                szDataType = Number_Range;
                TextLen = 0;
                memcpy ((char*)(&Range), pSummaryPos, sizeof(RANGE));
                pSummaryPos += sizeof(RANGE);
                for (j=0; j < Range.ListEntries; j++)
                {
                    memcpy ((char*)(&NumericItem), pSummaryPos,
                            sizeof(NUMBER));
                    sprintf (ItemText+TextLen, "%f ", NumericItem);
                    pSummaryPos += sizeof(NUMBER);
                    TextLen = strlen(ItemText);
                }
                if ((Range.ListEntries != 0) && (Range.RangeEntries != 0))
                {
                    memcpy (ItemText+TextLen, "; ", 2);
                    TextLen += 2;
                }
                for (j=0; j < Range.RangeEntries; j++)
                {
                    memcpy ((char*)(&NumberPair), pSummaryPos,
                            sizeof(NUMBER_PAIR));
                    sprintf (ItemText+TextLen, "%f - %f ", NumberPair.Lower,
                             NumberPair.Upper);
                    pSummaryPos += sizeof(NUMBER_PAIR);
                    TextLen = strlen(ItemText);
                }
                break;

#ifndef APITESTMODE

            case TYPE_TIME:
                szDataType = Time;
                memcpy ((char*)(&TimeItem), pSummaryPos, sizeof(TIMEDATE));
                pSummaryPos += sizeof(TIMEDATE);
                if (error = ConvertTIMEDATEToText (
                            NULL, NULL, &TimeItem,
                            ItemText, MAXALPHATIMEDATE,
                            &TimeStringLen))
                {
                    PRINTLOG ("Error: unable to convert TIMEDATE to text.\n");
                    TimeStringLen = 0;
                }
                ItemText[TimeStringLen] = '\0';
                break;

            case TYPE_TIME_RANGE:
                szDataType = Time_Range;
                TextLen = 0;
                memcpy ((char*)(&Range), pSummaryPos, sizeof(RANGE));
                pSummaryPos += sizeof(RANGE);
                for (j=0; j < Range.ListEntries; j++)
                {
                    if (j != 0)
                    {
                        memcpy (ItemText+TextLen, ",", 1);
                        TextLen++;
                    }
                    memcpy ((char*)(&TimeItem), pSummaryPos,
                            sizeof(TIMEDATE));
                    pSummaryPos += sizeof(TIMEDATE);
                    if (error = ConvertTIMEDATEToText (
                                                       NULL, NULL,
                                                       &TimeItem,
                                                       ItemText+TextLen,
                                                       MAXALPHATIMEDATE, &TimeStringLen))
                    {
                        PRINTLOG("Error: unable to convert TIMEDATE to text.\n");
                        TimeStringLen = 0;
                    }
                    TextLen += TimeStringLen;
                }

                if (Range.RangeEntries == 0)
                {
                    ItemText[TextLen] = '\0';
                    break;
                }

               /* Append a space, semicolon, space to separate time/dates
                  from time/date ranges.
                */

                memcpy (ItemText+TextLen, " ; ", 3);
                TextLen += 3;

                for (j=0; j < Range.RangeEntries; j++)

                {
                    if (j != 0)
                    {
                        memcpy (ItemText+TextLen, ",", 1);
                        TextLen++;
                    }
                    memcpy ((char*)&TimePairItem, pSummaryPos,
                            sizeof(TIMEDATE_PAIR));
                    pSummaryPos += sizeof(TIMEDATE_PAIR);
                    if (error = ConvertTIMEDATEToText (
                                                       NULL, NULL,
                                                       &TimePairItem.Lower, ItemText+TextLen,
                                                       MAXALPHATIMEDATE, &TimeStringLen))
                    {
                        PRINTLOG("Error: unable to convert TIMEDATE to text.\n");
                        TimeStringLen = 0;
                    }
                    TextLen += TimeStringLen;
                    memcpy (ItemText+TextLen, "-", 1);
                    TextLen++;
                    if (error = ConvertTIMEDATEToText (
                                                       NULL, NULL,
                                                       &TimePairItem.Upper, ItemText+TextLen,
                                                       MAXALPHATIMEDATE, &TimeStringLen))
                    {
                        PRINTLOG("Error: unable to convert TIMEDATE to text.\n");
                        TimeStringLen = 0;
                    }
                    TextLen += TimeStringLen;
                }
                ItemText[TextLen] = '\0'; /* zero terminate the text */
                break;

#endif
            /* Item is not one of the data types this program handles. */

            default:
                szDataType = Unrecognized;
                ItemText[0] = '\0';
                pSummaryPos += ValueLength;
                break;
        } /* end of switch on data type */

PrintItem:
        /* Print the item name, data type, and value. */
        PRINTLOG ("\tItem Name = %s\n", ItemName);
        PRINTLOG ("\tData Type = %s\n", szDataType);

#ifndef APITESTMODE
        PRINTLOG ("\tItem Value = \"%s\"\n\n", ItemText);
#else
        if (strcmp(ItemName,"$UpdatedBy")==0)
        {
            PRINTLOG ("\tItem Value =\n\n", ItemText);
        }
        else
        {
            PRINTLOG ("\tItem Value = \"%s\"\n\n", ItemText);
        }
#endif

    }/* End of loop over items in the summary. */

    /* Print final line feed to end display for this note. */

    PRINTLOG ("\n");

    return (NOERROR);
}

/************************************************************************

    FUNCTION:   ExtractTextList

    PURPOSE:    This function extracts an item of TYPE_TEXT_LIST from a
                pSummary buffer.

    INPUTS:     char *pBuffer

    OUTPUTS:    char *TextList

*************************************************************************/

STATUS ExtractTextList (char *pBuffer, char *TextList)
{
    LIST    List;
    char   *ListEntry;  /* pointer to list entry */
    WORD    TextLen;    /* total length of string output to TextList */
    WORD    EntryLen;   /* length of one entry */
    STATUS  error;      /* return code from API calls */
    USHORT  i;          /* a counter */

    /* Initialize the total length of the list. */

    TextLen = 0;

    /* Clear the string that we'll fill up. */

    TextList[0] = '\0';

    /* Get the list header structure with the number of entries. */

    memcpy ((char*)(&List), pBuffer, sizeof(LIST));

    /* Loop over each entry of the list. */

    for (i=0; i<List.ListEntries; i++)
    {
        /* Get one entry from the list. */

        if (error = ListGetText (
                                 pBuffer,
                                 FALSE, /* DataType not prepended to list */
                                 i,
                                 &ListEntry,
                                 &EntryLen))
        {
            return (ERR(error));
        }

        /*  Copy this entry to the string we are building and move the
            pointer that keeps track of how much we have in the string.
         */

        memcpy (TextList+TextLen, ListEntry, EntryLen);
        TextLen += EntryLen;

        /* Put a comma after the entry and advance the text length. */

        memcpy (TextList+TextLen, ",", 1);
        TextLen++;

    }   /* End of loop over list entries. */

    /* Put a null in place of the last comma */

    TextLen--;
    memcpy (TextList+TextLen, "\0", 1);

    return (NOERROR);
}


/************************************************************************

    FUNCTION:   ProcessArgs

    INPUTS:     argc, argv - directly from the command line

    OUTPUTS:    db_filename get data from the command line or from what
                the user types at a prompt

*************************************************************************/

void  LNPUBLIC  ProcessArgs (int argc, char *argv[], char *db_filename)
{
    if ( argc !=2 )
    {
        printf("Enter name of database: ");
        fflush(stdout);
        gets(db_filename);
    }else
    {
        strcpy(db_filename, argv[1]);
    } /* end if */

} /* ProcessArgs */
#ifdef __cplusplus
}
#endif
