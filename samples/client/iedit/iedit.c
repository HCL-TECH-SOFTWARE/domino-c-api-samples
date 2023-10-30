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

    FILE:    iedit.c

    PROGRAM: iedit

    PURPOSE: Custom edit-level import library - platform independent code
    
    DESCRIPTION:

    Iedit is an example edit level import library. Install iedit into
    the Domino and Notes User interface by copying the executable file into 
    the Domino and Notes     program directory, and adding the appropriate 
    EDITIMP line to the notes.ini file. Then, from Domino and Notes, open a 
    document for editing, move the cursor to a rich text field, and issue the 
    File Import command from the Domino and Notes user interface menu. In the 
    File Import dialog box, select "Sample ASCII Import" from the the list of 
    formats available. Domino and Notes will transfer control to this progam. 
    This program imports one or more ASCII text files into the rich text field 
    of the document currently being edited.
    
    You may need a custom import library similar to this sample program
    if none of the import modules provided with Domino and Notes support the 
    file format you require.  You may enhance this program to handle the
    particular file format you require.

    FUNCTIONS:

    MainEntryPoint()-   Import text into current Domino and Notes 
                        TYPE_COMPOSITE item.

****************************************************************************/

/* HCL C API for Notes/Domino includes */

#include <global.h>
#include <osmem.h>
#include <oserr.h>
#include <ixedit.h>
#include <ixport.h>
#include <easycd.h>

/* Local includes */

#include "iedit.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/* global variables */
char  buffer[IEDIT_BUFFSZ];  /* for copying data from CD file to temp file */



/************************************************************************

    FUNCTION:   MainEntryPoint

    PURPOSE:    Domino and Notes calls this function when user selects 
	            this filter.

    INPUTS:      
        EditorData = Pointer to data structure, initialized by Domino and 
                     Notes, that provides the name of output file this 
                     function should create and the FONTID of text at the 
                     caret position in the rich text field. See ixedit.h.
        Flags      = Import flags (see ixport.h)
        hModule    = handle to this module, if Windows. Not used if UNIX.
        AltLibraryName = Ignored here
        FileName   = name of file to import (fully expanded path name)

    OUTPUTS:     
        The Output File specfied in EditorData block is filled with CD
        records loadable by the editor.

    RETURNS:   
        NOERROR or STATUS if an error condition is encountered.

    DISCUSSION:
        Domino and Notes loads this library and calls this function, 
        MainEntryPoint, when the Domino and Notes user has selected this 
        file format in the File Import dialog box. This function must 
        create one output file, also known as a temporary file, with the 
        name specified by the OutputFileName member of the EditorData data 
        structure. This temporary file must containing the imported data in 
        Compound Document (rich text) format. After this function has returned 
        NOERROR, Domino and Notes reads the contents of this temporary file 
        into the rich text field being edited by the user. Then Domino and Notes 
        deletes the temporary file.

        If the user selects more than one file to import in the File Import
        dialog box, then Domino and Notes will call this function multiple times, 
        once on each imput file. The import flags parameter tells this function
        whether it is processing the First or Last input file.
        
        This function must conform to the syntax defined by the IXENTRYPROC
        typedef in HCL C API for Notes/Domino header file ixport.h. Under 
        Unix, this function MUST be named MainEntryPoint. Under OS/2 or Windows,
        this function may have any name, but must be the ordinally first
        function exported by this library.

    ALGORITHM:
        If processing the first input file,
            create the output file
        Else
            open output file and seek to the end
        Create Compound Text Context
        Initialize and define Compound Text style
        Add text from input file to Compound Text Context
        Close Compound Text Context (creates memory buffer or CD file)
        If CD text in memory buffer
        {
            if not first input file
                skip data type word in memory buffer
            write memory buffer to output file
        }
        Else (CD text in CD file)
        {
            open CD file
            if not first input file
                seek past datatype word in CD file
            copy data from CD file to output file
            close CD file
            delete CD file
        }
        Close output file

*************************************************************************/

