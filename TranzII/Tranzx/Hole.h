#pragma once
#include "Lands.h"

class CHole : public CWnd
{
	DECLARE_DYNAMIC(CHole)

public:
	CHole(CWnd* pParent);
	virtual ~CHole();

	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParent, UINT nID = NULL);

protected:
	//afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	DECLARE_MESSAGE_MAP()

private:
	CWnd*		m_pParent;
	int			m_nGrid;

	CPoint		m_ptCur;
	CRect		m_rect;
	CLands*		m_pLands;

public:
	void	Log(CString str);
	bool	Entrance(int id, CString strName, COLORREF color = RGB(16, 16, 16), CPoint* pPoint = NULL);
	bool	EntranceA(int id, CString strName, COLORREF color = RGB(16, 16, 16), BOOL bNew = TRUE);
	void	Redraw(CPoint pt);
	void	SetPos(CPoint pt);
	void	SetPos(int y);
	void	SetGrid(int grid);
	//int		GetScale();
	//void	SetScale(int scale);
	int		GetHeight();
	void	Copy(CDC* pDC, int nPage, int sizex, CSize factor, CRect& rect);
	CString	GetInfo()			{ return m_pLands->GetInfo(); }
	CRect	PartSelect(int id)	{ return m_pLands->PartSelect(id); }
	CRect	GetRect()			{ return m_rect; }
	CWnd*	GetParent()			{ return m_pParent; }
};


