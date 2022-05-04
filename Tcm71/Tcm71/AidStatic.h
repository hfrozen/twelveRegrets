#pragma once


// CAidStatic

#define	BKCOLOR		RGB(236, 233, 216)

class CAidStatic : public CStatic
{
	DECLARE_DYNAMIC(CAidStatic)

public:
	CAidStatic();
	virtual ~CAidStatic();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg HBRUSH CtlColor(CDC* /*pDC*/, UINT /*nCtlColor*/);

	void	SetColor(COLORREF crText, COLORREF crBkgnd);
	void	SetTextColor(COLORREF crText);
	void	SetBkColor(COLORREF crBkgnd);

protected:
	COLORREF	m_crText;
	COLORREF	m_crBkgnd;
	CBrush		m_brBkgnd;
};


