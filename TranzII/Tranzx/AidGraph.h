#pragma once

// CAidGraph

#define	OFFSETY_GRAPH	2

class CAidGraph : public CWnd
{
	DECLARE_DYNAMIC(CAidGraph)

public:
	CAidGraph();
	virtual ~CAidGraph();

	virtual bool Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID = NULL);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

private:
	CWnd*		m_pView;
	CFont		m_font;

	COLORREF	m_crBkgnd;
	COLORREF	m_crGrid;
	COLORREF	m_crPlot;
	COLORREF	m_crBound;
	CBrush		m_brBkgnd;

	CPoint		m_ptCur;
	int			m_nGrid;

	CDC			m_dcGrid;
	CDC			m_dcPlot;
	CBitmap		m_bitmapGrid;
	CBitmap		m_bitmapPlot;
	CBitmap*	m_pBitmapGrid;
	CBitmap*	m_pBitmapPlot;

	typedef struct _tagCOORD {
		double	x;
		double	y;
	} _COORD;
	_COORD		m_scaleMin;
	_COORD		m_scaleMax;
	_COORD		m_factor;
	_COORD		m_current;
	_COORD		m_prev;

	int			m_nPrecision;
	bool		m_bPrev;
	bool		m_bDevision;

	void	InvalidateCtrl();

public:
	void	ErasePlot();
	bool	AppendPoint(double x, double y, COLORREF color = RGB(16, 16, 16));
	bool	AppendText(int x, CString str, COLORREF color = RGB(16, 16, 16));
	CSize	GetTextWidth(CString str);
	void	SetRange(double minx, double maxx, double miny, double maxy, int nPrecision = 0);
	void	SetGridColor(COLORREF color);
	void	SetBkColor(COLORREF color);
	//void	Reset();
	void	SetPos(CPoint pt);
	void	SetGrid(int grid);
	bool	Copy(CDC* pDC, int nPage, int sizex, CSize factor, CRect& fRect);
	void	SetBound(CRect rect);
	void	InitPlot()					{ m_bPrev = false; }
	void	SetMainView(CWnd* pView)	{ m_pView = pView; }
};
