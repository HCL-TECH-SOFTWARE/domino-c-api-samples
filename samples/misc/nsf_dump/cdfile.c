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

    FILE:       cdfile.c

    DESCRIPTION:
      This file contains functions DumpCDFile, NSFDUMPEnumCDFile, and
      other code related to dumping a CD file.  A CD file contains data in 
	  Domino and Notes rich text format. A CD file starts with a data type
      word which must be either TYPE_COMPOSITE or TYPE_TEXT.  The data 
      type word is followed by a series of CD records. Each CD record
      must start on an even byte boundary. The data type word is in host-
      specific format. The rest of the CD file is in Canonical format.

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

/* nsf_dump - specific include file */
#include "nsf_dump.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/************************************************************************

    FUNCTION:   DumpCDFile

*************************************************************************/

STATUS LNPUBLIC DumpCDFile( char * szFilename )
{
	STATUS      Error;
	CDFILEHANDLE hCDFile;
	DWORD		dwFileSize;

	if (!(Error = NSFDUMPOpenFile( szFilename, &hCDFile ) ))
	{
		if (!(Error = NSFDUMPGetFileSize( hCDFile, &dwFileSize ) ))
		{
			if (!(Error = NSFDUMPSetFilePosition( hCDFile, 0L ) ))
			{
				Error = NSFDUMPEnumCDFile( hCDFile, dwFileSize, 0L,
					DumpOneCDRecord, (char*)NULL );
			}
		}

		NSFDUMPCloseFile( hCDFile );
	}

	return (Error);
}


/************************************************************************

    FUNCTION:   DumpCDStorageFile

	A CD storage file contains:
		WORD		Number of segments in the file (n)
		n WORDs		Length of each segment
		n segments

	Each segment contains:
		WORD		Item type (TYPE_COMPOSITE)
		CD records

*************************************************************************/

STATUS LNPUBLIC DumpCDStorageFile( char * szFilename )
{
	STATUS			Error;
	CDFILEHANDLE	hCDFile;
	DWORD			dwFileSize;
	DWORD			dwFileOffset;
	NSF_DUMP_CD_FILE_SEGMENT_TABLE *	pTable;
	int				i;

	if (!(Error = NSFDUMPOpenFile( szFilename, &hCDFile ) ))
	{
		if (!(Error = NSFDUMPGetFileSize( hCDFile, &dwFileSize ) ))
		{
				/* Read the segment table at the start of the file */
			if (!(Error = NSFDUMPReadSegmentTable (hCDFile, &pTable, &dwFileOffset) ))
			{
					/* Dump the individual segments */
				for (i = 0;
					(i < pTable->segmentCount) && (dwFileOffset < dwFileSize) && (!Error);
					i++)
				{
					fprintf (dumpfile, "\nDumping segment #%d - length=%d\n",
						i, pTable->segmentLengths[i]);
					Error = NSFDUMPEnumCDFile( hCDFile, (DWORD) (pTable->segmentLengths[i]),
						dwFileOffset, DumpOneCDRecord, (char *) NULL );
					dwFileOffset += (DWORD) (pTable->segmentLengths[i]);
				}

				if (((NSF_DUMP_CD_FILE_SEGMENT_TABLE *) NULL) != pTable)
					free (pTable);
			}
		}

		NSFDUMPCloseFile( hCDFile );
	}

	return (Error);
}

/************************************************************************

    FUNCTION:   NSFDUMPOpenFile

    INPUTS:
        szFilename - fully qualified path of CD file to open

    OUTPUTS:      
        phCDFile - gets handle to open file containing CD records

*************************************************************************/

STATUS LNPUBLIC NSFDUMPOpenFile(
	char * szFilename, 
	CDFILEHANDLE * phCDFile )
{
   STATUS      Error = NOERROR;

		/* Open binary file for reading */
	*phCDFile = fopen (szFilename, "rb");
    if (NULL != *phCDFile)   /* success */
    {
        Error = NOERROR;
    }
    else                        /* failure */
    {
        Error = ERR_UNKNOWN;
        perror( "nsf_dump " );
    }

    if (Error)
    {
        fprintf(stderr, "Unable to open CD file '%s'\n", szFilename);
    }
    return (Error);
}


