#include "stdafx.h"

#include "PropertiesWnd.h"
#include "Resource.h"
#include "MainFrm.h"
#include "Tranzx.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar

CPropertiesWnd::CPropertiesWnd()
{
	m_bSelectTrouble = false;
}

CPropertiesWnd::~CPropertiesWnd()
{
}

BEGIN_MESSAGE_MAP(CPropertiesWnd, CDockablePane)
	ON_WM_CREATE()
	ON_WM_SIZE()
	//ON_COMMAND(ID_EXPAND_ALL, OnExpandAllProperties)
	//ON_UPDATE_COMMAND_UI(ID_EXPAND_ALL, OnUpdateExpandAllProperties)
	//ON_COMMAND(ID_SORTPROPERTIES, OnSortProperties)
	//ON_UPDATE_COMMAND_UI(ID_SORTPROPERTIES, OnUpdateSortProperties)
	//ON_COMMAND(ID_PROPERTIES1, OnProperties1)
	//ON_UPDATE_COMMAND_UI(ID_PROPERTIES1, OnUpdateProperties1)
	//ON_COMMAND(ID_PROPERTIES2, OnProperties2)
	//ON_UPDATE_COMMAND_UI(ID_PROPERTIES2, OnUpdateProperties2)
	//ON_WM_SETFOCUS()
	//ON_WM_SETTINGCHANGE()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CResourceViewBar 메시지 처리기

void CPropertiesWnd::AdjustLayout()
{
	if (GetSafeHwnd() == NULL)	return;

	CRect rectClient;
	GetClientRect(rectClient);

	//m_wndObjectCombo.GetWindowRect(&rectCombo);

	//int cyCmb = rectCombo.Size().cy;
	//int cyTlb = m_wndToolBar.CalcFixedLayout(FALSE, TRUE).cy;

	//m_wndObjectCombo.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), 200, SWP_NOACTIVATE | SWP_NOZORDER);
	//m_wndToolBar.SetWindowPos(NULL, rectClient.left, rectClient.top + cyCmb, rectClient.Width(), cyTlb, SWP_NOACTIVATE | SWP_NOZORDER);
	//m_wndPropList.SetWindowPos(NULL, rectClient.left, rectClient.top, rectClient.Width(), rectClient.Height(), SWP_NOACTIVATE | SWP_NOZORDER);
}

int CPropertiesWnd::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CDockablePane::OnCreate(lpCreateStruct) == -1)	return -1;

	CRect rectDummy;
	rectDummy.SetRectEmpty();

	m_selectTrouble.Create(IDD_DIALOG_SELECT_TROUBLE, this);
	m_selectTrouble.SetWindowPos(NULL, 4, 4, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
	m_selectTrouble.ShowWindow(SW_HIDE);
	m_bSelectTrouble = false;
	//const DWORD dwViewStyle = WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER | CBS_SORT | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
	//if (!m_wndObjectCombo.Create(dwViewStyle, rectDummy, this, 1)) {
	//	TRACE0("속성 콤보 상자를 만들지 못했습니다. \n");
	//	return -1;
	//}
	//
	//m_wndObjectCombo.AddString(L"응용 프로그램");
	//m_wndObjectCombo.AddString(L"속성 창");
	//m_wndObjectCombo.SetFont(CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT)));
	//m_wndObjectCombo.SetCurSel(0);

	//if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2)) {
	//	TRACE0("속성 표를 만들지 못했습니다. \n");
	//	return -1;
	//}

	//InitPropList();

	//m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
	//m_wndToolBar.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE /* 잠금 */);
	//m_wndToolBar.CleanUpLockedImages();
	//m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* 잠금 */);
	//
	//m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() | CBRS_TOOLTIPS | CBRS_FLYBY);
	//m_wndToolBar.SetPaneStyle(m_wndToolBar.GetPaneStyle() & ~(CBRS_GRIPPER | CBRS_SIZE_DYNAMIC | CBRS_BORDER_TOP | CBRS_BORDER_BOTTOM | CBRS_BORDER_LEFT | CBRS_BORDER_RIGHT));
	//m_wndToolBar.SetOwner(this);
	//
	//m_wndToolBar.SetRouteCommandsViaFrame(FALSE);

	AdjustLayout();
	return 0;
}

