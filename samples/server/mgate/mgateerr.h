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
 */

#ifndef MGATE_ERR_DEFS
#define MGATE_ERR_DEFS

#define	PKG_MGATE 0x3300 /* must be the same as PKG_ADDIN in GLOBERR.H */

/*	String #0 must always be the add-in application's task name.
	String #1 must be the version.

	Both of these strings are defined in separate (one per .exe)
	.h files.
*/

#define MSG_MGATE_ADDIN_NAME            (PKG_MGATE+0)
#define MSG_MGATE_ADDIN_VERSION         (PKG_MGATE+1)


/*  Mail Gateway logging message definitions */

#define ERR_MGATE_STARTED               (PKG_MGATE+2)
#define ERR_MGATE_SHUTDOWN              (PKG_MGATE+3)

#define ERR_MGATE_NONDELIVERYSUBJ       (PKG_MGATE+4)
#define ERR_MGATE_MSGLIST               (PKG_MGATE+5)
#define ERR_MGATE_NOUNIQUE              (PKG_MGATE+6)
#define ERR_MGATE_RECIPCREATE           (PKG_MGATE+7)
#define ERR_MGATE_BODYFILE              (PKG_MGATE+8)
#define ERR_MGATE_MSGOPEN               (PKG_MGATE+9)
#define ERR_MGATE_SENDSUCCESS           (PKG_MGATE+10)
#define ERR_MGATE_SENDERROR             (PKG_MGATE+11)
#define ERR_MGATE_UNKNOWNITEM           (PKG_MGATE+12)
#define ERR_MGATE_TRANSFERSUCCESS       (PKG_MGATE+13)
#define ERR_MGATE_TRANSFERERROR         (PKG_MGATE+14)
#define ERR_MGATE_MSGFILE_OPEN          (PKG_MGATE+17)
#define ERR_MGATE_TEXT_OPEN             (PKG_MGATE+18)
#define ERR_MGATE_MSGFILE_CREATE        (PKG_MGATE+19)
#define ERR_MGATE_ATT_CREATE            (PKG_MGATE+20)
#define ERR_MGATE_ATT_WRITE             (PKG_MGATE+21)
#define ERR_MGATE_ATT_ADD               (PKG_MGATE+22)
#define ERR_MGATE_NODOMAIN              (PKG_MGATE+32)
#define ERR_MGATE_NODRIVE               (PKG_MGATE+33)

/*  Gateway status definitions (for server console display) */

#define ERR_MGATE_INITIALIZING          PKG_MGATE+100
#define ERR_MGATE_IDLE                  PKG_MGATE+101
#define ERR_MGATE_RECEIVING             PKG_MGATE+102
#define ERR_MGATE_TRANSFERING           PKG_MGATE+104
#define ERR_MGATE_TRANSFERING_TO        PKG_MGATE+105

#endif

