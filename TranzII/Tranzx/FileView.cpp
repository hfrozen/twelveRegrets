#include "stdafx.h"
#include "mainfrm.h"
#include "FileView.h"
#include "Resource.h"
#include "Tranzx.h"
#include "TranzxDoc.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CFileView

#define	TIMER_FILEVIEW		2
#define	FILEVIEW_TIME		500

CFileView::CFileView()
{
	m_strRoot = L"";
}

CFileView::~CFileView()
{
}

BOOL CFileView::OnShowControlBarMenu(CPoint point)
{
	return FALSE;
}

//============================================================================

void CFileView::Log(CString str)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	pFrame->Log(str);
}

bool CFileView::IsFolder(CString strFolder)
{
	CFileFind file;
	CString strFile = L"\\*.*";
	BOOL bResult = file.FindFile(strFolder + strFile);
	return bResult ? true : false;
}

HTREEITEM CFileView::InsertNode(CString strParent, TCHAR* pNodeName, HTREEITEM hParent)
{
	CString str;
	str.Format(L"%s\\%s", strParent, pNodeName);
	SHFILEINFO sfi = { 0 };
	SHGetFileInfo(str, 0, &sfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME | SHGFI_TYPENAME | SHGFI_SYSICONINDEX);
	HTREEITEM hItem = m_wndFileView.InsertItem(pNodeName, sfi.iIcon, sfi.iIcon, hParent);
	return hItem;
}

HTREEITEM CFileView::InsertNode(CString strParent, WIN32_FIND_DATA fd, HTREEITEM hParent)
{
	CString str;
	str.Format(L"%s\\%s", strParent, fd.cFileName);
	SHFILEINFO sfi = { 0 };
	SHGetFileInfo(str, 0, &sfi, sizeof(SHFILEINFO), SHGFI_DISPLAYNAME | SHGFI_TYPENAME | SHGFI_SYSICONINDEX);
	CString strNode;
	if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		strNode.Format(L"%s", fd.cFileName);
	else	strNode.Format(L"%s (%ld)", fd.cFileName, fd.nFileSizeLow);
	HTREEITEM hItem = m_wndFileView.InsertItem((LPCTSTR)strNode, sfi.iIcon, sfi.iIcon, hParent);
	return hItem;
}

bool CFileView::IsSysDirectory(CString strParent, TCHAR* pNodeName)
{
	if ((_tcslen(pNodeName) == 1 && *(pNodeName + 0) == '.') ||
		(_tcslen(pNodeName) == 2 && *(pNodeName + 0) == '.' && *(pNodeName + 1) == '.') ||
		(PathIsSystemFolder(strParent + L"\\" + pNodeName, NULL)))
		return true;
	return false;
}

bool CFileView::MakeDirectory(CString strParent, HTREEITEM hParent)
{
	CString strPath = strParent + L"\\*";
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(strPath, &fd);
	if (hFind == INVALID_HANDLE_VALUE) {
		CString strError;
		strError.Format(L"[FILE] 유효하지 않는 핸들입니다!(%u)\r\n", hFind);
		Log(strError);
		MessageBox(strError, L"File", MB_ICONWARNING);
		return false;
	}
	if (!IsSysDirectory(strParent, fd.cFileName)) {
		HTREEITEM hItem = InsertNode(strParent, fd/*.cFileName*/, hParent);
		//if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		//	MakeDirectory(strParent + L"\\" + fd.cFileName, hItem);
	}

	while (FindNextFile(hFind, &fd) != 0) {
		if (!IsSysDirectory(strParent, fd.cFileName)) {
			HTREEITEM hItem = InsertNode(strParent, fd/*.cFileName*/, hParent);
			//if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			//	MakeDirectory(strParent + L"\\" + fd.cFileName, hItem);
		}
	}
	DWORD dwError = GetLastError();
	FindClose(hFind);
	if (dwError != ERROR_NO_MORE_FILES) {
		CString strError;
		strError.Format(L"[FILE] 다음 파일을 찾을 수 없습니다!(%u).\r\n", dwError);
		Log(strError);
		MessageBox(strError, L"File", MB_ICONWARNING);
		return false;
	}
	return true;
}

CString CFileView::GetFullNodeName(HTREEITEM hNode)
{
	HTREEITEM hRoot = m_wndFileView.GetRootItem();
	if (hRoot == hNode)	return m_strRoot;
	CString strPath;
	strPath.Empty();
	while (hNode != NULL) {
		CString strCur = m_wndFileView.GetItemText(hNode);
		CString strTemp;
		if (strPath.IsEmpty())	strTemp = L"\\" + strCur;
		else	strTemp.Format(L"\\%s%s", strCur, strPath);
		strPath = strTemp;
		hNode = m_wndFileView.GetParentItem(hNode);
	}
	if (!strPath.IsEmpty())	strPath = strPath.Mid(1);
	return strPath;
}

bool CFileView::CreateSysImageList(CImageList* pImageList, CString root, BOOL bLargeIcon)
{
	ASSERT(pImageList != NULL);
	pImageList->Detach();
	pImageList->DeleteImageList();
	SHFILEINFO sfi = { 0 };
	CImageList* pSysImageList = NULL;
	if (bLargeIcon)
		pImageList->Attach((HIMAGELIST)SHGetFileInfo(L"D:\\", 0, &sfi, sizeof(SHFILEINFO), SHGFI_LARGEICON | SHGFI_SYSICONINDEX));
	else
		pImageList->Attach((HIMAGELIST)SHGetFileInfo(L"D:\\", 0, &sfi, sizeof(SHFILEINFO), SHGFI_SMALLICON | SHGFI_SYSICONINDEX));
	return true;
}

