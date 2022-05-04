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
// CResourceViewBar �޽��� ó����

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
	//	TRACE0("�Ӽ� �޺� ���ڸ� ������ ���߽��ϴ�. \n");
	//	return -1;
	//}
	//
	//m_wndObjectCombo.AddString(L"���� ���α׷�");
	//m_wndObjectCombo.AddString(L"�Ӽ� â");
	//m_wndObjectCombo.SetFont(CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT)));
	//m_wndObjectCombo.SetCurSel(0);

	//if (!m_wndPropList.Create(WS_VISIBLE | WS_CHILD, rectDummy, this, 2)) {
	//	TRACE0("�Ӽ� ǥ�� ������ ���߽��ϴ�. \n");
	//	return -1;
	//}

	//InitPropList();

	//m_wndToolBar.Create(this, AFX_DEFAULT_TOOLBAR_STYLE, IDR_PROPERTIES);
	//m_wndToolBar.LoadToolBar(IDR_PROPERTIES, 0, 0, TRUE /* ��� */);
	//m_wndToolBar.CleanUpLockedImages();
	//m_wndToolBar.LoadBitmap(theApp.m_bHiColorIcons ? IDB_PROPERTIES_HC : IDR_PROPERTIES, 0, 0, TRUE /* ��� */);
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
//	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
//}
//
//void CPropertiesWnd::OnUpdateProperties1(CCmdUI* /*pCmdUI*/)
//{
//	// TODO: ���⿡ ��� ������Ʈ UI ó���� �ڵ带 �߰��մϴ�.
//}
//
//void CPropertiesWnd::OnProperties2()
//{
//	// TODO: ���⿡ ��� ó���� �ڵ带 �߰��մϴ�.
//}
//
//void CPropertiesWnd::OnUpdateProperties2(CCmdUI* /*pCmdUI*/)
//{
//	// TODO: ���⿡ ��� ������Ʈ UI ó���� �ڵ带 �߰��մϴ�.
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
//	CMFCPropertyGridProperty* pGroup1 = new CMFCPropertyGridProperty(L"���");
//
//	pGroup1->AddSubItem(new CMFCPropertyGridProperty(L"3D ���", (_variant_t) false, L"â�� ���� �۲��� �ƴ� �۲��� �����ϰ�, ��Ʈ�ѿ� 3D �׵θ��� �����մϴ�."));
//
//	CMFCPropertyGridProperty* pProp = new CMFCPropertyGridProperty(L"�׵θ�", L"Dialog Frame", L"None, Thin, Resizable �Ǵ� Dialog Frame �� �ϳ��� �����մϴ�.");
//	pProp->AddOption(L"None");
//	pProp->AddOption(L"Thin");
//	pProp->AddOption(L"Resizable");
//	pProp->AddOption(L"Dialog Frame");
//	pProp->AllowEdit(FALSE);
//
//	pGroup1->AddSubItem(pProp);
//	pGroup1->AddSubItem(new CMFCPropertyGridProperty(L"ĸ��", (_variant_t) L"����", L"â�� ���� ǥ���ٿ� ǥ�õǴ� �ؽ�Ʈ�� �����մϴ�."));
//
//	m_wndPropList.AddProperty(pGroup1);
//
//	CMFCPropertyGridProperty* pSize = new CMFCPropertyGridProperty(L"â ũ��", 0, TRUE);
//
//	pProp = new CMFCPropertyGridProperty(L"����", (_variant_t) 250l, L"â�� ���̸� �����մϴ�.");
//	pProp->EnableSpinControl(TRUE, 50, 300);
//	pSize->AddSubItem(pProp);
//
//	pProp = new CMFCPropertyGridProperty(L"�ʺ�", (_variant_t) 150l, L"â�� �ʺ� �����մϴ�.");
//	pProp->EnableSpinControl(TRUE, 50, 200);
//	pSize->AddSubItem(pProp);
//
//	m_wndPropList.AddProperty(pSize);
//
//	CMFCPropertyGridProperty* pGroup2 = new CMFCPropertyGridProperty(L"�۲�");
//
//	LOGFONT lf;
//	CFont* font = CFont::FromHandle((HFONT) GetStockObject(DEFAULT_GUI_FONT));
//	font->GetLogFont(&lf);
//
//	lstrcpy(lf.lfFaceName, L"���� ���");
//
//	pGroup2->AddSubItem(new CMFCPropertyGridFontProperty(L"�۲�", lf, CF_EFFECTS | CF_SCREENFONTS, L"â�� �⺻ �۲��� �����մϴ�."));
//	pGroup2->AddSubItem(new CMFCPropertyGridProperty(L"�ý��� �۲��� ����մϴ�.", (_variant_t) true, L"â���� MS Shell Dlg �۲��� ����ϵ��� �����մϴ�."));
//
//	m_wndPropList.AddProperty(pGroup2);
//
//	CMFCPropertyGridProperty* pGroup3 = new CMFCPropertyGridProperty(L"��Ÿ");
//	pProp = new CMFCPropertyGridProperty(L"(�̸�)", L"���� ���α׷�");
//	pProp->Enable(FALSE);
//	pGroup3->AddSubItem(pProp);
//
//	CMFCPropertyGridColorProperty* pColorProp = new CMFCPropertyGridColorProperty(L"â ����", RGB(210, 192, 254), NULL, L"â�� �⺻ ������ �����մϴ�.");
//	pColorProp->EnableOtherButton(L"��Ÿ...");
//	pColorProp->EnableAutomaticButton(L"�⺻��", ::GetSysColor(COLOR_3DFACE));
//	pGroup3->AddSubItem(pColorProp);
//
//	static TCHAR BASED_CODE szFilter[] = L"������ ����(*.ico)|*.ico|��� ����(*.*)|*.*||";
//	pGroup3->AddSubItem(new CMFCPropertyGridFileProperty(L"������", TRUE, L"", L"ico", 0, szFilter, L"â �������� �����մϴ�."));
//
//	pGroup3->AddSubItem(new CMFCPropertyGridFileProperty(L"����", L"c:\\"));
//
//	m_wndPropList.AddProperty(pGroup3);
//
//	CMFCPropertyGridProperty* pGroup4 = new CMFCPropertyGridProperty(L"����");
//
//	CMFCPropertyGridProperty* pGroup41 = new CMFCPropertyGridProperty(L"ù��° ���� ����");
//	pGroup4->AddSubItem(pGroup41);
//
//	CMFCPropertyGridProperty* pGroup411 = new CMFCPropertyGridProperty(L"�� ��° ���� ����");
//	pGroup41->AddSubItem(pGroup411);
//
//	pGroup411->AddSubItem(new CMFCPropertyGridProperty(L"�׸� 1", (_variant_t) L"�� 1", L"�����Դϴ�."));
//	pGroup411->AddSubItem(new CMFCPropertyGridProperty(L"�׸� 2", (_variant_t) L"�� 2", L"�����Դϴ�."));
//	pGroup411->AddSubItem(new CMFCPropertyGridProperty(L"�׸� 3", (_variant_t) L"�� 3", L"�����Դϴ�."));
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
