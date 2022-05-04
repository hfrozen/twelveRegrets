
#include "stdafx.h"
#include "MainFrm.h"
#include "ClassView.h"
#include "Resource.h"
#include "Tranzx.h"

#include "ChildFrm.h"
#include "TranzxDoc.h"
#include "TranzxView.h"

class CClassViewMenuButton : public CMFCToolBarMenuButton
{
	friend class CClassView;

	DECLARE_SERIAL(CClassViewMenuButton)

public:
	CClassViewMenuButton(HMENU hMenu = NULL) : CMFCToolBarMenuButton((UINT)-1, hMenu, -1)
	{
	}

	virtual void OnDraw(CDC* pDC, const CRect& rect, CMFCToolBarImages* pImages, BOOL bHorz = TRUE,
		BOOL bCustomizeMode = FALSE, BOOL bHighlight = FALSE, BOOL bDrawBorder = TRUE, BOOL bGrayDisabledButtons = TRUE)
	{
		pImages = CMFCToolBar::GetImages();

		CAfxDrawState ds;
		pImages->PrepareDrawImage(ds);

		CMFCToolBarMenuButton::OnDraw(pDC, rect, pImages, bHorz, bCustomizeMode, bHighlight, bDrawBorder, bGrayDisabledButtons);

		pImages->EndDrawImage(ds);
	}
};

IMPLEMENT_SERIAL(CClassViewMenuButton, CMFCToolBarMenuButton, 1)


	CClassView::CClassView()
{
	m_nType = FILETYPE_NON;
	m_pDoc = NULL;
	m_nCurrSort = ID_SORTING_GROUPBYTYPE;
	m_pImageList = NULL;
	m_bMakedItem = false;
	m_bChange = false;
	m_strSelectedItem = L"";
}

CClassView::~CClassView()
{
	if (m_pImageList != NULL) {
		delete m_pImageList;
		m_pImageList = NULL;
	}
}

BOOL CClassView::OnShowControlBarMenu(CPoint point)
{
	return FALSE;
}

//============================================================================

void CClassView::Log(CString str)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	pFrame->Log(str);
}

int CClassView::MakeItem(HTREEITEM hParent, BYTE nHierarchy, int id)
{
	CTranzxDoc* pDoc = (CTranzxDoc*)m_pDoc;

	HTREEITEM hItem = hParent;
	BYTE hier = 0;
	TV_INSERTSTRUCT ti;
	do {
		hier = pDoc->GetLBArchiveHierarchy(id);
		if (hier == nHierarchy) {
			TCHAR type = pDoc->GetLBArchiveType(id);
			int image;
			switch (type) {
			case 'H' :
			case 'K' :
			case 'L' :	image = 2;	break;
			case 'S' :
			case 'U' :	image = 4;	break;
			case 'D' :	image = 6;	break;
			default :	image = 0;	break;
			//default :	image = 4;	break;
			}
			ti.hParent = hParent;
			ti.hInsertAfter = TVI_LAST;
			ti.item.iImage = image;
			ti.item.iSelectedImage = image + 1;
			CString str = pDoc->GetLBArchiveName(id);
			BSTR bstr = str.AllocSysString();
			ti.item.pszText = (LPWSTR)bstr;
			SysFreeString(bstr);
			ti.item.mask = TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_TEXT;
			hItem = m_wndClassView.InsertItem(&ti);
			//hItem = m_wndClassView.InsertItem(pDoc->GetOperArchName(id), image, image + 1, hParent);
			++ id;
		}
		else if (hier > nHierarchy)	id = MakeItem(hItem, hier, id);
	} while (hier >= nHierarchy);
	return id;
}

CString  CClassView::GetFullNodeName(HTREEITEM hNode)
{
	CString strPath;
	strPath.Empty();
	while (hNode != NULL) {
		CString strCur = m_wndClassView.GetItemText(hNode);
		CString strTemp;
		if (strPath.IsEmpty())	strTemp = L"|" + strCur;
		else	strTemp.Format(L"|%s%s", strCur, strPath);
		strPath = strTemp;
		hNode = m_wndClassView.GetParentItem(hNode);
	}
	if (!strPath.IsEmpty())	strPath = strPath.Mid(1);
	return strPath;
}

int CClassView::FindItem(CString strItem, int id)
{
	CString strFind = strItem;
	int n = strItem.Find((TCHAR)'|');
	if (n > 0) {
		strFind = strItem.Left(n);
		strItem = strItem.Mid(n + 1);
	}
	else	strItem = L"";

	CTranzxDoc* pDoc = (CTranzxDoc*)m_pDoc;
	BYTE base = pDoc->GetLBArchiveHierarchy(id);
	BYTE hier = base;
	while (hier > 0) {
		if (hier == base) {
			if (!strFind.Compare(pDoc->GetLBArchiveName(id))) {
				if (strItem.IsEmpty())	return id;
				return FindItem(strItem, id + 1);
			}
		}
		hier = pDoc->GetLBArchiveHierarchy(++ id);
	}
	return -1;
}

