#pragma once


// CItem
#include "AidTitle.h"
#include "AidGraph.h"

class CItem : public CWnd
{
	DECLARE_DYNAMIC(CItem)

public:
	CItem(CWnd* pParent);
	virtual ~CItem();

	virtual bool Create(DWORD dwStyle, const RECT& rect, CWnd* pParent, UINT nID = NULL);

protected:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnPartDelete(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

private:
	CWnd*		m_pParent;
	CRect		m_rect;
	CAidGraph	m_context;
	CRect		m_rectContext;
	CRect		m_rectTitle;
	CPoint		m_ptCur;
	int			m_nGrid;
	COLORREF	m_crBkgnd;
	CBrush		m_brBkgnd;

	CPtrList	m_partList;

	int		GetNextRight();

public:
	typedef struct _tagIDLIST {
		int			id;
		COLORREF	color;
		CRect		rect;
		CAidTitle*	pTitle;
	} PART, *PPART;

	void	Log(CString str);
	CAidTitle*	GetTitle(int id);
	TCHAR	GetFirstTitleType();
	bool	Draw(int id);
	bool	AddPart(int id, CString strName, COLORREF color);
	CString	GetName(int id);
	int64_t	GetPscMax(int id);
	int64_t	GetPscMin(int id);
	int64_t	GetLogMax(int id);
	int64_t	GetLogMin(int id);
	void	Redraw();
	void	SetPos(CPoint pt);
	void	SetGrid(int grid);
	CSize	GetFrameSize();
	bool	PartSelect(int id);
	bool	Copy(CDC* pDC, int nPage, int sizex, CSize factor, CRect& fRect);
	void	SetBound(CRect rect);
	CString	GetInfo();
	//CPtrList*	GetPartList()	{ return &m_partList; }
	CWnd*	GetParent()		{ return m_pParent; }
};
