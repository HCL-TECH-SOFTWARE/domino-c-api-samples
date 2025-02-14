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
 *
     SENDMAIL.H

***************************************************************************/

#define TITLE_LEN            30
#define ERR_STR_LEN         256
#define SENDMAILERROR       1

/* constants */
#define     READMAIL_BODY_LINELEN   40
char szDriveLetter_blank[] = "";
char szExtensionName_TMP[] = "TMP";
char szNewline_Terminate[] = "\r\n";

void  LNPUBLIC  ProcessArgs(int, char*[], char *, char *, char *, char *, char *);
int SendMailMainProc();
int QuitSendMail();
int SendMail();
void LNPUBLIC ConvertNewlinesToNulls(char *, int);
char *BuildErrStr(char *);
STATUS near pascal GetUniqueFileName(char *Drive, char *Ext,
                                        char *FileName);
STATUS PrintMail(char *szServerName);
