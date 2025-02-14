/****************************************************************************
 *
 * Copyright HCL Technologies 1996, 2025.
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

    PROGRAM: RUNMACRO.EXE

    FILE:    RUNMACRO.H

****************************************************************************/


#define ERR_RUNMACRO_WRONGTYPE   (PKG_ADDIN + 1)
#define ERR_RUNMACRO_ZEROLENGTH  (PKG_ADDIN + 2)
#define ERR_RUNMACRO_BADSCAN     (PKG_ADDIN + 3)
#define ERR_RUNMACRO_EMPTYCOLL   (PKG_ADDIN + 4)
#define ERR_RUNMACRO_NONEWNOTES  (PKG_ADDIN + 5)
#define ERR_RUNMACRO_UNSUPPORTED (PKG_ADDIN + 6)
#define ERR_RUNMACRO_NOTTHISMACHINE (PKG_ADDIN + 7)
#define ERR_RUNMACRO_BADOBJECTTYPE (PKG_ADDIN + 8)

#define ERR_RUNMACRO_MIN   ERR_RUNMACRO_WRONGTYPE
#define ERR_RUNMACRO_MAX   ERR_RUNMACRO_BADOBJECTTYPE

/* function prototypes */
STATUS LNPUBLIC GetMacroInfo(NOTEHANDLE,WORD*,WORD*,WORD*,
                                 BLOCKID*,BLOCKID*,BOOL*);
STATUS LNPUBLIC MacroIsSupported(NOTEHANDLE,char*,WORD,WORD,WORD,BOOL);
STATUS LNPUBLIC RunMacroOnThisMachine(NOTEHANDLE,WORD);
STATUS LNPUBLIC GetTableOfIDsToProcess(DBHANDLE,NOTEHANDLE,WORD,DHANDLE);
STATUS LNPUBLIC GetIDsOfAllDocsInDb(DBHANDLE,DHANDLE);
STATUS LNPUBLIC GetIDsOfAllDocsInView(DBHANDLE,DHANDLE);
STATUS LNPUBLIC GetIDsOfAllNewDocs(DBHANDLE,NOTEHANDLE,DHANDLE);
STATUS LNPUBLIC UpdateLeftToDo(char*,NOTEID,DHANDLE,DHANDLE);
STATUS LNPUBLIC ReadLeftToDoObject(DBHANDLE,NOTEHANDLE,
                  OBJECT_DESCRIPTOR*,DHANDLE*,TIMEDATE*,WORD*);
STATUS LNPUBLIC MergeNoteIDs(void*,DWORD);
STATUS LNPUBLIC AddIDUnique(void*,SEARCH_MATCH*,ITEM_TABLE*);
STATUS LNPUBLIC GetNoteID(void*,SEARCH_MATCH*,ITEM_TABLE*);
STATUS LNPUBLIC RemoveNoteID(void*,DWORD);
STATUS LNPUBLIC CreateIDTables(DHANDLE*,DHANDLE*,DHANDLE*);
STATUS LNPUBLIC ProcessOneNote(DBHANDLE,NOTEHANDLE,NOTEID,
                  WORD,WORD,WORD,BOOL,HCOMPUTE,BOOL,HCOMPUTE,DHANDLE,DHANDLE);
STATUS LNPUBLIC SetMacroMachineName(NOTEHANDLE,WORD);

