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

 * MAKEFORM.H
 */

#ifndef __MAKEFORM_H
#define __MAKEFORM_H

#define CD_BUFFER_LENGTH       64000

/* forward declarations for CD manipulation functions  */

BOOL LNPUBLIC PutPara (char far * far *ppBuf,
                         WORD wLength);
                               
BOOL LNPUBLIC PutPabDef(char far * far *ppBuf,
                         WORD wPabDefNumber,
                         WORD wLength,
                         WORD wJustifyMode);
                                
BOOL LNPUBLIC PutPabRef(char far * far *ppBuf,
                          WORD wPabDefNumber,
                          WORD wLength);
                                
BOOL LNPUBLIC PutText(char far * far *ppBuf,
                        char far * pText,
                        WORD wLength);

STATUS LNPUBLIC CreateForm(void);
STATUS LNPUBLIC CreateSubForm(void);
STATUS LNPUBLIC CreateFrame(void);
STATUS LNPUBLIC GetNoteLinkInfo(void);

char far * ImportGifImage(char *ImageFileName, char far *pBuf, WORD width, WORD height);
char far * AddFrame(char far * far *pBuf, char far *pBufferStart);
char far * AddNoteLink(char far * far *pBuf, char far *pBufferStart);
                            
#endif




