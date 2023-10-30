/****************************************************************************************
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
 * 
 *      PROGRAM:        DUSSPI.DLL      
 *
 *      FILE:           DUSSPI.C
 *
 *      PURPOSE:        Demonstrates the use of the DUS (Domino Upgrade Services) SPI
 *
 *****************************************************************************************
 */
#include <windows.h>
#include <global.h>
#include <osmisc.h>
#include <osmem.h>
#include <dus.h>
#include <regerr.h>
#include <misc.h>
#include <winbase.h>
#include <stdnames.h>
#include <nsfnote.h>

#include "dusspi.h"
#include "duspierr.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

PDUS_CONTEXT pDUSCtx;           

/*************************************************************************
    FUNCTION:   DUSGetName

    PURPOSE:    Provide UI strings immediately after dll is loaded.

	INPUTS:         hInstance -- instance handle of this DUS.
				DUSNameBufLen -- size of the DUSNameBuf.
				DUSDescriptionBufLen -- size of the DUSDescriptionBuf.

	OUTPUTS:        DUSNameBuf -- buffer into which the DUS will copy the name
				of the DUS as it will appear in the Foreign Directory Source
				listbox (in the upgrade dialog).
				DUSDescriptionBuf -- buffer into which the DUS will copy a brief
				description of the DUS as it will appear in the upgrade dialog.

    COMMENTS:   This call notifies the upgrade DLL that the upgrade dialog
			is being displayed.  It also passes in the instance handle of
			the DUS, which the DUS may want to store for later DUS operations.
*************************************************************************/
STATUS LNCALLBACK DUSGetName(HMODULE hInstance,
			     char *DUSNameBuf,
			     WORD DUSNameBufLen,
			     char *DUSDescriptionBuf,
			     WORD DUSDescriptionBufLen )
{
    STATUS error = NOERROR;

    OSLoadString(hInstance, STR_DUS_NAME, DUSNameBuf, DUSNameBufLen);
    OSLoadString(hInstance, STR_DUS_DESCRIPTION, DUSDescriptionBuf, DUSDescriptionBufLen);

    return(error);
}

/*************************************************************************
    FUNCTION:   DUSStart

    PURPOSE:    Notifies the DUS of one of two situations:
		1) It has been selected from the foreign directory
		   source list in the foreign directory dialog or;
		2) The first of possibly several users retrieved
		   from this DUS is about to be registered in Notes.

		It also supplies the DUS framework with a context handle,
		which will be passed back to the DUS with every subsequent
		DUSxxx function.
		finally, it supplies the DUS with flags representing
		UI defaults for the Foreign Directory dialog.

    INPUTS:     hInstance -- Instance handle to this DUS dll.

		hUserNote -- this note handle will only be valid (non-NULL)
		if DUSStart is called just prior to user registration
		(from the user registration dialog, as opposed to from the
		foreign directory dialog).  The hUserNote contains context
		information (represented by the context handle passed back
		to the caller).  The DUS can stores this context information
		in the hUserNote passed in with DUSGetUserInformation.
		When DUSStart is called from the Foreign Directory dialog,
		hUserNote will be NULLHANDLE and the DUS allocates and inits
		a context buffer for the first time.

		DUSProgressBar -- pointer to a callback used by the DUS
		to display progress on potentially length operations, such
		as user retrieval.  This should be retained by the DUS for
		subsequent calls.
			
		DUSLogEvent -- pointer to a callback used by the DUS
		to log progress to the Notes log on any operation, such
		as user retrieval.  This should be retained by the DUS for
		subsequent calls.  The log cannot be written to after DUSStop
		is called.

    OUTPUTS:    pRethContext -- pointer to a context handle the DUS can optionally
		allocate/return to the DUS framework.  On every subsequent call
		to the DUS in the current session, this context handle will be passed
		to the DUS by the DUS framework.  The DUS is responsible for freeing
		this handle in DUSStop and should store the context handle in the
		hUserNote if the DUS needs to convert mail or take pre- or post-
		user registration actions available with DUSRegistrationNotify.


    COMMENTS:   This function is also where the DUS should put up any secondary UI
		necessary to handle directory specific operations, such as
		retrieving users.  For example, if the administrator will have
		to supply a password to access the foreign directory source,
		the DUS should prompt for it here.

		DUSStart is called once per Foreign Directory dialog session
		(for each DUS) and once per registration session (for each DUS,
		if users from that DUS source are being registered).
*************************************************************************/
STATUS LNCALLBACK DUSStart(HMODULE hInstance,
			   DHANDLE *pRethContext,
			   NOTEHANDLE hUserNote,
			   DWORD *pRetStartFlags,
			   DUSPROGRESSBARPROC DUSProgressBar,
			   DUSLOGEVENTPROC DUSLogEvent)
{
	STATUS error = NOERROR;
	DWORD size;
	
	/* This is where the following flags would be set. 
		- fDUSDoMailConversion  
		- fDUSGenerateRandPWs
		- fDUSAdvancedDlg
		- fDUSUseFullNameProvided
		- fDUSUseMailFileNameProvided
		- fDUSAllowEmptyGroups
	*/
	/* In our case we want this DUS to:
		1) generate random passwords
	*/
	*pRetStartFlags = fDUSGenerateRandPWs;

	/* Allocate the DUS Context Buffer. */
	pDUSCtx = NULL;
	
	size = sizeof(DUS_CONTEXT);
	if(error = OSMemAlloc(0, size, pRethContext))
	{
	    pRethContext = NULLHANDLE;
	}
	else
	{
	    pDUSCtx = OSLock(DUS_CONTEXT, *pRethContext);
	    memset(pDUSCtx, 0, (size));
	}
	 
	pDUSCtx->hDUSModule = hInstance;
	pDUSCtx->ProgressBarProc = DUSProgressBar;
	pDUSCtx->LogEventProc = DUSLogEvent;
	/*
	 * This is where a check can be made to see if DUSStart() is being called from the 'People and Groups 
	 * Migration' screen, or if DUSStart() is being called from user/group registration.  If the
	 * hUserNote == NULLHANDLE then the DUS is being called from the 'People and Groups Migration'
	 * screen and a secondary UI can be displayed otherwise DUSStart() is being called from 
	 * user registration where registration on a user is about to begin.
	 *      if(hUserNote == NULLHANDLE)
	 *         .... being called from migration screen, put up secondary UI if necessary
	 */
	
		
	if(pDUSCtx != NULL)
	{
	    OSUnlock(*pRethContext);
	    pDUSCtx = NULL;
	}

	return(error);
}

