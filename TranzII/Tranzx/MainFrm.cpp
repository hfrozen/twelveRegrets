3// MainFrm.cpp
#include "stdafx.h"
#include "Tranzx.h"

#include "MainFrm.h"
#include "ChildFrm.h"
#include "TranzxDoc.h"
#include "TranzxView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNAMIC(CMainFrame, CMDIFrameWndEx)

const int  iMaxUserToolbars = 10;
const UINT uiFirstUserToolBarId = AFX_IDW_CONTROLBAR_FIRST + 40;
const UINT uiLastUserToolBarId = uiFirstUserToolBarId + iMaxUserToolbars - 1;

BEGIN_MESSAGE_MAP(CMainFrame, CMDIFrameWndEx)
	ON_WM_CREATE()
	ON_COMMAND(ID_WINDOW_MANAGER, &CMainFrame::OnWindowManager)
	ON_COMMAND(ID_VIEW_CUSTOMIZE, &CMainFrame::OnViewCustomize)
	ON_REGISTERED_MESSAGE(AFX_WM_CREATETOOLBAR, &CMainFrame::OnToolbarCreateNew)
	ON_COMMAND_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_OFF_2007_AQUA, &CMainFrame::OnApplicationLook)
	ON_UPDATE_COMMAND_UI_RANGE(ID_VIEW_APPLOOK_WIN_2000, ID_VIEW_APPLOOK_OFF_2007_AQUA, &CMainFrame::OnUpdateApplicationLook)
	ON_WM_NCLBUTTONDBLCLK()
	ON_WM_NCLBUTTONDOWN()
	ON_COMMAND(ID_CHANGEPATH, &CMainFrame::OnChangepath)
	//ON_REGISTERED_MESSAGE(AFX_WM_CHANGING_ACTIVE_TAB, &CMainFrame::OnAfxWmChangingActiveTab)
	ON_REGISTERED_MESSAGE(AFX_WM_CHANGE_ACTIVE_TAB, &CMainFrame::OnAfxWmChangeActiveTab)
	ON_MESSAGE(WM_CHANGEOUTPUTWND, &CMainFrame::OnChangeOuputWnd)
	ON_MESSAGE(WM_FINDTROUBLECODE, &CMainFrame::OnFindTroubleCode)
	ON_MESSAGE(WM_CLOSEFINDTROUBLE, &CMainFrame::OnCloseFindTrouble)
	//ON_WM_MDIACTIVATE()
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // 상태 줄 표시기
	ID_INDICATOR_NAME,
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

CMainFrame::CMainFrame()
{
	theApp.m_nAppLook = theApp.GetInt(L"ApplicationLook", ID_VIEW_APPLOOK_VS_2005);
	m_nMonitor = 1;
	m_bPropertyState = false;
	m_pFindTroubles = NULL;
	m_strFindPath = L"";
}

CMainFrame::~CMainFrame()
{
	if (m_pFindTroubles != NULL) {
		delete m_pFindTroubles;
		m_pFindTroubles = NULL;
	}
}

//============================================================================

void CMainFrame::Log(CString str, bool bPos)
{
	if (bPos)	m_wndOutput.FindOutput(str);
	else	m_wndOutput.LogOutput(str);
}

void CMainFrame::DeleteLog(bool bPos)
{
	if (bPos)	m_wndOutput.FindReset();
	else	m_wndOutput.LogReset();
}

void CMainFrame::ShowProperties(bool bShow)
{
	m_wndProperties.ShowPane(bShow, false, bShow);
	m_bPropertyState = bShow;
	if (!m_bPropertyState)	m_wndProperties.EnableSelectTrouble(false);
}

