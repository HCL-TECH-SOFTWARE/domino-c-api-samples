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

 * CDRecord.h  local include file for the large graphic import example
 */

#ifndef __IMPORT_H
#define __IMPORT_H



#define CD_BUFFER_LENGTH       64000
#define CD_HIGH_WATER_MARK     40000
 
/*
 *  Error code and error message text to return if the call to 
 *  AppendImportItem fails.
 */
 
#define ERR_APPEND_RICHTEXT_ERROR  PKG_ODS+50     
errortext (ERR_APPEND_RICHTEXT_ERROR, "Error appending rich text item.");


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
#endif




