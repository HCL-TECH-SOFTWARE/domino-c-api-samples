/*
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
 * extpwd.c - Source for extension manager password override program
 */

/*       "C" header files */
#include "string.h"

/*       Platform header files */
#if defined (NT)
#include "windows.h"
#endif

/*       HCL C API for Notes/Domino header files */
#include "global.h"
#include "extmgr.h"
#include "bsafeerr.h"
#include "nsferr.h"

/*       Program header files */
#include "extpwd.h"

/*
 *       Global Data
 */

HEMREGISTRATION hHandler = 0;

/*
 *        Architecture
 *        ------------
 *
 *        An Extension Manager DLL contains the following major components:
 *
 *        1)  DLL startup and shutdown (in platform DLL file, e.g., extwin.c):
 *            Under Win32:   the -DllEntryPoint function, here DllMain
 *
 *        2)  Domino and Notes entry point at ordinal 1:  MainEntryPoint
 *
 *        3)  Extension manager callback:  ExtHandler
 */

/* Extension manager entry point */
DLL_EXPORT_PREFIX STATUS LNPUBLIC DLL_EXPORT_INFIX MainEntryPoint (void) 
{
    STATUS                status;
    char                msgBuf [256];

    status = EMRegister (
                         EM_GETPASSWORD,
                         EM_REG_BEFORE,
                         ExtHandler,
                         0,
                         &hHandler);

    if (NOERROR != status)
    {
#if defined (NT)
    wsprintf (msgBuf, "Could not register extension handler - status: 0x%lX",
              status);
#else
    sprintf (msgBuf, "Could not register extension handler - status: 0x%lX",
             status);
#endif
    ExtDisplayMessageBox (msgBuf, "ExtMgr Password");
    }

    return (status);
}

/* Deregister extensions */
DLL_EXPORT_PREFIX STATUS LNPUBLIC DLL_EXPORT_INFIX ExtClear (void) 
{
    STATUS                status;

    if (0 != hHandler)
        status = EMDeregister (hHandler);
    else
        status = NOERROR;

    return (status);
}

/* Master callback handler */
STATUS LNCALLBACK ExtHandler (EMRECORD far *        pRecord) 
{
    VARARG_PTR            pArgs;
    int                   answer;

    /* Function arguments */
    DWORD                 MaxPwdLen;
    DWORD far *           retLength;
    char far *            retPassword;
    char far *            FileName;
    char far *            OwnerName;
    //DWORD                 DataLen;
    //BYTE far *            Data;

    if (EM_GETPASSWORD != pRecord->EId)
        return (ERR_EM_CONTINUE);

    /* check error status */
    if (pRecord->Status != NOERROR)
        return (ERR_EM_CONTINUE);

    /* Fetch the arguments */
    pArgs = pRecord->Ap;
    MaxPwdLen = VARARG_GET (pArgs, DWORD);
    retLength = VARARG_GET (pArgs, DWORD far *);
    retPassword = VARARG_GET (pArgs, char far *);
    FileName = VARARG_GET (pArgs, char far *);
    OwnerName = VARARG_GET (pArgs, char far *);
    //DataLen = VARARG_GET (pArgs, DWORD);
    // Data = VARARG_GET (pArgs, BYTE far *);

    /* Use the current password */
    if ((NULL != retLength)
        && (NULL != retPassword))
    {
        answer = ExtGetPasswordText (
                                     MaxPwdLen,
                                     FileName,
                                     OwnerName,
                                     retLength,
                                     retPassword);
        
        switch (answer)
        {
            case DLG_ANSWER_OK:
                return (ERR_BSAFE_EXTERNAL_PASSWORD);

            case DLG_ANSWER_CANCEL:
                return (ERR_BSAFE_USER_ABORT);

            default:
                return (ERR_BSAFE_PASSWORD_REQUIRED);
        }
    }

    return (ERR_EM_CONTINUE);
}