void CMainFrame::LaunchFindTroubles(bool bFolder)
{
	if (m_pFindTroubles != NULL)	delete m_pFindTroubles;
	m_pFindTroubles = new CFindTroubles(this, bFolder);
	if (m_strFindPath.IsEmpty())	m_strFindPath = GetRoot();
	m_pFindTroubles->SetFindPath(m_strFindPath);
	m_pFindTroubles->Create(IDD_DIALOG_FINDTROUBLES, this);
	m_pFindTroubles->ShowWindow(SW_SHOW);
	m_nTroubleFind = -1;

	CRect rect;
	m_wndProperties.GetWindowRect(rect);
	CRect rectFind;
	m_pFindTroubles->GetWindowRect(rectFind);
	m_pFindTroubles->SetWindowPos(NULL, rect.left - rectFind.Width(), rect.top, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
}

/*CString CMainFrame::GetFrameLog()
{
	return m_strFrameLog;
}

void CMainFrame::SetFrameLog(CString str)
{
	m_strFrameLog = str;
	m_wndStatusBar.SetPaneText(1, m_strFrameLog);
	//SetWindowText(L"Train data analyzer        " + str);
}
*/
//============================================================================

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CMDIFrameWndEx::OnCreate(lpCreateStruct) == -1)	return -1;

	OnApplicationLook(theApp.m_nAppLook);

	CMDITabInfo mdiTabParams;
	mdiTabParams.m_style = CMFCTabCtrl::STYLE_3D_ONENOTE;
	mdiTabParams.m_bActiveTabCloseButton = TRUE;
	mdiTabParams.m_bTabIcons = FALSE;
	mdiTabParams.m_bAutoColor = TRUE;
	mdiTabParams.m_bDocumentMenu = TRUE;
	EnableMDITabbedGroups(TRUE, mdiTabParams);

	if (!m_wndMenuBar.Create(this)) {
		TRACE0("메뉴 모음을 만들지 못했습니다.\n");
		return -1;
	}

	DWORD style = m_wndMenuBar.GetPaneStyle();
	style &= ~CBRS_GRIPPER;
	style |= (CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndMenuBar.SetPaneStyle(style);
	//m_wndMenuBar.SetPaneStyle(m_wndMenuBar.GetPaneStyle() | CBRS_SIZE_DYNAMIC | CBRS_TOOLTIPS | CBRS_FLYBY);

	CMFCPopupMenu::SetForceMenuFocus(FALSE);

	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP | CBRS_GRIPPER | CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC) ||
		!m_wndToolBar.LoadToolBar(theApp.m_bHiColorIcons ? IDR_MAINFRAME_256 : IDR_MAINFRAME)) {
		TRACE0("도구 모음을 만들지 못했습니다.\n");
		return -1;
	}

	CString strToolBarName;
	BOOL bNameValid = strToolBarName.LoadString(IDS_TOOLBAR_STANDARD);
	ASSERT(bNameValid);
	m_wndToolBar.SetWindowText(strToolBarName);

	CString strCustomize;
	bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);
	m_wndToolBar.EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);

	m_wndToolBar.ShowWindow(SW_HIDE);	// ?????

	InitUserToolbars(NULL, uiFirstUserToolBarId, uiLastUserToolBarId);

	if (!m_wndStatusBar.Create(this)) {
		TRACE0("상태 표시줄을 만들지 못했습니다.\n");
		return -1;
	}
	m_wndStatusBar.SetIndicators(indicators, sizeof(indicators)/sizeof(UINT));

	m_wndStatusBar.SetPaneInfo(1, ID_INDICATOR_NAME, SBPS_NORMAL, 400);
	m_wndStatusBar.SetPaneTextColor(1, RGB(0, 0, 0));

	m_wndMenuBar.EnableDocking(CBRS_ALIGN_ANY);
	m_wndToolBar.EnableDocking(CBRS_ALIGN_ANY);
	EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndMenuBar);
	DockPane(&m_wndToolBar);


	// Visual Studio 2005 스타일 도킹 창 동작을 활성화합니다.
	//CDockingManager::SetDockingMode(DT_SMART);
	// Visual Studio 2005 스타일 도킹 창 자동 숨김 동작을 활성화합니다.
	//EnableAutoHidePanes(CBRS_ALIGN_ANY);

	CMFCToolBar::AddToolBarForImageCollection(IDR_MENU_IMAGES, theApp.m_bHiColorIcons ? IDB_MENU_IMAGES_24 : 0);

	if (!CreateDockingWindows()) {
		TRACE0("도킹 창을 만들지 못했습니다.\n");
		return -1;
	}

	m_wndFileView.EnableDocking(CBRS_ALIGN_ANY);
	m_wndClassView.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndFileView);
	CDockablePane* pTabbedBar = NULL;
	m_wndClassView.AttachToTabWnd(&m_wndFileView, DM_SHOW, TRUE, &pTabbedBar);
	m_wndOutput.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndOutput);
	pTabbedBar->SetControlBarStyle(AFX_CBRS_RESIZE);	// !!!!!
	m_wndFileView.ShowPane(true, false, true);
	m_wndProperties.EnableDocking(CBRS_ALIGN_ANY);
	DockPane(&m_wndProperties);

	EnableWindowsDialog(ID_WINDOW_MANAGER, IDS_WINDOWS_MANAGER, TRUE);
	EnablePaneMenu(TRUE, ID_VIEW_CUSTOMIZE, strCustomize, ID_VIEW_TOOLBAR);
	CMFCToolBar::EnableQuickCustomization();

	if (CMFCToolBar::GetUserImages() == NULL) {
		if (m_UserImages.Load(L".\\UserImages.bmp")) {
			m_UserImages.SetImageSize(CSize(16, 16), FALSE);
			CMFCToolBar::SetUserImages(&m_UserImages);
		}
	}

	CList<UINT, UINT> lstBasicCommands;

	lstBasicCommands.AddTail(ID_FILE_NEW);
	lstBasicCommands.AddTail(ID_FILE_OPEN);
	lstBasicCommands.AddTail(ID_FILE_SAVE);
	lstBasicCommands.AddTail(ID_FILE_PRINT);
	lstBasicCommands.AddTail(ID_APP_EXIT);
	lstBasicCommands.AddTail(ID_EDIT_CUT);
	lstBasicCommands.AddTail(ID_EDIT_PASTE);
	lstBasicCommands.AddTail(ID_EDIT_UNDO);
	lstBasicCommands.AddTail(ID_APP_ABOUT);
	lstBasicCommands.AddTail(ID_VIEW_STATUS_BAR);
	lstBasicCommands.AddTail(ID_VIEW_TOOLBAR);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2003);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_VS_2005);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_BLUE);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_SILVER);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_BLACK);
	lstBasicCommands.AddTail(ID_VIEW_APPLOOK_OFF_2007_AQUA);
	lstBasicCommands.AddTail(ID_SORTING_SORTALPHABETIC);
	lstBasicCommands.AddTail(ID_SORTING_SORTBYTYPE);
	lstBasicCommands.AddTail(ID_SORTING_SORTBYACCESS);
	lstBasicCommands.AddTail(ID_SORTING_GROUPBYTYPE);

	CMFCToolBar::SetBasicCommands(lstBasicCommands);

	if (m_nMonitor > 1)
		SetWindowPos(NULL, m_rect[0].left, m_rect[0].top, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CMDIFrameWndEx::PreCreateWindow(cs))	return FALSE;

	m_rect[0] = CRect(0, 0, 0, 0);
	m_nMonitor = ::GetSystemMetrics(SM_CMONITORS);
	int cx = ::GetSystemMetrics(SM_CXFULLSCREEN);
	int cy = ::GetSystemMetrics(SM_CYFULLSCREEN);
	m_rect[0] = m_rect[1] = CRect(0, 0, cx, cy);
	if (m_nMonitor > 1) {
		int ml = 2;
		int x = ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
		POINT pt;
		pt.x = pt.y = 0;
		HMONITOR hMonitorl = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
		HMONITOR hMonitorr;
		for (pt.x = 0; pt.x < x; pt.x += 1000) {
			hMonitorr = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
			if (hMonitorr != hMonitorl) {
				MONITORINFOEX mi;
				mi.cbSize = sizeof(MONITORINFOEX);
				::GetMonitorInfoW(hMonitorr, &mi);
				m_rect[ml ++] = CRect(mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right, mi.rcMonitor.bottom);
				hMonitorl = hMonitorr;
			}
		}
		//m_rect[0] = m_rect[m_nMonitor];
		m_rect[0] = m_rect[2];
	}

	cs.lpszName = L"2호선 운행 기록 분석기";
	cs.style &= ~FWS_ADDTOTITLE;	// | WS_MAXIMIZEBOX | WS_MINIMIZEBOX);

	return TRUE;
}

