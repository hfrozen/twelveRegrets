// Sum.cpp : implementation file
//

#include "stdafx.h"
#include "Terminal.h"
#include "Sum.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSum dialog


CSum::CSum(CWnd* pParent /*=NULL*/)
	: CDialog(CSum::IDD, pParent)
{
	m_nSumType = -1;
	m_nForm = -1;
	m_nLength = -1;
	m_bTimeDisp = FALSE;
	m_bHexDisp = FALSE;
}


void CSum::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSum)
	DDX_Radio(pDX, IDC_RADIO_ADDSUM, m_nSumType);
	DDX_Radio(pDX, IDC_RADIO_ASCIISUM, m_nForm);
	DDX_Radio(pDX, IDC_RADIO_BYTESUM, m_nLength);
	//}}AFX_DATA_MAP
	DDX_Check(pDX, IDC_CHECK_TIME, m_bTimeDisp);
	DDX_Check(pDX, IDC_CHECK_NUMBER, m_bHexDisp);
}


BEGIN_MESSAGE_MAP(CSum, CDialog)
//	ON_BN_CLICKED(IDC_RADIO_ADDSUM, OnSelectSum)
//	ON_BN_CLICKED(IDC_RADIO_XORSUM, OnSelectSum)
//	ON_BN_CLICKED(IDC_RADIO_XORP1SUM, OnSelectSum)
//	ON_BN_CLICKED(IDC_RADIO_XORM1SUM, OnSelectSum)
//	ON_BN_CLICKED(IDC_RADIO_ASCIISUM, OnSelectForm)
//	ON_BN_CLICKED(IDC_RADIO_BINSUM, OnSelectForm)
//	ON_BN_CLICKED(IDC_RADIO_BYTESUM, OnSelectLength)
//	ON_BN_CLICKED(IDC_RADIO_WORDSUM, OnSelectLength)
	ON_BN_CLICKED(IDOK, &CSum::OnBnClickedOk)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSum message handlers

BOOL CSum::PreTranslateMessage(MSG* pMsg) 
{
	/*if (pMsg->message == WM_KEYDOWN) {
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_ESCAPE)
			pMsg->wParam = VK_TAB;
	}*/
	return CDialog::PreTranslateMessage(pMsg);
}

//void CSum::OnSelectSum() 
//{
//	UpdateData(TRUE);
//}
//
//void CSum::OnSelectForm() 
//{
//	UpdateData(TRUE);
//}
//
//void CSum::OnSelectLength() 
//{
//	UpdateData(TRUE);
//}

void CSum::OnBnClickedOk()
{
	UpdateData(TRUE);
	//CString str;
	//str.Format(_T("sum=%d, form=%d, length=%d, time=%d, hex=%d"),
	//		m_nSumType, m_nForm, m_nLength, m_bTimeDisp, m_bHexDisp);
	//MessageBox(str);
	CDialog::OnOK();
}
