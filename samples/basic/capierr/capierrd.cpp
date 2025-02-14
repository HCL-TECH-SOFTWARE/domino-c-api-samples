/*
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
*
* CapiErrD.cpp : implementation file
*/

#include "stdafx.h"
#include "CapiErr.h"
#include "CapiErrD.h"

#include "global.h"
#include "osmisc.h"
#include "nsfdb.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCapiErrD dialog

CCapiErrD::CCapiErrD(CWnd* pParent /*=NULL*/)
	: CDialog(CCapiErrD::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCapiErrD)
	m_errCode = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CCapiErrD::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCapiErrD)
	DDX_Text(pDX, IDC_ERROR_CODE, m_errCode);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCapiErrD, CDialog)
	//{{AFX_MSG_MAP(CCapiErrD)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_GET_ERROR, OnGetErrorClick)
	ON_BN_CLICKED(IDC_DEC, OnDecClick)
	ON_BN_CLICKED(IDC_HEX, OnHexClick)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCapiErrD message handlers

BOOL CCapiErrD::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	CString strAboutMenu;
	strAboutMenu.LoadString(IDS_ABOUTBOX);
	if (!strAboutMenu.IsEmpty())
	{
	    pSysMenu->AppendMenu(MF_SEPARATOR);
	    pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	STATUS  nError = NotesInit();   /* Init the Domino and Notes runtime system. */
	
	// If the call to NotesInit fails, display an error message
	// and exit the program.
	if (nError != NOERROR)
	{
	    AfxMessageBox ("Unable to Initialize Notes!", MB_OK);
	}

	((CButton *)GetDlgItem(IDC_DEC))->SetCheck(1);
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CCapiErrD::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
	    CAboutDlg dlgAbout;
	    dlgAbout.DoModal();
	}
	else
	{
	    CDialog::OnSysCommand(nID, lParam);
	}
}

void CCapiErrD::OnPaint() 
{
	if (IsIconic())
	{
	    CPaintDC dc(this); // device context for painting

	    SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

	    // Center icon in client rectangle
	    int cxIcon = GetSystemMetrics(SM_CXICON);
	    int cyIcon = GetSystemMetrics(SM_CYICON);
	    CRect rect;
	    GetClientRect(&rect);
	    int x = (rect.Width() - cxIcon + 1) / 2;
	    int y = (rect.Height() - cyIcon + 1) / 2;

	    // Draw the icon
	    dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
	    CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCapiErrD::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CCapiErrD::OnDestroy() 
{
	CDialog::OnDestroy();
	
	NotesTerm(); /* Terminate the Domino and Notes runtime system. */
}

void CCapiErrD::OnGetErrorClick() 
{
	STATUS  nError = NOERROR;
	CString strFormatText = "";
	int		iStatus = 0;
	LPTSTR	s = 0;

	s = m_errCode.GetBuffer(LINEOTEXT);

	GetDlgItemText(IDC_ERROR_CODE, m_errCode); 

	if(((CButton *)GetDlgItem(IDC_DEC))->GetCheck())
	{
	    iStatus = sscanf(LPCTSTR(m_errCode), "%d", &nError);
	    strFormatText.Format("Status: %d(dec) %x(hex)\n", nError, nError);
	}
	else
	{		
	    iStatus = sscanf(LPCTSTR(m_errCode), "%x", &nError);
	    strFormatText.Format("Status: %x(hex) %d(dec)\n", nError, nError);
	}
	
	if(EOF == iStatus ||  0 == iStatus)
	{
	    strFormatText = "Invalid entry.";
	}
	else
	{
	    strFormatText+= "\n";
	    OSLoadString(NULLHANDLE, ERR(nError), s, LINEOTEXT-1);
	    if(strlen(s)) 
	    {
	        strFormatText += s;
	    }
	    else
	    {
	        strFormatText += "No matching ASCII text";
	    }
	}
	SetDlgItemText(IDC_ERROR_MESSAGE, strFormatText);
	m_errCode.ReleaseBuffer();
	((CEdit*)GetDlgItem(IDC_ERROR_CODE))->SetSel((int) 0,(int) -1);
}

void CCapiErrD::OnDecClick() 
{
	SetDlgItemText(IDC_ERROR_MESSAGE, "");	
}

void CCapiErrD::OnHexClick() 
{
	SetDlgItemText(IDC_ERROR_MESSAGE, "");	
}
