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

    PROGRAM:    nsf_dump

    FILE:       mime.c

    DESCRIPTION:
      This file contains functions DumpMimePart, DumpRFC822Item
      and PrintItemString.
****************************************************************************/
#if defined(OS400)
#pragma convert(850)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>

/* HCL C API for Notes/Domino include files */
#include <global.h>
#include <nsf.h>
#include <osmem.h>
#include <ods.h>                        /* LONGRECORDLENGTH, etc. */
#include <odserr.h>                     /* ERR_DATATYPE */
#include <oserr.h>                      /* ERR_UNKNOWN, etc. */
#include <fontid.h>
#include <mimeods.h>

/* nsf_dump - specific include file */
#include "nsf_dump.h"

/*
 *  MimePartTypeTable - used by GetLiteralString()
 */
char szMIME_PART_PROLOG[]        = "MIME_PART_PROLOG";
char szMIME_PART_BODY[]          = "MIME_PART_BODY";
char szMIME_PART_EPILOG[]        = "MIME_PART_EPILOG";
char szMIME_PART_RETRIEVE_INFO[] = "MIME_PART_RETRIEVE_INFO";
char szMIME_PART_MESSAGE[]       = "MIME_PART_MESSAGE";
char szUNKNOWN_MIME_PART_TYPE[]   = "UNKNOWN MIME PART TYPE";


LITERAL_NAME_ENTRY MimePartTypeTable [] =
{
   {MIME_PART_PROLOG,          szMIME_PART_PROLOG},
   {MIME_PART_BODY,            szMIME_PART_BODY},
   {MIME_PART_EPILOG,          szMIME_PART_EPILOG},
   {MIME_PART_RETRIEVE_INFO,   szMIME_PART_RETRIEVE_INFO},
   {MIME_PART_MESSAGE,         szMIME_PART_MESSAGE},

      /* This must be the last entry! */
   {LITERAL_TABLE_END,         szUNKNOWN_MIME_PART_TYPE}
};


/************************************************************************

    FUNCTION:   DumpMimePart

************************************************************************/

void    LNPUBLIC  DumpMimePart (char *  pData, DWORD length)
{
   char *   pMessage;
   char *   p = pData;
   char *   pPart;
   MIME_PART  MimePartStruct;
   long int nChars = 0;  /* How many characters have been written out? */

   fprintf (dumpfile, "    MIME_PART\n");

   ODSReadMemory ( &p, _MIME_PART, &MimePartStruct, 1);

   fprintf (dumpfile, "      wVersion = %u\n",
      MimePartStruct.wVersion);
   fprintf (dumpfile, "      dwFlags (0x%lX) = ",
      MimePartStruct.dwFlags);
   if (MIME_PART_HAS_BOUNDARY & MimePartStruct.dwFlags)
      fprintf (dumpfile, "MIME_PART_HAS_BOUNDARY  ");
   if (MIME_PART_HAS_HEADERS & MimePartStruct.dwFlags)
      fprintf (dumpfile, "MIME_PART_HAS_HEADERS  ");
   if (MIME_PART_BODY_IN_DBOBJECT & MimePartStruct.dwFlags)
      fprintf (dumpfile, "MIME_PART_BODY_IN_DBOBJECT  ");
   if (MIME_PART_SHARED_DBOBJECT & MimePartStruct.dwFlags)
    fprintf (dumpfile, "MIME_PART_SHARED_DBOBJECT  ");
   if (MIME_PART_SKIP_FOR_CONVERSION & MimePartStruct.dwFlags)
      fprintf (dumpfile, "MIME_PART_SKIP_FOR_CONVERSION  ");
   fprintf (dumpfile, "\n");

   pMessage = GetLiteralString (MimePartTypeTable, MimePartStruct.cPartType);
   fprintf (dumpfile, "      cPartType (%u) = %s\n",
      MimePartStruct.cPartType, pMessage);

   fprintf (dumpfile, "      cSpare = %u\n",
      MimePartStruct.cSpare);

   fprintf (dumpfile, "      wByteCount = %u\n",
      MimePartStruct.wByteCount);
   fprintf (dumpfile, "      wBoundaryLen = %u\n",
      MimePartStruct.wBoundaryLen);
   fprintf (dumpfile, "      wHeadersLen = %u\n",
      MimePartStruct.wHeadersLen);
   fprintf (dumpfile, "      wSpare = %u\n",
      MimePartStruct.wSpare);
   fprintf (dumpfile, "      dwSpare = %u\n",
      MimePartStruct.dwSpare);

   pPart = p;

   if (MimePartStruct.wByteCount != 0)
   {
      fprintf (dumpfile, "      part-data:    ");
      PrintItemString(pPart, MimePartStruct.wByteCount);
   }
   else
      fprintf (dumpfile, "      no part-data\n");

   return;
}


/************************************************************************

    FUNCTION:   DumpRFC822Item

************************************************************************/