/*************************************************************************
    FUNCTION:   DUSRetrieveUsers

    Purpose:
	This function allocates and passes back an array of DUS_ENTRY structs
	containing information about the users available for upgrade/migration.

	Inputs:
		hContext -- provided by the DUS with DUSStart and passed back
			    with every call by the framework.
		StartIndex -- 0L the first time the DUS is called.  If all users
			      are not retrieved the first time and subsequent calls
			      are necessary, this value is equal to the resume index
			      passed back by the DUS on the previous call (*pResumeIndex).
		NumUsersRequested -- number of users that should be provided
				     to the caller, if available.

	Outputs:
		pResumeIndex -- Set this value if additional users need to be
			returned to the DUS framework (number of available users exceeds
			the number requested by the framework).  This value will be returned
			to the DUS on the subsequent DUSRetrieveUsers call as the StartIndex.
			When user retrieval is complete the pResumeIndex should be set to 0L.
			The pResumeIndex will always be 0L the first time DUSRetrieveUsers
			is called.
			
		pNumUsersReturned -- points to the number of users returned by the DUS to
				     the caller (users returned via the array of DUS_ENTRY
				     structs represented  by pRethExternalUsers).  This value
				     should be used to allocate an array of DUS_ENTRY structs
				     with the size:
				     NumUsersRequested * sizeof(DUS_ENTRY)
		pRethExternalUsers -- pointer to the memory handle of an array of DUS_ENTRY
				      structs returned to the caller.  The DUS allocs and assigns
				      this handle, sets the user information into the array, and
				      returns the handle UNLOCKED to the caller. The array should
				      contain *pNumUsersReturned structs.  The caller will
				      free the memory associated with the handle.
		pRetUserEntrySize -- pointer to the size of the DUS_ENTRY struct (an array of
				     which are alloced by the DUS and passed back to the caller)

	Comments:       DUSRetrieveUsers will be continue to be called by the DUS framework
				until the DUS returns a resume index of 0 or the *pNumUsersReturned is
				less than *pNumUsersRequested.

*************************************************************************/
STATUS LNCALLBACK DUSRetrieveUsers(DHANDLE hContext,
				   DWORD StartIndex,
				   DWORD *pResumeIndex,
				   DWORD NumUsersRequested,
				   DWORD *pNumUsersReturned,
				   DHANDLE *pRethExternalUsers,
				   DWORD *pRetUserEntrySize)

