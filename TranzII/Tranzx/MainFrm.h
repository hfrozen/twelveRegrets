// MainFrm.h :
#include "FileView.h"
#include "ClassView.h"
#include "OutputWnd.h"
#include "PropertiesWnd.h"
#include "FindTroubles.h"

class CMainFrame : public CMDIFrameWndEx
{
	DECLARE_DYNAMIC(CMainFrame)
public:
	CMainFrame();

public:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL LoadFrame(UINT nIDResource, DWORD dwDefaultStyle = WS_OVERLAPPEDWINDOW | FWS_ADDTOTITLE, CWnd* pParentWnd = NULL, CCreateContext* pContext = NULL);
	virtual BOOL PreTranslateMessage(MSG* pMsg);

public:
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	CMFCMenuBar			m_wndMenuBar;
	CMFCToolBar			m_wndToolBar;
	CMFCStatusBar		m_wndStatusBar;
	CMFCToolBarImages	m_UserImages;
	CFileView			m_wndFileView;
	CClassView			m_wndClassView;
	COutputWnd			m_wndOutput;
	CPropertiesWnd		m_wndProperties;

	CFindTroubles*	m_pFindTroubles;
	CString		m_strFindPath;
	int			m_nTroubleFind;

	bool	m_bPropertyState;

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnWindowManager();
	afx_msg void OnViewCustomize();
	afx_msg LRESULT OnToolbarCreateNew(WPARAM wp, LPARAM lp);
	afx_msg void OnApplicationLook(UINT id);
	afx_msg void OnUpdateApplicationLook(CCmdUI* pCmdUI);
	afx_msg void OnNcLButtonDblClk(UINT nHitTest, CPoint point);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg void OnChangepath();
	//afx_msg LRESULT OnAfxWmChangingActiveTab(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnAfxWmChangeActiveTab(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnChangeOuputWnd(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnFindTroubleCode(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnCloseFindTrouble(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	BOOL CreateDockingWindows();
	void SetDockingWindowIcons(BOOL bHiColorIcons);

private:
	int		m_nMonitor;
	CRect	m_rect[10];

public:
	void	Log(CString str, bool bPos = false);
	void	DeleteLog(bool bPos = false);
	void	ShowProperties(bool bShow);
	void	LaunchFindTroubles(bool bFolder);
	void	SetTroubleCode(UINT nCode)	{ m_wndOutput.SetTroubleCode(nCode); }
	void	SearchTrouble()				{ if (m_pFindTroubles != NULL)	m_pFindTroubles->SearchTrouble(); }
	bool	IsFindingTroubles()			{ return m_pFindTroubles == NULL ? false : true; }
	//CString	GetFrameLog();
	//void	SetFrameLog(CString str);
	UINT	GetMonitorLength()			{ return m_nMonitor; }
	CRect	GetMonitorRect()			{ return m_rect; }
	CString	GetRoot()					{ return m_wndFileView.GetRoot(); }
	TCHAR	GetItemType()				{ return m_wndClassView.GetType(); }
	void	MakeItemList(CDocument* pDoc, TCHAR nType)		{ m_wndClassView.SetItem(pDoc, nType); }
	CString	GetSelectedItemName()		{ return m_wndClassView.GetSelectedItemName(); }

	void	EnableSelectTrouble(bool bEnable)	{ m_wndProperties.EnableSelectTrouble(bEnable); }
	bool	IsEnableSelectTrouble()				{ return m_wndProperties.IsEnableSelectTroueble(); }
	void	SetSelectTroubleState(UINT nSel)	{ m_wndProperties.SetSelectTroubleState(nSel); }
	UINT	GetSelectTroubleState()				{ return m_wndProperties.GetSelectTroubleState(); }
};


