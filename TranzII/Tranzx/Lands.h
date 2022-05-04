#pragma once
#include "Item.h"

class CLands : public CWnd
{
	DECLARE_DYNAMIC(CLands)

public:
	CLands(CWnd* pParent);
	virtual ~CLands();

	virtual bool Create(DWORD dwStyle, const RECT& rect, CWnd* pParent, UINT nID = NULL);

protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg LRESULT OnItemDelete(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	CWnd*		m_pParent;
	COLORREF	m_crBkgnd;
	COLORREF	m_crGrid;
	CBrush		m_brBkgnd;
	int			m_nGrid;

	CPoint		m_ptCur;
	CRect		m_rect;
	//CDC			m_dc;
	//CBitmap		m_bitmap;
	//CBitmap*	m_pBitmap;

	typedef struct _tagCONTENTS {
		CItem*	pItem;
		CRect	rect;
	} CONTENTS, *PCONTENTS;
	CPtrList	m_contentsList;

	int		GetLastBottom();
	PCONTENTS	GetContentsByPosition(int y);
	CAidTitle*	GetPartByID(int id);
	bool	CheckRange(int id);
	bool	AppendItem(int id, CString strName, COLORREF color);

public:
	void	Log(CString str);
	bool	Entrance(int id, CString strName, COLORREF color = RGB(16, 16, 16), CPoint* pPoint = NULL);
	bool	EntranceA(int id, CString strName, COLORREF color = RGB(16, 16, 16), bool bNew = true);
	void	Redraw(CPoint pt, CRect rect);
	void	SetPos(CPoint pt);
	void	SetPos(int y);
	void	SetGrid(int grid);
	//int		GetScale();
	//void	SetScale(int scale);
	CRect	PartSelect(int id);
	void	Copy(CDC* pDC, int nPage, int sizex, CSize factor, CRect& rect);
	CString	GetInfo();
	int		GetHeight()		{ return m_rect.Height(); }
	CWnd*	GetParent()		{ return m_pParent; }
};


