#pragma once


// CAidTitle

#define	HEIGHT_TEXT		12

class CAidTitle : public CWnd
{
	DECLARE_DYNAMIC(CAidTitle)

public:
	CAidTitle(CWnd* pParent);
	virtual ~CAidTitle();

	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID = NULL);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

private:
	CWnd*		m_pParent;
	CButton		m_button;
	CFont		m_font;
	CFont		m_fontHz;
	int			m_nID;

	bool		m_bSelected;
	COLORREF	m_crBkgnd;
	COLORREF	m_crText;
	COLORREF	m_crProg;

	TCHAR		m_nType;
	BYTE		m_nLength;
	CString		m_strName;
	CString		m_strUnit;
	int			m_nDecimal;
	int64_t		m_pscMin;
	int64_t		m_pscMax;
	int64_t		m_logMin;
	int64_t		m_logMax;

	CBrush		m_brBkgnd;
	CBrush		m_brProg;

	CPoint		m_ptCur;
	CRect		m_rect;
	CDC			m_dc;
	CBitmap		m_bitmap;
	CBitmap*	m_pBitmap;
	CProgressCtrl*	m_pProgress;

#define	PROGRESSBAR_X			12
	CRect		m_rtProg;

	void	InvalidateCtrl();

public:
	void	Current(int64_t v, COLORREF color = RGB(16, 16, 16));
	bool	SetItem(int nID, TCHAR nType, BYTE nLeng, CString strName,
						int64_t pscMin, int64_t pscMax, int64_t logMin, int64_t logMax, CString strUnit, COLORREF color);
	//void	SetGridColor(COLORREF color);
	//void	SetPos(CPoint pt);
	void	SetValue(TCHAR type, BYTE leng, _variant_t var);
	void	SetBkColor(COLORREF color);
	void	SetTextColor(COLORREF color);
	int		GetWidth();
	bool	PartSelect(int id);
	bool	Copy(CDC* pDC, CSize factor, CRect& fRect);
	void	SetBound(CRect rect);
	CString	GetInfo();
	TCHAR	GetType()	{ return m_nType; }
	BYTE	GetLength()	{ return m_nLength; }
	int64_t	GetPscMin()	{ return m_pscMin; }
	int64_t	GetPscMax()	{ return m_pscMax; }
	int64_t	GetLogMin()	{ return m_logMin; }
	int64_t	GetLogMax()	{ return m_logMax; }
	COLORREF	GetColor()	{ return m_crText; }
	void	Redraw()	{ SetTextColor(m_crText); }
};
