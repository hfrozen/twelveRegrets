// IpEdit.h
#pragma once

class CIpEdit : public CDialogEx
{
	DECLARE_DYNAMIC(CIpEdit)

public:
	CIpEdit(CWnd* pParent = NULL);
	virtual ~CIpEdit();

	enum { IDD = IDD_DIALOG_IP };

	int		m_nUnit;
	CString	m_strIp;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	virtual BOOL OnInitDialog();
	virtual void OnOK();
	DECLARE_MESSAGE_MAP()
};
