// NewMacro.cpp : implementation file
//

#include "stdafx.h"
#include "Terminal.h"
#include "NewMacro.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CNewMacro dialog


CNewMacro::CNewMacro(CWnd* pParent /*=NULL*/)
	: CDialog(CNewMacro::IDD, pParent)
{
	//{{AFX_DATA_INIT(CNewMacro)
	m_strMacro = _T("");
	//}}AFX_DATA_INIT
	m_nAtt = m_nKey = 0;
}


void CNewMacro::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CNewMacro)
	DDX_Control(pDX, IDC_MACRO_NAME_EDIT, m_ctrlName);
	DDX_Text(pDX, IDC_MACRO_STRING_EDIT, m_strMacro);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CNewMacro, CDialog)
	//{{AFX_MSG_MAP(CNewMacro)
	ON_WM_CTLCOLOR()
	ON_WM_DESTROY()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CNewMacro message handlers

BOOL CNewMacro::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	m_whiteBrush.CreateSolidBrush(RGB(255, 255, 255));

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

BOOL CNewMacro::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN) {
		if (pMsg->wParam == VK_RETURN) {
			CWnd* pWndOK = GetDlgItem(IDOK);
			CWnd* pWndEsc = GetDlgItem(IDCANCEL);
			if (GetFocus() != pWndOK && GetFocus() != pWndEsc)
				pMsg->wParam = VK_TAB;
		}
		else {
			CWnd* pWnd = GetDlgItem(IDC_MACRO_NAME_EDIT);
			if (GetFocus() == pWnd) {
				if ((pMsg->wParam >= VK_F1 && pMsg->wParam <= VK_F2) ||
					(pMsg->wParam >= '0' && pMsg->wParam <= '9') ||
					(pMsg->wParam >= 'A' && pMsg->wParam <= 'Z') ||
					(pMsg->wParam >= 'a' && pMsg->wParam <= 'z')) {
					m_nKey = pMsg->wParam;
					CString str = _T("");
					m_nAtt = 0;
					if (::GetKeyState(VK_SHIFT) < 0) {
						str += _T("SHIFT+");
						m_nAtt |= SHIFT_ATT;
					}
					if (::GetKeyState(VK_CONTROL) < 0) {
						str += _T("CTRL+");
						m_nAtt |= CONTROL_ATT;
					}
					if (::GetKeyState(VK_MENU) < 0) {
						str += _T("ALT+");
						m_nAtt |= ALT_ATT;
					}
					if (pMsg->wParam >= VK_F1 && pMsg->wParam <= VK_F12) {
						CString strTemp;
						strTemp.Format(_T("F%d"), pMsg->wParam - VK_F1 + 1);
						str += strTemp;
						m_ctrlName.SetWindowText(str);
					}
					else {
						CString strTemp;
						strTemp.Format(_T("%c"), pMsg->wParam);
						str += strTemp;
						m_ctrlName.SetWindowText(str);
					}
					return TRUE;
				}
			}
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

HBRUSH CNewMacro::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor) 
{
	HBRUSH hbr = CDialog::OnCtlColor(pDC, pWnd, nCtlColor);
	
	// TODO: Change any attributes of the DC here
	
	// TODO: Return a different brush if the default is not desired
	if (pWnd->m_hWnd == m_ctrlName.m_hWnd) {
		pDC->SetBkColor(RGB(255, 255, 255));
		return (HBRUSH)m_whiteBrush;
	}
	return hbr;
}

void CNewMacro::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	m_whiteBrush.DeleteObject();
}
