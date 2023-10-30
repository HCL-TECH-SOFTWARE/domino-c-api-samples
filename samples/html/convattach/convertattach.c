/*
* Copyright HCL Technologies 1996, 2023.
*
* Licensed under the Apache License, Version 2.0 (the "License");
*you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
*WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/

#if defined(OS400)   
#pragma convert(850) 
#endif  

/*  
	Example program to demonstrate using the HTML API.
	Standard input	: None
	Standard Ouput	: output.log,output.html 
*/

/*Standard C include files*/
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <ctype.h>

#ifndef OS400
#include <malloc.h>
#endif

/*Notes C API inlcude files*/
#include "global.h"
#include "addin.h"
#define RSRCLNK_DEFS /*for include htmlapi.h*/
#include "htmlapi.h"
#include "nsfdb.h"
#include "nsfnote.h"
#include "osmem.h"
#include "osmisc.h"
#include "../../printLog.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

/*Const defination*/
#define EXIT_SUCCESS		0

#ifndef OS400
#define EXIT_FAILURE		1
#else
#ifdef EXIT_FAILURE
#undef EXIT_FAILURE
#define EXIT_FAILURE		1
#endif
#endif

#define BuufferSize 100
#define ERR_CREATE_CONVETOR 3
#define ERR_CONVERT_NOTE    4
#define ERR_SET_HTMLOPT		5


STATUS rslt = NOERROR;

/*NSF releated defination*/
NOTEHANDLE m_NoteHandle = NULLHANDLE;
DBHANDLE m_DBHandle = NULLHANDLE;
NOTEID	ViewID = 0;

/*HTML releated defination*/
HTMLHANDLE	cvtr = NULL;
MEMHANDLE hRef = 0;
HTMLAPIReference *pRef = 0;
const char* HTMLOption[4] = {	"ForceSectionExpand=1", 
				"RowAtATimeTableAlt=1", 
				"ForceOutlineExpand=1", 
				NULL};
//DWORD fullTextLength = 100;
DWORD fileNumber =0;
//char  text[fullTextLength+1];
//DWORD gotTextLength = fullTextLength;

/*file releated defination*/
char *DatabaseName = "attachment.nsf";
char *HTMLFile = "output.html";
char *LogFile = "output.log";
char *LogInfoFormat = "%s%d\n"; 
FILE *m_pHTMLFile = NULL;
FILE *m_pLogFile = NULL;

/*Function Defination*/
int convert(char c);
void PrintSeperator(void);
void PrintLogInfo(const char*);
STATUS TryNote(char *DatabaseName, NOTEID ViewID, WORD *ErrCode);
STATUS PutOutStuff(HTMLHANDLE cvtr);
STATUS PutText(HTMLHANDLE cvtr);
STATUS PutRefs(HTMLHANDLE cvtr);

char *GetAddressType(HTMLAPI_URLTargetComponent tgt);
char *GetReferenceType(HTMLAPI_URLTargetComponent tgt);


