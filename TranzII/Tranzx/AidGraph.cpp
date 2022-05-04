// AidGraph.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "Tranzx.h"
#include "AidGraph.h"

#include "math.h"

// CAidGraph

#include "TranzxView.h"

IMPLEMENT_DYNAMIC(CAidGraph, CWnd)

CAidGraph::CAidGraph()
{
	m_pView = NULL;

	m_crBkgnd = RGB(255, 255, 255);
	m_crGrid = RGB(64, 64, 64);
	m_crBound = RGB(196, 196, 196);
	m_brBkgnd.CreateSolidBrush(m_crBkgnd);

	m_ptCur.x = m_ptCur.y = 0;
	m_nGrid = -1;

	m_pBitmapGrid = NULL;
	m_pBitmapPlot = NULL;

	m_scaleMin.x = 0.f;
	m_scaleMin.y = 0.f;
	m_scaleMax.x = 0.f;
	m_scaleMax.y = 0.f;
	m_factor.x = 0.f;
	m_factor.y = 0.f;
	m_current.x = 0.f;
	m_current.y = 0.f;
	m_prev.x = 0.f;
	m_prev.y = 0.f;
	m_nPrecision = 0;
	m_bPrev = false;
	m_bDevision = false;
}

CAidGraph::~CAidGraph()
{
	if (m_pBitmapGrid != NULL)
		m_dcGrid.SelectObject(m_pBitmapGrid);
	if (m_pBitmapPlot != NULL)
		m_dcPlot.SelectObject(m_pBitmapPlot);
	m_font.DeleteObject();
	m_brBkgnd.DeleteObject();
}

