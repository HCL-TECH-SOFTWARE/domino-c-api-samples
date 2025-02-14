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
#define INCL_BASE
#include <direct.h>
#include <windows.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <io.h>
#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>

#include "global.h"

#ifndef NAME_DEFS
#include "names.h"						
#endif

#define DEFAULT_DELIVERYINTERVAL 3                      /* Delivery interval (minutes) */
#define MGATE_MSGFILE_NAME "MGATE.NSF"                  /* Message file name */
#define MGATE_INBOUND_DIR "\\MGATE\\INBOUND"            /* Inbound message directory */
#define MGATE_OUTBOUND_DIR "\\MGATE\\USER"              /* Outbound message directory (contains user directories) */
#define MGATE_MSG_EXT "MSG"                             /* Message file extension */
#define MGATE_ATT_EXT "ATT"                             /* Message attachment extension */

/* Configuration (NOTES.INI) variable names */

#define MGATE_DOMAIN "MGateDomain"                      /* Foreign mail domain name */
#define MGATE_DRIVE "MGateDrive"                        /* Inbound/outbound disk drive letter */
#define MGATE_INTERVAL "MGateInterval"                  /* Polling interval (minutes) */

/* Static variables */

HANDLE near hModule;                                    /* Mail program module handle */
DHANDLE near hMessageFile;                              /* Outbound message file handle */
char near GatewayDomain[MAXDOMAINNAME+1];               /* Foreign gateway domain name */
char near GatewayDrive[3];                              /* Drive letter to for gateway's use */

/* INBOUND.C */

void near pascal InboundTask(void);

/* OUTBOUND.C */

void near pascal OutboundTask(void);
STATUS near pascal GetUniqueFileName(char *Drive, char *Dir, char *Ext, char *FileName);