int main(int argc, char **argv)
{
	HCOLLECTION hCollection;
	COLLECTIONPOSITION CollPosition;
	DHANDLE hBuffer;
	DWORD EntriesFound;
	WORD	SignalFlags;
	DWORD i;
	NOTEID *IDList;

	WORD ErrCode = 0;
	
	m_pLogFile = fopen(LogFile, "w");
	if(!m_pLogFile) return 1;
	m_pHTMLFile = fopen(HTMLFile, "w");
	if(!m_pHTMLFile) return 1;
	

	if(rslt = NotesInitExtended(argc, argv)){
	    fprintf(m_pLogFile, "%s\n", "Notes Init fail");
	    return EXIT_FAILURE;
	}
	
	/*Begin HTML Process*/
	PrintSeperator();
	PrintLogInfo("Hi from HAPI");

	rslt = HTMLProcessInitialize();
	
	if ( NOERROR != rslt ) 
	{
	    PrintLogInfo("** Error on HTML-process init");
	    goto ExitStartUp;
	}
		
	if(rslt = NSFDbOpen(DatabaseName, &m_DBHandle))
	{
	    fprintf(m_pLogFile, "%s%s\n", "** Error on Open database : ", DatabaseName);
	    goto ExitTermProcess;
	}
	
	if(rslt = NIFFindView(m_DBHandle, "MyView", &ViewID))
	{
	    fprintf(m_pLogFile, "%s%s\n", "** Error on Find View : ", "MyView");
	    goto ExitCloseDb;
	}
	
	////	look through the View , Get all documents, use HTMLConvertNote
	////	In each document, Convert the RichTextField.
	PrintSeperator();
	PrintLogInfo("look through the View , Get all documents");
	if(rslt = NIFOpenCollection(m_DBHandle, m_DBHandle, ViewID, 0, NULLHANDLE, &hCollection,
	                            NULL, NULL, NULL, NULL))
	{
	    fprintf(m_pLogFile, "%s\n", "** Error on Open View's collection");
	    goto ExitCloseDb;
	}
	
	CollPosition.Level = 0;
	CollPosition.Tumbler[0] = 0;
	
	do
	{
	    
	    rslt = NIFReadEntries(hCollection, &CollPosition, NAVIGATE_NEXT,
	                          1L, NAVIGATE_NEXT, 0xFFFFFFFF, READ_MASK_NOTEID,
	                          &hBuffer, NULL, NULL, &EntriesFound,
	                          &SignalFlags);
	    if (rslt)
	    {
	        fprintf(m_pLogFile, "%s\n", "** Error on Read Entry from View's collection");
	        goto ExitCloseColl;
	    }
		
	    if (hBuffer == NULLHANDLE)
	    {
	        fprintf(m_pLogFile, "%s\n", "** Error on Read Entry from View's collection,buffer is empty");
	        goto ExitCloseColl;
	    }
		
	    /*Iterate through all the documents within the view*/
	    PrintSeperator();
	    PrintLogInfo("Iterate through all the documents within the view");
	    PrintLogInfo("The ID List is:");
	    IDList = (NOTEID far *) OSLockObject (hBuffer);
	    
	    for (i=0; i<EntriesFound; i++)
	    {
	        fprintf(m_pLogFile, "%d\n", IDList[i]);
	        if (NOTEID_CATEGORY & IDList[i]) continue; 
	        {
	            PrintLogInfo("Open the document to convert");

	            if(rslt = NSFNoteOpen(m_DBHandle, IDList[i], OPEN_EXPAND, &m_NoteHandle))
	            {
	                fprintf(m_pLogFile, LogInfoFormat, "** Error in opening note : ", IDList[i]);
	                goto ExitCloseNote;
	            }
	            
	            if(rslt = TryNote(DatabaseName, IDList[i], &ErrCode))
	            {
	                switch(ErrCode) {
	                    case ERR_CREATE_CONVETOR:
	                        goto ExitCloseNote;
	                        break;
	                    case ERR_CONVERT_NOTE:
	                    case ERR_SET_HTMLOPT:
	                        goto ExitDestroyConvetor;
	                        break;
	                }
	            }
	                  
	            if(rslt = PutOutStuff(cvtr))
	            {
	                goto ExitDestroyConvetor;
	            }
	            NSFNoteClose(m_NoteHandle);	
	            m_NoteHandle = NULLHANDLE;
	    
	        }
	    }
	    
	    OSUnlockObject (hBuffer);
	    OSMemFree (hBuffer);
	    
	} while (SignalFlags & SIGNAL_MORE_TO_DO);
	

ExitDestroyConvetor:
	HTMLDestroyConverter(cvtr);
ExitCloseNote:
	if(m_NoteHandle) 
	{
	    NSFNoteClose(m_NoteHandle);	
	    m_NoteHandle = NULLHANDLE;
	}
ExitCloseColl:
	NIFCloseCollection(hCollection);
ExitCloseDb:
	NSFDbClose(m_DBHandle);
ExitTermProcess:
	HTMLProcessTerminate();
ExitStartUp:
	if(rslt)
	{
	    PRINTERROR(rslt,"HTMLProcessInitialize");
	    PRINTLOG("%s\n", "error occur, check the log file");
	    return EXIT_FAILURE;
	}

	if(m_pHTMLFile)	
	{
	    fclose(m_pHTMLFile);
	    m_pHTMLFile = NULL;
	}
	fprintf(m_pLogFile, "%s\n", ">>>>> terminating HTMLProcess <<<<<<\n");
	fprintf(m_pLogFile, "%s\n", "Bye From HAPI");
	if(m_pLogFile)  
	{
	    fclose(m_pLogFile);
	    m_pLogFile = NULL;
	}
	NotesTerm();
	PRINTLOG("%s\n", "Sucess to execute this sample");
	return EXIT_SUCCESS;
}

