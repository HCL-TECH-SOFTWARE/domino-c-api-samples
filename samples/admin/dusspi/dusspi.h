/********************************************************************************************
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
 *
 *	PROGRAM:		DUSSPI.DLL
 *	
 *	MODULE:			DUSSPI.H
 *
 *	DESCRIPTION:	Constant definitions for DUS SPI DLL DUSSPI.DLL.
 *
 ********************************************************************************************
 */
typedef struct
{
	HMODULE                 hDUSModule;			/* Instance handle to this DLL */
	WORD                    ExtendedError;		/* User defined error, defined in duspierr.h */
	WORD                    ExtendedErrorLevel;	/* DUS_ERROR_LEVEL_xxx in DUSExtendedErrorText */
	DUSPROGRESSBARPROC      ProgressBarProc;	/* Progress Bar function pointer passed in with DUSStart */
	DUSLOGEVENTPROC         LogEventProc;		/* Log Event function pointer passes in with DUSStart */
}DUS_CONTEXT, *PDUS_CONTEXT;

#define CHAR_CR 13	
#define CHAR_LF 10
#define CHAR_SPC 32
#define CHAR_COMMA 44

