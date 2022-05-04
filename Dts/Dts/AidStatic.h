// AidStatic.h
#pragma once

class CAidStatic : public CStatic
{
	DECLARE_DYNAMIC(CAidStatic)

public:
	CAidStatic();
	virtual ~CAidStatic();

protected:
	COLORREF	m_crText, m_crBkgnd;
	CBrush		m_brBkgnd;

public:
	void	SetColor(COLORREF crText, COLORREF crBkgnd);
	void	SetTextColor(COLORREF crText)	{ SetColor(crText, m_crBkgnd); }
	void	SetBkColor(COLORREF crBkgnd)	{ SetColor(m_crText, crBkgnd); }

protected:
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	DECLARE_MESSAGE_MAP()
};