BOOL CMainFrame::LoadFrame(UINT nIDResource, DWORD dwDefaultStyle, CWnd* pParentWnd, CCreateContext* pContext) 
{
	if (!CMDIFrameWndEx::LoadFrame(nIDResource, dwDefaultStyle, pParentWnd, pContext))	return FALSE;

	CString strCustomize;
	BOOL bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	for (int i = 0; i < iMaxUserToolbars; i ++) {
		CMFCToolBar* pUserToolbar = GetUserToolBarByIndex(i);
		if (pUserToolbar != NULL)
			pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	}

	return TRUE;
}

BOOL CMainFrame::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN) {
		bool bFolder = (GetKeyState(VK_SHIFT) & 0x8000) ? true : false;
		if (GetKeyState(VK_CONTROL) && bFolder && pMsg->wParam == 'F') {
			LaunchFindTroubles(true);

			::TranslateMessage(pMsg);
			::DispatchMessage(pMsg);
			return TRUE;
		}
	}

	return CMDIFrameWndEx::PreTranslateMessage(pMsg);
}

BOOL CMainFrame::CreateDockingWindows()
{
	CString strClassView;
	BOOL bNameValid = strClassView.LoadString(IDS_CLASS_VIEW);
	ASSERT(bNameValid);
	if (!m_wndClassView.Create(strClassView, this, CRect(0, 0, 250, 200), TRUE, ID_VIEW_CLASSVIEW,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT | CBRS_FLOAT_MULTI,
		AFX_CBRS_REGULAR_TABS, AFX_CBRS_RESIZE)) {
		TRACE0("클래스 보기 창을 만들지 못했습니다.\n");
		return FALSE;
	}

	CString strFileView;
	bNameValid = strFileView.LoadString(IDS_FILE_VIEW);
	ASSERT(bNameValid);
	if (!m_wndFileView.Create(strFileView, this, CRect(0, 0, 250, 200), TRUE, ID_VIEW_FILEVIEW,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_LEFT| CBRS_FLOAT_MULTI,
		AFX_CBRS_REGULAR_TABS, AFX_CBRS_RESIZE)) {
		TRACE0("파일 보기 창을 만들지 못했습니다.\n");
		return FALSE;
	}

	CString strOutputWnd;
	bNameValid = strOutputWnd.LoadString(IDS_OUTPUT_WND);
	ASSERT(bNameValid);
	if (!m_wndOutput.Create(strOutputWnd, this, CRect(0, 0, 100, 100), TRUE, ID_VIEW_OUTPUTWND,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_BOTTOM | CBRS_FLOAT_MULTI)) {
		TRACE0("출력 창을 만들지 못했습니다.\n");
		return FALSE;
	}
	DWORD dwStyle = m_wndOutput.GetControlBarStyle();
	dwStyle &= ~(AFX_CBRS_CLOSE | AFX_CBRS_FLOAT);
	m_wndOutput.SetControlBarStyle(dwStyle);

	CString strPropertiesWnd;
	bNameValid = strPropertiesWnd.LoadString(IDS_PROPERTIES_WND);
	ASSERT(bNameValid);
	if (!m_wndProperties.Create(strPropertiesWnd, this, CRect(0, 0, 200, 200), TRUE, ID_VIEW_PROPERTIESWND,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CBRS_RIGHT | CBRS_FLOAT_MULTI)) {
		TRACE0("속성 창을 만들지 못했습니다.\n");
		return FALSE;
	}
	dwStyle = m_wndProperties.GetControlBarStyle();
	dwStyle &= ~(AFX_CBRS_CLOSE | AFX_CBRS_FLOAT);
	m_wndProperties.SetControlBarStyle(dwStyle);

	CRect rect;
	m_wndProperties.GetWindowRect(rect);
	m_wndProperties.SetWindowPos(NULL, 0, 0, rect.Width() * 2, rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

	SetDockingWindowIcons(theApp.m_bHiColorIcons);
	return TRUE;
}

void CMainFrame::SetDockingWindowIcons(BOOL bHiColorIcons)
{
	HICON hFileViewIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_FILE_VIEW_HC : IDI_FILE_VIEW), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndFileView.SetIcon(hFileViewIcon, FALSE);

	HICON hClassViewIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_CLASS_VIEW_HC : IDI_CLASS_VIEW), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndClassView.SetIcon(hClassViewIcon, FALSE);

	HICON hOutputBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_OUTPUT_WND_HC : IDI_OUTPUT_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndOutput.SetIcon(hOutputBarIcon, FALSE);

	HICON hPropertiesBarIcon = (HICON) ::LoadImage(::AfxGetResourceHandle(), MAKEINTRESOURCE(bHiColorIcons ? IDI_PROPERTIES_WND_HC : IDI_PROPERTIES_WND), IMAGE_ICON, ::GetSystemMetrics(SM_CXSMICON), ::GetSystemMetrics(SM_CYSMICON), 0);
	m_wndProperties.SetIcon(hPropertiesBarIcon, FALSE);

	UpdateMDITabbedBarsIcons();
}

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CMDIFrameWndEx::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CMDIFrameWndEx::Dump(dc);
}
#endif //_DEBUG

