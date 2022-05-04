#pragma once
#include "afxwin.h"


// CRange ��ȭ �����Դϴ�.

class CRange : public CDialog
{
	DECLARE_DYNAMIC(CRange)

public:
	CRange(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CRange();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_DIALOG_RANGE };

	virtual BOOL OnInitDialog();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

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
