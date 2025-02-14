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


 * HOTSPOT.H
 */

#ifndef __HOTSPOT_H
#define __HOTSPOT_H

/* forward declarations for CD manipulation functions  */

BOOL PutPara (char far * far *ppBuf,
                         WORD wLength);

BOOL PutText(char far * far *ppBuf,
                        char far * pText,
                        WORD wLength, int Type);

STATUS PutButton(char far * far *pbRTItem,
                            char far * pButtonText,
                            WORD wLength,
                            WORD far *pwRTLength);

STATUS PutBar(char far * far *pbRTItem,
                            char far * pBarText,
                            WORD wLength,
                            WORD far *pwRTLength);

STATUS PutAction(char FAR * FAR *pbRTItem,
                            char * pToInfo,
                            char * pSubjectField,
                            char * pBodyField,
                            WORD wLength,
                            WORD FAR *pwRTLength);

STATUS PutHotSpotEnd(char far * far *pbRTItem,
                                WORD wLength,
                                WORD far *pwRTLength);

STATUS PutV4HotSpotEnd(char far * far *pbRTItem,
                                WORD wLength,
                                WORD far *pwRTLength);

STATUS PutPabRef(char far * far *pbRTItem,
                            WORD wPabDefNumber,
                            WORD wLength,
                            WORD *pwRTLength);

/*  Error message definitions */

#define ERR_CREATING_RICHTEXT   (PKG_ADDIN + 10)

errortext(ERR_CREATING_RICHTEXT, "Error creating rich text item.")

#endif