STATUS TryNote(char *DatabaseName, NOTEID ViewID, WORD *ErrCode)
{
	PrintSeperator();
	PrintLogInfo("Creating HTMLConverter");
	if(rslt = HTMLCreateConverter(&cvtr))
	{
	    PrintLogInfo("Error in Creating HTMLConverter");
	    *ErrCode =  ERR_CREATE_CONVETOR;
	    return rslt;
	}
	
	if(rslt = HTMLSetHTMLOptions(cvtr, HTMLOption))
	{
	    PrintLogInfo("Error in Setting HTMLOption");
	    *ErrCode =  ERR_SET_HTMLOPT;
	    return rslt;
	}
	
	PrintSeperator();
	PrintLogInfo("Begin to Convert Note");
	
	if(rslt = HTMLConvertNote(cvtr, m_DBHandle, m_NoteHandle, 0, 0))
	{
	    PrintLogInfo("Error in Convertting Note");
	    *ErrCode =  ERR_CONVERT_NOTE;
	    return rslt;
	}

	return NOERROR;
}


STATUS PutOutStuff(HTMLHANDLE cvtr)
{
	PrintSeperator();
	PrintLogInfo("</pre>");
	PrintLogInfo("<!--");
	if(rslt = PutRefs(cvtr))
	{
	    PrintSeperator();
	    PrintLogInfo("Error when put out the reference in the document");
	    return rslt;
	}
//	if(rslt = PutArgs(cvtr))
//	{
//		PrintSeperator();
//		PrintLogInfo("Error when put out the arguemnt in the reference");
//		return rslt;
//	}
	PrintLogInfo("-->");
	PrintLogInfo("<hr>");
	if(rslt = PutText(cvtr))
	{
	    PrintSeperator();
	    PrintLogInfo("Error when put out the translated HTML text");
	    return rslt;
	}
	PrintLogInfo("<hr>");
	return NOERROR;
}

