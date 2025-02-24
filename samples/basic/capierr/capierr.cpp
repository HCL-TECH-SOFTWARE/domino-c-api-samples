/*
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
*
*CapiErr.cpp : Defines the class behaviors for the application.
*/

#include "stdafx.h"
#include "CapiErr.h"
#include "CapiErrD.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCapiErrApp

BEGIN_MESSAGE_MAP(CCapiErrApp, CWinApp)
   //{{AFX_MSG_MAP(CCapiErrApp)
        // NOTE - the ClassWizard will add and remove mapping macros here.
        //    DO NOT EDIT what you see in these blocks of generated code!
    //}}AFX_MSG
    ON_COMMAND(ID_HELP, CWinApp::OnHelp)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCapiErrApp construction

CCapiErrApp::CCapiErrApp()
{
}

/////////////////////////////////////////////////////////////////////////////
// The one and only CCapiErrApp object

CCapiErrApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CCapiErrApp initialization

BOOL CCapiErrApp::InitInstance()
{
    // Standard initialization
    CCapiErrD dlg;
    m_pMainWnd = &dlg;
    dlg.DoModal();
    
    // Since the dialog has been closed, return FALSE so that we exit the
    //  application, rather than start the application's message pump.
    return FALSE;
}
