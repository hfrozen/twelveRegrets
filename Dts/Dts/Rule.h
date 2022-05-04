#pragma once

#include "AidChkList.h"

class CRule : public CDialogEx
{
	DECLARE_DYNAMIC(CRule)

public:
	CRule(CWnd* pParent = NULL);
	virtual ~CRule();

	enum { IDD = IDD_DIALOG_RULE };

private:
	CWnd*	m_pParent;
	CAidChkList	m_list[4];

	void	MakeCheckList(CAidChkList* pList, int col, WORD* pW);

public:
	ARTICLE	m_article;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnBnClickedOk();
};