STATUS PutRefs(HTMLHANDLE cvtr)
{
	DWORD retval = 0;	// reference number
	char *temp = NULL;  // local variable, used to number transfer
	unsigned int refi = 0;       // reference loop variable
	DWORD nTargets = 0; // target number in a reference
	DWORD nArgs = 0;	// Arguments number in a reference
	DWORD t = 0;		// target loop variable
	HTMLAPI_URLTargetComponent tgt; //target component within some target in a reference
	HTMLAPI_URLArg arg;
	
	/* below is variable used to get the element's number and its offset number within an item*/
	char *pszItemName;  // field name
	BOOL pointFound = FALSE; // used to determine the offset of an element whthin an item
	char *elemOffset = NULL;
	char ElemNumber[10];
	char OffsetNumber[10];
	int	ItemIndex = 0;
	int ElemIndex = 0;
	int postion = 0;
	unsigned int pos = 0;
	int length;           // length of offset number
	int aa = 0;   // loop variable
	unsigned int i = 0;
	
	/* below variable will be used to convert the element and get the text*/
	HTMLHANDLE hHTML = NULLHANDLE; 
	BOOL bBinary = FALSE;
	DWORD wTextLength = 0;
	char *pszBinaryText = NULL;
	
	/*for the picture in the notes, save it to disk*/
	FILE *PictureFie = 0;
	char PictureName[30] = "";
	
	memset(ElemNumber, '\0', sizeof(ElemNumber));
	memset(OffsetNumber, '\0', sizeof(OffsetNumber));
	
	rslt = HTMLGetProperty(cvtr, HTMLAPI_PROP_NUMREFS, &retval);
	fprintf(m_pLogFile, LogInfoFormat, "<HAPI:RefList count=", retval);	

	for ( refi = 0 ; refi < retval; refi++ )
	{
	    rslt = HTMLGetReference(cvtr, refi, &hRef);
	    if(rslt)
	    {
	        PrintLogInfo("Error in Getting Reference");
	        return rslt;
	    }
		
	    rslt = HTMLLockAndFixupReference(hRef, &pRef);
	    if(rslt)
	    {
	        PrintLogInfo("Error in Lock And Fixup Reference");
	        return rslt;
	    }
		
	    fprintf(m_pLogFile, "%s%d%s\n", "<Reference N=\"", refi, "\"");
	    fprintf(m_pLogFile, LogInfoFormat, "	Type:     ", pRef->RefType);
	    fprintf(m_pLogFile, LogInfoFormat, "	CmdId:    ", pRef->CommandId);
	    fprintf(m_pLogFile, LogInfoFormat, "	Text:     ", pRef->pRefText);
	    fprintf(m_pLogFile, LogInfoFormat, "	NTargets: ", pRef->NumTargets);

	    // show the target components of one reference.
	    nTargets = pRef->NumTargets;

	    for (t = 0; t < nTargets; t++) 
	    {
	        tgt = pRef->pTargets[t].Target;
	        fprintf(m_pLogFile, "%s%d%s\n", "		<Component N=\"", t, "\"");
	        fprintf(m_pLogFile, LogInfoFormat, "			AddressableType: ", 
	                GetAddressType(tgt));
	        fprintf(m_pLogFile, LogInfoFormat, "			ReferenceType:   ",
	                GetReferenceType(tgt));
	        fprintf(m_pLogFile, "%s\n", "				<RefValue>:");

	        switch( tgt.ReferenceType ) {
	            case URT_None: 
	                PrintLogInfo("				-none-");
	                break;
	            case URT_Name:  
	                fprintf(m_pLogFile, "%s%s\n", "				", tgt.Value.name); 
	                break;
	            case URT_Unid: 
	                PrintLogInfo("				-uViewID-"); break;
	            case URT_NoteId: 
	                fprintf(m_pLogFile, LogInfoFormat, "				", (void*)tgt.Value.nid);
	                break;
	            case URT_Special: 
	                fprintf(m_pLogFile, LogInfoFormat, "				", tgt.Value.special);
	                break;
	            case URT_RepId: 
	                PrintLogInfo("				-repid-");
	                break;
	        }
	        PrintLogInfo("				</RefValue>\n");
	        PrintLogInfo("		</Component>");
	        PrintSeperator();
	    }

	    PrintLogInfo("</Reference>");
	    pointFound = FALSE;
	    ItemIndex = 0;
	    ElemIndex = 0;
	    postion = 0;
	    
	    for ( t = 0; t < nTargets; t++) 
	    {
	        //check if the refer type is 2 : that is image, then use HTMLSetrRferenceText to 
	        //change the text,
	        tgt = pRef->pTargets[t].Target;
	        
	        switch(pRef->RefType) {
	            case HTMLAPI_REF_OBJECT:
	                break;
	            case HTMLAPI_REF_IMG:
	            {
	                if(t == (nTargets - 2))
	                {
	                    if(tgt.AddressableType == UAT_Field)
	                        pszItemName = tgt.Value.name;
	                }
					
	                if(t == (nTargets - 1))
	                {
	                    elemOffset = tgt.Value.name;
	                    if(tgt.AddressableType == UAT_FieldOffset)
	                    {
	                        /*Get the element number and the offset number which will be used 
	                          in the API HTMLConvertElement	*/
	                        
	                        for(pos = 0; pos < strlen(elemOffset); pos++)
	                        {
	                            if(*(elemOffset+pos) == '.')
	                            {
	                                pointFound = TRUE;
	                                postion = pos;
	                                continue;
	                            }
	                            if(!pointFound)
	                            {
	                                ElemNumber[pos] = elemOffset[pos];
	                                continue;
	                            }
	                            
	                            OffsetNumber[pos - postion -1] = elemOffset[pos];
	                        }
	                        
	                        ItemIndex = atoi(ElemNumber);
	                        length = strlen(OffsetNumber)-1;
	                        for(aa = length; aa >=0; aa--)
	                        {
	                            ElemIndex += (int)pow(16, length-aa) * convert(OffsetNumber[aa]);
	                        }
	
	                        if(rslt = HTMLCreateConverter(&hHTML))
	                        {
	                            PrintLogInfo("Converter creation fail");
	                            //	PrintAPIError(rslt);
	                            return rslt;
	                        }
	                        
	                        rslt = HTMLConvertElement(hHTML, m_DBHandle, m_NoteHandle, pszItemName, 
	                                                  (DWORD)ItemIndex,(DWORD)ElemIndex);
	                        if(rslt)
	                        {
	                            PrintLogInfo("Error: Convert Element fail");
	                            HTMLDestroyConverter(hHTML);
	                            return rslt;
	                        }
	                        rslt = HTMLGetProperty(hHTML, HTMLAPI_PROP_BINARYDATA, &bBinary);
	                        //whatever the bBinary is TRUE or FALSE, we continue
	                        if(rslt = HTMLGetProperty(hHTML, HTMLAPI_PROP_TEXTLENGTH, &wTextLength))
	                        {
	                            PrintLogInfo("HTML Get Image Binary Length fail");
	                            //PrintAPIError(rslt);
	                            return rslt;
	                        }
	                        
	                        pszBinaryText =(char *)malloc(wTextLength);
	                        if(rslt = HTMLGetText(hHTML, 0, &wTextLength, pszBinaryText))
	                        {
	                            PrintLogInfo("HTML Get Image Binary text fail");
	                            //PrintAPIError(rslt);
	                            return rslt;
	                        }
	                        
	                        rslt = HTMLDestroyConverter(hHTML);
	                        //whatever the converter is destroyed or not, we continue

	                        strcpy(PictureName, pszItemName);
	                        strcat(PictureName, ElemNumber);
	                        strcat(PictureName, OffsetNumber);
	                        strcat(PictureName, ".gif");
	                        PictureFie = fopen(PictureName, "wb");
	                        fwrite(pszBinaryText, 1, wTextLength, PictureFie);
	                        fclose(PictureFie);
	                        if(rslt = HTMLSetReferenceText(cvtr, refi, PictureName)) 
	                        {
	                            PrintLogInfo("Error: HTML Set Reference Text Fail");
	                            return rslt;
	                        }

	                        free(pszBinaryText);
	                    }
	                }
	            }
	            break;
	            
	            case HTMLAPI_REF_HREF:
	            {
	                //if there exist attachment files
	                if (tgt.AddressableType == UAT_Filename && tgt.ReferenceType == URT_Name) 
	                {
	                    char *AttachFileName = tgt.Value.name;
	                    BLOCKID item_BlockID;
	                    BLOCKID Prev_Item_BlockID;
	                    WORD	Value_DataType;
	                    BLOCKID	Value_BlockID;
	                    DWORD	Value_Lenth;
	                    fileNumber++;
	                    if(fileNumber == 1)
	                    {
	                        if(rslt = NSFItemInfo(m_NoteHandle, "$FILE", 
	                                              (WORD)strlen("$FILE"), &item_BlockID, &Value_DataType, 
	                                              &Value_BlockID, &Value_Lenth))
	                            PRINTERROR(rslt,"NSFItemInfo");
	                    }
	                    else
	                    {
	                        Prev_Item_BlockID = item_BlockID;
	                        if(rslt = NSFItemInfoNext(m_NoteHandle, Prev_Item_BlockID,
	                                                  "$FILE",(WORD)strlen("$FILE"), &item_BlockID, &Value_DataType,
	                                                  &Value_BlockID, &Value_Lenth))
	                            PRINTERROR(rslt,"NSFItemInfoNext");
	                    }
	                    if( rslt = NSFNoteExtractFileExt(m_NoteHandle, item_BlockID, AttachFileName,
	                                                     NULL, NTEXT_FTYPE_FLAT))
	                        PRINTERROR(rslt,"NSFNoteExtractFileExt");
	                    if(rslt = HTMLSetReferenceText(cvtr, refi, AttachFileName))
	                        PRINTERROR(rslt,"HTMLSetReferenceText");
	                }
	            }
	            break;
	            case HTMLAPI_REF_FRAME:
	                break;
	            case HTMLAPI_REF_APPLET:
	                break;
	            case HTMLAPI_REF_EMBED:
	                break;
	            case HTMLAPI_REF_BASE:
	                break;
	            case HTMLAPI_REF_BACKGROUND:
	                break;
	            case HTMLAPI_REF_CID:
	                break;
	            case HTMLAPI_REF_UNKNOWN:
	                break;
	        }
	    }
	    
	    
	    nArgs = pRef->NumArgs;
	    for (t = 0; t < nArgs; t++) 
	    {
	        arg = pRef->pArgs[t].Arg;
	        fprintf(m_pLogFile, LogInfoFormat, "<URLArg N=", t);
	        fprintf(m_pLogFile, LogInfoFormat, "		CmdArgID: ", arg.Id);
	        fprintf(m_pLogFile, LogInfoFormat, "		CmdArgValueType:   ", arg.Type);
	        PrintLogInfo("			<ArgValue>:");

	        switch( arg.Type ) 
	        {
	            case CAVT_Int: 
	                fprintf(m_pLogFile, "			", arg.Value.n);
	                break;
	            case CAVT_NoteId: 
	                fprintf(m_pLogFile, "			", arg.Value.nid);
	                break;
	            case CAVT_UNID: 
	                PrintLogInfo("			_UNID");
	                break;
	            case CAVT_String:
	                fprintf(m_pLogFile, "%s%s\n", "			", arg.Value.s);
	                break;
	            case CAVT_StringList:
	                temp = arg.Value.slist.first;
	                for(i=0; i<arg.Value.slist.count;i++)
	                {
	                    fprintf(m_pLogFile, "%s%s\n", "			", temp);
	                    temp += strlen(temp);
	                }
	                break;
	        }

	        PrintLogInfo("			</ArgValue>");
	        PrintLogInfo("</URLArg>");
	    }
	    
	    // clean up this ref before going on to next
	    if (hRef) 
	    {
	        OSMemoryUnlock(hRef);
	        OSMemoryFree(hRef);
	        hRef = NULLHANDLE;
	    }
	}
	// the args
	return 0;
}