void CClassView::SetItem(CDocument* pDoc, TCHAR nType)
{
	if (m_bMakedItem || pDoc == NULL || m_nType == nType)	return;
	m_nType = nType;
	m_pDoc = pDoc;
	m_wndClassView.DeleteAllItems();

	m_pImageList = new CImageList();
	m_pImageList->Create(16, 16, ILC_COLOR24, 8, 8);
	for (UINT nID = IDB_FD_CLS; nID <= IDB_D_OP; nID ++) {
		CBitmap bitmap;
		bitmap.LoadBitmap(nID);
		m_pImageList->Add(&bitmap, (COLORREF)0xffffff);
		bitmap.DeleteObject();
	}
	m_wndClassView.SetImageList(m_pImageList, TVSIL_NORMAL);

	MakeItem(TVI_ROOT, 1, 0);
	m_bMakedItem = true;
	m_bChange = false;
}

//void* CClassView::GetTranzxDocument()
//{
//	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
//	if (pFrame != NULL) {
//		CChildFrame* pChild = (CChildFrame*)pFrame->GetActiveFrame();
//		if (pChild != NULL) {
//			CTranzxView* pView = (CTranzxView*)pChild->GetActiveView();
//			CTranzxDoc* pDoc = (CTranzxDoc*)pChild->GetActiveDocument();
//			return pDoc;
//		}
//	}
//	return NULL;
//}
//
//============================================================================

BEGIN_MESSAGE_MAP(CClassView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	//ON_MESSAGE(WM_SELECT, OnSelect)
	ON_MESSAGE(WM_SELECTED, OnSelected)
	ON_MESSAGE(WM_DROPED, OnDroped)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClassView 메시지 처리기

int CClassView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	// 뷰를 만듭니다.
	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT |
		TVS_HASBUTTONS /*| TVS_TRACKSELECT */| WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

	if (!m_wndClassView.Create(dwViewStyle, rectDummy, this, 2))
	{
		TRACE0("클래스 뷰를 만들지 못했습니다.\n");
		return -1;      // 만들지 못했습니다.
	}
	m_wndClassView.SetParent((CWnd*)this);
	m_wndClassView.EnableDrag(TRUE);

	// 이미지를 로드합니다.
	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_SORT);
	m_wndToolBar.LoadToolBar(IDR_SORT, 0, 0, TRUE);

	//OnChangeVisualStyle();

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));

	m_wndToolBar.SetOwner(this);

	// 모든 명령은 부모 프레임이 아닌 이 컨트롤을 통해 라우팅됩니다.
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	CMenu menuSort;
	menuSort.LoadMenu(IDR_POPUP_SORT);

	m_wndToolBar.ReplaceButton(ID_SORT_MENU, CClassViewMenuButton(menuSort.GetSubMenu(0)->GetSafeHmenu()));

	CClassViewMenuButton* pButton =  DYNAMIC_DOWNCAST(CClassViewMenuButton, m_wndToolBar.GetButton(0));

	if (pButton != NULL)
	{
		pButton->m_bText = FALSE;
		pButton->m_bImage = TRUE;
		pButton->SetImage(GetCmdMgr()->GetCmdImage(m_nCurrSort));
		pButton->SetMessageWnd(this);
	}

	// 정적 트리 뷰 데이터를 더미 코드로 채웁니다.
	//FillClassView();

	return 0;
}

void CClassView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

/*void CClassView::FillClassView()
{
	HTREEITEM hRoot = m_wndClassView.InsertItem(L"FakeApp 클래스", 0, 0);
	m_wndClassView.SetItemState(hRoot, TVIS_BOLD, TVIS_BOLD);

	HTREEITEM hClass = m_wndClassView.InsertItem(L"CFakeAboutDlg", 1, 1, hRoot);
	m_wndClassView.InsertItem(L"CFakeAboutDlg()", 3, 3, hClass);

	m_wndClassView.Expand(hRoot, TVE_EXPAND);

	hClass = m_wndClassView.InsertItem(L"CFakeApp", 1, 1, hRoot);
	m_wndClassView.InsertItem(L"CFakeApp()", 3, 3, hClass);
	m_wndClassView.InsertItem(L"InitInstance()", 3, 3, hClass);
	m_wndClassView.InsertItem(L"OnAppAbout()", 3, 3, hClass);

	hClass = m_wndClassView.InsertItem(L"CFakeAppDoc", 1, 1, hRoot);
	m_wndClassView.InsertItem(L"CFakeAppDoc()", 4, 4, hClass);
	m_wndClassView.InsertItem(L"~CFakeAppDoc()", 3, 3, hClass);
	m_wndClassView.InsertItem(L"OnNewDocument()", 3, 3, hClass);

	hClass = m_wndClassView.InsertItem(L"CFakeAppView", 1, 1, hRoot);
	m_wndClassView.InsertItem(L"CFakeAppView()", 4, 4, hClass);
	m_wndClassView.InsertItem(L"~CFakeAppView()", 3, 3, hClass);
	m_wndClassView.InsertItem(L"GetDocument()", 3, 3, hClass);
	m_wndClassView.Expand(hClass, TVE_EXPAND);

	hClass = m_wndClassView.InsertItem(L"CFakeAppFrame", 1, 1, hRoot);
	m_wndClassView.InsertItem(L"CFakeAppFrame()", 3, 3, hClass);
	m_wndClassView.InsertItem(L"~CFakeAppFrame()", 3, 3, hClass);
	m_wndClassView.InsertItem(L"m_wndMenuBar", 6, 6, hClass);
	m_wndClassView.InsertItem(L"m_wndToolBar", 6, 6, hClass);
	m_wndClassView.InsertItem(L"m_wndStatusBar", 6, 6, hClass);

	hClass = m_wndClassView.InsertItem(L"Globals", 2, 2, hRoot);
	m_wndClassView.InsertItem(L"theFakeApp", 5, 5, hClass);
	m_wndClassView.Expand(hClass, TVE_EXPAND);
}*/

