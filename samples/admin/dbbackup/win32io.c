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

 PROGRAM:   dbbackup/dbrecs

 FILE:      win32io.c

 PURPOSE:   Facilitate Windows 32-bit file I/O and print api error text.

 FUNCTIONS: SysFileOpenRead
            SysFileCreate
            SysFileWrite
            SysFileRead
            SysFileSeek
            SysFileClose
            SysFileDelete
            SysFileCreateDirectory
            SysFileCopy
            MapDosError
            print_api_error

****************************************************************************/

/* Windows header file. Include only if compiling for Windows. */
#if (defined(DOS) && defined(W)) || (defined(NT))
#include <windows.h>
#endif

/* C header files */
#include <stdio.h>
#ifdef LINUX
#include <libio.h>
#include <sys/types.h>
#include <sys/stat.h>
#else
#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>
#endif


/* Notes header files */
#include <global.h>
#include <globerr.h>
#include <oserr.h>
#include <osmisc.h>
#include <names.h>

/* Local header files */
#include "win32io.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/* Function Prototypes */
STATUS  MapDosError(DWORD wDosCode);

char    error_text[200];


STATUS LNPUBLIC SysFileOpenRead( char * FileName, int * phRetFile )
/************************************************************************

    FUNCTION:   SysFileOpenRead

    PURPOSE:    Open the file to be imported into Notes for read access.

    INPUTS:     FileName    - fully expanded path and file name

    OUTPUTS:    *phRetFile  - gets the MS-DOS file handle to the open file.

    RETURNS:    NOERROR or status code appropriate for the error

*************************************************************************/
{
    //HFILE   hFile;
	HANDLE   hFile;
	
    DWORD   error;
    char    NFileName[MAXPATH];

int a;
DHANDLE b;
HFILE c;
printf("int: %d\n",sizeof(a));
printf("handle: %d\n",sizeof(hFile));
printf("dhandle: %d\n",sizeof(b));
printf("HFILE: %d\n",sizeof(c));
printf("HFILE_ERROR = : %d\n",HFILE_ERROR);

    OSTranslate (OS_TRANSLATE_LMBCS_TO_NATIVE,
                 FileName,
                 MAXWORD,
                 NFileName,
                 sizeof(NFileName) );

    hFile = CreateFile( NFileName,
                                GENERIC_READ,
                                FILE_SHARE_READ|FILE_SHARE_WRITE,
                                NULL,
                                OPEN_EXISTING,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);

    if(hFile == INVALID_HANDLE_VALUE)
    {
        error = GetLastError();
        return (MapDosError(error));
    }
    *phRetFile = HANDLE_TO_DHANDLE(hFile);
    return NOERROR;
}

STATUS LNPUBLIC SysFileCreate( char * FileName, int * phRetFile )
/************************************************************************

    FUNCTION:   SysFileCreate

    PURPOSE:    Create the file in preparation for writing to it.

    INPUTS:     FileName    - fully expanded path and file name

    OUTPUTS:    phRetFile  - gets the file handle to the open file

    RETURNS:    NOERROR or status code appropriate for the error

*************************************************************************/
{
    //HFILE   hFile;
	HANDLE   hFile;
    DHANDLE  nHandle = NULL;
    DWORD   error;
    char    NFileName[MAXPATH];

    OSTranslate (OS_TRANSLATE_LMBCS_TO_NATIVE,
                 FileName,
                 MAXWORD,
                 NFileName,
                 sizeof(NFileName) );

    hFile = CreateFile( NFileName,
                                GENERIC_READ|GENERIC_WRITE,
                                FILE_SHARE_READ|FILE_SHARE_WRITE,
                                NULL,
                                OPEN_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                NULL);

    if(hFile == INVALID_HANDLE_VALUE)
    {
        error = GetLastError();
        return (MapDosError(error));
    }
    *phRetFile = HANDLE_TO_DHANDLE(hFile);
    return NOERROR;
}

STATUS LNPUBLIC SysFileWrite( int hFile, char *pData, DWORD wCount )
/************************************************************************

    FUNCTION:   SysFileWrite

    PURPOSE:    Write specified number of bytes to a file.

    INPUTS:     hFile  - MS-DOS file handle
                pData  - pointer to data to write
                wCount - number of bytes of data to write

    RETURNS:    NOERROR or status code appropriate for the error

*************************************************************************/
{
    DWORD   WrittenBytes;
    DWORD   error = NOERROR;

    if ( !WriteFile(DHANDLE_TO_HANDLE(hFile), pData, wCount, &WrittenBytes, NULL) )
    {
        error = GetLastError();
        return (MapDosError(error));
    }
    else
    {
       if (!error && wCount != WrittenBytes)
       {
           return 1;
       }
    }
    return NOERROR;
}

STATUS LNPUBLIC SysFileRead( int hFile, char *pData, DWORD wCount)
/************************************************************************

    FUNCTION:   SysFileRead

    PURPOSE:    Read the specified number of bytes from a file.

    INPUTS:     hFile  - MS-DOS file handle
                pData  - pointer to buffer to receive data
                wCount - number of bytes of data to read

    RETURNS:    NOERROR if successfully read dwCount bytes from file

*************************************************************************/
{
    DWORD   ReadBytes;
    DWORD   error = NOERROR;

    if ( !ReadFile(DHANDLE_TO_HANDLE(hFile), pData, wCount, &ReadBytes, NULL) )
    {
        error = GetLastError();
        return (MapDosError(error));
    }
    else
    {
       if (!error && wCount != ReadBytes)
       {
           return 1;
       }
    }
    return NOERROR;
}

