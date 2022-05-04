// Dashb.cpp

#include "stdafx.h"
#include "Dts.h"
#include "Dashb.h"
#include "afxdialogex.h"

#define	DASH_TIMER		2
#define	TIME_DASH		500

IMPLEMENT_DYNAMIC(CDashb, CDialogEx)

CDashb::CDashb(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDashb::IDD, pParent)
{
	m_pParent = pParent;
}

CDashb::~CDashb()
{
	for (int n = 0; n < 6; n ++) {
		if (m_font[n].GetSafeHandle())	m_font[n].DeleteObject();
	}
}

void CDashb::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDashb, CDialogEx)
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
	ON_WM_DESTROY()
END_MESSAGE_MAP()


BOOL CDashb::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_font[0].CreateFontW(-14, 0, 0, 0, 300, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("¸¼Àº °íµñ"));
	m_font[1].CreateFontW(-16, 0, 0, 0, 400, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("¸¼Àº °íµñ"));
	m_font[2].CreateFontW(-18, 0, 0, 0, 600, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("¸¼Àº °íµñ"));
	m_font[3].CreateFontW(-14, 0, 0, 0, 300, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
	m_font[4].CreateFontW(-16, 0, 0, 0, 400, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));
	m_font[5].CreateFontW(-18, 0, 0, 0, 700, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
						CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Tahoma"));
	for (int n = 0; n < DITEM_MAX; n ++) {
		m_item[n].SubclassDlgItem(IDC_STATIC_DEV0 + n, this);
		m_item[n].SetColor(COLOR_BLUE, COLOR_WHITE);
		m_item[n].SetFont(&m_font[1]);
	}
	m_item[DITEM_DATE].SetColor(RGB(255, 255, 192), RGB(73, 141, 243));
	m_item[DITEM_DATE].SetFont(&m_font[5]);
	m_item[DITEM_TIME].SetColor(RGB(255, 255, 176), RGB(73, 141, 243));
	m_item[DITEM_TIME].SetFont(&m_font[5]);
	m_item[DITEM_SPEED].SetColor(COLOR_WHITE, RGB(99, 154,231));
	m_item[DITEM_SPEED].SetFont(&m_font[5]);
	m_item[DITEM_DIST].SetColor(COLOR_WHITE, RGB(99, 154,231));
	m_item[DITEM_DIST].SetFont(&m_font[5]);
	m_item[DITEM_CURR].SetColor(RGB(73, 7, 248), RGB(0, 255, 255));
	//m_item[DITEM_CURR].SetFont(&m_font[2]);
	m_item[DITEM_NEXT].SetColor(RGB(73, 7, 248), RGB(0, 255, 255));
	//m_item[DITEM_NEXT].SetFont(&m_font[2]);
	m_item[DITEM_DEST].SetColor(RGB(73, 7, 248), RGB(0, 255, 255));
	//m_item[DITEM_DEST].SetFont(&m_font[2]);
	SetTimer(DASH_TIMER, TIME_DASH, NULL);

	return TRUE;
}

BOOL CDashb::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)) {
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CDashb::OnDestroy()
{
	CDialogEx::OnDestroy();
}

BOOL CDashb::OnEraseBkgnd(CDC* pDC)
{
	CRect rc;
	GetClientRect(&rc);
	CBrush brush(COLOR_WHITE);
	CBrush* pOldBrush = pDC->SelectObject(&brush);
	pDC->Rectangle(rc);
	pDC->SelectObject(pOldBrush);
	return TRUE;
	//return CDialogEx::OnEraseBkgnd(pDC);
}

void CDashb::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == DASH_TIMER) {
		COleDateTime odt = COleDateTime::GetCurrentTime();
		CString str = odt.Format(L"%Y/%m/%d");
		CString strTemp;
		m_item[DITEM_DATE].GetWindowTextW(strTemp);
		if (str.Compare(strTemp))	m_item[DITEM_DATE].SetWindowTextW(str);
		str = odt.Format(L"%H:%M:%S");
		m_item[DITEM_TIME].GetWindowTextW(strTemp);
		if (str.Compare(strTemp))	m_item[DITEM_TIME].SetWindowTextW(str);
	}

	CDialogEx::OnTimer(nIDEvent);
}
