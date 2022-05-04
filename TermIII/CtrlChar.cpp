// CtrlChar.cpp : implementation file
//

#include "stdafx.h"
#include "Terminal.h"
#include "CtrlChar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCtrlChar dialog


CCtrlChar::CCtrlChar(CWnd* pParent /*=NULL*/)
	: CDialog(CCtrlChar::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCtrlChar)
	m_strAck = _T("");
	m_strEnq = _T("");
	m_strEot = _T("");
	m_strEtx = _T("");
	m_strNak = _T("");
	m_strSoh = _T("");
	m_strStx = _T("");
	m_strSum = _T("");
	//}}AFX_DATA_INIT
	m_pItemStr[0] = &m_strSoh;
	m_pItemStr[1] = &m_strStx;
	m_pItemStr[2] = &m_strEtx;
	m_pItemStr[3] = &m_strEot;
	m_pItemStr[4] = &m_strEnq;
	m_pItemStr[5] = &m_strAck;
	m_pItemStr[6] = &m_strNak;
	m_pItemStr[7] = &m_strSum;
}


void CCtrlChar::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCtrlChar)
	DDX_Text(pDX, IDC_EDIT_ACK, m_strAck);
	DDX_Text(pDX, IDC_EDIT_ENQ, m_strEnq);
	DDX_Text(pDX, IDC_EDIT_EOT, m_strEot);
	DDX_Text(pDX, IDC_EDIT_ETX, m_strEtx);
	DDX_Text(pDX, IDC_EDIT_NAK, m_strNak);
	DDX_Text(pDX, IDC_EDIT_SOH, m_strSoh);
	DDX_Text(pDX, IDC_EDIT_STX, m_strStx);
	DDX_Text(pDX, IDC_EDIT_SUM, m_strSum);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(CCtrlChar, CDialog)
	//{{AFX_MSG_MAP(CCtrlChar)
		// NOTE: the ClassWizard will add message map macros here
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCtrlChar message handlers

BOOL CCtrlChar::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN) {
		if (pMsg->wParam == VK_RETURN) {
			CWnd* pWndOK = GetDlgItem(IDOK);
			CWnd* pWndEsc = GetDlgItem(IDCANCEL);
			if (GetFocus() != pWndOK && GetFocus() != pWndEsc)
				pMsg->wParam = VK_TAB;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}
