// AidStatic.h
#pragma once

//#define	BKGNDCOLOR		RGB(236, 233, 216)
//#define TEXTCOLOR		RGB(0, 0, 0)
//#define	INVBKGNDCOLOR	RGB(18, 116, 220)
//#define INVTEXTCOLOR	RGB(255, 255, 255)

#define	WM_LBTNDBCLK	WM_USER + 12

class CAidStatic : public CStatic
{
public:
	CAidStatic();
	CAidStatic(CWnd* pParent);
	virtual ~CAidStatic();

protected:
	CWnd*		m_pParent;
	int			m_nID;
	bool		m_bInv;
	COLORREF	m_crText;
	COLORREF	m_crBkgnd;
	CBrush		m_brBkgnd;

public:
	void	SetID(int id)			{ m_nID = id; }
	void	SetColor(COLORREF crText, COLORREF crBkgnd);
	void	SetTextColor(COLORREF crText);
	void	SetBkColor(COLORREF crBkgnd);
	COLORREF	GetTextColor()		{ return m_crText; }
	bool	GetInvert()				{ return m_bInv; }

protected:
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
};
