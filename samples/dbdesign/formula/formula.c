/*************************************************************************

    PROGRAM:    formula

    FILE:       formula.c

    PURPOSE:    shows how to call the function NSFComputeEvaluate()
    
    SYNTAX:     formula

    DESCRIPTION:
        This evaluates a formula that returns the value of a number field.

        First, this opens the specified database and creates a new note 
        on which to evaluate the formula. It sets the value of the number 
        field, updates the note, then gets the Note ID and closes the note.

        Then it opens the note using the Note ID. This provides the note 
        handle required for the call to NSFComputeEvaluate later. 
        
        Next it compiles a formula using NSFFormulaCompile. This provides 
        the formula, which is required for input to NSFComputeStart.
        
        Next it calls NSFComputeStart, which provides a compute handle,
        (an HCOMPUTE).  
        
        Finally, it calls NSFComputeEvaluate, passing in all necessary
        parameters. This yields a handle to the results of the formula
        evaluation.
        
        This processes the results of the evaluation, then calls 
        NSFComputeStop, closes the note and closes the database. 

***************************************************************************/ 
#if defined(OS400)
#pragma convert(850)
#endif

/* OS and C include files */

#include <stdio.h>
#include <string.h>

/* HCL C API for Notes/Domino include files */

#include <global.h>
#include <nsfdb.h>
#include <nsfdata.h>
#include <nsfnote.h>
#include <ostime.h>
#include <nsfsearc.h>
#include <osmem.h>
#include <misc.h>
#include <osmisc.h>

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

void PrintAPIError (STATUS);

/************************************************************************

    FUNCTION:   main 

    PURPOSE:    HCL C API for Notes/Domino subroutine 

*************************************************************************/