void CPropertiesWnd::OnSize(UINT nType, int cx, int cy)
{
	CDockablePane::OnSize(nType, cx, cy);
	AdjustLayout();
}

//void CPropertiesWnd::OnExpandAllProperties()
//{
//	m_wndPropList.ExpandAll();
//}
//
//void CPropertiesWnd::OnUpdateExpandAllProperties(CCmdUI* pCmdUI)
//{
//}
//
//void CPropertiesWnd::OnSortProperties()
//{
//	m_wndPropList.SetAlphabeticMode(!m_wndPropList.IsAlphabeticMode());
//}
//
//void CPropertiesWnd::OnUpdateSortProperties(CCmdUI* pCmdUI)
//{
//	pCmdUI->SetCheck(m_wndPropList.IsAlphabeticMode());
//}
//
//void CPropertiesWnd::OnProperties1()
//{
//	// TODO: 여기에 명령 처리기 코드를 추가합니다.
//}
//
//void CPropertiesWnd::OnUpdateProperties1(CCmdUI* /*pCmdUI*/)
//{
//	// TODO: 여기에 명령 업데이트 UI 처리기 코드를 추가합니다.
//}
//
//void CPropertiesWnd::OnProperties2()
//{
//	// TODO: 여기에 명령 처리기 코드를 추가합니다.
//}
//
//void CPropertiesWnd::OnUpdateProperties2(CCmdUI* /*pCmdUI*/)
//{
//	// TODO: 여기에 명령 업데이트 UI 처리기 코드를 추가합니다.
//}
//
//void CPropertiesWnd::InitPropList()
//{
//	SetPropListFont();
//
//	m_wndPropList.EnableHeaderCtrl(FALSE);
//	m_wndPropList.EnableDescriptionArea();
//	m_wndPropList.SetVSDotNetLook();
//	m_wndPropList.MarkModifiedProperties();
//
//	CMFCPropertyGridProperty* pGroup1 = new CMFCPropertyGridProperty(L"모양");
//
//	pGroup1->AddSubItem(new CMFCPropertyGridProperty(L"3D 모양", (_variant_t) false, L"창에 굵은 글꼴이 아닌 글꼴을 지정하고, 컨트롤에 3D 테두리를 지정합니다."));
//
//	CMFCPropertyGridProperty* pProp = new CMFCPropertyGridProperty(L"테두리", L"Dialog Frame", L"None, Thin, Resizable 또는 Dialog Frame 중 하나를 지정합니다.");
//	pProp->AddOption(L"None");
//	pProp->AddOption(L"Thin");
//	pProp->AddOption(L"Resizable");
//	pProp->AddOption(L"Dialog Frame");
//	pProp->AllowEdit(FALSE);
//
//	pGroup1->AddSubItem(pProp);
//	pGroup1->AddSubItem(new CMFCPropertyGridProperty(L"캡션", (_variant_t) L"정보", L"창의 제목 표시줄에 표시되는 텍스트를 지정합니다."));
//
//	m_wndPropList.AddProperty(pGroup1);
//
//	CMFCPropertyGridProperty* pSize = new CMFCPropertyGridProperty(L"창 크기", 0, TRUE);
//
//	pProp = new CMFCPropertyGridProperty(L"높이", (_variant_t) 250l, L"창의 높이를 지정합니다.");
//	pProp->EnableSpinControl(TRUE, 50, 300);
//	pSize->AddSubItem(pProp);
//
//	pProp = new CMFCPropertyGridProperty(L"너비", (_variant_t) 150l, L"창의 너비를 지정합니다.");
//	pProp->EnableSpinControl(TRUE, 50, 200);
//	pSize->AddSubItem(pProp);
//
//	m_wndPropList.AddProperty(pSize);
//
//	CMFCPropertyGridProperty* pGroup2 = new CMFCPropertyGridProperty(L"글꼴");
//
//	LOGFONT lf;
//	CFont* font = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
//	font->GetLogFont(&lf);
//
//	lstrcpy(lf.lfFaceName, L"맑은 고딕");
//
//	pGroup2->AddSubItem(new CMFCPropertyGridFontProperty(L"글꼴", lf, CF_EFFECTS | CF_SCREENFONTS, L"창의 기본 글꼴을 지정합니다."));
//	pGroup2->AddSubItem(new CMFCPropertyGridProperty(L"시스템 글꼴을 사용합니다.", (_variant_t) true, L"창에서 MS Shell Dlg 글꼴을 사용하도록 지정합니다."));
//
//	m_wndPropList.AddProperty(pGroup2);
//
//	CMFCPropertyGridProperty* pGroup3 = new CMFCPropertyGridProperty(L"기타");
//	pProp = new CMFCPropertyGridProperty(L"(이름)", L"응용 프로그램");
//	pProp->Enable(FALSE);
//	pGroup3->AddSubItem(pProp);
//
//	CMFCPropertyGridColorProperty* pColorProp = new CMFCPropertyGridColorProperty(L"창 색상", RGB(210, 192, 254), NULL, L"창의 기본 색상을 지정합니다.");
//	pColorProp->EnableOtherButton(L"기타...");
//	pColorProp->EnableAutomaticButton(L"기본값", ::GetSysColor(COLOR_3DFACE));
//	pGroup3->AddSubItem(pColorProp);
//
//	static TCHAR BASED_CODE szFilter[] = L"아이콘 파일(*.ico)|*.ico|모든 파일(*.*)|*.*||";
//	pGroup3->AddSubItem(new CMFCPropertyGridFileProperty(L"아이콘", TRUE, L"", L"ico", 0, szFilter, L"창 아이콘을 지정합니다."));
//
//	pGroup3->AddSubItem(new CMFCPropertyGridFileProperty(L"폴더", L"c:\\"));
//
//	m_wndPropList.AddProperty(pGroup3);
//
//	CMFCPropertyGridProperty* pGroup4 = new CMFCPropertyGridProperty(L"계층");
//
//	CMFCPropertyGridProperty* pGroup41 = new CMFCPropertyGridProperty(L"첫번째 하위 수준");
//	pGroup4->AddSubItem(pGroup41);
//
//	CMFCPropertyGridProperty* pGroup411 = new CMFCPropertyGridProperty(L"두 번째 하위 수준");
//	pGroup41->AddSubItem(pGroup411);
//
//	pGroup411->AddSubItem(new CMFCPropertyGridProperty(L"항목 1", (_variant_t) L"값 1", L"설명입니다."));
//	pGroup411->AddSubItem(new CMFCPropertyGridProperty(L"항목 2", (_variant_t) L"값 2", L"설명입니다."));
//	pGroup411->AddSubItem(new CMFCPropertyGridProperty(L"항목 3", (_variant_t) L"값 3", L"설명입니다."));
//
//	pGroup4->Expand(FALSE);
//	m_wndPropList.AddProperty(pGroup4);
//}
//
//void CPropertiesWnd::OnSetFocus(CWnd* pOldWnd)
//{
//	CDockablePane::OnSetFocus(pOldWnd);
//	m_wndPropList.SetFocus();
//}
//
//void CPropertiesWnd::OnSettingChange(UINT uFlags, LPCTSTR lpszSection)
//{
//	CDockablePane::OnSettingChange(uFlags, lpszSection);
//	SetPropListFont();
//}
//
//void CPropertiesWnd::SetPropListFont()
//{
//	::DeleteObject(m_fntPropList.Detach());
//
//	LOGFONT lf;
//	afxGlobalData.fontRegular.GetLogFont(&lf);
//
//	NONCLIENTMETRICS info;
//	info.cbSize = sizeof(info);
//
//	afxGlobalData.GetNonClientMetrics(info);
//
//	lf.lfHeight = info.lfMenuFont.lfHeight;
//	lf.lfWeight = info.lfMenuFont.lfWeight;
//	lf.lfItalic = info.lfMenuFont.lfItalic;
//
//	m_fntPropList.CreateFontIndirect(&lf);
//
//	m_wndPropList.SetFont(&m_fntPropList);
//}
//

BOOL CPropertiesWnd::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(rect);
	CBrush brush(RGB(255, 255, 255));
	CBrush* pOldBrush = pDC->SelectObject(&brush);
	pDC->Rectangle(rect);
	pDC->SelectObject(pOldBrush);
	return TRUE;
	//return CDockablePane::OnEraseBkgnd(pDC);
}
