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

    PROGRAM: addmacro

    FILE:    addmacro.h

****************************************************************************/

/* Constants */
#define  MAX_FLAGS   32

/* Function Prototypes */
STATUS  LNPUBLIC  AddFilterMacro( DBHANDLE );
STATUS  LNPUBLIC  AddBackgroundMacro( DBHANDLE );
STATUS  LNPUBLIC  AddOnceMacro( DBHANDLE );
STATUS  LNPUBLIC  CreateMacroNote ( DBHANDLE, NOTEHANDLE* );
STATUS  LNPUBLIC  SetMacroTitle( NOTEHANDLE, char * );
STATUS  LNPUBLIC  SetMacroComment( NOTEHANDLE, char * );
STATUS  LNPUBLIC  SetMacroFormula( NOTEHANDLE, char * );
STATUS  LNPUBLIC  SetMacroType( NOTEHANDLE, WORD );
STATUS  LNPUBLIC  SetMacroOperation( NOTEHANDLE, WORD );
STATUS  LNPUBLIC  SetMacroScan( NOTEHANDLE, WORD );
STATUS  LNPUBLIC  SetMacroFlags( NOTEHANDLE, char * );
STATUS  LNPUBLIC  SetMacroMachineName( NOTEHANDLE );
STATUS  LNPUBLIC  SetMacroPeriod( NOTEHANDLE, WORD );
STATUS  LNPUBLIC  SetMacroLeftToDo( DBHANDLE, NOTEHANDLE );