STATUS LNPUBLIC  MainEntryPoint(EDITIMPORTDATA *EditorData,WORD Flags,
                                HMODULE hModule, char *AltLibraryName,
                                char *FileName)
{
    STATUS          error, rerror;
    DHANDLE           hCompound;  /* handle to Compound Text context */
    COMPOUNDSTYLE   Style;
    DWORD           dwStyleID;
    int             hOutputFile, hCDTextFile;
    DHANDLE           hReturnCD;
    DWORD           dwReturnCDLen;
    WORD            wRead;
    char            achReturnCDFileName[MAXPATH];
    char           *pCDText;

    /* If processing the first input file, create the output file. */

    if (Flags & IXFLAG_FIRST)
    {
        if (error = IeditFileCreate( EditorData->OutputFileName,
                                    &hOutputFile ))
        {
            IeditMessageBox("Unable to create temporary file", error);
            goto Exit0;
        }
    }
    else /* Else output file already exists. Open it and seek to the end. */
    {
        if (error = IeditFileOpenForWrite( EditorData->OutputFileName,
                                    &hOutputFile ))
        {
            IeditMessageBox ("Unable to open temporary file", error);
            goto Exit0;
        }
    }

    /* Create a stand-alone Compound Text Context */

    if (error = CompoundTextCreate( NULLHANDLE, NULL, &hCompound ))
    {
        IeditMessageBox("Unable to create compound text context", error);
        goto Exit1;
    }

    /* Initialize and define the Compound Text Style */

    CompoundTextInitStyle( &Style );
    if (error = CompoundTextDefineStyle( hCompound, "", &Style,
                                         &dwStyleID ))
    {
        IeditMessageBox ("Unable to define compound text style", error);
        CompoundTextDiscard( hCompound );
        goto Exit1;
    }

    /* Add text from the input file to the Compound Text Context */

    if (error = CompoundTextAddTextExt (
                                        hCompound,          /* context */
                                        dwStyleID,          /* style  */
                                        EditorData->FontID, /* font ID */
                                        FileName,           /* name of text file to read */
                                        0,                  /* not used if COMP_FROM_FILE */
                                        "\r\n",             /* newline delimiter */
                                        COMP_FROM_FILE |    /* take input from file */
                                        COMP_PRESERVE_LINES,/* preserve line breaks */
                                        /* don't specify any paragraph directive: 1K each */
                                        NULLHANDLE))        /* CLS translation table */
    {
        IeditMessageBox ("Unable to add text from file to CD Context", error);
        CompoundTextDiscard (hCompound);
        goto Exit1;
    }

   /* Close Compound Text Context. This store the resulting Compound Text
      data in a memory buffer, if it fits. Otherwise, this creates a CD
      file. The Compound Text data consists of a data type word (which
      will be TYPE_COMPOSITE) followed by a series of CD records. If this
      CD text is in a memory buffer, write the memory buffer to the output
      file. Else if the CD text is in a CD file, copy the data from the
      CD file to output file.
    */
    if (error = CompoundTextClose( hCompound,  /* context to close */
                                   &hReturnCD, /* if it fits in memory */
                                   &dwReturnCDLen,
                                   achReturnCDFileName, /* else in file */
                                   MAXPATH-1 ))
    {
        IeditMessageBox ("Unable to close compound text context", error);
        CompoundTextDiscard (hCompound);
        goto Exit1;
    }

    if (hReturnCD != NULLHANDLE)      /* the CD text fit into memory */
    {
        pCDText = (char *)OSLockObject( hReturnCD ) ;

       /* if not processing the first input file, skip over the data
          type word in memory buffer.
        */
        if (!(Flags & IXFLAG_FIRST))
        {
            pCDText += sizeof(WORD);
            dwReturnCDLen -= sizeof(WORD);
        }

        /* Write memory buffer to output file */

        wRead = (WORD)dwReturnCDLen;
        error = IeditFileWrite( hOutputFile, pCDText, wRead );

        /* unlock and free the hReturnCD buffer */
        OSUnlockObject( hReturnCD );
        OSMemFree( hReturnCD );

        if (error)
        {
            IeditMessageBox ("Unable to write to temporary file", error);
            goto Exit1;
        }
    }
    else /* assimilated rich text is in the named file. */
    {
        /* open the CD file */
        if (error = IeditFileOpenForRead( achReturnCDFileName,
                    &hCDTextFile ))
        {
            IeditMessageBox ("Unable to open CD file to read", error);
            goto Exit1;
        }
       /* if not processing the first input file, seek past the
          datatype word in CD file.
        */
        if (!(Flags & IXFLAG_FIRST))
        {
            if (error = IeditFileSeek( hCDTextFile, sizeof(WORD) ))
            {
                IeditMessageBox ("Error seeking past first word of CD file",
                                 error);
                goto Exit2;
            }
        }

        /* copy data from CD file to output file */
              
        while (TRUE)
        {
            rerror = IeditFileRead(hCDTextFile, buffer, IEDIT_BUFFSZ, &wRead);
            if (rerror == ERR_IOERROR)
            {
                IeditMessageBox ("Error reading from CD file", error=rerror);
                goto Exit2;
            }

            if (wRead)
            {
                if (error = IeditFileWrite( hOutputFile, buffer, wRead))
                {
                    IeditMessageBox("Error writing to temporary file",error);
                    goto Exit2;
                }
            }
            if (rerror == ERR_IEDIT_EOF)
            {
                break;
            }
        }

Exit2:
        /* close CD file */
        IeditFileClose( hCDTextFile );

        /* delete CD file */
        IeditFileDelete( achReturnCDFileName );
    }

Exit1:
    /* close the output file */
    IeditFileClose( hOutputFile ) ;

Exit0:
    return (ERR(error));
}

