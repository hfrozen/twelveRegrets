// TickPanel.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "Tranzx.h"
#include "TickPanel.h"

#include "TranzxView.h"
#include "TranzxDoc.h"

// CTickPanel

IMPLEMENT_DYNAMIC(CTickPanel, CWnd)

CTickPanel::CTickPanel(CWnd* pParent)
{
	ASSERT(pParent);
	m_pParent = pParent;

	m_crBkgnd = RGB(255, 255, 255);
	m_crText = RGB(16, 16, 16);
	m_crGrid = RGB(255, 0, 0);
	m_brBkgnd.CreateSolidBrush(m_crBkgnd);
	m_brText.CreateSolidBrush(m_crText);
	m_brGrid.CreateSolidBrush(m_crGrid);

	m_ptCur.x = m_ptCur.y = 0;
	m_nGrid = -1;

	m_pBitmapGrid = NULL;
	m_pBitmapPlot = NULL;
}

CTickPanel::~CTickPanel()
{
	if (m_pBitmapGrid != NULL)
		m_dcGrid.SelectObject(m_pBitmapGrid);
	if (m_pBitmapPlot != NULL)
		m_dcPlot.SelectObject(m_pBitmapPlot);
	m_brGrid.DeleteObject();
	m_brText.DeleteObject();
	m_brBkgnd.DeleteObject();
	m_font.DeleteObject();
}

//============================================================================

void CTickPanel::InvalidateCtrl()
{
	CClientDC dc(this);
	CRect rect;
	GetClientRect(rect);
	CSize sz = rect.Size();
	if (sz.cx < 1 || sz.cy < 1)	return;

	if (m_dcGrid.GetSafeHdc() == NULL) {
		m_dcGrid.CreateCompatibleDC(&dc);
		m_bitmapGrid.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
		m_pBitmapGrid = (CBitmap*)m_dcGrid.SelectObject(&m_bitmapGrid);
	}

	m_dcGrid.SetBkColor(m_crBkgnd);
	m_dcGrid.FillRect(rect, &m_brBkgnd);
	if (m_nGrid > 0) {
		CPen pen(PS_SOLID, 0, m_crGrid);
		CPen* pPenOld = (CPen*)m_dcGrid.SelectObject(&pen);
		CBrush* pBrushOld = (CBrush*)m_dcGrid.SelectObject(&m_brGrid);
		CPoint pts[3] = { CPoint(m_nGrid, rect.top + 5), CPoint(m_nGrid - 5, rect.bottom - 5), CPoint(m_nGrid + 5, rect.bottom - 5) };
		m_dcGrid.Polygon(pts, 3);
		m_dcGrid.SelectObject(pBrushOld);
		m_dcGrid.SelectObject(pPenOld);
	}

	rect.right = rect.left + MAX_X;
	if (m_dcPlot.GetSafeHdc() == NULL) {
		m_dcPlot.CreateCompatibleDC(&dc);
		m_bitmapPlot.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
		m_pBitmapPlot = (CBitmap*)m_dcPlot.SelectObject(&m_bitmapPlot);
		m_dcPlot.SetBkColor(m_crBkgnd);
		m_dcPlot.FillRect(rect, &m_brBkgnd);
	}

	InvalidateRect(rect);
}

void CTickPanel::MakeRule()
{
	if (m_dcPlot.GetSafeHdc() == NULL)	return;

	CTranzxView* pView = (CTranzxView*)m_pParent;
	if (pView == NULL)	return;
	CTranzxDoc* pDoc = (CTranzxDoc*)pView->GetDocument();
	if (pDoc == NULL)	return;

	CRect rect;
	GetClientRect(rect);

	rect.right = rect.left + MAX_X;		// pDoc->GetMaxX();
	m_dcPlot.SetBkColor(m_crBkgnd);
	m_dcPlot.FillRect(rect, &m_brBkgnd);

	CFont* pFontOld = (CFont*)m_dcPlot.SelectObject(&m_font);
	m_dcPlot.SetTextColor(m_crText);
	CPen solid(PS_SOLID, 0, m_crText);
	CPen* pPenOld = (CPen*)m_dcPlot.SelectObject(&solid);

	//m_dc.FrameRect(rect, &m_brText);
	CString str;
	str.Format(L"[TICK] left=%d right=%d top=%d bottom=%d", rect.left, rect.right, rect.top, rect.bottom);
	pView->Log(str);

	m_dcPlot.SetTextColor(m_crText);
	m_dcPlot.SetTextAlign(TA_CENTER | TA_TOP);

	int max = (int)pDoc->GetMaxX();
	for (int x = 0; x < max; x ++) {
		CTick cur;
		int crevice = pDoc->GetLogbookTime(x, cur);
		if (crevice > -1) {
			if (!crevice) {
				CTick cur;
				pDoc->GetLogbookTime(x, cur);
				m_dcPlot.MoveTo(rect.left + x, rect.top);
				int y;
				if (cur.m_nSec % 5)	y = 2;
				else	y = 4;
				m_dcPlot.LineTo(rect.left + x, rect.top + y);
				if (cur.m_nSec == 0) {	// && !(cur.m_nMin % pDoc->GetTimeByScale())) {
					str.Format(L"%02d:%02d:%02d", cur.m_nHour, cur.m_nMin, cur.m_nSec);
					m_dcPlot.TextOutW(rect.left + x, rect.top + 5, str);
				}
			}
		}
	}

	m_dcPlot.SelectObject(pPenOld);
	m_dcPlot.SelectObject(pFontOld);

	InvalidateRect(rect);
}

