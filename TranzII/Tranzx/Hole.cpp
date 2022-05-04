// Hole.cpp
#include "stdafx.h"
#include "Tranzx.h"
#include "Hole.h"

#include "TranzxView.h"
#include "TranzxDoc.h"
#include "Range.h"

IMPLEMENT_DYNAMIC(CHole, CWnd)

CHole::CHole(CWnd* pParent)
{
	m_pParent = pParent;
	m_ptCur.x = m_ptCur.y = 0;
	m_nGrid = -1;
	m_rect = CRect(0, 100, 0, 100);
	m_pLands = NULL;
}

CHole::~CHole()
{
	if (m_pLands != NULL) {
		delete m_pLands;
		m_pLands = NULL;
	}
}

BOOL CHole::Create(DWORD dwStyle, const RECT& rect, CWnd* pParent, UINT nID)
{
	static CString strClassName = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW);

	if (CWnd::Create(strClassName, NULL, dwStyle, rect, pParent, nID)) {
		m_pParent = pParent;

		CClientDC dc(this);
		GetClientRect(m_rect);
		CString str;
		str.Format(L"[HOLE] ÇÁ·¹ÀÓ left=%d top=%d right=%d bottom=%d", m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
		Log(str);

		m_pLands = new CLands(this);
		return m_pLands->Create(WS_VISIBLE | WS_CHILD, m_rect, this);
	}
	return FALSE;
}

//============================================================================

void CHole::Log(CString str)
{
	CTranzxView* pView = (CTranzxView*)m_pParent;
	ASSERT(pView);

	pView->Log(str);
}

bool CHole::Entrance(int id, CString strName, COLORREF color, CPoint* pPoint)
{
	if (m_pLands != NULL)	return m_pLands->Entrance(id, strName, color, pPoint);
	return false;
}

bool CHole::EntranceA(int id, CString strName, COLORREF color, BOOL bNew)
{
	if (m_pLands != NULL)	return m_pLands->EntranceA(id, strName, color, bNew);
	return false;
}

void CHole::Redraw(CPoint pt)
{
	m_ptCur = pt;
	CRect rect;
	GetClientRect(&rect);
	m_rect = rect;
	if (m_pLands != NULL)	m_pLands->Redraw(m_ptCur, rect);
	SetPos(m_ptCur.y);
}

void CHole::SetPos(CPoint pt)
{
	m_ptCur = pt;
	if (m_pLands != NULL)	m_pLands->SetPos(m_ptCur);
}

void CHole::SetPos(int y)
{
	m_ptCur.y = y;
	if (m_pLands != NULL) {	//m_pLands->SetPos(m_ptCur.y);
		m_pLands->SetWindowPos(NULL, 0, -y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
		m_pLands->Invalidate();
	}
	InvalidateRect(m_rect);
}

void CHole::SetGrid(int grid)
{
	m_nGrid = grid;
	if (m_pLands != NULL)	m_pLands->SetGrid(m_nGrid);
}

//int CHole::GetScale()
//{
//	CTranzxView* pView = (CTranzxView*)m_pParent;
//	ASSERT(pView);
//
//	return pView->GetScale();
//}
//
//void CHole::SetScale(int scale)
//{
//	CTranzxView* pView = (CTranzxView*)m_pParent;
//	ASSERT(pView);
//
//	pView->SetScale(scale);
//}

int CHole::GetHeight()
{
	if (m_pLands != NULL)	return m_pLands->GetHeight();
	return 0;
}

void CHole::Copy(CDC* pDC, int nPage, int sizex, CSize factor, CRect& rect)
{
	if (m_pLands != NULL)	m_pLands->Copy(pDC, nPage, sizex, factor, rect);
}

//============================================================================

BEGIN_MESSAGE_MAP(CHole, CWnd)
//	ON_WM_PAINT()
	ON_WM_SIZE()
END_MESSAGE_MAP()

/*void CHole::OnPaint()
{
	CPaintDC dc(this);

	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);

	CRect rect;
	GetClientRect(rect);

	CBitmap bitmapMem;
	bitmapMem.CreateCompatibleBitmap(&dc, m_rect.Width(), m_rect.Height());
	CBitmap* pBitmapOld = (CBitmap*)dcMem.SelectObject(&bitmapMem);

	if (dcMem.GetSafeHdc() != NULL) {
		dcMem.BitBlt(0, 0, rect.Width(), rect.Height(), &m_dcGrid, 0, 0, SRCCOPY);	//m_ptCur.x, m_ptCur.y, SRCCOPY);
		dcMem.BitBlt(0, 0, rect.Width(), rect.Height(), &m_dcPlot, 0, m_ptCur.y, SRCAND);
		dc.BitBlt(0, 0, rect.Width(), rect.Height(), &dcMem, 0, 0, SRCCOPY);
	}
	dcMem.SelectObject(pBitmapOld);
}
*/
void CHole::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
}
