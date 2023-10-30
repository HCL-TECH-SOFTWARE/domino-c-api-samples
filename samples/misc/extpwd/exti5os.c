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
*/

#if defined(OS400)
#pragma convert(850) 
#endif  

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#if defined (OS400)
#include <unistd.h>
#endif

#include <global.h>
#include <names.h>
#include <extmgr.h>
#include <bsafeerr.h>
#include <nsferr.h>

HEMREGISTRATION hHandler;

STATUS LNPUBLIC handler(EMRECORD *data);
STATUS LNPUBLIC MainEntryPoint();

STATUS LNPUBLIC handler(EMRECORD *data)
{
    VARARG_PTR              pArgs;
    DWORD                   MaxPwdLen;
    DWORD far *             retLength;
    char far *              retPassword;
    char far *              FileName;
    char far *              OwnerName;
    DWORD                   DataLen;
    BYTE far *              Data;
    
    FILE *fptr;
    char line[MAX_ID_PASSWORD], DataPath[MAXPATH];
    int len;

    if (EM_GETPASSWORD != data->EId)
            return (ERR_EM_CONTINUE);

    /* Fetch the arguments */
    VARARG_COPY(pArgs, data->Ap);

    MaxPwdLen = VARARG_GET (pArgs, DWORD);
    retLength = VARARG_GET (pArgs, DWORD far *);
    retPassword = VARARG_GET (pArgs, char far *);
    FileName = VARARG_GET (pArgs, char far *);
    OwnerName = VARARG_GET (pArgs, char far *);
    DataLen = VARARG_GET (pArgs, DWORD);
    Data = VARARG_GET (pArgs, BYTE far *);

    if ((retLength != NULL) && (retPassword != NULL))
    {
        len = OSGetDataDirectory(DataPath);
        strcat(DataPath, "/");
        strcat(DataPath, "password.txt");
        
        if ((fptr = fopen(DataPath, "r")) == NULL)
            return ERR_EM_CONTINUE;
        else
        {
            fopen("hehe3.hehe", "w");
            fgets(line, MAX_ID_PASSWORD, fptr);
            len = strlen(line);
            *retLength = len;
            strcpy(retPassword, line);
        }
        fclose(fptr);
        
        return ERR_BSAFE_EXTERNAL_PASSWORD;
    }

    return ERR_EM_CONTINUE;
}

STATUS LNPUBLIC MainEntryPoint()
{
    STATUS err;

    if(err=EMRegister(EM_GETPASSWORD, EM_REG_BEFORE, handler,0, &hHandler))
        fprintf(stderr, "EMRegister failed.\n");

    return err;
}
#ifdef __cplusplus
}
#endif
