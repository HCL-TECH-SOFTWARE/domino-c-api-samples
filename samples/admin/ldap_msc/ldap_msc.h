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

    PROGRAM:      ldap_msc

    FILE:         ldap_msc.h

    DESCRIPTION:	Header file containing definitions for the API program
	               'ldap_msc'.

****************************************************************************/

#include "global.h"
#include "misc.h"
#include "osmisc.h"

#include "ldap.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define PORT         389
#define FILTER       "(cn=j*)"

/*#define MOD_DN       "CN=John Public, O=HCLPNP"*/
#define MOD_DN       "CN=testuser1"
/*#define MOD_NDN      "CN=John Q Public, O=HCLPNP"*/
#define MOD_NDN      "CN=testuser2"
/*#define MOD_NRDN     "CN=John Q Public"*/
#define MOD_NRDN     "CN=testuser2"
