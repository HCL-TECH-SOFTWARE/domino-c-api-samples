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
 *

    HEADER:    printLog

    FILE:      printLog.h

    DESCRIPTION:
               This header file is used for common error logging,
               which helps developers, or user to toggle the logging
               to file or to console based on the OS environment variable.

               ENV variable,
               CAPI_FILE_LOGGING - To toggle file logging or console (by default).
                                   For file logging, set value of this env to true and set value to false for console logging.
               CAPI_LOG_PATH     - Log file directory.
                                   Consider the current folder as logging, if this env is not set.

Setting ENV:
     WINDOWS:
       >set CAPI_FILE_LOGGING=true
       >set CAPI_LOG_PATH=D:\CAPI\notes\Log\

     Linux:
       >export CAPI_FILE_LOGGING=true
       >export CAPI_LOG_PATH=/opt/hcl/domino/notesAPI/

Note: If CAPI_FILE_LOGGING env is not set then it is considered as console logging.
****************************************************************************/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>

#define CONSOLE_LOGGING        "false"
#define FILE_LOGGING           "true"
#define LOG_STRING_LENGTH  256

/* This macro is used to convert upper case to lower case char */
#define TOLOWER(inString, outString) \
{ \
   int i = 0; \
   while (inString != NULL && *inString != '\0') {\
         outString[i]=tolower(*inString); \
	 inString++; \
	 i++; \
   } \
}


#define PRINTERROR(api_error,api_name)\
{\
     FILE *logFP = stdout;\
     STATUS  string_id = ERR(api_error);\
     char    szErrorText[256] = { 0 };\
     WORD    wTextLen = 0;\
     char *__filePtr__=__FILE__;\
     if (logFP) {\
     /* Get the message for this error code from the resource string table.*/\
         wTextLen = OSLoadString(NULLHANDLE,\
         string_id,\
         szErrorText,\
         sizeof(szErrorText));\
     /* Print error message. */\
         fprintf(logFP, "[ERROR]:%s:%d:%s - %s", __filePtr__,__LINE__,api_name,szErrorText);\
     }\
}


#define PRINTLOG(...) \
{\
     char logFileName[LOG_STRING_LENGTH]={0}; /*to store log file name */\
     FILE *logFP = stdout; \
     short isLogFile = 0; \
	/* CAPI_FILE_LOGGING is ENV variable to toggle file logging or console (by default) */\
	/* CAPI_FILE_LOGGING = true -- FILE log is enabled */\
	/* CAPI_FILE_LOGGING = false -- CONSOLE Log is enabled */\
	/* CAPI_FILE_LOGGING not set then default is CONSOLE log */\
     char *cpFlag = NULL;\
     cpFlag=getenv("CAPI_FILE_LOGGING");\
     if (cpFlag) {\
         char *__filePtr__=__FILE__;\
         char lcpFlag[LOG_STRING_LENGTH]={0}; \
         strncpy(logFileName, __filePtr__, strstr(__filePtr__, ".c")-__filePtr__); \
         TOLOWER(cpFlag, lcpFlag) \
         if (!strcmp(lcpFlag, FILE_LOGGING)) {\
               char path[LOG_STRING_LENGTH] = {0}; \
               char *sEnvPath = getenv("CAPI_LOG_PATH"); \
               if (sEnvPath) strncat(path, sEnvPath, sizeof(path)); \
               strncat(path, logFileName, sizeof(path)-1); \
               strncat(path, "_actual.log", sizeof(path)-1);\
               logFP = fopen(path, "a"); \
               if (!logFP) {\
                   printf("\n Error in opening the log file!!");\
                   printf("\n Using Console logging by default.");\
               }\
	       else {\
                  isLogFile=1;\
	       }\
          }\
     }\
     if (logFP) {\
        fprintf(logFP, __VA_ARGS__); \
        if (isLogFile) {\
           fclose(logFP);\
        }\
        else {\
           fflush(logFP);\
        }\
     }\
}
