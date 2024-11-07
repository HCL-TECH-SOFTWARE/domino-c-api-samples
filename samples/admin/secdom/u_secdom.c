/*************************************************************************
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

    PROGRAM:    SECDOM

    FILE:       U_SECDOM.C (Unix specific code)

    PURPOSE:    C API Sample program that illustrates how to create a
	             library that will, from the web, authenticate a Domino
					 user through his Operating System user account via DSAPI.
					
*************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

/* Inpbut and output include files */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* unix authentication includes */
#ifdef LINUX
#include <shadow.h>
#include <crypt.h>
#endif

#ifdef AIX
#include <sys/types.h>
#include <pwd.h>
#endif


int unixAuthenticate(char *userName, char *password)
{

char buffer[1024];
int error = -1;
int success = 0;
int unknown = 1;
		
/* Get the unix record for this user */


#ifdef LINUX
    struct spwd result;
#endif

#ifdef AIX
    struct passwd *result;
#endif


/* Get the unix record for this user */

#ifdef LINUX

if (getspnam_r(userName, &result, buffer, sizeof(buffer))) 
{
/* Encrypt the password and see if it matches the
 * encrypted password from the user's record.
 */
    char *thisCrypt = NULL;
    thisCrypt = (char *)crypt(password, result.sp_pwdp);
    if (strcmp (result.sp_pwdp, thisCrypt) == 0) {
        return success;
    } else {
        return error;
    }
}
#endif

#ifdef AIX
result = getpwnam(userName);
if (result && result->pw_passwd)
{
/* Encrypt the password and see if it matches the
 * encrypted password from the user's record.
 */
    char *thisCrypt = NULL;
    thisCrypt = (char *)crypt(password,
                                  result->pw_passwd);
    if (strcmp (result->pw_passwd, thisCrypt) == 0)
    {
        return success;
    }
    else
    {
        return error;
    }
}
#endif


return unknown;
}

#ifdef __cplusplus
}
#endif
