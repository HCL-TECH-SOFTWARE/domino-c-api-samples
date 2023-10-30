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

    FILE:    win16.c

    PROGRAM: iedit

    PURPOSE: Windows 3.X - specific routines. This file is only compiled
             and linked when building _iedit.dll for MS Windows.
    
    FUNCTIONS:
        IeditMessageBox
        IeditFileOpenForRead
        IeditFileCreate
        IeditFileOpenForWrite
        IeditFileWrite
        IeditFileRead
        IeditFileSeek
        IeditFileClose
        IeditFileDelete
        MapDosError
        LibMain             Standard Windows 3.X DLL Entry.
        WEP                 Standard Windows 3.X DLL Exit.

****************************************************************************/

/*
 *  Windows header file. Include only if compiling for Windows.
 */

#if (defined(DOS) && defined(W)) || (defined(NT))
#include <windows.h>
#endif

/*
 * C header files
 */ 

#include <io.h>
#include <sys\types.h>
#include <sys\stat.h>

/*
 *  Notes header files
 */
 
#include <global.h>
#include <globerr.h>
#include <oserr.h>
#include <osmisc.h>

/*
 *  Local header files.
 */
 
#include "iedit.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/* Function Prototypes */
STATUS  MapDosError(WORD wDosCode);


/************************************************************************

    FUNCTION:   IeditMessageBox

    PURPOSE:    Display a Notes error in a windows message box.

    INPUTS:     error  - Notes error code
                szMessage - application - specific message.

*************************************************************************/

VOID LNPUBLIC IeditMessageBox( char * szMessage, STATUS error  )
{
    char    szErrStr[ERR_STR_LEN+1];
    WORD    wLen;

    if (szMessage != (char*)NULL)
    {
        lstrcpy( szErrStr, szMessage );
    }
    if (error)
    {
        lstrcat( szErrStr, " - \n" );
        wLen = lstrlen( szErrStr );
        OSLoadString ( NULLHANDLE, ERR(error), szErrStr+wLen,
                       (WORD)(ERR_STR_LEN-wLen) );
    }
    MessageBox( GetFocus(), szErrStr, "IEDIT Edit Level Import Sample",
                MB_ICONHAND | MB_OK );
}


/************************************************************************

    FUNCTION:   IeditFileOpenForRead

    PURPOSE:    Open the file to be imported into Notes for read access.

    INPUTS:     FileName    - fully expanded path and file name

    OUTPUTS:    *phRetFile  - gets the MS-DOS file handle to the open file.

    RETURNS:    NOERROR or status code appropriate for the error

*************************************************************************/

STATUS LNPUBLIC IeditFileOpenForRead( char * FileName, int * phRetFile )
{
    HFILE       hFile;
    OFSTRUCT    ImpOfStruct;

    hFile = OpenFile( FileName, &ImpOfStruct, OF_READ);

    if(hFile == HFILE_ERROR)
    {
        return (MapDosError(ImpOfStruct.nErrCode));
    }
    *phRetFile = hFile;
    return NOERROR;
}


/************************************************************************

    FUNCTION:   IeditFileCreate

    PURPOSE:    Create the file in preparation for writing to it.

    INPUTS:     FileName    - fully expanded path and file name

    OUTPUTS:    *phRetFile  - gets the file handle to the open file

    RETURNS:    NOERROR or status code appropriate for the error

*************************************************************************/

STATUS LNPUBLIC IeditFileCreate( char * FileName, int * phRetFile )
{
    HFILE       hFile;
    OFSTRUCT    ImpOfStruct;

    hFile = OpenFile( FileName, &ImpOfStruct, OF_CREATE );

    if(hFile == HFILE_ERROR)
    {
        return (MapDosError(ImpOfStruct.nErrCode));
    }
    *phRetFile = hFile;
    return NOERROR;
}

/************************************************************************

    FUNCTION:   IeditFileOpenForWrite

    PURPOSE:    Open a file for write access and seek to the end
                in preparation for appending data to it.

    INPUTS:     FileName    - fully expanded path and file name

    OUTPUTS:    *phRetFile  - gets the file handle to the open file

    RETURNS:    NOERROR or status code appropriate for the error

*************************************************************************/

STATUS LNPUBLIC IeditFileOpenForWrite( char * FileName, int * phRetFile )
{
    HFILE       hFile;
    OFSTRUCT    ofStruct;

    hFile = OpenFile( FileName, &ofStruct, OF_WRITE );

    if(hFile == HFILE_ERROR)
    {
        return (MapDosError(ofStruct.nErrCode));
    }

    _llseek( hFile, 0L, SEEK_END );

    *phRetFile = hFile;
    return NOERROR;
}