void CClassView::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)
	{
		return;
	}

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndClassView.SetWindowPos(NULL, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

BOOL CClassView::PreTranslateMessage(MSG* pMsg)
{
	return CDockablePane::PreTranslateMessage(pMsg);
}

void CClassView::OnPaint()
{
	CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.

	CRect rectTree;
	m_wndClassView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CClassView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_wndClassView.SetFocus();
}

/*void CClassView::OnChangeVisualStyle()
{
	m_ClassViewImages.DeleteImageList();

	UINT uiBmpId = theApp.m_bHiColorIcons ? IDB_CLASS_VIEW_24 : IDB_CLASS_VIEW;

	CBitmap bmp;
	if (!bmp.LoadBitmap(uiBmpId))
	{
		TRACE(L"비트맵을 로드할 수 없습니다. %x\n", uiBmpId);
		ASSERT(FALSE);
		return;
	}

	BITMAP bmpObj;
	bmp.GetBitmap(&bmpObj);

	UINT nFlags = ILC_MASK;

	nFlags |= (theApp.m_bHiColorIcons) ? ILC_COLOR24 : ILC_COLOR4;

	m_ClassViewImages.Create(16, bmpObj.bmHeight, nFlags, 0, 0);
	m_ClassViewImages.Add(&bmp, RGB(255, 0, 0));

	m_wndClassView.SetImageList(&m_ClassViewImages, TVSIL_NORMAL);

	m_wndToolBar.CleanUpLockedImages();
	m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_SORT_24 : IDR_SORT, 0, 0, TRUE);
}
*/

LRESULT CClassView::OnSelect(WPARAM wParam, LPARAM lParam)
{
	HTREEITEM hSelItem = (HTREEITEM)wParam;
	CString str = GetFullNodeName(hSelItem);
	int id = FindItem(str);

	return 0;
}

LRESULT CClassView::OnSelected(WPARAM wParam, LPARAM lParam)
{
	HTREEITEM hSelItem = (HTREEITEM)wParam;
	CString str = GetFullNodeName(hSelItem);
	int id = FindItem(str);
	if (id >= 0/* && m_bChange*/) {
		m_strSelectedItem = str;
		CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
		ASSERT(pFrame);
		CChildFrame* pChild = (CChildFrame*)pFrame->GetActiveFrame();
		ASSERT(pChild);
		CTranzxDoc* pDoc = (CTranzxDoc*)pChild->GetActiveDocument();
		ASSERT(pDoc);

		TCHAR type = pDoc->GetLBArchiveType(id);
		//if (/*type == 'T' || */type == 'D' || type == 'A' || type == 'S') {
		if (type != 'N') {
			CView* pView = pChild->GetActiveView();
			if (pView != NULL)
				pView->SendMessage(WM_SELITEM, (WPARAM)id);
			//str.Format(L"%d %c %s", id, pDoc->GetOperArchType(id), (CA2CT)pDoc->GetOperArchName(id));
			//Log(str);
		}
	}
	//m_bChange = TRUE;

	return 0;
}

LRESULT CClassView::OnDroped(WPARAM wParam, LPARAM lParam)
{
	HTREEITEM hSelItem = (HTREEITEM)wParam;
	CString str = GetFullNodeName(hSelItem);
	int id = FindItem(str);
	if (id >= 0) {
		m_strSelectedItem = str;
		CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
		ASSERT(pFrame);
		CChildFrame* pChild = (CChildFrame*)pFrame->GetActiveFrame();
		ASSERT(pChild);
		CView* pView = pChild->GetActiveView();
		if (pView != NULL)
			pView->SendMessage(WM_DROPITEM, (WPARAM)id, lParam);
	}

	return 0;
}