STATUS PutText(HTMLHANDLE cvtr)
{
	DWORD FullTextLength = 0;
	char  *loc_text = NULL;
	DWORD StartOffset = 0;
	char * pText;
	DWORD TextLength;
	DWORD ExpectedTextLength;
	
	if(rslt = HTMLGetProperty (cvtr, HTMLAPI_PROP_TEXTLENGTH, &FullTextLength))
	{
	    PrintLogInfo("Error: When getting text Property From converter, error occur");
	    return rslt;
	}

	TextLength = FullTextLength;
	ExpectedTextLength = FullTextLength;
	loc_text =(char *)malloc(FullTextLength+1);
	pText = loc_text;
	
	if(rslt = HTMLGetText(cvtr, StartOffset, &TextLength, pText))
	{
	    PrintLogInfo("Error: When getting text From converter, error occur");
	    return rslt;
	}
	
	if (TextLength != ExpectedTextLength) 
	{
	    PrintLogInfo("Error: The text length Got is not equal to the expected");
	    PRINTLOG("%s\n", "Error: The text length Got is not equal to the expected");
	    //not fatal error, continue
	}
	
	loc_text[FullTextLength] = '\0';
	
	HTMLGetProperty (cvtr, HTMLAPI_PROP_TEXTLENGTH, &TextLength);
	fprintf(m_pLogFile, LogInfoFormat, "<HAPI:HTMLText length=", TextLength);
	fprintf(m_pLogFile, LogInfoFormat, " MIMEMaxLineLenSeen=", TextLength);
	PrintLogInfo(">");
	PrintLogInfo(loc_text);
	PrintLogInfo("</HAPI:HTMLText>");
	fprintf(m_pHTMLFile, "%s\n", loc_text);
	free(loc_text);
	return NOERROR;
}
////////////////////////////////////

