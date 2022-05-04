#pragma once
#include "afxwin.h"


// CRange 대화 상자입니다.

class CRange : public CDialog
{
	DECLARE_DYNAMIC(CRange)

public:
	CRange(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CRange();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG_RANGE };

	virtual BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
	virtual void OnOK();

private:
	CStatic	m_stText;
	CEdit	m_editMin;
	CEdit	m_editMax;

public:
	int64_t	m_var;
	int64_t	m_min;
	int64_t	m_max;
};
