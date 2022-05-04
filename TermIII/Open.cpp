// Open.cpp : implementation file
//

#include "stdafx.h"
#include "Terminal.h"
#include "Open.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COpen dialog

const DWORD	COpen::m_dwBpss[] = {
	1200,		// 0
	2400,		// 1
	4800,		// 2
	9600,		// 3
	19200,		// 4
	38400,		// 5
	57600,		// 6
	115200,		// 7
	128000,		// 8
	256000,		// 9
	0
};

COpen::COpen(CWnd* pParent /*=NULL*/)
	: CDialog(COpen::IDD, pParent)
{
	//{{AFX_DATA_INIT(COpen)
	m_nDls = -1;
	m_nSls = -1;
	m_nPrs = -1;
	m_nDtr = -1;
	m_nRts = -1;
	//}}AFX_DATA_INIT
}


void COpen::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COpen)
	DDX_Control(pDX, IDC_COMBO_PORT, m_ctrlPort);
	DDX_Control(pDX, IDC_COMBO_BPS, m_ctrlBps);
	DDX_Radio(pDX, IDC_RADIO_DATAS8, m_nDls);
	DDX_Radio(pDX, IDC_RADIO_STOP1, m_nSls);
	DDX_Radio(pDX, IDC_RADIO_NONS, m_nPrs);
	DDX_Radio(pDX, IDC_RADIO_DISABLE_DTR, m_nDtr);
	DDX_Radio(pDX, IDC_RADIO_DISABLE_RTS, m_nRts);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COpen, CDialog)
	ON_BN_CLICKED(IDOK, &COpen::OnBnClickedOk)
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// COpen message handlers

BOOL COpen::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	/*if (pMsg->message == WM_KEYDOWN) {
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
			pMsg->wParam = VK_TAB;
	}*/
	return CDialog::PreTranslateMessage(pMsg);
}

BOOL COpen::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
	// TODO: Add extra initialization here
	for (int n = 0; n < 128; n ++) {
		CString str;
		str.Format(_T("COM%d"), n);
		m_ctrlPort.AddString(str);
	}
	int cur = m_pao.nPort < 0 ? 0 : m_pao.nPort;
	m_ctrlPort.SetCurSel(cur);

	cur = -1;
	for (int n = 0; m_dwBpss[n] > 0; n ++) {
		CString str;
		str.Format(_T("%ld"), m_dwBpss[n]);
		m_ctrlBps.AddString(str);
		if (m_pao.dwBps == m_dwBpss[n])	cur = n;
	}
	if (cur < 0)	cur = 0;
	m_ctrlBps.SetCurSel(cur);

	m_nDls = m_pao.GetDataLength() == 8 ? 0 : 1;
	m_nSls = m_pao.GetStopLength();
	m_nPrs = m_pao.GetParity();
	m_nDtr = m_pao.GetDtr();
	m_nRts = m_pao.GetRts();

	UpdateData(FALSE);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void COpen::OnBnClickedOk()
{
	UpdateData(TRUE);
	m_pao.nPort = m_ctrlPort.GetCurSel();
	int cur = m_ctrlBps.GetCurSel();
	m_pao.dwBps = m_dwBpss[cur];
	m_pao.nDatas = m_nDls == 0 ? 8 : 7;
	m_pao.nStps = m_nSls;
	m_pao.nParity = m_nPrs;
	m_pao.nDtr = m_nDtr;
	m_pao.nRts = m_nRts;
	CDialog::OnOK();
}
