#pragma once
#include "afxwin.h"

#ifdef _WIN32_WCE
#error "Windows CE������ CDHtmlDialog�� �������� �ʽ��ϴ�."
#endif 

// CTimeDiv ��ȭ �����Դϴ�.

class CTimeDiv : public CDialog
{
	DECLARE_DYNCREATE(CTimeDiv)

public:
	CTimeDiv(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CTimeDiv();

	enum { IDD = IDD_DIALOG_FACTOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.
	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedCancel();
	DECLARE_MESSAGE_MAP()

private:
	CButton m_radio1;
	CButton m_radio2;
	CButton m_radio3;
	CButton m_radio4;

public:
	int m_nDiv;
	int	m_nMax;
};
