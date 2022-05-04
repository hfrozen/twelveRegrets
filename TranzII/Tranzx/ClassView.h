#pragma once

#include "ViewTree.h"

class CClassToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CClassView : public CDockablePane
{
public:
	CClassView();

	void AdjustLayout();

protected:
	CViewTree	m_wndClassView;
	CClassToolBar m_wndToolBar;
	CImageList	m_ClassViewImages;
	UINT		m_nCurrSort;

public:
	virtual ~CClassView();
	virtual BOOL OnShowControlBarMenu(CPoint point);

public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg LRESULT OnSelect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSelected(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnDroped(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

private:
	TCHAR	m_nType;
	CDocument*	m_pDoc;
	CImageList*	m_pImageList;
	bool	m_bMakedItem;
	bool	m_bChange;
	CString	m_strSelectedItem;

	void	Log(CString str);
	int		MakeItem(HTREEITEM hParent, BYTE nHierarchy, int id);

public:
	CString	GetFullNodeName(HTREEITEM hNode);
	int		FindItem(CString strItem, int id = 0);
	void	SetItem(CDocument* pDoc, TCHAR nType);
	CDocument*	GetDocument()		{ return m_pDoc; }
	CString	GetSelectedItemName()	{ return m_strSelectedItem; }
	TCHAR	GetType()				{ return m_nType; }
};

