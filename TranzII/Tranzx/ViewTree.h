
#pragma once

#define	WM_SELECT		WM_USER + 2
#define	WM_SELECTED		WM_USER + 4
#define	WM_DROPED		WM_USER + 6
#define	WM_RIGHTCLICK	WM_USER + 8

/////////////////////////////////////////////////////////////////////////////
// CViewTree â�Դϴ�.

class CViewTree : public CTreeCtrl
{
// �����Դϴ�.
public:
	CViewTree();

// �������Դϴ�.
protected:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);

// �����Դϴ�.
public:
	virtual ~CViewTree();

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTvnSelchanging(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnNMRClick(NMHDR *pNMHDR, LRESULT *pResult);
	//afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);

private:
	CWnd*	m_pParent;

	bool	m_bEnableDrag;
	bool	m_bDraggin;
	HTREEITEM	m_hDraggedItem;
	//HTREEITEM	m_hDropItem;
	CImageList*	m_pDraggedImage;

	bool	IsEnableDrag(HTREEITEM hItem);

public:
	void	SetParent(CWnd* pParent)	{ m_pParent = pParent; }
	void	EnableDrag(bool drag)		{ m_bEnableDrag = drag; }
};
