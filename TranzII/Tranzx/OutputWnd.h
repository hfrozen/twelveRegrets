#pragma once

#define WM_CHANGEOUTPUTWND	WM_USER + 16

class COutputList : public CListBox
{
public:
	COutputList();

protected:
	UINT	m_nTroubleCode;

public:
	virtual ~COutputList();

	void	SetTroubleCode(UINT nCode)		{ m_nTroubleCode = nCode; }

protected:
	afx_msg void OnEditCopy();
	afx_msg void OnEditClear();
	afx_msg void OnViewOutput();
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

	DECLARE_MESSAGE_MAP()
};

class COutputWnd : public CDockablePane
{
public:
	COutputWnd();

protected:
	CFont m_font;

	CMFCTabCtrl	m_wndTabs;

	COutputList m_wndOutputBuild;
	//COutputList m_wndOutputDebug;
	COutputList m_wndOutputFind;

protected:
	//void FillBuildWindow();
	//void FillDebugWindow();
	//void FillFindWindow();

	void AdjustHorzScroll(CListBox& wndListBox);

// 구현입니다.
public:
	virtual ~COutputWnd();

	void	LogOutput(CString str);
	void	FindOutput(CString str);
	void	LogReset();
	void	FindReset();
	void	SetTroubleCode(UINT nCode)		{ m_wndOutputFind.SetTroubleCode(nCode); }

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg LRESULT OnAfxWmChangeActiveTab(WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
};