/************************************************************************

    FUNCTION:   NSFDUMPCloseFile

*************************************************************************/

VOID LNPUBLIC NSFDUMPCloseFile( CDFILEHANDLE hCDFile )
{
	fclose (hCDFile);
}


/************************************************************************

    FUNCTION:   NSFDUMPEnumCDFile

    PURPOSE:    Call action routine on each CD record in a CD file.

    DESCRIPTION:   

    This routine scans a CD file and calls the specified action routine
    once for every CD record encountered in the file.  The syntax
    of the action routine is the same as the action routine for 
    EnumCompositBuffer(). This routine stops scanning the CD buffer 
    when the action routine returns a non-zero error code, namely an 
    error has occurred, or, done processing status (ERR_ODS_ENUM_COMPLETE).

    Important Note: Under OS/2, the action routine called by this routine
    must be exported in the .def file for DS to be set up properly
    for action routine (if action routine uses DS values).

    INPUTS:      
        hf              - handle to file containing CD records
        ActionRoutine   - Address of subroutine to call for each CD record.
        Context         - contextual data passed as input to action routine

    RETURNS:   
        (Routine)       - Error status

*************************************************************************/

#define INITIAL_SIZE 4096

STATUS LNPUBLIC NSFDUMPEnumCDFile(
	CDFILEHANDLE		hCDFile,
	DWORD				dwFileSize,
	DWORD				dwFileOffset,  /* offset to 1st CD record - past datatype WORD */
	ActionRoutinePtr	ActionRoutine, /* STATUS (LNCALLBACKPTR ActionRoutine)(char *, WORD, DWORD, void*); */
	char *				Context
) {
    STATUS  Error;
    WORD    Datatype;
    DWORD   CurrentBufferSize;
    char   *pRecBuf;
    DHANDLE  hRecBuf;

    /* Read the first WORD of file - it is the datatype word */
    if (Error = NSFDUMPReadFromFile( hCDFile, (char far *) &Datatype, 
                                    sizeof(WORD)) )
        return (Error);

    dwFileSize -= sizeof(WORD);
    dwFileOffset += sizeof(WORD);

    /* Note:  if the CD file is built on a host whose file format 
       differs from the one running nsf_dump, then the bytes
       for Datatype will need to be swapped.  For example,
       if the CD file was built on an Intel platform in which
       the bytes are ordered from least significant to most
       significant, and if this program is running on a
       platform, such as UNIX, where the bytes are ordered
       from most significant to least significant, then
	   the bytes have to be reversed in the type word.
     */
 
    if ((TYPE_COMPOSITE != Datatype)
		&& (TYPE_TEXT != Datatype)
		&& (TYPE_COMPOSITE != NSFDUMPByteSwap (Datatype))
		&& (TYPE_TEXT != NSFDUMPByteSwap (Datatype)))
    {
        fprintf( stderr, "Error: first word of CD file not recognized: 0x%X.\n",
                 Datatype );
        return (ERR_DATATYPE);
    }
    
    /* Allocate a buffer to read CD records into */

    if (Error = OSMemAlloc( MEM_SHARE | MEM_GROWABLE,
                            (DWORD) INITIAL_SIZE, &hRecBuf ) )
    {
        fprintf( stderr, "Error: unable to allocate %d bytes memory.\n",
        INITIAL_SIZE );
        return (Error);
    }

    CurrentBufferSize = INITIAL_SIZE;

    /* Read each CD record into a buffer and pass it to the action routine */

    while (dwFileSize)
    {
        WORD    RecordTypeCanonical, RecordTypeHost;
        WORD    wLengthCanonical, wLengthHost, FixedSize;
        void   *p;
        DWORD   dwLengthCanonical, dwLengthHost;

        /*  In order to get the record length, we must first get the signature
            byte. Read the first word of the CD record. Since this is in 
            canonical format, convert it to Host format. Then examine the 
            high-order byte. This byte -- the signature byte -- contains a 
            code which indicates whether the record length is stored in a 
            long word, a word, or a byte. Code LONGRECORDLENGTH = record 
            starts with an LSIG and the length is stored in a DWORD.
            Code WORDRECORDLENGTH = record starts with a WSIG and the 
            length is stored in a WORD. Otherwise, CD record starts with 
            a BSIG and the code byte is, itself, the length.
         */

        if (Error = NSFDUMPReadFromFile( hCDFile, (char far *)&RecordTypeCanonical, 
                  sizeof(WORD) ))
    {
            break;
    }
    
        p = (void*)&RecordTypeCanonical;
    
        ODSReadMemory( &p, _WORD, &RecordTypeHost, 1 );

        switch (RecordTypeHost & 0xFF00)
        {
            case LONGRECORDLENGTH:      /* LSIG */
                if (Error = NSFDUMPReadFromFile(hCDFile, 
                     (char far *) &dwLengthCanonical, 
                     sizeof(DWORD)))
                {
                    break;
                }

        p = (void*)&dwLengthCanonical;
    
        ODSReadMemory( &p, _DWORD, &dwLengthHost, 1 );
    
                FixedSize = sizeof(LSIG);

                break;
            
            case WORDRECORDLENGTH:      /* WSIG */
                if (Error = NSFDUMPReadFromFile(hCDFile, 
                     (char far *) &wLengthCanonical, 
                     sizeof(WORD)))
                {
                    break;
                }
              
                p = (void *)&wLengthCanonical;
        
        ODSReadMemory( &p, _WORD, &wLengthHost, 1 );
        
                dwLengthHost = (DWORD) wLengthHost;
        
                FixedSize = sizeof(WSIG);

                break;

            default:                    /* BSIG */
                dwLengthHost = (DWORD) ((RecordTypeHost >> 8) & 0x00FF);
                RecordTypeHost &= 0x00FF; /* Length not part of signature */
                FixedSize = sizeof(BSIG);
        }

        if (Error)
            break;
        
        
       /* See if the data in the CD record will fit into our buffer. If
          not, realloc it.
        */

        if (dwLengthHost > CurrentBufferSize)
        {
            if (Error = OSMemRealloc( hRecBuf, dwLengthHost ))
                break;

            CurrentBufferSize = dwLengthHost;
        }

        pRecBuf = (char *) OSLockObject( hRecBuf );

        /* Seek backwards to beginning of record */

        if (Error = NSFDUMPSetFilePosition( hCDFile, dwFileOffset ))
    {
        OSUnlock(hRecBuf);
    }

        /* Read the CD record into the buffer */

        if (Error = NSFDUMPReadFromFile( hCDFile, pRecBuf, dwLengthHost ))
        {
            OSUnlock(hRecBuf);
            break;
        }

        /*  Call the action routine with the pointer to the record, 
            its type, and its length. Stop iterating if action routine 
            returns an Error. 
         */

        Error = (*ActionRoutine)(pRecBuf,RecordTypeHost,dwLengthHost,NULL);
    
        OSUnlock(hRecBuf);

        if (Error)
            break;

        if (dwLengthHost)
        {
            dwFileSize -= dwLengthHost;
            dwFileOffset += dwLengthHost;
        }
        else
        {
            dwFileSize -= FixedSize;
            dwFileOffset += FixedSize;
        }        

        /* If we are at an odd file offset, ignore the filler byte */

        if ((dwFileOffset & 1L) && (dwFileSize > 0) )
        {
            char   Filler;

            if (Error = NSFDUMPReadFromFile( hCDFile, &Filler, 1L ))
                break;
            dwFileSize -= 1;            
            dwFileOffset += 1;
        }

    } /* end while */


    OSMemFree(hRecBuf);

    return (Error);
}