{
	
	STATUS          error = NOERROR;
	PDUS_ENTRY      pExternalUsers = NULL;
	DWORD size, i;
	HFILE hFile;
	char *FileName = "d:\\api_v5\\samples\\admin\\dusspi\\names.txt";
	DWORD FileSize = 0, NumberOfBytesRead = 0, index = 0, position = 0;
	char FileBuffer[1024];
	char Message[MAXSPRINTF];
	
	
	if(hContext)
	    pDUSCtx = OSLock(DUS_CONTEXT, hContext);
	else
	    return(ERR_DUS_CONTEXT_CORRUPT);

	pDUSCtx->LogEventProc( STR_DUS_GETTING_USERS, pDUSCtx->hDUSModule, NOERROR );
	*pResumeIndex = StartIndex;
	*pNumUsersReturned = 0;

	/* 
	 * Read the input file "names.txt" retrieving the users to migrate.
	 * 
	 * Structure of Input File:
	 *
	 * FirstName LastNameCRLF
	 * FirstName LastNameCRLF
	 * .....
	 * FirstName LastNameCRLFCRLF
	 */
	hFile = CreateFile( FileName, GENERIC_READ, FILE_SHARE_READ, 
	                    NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, 
	                    NULL);
	if(hFile == HFILE_ERROR)
	{
	    error = ERR_DUS_EXTENDED_ERROR;
	    pDUSCtx->ExtendedError = STR_DUS_ERR_NOFILE_NAMES;
	    pDUSCtx->ExtendedErrorLevel = DUS_ERROR_LEVEL_ERROR; /* Going to gracefully shutdown */
	}
	else
	{
	    FileSize = GetFileSize((DHANDLE)hFile, NULL);
	    if(ReadFile((DHANDLE)hFile, FileBuffer, FileSize, &NumberOfBytesRead, NULL) == 0)
	    {
	        error = ERR_DUS_EXTENDED_ERROR;
	        pDUSCtx->ExtendedError = STR_DUS_ERR_READFILE_NAMES;
	        pDUSCtx->ExtendedErrorLevel = DUS_ERROR_LEVEL_ERROR; /* Going to gracefully shutdown */
	        
	    }
	    else
	    {
	        /* Parse through the file to get the number of users available for migration. */
	        for( i = 0; i < NumberOfBytesRead; i++)
	        {
	            if((FileBuffer[i] == CHAR_LF) && (FileBuffer[i+1] != CHAR_CR))
	                *pNumUsersReturned += 1;
	        }

	        /* 
	         * Allocate space for the number of users available for migration.
	         */
	        size = (*pNumUsersReturned * sizeof(DUS_ENTRY));
	        if( error = OSMemAlloc(0, size, pRethExternalUsers))
	        {
	            pRethExternalUsers = NULLHANDLE;
	        }
	        else
	        {
	            pExternalUsers = OSLock(DUS_ENTRY, *pRethExternalUsers);
	            memset(pExternalUsers, 0, (size));
	            OSLoadString(pDUSCtx->hDUSModule, STR_DUS_GETTING_USERS, Message, sizeof(Message));
	            for(i=0, index=0; i < *pNumUsersReturned; i++) 
	            {
	                pDUSCtx->ProgressBarProc((DWORD)*pNumUsersReturned, i, Message);
	                Sleep(100);                     /* Allow for illustration of Progress Bar status */
	                position = 0; 
	                pExternalUsers[i].ID = i + *pNumUsersReturned;
	                while(((FileBuffer[index] != CHAR_CR) && (FileBuffer[index] != CHAR_LF)) && (index < NumberOfBytesRead))
	                {
	                    /* Parse the data from the input file */
	                    pExternalUsers[i].Name[position] = FileBuffer[index];
	                    index++;
	                    position++;
	                }
	                index+=2; /* Hop over CRLF in the input File */
	            }
	            pDUSCtx->ProgressBarProc(*pNumUsersReturned, *pNumUsersReturned, Message); /* Reset Progress Bar */
	            OSUnlock(*pRethExternalUsers);
	            *pRetUserEntrySize = sizeof(DUS_ENTRY);
	        }
	        pDUSCtx->LogEventProc( STR_DUS_FINISHED_USERS, pDUSCtx->hDUSModule, NOERROR );
	    } /* endif ReadFile */
	} /* endif CreateFile */
	if(CloseHandle((DHANDLE)hFile) == 0)
	{
	    error = ERR_DUS_EXTENDED_ERROR;
	    pDUSCtx->ExtendedError = STR_DUS_ERR_CLOSEFILE_NAMES;
	    pDUSCtx->ExtendedErrorLevel = DUS_ERROR_LEVEL_ERROR; /* Going to gracefully shutdown */
	}               
	if(pDUSCtx != NULL)
	{
	    OSUnlock(hContext);
	    pDUSCtx = NULL;
	}
	return(error);

}