/************************************************************************

    FUNCTION:   IeditFileWrite

    PURPOSE:    Write specified number of bytes to a file.

    INPUTS:     int   hFile    - MS-DOS file handle
                char *pData    - pointer to data to write
                WORD  wCount   - number of bytes of data to write

    RETURNS:    NOERROR or status code appropriate for the error

*************************************************************************/

STATUS LNPUBLIC IeditFileWrite( int hFile, char *pData, WORD wCount )
{
    if (HFILE_ERROR == _lwrite(hFile, pData, (UINT) wCount))
    {
        return ERR_IOERROR;
    }
    return NOERROR;
}

/************************************************************************

    FUNCTION:   IeditFileRead

    PURPOSE:    Read the specified number of bytes from a file.

    INPUTS:     int   hFile    - MS-DOS file handle
                char *pData    - pointer to buffer to receive data
                WORD  wCount   - number of bytes of data to read
                WORD *pwRead   - gets number of bytes actually read

    RETURNS:    NOERROR if successfully read dwCount bytes from file
                ERR_IEDIT_EOF if End of File encountered, or
                ERR_IOERROR if any other read error occurs

*************************************************************************/

STATUS LNPUBLIC IeditFileRead( int hFile, char *pData,
                                 WORD wCount,
                                 WORD *pwRead )
{
    UINT    wRead;

    *pwRead = (WORD)0;

    wRead = _lread (hFile, pData, (UINT)wCount);

    if (wRead == (UINT)wCount)
    {
        *pwRead = wCount;
        return NOERROR;
    }
    if (wRead < (UINT)wCount)
    {
        *pwRead = (WORD)wRead;
        return ERR_IEDIT_EOF;
    }
    /* else if (wRead == HFILE_ERROR) */

    return ERR_IOERROR;
}


/************************************************************************

    FUNCTION:   IeditFileSeek

    PURPOSE:    Seek to the specified position relative to the start
                of a file.

    INPUTS:     int   hFile    - MS-DOS file handle
                DWORD dwPos    - Offset in the file

    RETURNS:    NOERROR

*************************************************************************/

STATUS LNPUBLIC IeditFileSeek( int hFile, DWORD dwPos )
{
    _llseek( hFile, dwPos, SEEK_SET);
    return NOERROR;
}


/************************************************************************

    FUNCTION:   IeditFileClose

    PURPOSE:    Close the specified file.

    INPUTS:     int   hFile    - MS-DOS file handle

    RETURNS:    NOERROR

*************************************************************************/

STATUS LNPUBLIC IeditFileClose( int hFile )
{
    _lclose( hFile );
    return NOERROR;
}


/************************************************************************

    FUNCTION:   IeditFileDelete

    PURPOSE:    Delete the specified file.

    INPUTS:     FileName    - fully expanded path and file name

    RETURNS:    NOERROR

*************************************************************************/

STATUS LNPUBLIC IeditFileDelete( char * FileName )
{
    HFILE       hFile;
    OFSTRUCT    ofStruct;

    hFile = OpenFile( FileName, &ofStruct, OF_DELETE );

    if(hFile == HFILE_ERROR)
    {
        return (MapDosError(ofStruct.nErrCode));
    }

    return NOERROR;
}

/************************************************************************

    FUNCTION:   MapDosError

    PURPOSE:    For DOS/Windows only: map MS-DOS extended error codes
                to Notes error codes.

*************************************************************************/

STATUS MapDosError(WORD wDosCode)
{
    STATUS  usNotesErr = NOERROR;

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

/*============================================================================
            W I N D O W S  S P E C I F I C  F U N C T I O N S 
============================================================================*/
/*
* LibMain and WEP only need to be defined under Windows.
*/
 
#if defined(W) && defined(DOS)

/************************************************************************

    FUNCTION:   LibMain

    PURPOSE:    Standard Windows 3.X DLL entry point.

*************************************************************************/

int LNPUBLIC  LibMain(DHANDLE hInstance, WORD wDataSeg,
                        WORD cbHeapSize, char far *lpszCmdLine)
{
    if (cbHeapSize != 0)
        UnlockData(0);
    return (1);
}


/************************************************************************

    FUNCTION:   WEP

    PURPOSE:    Standard Windows DLL exit procedure

*************************************************************************/
int    LNPUBLIC  WEP(int nParameter)
{

    /* System shutdown in progress. Respond accordingly */

    if (nParameter == WEP_SYSTEM_EXIT)
        return (1);

    /* DLL use count is zero.  Every appl. that had loaded */
    /* the DLL has freed it.                               */

    else if(nParameter == WEP_FREE_DLL)
        return (1);

    /* Undefined value. IGNORE! */

    else
        return (1);

}
#endif
