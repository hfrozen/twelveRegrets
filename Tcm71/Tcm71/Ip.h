#pragma once


// CIp ��ȭ �����Դϴ�.

class CIp : public CDialog
{
	DECLARE_DYNAMIC(CIp)

public:
	CIp(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CIp();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_DIALOG_IP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();

public:
	CString	m_strIp;
};