/*************************************************************************
    FUNCTION:   DUSRetrieveGroups

    Purpose:
	This function allocates and passes back an array of DUS_ENTRY structs
	containing information about the users available for upgrade/migration.

	Inputs:
		hContext -- provided by the DUS with DUSStart and passed back
			    with every call by the framework.
		StartIndex -- 0L the first time the DUS is called.  If all groups
			      are not retrieved the first time and subsequent calls
			      are necessary, this value is equal to the resume index
			      passed back by the DUS on the previous call (*pResumeIndex).
		NumGroupsRequested -- number of groups that should be provided
				      to the caller, if available.

	Outputs:
		pResumeIndex -- Set this value if additional groups need to be
			returned to the DUS framework (number of available groups exceeds
			the number requested by the framework).  This value will be returned
			to the DUS on the subsequent DUSRetrieveGroups call as the StartIndex.
			When group retrieval is complete the pResumeIndex should be set to 0L.
			The pResumeIndex will always be 0L the first time DUSRetrieveGroups
			is called.
			
		pNumGroupsReturned -- points to the number of groups returned by the DUS to
				      the caller (groups returned via the array of DUS_ENTRY
				      structs represented     by pRethExternalGroups).  This value
				      should be used to allocate an array of DUS_ENTRY structs
				      with the size:
				      NumGroupsRequested * sizeof(DUS_ENTRY)
		pRethExternalGroups -- pointer to the memory handle of an array of DUS_ENTRY
				       structs returned to the caller.  The DUS allocs and assigns
				       this handle, sets the user information into the array, and
				       returns the handle UNLOCKED to the caller. The array should
				       contain *pNumUsersReturned structs.  The caller will
				       free the memory associated with the handle.
		pRetGroupEntrySize -- pointer to the size of the DUS_ENTRY struct (an array of
				      which are alloced by the DUS and passed back to the caller)

	Comments:       DUSRetrieveGroups will be continue to be called by the DUS framework
			until the DUS returns a resume index of 0 or the *pNumGroupsReturned is
			less than *pNumGroupsRequested.

*************************************************************************/
STATUS LNCALLBACK DUSRetrieveGroups(DHANDLE hContext,
				    DWORD StartIndex,
				    DWORD *pResumeIndex,
				    DWORD NumGroupsRequested,
				    DWORD *pNumGroupsReturned,
				    DHANDLE *pRethExternalGroups,
				    DWORD *pRetGroupEntrySize)
{
	/* This sample is only dealing with individual users and not groups. */
	return(NOERROR);
}
/***********************************************************************
FUNCTION:       DUSGetGroupInformation

Purpose:
	Returns full information about the group for display immediately
	after the administrator has selected the group from the available list. 

Inputs:
	hContext -- pointer to DUS specific context.  Alloced/provided by
	the DUS in DUSStart and passed back to the DUS with every DUS function.

	GroupName -- name of the group selected (originally supplied by the DUS
	with DUSRetrieveGroups).

Output:
	Additional group information in the form of fields written to the hNote.
	The following fields can be supplied:
		GroupType -- type of Notes group, if known (Multi-purpose, mail only, etc.)
			     Field name is ADMINP_GROUP_TYPE.

		Description -- description of the group if known (MAIL_LISTDESCRIPTION_ITEM)

		ParentGroupNamesList -- list of the groups parent groups, if applicable.
					Field name is USERREG_DUSPARENTGROUPS_ITEM.

		See stdnames.h for the field names of a group document.

************************************************************************/
STATUS LNCALLBACK DUSGetGroupInformation(DHANDLE hContext,
					char *GroupName,
					NOTEHANDLE hGroupNote)
{
	/* This sample is only dealing with individual users and not groups. */ 
	return(NOERROR);
}

/***********************************************************************
FUNCTION:       DUSGetGroupMembers

Purpose:
	Returns membership information about the group.  Members will also be
	added to the selected list for upgrade, depending on options chosen
	by the administrator in the group dialog.  Called after DUSGetGroupInformation
	(the administrator has selected the group from the available list). 

Inputs:
	hContext -- pointer to DUS specific context.  Alloced/provided by
	the DUS in DUSStart and passed back to the DUS with every DUS function.

	GroupName -- name of the group selected (originally supplied by the DUS
	with DUSRetrieveGroups).

Output:
	Additional group information in the form of fields written to the hNote.

	hUserMembersList -- allocated by the DUS, this is a list containing all
	of the user members of the Group identified by GroupName.

	hGroupMembersList -- allocated by the DUS, this is a list containing all
	of the immediate group members of the Group identified by GroupName.

	These LIST handles must be created with the prefix data type set
	to FALSE (see ListAllocate) and must be passed UNLOCKED back to 
	Domino or Notes. Domino or Notes will take care of deallocation.

Comments:
************************************************************************/
STATUS LNCALLBACK DUSGetGroupMembers( DHANDLE hContext,
				      char *GroupName,
				      NOTEHANDLE hGroupNote,
				      DHANDLE *phGroupMembersList,
				      DHANDLE *phUserMembersList)
{
	/* This sample is only dealing with individual users and not groups. */
	return(NOERROR);
}