/************************************************************************

    FUNCTION:   NSFDUMPGetFileSize

*************************************************************************/

STATUS LNPUBLIC NSFDUMPGetFileSize (CDFILEHANDLE hCDFile, DWORD *rtnSize)
{
    STATUS	err;
    long	pos;
    long	fileSize;

	*rtnSize = 0L;

		/* Get the current file position */
	pos = ftell (hCDFile);
	if (pos == -1L)
		err = ERR_NOEXIST;
	else
	{
			/* Move to the end of the file */
		if (0 != fseek (hCDFile, 0L, SEEK_END))
			err = ERR_NOEXIST;
		else
		{
				/* Get the file size */
			fileSize = ftell (hCDFile);
			if (-1L == fileSize)
			{
				err = ERR_NOEXIST;
			}
			else
			{
				*rtnSize = (DWORD) fileSize;
				err = NOERROR;
			}
					/* Put the file position back where it was */
			fseek (hCDFile, pos, SEEK_SET);
		}
	}

	return (err);
}

/************************************************************************

    FUNCTION:   NSFDUMPReadFromFile

    ALGORITHM:  Reads specified no. bytes from file in 32K chunks.

    INPUTS:      
        hCDFile      Open file handle
        Length       number of bytes to read

    OUTPUTS:     
        char * Buffer   gets data from file

*************************************************************************/