void CMainFrame::OnWindowManager()
{
	ShowWindowsDialog();
}

void CMainFrame::OnViewCustomize()
{
	CMFCToolBarsCustomizeDialog* pDlgCust = new CMFCToolBarsCustomizeDialog(this, TRUE /* 메뉴를 검색합니다. */);
	pDlgCust->EnableUserDefinedToolbars();
	pDlgCust->Create();
}

LRESULT CMainFrame::OnToolbarCreateNew(WPARAM wp,LPARAM lp)
{
	LRESULT lres = CMDIFrameWndEx::OnToolbarCreateNew(wp,lp);
	if (lres == 0)
	{
		return 0;
	}

	CMFCToolBar* pUserToolbar = (CMFCToolBar*)lres;
	ASSERT_VALID(pUserToolbar);

	CString strCustomize;
	BOOL bNameValid = strCustomize.LoadString(IDS_TOOLBAR_CUSTOMIZE);
	ASSERT(bNameValid);

	pUserToolbar->EnableCustomizeButton(TRUE, ID_VIEW_CUSTOMIZE, strCustomize);
	return lres;
}

void CMainFrame::OnApplicationLook(UINT id)
{
	CWaitCursor wait;

	theApp.m_nAppLook = id;

	switch (theApp.m_nAppLook)
	{
	case ID_VIEW_APPLOOK_WIN_2000:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManager));
		break;

	case ID_VIEW_APPLOOK_OFF_XP:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOfficeXP));
		break;

	case ID_VIEW_APPLOOK_WIN_XP:
		CMFCVisualManagerWindows::m_b3DTabsXPTheme = TRUE;
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
		break;

	case ID_VIEW_APPLOOK_OFF_2003:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2003));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	case ID_VIEW_APPLOOK_VS_2005:
		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerVS2005));
		CDockingManager::SetDockingMode(DT_SMART);
		break;

	default:
		switch (theApp.m_nAppLook)
		{
		case ID_VIEW_APPLOOK_OFF_2007_BLUE:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_LunaBlue);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_BLACK:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_ObsidianBlack);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_SILVER:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Silver);
			break;

		case ID_VIEW_APPLOOK_OFF_2007_AQUA:
			CMFCVisualManagerOffice2007::SetStyle(CMFCVisualManagerOffice2007::Office2007_Aqua);
			break;
		}

		CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerOffice2007));
		CDockingManager::SetDockingMode(DT_SMART);
	}

	RedrawWindow(NULL, NULL, RDW_ALLCHILDREN | RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME | RDW_ERASE);

	theApp.WriteInt(L"ApplicationLook", theApp.m_nAppLook);
}