void CAidGraph::InvalidateCtrl()
{
	CClientDC dc(this);
	CRect rect;
	GetClientRect(rect);
	rect.right = rect.left + MAX_X;

	if (m_dcGrid.GetSafeHdc() == NULL) {
		m_dcGrid.CreateCompatibleDC(&dc);
		m_bitmapGrid.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
		m_pBitmapGrid = (CBitmap*)m_dcGrid.SelectObject(&m_bitmapGrid);
	}

	m_dcGrid.SetBkColor(m_crBkgnd);
	m_dcGrid.FillRect(rect, &m_brBkgnd);

	CFont* pFondOld = (CFont*)m_dcGrid.SelectObject(&m_font);
	m_dcGrid.SetTextColor(RGB(16, 16, 16));

	// draw outline
	CPen solid(PS_DOT, 0, m_crBound);
	CPen* pPenOld = (CPen*)m_dcGrid.SelectObject(&solid);
	m_dcGrid.MoveTo(rect.left, rect.bottom - OFFSETY_GRAPH);
	m_dcGrid.LineTo(rect.right, rect.bottom - OFFSETY_GRAPH);

	if (m_nGrid > 0) {
		CPen dash(PS_DOT, 0, m_crGrid);
		m_dcGrid.SelectObject(&dash);
		m_dcGrid.MoveTo(m_nGrid, rect.top);
		m_dcGrid.LineTo(m_nGrid, rect.bottom);
	}
	m_dcGrid.SelectObject(&pPenOld);
	m_dcGrid.SelectObject(pFondOld);

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

void CAidGraph::ErasePlot()
{
	if (m_dcPlot.GetSafeHdc() == NULL)	return;

	CRect rect;
	GetClientRect(rect);
	rect.right = rect.left + MAX_X;
	m_dcPlot.SetBkColor(m_crBkgnd);
	m_dcPlot.FillRect(rect, &m_brBkgnd);
	InvalidateRect(rect);
}

bool CAidGraph::AppendPoint(double x, double y, COLORREF color)
{
	if (m_dcPlot.GetSafeHdc() == NULL)	return false;

	CRect rect;
	GetClientRect(rect);
	rect.right = rect.left + MAX_X;

	m_current.x = x;
	m_current.y = y;
	if (m_current.x < m_scaleMin.x)	return false;

	if (!m_bPrev) {
		m_prev.x = m_current.x;
		m_prev.y = m_current.y;
		m_bPrev = true;
		return true;
	}

	CPen pen;
	pen.CreatePen(PS_SOLID, 0, color);
	CPen* pPenOld = (CPen*)m_dcPlot.SelectObject(&pen);
	int px = rect.left + (long)((m_prev.x - m_scaleMin.x) * m_factor.x);
	int py = rect.bottom - OFFSETY_GRAPH - (long)((m_prev.y - m_scaleMin.y) * m_factor.y);
	m_dcPlot.MoveTo(px, py);

	int cx = rect.left + (long)((m_current.x - m_scaleMin.x) * m_factor.x);
	int cy = rect.bottom - OFFSETY_GRAPH - (long)((m_current.y - m_scaleMin.y) * m_factor.y);
	m_dcPlot.LineTo(cx, cy);

	m_dcPlot.SelectObject(pPenOld);

	m_prev.x = m_current.x;
	m_prev.y = m_current.y;
	Invalidate();
	return true;
}

bool CAidGraph::AppendText(int x, CString str, COLORREF color)
{
	if (m_dcPlot.GetSafeHdc() == NULL)	return false;

	CRect rect;
	GetClientRect(rect);
	rect.right = rect.left + MAX_X;

	CPen pen;
	pen.CreatePen(PS_SOLID, 0, color);
	CPen* pPenOld = (CPen*)m_dcPlot.SelectObject(&pen);

	m_dcPlot.MoveTo(x, rect.top);
	m_dcPlot.LineTo(x, rect.bottom - 1);

	CFont* pFondOld = (CFont*)m_dcPlot.SelectObject(&m_font);
	m_dcPlot.SetTextColor(color);
	m_dcPlot.SetBkColor(m_crBkgnd);

	m_dcPlot.SetTextAlign(TA_LEFT | TA_BOTTOM);
	//m_dcPlot.SetBkMode(TRANSPARENT);
	//CPoint pt = rect.CenterPoint();
	m_dcPlot.TextOut(x + 2, rect.bottom - 1, str);

	m_dcPlot.SelectObject(pFondOld);
	m_dcPlot.SelectObject(pPenOld);

	Invalidate();
	return true;
}

CSize CAidGraph::GetTextWidth(CString str)
{
	CSize size(0, 0);
	if (m_dcPlot.GetSafeHdc() == NULL)	return size;
	CFont* pFondOld = (CFont*)m_dcPlot.SelectObject(&m_font);
	size = m_dcPlot.GetTextExtent((LPCTSTR)str);
	m_dcPlot.SelectObject(pFondOld);
	return size;
}

void CAidGraph::SetRange(double minx, double maxx, double miny, double maxy, int nPrecision)
{
	ASSERT(maxx > minx);
	ASSERT(maxy > miny);

	m_scaleMax.x = maxx;
	m_scaleMax.y = maxy;
	m_scaleMin.x = minx;
	m_scaleMin.y = miny;
	m_nPrecision = nPrecision;

	CRect rect;
	GetClientRect(rect);

	m_factor.x = (double)rect.Width() / (m_scaleMax.x - m_scaleMin.x);
	m_factor.y = (double)(rect.Height() - OFFSETY_GRAPH) / (m_scaleMax.y - m_scaleMin.y);
	m_factor.x = 1;
	m_prev.x = m_scaleMin.x;
	m_prev.y = m_scaleMin.y;

	InvalidateCtrl();
}

void CAidGraph::SetGridColor(COLORREF color)
{
	m_crGrid = color;
	InvalidateCtrl();
}

void CAidGraph::SetBkColor(COLORREF color)
{
	m_crBkgnd = color;
	m_brBkgnd.DeleteObject();
	m_brBkgnd.CreateSolidBrush(m_crBkgnd);
	InvalidateCtrl();
}

//void CAidGraph::Reset()
//{
//	InvalidateCtrl();
//}
//
void CAidGraph::SetPos(CPoint pt)
{
	if (pt.x < 0)	return;

	m_ptCur = pt;
	Invalidate();
}

void CAidGraph::SetGrid(int grid)
{
	m_nGrid = grid;
	InvalidateCtrl();
}

bool CAidGraph::Copy(CDC* pDC, int nPage, int sizex, CSize factor, CRect& fRect)
{
	if (nPage < 1)	return false;

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
	//if (sw > MAX_X)	return false;

	CRect rect;
	GetClientRect(rect);

	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);

	CBitmap bitmapMem;
	bitmapMem.CreateCompatibleBitmap(pDC, dw - dx, rect.Height());
	CBitmap* pBitmapOld = (CBitmap*)dcMem.SelectObject(&bitmapMem);

	if (dcMem.GetSafeHdc() == NULL) {
		dcMem.SelectObject(pBitmapOld);
		return false;
	}
	dcMem.BitBlt(0, 0, dw, rect.Height(), &m_dcGrid, 0, 0, SRCCOPY);
	/*CRect rt = rect;
	rt.bottom -= 2;
	dcMem.FillRect(rt, &m_brBkgnd);
	if (m_nGrid > 0) {
		int grid = m_ptCur.x + m_nGrid;
		if (sx <= grid && grid <= (sx + sizex)) {
			CPen dash(PS_DOT, 0, m_crGrid);
			CPen* pPenOld = (CPen*)dcMem.SelectObject(&dash);
			dcMem.MoveTo(grid - sx, rt.top);
			dcMem.LineTo(grid - sx, rt.bottom);
			dcMem.SelectObject(pPenOld);
		}
	}*/
	//dcMem.BitBlt(0, 0, dw, rect.Height(), &m_dcPlot, sx, 0, SRCAND);
	dcMem.BitBlt(0, 0, dw, rect.Height(), &m_dcPlot, sx, 0, SRCCOPY);
	pDC->StretchBlt(fRect.left, fRect.top, dw * factor.cx, rect.Height() * factor.cy,
		&dcMem, 0, 0, dw, rect.Height(), SRCCOPY);

	fRect.right = fRect.left + (dw - dx) * factor.cx;
	fRect.bottom = fRect.top + rect.Height() * factor.cy;

	dcMem.SelectObject(pBitmapOld);
	return true;
}

