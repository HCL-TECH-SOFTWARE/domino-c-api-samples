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
 * extcwin.c - Windows DLL framework for extension manager.
 */

	/* Windows header files */
#include "windows.h"

	/* HCL C API for Notes/Domino header files */
#include "global.h"

#/* Program specific include file */
#include "extconf.h"

#if !defined(ND64) 
    #define DHANDLE HANDLE 
#endif

HINSTANCE	hDllInstance = (HINSTANCE) 0;

/*
 *		DLL Management
 */

#if defined (_WIN32)

/* Win32 DLL Startup/Shutdown routine */
DLL_EXPORT_PREFIX BOOL WINAPI DllMain (HANDLE    hInst,
                                       ULONG    dwReason,
                                       LPVOID   lpReserved) 
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            /* Save the instance handle */
            hDllInstance = hInst;
            break;

        case DLL_PROCESS_DETACH:
            /* Deregister extension manager callbacks */
            ExtClear ();
            break;
    }

      return (TRUE);
}

#endif


