// TimeDiv.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "Tranzx.h"
#include "TimeDiv.h"


// CTimeDiv 대화 상자입니다.

IMPLEMENT_DYNCREATE(CTimeDiv, CDialog)

CTimeDiv::CTimeDiv(CWnd* pParent /*=NULL*/)
	: CDialog(CTimeDiv::IDD, pParent)
{
	m_nDiv = -1;
}

CTimeDiv::~CTimeDiv()
{
}

void CTimeDiv::DoDataExchange(CDataExchange* pDX)
{
	DDX_Radio(pDX, IDC_RADIO_FACTOR1, m_nDiv);
	DDX_Control(pDX, IDC_RADIO_FACTOR1, m_radio1);
	DDX_Control(pDX, IDC_RADIO_FACTOR2, m_radio2);
	DDX_Control(pDX, IDC_RADIO_FACTOR5, m_radio3);
	DDX_Control(pDX, IDC_RADIO_FACTOR10, m_radio4);
}

BOOL CTimeDiv::OnInitDialog()
{
	CDialog::OnInitDialog();

	m_radio4.EnableWindow(m_nMax < 3 ? FALSE : TRUE);
	m_radio3.EnableWindow(m_nMax < 2 ? FALSE : TRUE);
	m_radio2.EnableWindow(m_nMax < 1 ? FALSE : TRUE);
	m_radio1.EnableWindow(TRUE);

	return TRUE;
}

BEGIN_MESSAGE_MAP(CTimeDiv, CDialog)
	ON_BN_CLICKED(IDCANCEL, &CTimeDiv::OnBnClickedCancel)
END_MESSAGE_MAP()

void CTimeDiv::OnBnClickedCancel()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	OnCancel();
}
