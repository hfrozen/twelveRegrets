// Ip.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "Tcm71.h"
#include "Ip.h"


// CIp ��ȭ �����Դϴ�.

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


// CIp �޽��� ó�����Դϴ�.

BOOL CIp::OnInitDialog()
{
	CDialog::OnInitDialog();

	// TODO:  ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	GetDlgItem(IDC_IPADDRESS)->SetWindowText(m_strIp);
	UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	// ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}

void CIp::OnOK()
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.
	GetDlgItem(IDC_IPADDRESS)->GetWindowText(m_strIp);

	CDialog::OnOK();
}