bool CFileView::Unfold()
{
	if (m_strRoot.IsEmpty() || m_strRoot == L"") {
		CString strError = L"[FILE] 루트가 비었습니다!";
		Log(strError);
		MessageBox(strError, L"File", MB_ICONWARNING);
		return false;
	}

	m_wndFileView.DeleteAllItems();
	CreateSysImageList(&m_imageListSmall, m_strRoot, false);
	CreateSysImageList(&m_imageListLarge, m_strRoot, true);
	m_wndFileView.SetImageList(&m_imageListSmall, TVSIL_NORMAL);

	UINT nFlags = SHGFI_DISPLAYNAME | SHGFI_TYPENAME | SHGFI_SYSICONINDEX;
	SHFILEINFO sfi = { 0 };
	SHGetFileInfo(m_strRoot, 0, &sfi, sizeof(SHFILEINFO), nFlags);
	HTREEITEM hRoot = m_wndFileView.InsertItem(m_strRoot, sfi.iIcon, sfi.iIcon, TVI_ROOT);

	BeginWaitCursor();
	MakeDirectory(m_strRoot, hRoot);
	EndWaitCursor();

	m_wndFileView.Expand(m_wndFileView.GetRootItem(), TVE_TOGGLE);
	return true;
}

void CFileView::SetRoot(CString strRoot)
{
	if (strRoot.IsEmpty()) {
		strRoot = AfxGetApp()->GetProfileStringW(L"TranzxRef", L"Folder");
		while (strRoot.IsEmpty() || !IsFolder(strRoot)) {
			BROWSEINFO bi;
			ZeroMemory(&bi, sizeof(BROWSEINFO));
			bi.hwndOwner = GetSafeHwnd();
			bi.ulFlags = BIF_RETURNONLYFSDIRS;
			bi.lpszTitle = L"Download folder select";
			LPITEMIDLIST pidl = SHBrowseForFolder(&bi);
			TCHAR szFolder[MAX_PATH * 2];
			szFolder[0] = L'\0';
			if (pidl && SHGetPathFromIDList(pidl, szFolder))	strRoot = (LPCTSTR)szFolder;
		}
	}
	m_strRoot = strRoot;
	AfxGetApp()->WriteProfileStringW(L"TranzxRef", L"Folder", m_strRoot);

	Unfold();
}

//============================================================================

BEGIN_MESSAGE_MAP(CFileView, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_PAINT()
	ON_WM_SETFOCUS()
	ON_WM_TIMER()
	ON_MESSAGE(WM_SELECTED, OnSelected)
	ON_MESSAGE(WM_RIGHTCLICK, OnRightClick)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CWorkspaceBar 메시지 처리기

int CFileView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)
		return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | TVS_HASLINES | TVS_LINESATROOT | TVS_HASBUTTONS;
	if (!m_wndFileView.Create(dwViewStyle, rectDummy, this, 4)) {
		TRACE0("파일 뷰를 만들지 못했습니다.\n");
		return -1;
	}

	m_wndFileView.SetParent((CWnd*)this);
	m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_EXPLORER);
	m_wndToolBar.LoadToolBar(IDR_EXPLORER, 0, 0, TRUE);

	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	m_wndToolBar.SetOwner(this);
	m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	AdjustLayout();
	SetTimer(TIMER_FILEVIEW, FILEVIEW_TIME, NULL);

	return 0;
}

void CFileView::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

void CFileView::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)	return;

	CRect rectClient;
	GetClientRect(rectClient);

	int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;
	m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	m_wndFileView.SetWindowPos(NULL, rectClient.left + 1, rectClient.top + cyTlb + 1, rectClient.Width() - 2, rectClient.Height() - cyTlb - 2, SWP_NOACTIVATE | SWP_NOZORDER);
}

void CFileView::OnPaint()
{
	CPaintDC dc(this);

	CRect rectTree;
	m_wndFileView.GetWindowRect(rectTree);
	ScreenToClient(rectTree);

	rectTree.InflateRect(1, 1);
	dc.Draw3dRect(rectTree, ::GetSysColor(COLOR_3DSHADOW), ::GetSysColor(COLOR_3DSHADOW));
}

void CFileView::OnSetFocus(CWnd* pOldWnd)
{
	CDockablePane::OnSetFocus(pOldWnd);

	m_wndFileView.SetFocus();
}

void CFileView::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == TIMER_FILEVIEW) {
		KillTimer(TIMER_FILEVIEW);
		SetRoot(L"");
	}

	CDockablePane::OnTimer(nIDEvent);
}

LRESULT CFileView::OnSelected(WPARAM wParam, LPARAM lParam)
{
	HTREEITEM hSelItem = (HTREEITEM)wParam;
	CString strPath = GetFullNodeName(hSelItem);
	int n = strPath.Find((TCHAR)'(');
	if (n > 0) {
		strPath = strPath.Left(n);
		strPath.TrimRight();
	}
	CFileStatus status;
	if (CFile::GetStatus((LPCTSTR)strPath, status)) {
		if ((status.m_attribute & 0x10) && !m_wndFileView.ItemHasChildren(hSelItem))
			MakeDirectory(strPath, hSelItem);
		else if ((status.m_attribute & 0x1e) == 0) {
			// !hidden !system !volume !directory
			CTranzxApp* pApp = (CTranzxApp*)AfxGetApp();
			pApp->OpenDocumentFile(strPath);
		}
	}

	return 0;
}

LRESULT CFileView::OnRightClick(WPARAM wParam, LPARAM lParam)
{
	HTREEITEM htItem = (HTREEITEM)wParam;
	CString strPath = GetFullNodeName(htItem);
	CString str;
	str.Format(L"RCLICK %s", strPath);
	MessageBox(str);

	return 0;
}