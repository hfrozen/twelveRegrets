#pragma once
#include "afxwin.h"

#ifdef _WIN32_WCE
#error "Windows CE에서는 CDHtmlDialog가 지원되지 않습니다."
#endif 

// CTimeDiv 대화 상자입니다.

class CTimeDiv : public CDialog
{
	DECLARE_DYNCREATE(CTimeDiv)

public:
	CTimeDiv(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CTimeDiv();

	enum { IDD = IDD_DIALOG_FACTOR };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
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