void CAidGraph::SetBound(CRect rect)
{
	SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
}


BEGIN_MESSAGE_MAP(CAidGraph, CWnd)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

bool CAidGraph::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	static CString strClassName = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW);

	if (CWnd::CreateEx(0, strClassName, NULL, dwStyle,
		rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, pParentWnd->GetSafeHwnd(), (HMENU)nID)) {
		m_font.CreateFont(14, 0, 0, 0, 300, false, false, 0,
			ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
		InvalidateCtrl();
		return true;
	}
	return false;
}

void CAidGraph::OnPaint()
{
	CPaintDC dc(this);

	CRect rect;
	GetClientRect(rect);

	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);

	CBitmap bitmapMem;
	bitmapMem.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
	CBitmap* pBitmapOld = (CBitmap*)dcMem.SelectObject(&bitmapMem);

	if (dcMem.GetSafeHdc() != NULL) {
		dcMem.BitBlt(0, 0, rect.Width(), rect.Height(), &m_dcGrid, 0, 0, SRCCOPY);	//m_ptCur.x, m_ptCur.y, SRCCOPY);
		dcMem.BitBlt(0, 0, rect.Width(), rect.Height(), &m_dcPlot, m_ptCur.x, 0, SRCAND);
		dc.BitBlt(0, 0, rect.Width(), rect.Height(), &dcMem, 0, 0, SRCCOPY);
	}
	dcMem.SelectObject(pBitmapOld);
	// Do not call CWnd::OnPaint()
}

void CAidGraph::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);

	CRect rect;
	GetClientRect(rect);
	m_factor.x = (double)rect.Width() / (m_scaleMax.x - m_scaleMin.x);
	m_factor.y = (double)(rect.Height() - OFFSETY_GRAPH) / (m_scaleMax.y - m_scaleMin.y);
	m_factor.x = 1;
}

void CAidGraph::OnLButtonUp(UINT nFlags, CPoint point)
{
	CTranzxView* pView = (CTranzxView*)m_pView;
	ASSERT(pView);
	pView->SetGrid(point.x);

	CWnd::OnLButtonUp(nFlags, point);
}
