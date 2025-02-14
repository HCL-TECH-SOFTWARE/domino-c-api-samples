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

#define HISTORY_NAME                       (PKG_ADDIN+0)
#define HISTORY_VERSION                    (PKG_ADDIN+1)
#define DEFAULT_HISTORY_DBFILENAME         (PKG_ADDIN+2)
#define DEFAULT_HISTORY_DBSERVER           (PKG_ADDIN+3)
#define HISTORY_RESP_AUTHOR_STAT_TXT       (PKG_ADDIN+4)
#define HISTORY_RESP_DATE_STAT_TXT         (PKG_ADDIN+5)
#define HISTORY_RESP_TOPIC_STAT_TXT        (PKG_ADDIN+6)
#define HISTORY_RESP_COMMENTS_STAT_TXT     (PKG_ADDIN+7)
#define ERR_HISTORY_CANTINIT               (PKG_ADDIN+8)
#define ERR_HISTORY_DBPATH                 (PKG_ADDIN+9)
#define ERR_HISTORY_PATHTOOLONG            (PKG_ADDIN+10)
#define ERR_HISTORY_CANTOPENDEFDB          (PKG_ADDIN+11)
#define ERR_HISTORY_CANTOPENDB             (PKG_ADDIN+12)
#define ERR_HISTORY_NBFDBINFO              (PKG_ADDIN+13)
#define ERR_HISTORY_MAKEPROC               (PKG_ADDIN+14)
#define ERR_HISTORY_SEARCH                 (PKG_ADDIN+15)
#define ERR_HISTORY_OPENVIEW               (PKG_ADDIN+16)
#define ERR_HISTORY_READNEXT               (PKG_ADDIN+17)
#define ERR_HISTORY_EMPTYVIEW              (PKG_ADDIN+18)
#define ERR_HISTORY_READRESP               (PKG_ADDIN+19)
#define ERR_HISTORY_SKIPRESP               (PKG_ADDIN+20)
#define ERR_HISTORY_SKIPCAT                (PKG_ADDIN+21)
#define ERR_HISTORY_OPENMAIN               (PKG_ADDIN+22)
#define ERR_HISTORY_UPDATE                 (PKG_ADDIN+23)
#define ERR_HISTORY_SUCCESS                (PKG_ADDIN+24)
#define ERR_HISTORY_FAILURE                (PKG_ADDIN+25)
#define ERR_HISTORY_NORESPONSE             (PKG_ADDIN+26)
#define ERR_HISTORY_CONTEXT                (PKG_ADDIN+27)
#define ERR_HISTORY_STYLE                  (PKG_ADDIN+28)
#define ERR_HISTORY_OLDHISTORY             (PKG_ADDIN+29)
#define ERR_HISTORY_DELETE                 (PKG_ADDIN+30)
#define ERR_HISTORY_RESPONSE               (PKG_ADDIN+31)
#define ERR_HISTORY_ASSIMRESP              (PKG_ADDIN+32)
#define ERR_HISTORY_DATEITEM               (PKG_ADDIN+33)
#define ERR_HISTORY_AUTHSTAT               (PKG_ADDIN+34)
#define ERR_HISTORY_FROMITEMINFO           (PKG_ADDIN+35)
#define ERR_HISTORY_FROMITEMTEXT           (PKG_ADDIN+36)
#define ERR_HISTORY_AUTHITEM               (PKG_ADDIN+37)
#define ERR_HISTORY_TOPICSTAT              (PKG_ADDIN+38)
#define ERR_HISTORY_TOPICITEMINFO          (PKG_ADDIN+39)
#define ERR_HISTORY_TOPICITEMTEXT          (PKG_ADDIN+40)
#define ERR_HISTORY_COMMNTSTAT             (PKG_ADDIN+41)
#define ERR_HISTORY_DELETEHIST             (PKG_ADDIN+42)
#define ERR_HISTORY_CLOSECOMP              (PKG_ADDIN+43)
#define ERR_HISTORY_APPENDHIST             (PKG_ADDIN+44)
#define ERR_HISTORY_VIEWTITLE              (PKG_ADDIN+45)
#define ERR_HISTORY_ADDVIEW                (PKG_ADDIN+46)
#define ERR_HISTORY_ADDVIEWSPACE           (PKG_ADDIN+47)
#define ERR_HISTORY_GETSELVIEWID           (PKG_ADDIN+48)
#define ERR_HISTORY_LINKSINFO              (PKG_ADDIN+49)
#define ERR_HISTORY_BODYINFO               (PKG_ADDIN+50)
#define ERR_HISTORY_MALLOC                 (PKG_ADDIN+51)
#define ERR_HISTORY_DELOLDBODY             (PKG_ADDIN+52)
#define ERR_HISTORY_APPENDBODY             (PKG_ADDIN+53)
#define ERR_HISTORY_CDMALLOC               (PKG_ADDIN+54)
#define ERR_HISTORY_ENUMBODY               (PKG_ADDIN+55)
#define ERR_HISTORY_TOOMANYRESBDY          (PKG_ADDIN+56)
#define ERR_HISTORY_NEWRESBDY              (PKG_ADDIN+57)
#define ERR_HISTORY_NEWCONTXT              (PKG_ADDIN+58)
#define ERR_HISTORY_ASSIMFILE              (PKG_ADDIN+59)
#define ERR_HISTORY_CLOSENEW               (PKG_ADDIN+60)