char* GetAddressType(HTMLAPI_URLTargetComponent tgt)
{
	switch(tgt.AddressableType) {
	    case UAT_None:
	        return "UAT_None"; 
	        break;
	    case UAT_Server:
	        return "UAT_Server";
	        break;
	    case UAT_Database:
	        return "UAT_Database";
	        break;
	    case UAT_View:
	        return "UAT_View";
	        break;
	    case UAT_Form:
	        return "UAT_Form";
	        break;
	    case UAT_Navigator:
	        return "UAT_Navigator";
	        break;
	    case UAT_Agent:
	        return "UAT_Agent";
	        break;
	    case UAT_Document:
	        return "UAT_Document";
	        break;
	    case UAT_Filename: 	/* internal filename of attachment */
	        return "UAT_Filename";
	        break;
	    case UAT_ActualFilename:/* external filename of attachment if different */
	        return "UAT_ActualFilename";
	        break;
	    case UAT_Field:
	        return "UAT_Field";
	        break;
	    case UAT_FieldOffset:
	        return "UAT_FieldOffset";
	        break;
	    case UAT_FieldSuboffset:
	        return "UAT_FieldSuboffset";
	        break;
	    case UAT_Page:
	        return "UAT_Page";
	        break;
	    case UAT_FrameSet:
	        return "UAT_FrameSet";
	        break;
	    case UAT_ImageResource:
	        return "UAT_ImageResource";
	        break;
	    case UAT_CssResource:
	        return "UAT_CssResource";
	        break;
	    case UAT_JavascriptLib:
	        return "UAT_JavascriptLib";
	        break;
	    case UAT_FileResource:
	        return "UAT_FileResource";
	        break;
	    case UAT_About:
	        return "UAT_About";
	        break;
	    case UAT_Help:
	        return "UAT_Help";
	        break;
	    case UAT_Icon:
	        return "UAT_Icon";	
	        break;
	    case UAT_SearchForm:
	        return "UAT_SearchForm";
	        break;
	    case UAT_SearchSiteForm:
	        return "UAT_SearchSiteForm";
	        break;
	    case UAT_Outline:
	        return "UAT_Outline";
	        break;
	    case UAT_NumberOfTypes:			/* must be the last one */
	        return "UAT_NumberOfTypes";
	        break;
	}
	return "UnKnown";
}