void CMainFrame::OnUpdateApplicationLook(CCmdUI* pCmdUI)
{
	pCmdUI->SetRadio(theApp.m_nAppLook == pCmdUI->m_nID);
}

void CMainFrame::OnNcLButtonDblClk(UINT nHitTest, CPoint point)
{
	//if (nHitTest == HTCAPTION)	return;

	CMDIFrameWndEx::OnNcLButtonDblClk(nHitTest, point);
}

void CMainFrame::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	//if (nHitTest == HTCAPTION)	return;

	CMDIFrameWndEx::OnNcLButtonDown(nHitTest, point);
}

void CMainFrame::OnChangepath()
{
	//m_wndFileView.SetRoot(L"");
	BROWSEINFO bi;
	ZeroMemory(&bi, sizeof(BROWSEINFO));
	bi.hwndOwner = GetSafeHwnd();
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	bi.lpszTitle = L"Download folder select";
	LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
	TCHAR szFolder[MAX_PATH * 2];
	szFolder[0] = L'\0';
	if (pidl) {
		if (SHGetPathFromIDList(pidl, szFolder)) {
			CString strRoot = (LPCTSTR)szFolder;
			m_wndFileView.SetRoot(strRoot);
			AfxGetApp()->WriteProfileString(L"TranzxRef", L"Folder", strRoot);
		}
	}
}