void CTickPanel::Draw(CPoint* ppt)
{
	if (ppt != NULL && ppt->x >= 0)	m_ptCur = *ppt;

	CTranzxView* pView = (CTranzxView*)m_pParent;
	ASSERT(pView);
	CTranzxDoc* pDoc = (CTranzxDoc*)pView->GetDocument();
	ASSERT(pDoc);

	CRect rect;
	GetClientRect(rect);
	//rect.right = rect.left + pDoc->GetMaxXbyScale();
	SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
	MakeRule();
	InvalidateCtrl();
}

void CTickPanel::SetPos(CPoint pt)
{
	if (pt.x < 0)	return;

	m_ptCur = pt;
	Invalidate();
}

bool CTickPanel::Copy(CDC* pDC, CPoint org, int nPage, int sizex, CSize factor, CRect& fRect)
{
	if (nPage < 1)	return FALSE;

	int sx, dx, dw;
	if (nPage == 1) {
		sx = 0;
		dx = ITEM_NAME_X;
		dw = sizex;
	}
	else {
		sx = (sizex - ITEM_NAME_X) + (nPage - 2) * sizex;
		dx = 0;
		dw = sizex;
	}
	//if (sw > MAX_X)	return;

	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);

	CBitmap bitmapMem;
	bitmapMem.CreateCompatibleBitmap(pDC, dw - dx, ITEM_TIME_Y);
	CBitmap* pBitmapOld = (CBitmap*)dcMem.SelectObject(&bitmapMem);

	if (dcMem.GetSafeHdc() == NULL) {
		dcMem.SelectObject(pBitmapOld);
		return false;
	}
	dcMem.BitBlt(0, 0, dw, ITEM_TIME_Y, &m_dcPlot, sx, 0, SRCCOPY);
	pDC->StretchBlt(org.x, org.y, dw * factor.cx, ITEM_TIME_Y * factor.cy,
		&dcMem, 0, 0, dw, ITEM_TIME_Y, SRCCOPY);
	dcMem.SelectObject(pBitmapOld);

	fRect.right = fRect.left + (dw - dx) * factor.cx;
	fRect.bottom = fRect.top + ITEM_TIME_Y * factor.cy;

	dcMem.SelectObject(pBitmapOld);
	return true;
}


//============================================================================

BEGIN_MESSAGE_MAP(CTickPanel, CWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

BOOL CTickPanel::Create(DWORD dwStyle, const RECT& rect, CWnd* pParent, UINT nID)
{
	static CString strClassName = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW);

	BOOL bResult = CWnd::CreateEx(0, strClassName, NULL, dwStyle,
		rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, pParent->GetSafeHwnd(), (HMENU)nID);

	if (bResult) {
		m_font.CreateFont(12, 0, 0, 0, 300, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
		InvalidateCtrl();
	}

	return bResult;
}

// CTickPanel 메시지 처리기입니다.

void CTickPanel::OnPaint()
{
	CPaintDC dc(this);

	CRect rect;
	GetClientRect(rect);

	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);

	CBitmap bitmap;
	bitmap.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
	CBitmap* pBitmap = (CBitmap*)dcMem.SelectObject(&bitmap);

	if (dcMem.GetSafeHdc() != NULL) {
		dcMem.BitBlt(0, 0, rect.Width(), rect.Height(), &m_dcGrid, 0, 0, SRCCOPY);
		dcMem.BitBlt(0, 0, rect.Width(), rect.Height(), &m_dcPlot, m_ptCur.x, 0, SRCAND);
		dc.BitBlt(0, 0, rect.Width(), rect.Height(), &dcMem, 0, 0, SRCCOPY);
	}
	dcMem.SelectObject(pBitmap);
}

void CTickPanel::OnLButtonUp(UINT nFlags, CPoint point)
{
	CTranzxView* pView = (CTranzxView*)m_pParent;
	CString str;
	str.Format(L"[TICK] x=%d y=%d", point.x, point.y);
	pView->Log(str);
	pView->SetGrid(point.x);
	InvalidateCtrl();

	CWnd::OnLButtonUp(nFlags, point);
}
