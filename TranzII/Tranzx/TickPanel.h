#pragma once

// CTickPanel

class CTickPanel : public CWnd
{
	DECLARE_DYNAMIC(CTickPanel)

public:
	//CTickPanel();
	CTickPanel(CWnd* pParent);
	virtual ~CTickPanel();

	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParent, UINT nID = NULL);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

private:
	CWnd*		m_pParent;
	CFont		m_font;

	COLORREF	m_crBkgnd;
	COLORREF	m_crText;
	COLORREF	m_crGrid;
	CBrush		m_brBkgnd;
	CBrush		m_brText;
	CBrush		m_brGrid;

	CPoint		m_ptCur;
	int			m_nGrid;

	CDC			m_dcGrid;
	CDC			m_dcPlot;
	CBitmap		m_bitmapGrid;
	CBitmap		m_bitmapPlot;
	CBitmap*	m_pBitmapGrid;
	CBitmap*	m_pBitmapPlot;

	void	InvalidateCtrl();
	void	MakeRule();

public:
	void	Draw(CPoint* ppt = NULL);
	void	SetPos(CPoint pt);
	bool	Copy(CDC* pDC, CPoint org, int nPage, int sizex, CSize factor, CRect& fRect);
	void	SetGrid(int grid)		{ m_nGrid = grid; InvalidateCtrl(); }
};