/*************************************************************************
    FUNCTION:   DUSGetUserInformation(  DHANDLE, NOTEHANDLE, DWORD);

    Comments:
	This call supplies the upgrade DLL with one of the users selected
		for upgrade and returns full information for this user.

	Inputs:
		hContext -- handle to this DUS's context information,
			    initially set in DUSStart.  Passed with every DUS call.
		UserName -- The text originally supplied to the DUS framework with DUSRetrieveUsers.
		hUserNote -- contains the UserName and ID used by the DUS to identify
			     the user for which full information is being requested.
			     The DUS stores and passes back information to be set
			     in the NAB person document via the hUserNote.
		TotalLeftToRead -- This identifies the number of selected users for
				   which full information still needs to be returned by the DUS.
				   When this value is 1, it should be the last time DUSGetUserInformation
				   is called.  This is passed as a courtesy to the DUS, which
				   might be able to make retrieval more efficient if it knows:
				   1) the number of users selected
				   2) when retrieval is complete
	Outputs:
		hUserNote -- contains the UserName and ID used by the DUS to identify
			     the user for which full information is being requested.
			     The DUS stores and passes back information to be set
			     in the NAB person document via the hUserNote.  When the DUS
			     is called before/after user registration (DUSRegistrationNotify
			     and optionally, DUSConvertMailFile) this hUserNote will be
			     passed back to the DUS.  Any information the DUS needs at that
			     time should be stored here, such as information represented by
			     the hContext handle passed in to the DUS with every DUS function.
			     If the DUS writes it's own non-standard "context" information
			     to the hUserNote, it should store this info in one or more
			     fields prefixed with "Context", such as "ContextPostOfficeInfo".
			     This prevents an unintentional name collision with other field
			     names in the hUser Note.

			     The hUserNote should NOT be updated or closed by the DUS.  The
			     DUS framework (caller) writes additional fields to this note and
			     will update/close the hUserNote appropriately. 

	Comments:
*************************************************************************/
STATUS LNCALLBACK DUSGetUserInformation(DHANDLE hContext,
					char *UserName,
					NOTEHANDLE hUserNote,
					DWORD TotalLeftToRead)
{
	/*
	 * Once the user has hit the button 'Migrate', this function will be called to get all the
	 * user information.  Since our information is provided in an input file.  We will read
	 * this file to get the information provided.
	 */
	STATUS error = NOERROR;
	WORD lname,fname,sname, result=0;
	char FirstName[25], LastName[25], ParseName[50], ShortName[25];
	char *FileName = "d:\\api_v5\\samples\\admin\\dusspi\\address.txt";
	HFILE hFile;
	char *FileBuffer, LineOfData[1024], *pdest;
	DWORD FileSize=0, NumberOfBytesRead=0, NumLines=0, pos=0,i=0, NameLen=0, index=0;
	BOOL t;

	if(hContext)
	    pDUSCtx = OSLock(DUS_CONTEXT, hContext);
	else
	    return(ERR_DUS_CONTEXT_CORRUPT);

	if(error = NSFItemSetText(hUserNote, USERREG_FULLNAME_ITEM, UserName, (WORD)strlen(UserName)))
	    goto Finish;
	/*
	 * Take apart UserName to get First and Last Name.  We are not dealing with Titles nor Middle
	 * Initials.
	 */
	strcpy(ParseName, UserName);
	NameLen = strlen(UserName);
	for( i=0, fname=0, lname=0, sname=0, index=0; i < NameLen; i++)
	{
	    while((ParseName[index] != CHAR_SPC) && (index < NameLen))
	    {
	        FirstName[fname] = ParseName[index];
	        if( fname == 0 )
	        {
	            ShortName[sname] = ParseName[index];
	            sname++;
	    }
	        index++;
	        fname++;
	    }
	    index++;
	    while(index < NameLen)
	    {
	        LastName[lname] = ParseName[index];
	        ShortName[sname] = ParseName[index];
	        index++;
	        lname++;
	        sname++;
	    }
	}
	if(error = NSFItemSetText(hUserNote, USERREG_LASTNAME_ITEM, LastName, lname))
	    goto Finish;
	
	if(error = NSFItemSetText(hUserNote, USERREG_FIRSTNAME_ITEM, FirstName, fname))
	    goto Finish;

	if(error = NSFItemSetText(hUserNote, USERREG_SHORTNAME_ITEM, ShortName, sname))
	    goto Finish;
	
	/* 
	 * USERREG_DUSADVANCEDINFO_ITEM must be set to '1' in order for the DUS framework to 
	 * accept data for the following fields:
	 *      
	 *              (please refer to 'stdnames.h' for an up to date list)
	 *              USERREG_STORAGE_ITEM                      "MessageStorage"
	 *              USERREG_STORAGE_UNKNOWN                   0xFF
	 *              USERREG_STORAGE_CDRECORDS                 0
	 *              USERREG_STORAGE_CDRECORDS_AND_RFC822      1 
	 *              USERREG_STORAGE_RFC822                    2
	 *              USERREG_STORAGE_NATIVE_MIME               3
	 *              USERREG_STORAGE_NATIVE_MIME_ONLY          4               
	 *              USERREG_PERSONAL_TITLE          "Title"
	 *              USERREG_GENERATION_QUALIFIER    "Suffix"
	 *              USERREG_HOME_STREETADDRESS      "StreetAddress"
	 *              USERREG_HOME_CITY               "City"
	 *              USERREG_HOME_STATE              "State"
	 *              USERREG_HOME_ZIP                "Zip"
	 *              USERREG_HOME_COUNTRY            "Country"
	 *              USERREG_PHONENUMBER_ITEM        "PhoneNumber"
	 *              USERREG_HOME_FAX                "HomeFAXPhoneNumber"
	 *              USERREG_SPOUSE                  "Spouse"              
	 *              USERREG_CHILDREN                "Children"
	 *              USERREG_COMPANYNAME_ITEM        "CompanyName"
	 *              USERREG_JOB_TITLE               "JobTitle"
	 *              USERREG_DEPARTMENT_ITEM         "Department"
	 *              USERREG_MANAGER                 "Manager"
	 *              USERREG_OFFICEPHONE_ITEM        "OfficePhoneNumber"
	 *              USERREG_CELL_PHONE              "CellPhoneNumber"
	 *              USERREG_PAGER                   "PhoneNumber_6"
	 *              USERREG_OFFICE_FAX              "OfficeFAXPhoneNumber"
	 *              USERREG_ASSISTANT               "Assistant"
	 *              USERREG_OFFICE_STREETADDRESS    "OfficeStreetAddress"
	 *              USERREG_OFFICE_CITY             "OfficeCity"
	 *              USERREG_OFFICE_STATE            "OfficeState"
	 *              USERREG_OFFICE_ZIP              "OfficeZip"
	 *              USERREG_OFFICE_COUNTRY          "OfficeCountry"
	 *              USERREG_OFFICE_NUMBER           "OfficeNumber"
	 *              USERREG_ENCRYPT_INCOMING_MAIL   "EncryptIncomingMail"
	 *              USERREG_X400_ADDRESS            "x400Address"
	 *              USERREG_WEB_SITE                "WebSite"
	 *              USERREG_CALENDARDOMAIN_ITEM     "CalendarDomain" 
	 */

	if(error = NSFItemSetText(hUserNote, USERREG_DUSADVANCEDINFO_ITEM, "1", 1))
	    goto Finish;
	/*
	 * Read the address.txt file to get the address information for this username.
	 */
	hFile = CreateFile( FileName, GENERIC_READ, FILE_SHARE_READ, NULL,
	                     OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if(hFile == HFILE_ERROR)
	{
	    error = ERR_DUS_EXTENDED_ERROR;
	    pDUSCtx->ExtendedError = STR_DUS_ERR_NOFILE_ADDRESS;
	    pDUSCtx->ExtendedErrorLevel = DUS_ERROR_LEVEL_ERROR; /* Going to gracefully shutdown */
	}
	else
	{
	    FileSize = GetFileSize((DHANDLE)hFile, NULL);
	    (char *)FileBuffer = malloc(FileSize);
	    if(FileBuffer == NULL)
	    {
	        error = ERR_DUS_EXTENDED_ERROR;
	        pDUSCtx->ExtendedError = STR_DUS_ERR_NOMEM;
	        pDUSCtx->ExtendedErrorLevel = DUS_ERROR_LEVEL_ERROR;
	    }
	    else
	    {
	        if(ReadFile((DHANDLE)hFile, FileBuffer, FileSize, &NumberOfBytesRead, NULL) == 0)
	        {
	            error = ERR_DUS_EXTENDED_ERROR;
	            pDUSCtx->ExtendedError = STR_DUS_ERR_READFILE_ADDRESS;
	            pDUSCtx->ExtendedErrorLevel = DUS_ERROR_LEVEL_ERROR; /* Going to gracefully shutdown */
	        }
	        else
	        {
	            for( i=0, index=0; i < NumberOfBytesRead; i++)
	            {
	                if((FileBuffer[i] == CHAR_LF) && (FileBuffer[i+1] != CHAR_CR))
	                    NumLines++;
	            }
	            t = TRUE;
	            for( i=0, index=0; ((i < NumLines) && (t == TRUE)); i++)
	            {
	                pos = 0;
	                while(((FileBuffer[index] != CHAR_CR) && (FileBuffer[index] != CHAR_LF)) 
	                        && (index < NumberOfBytesRead))
	                {
	                    LineOfData[pos] = FileBuffer[index];
	                    pos++;
	                    index++;
	                }
	                LineOfData[pos] = '\0';
	                pdest = strchr(LineOfData, CHAR_COMMA);
	                result = pdest - LineOfData + 1;
	                if(strncmp(LastName, LineOfData, result-1) == 0)
	                {
	                    /* Then we have a match on this user information. */
	                    pos = result;
	                    pdest = strchr(&LineOfData[pos], CHAR_COMMA);
	                    result = pdest - &LineOfData[pos] + 1;
	                    if(pdest == NULL) 
	                        goto Finish;
	                    if(error = NSFItemSetText(hUserNote, USERREG_HOME_STREETADDRESS, 
	                                              ((char *)&LineOfData[pos]), (WORD)(result-1)))
	                        goto Finish;
	                    pos += result;
	                    pdest = strchr(&LineOfData[pos], CHAR_COMMA);
	                    result = pdest - &LineOfData[pos] + 1;
	                    if(pdest == NULL)
	                        goto Finish;
	                    if(error = NSFItemSetText(hUserNote, USERREG_HOME_CITY, 
	                                              ((char *)&LineOfData[pos]), (WORD)(result-1)))
	                        goto Finish;
	                    pos += result;
	                    pdest = strchr(&LineOfData[pos], CHAR_COMMA);
	                    result = pdest - &LineOfData[pos] + 1;
	                    if(pdest == NULL)
	                        goto Finish;
	                    if(error = NSFItemSetText(hUserNote, USERREG_HOME_STATE, 
	                                              ((char *)&LineOfData[pos]), (WORD)(result-1)))
	                        goto Finish;
	                    pos+= result;
	                    if(error = NSFItemSetText(hUserNote, (char *)USERREG_HOME_ZIP, 
	                                              ((char *)&LineOfData[pos]), 5))
	                        goto Finish;
	                    t = FALSE; /* We have a match so we can exit from searching through this file */
	                }
	                index += 2; /* Hope over CRLF in the input File */
	            }
	            if(CloseHandle((DHANDLE)hFile) == 0)
	            {
	                error = ERR_DUS_EXTENDED_ERROR;
	                pDUSCtx->ExtendedError = STR_DUS_ERR_CLOSEFILE_ADDRESS;
	                pDUSCtx->ExtendedErrorLevel = DUS_ERROR_LEVEL_ERROR; /* Going to gracefully shutdown */
	            }       
	        }/* endif ReadFile */
	        free(FileBuffer);
	    } /* endif FileBuffer */
	} /* endif hFile */

Finish:
	if((error != NOERROR) && (error != ERR_DUS_EXTENDED_ERROR))
	{
	    error = ERR_DUS_EXTENDED_ERROR;
	    pDUSCtx->ExtendedError = STR_DUS_ERR_ADDRESSES;
	    pDUSCtx->ExtendedErrorLevel = DUS_ERROR_LEVEL_ERROR;
	}

	if(pDUSCtx != NULL)
	{
	    OSUnlock(hContext);
	    pDUSCtx = NULL;
	}
	return(error);
}


/***********************************************************************
FUNCTION:       DUSAdvancedDlg

Purpose:
	Allows this dll to put up it's own dialog from the Foreign
	Directory dialog when the "Advanced" button is pushed.

Inputs:
	hContext -- pointer to DUS specific context.  Alloced/provided by
	the DUS in DUSStart and passed back to the DUS with every DUS function.

Output:
	Advanced dialog put up the by DUS.

Comments:
	DUSStart can return a flag which ORs in fDUSAdvancedDlg.
	If it does so, when the "Advanced" button is pressed
	from the foreign Directory dialog, DUSAdvancedDlg will be called.

	Information collected from the dialog can be stored in the buffer
	represented by hContext, but the handle itself should not change.
************************************************************************/
STATUS LNCALLBACK DUSAdvancedDlg( DHANDLE hContext)
{
	/*
	 * There is no need for further information to be gathered. So we are not going to 
	 * support an advanced dialog for this DUS.
	 */
	return(NOERROR);
}

/***********************************************************************
FUNCTION:       DUSStop

Purpose:
	Notifies the DUS that the end of this session (started
	by DUSStart and represented by hContext) is over.  Memory
	associated with the context handle (alloced in DUSStart) should
	now be freed.

Inputs:
	hContext -- pointer to DUS specific context.  Alloced/provided by
	the DUS in DUSStart and passed back to the DUS with every DUS function.

Comments:
	DUSStop is called from one of two places:
	1)      If the DUS was started from the foreign Directory dialog,
		DUSStop will be called when that dialog is closed.

	2)      If the DUS was started during user registration because
		a user from this DUS was being registered, DUSStop will
		be called when registration of ALL users in that session
		is complete.
************************************************************************/
STATUS LNCALLBACK DUSStop( DHANDLE hContext )
{
	STATUS error = NOERROR;

	if (hContext)
	{
	    OSMemFree(hContext);
	}

	pDUSCtx = NULL;
	return(error);
}


/*************************************************************************
	FUNCTION:       DUSTerm

    Comments:
	This call notifies the DUS DLL that it is being terminated/unloaded.
	Any alloced memory not already freed should be freed here.
*************************************************************************/
STATUS LNCALLBACK DUSTerm( void)
{
	/* Not sure if anything needs to be done here in this case. */
	return(NOERROR);
}

/***********************************************************************
FUNCTION:       DUSRegistrationNotify

Purpose:
	Notifies the DUS that Domino and Notes user registration for the user 
	represented by UserName (and originally retrieved from this DUS) is 
	about to start or just finished.  This allows the DUS to take pre or
	post registration action to supplement registration-related activity.

Inputs:
	hContext -- pointer to DUS specific context.  Alloced/provided by
	the DUS in DUSStart and passed back to the DUS with every DUS function.

Inputs:
	UserName -- lmbcs string identifying the user being registered,
		    originally passed in by the DUS with DUSRetrieveUsers.

	hUserNote -- contains user and DUS specific information.

	bAfterEvent -- FALSE if the notification is just prior to user registration,
		       TRUE if the notification is just after.

Comments:
	DUSRegistrationNotify is an optional function, which must be implemented
	but can do nothing if not needed by the DUS.
************************************************************************/
STATUS LNCALLBACK DUSRegistrationNotify(DHANDLE hContext,
					char *UserName,
					NOTEHANDLE hUserNote,
					BOOL bAfterEvent)
{
	/*
	 * Since it is not necessary for to gather any other information during this user registration,
	 * just return from this call.
	 */
	return(NOERROR);
}

/*************************************************************************
FUNCTION:       DUSConvertMailFile

Purpose:
	Converts DUS mail into Domino and Notes mail file.

Inputs:
	UserName -- lmbcs string identifying the user being registered,
		    originally passed in by the DUS with DUSRetrieveUsers.

	MailFilePath -- lmbcs string identifying the fully qualified mail
			file path (including mail server) of the empty 
			Notes mail file to be converted.

	hUserNote -- contains user and DUS specific information.

	hNewPersonNote -- the Note handle of the person document just created
			  by user registration in the Address book.  If the DUS
			  modifies this note it must update the note (NSFNoteUpdate),
			  but the DUS framework will close the note.

	SignalStatus -- address of a function to display status in the
			Status bar of the admin panel (below the user reg
			dialog).  The DUS can use this proc to apprise the
			administrator of mail conversion progress if desired.

Comments:
	DUSConvertMailFile is called after the user has been
	successfully registered and a Domino and Notes mail file (empty)
	has been created.

Outputs:
	Converted mail file (implicit).
*************************************************************************/
STATUS LNCALLBACK DUSConvertMailFile(DHANDLE hContext,
                                     char *UserName,
                                     char *MailFilePath,
                                     NOTEHANDLE hUserNote,
                                     NOTEHANDLE hNewPersonNote,
                                     REGSIGNALPROC SignalStatus)
{

	/*
	 * Since new users are being added from the input file, there is no mail conversion 
	 * necessary.
	 */
	return(NOERROR);

}       /* end of DUSConvertMailFile() */

/****************************************************************************
    FUNCTION:   DUSExtendedErrorText

    Purpose:
	This call retrieves the text to be displayed in the
	event that a call to the DUS extension DLL fails and the dll
	wishes to return it's own error string.  Any call implemented
	in this dll wanting this callback should return ERR_DUS_EXTENDED_ERROR.

	Inputs:
		hContext -- pointer to DUS specific context.  Alloced/provided by the DUS
			    in DUSStart and passed back to the DUS with every DUS function.

		BufferLen -- length of ErrorBuffer

	Outputs:
		ErrorBuffer -- lmbcs string passed to DUS framework for display.

		pErrorLevel -- pointer to one of three error level flags supplied by the DUS
			       (see DUS_ERROR_LEVEL_xxx in dus.h).  This value determines
			       how the error message box will be displayed.  The three flags
			       relating to error level should only be or'd in exclusively (not
			       with each other).  A fourth flag, DUS_ERROR_NO_DISPLAY, prevents
			       display of the error if or'd in.
*******************************************************************************/
void LNCALLBACK DUSExtendedErrorText(DHANDLE hContext,
                                     char * ErrorBuffer,
                                     WORD BufferLen,
                                     WORD *pErrorLevel)
{

	if(hContext)
	    pDUSCtx = OSLock(DUS_CONTEXT, hContext);
	else
	    return;

	if(pDUSCtx->ExtendedError)
	{
	    OSLoadString(pDUSCtx->hDUSModule, pDUSCtx->ExtendedError, ErrorBuffer, BufferLen);
	    *pErrorLevel = pDUSCtx->ExtendedErrorLevel;
	}
	else
	    strcpy(ErrorBuffer, "");

	pDUSCtx->ExtendedError = 0;
	pDUSCtx->ExtendedErrorLevel = 0;

	if(pDUSCtx != NULL)
	{
	    OSUnlock(hContext);
	    pDUSCtx = NULL;
	}
	return;
}