void    LNPUBLIC  DumpRFC822Item (char *  pData, DWORD length)
{
   char *   p = pData;
   char *   pPart;
   RFC822ITEMDESC  RFC822ItemStruct;
   long int nChars = 0;  /* How many characters have been written out? */

   fprintf (dumpfile, "    RFC822ITEMDESC\n");

   ODSReadMemory ( &p, _RFC822ITEMDESC, &RFC822ItemStruct, 1);

   fprintf (dumpfile, "      wVersion = %u\n",
      RFC822ItemStruct.wVersion);

   fprintf (dumpfile, "      dwFlags (0x%lX) = ",
      RFC822ItemStruct.dwFlags);

   if ((RFC822_ITEM_FORMAT_MASK & RFC822ItemStruct.dwFlags) == RFC822_ITEM_FORMAT_ADDR)
      fprintf (dumpfile, "RFC822_ITEM_FORMAT_ADDR  ");
   if ((RFC822_ITEM_FORMAT_MASK & RFC822ItemStruct.dwFlags) == RFC822_ITEM_FORMAT_DATE)
      fprintf (dumpfile, "RFC822_ITEM_FORMAT_DATE  ");
   if ((RFC822_ITEM_FORMAT_MASK & RFC822ItemStruct.dwFlags) == RFC822_ITEM_FORMAT_TEXT)
      fprintf (dumpfile, "RFC822_ITEM_FORMAT_TEXT  ");
   if (RFC822_ITEM_STORAGE_STRICT & RFC822ItemStruct.dwFlags)
      fprintf (dumpfile, "RFC822_ITEM_STORAGE_STRICT  ");
   if (RFC822_ITEM_TEXT_LIST & RFC822ItemStruct.dwFlags)
      fprintf (dumpfile, "RFC822_ITEM_TEXT_LIST  ");
   if (RFC822_TEXT_UNUSED & RFC822ItemStruct.dwFlags)
      fprintf (dumpfile, "RFC822_TEXT_UNUSED  ");
   fprintf (dumpfile, "\n");


   fprintf (dumpfile, "      wNotesNativeLen = %u\n",
      RFC822ItemStruct.wNotesNativeLen);
   fprintf (dumpfile, "      w822NameLen  = %u\n",
      RFC822ItemStruct.w822NameLen);
   fprintf (dumpfile, "      w822DelimLen = %u\n",
      RFC822ItemStruct.w822DelimLen);
   fprintf (dumpfile, "      w822BodyLen  = %u\n",
      RFC822ItemStruct.w822BodyLen);

   pPart = p;

   fprintf (dumpfile, "      NotesNative = ");
   PrintItemString(pPart, RFC822ItemStruct.wNotesNativeLen);

   pPart += RFC822ItemStruct.wNotesNativeLen;

   fprintf (dumpfile, "      822Name     = ");
   PrintItemString(pPart, RFC822ItemStruct.w822NameLen);

   pPart += RFC822ItemStruct.w822NameLen;

   fprintf (dumpfile, "      822Delim    = ");
   PrintItemString(pPart, RFC822ItemStruct.w822DelimLen);

   pPart += RFC822ItemStruct.w822DelimLen;

   fprintf (dumpfile, "      822Body     = ");
   PrintItemString(pPart, RFC822ItemStruct.w822BodyLen);

   return;
}

void    LNPUBLIC  PrintItemString (char * pText, WORD iText)
{
   long int nChars = 0;  /* How many characters have been written out? */

   if (iText != 0)
   {
      fprintf (dumpfile, "\"");

      while (iText--)
      {

       /* Print the next character, quoting and hexing where necessary */
         unsigned char  c = *pText++;

         if (c == '"')
         {
            fprintf (dumpfile, "\\\"");
            nChars += 2;
         }
         else if (c == '\t')
         {
            fprintf (dumpfile, "\\t");
            nChars += 2;
         }
         else if (c == '\n')
         {
            fprintf (dumpfile, "\\n");
            /* Force a newline by lying about the number of characters */
            nChars = 99999;
         }
         else if (c == '\r')
         {
            fprintf (dumpfile, "\\r");
            nChars += 2;
         }
         else if (c == '\0')
         {
            fprintf (dumpfile, "\\0");
            nChars += 2;
         }
         else if (c == '\\')
         {
            fprintf (dumpfile, "\\\\");
            nChars += 2;
         }
         else if ((c < 32) || (c >= 127))
         {
            fprintf (dumpfile, "\\x%02x", c);
            nChars += 4;
         }
         else
         {
            fprintf (dumpfile, "%c", c);
            nChars++;
         }

         if (nChars > 80)
         {
            fprintf (dumpfile, "\"\n                    \"");

            nChars = 0;
         }
      }
      fprintf (dumpfile, "\"\n");
   }
   return;
}

#ifdef __cplusplus
}
#endif