STATUS LNPUBLIC NSFDUMPReadFromFile (CDFILEHANDLE hCDFile, 
                char far *Buffer, 
                unsigned long Length)
{
    DWORD    bytesRead;

		/* Read bytes from the file */
	bytesRead = fread (Buffer, 1, (WORD) Length, hCDFile);
	if (bytesRead == Length)
		return (NOERROR);
	else
        return (ERR_TRUNC);
}

/************************************************************************

    FUNCTION:   NSFDUMPSetFilePosition

*************************************************************************/

STATUS LNPUBLIC NSFDUMPSetFilePosition( CDFILEHANDLE hCDFile, DWORD dwPos )
{
	if (fseek (hCDFile, (long) dwPos, SEEK_SET))
		return (ERR_UNKNOWN);
	else
		return (NOERROR);
}

/************************************************************************

    FUNCTION:   NSFDUMPByteSwap

	ALGORITHM:	Just perform a byte-swap on the 2-byte argument
				and return the result.

*************************************************************************/

WORD NSFDUMPByteSwap (
	WORD	input
) {
	return (((input & 0x00FF) << 8) | ((input & 0xFF00) >> 8));
}

/************************************************************************

    FUNCTION:   NSFDUMPReadSegmentTable

    INPUTS:
        hCDFile - Handle of open file

    OUTPUTS:      
		ppTable - Address where table pointer should be stored
		pOffset - Address where file offset should be stored

*************************************************************************/

STATUS LNPUBLIC NSFDUMPReadSegmentTable (
	CDFILEHANDLE	hCDFile,
	NSF_DUMP_CD_FILE_SEGMENT_TABLE ** ppTable,
	DWORD *			pOffset
) {
	STATUS			Error;
	WORD			dataBuf;
	WORD			numSegments;
	NSF_DUMP_CD_FILE_SEGMENT_TABLE *	pTable;
	char *			pBuffer;
	int				i;

	*ppTable = (NSF_DUMP_CD_FILE_SEGMENT_TABLE *) NULL;
	*pOffset = 0L;

	if (Error = NSFDUMPReadFromFile (hCDFile, (char far *) &dataBuf, sizeof(WORD)) )
        return (Error);

	pBuffer = (char *) &dataBuf;
	ODSReadMemory (&pBuffer, _WORD, &numSegments, 1);

	if (0 == numSegments)
	{
        fprintf( stderr, "Error: first word of CD file not recognized: 0x%X.\n",
                 dataBuf );
        return (ERR_DATATYPE);
	}

		/* Allocate the segment table */
	pTable = (NSF_DUMP_CD_FILE_SEGMENT_TABLE *) malloc ((1 + numSegments) * sizeof (WORD));
	if (((NSF_DUMP_CD_FILE_SEGMENT_TABLE *) NULL) == pTable)
		return (ERR_MEMORY);

	pTable->segmentCount = numSegments;

	*pOffset = ODSLength (_WORD);

	for (i = 0; (i < numSegments) && (!Error); i++)
	{
		if (!(Error = NSFDUMPReadFromFile (hCDFile, (char far *) &dataBuf, sizeof(WORD)) ))
		{
			pBuffer = (char *) &dataBuf;
			ODSReadMemory (&pBuffer, _WORD, &(pTable->segmentLengths[i]), 1);
			*pOffset += ODSLength (_WORD);
		}
	}

	if (Error)
		free (pTable);
	else
		*ppTable = pTable;

	return (Error);
}

#ifdef __cplusplus
}
#endif

