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

 * AUTH_FLD.H
 */

#ifndef __AUTH_FLD_H
#define __AUTH_FLD_H


/* Function prototypes */
STATUS  LNPUBLIC  OpenNotesDB (char *, char *, DBHANDLE *);
STATUS  LNPUBLIC  GetFileFirstLine (char *, char ** );
STATUS  LNPUBLIC  CreateMainTopic (DBHANDLE, char *, char *, OID *);
STATUS  LNPUBLIC  CreateResponse (DBHANDLE, char *, char *, char *, OID);
STATUS  LNPUBLIC  AppendFormItem (NOTEHANDLE, char *);
STATUS  LNPUBLIC  AppendAuthorItem (NOTEHANDLE, char *);
STATUS  LNPUBLIC  AppendEditorsItem (NOTEHANDLE);
STATUS  LNPUBLIC  AppendReadersItem (NOTEHANDLE);
STATUS  LNPUBLIC  AppendCategoriesItem (NOTEHANDLE);
STATUS  LNPUBLIC  AppendBodyItem (NOTEHANDLE, char *);
STATUS  LNPUBLIC  AppendSubjectItem (NOTEHANDLE, char *);
STATUS  LNPUBLIC  AppendUpdatedByItem (NOTEHANDLE, char *);
STATUS  LNPUBLIC  AppendRefItem (NOTEHANDLE, OID);
STATUS  LNPUBLIC  AppendOriginalSubjectItem (NOTEHANDLE, char *);

/* Local Defines */
#define FIRST_LINE_MAX_SIZE     (1024*15)

/* Error codes returned from subroutines in this program */
#define ERR_AUTH_FLD_DBOPEN     (PKG_ADDIN + 1)
#define ERR_AUTH_FLD_FILEOPEN   (PKG_ADDIN + 2)
#define ERR_AUTH_FLD_MALLOC     (PKG_ADDIN + 3)
#define ERR_AUTH_FLD_EOF        (PKG_ADDIN + 4)
#define ERR_AUTH_FLD_READ_ERROR (PKG_ADDIN + 5)

/* Item names. These depend on the design of the forms in the DISCUSS.NSF
   database.
 */
#define DISCUSS_ITEM_FORM               FIELD_FORM  /* "Form" */
#define DISCUSS_ITEM_AUTHOR             "Author"
#define DISCUSS_ITEM_EDITORS            "Editors"
#define DISCUSS_ITEM_READERS            "Readers"
#define DISCUSS_ITEM_CATEGORIES         "Categories"
char    szDISCUSS_ITEM_BODY[] = "Body";
#define DISCUSS_ITEM_SUBJECT            "Subject"
#define DISCUSS_ITEM_UPDATEDBY          FIELD_UPDATED_BY /* "$UpdatedBy" */
#define DISCUSS_ITEM_ORIGINALSUBJECT    "OriginalSubject"

/* Item values */
char    szMAINTOPIC_FORM[]   = "MainTopic";
char    szMAINTOPIC_AUTHOR[] = "CN=Stephen N. Phillips/O=IBM Corporation";
char    szRESPONSE_FORM[]    = "Response";
char    szRESPONSE_AUTHOR[]  = "CN=Shelby T. Williams/O=IBM Corporation";

#define EDITORS1        "NADC Executives"
#define EDITORS2        "NADC Marketing"
#define EDITORS_COUNT   2
#define READERS1        "CN=Jane Doe/OU=Sales/O=ABCorp"
#define READERS2        "NADC Manufacturing"
#define READERS_COUNT   2
#define CATEGORIES      "Using Notes"

#define  STRING_LENGTH  256

void  LNPUBLIC  ProcessArgs (int argc, char *argv[],
                               char *ServerName, char *DbPathName,
                               char *MainFileName, char *RespFileName);

#endif
