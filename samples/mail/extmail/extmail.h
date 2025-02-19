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
 */

#ifndef _EXTMAILH
#define _EXTMAILH
#endif
/*
	extmail.H
*/

/*
	REQUIRED WHEN COMPILING SAMPLE FOR WINDOWS
*/
#include <windows.h>
#include <string.h>
#include <stdio.h>

/*
	LOTUS C API FOR DOMINO AND NOTES HEADER FILES
*/

#include <global.h>
#include <event.h>
#include <kfm.h>
#include <nsfnote.h>
#include <nsferr.h>
#include <nsfdb.h>
#include <nsfdata.h>
#include <nsfnote.h>
#include <osmisc.h>
#include <misc.h>
#include <ostime.h>
#include <extmgr.h>
#include <OSfile.h>

/*============ GLOBAL DEFINITIONS =========================*/

/* Log file containing output from sample */

#define LOG_FILE_NAME "\\EXTMAIL.LOG"

/* #define DLL_EXPORT	__declspec( dllexport ) */
#define WRITE_PERMISSION			"w+"
#define APPEND_PERMISSION			"a+"
#define READ_PERMISSION		        "r"
#define DLL_EXPORT

/*---------------------------------------------------------
	PLATFORM SPECIFIC MACROS FOR FILE AND MEMORY FUNCTIONS
-----------------------------------------------------------*/
#define DLL_IMPORT_PREFIX  __declspec (dllimport)
#define DLL_EXPORT_PREFIX  __declspec (dllexport)
#define DLL_IMPORT_INFIX   
#define DLL_EXPORT_INFIX

/* Memory Allocation */
#define __AllocateMemory(__sz)			malloc( __sz )
#define __FreeMemory(__ptr)				free( __ptr )
#define __sprintf						sprintf


/* File Managment */
#define __FILE 							        FILE
#define __OpenFile( _fn, _perm )		        fopen(_fn, _perm )
#define __CloseFile( _file )			        fclose( _file )
#define __WriteLine( _buf, _file )		        fwrite( _buf, strlen(_buf), 1, _file )
#define __FileError(_file)				        ferror(_file)
#define __ReadLine( _buf, _len, _f )			fgets( _buf, _len, _f )
#define __FileSeek( _file, _offset, _whence )	fseek( _file, _offset, _whence )


