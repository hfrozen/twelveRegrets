#pragma once


// CIp 대화 상자입니다.

class CIp : public CDialog
{
	DECLARE_DYNAMIC(CIp)

public:
	CIp(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CIp();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG_IP };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
protected:
	virtual void OnOK();

public:
	CString	m_strIp;
};