//LRESULT CMainFrame::OnAfxWmChangingActiveTab(WPARAM wParam, LPARAM lParam)
//{
//	CMDIChildWnd* pChild = (CMDIChildWnd*)GetActiveFrame();
//	CTranzxView* pView = (CTranzxView*)pChild->GetActiveView();
//	if (pView != NULL) {
//		TCHAR type = pView->GetType();
//		if (type == FILETYPE_TROUBLE)	ShowProperties(true);
//		else if (type == FILETYPE_LOGBOOK)	ShowProperties(false);
//	}
//	return 0;
//}
//
LRESULT CMainFrame::OnAfxWmChangeActiveTab(WPARAM wParam, LPARAM lParam)
{
	CMDIChildWnd* pChild = (CMDIChildWnd*)GetActiveFrame();
	CTranzxView* pView = (CTranzxView*)pChild->GetActiveView();
	if (pView != NULL) {
		TCHAR type = pView->GetType();
		bool bShow = (type == FILETYPE_TROUBLE) ? true : false;
		if (bShow != m_bPropertyState) {
			m_bPropertyState = bShow;
			ShowProperties(m_bPropertyState);
		}
		if (m_bPropertyState) {
			if (!IsEnableSelectTrouble())	EnableSelectTrouble(true);
			SetSelectTroubleState(pView->GetTroubleType());
		}
	}
	return 0;
}

LRESULT CMainFrame::OnChangeOuputWnd(WPARAM wParam, LPARAM lParam)
{
	CString* pStr = (CString*)lParam;
	if (!pStr->IsEmpty())	m_wndOutput.SetWindowTextW(*pStr);
	return 0;
}

LRESULT CMainFrame::OnFindTroubleCode(WPARAM wParam, LPARAM lParam)
{
	CChildFrame* pChild = (CChildFrame*)GetActiveFrame();
	ASSERT(pChild);
	CTranzxView* pView = (CTranzxView*)pChild->GetActiveView();
	ASSERT(pView);
	UINT find = (UINT)wParam;
	m_nTroubleFind = pView->FindTroubleCode(find, m_nTroubleFind);

	return 0;
}

LRESULT CMainFrame::OnCloseFindTrouble(WPARAM wParam, LPARAM lParam)
{
	if (m_pFindTroubles != NULL) {
		if (m_pFindTroubles->GetFolderType())	m_strFindPath = m_pFindTroubles->GetFindPath();
		delete m_pFindTroubles;
		m_pFindTroubles = NULL;
	}
	return 0;
}

//void CMainFrame::OnMDIActivate(BOOL bActivate, CWnd* pActivateWnd, CWnd* pDeactivateWnd)
//{
//	CMDIFrameWndEx::OnMDIActivate(bActivate, pActivateWnd, pDeactivateWnd);
//
//	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
//	AfxMessageBox(L"Frame Tab");
//}
