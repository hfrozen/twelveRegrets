// CAidInner.h
#pragma once

#define	WM_NEXTITEM		WM_USER + 2

class CAidInner : public CEdit
{
	DECLARE_DYNAMIC(CAidInner)

public:
	CAidInner();
	CAidInner(int nItem, int nSubItem, CString strInit, COLORREF crText, COLORREF crBkgnd);
	virtual ~CAidInner();

public:
	int		m_nItem;
	int		m_nSubItem;
private:
	CString	m_strInit;
	bool	m_bEsc;
	COLORREF	m_crText;
	COLORREF	m_crBkgnd;
	CBrush	m_brBkgnd;

protected:
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void OnChar(UINT nChar, UINT nRepCnt, UINT nFlags);
	afx_msg void OnKillFocus(CWnd* pNewWnd);
	afx_msg void OnNcDestroy();
};