int main(int argc, char *argv[])
{
/* Local defines. */
 
#define BODY_TEMP  98.6

/*  Local data declarations */

    char        path_name[]="formula.nsf";   /* pathname of database */
    DBHANDLE    hDb;            /* database handle      */
    NOTEHANDLE  hNote;          /* Note handle.         */

    char        szFormula[] = "numberfield";

    char       *pFormula;       /* pointer to compiled formula.    */
    FORMULAHANDLE hFormula;
    WORD        wFormulaLen;

    NOTEID      note_id;        /* Noteid for newly created note. */

    DHANDLE       hResult;        /* Handle to results returned by  */
                                /* NSFComputeEvaluate()           */
    char       *pResult;        /* Pointer to results buffer.     */
    WORD        wResultLen;     /* Length of result buffer.       */

    HCOMPUTE    hCompute;       /* Handle to COMPUTE returned by  */
                                /* NSFComputeStart()              */
    BOOL        bModified;      /* Set to TRUE by NSFComputeEvaluate  */
                                /*  if note modified by formula.      */
    BOOL        bShouldDelete;  /* TRUE if formula indicates that the */
                                /*   note should be deleted.          */

    WORD        wdc;            /* "We Don't Care" - We're not interested 
                                    in some of the info passed back by 
                                    NSFFormulaCompile(), but the syntax
                                    requires several word pointers. We  
                                    pass the address of this word for all 
                                    parameters we are not interested in.
                                 */

    STATUS      sError = NOERROR;     /* error code from API calls */

    WORD        wDataType;
    double      dNumber = BODY_TEMP;
    RANGE      *pRange;
    NUMBER     *pNumber;
    WORD        wEntry;
    char        szTextNum[MAXALPHANUMBER+1];
    WORD        wTextNumLen;


	if (sError = NotesInitExtended (argc, argv))
	{
        printf("\n Unable to initialize Notes.\n");
        return (1);
	}


/*
 *  Open the database. 
 */

    if (sError = NSFDbOpen (path_name, &hDb))
    {
        PrintAPIError (sError);  
        NotesTerm();
        return (1);
    } 


/*
 *  Create a new note.
 */
 
    if (sError = NSFNoteCreate(hDb, &hNote))
    {
        NSFDbClose (hDb);
        PrintAPIError (sError);  
        NotesTerm();
        return (1);
    }

/*
 *  Write a field named FORM to the note -- this field specifies the
 *  default form to use when displaying the note.
 */

    if (sError = NSFItemSetText (hNote, 
                                 "FORM",
                                 "Numbers", 
                                 MAXWORD))
    {
        NSFDbClose (hDb);
        PrintAPIError (sError);  
        NotesTerm();
        return (1);
    }

/*
 *  Set the number field.
 */
 
    if (sError = NSFItemSetNumber(hNote,
                                  "numberfield",
                                  &dNumber))
    {
        NSFDbClose (hDb);
        PrintAPIError (sError);  
        NotesTerm();
        return (1);
    }

/*
 *  Update the note to generate the new Note ID. Before a new note
 *  is updated, it's Note ID is zero.
 */
 
    if (sError = NSFNoteUpdate(hNote, 0))
    {
        NSFDbClose (hDb);
        PrintAPIError (sError);  
        NotesTerm();
        return (1);
    }

/*
 * Get the new note's NOTEID
 */

    NSFNoteGetInfo(hNote, _NOTE_ID, &note_id);

/*
 *  Close the note to prove that we're accessing an on-disk note.
 */

    if (sError = NSFNoteClose(hNote))
    {
        NSFDbClose (hDb);
        PrintAPIError (sError);  
        NotesTerm();
        return (1);
    }

/*
 *  Now reopen the note.
 */
 
    if (sError = NSFNoteOpen(hDb, note_id, OPEN_EXPAND, &hNote))
    {
        NSFDbClose (hDb);
        PrintAPIError (sError);  
        NotesTerm();
        return (1);
    }

/*
 * Compile the formula that will be evaluated.
 */

    if (sError = NSFFormulaCompile(NULL,
                                   0,
                                   szFormula,
                                   (WORD) strlen(szFormula),
                                   &hFormula,
                                   &wFormulaLen,
                                   &wdc, &wdc, &wdc,
                                   &wdc, &wdc))
    {
        NSFDbClose (hDb);
        PrintAPIError (sError);  
        NotesTerm();
        return (1);
    }

/*
 *  Get a pointer to the compiled formula.
 */

    pFormula = (char*) OSLockObject(hFormula);

/*
 *  Call NSFComputeStart to mark start of evaluation
 */
 
    if (sError = NSFComputeStart(0, pFormula, (HCOMPUTE far *) &hCompute))
    {
        OSUnlockObject(hFormula);
        OSMemFree(hFormula);
        NSFDbClose (hDb);
        PrintAPIError (sError);  
        NotesTerm();
        return (1);
    }

/*
 * Evaluate the formula. The result value is stored in a buffer.
 * A handle to this buffer is returned in hResult. We are responsible
 * for freeing this buffer when we are finished processing it.
 */
 
    if (sError = NSFComputeEvaluate(hCompute,
                                    hNote,
                                    (DHANDLE far *) &hResult,
                                    &wResultLen,
                                    0,
                                    (BOOL far *) &bShouldDelete,
                                    (BOOL far *) &bModified))
    {
        OSUnlockObject(hFormula);
        OSMemFree(hFormula);
        NSFDbClose (hDb);
        PrintAPIError (sError);  
        NotesTerm();
        return (1);
    }

/*
 *  Process the result by locking the handle returned in hResult. This
 *  yields a pointer to a buffer containing data.
 */

    pResult = (char*) OSLockObject (hResult);

/*
 *  The format of the data depends on the datatype. The first word of
 *  the buffer identifies the datatype. The formula we evaluated will
 *  yield a TYPE_NUMBER or a TYPE_NUMBER_RANGE.
 *
 *  NOTE: Data returned by NSFComputeEvaluate is usually returned as 
 *  a LIST datatype.  For intstance, when evaluating a formula that 
 *  returns a number, the data actually returned will be of type 
 *  TYPE_NUMBER_RANGE.  API programs that call NSFComputeEvaluate should 
 *  always handle both the simple datatype and the list datatype.
 */    

    wDataType = *(WORD *) pResult; /* Get the datatype of the result. */
    pResult += sizeof(WORD);  /* Set pResult to point to the actual data.*/

    switch (wDataType)
    {
        case TYPE_NUMBER:

            sError = ConvertFLOATToText( NULL, NULL,
                            (NUMBER*)pResult,
                            szTextNum,
                            MAXALPHANUMBER,
                            &wTextNumLen );
            if (ERR(sError))
            {
                printf("Error: unable to convert number to text.\n");
            }
            else
            {
                printf( "The number is: %s\n", szTextNum);
            }
         
            break;

        case TYPE_NUMBER_RANGE:

            pRange = (RANGE *) pResult;
            pNumber = (NUMBER *) &pRange[1];

            for(wEntry = 0; 
                wEntry < pRange->ListEntries; 
                wEntry++, pNumber++)
            {
                if (sError = ConvertFLOATToText( NULL, NULL, 
                                    pNumber, szTextNum,
                                    MAXALPHANUMBER, &wTextNumLen ))
                {
                    printf( "Error: unable to convert number to text.\n" );
                }
                else
                {
                    printf( "The number is: %s\n", szTextNum );
                }
            }
            break;

        case TYPE_INVALID_OR_UNKNOWN:
        default:
            printf( "TYPE_INVALID_OR_UNKNOWN : %x\n", wDataType );
            break;

    }
    
    OSUnlockObject (hResult);
    OSMemFree(hResult);


/*
 *  Unlock the handle to the compiled formula and free the memory.
 */

    OSUnlockObject(hFormula);
    OSMemFree(hFormula);

/*
 *  Call NSFComputeStop to mark the end of the formula evaluation.        
 */
 
    if (sError = NSFComputeStop(hCompute))
    {
        NSFDbClose (hDb);
        PrintAPIError (sError);  
        NotesTerm();
        return (1);
    }

/* Close the database. */

    if (sError = NSFDbClose (hDb))
	{
        PrintAPIError (sError);  
        NotesTerm();
        return (1);
	}

/* End of program. */
    printf("\nProgram completed successfully\n"); 
    NotesTerm();
    return (0); 

}


/*************************************************************************

    FUNCTION:   PrintAPIError

    PURPOSE:    This function prints the HCL C API for Notes/Domino 
		error message associated with an error code.

**************************************************************************/

void PrintAPIError (STATUS api_error)

{
    STATUS  string_id = ERR(api_error);
    char    error_text[200];
    WORD    text_len;

    /* Get the message for this HCL C API for Notes/Domino error code
       from the resource string table. */

    text_len = OSLoadString (NULLHANDLE,
                             string_id,
                             error_text,
                             sizeof(error_text));

    /* Print it. */
    fprintf (stderr, "\n%s\n", error_text);

}
