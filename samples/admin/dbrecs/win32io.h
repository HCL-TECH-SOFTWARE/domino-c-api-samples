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

 PROGRAM: dbbackup/dbrecs

 FILE:    win32io.h

 PURPOSE: Symbolic constants and function prototypes.

****************************************************************************/

/* Function prototypes */
STATUS LNPUBLIC SysFileOpenRead( char * FileName, int * phRetFile );
STATUS LNPUBLIC SysFileCreate( char * FileName, int * phRetFile );
STATUS LNPUBLIC SysFileWrite( int hFile, char *pData, DWORD wCount );
STATUS LNPUBLIC SysFileRead( int hFile, char *pData, DWORD wCount);
STATUS LNPUBLIC SysFileSeek( int hFile, DWORD dwPos, DWORD Loc );
STATUS LNPUBLIC SysFileClose( int hFile );
STATUS LNPUBLIC SysFileDelete( char * FileName );
STATUS LNPUBLIC SysFileCreateDirectory ( char * lFile );
STATUS LNPUBLIC SysFileCopy ( char * Source, char * Target, BOOL exist );
char * print_api_error (STATUS api_error);