char* GetReferenceType(HTMLAPI_URLTargetComponent tgt)
{
	switch(tgt.ReferenceType) {
	    case URT_None:
	        return "URT_None";
	        break;
	    case URT_Name:
	        return "URT_Name";
	        break;
	    case  URT_Unid:
	        return "URT_Unid";
	        break;
	    case URT_NoteId	:
	        return "URT_NoteId";
	        break;
	    case 	URT_Special:		/* special name */
	        return "URT_Special";
	        break;
	    case 	URT_RepId:
	        return "URT_RepId";
	        break;
	    case URT_NumberOfTypes:
	        return "URT_NumberOfTypes";
	        break;
	}
	return "UnKnown";
}

void PrintSeperator(void)
{
	if(!m_pLogFile) m_pLogFile = fopen(LogFile, "rw");
	fprintf(m_pLogFile, "%s\n", "================================");
}

void PrintLogInfo(const char* Info)
{
	if(!m_pLogFile) m_pLogFile = fopen(LogFile, "rw");
	fprintf(m_pLogFile, "%s\n", Info);
}

int convert(char c)
{
	char pszStr[2]= "\0";
	switch(toupper(c)) {
	    case 'A':
	        return 10;
	        break;
	    case 'B':
	        return 11;
	        break;
	    case 'C':
	        return 12;
	        break;
	    case 'D':
	        return 13;
	        break;
	    case 'E':
	        return 14;
	        break;
	    case 'F':
	        return 15;
	        break;
	}
	pszStr[0]=c;
	return atoi(pszStr);
}
