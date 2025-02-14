/****************************************************************************
 *
 * Copyright HCL Technologies 1996, 2024.
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

    PROGRAM:   dxlimport

    MODULE:    dxlimport.h

    DESCRIPTION:
	Constant definitions and function prototypes for API program dxlimport

****************************************************************************/
#include <sys/stat.h>

struct ImportProperties
{
	int		aclOpt;			/* ACL Option value defined in DXLIMPORTOPTION */	
	int		designOpt;		/* Design option valued defined in DXLIMPORTOPTION */
	int		documentsOpt;	/* Documents option value defined in DXLIMPORTOPTION */
	BOOL	createFullText;	/* Set if changing iCreateFullTextIndex option */ 
	BOOL	replaceDbProp;	/* Set if changing iReplaceDbProperties */
	int		inputValOpt;	/* Input validation option value defined in Xml_Validation_Option */
	BOOL	replicaRequiredForReplaceOrUpdate; /* Set if changin iReplcaRequiredForReplaceOrUpdate */
	BOOL	exitOnFirstFatalError; /* Set if changing iExitOnFirstFatalError */
	int		tokenLogOpt;	/* Unknown Token Log value defined in DXLLOGOPTION */
} Properties;

/* Structure contianing information needed before Importing */
struct ImportOptions
{
	char	*fileDatabase;			/* NSF Database to import to */
	char	*pInputFile;			/* Pointer to what is being used as an XML Input Source */
	char	*serverName;			/* Server Name if supplied */
	struct	ImportProperties	Properties;		/* Possible Import Properties to set */
	
}impOptions;

/* Sturcture containing information while importing data */
struct ImportContext
{
	FILE	*dxlInputFile;		/* FILE pointer if XML Input Source is a FILE */
	char	*dxlFileName;		/* User requested input file name */
	int		InputFDHandle;
	long	InputFDSize;
	DWORD	bytesRemaining;		/* Bytes remaining from XML Input Source */
	struct	stat buf;

}impCtx;

enum DXLI_ReturnCodes
{
	DXLI_RET_NOERROR,			/* no errors */
	DXLI_RET_NO_ARGS,			/* no command line args */
	DXLI_RET_INVALID_ARGS		/* invalid command line */
};


/* Local function prototypes */
void    LNPUBLIC  PrintUsage();
void PrintAPIError (STATUS);
STATUS LNPUBLIC ProcessArgs (int argc, char *argv[], struct ImportOptions *impOptions);
STATUS LNPUBLIC ImportXMLData( DBHANDLE hDB, struct ImportOptions *impOptions); 
DWORD LNCALLBACK DXLReaderFunc(unsigned char *pBuffer, const DWORD MaxToRead, void far *pImAction);
