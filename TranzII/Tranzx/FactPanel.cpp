// FactPanel.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "Tranzx.h"
#include "FactPanel.h"

#include "TranzxView.h"
#include "TimeDiv.h"

// CFactPanel

IMPLEMENT_DYNAMIC(CFactPanel, CWnd)

CFactPanel::CFactPanel(CWnd* pParent)
{
	ASSERT(pParent);
	m_pParent = pParent;
	m_pBitmap = NULL;
	m_crBkgnd = RGB(255, 255, 255);
	m_crText = RGB(16, 16, 16);
	m_brBkgnd.CreateSolidBrush(m_crBkgnd);
	m_brText.CreateSolidBrush(m_crText);
	//m_strFact.Empty();
	m_strTime.Empty();
}

CFactPanel::~CFactPanel()
{
	if (m_pBitmap != NULL)
		m_dc.SelectObject(m_pBitmap);
	m_brBkgnd.DeleteObject();
	m_brText.DeleteObject();
	m_font.DeleteObject();
}

//============================================================================

#define	OFFSET_Y	3

void CFactPanel::InvalidateCtrl()
{
	CClientDC dc(this);
	CRect rect;
	GetClientRect(rect);

	if (m_dc.GetSafeHdc() == NULL) {
		m_dc.CreateCompatibleDC(&dc);
		m_bitmap.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
		m_pBitmap = (CBitmap*)m_dc.SelectObject(&m_bitmap);
	}

	m_dc.SetBkColor(m_crBkgnd);
	m_dc.FillRect(rect, &m_brBkgnd);
//	m_dc.FrameRect(rect, &m_brText);

	CFont* pFondOld = (CFont*)m_dc.SelectObject(&m_font);
	m_dc.SetTextColor(m_crText);
	m_dc.SetTextAlign(TA_LEFT | TA_TOP);

	if (!m_strTime.IsEmpty())
		m_dc.TextOut(rect.left + 70, rect.top + OFFSET_Y, m_strTime);

	m_dc.SelectObject(pFondOld);
	InvalidateRect(rect);

}

//void CFactPanel::Draw(CString strFact, CString strTime)
//{
//	m_strFact = strFact;
//	m_button.SetWindowText(m_strFact);
//	m_strTime = L"TRIG : " + strTime;
//	InvalidateCtrl();
//}
//
void CFactPanel::Draw(CString strTime)
{
	m_strTime = L"TRIG : " + strTime;
	InvalidateCtrl();
}

bool CFactPanel::Copy(CDC* pDC, CPoint org, CSize factor, CRect& fRect)
{
	CRect rect;
	GetClientRect(rect);

	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);

	CBitmap bitmapMem;
	bitmapMem.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
	CBitmap* pBitmapOld = (CBitmap*)dcMem.SelectObject(&bitmapMem);

	if (dcMem.GetSafeHdc() == NULL) {
		dcMem.SelectObject(pBitmapOld);
		return false;
	}
	dcMem.BitBlt(0, 0, rect.Width(), rect.Height(), &m_dc, 0, 0, SRCCOPY);
	pDC->StretchBlt(org.x, org.y, rect.Width() * factor.cx, rect.Height() * factor.cy,
		&dcMem, 0, 0, rect.Width(), rect.Height(), SRCCOPY);
	dcMem.SelectObject(pBitmapOld);

	fRect.right = fRect.left + rect.Width() * factor.cx;
	fRect.bottom = fRect.top + rect.Height() * factor.cy;

	dcMem.SelectObject(pBitmapOld);
	return true;
}

//============================================================================


BEGIN_MESSAGE_MAP(CFactPanel, CWnd)
	ON_WM_PAINT()
END_MESSAGE_MAP()

BOOL CFactPanel::Create(DWORD dwStyle, const RECT& rect, CWnd* pParent, UINT nID)
{
	static CString strClassName = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW);

	BOOL bResult = CWnd::CreateEx(0, strClassName, NULL, dwStyle,
		rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, pParent->GetSafeHwnd(), (HMENU)nID);

	if (bResult) {
		CRect clientRect;
		GetClientRect(clientRect);
		m_font.CreateFont(14, 0, 0, 0, 300, false, false, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");

		//CRect btr;
		//btr.left = clientRect.left;
		//btr.right = btr.left + 65;
		//btr.top = clientRect.top;
		//btr.bottom = clientRect.bottom;
		//m_button.Create(L"DIV 1", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, btr, this, IDC_BUTTON_FACT_DIV);
		//m_button.SetFont(&m_font);
		//m_button.EnableWindow(true);
		//m_button.ShowWindow(SW_SHOW);

		InvalidateCtrl();
	}

	return bResult;
}

void CFactPanel::OnPaint()
{
	CPaintDC dc(this);

	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);

	CRect rect;
	GetClientRect(rect);

	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
	CBitmap* pBitmap = (CBitmap*)dcMem.SelectObject(&bitmap);

	if (dcMem.GetSafeHdc() != NULL) {
		dcMem.BitBlt(0, 0, rect.Width(), rect.Height(), &m_dc, 0, 0, SRCCOPY);
		dc.BitBlt(0, 0, rect.Width(), rect.Height(), &dcMem, 0, 0, SRCCOPY);
	}
	dcMem.SelectObject(pBitmap);
}

BOOL CFactPanel::OnCommand(WPARAM wParam, LPARAM lParam)
{
	//WORD id = LOWORD(wParam);
	//if (id == IDC_BUTTON_FACT_DIV) {
	//	CTranzxView* pView = (CTranzxView*)m_pParent;
	//	CTimeDiv dlg(this);
	//	dlg.m_nDiv = pView->GetScale();
	//	dlg.m_nMax = pView->GetEnableScale();
	//	if (dlg.DoModal() == IDOK)
	//		pView->SetScale(dlg.m_nDiv);
	//}

	return CWnd::OnCommand(wParam, lParam);
}