STATUS LNPUBLIC SysFileSeek( int hFile, DWORD dwPos, DWORD Loc )
/************************************************************************

    FUNCTION:   SysFileSeek

    PURPOSE:    Seek to the specified position relative to the start
                or end of a file.

    INPUTS:     hFile - MS-DOS file handle
                dwPos - Offset in the file
                Loc   - File pointer location (START or END)

    RETURNS:    NOERROR

*************************************************************************/
{
    DWORD   HighDW=0;
    DWORD   error;

    if(Loc)
       Loc = FILE_END;
    else
       Loc = FILE_BEGIN;

    if( SetFilePointer( DHANDLE_TO_HANDLE(hFile), dwPos, &HighDW, Loc) == MAXWORD)
    {
        error = GetLastError();
        return (MapDosError(error));
    }
    return NOERROR;
}

STATUS LNPUBLIC SysFileClose( int hFile )
/************************************************************************

    FUNCTION:   SysFileClose

    PURPOSE:    Close the specified file.

    INPUTS:     hFile - MS-DOS file handle

    RETURNS:    NOERROR

*************************************************************************/
{
    DWORD   error;

    if (CloseHandle (DHANDLE_TO_HANDLE(hFile)))
        return NOERROR;
    error = GetLastError();
    return (MapDosError(error));
}

STATUS LNPUBLIC SysFileDelete( char * FileName )
/************************************************************************

    FUNCTION:   SysFileDelete

    PURPOSE:    Delete the specified file.

    INPUTS:     FileName - fully expanded path and file name

    RETURNS:    NOERROR

*************************************************************************/
{
    DWORD   error;
    char    NFileName[MAXPATH];

    OSTranslate (OS_TRANSLATE_LMBCS_TO_NATIVE,
                 FileName,
                 MAXWORD,
                 NFileName,
                 sizeof(NFileName) );

    if( DeleteFile (NFileName) )
    {
        error = GetLastError();
        return (MapDosError(error));
    }
    else
        return NOERROR;
}

STATUS LNPUBLIC SysFileCreateDirectory ( char * lFile )
/************************************************************************

    FUNCTION:   SysFileCreateDirectory

    PURPOSE:    Create a directory.

    INPUTS:     lFile - file

    OUTPUTS:

    RETURNS:    NOERROR or status code appropriate for the error

*************************************************************************/
{
    char    NlFile[MAXPATH];
    DWORD   error;

    OSTranslate (OS_TRANSLATE_LMBCS_TO_NATIVE,
                 lFile,
                 MAXWORD,
                 NlFile,
                 sizeof(NlFile) );

    if (error = CreateDirectory(NlFile, NULL))
        return NOERROR;
    else
        error = GetLastError();
        if (error == 183)
           return NOERROR;
        return (1);
}

STATUS LNPUBLIC SysFileCopy ( char * Source, char * Target, BOOL exist )
/************************************************************************

    FUNCTION:   SysFileCopy

    PURPOSE:    Copy a file.

    INPUTS:     Source - pointer to a name of an existing file
                Target - pointer to a name of a file to copy to
                exist  - flag for operatoin if fileexists

    OUTPUTS:

    RETURNS:    TRUE if function succeeds
                FALSE if function fails

*************************************************************************/
{
    BOOL   success;

    success = CopyFile(Source, Target, exist);
    return success;
}

STATUS MapDosError(DWORD wDosCode)
/************************************************************************

    FUNCTION:   MapDosError

    PURPOSE:    For DOS/Windows only: map MS-DOS extended error codes
                to Notes error codes.

*************************************************************************/
{
    STATUS   usNotesErr = NOERROR;

    switch (wDosCode)
    {
        case   0x02:  usNotesErr = ERR_NOEXIST;                 break;
        case   0x03:  usNotesErr = ERR_NO_MORE_FILES;           break;
        case   0x04:  usNotesErr = ERR_FILES;                   break;
        case   0x05:  usNotesErr = ERR_LOCK;                    break;
        case   0x06:  usNotesErr = ERR_FD;                      break;
        case   0x0F:  usNotesErr = ERR_INVALID_DRIVE;           break;
        case   0x13:  usNotesErr = ERR_PROTECTED;               break;
        case   0x1D:  usNotesErr = ERR_IOERROR;                 break;
        case   0x1E:  usNotesErr = ERR_IOERROR;                 break;
        case   0x20:  usNotesErr = ERR_LOCK; /* same as 0x05 */ break;
        case   0x21:  usNotesErr = ERR_LOCKS;                   break;
        default:      usNotesErr = ERR_UNKNOWN;                 break;
    }
    return(usNotesErr);
}

char * print_api_error (STATUS api_error)
/************************************************************************

    FUNCTION:   print_api_error

    PURPOSE:    This function prints the API error message associated
                with an error   code.

*************************************************************************/
{
    STATUS   string_id = ERR(api_error);
    WORD     text_len;

    /* Get the text for this API error code from the resource string table. */
    text_len = OSLoadString (NULLHANDLE,
                             string_id,
                             error_text,
                             sizeof(error_text));

    /* Print error text. */
    fprintf (stderr, "\n %s\n", error_text);
    return error_text;
}
