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
    PROGRAM: iedit

    FILE:    iedit.h

    PURPOSE: Symbolic constants and function prototypes.

****************************************************************************/

/* Symbolic constants */
#define ERR_STR_LEN                 127
#define IEDIT_BUFFSZ                1024
#define ERR_IEDIT_EOF               PKG_ADDIN+1

/* Function prototypes */
VOID   LNPUBLIC IeditMessageBox( char * szMessage, STATUS error );
STATUS LNPUBLIC IeditFileOpenForRead( char * FileName, int * phRetFile );
STATUS LNPUBLIC IeditFileCreate( char * FileName, int * phRetFile );
STATUS LNPUBLIC IeditFileOpenForWrite( char * FileName, int * phRetFile );
STATUS LNPUBLIC IeditFileWrite( int hFile, char *pData, WORD wCount );
STATUS LNPUBLIC IeditFileRead( int hFile, char *pData, WORD wCount, WORD *pwRead );
STATUS LNPUBLIC IeditFileSeek( int hFile, DWORD dwPos );
STATUS LNPUBLIC IeditFileClose( int hFile );
STATUS LNPUBLIC IeditFileDelete( char * FileName );

