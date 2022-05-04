// IpEdit.cpp
#include "stdafx.h"
#include "Watcher.h"
#include "IpEdit.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(CIpEdit, CDialogEx)

CIpEdit::CIpEdit(CWnd* pParent /*=NULL*/)
	: CDialogEx(CIpEdit::IDD, pParent)
{
	m_nUnit = 0;
	m_strIp = L"";
}

CIpEdit::~CIpEdit()
{
}

void CIpEdit::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Radio(pDX, IDC_RADIO_TU, m_nUnit);
}

BEGIN_MESSAGE_MAP(CIpEdit, CDialogEx)
END_MESSAGE_MAP()

BOOL CIpEdit::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	GetDlgItem(IDC_IPADDRESS)->SetWindowTextW(m_strIp);
	UpdateData(false);

	return TRUE;
}

void CIpEdit::OnOK()
{
	UpdateData(true);
	GetDlgItem(IDC_IPADDRESS)->GetWindowTextW(m_strIp);

	CDialogEx::OnOK();
}
