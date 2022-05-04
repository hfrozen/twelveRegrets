// FileView.h
#pragma once

#include "ViewTree.h"

class CFileViewToolBar : public CMFCToolBar
{
	virtual void OnUpdateCmdUI(CFrameWnd* /*pTarget*/, BOOL bDisableIfNoHndler)
	{
		CMFCToolBar::OnUpdateCmdUI((CFrameWnd*) GetOwner(), bDisableIfNoHndler);
	}

	virtual BOOL AllowShowOnList() const { return FALSE; }
};

class CFileView : public CDockablePane
{
public:
	CFileView();

	void AdjustLayout();

protected:
	CViewTree m_wndFileView;
	CFileViewToolBar m_wndToolBar;

public:
	virtual ~CFileView();
	virtual BOOL OnShowControlBarMenu(CPoint point);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnPaint();
	afx_msg void OnSetFocus(CWnd* pOldWnd);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnSelected(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnRightClick(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()

private:
	CString		m_strRoot;
	CImageList	m_imageListSmall;
	CImageList	m_imageListLarge;

	void	Log(CString str);
	bool	IsFolder(CString strFolder);
	HTREEITEM	InsertNode(CString strParent, TCHAR* pNodeName, HTREEITEM hParent);
	HTREEITEM	InsertNode(CString strParent, WIN32_FIND_DATA fd, HTREEITEM hParent);
	bool	IsSysDirectory(CString strParent, TCHAR* pNodeName);
	bool	MakeDirectory(CString strParent, HTREEITEM hParent);
	CString	GetFullNodeName(HTREEITEM hNode);
	bool	CreateSysImageList(CImageList* pImageList, CString root, BOOL bLargeIcon);
	bool	Unfold();

public:
	void	SetRoot(CString strRoot);
	CString	GetRoot()		{ return m_strRoot; }
};

