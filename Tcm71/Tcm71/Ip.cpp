// Ip.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "Tcm71.h"
#include "Ip.h"


// CIp 대화 상자입니다.

IMPLEMENT_DYNAMIC(CIp, CDialog)

CIp::CIp(CWnd* pParent /*=NULL*/)
	: CDialog(CIp::IDD, pParent)
{
	m_strIp = _T("");
}

CIp::~CIp()
{
}

void CIp::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CIp, CDialog)
END_MESSAGE_MAP()


// CIp 메시지 처리기입니다.

BOOL CIp::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	GetDlgItem(IDC_IPADDRESS)->SetWindowText(m_strIp);
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CIp::OnOK()
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	GetDlgItem(IDC_IPADDRESS)->GetWindowText(m_strIp);

	CDialog::OnOK();
}
