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
 */

// CapiErrD.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCapiErrD dialog

#define LINEOTEXT   256

class CCapiErrD : public CDialog
{
// Construction
public:
	CCapiErrD(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CCapiErrD)
	enum { IDD = IDD_CapiErr_DIALOG };
	CString	m_errCode;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCapiErrD)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CCapiErrD)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnDestroy();
	afx_msg void OnGetErrorClick();
	afx_msg void OnDecClick();
	afx_msg void OnHexClick();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};
