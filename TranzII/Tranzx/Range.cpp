// Range.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "Tranzx.h"
#include "Range.h"


// CRange 대화 상자입니다.

IMPLEMENT_DYNAMIC(CRange, CDialog)

CRange::CRange(CWnd* pParent /*=NULL*/)
	: CDialog(CRange::IDD, pParent)
{
	m_var = m_min = m_max = 0;
}

CRange::~CRange()
{
}

void CRange::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_RANGE_TEXT, m_stText);
	DDX_Control(pDX, IDC_EDIT_RANGE_MIN, m_editMin);
	DDX_Control(pDX, IDC_EDIT_RANGE_MAX, m_editMax);
}


BEGIN_MESSAGE_MAP(CRange, CDialog)
END_MESSAGE_MAP()


BOOL CRange::OnInitDialog()
{
	CDialog::OnInitDialog();

	CString str;
	str.Format(L"이 항목의 범위는 유효하지 않습니다(%lld)\r\n다시 설정하여 주십시오.", m_var);
	m_stText.SetWindowText(str);

	m_min = m_max = m_var;
	str.Format(L"%lld", m_min);
	m_editMin.SetWindowText(str);

	str.Format(L"%lld", m_max);
	m_editMax.SetWindowText(str);

	return TRUE;
}

void CRange::OnOK()
{
	CString str;
	m_editMin.GetWindowTextW(str);
	m_min = _wtoi(str);

	m_editMax.GetWindowTextW(str);
	m_max = _wtoi(str);

	if (m_min >= m_max || m_var < m_min || m_var > m_max) {
		str.Format(L"설정하신 범위가 %lld를 벗어났습니다.\r\n다시 설정하십시오.", m_var);
		m_stText.SetWindowText(str);
		return;
	}

	CDialog::OnOK();
}
