#include "stdafx.h"

#include "OutputWnd.h"
#include "Resource.h"
#include "Tranzx.h"
#include "MainFrm.h"
#include "ChildFrm.h"
#include "TranzxView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

COutputWnd::COutputWnd()
{
}

COutputWnd::~COutputWnd()
{
}

void COutputWnd::LogOutput(CString str)
{
	int i = m_wndTabs.GetActiveTab();
	if (i != 0)	m_wndTabs.SetActiveTab(0);

	m_wndOutputBuild.AddString(str);
	m_wndOutputBuild.SetCurSel(m_wndOutputBuild.GetCount() - 1);
	m_wndOutputBuild.SetCurSel(-1);
}

void COutputWnd::FindOutput(CString str)
{
	int i = m_wndTabs.GetActiveTab();
	if (i != 1)	m_wndTabs.SetActiveTab(1);

	m_wndOutputFind.AddString(str);
	m_wndOutputFind.SetCurSel(m_wndOutputFind.GetCount() - 1);
	m_wndOutputFind.SetCurSel(-1);
}

void COutputWnd::LogReset()
{
	m_wndOutputBuild.ResetContent();
}

void COutputWnd::FindReset()
{
	m_wndOutputFind.ResetContent();
}

void COutputWnd::AdjustHorzScroll(CListBox& wndListBox)
{
	CClientDC dc(this);
	CFont* pOldFont = dc.SelectObject(&m_font);

	int cxExtentMax = 0;

	for (int i = 0; i < wndListBox.GetCount(); i ++) {
		CString strItem;
		wndListBox.GetText(i, strItem);

		cxExtentMax = max(cxExtentMax, dc.GetTextExtent(strItem).cx);
	}

	wndListBox.SetHorizontalExtent(cxExtentMax);
	dc.SelectObject(pOldFont);
}

BEGIN_MESSAGE_MAP(COutputWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_REGISTERED_MESSAGE(AFX_WM_CHANGE_ACTIVE_TAB, &COutputWnd::OnAfxWmChangeActiveTab)
	ON_WM_MBUTTONDBLCLK()
END_MESSAGE_MAP()

int COutputWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)	return -1;

	m_font.CreateStockObject(DEFAULT_GUI_FONT);

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	if (!m_wndTabs.Create(CMFCTabCtrl::STYLE_FLAT, rectDummy, this, 1)) {
		TRACE0("출력 탭 창을 만들지 못했습니다.\n");
		return -1;
	}

	const DWORD dwStyle = LBS_NOINTEGRALHEIGHT | WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL;

	if (!m_wndOutputBuild.Create(dwStyle, rectDummy, &m_wndTabs, 2) ||
	//	!m_wndOutputDebug.Create(dwStyle, rectDummy, &m_wndTabs, 3) ||
		!m_wndOutputFind.Create(dwStyle, rectDummy, &m_wndTabs, 3)) {
		TRACE0("출력 창을 만들지 못했습니다.\n");
		return -1;
	}

	m_wndOutputBuild.SetFont(&m_font);
	//m_wndOutputDebug.SetFont(&m_Font);
	m_wndOutputFind.SetFont(&m_font);

	CString strTabName;
	//BOOL bNameValid;

	// 탭에 목록 창을 연결합니다.
	BOOL bNameValid = strTabName.LoadString(IDS_BUILD_TAB);
	ASSERT(bNameValid);
	m_wndTabs.AddTab(&m_wndOutputBuild, strTabName, (UINT)0);
	//bNameValid = strTabName.LoadString(IDS_DEBUG_TAB);
	//ASSERT(bNameValid);
	//m_wndTabs.AddTab(&m_wndOutputDebug, strTabName, (UINT)1);
	bNameValid = strTabName.LoadString(IDS_FIND_TAB);
	ASSERT(bNameValid);
	m_wndTabs.AddTab(&m_wndOutputFind, strTabName, (UINT)1);

	// 출력 탭을 더미 텍스트로 채웁니다.
	//FillBuildWindow();
	//FillDebugWindow();
	//FillFindWindow();

	return 0;
}

void COutputWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);

	m_wndTabs.SetWindowPos (NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
	//m_wndOutputBuild.SetWindowPos (NULL, -1, -1, cx, cy, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOZORDER);
}

LRESULT COutputWnd::OnAfxWmChangeActiveTab(WPARAM wParam, LPARAM lParam)
{
	int i = m_wndTabs.GetActiveTab();
	CString str;
	m_wndTabs.GetTabLabel(i, str);
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	if (pFrame != NULL)	pFrame->SendMessage(WM_CHANGEOUTPUTWND, 0, (LPARAM)&str);

	return 0;
}

/*void COutputWnd::FillBuildWindow()
{
	m_wndOutputBuild.AddString(L"여기에 빌드 출력이 표시됩니다.");
	m_wndOutputBuild.AddString(L"출력이 목록 뷰 행에 표시되지만");
	m_wndOutputBuild.AddString(L"표시 방법을 원하는 대로 변경할 수 있습니다.");
}

void COutputWnd::FillDebugWindow()
{
	m_wndOutputDebug.AddString(L"여기에 디버그 출력이 표시됩니다.");
	m_wndOutputDebug.AddString(L"출력이 목록 뷰 행에 표시되지만");
	m_wndOutputDebug.AddString(L"표시 방법을 원하는 대로 변경할 수 있습니다.");
}

void COutputWnd::FillFindWindow()
{
	m_wndOutputFind.AddString(L"여기에 찾기 출력이 표시됩니다.");
	m_wndOutputFind.AddString(L"출력이 목록 뷰 행에 표시되지만");
	m_wndOutputFind.AddString(L"표시 방법을 원하는 대로 변경할 수 있습니다.");
}
*/

/////////////////////////////////////////////////////////////////////////////
// COutputList1

COutputList::COutputList()
{
	m_nTroubleCode = 0;
}

COutputList::~COutputList()
{
}

BEGIN_MESSAGE_MAP(COutputList, CListBox)
	ON_COMMAND(ID_EDIT_COPY, OnEditCopy)
	ON_COMMAND(ID_EDIT_CLEAR, OnEditClear)
	ON_COMMAND(ID_VIEW_OUTPUTWND, OnViewOutput)
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_LBUTTONDBLCLK()
END_MESSAGE_MAP()

void COutputList::OnEditCopy()
{
	MessageBox(L"출력 복사");
}

void COutputList::OnEditClear()
{
	MessageBox(L"출력 지우기");
}

void COutputList::OnViewOutput()
{
	CDockablePane* pParentBar = DYNAMIC_DOWNCAST(CDockablePane, GetOwner());
	CMDIFrameWndEx* pMainFrame = DYNAMIC_DOWNCAST(CMDIFrameWndEx, GetTopLevelFrame());

	if (pMainFrame != NULL && pParentBar != NULL) {
		pMainFrame->SetFocus();
		pMainFrame->ShowPane(pParentBar, FALSE, FALSE, FALSE);
		pMainFrame->RecalcLayout();

	}
}

void COutputList::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	CMFCTabCtrl* pTab = (CMFCTabCtrl*)GetParent();
	if (pTab == NULL || pTab->GetActiveTab() != 1)	return;

	int sel = GetCurSel();
	CString str;
	GetText(sel, str);
	str.Remove(L'\r');
	str.Remove(L'\n');
	if (str.IsEmpty())	return;

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	ASSERT(pFrame);
	CString strRoot = pFrame->GetRoot();
	if (strRoot.IsEmpty())	return;

	if (str.Find(strRoot) != 0)	return;

	CTranzxApp* pApp = (CTranzxApp*)AfxGetApp();
	pApp->OpenDocumentFile(str);

	if (m_nTroubleCode > 0) {
		CChildFrame* pChild = (CChildFrame*)pFrame->GetActiveFrame();
		CTranzxView* pView = (CTranzxView*)pChild->GetActiveView();
		ASSERT(pView);
		pView->FindTroubleCode(m_nTroubleCode, -1);
	}

	CListBox::OnLButtonDblClk(nFlags, point);
}
