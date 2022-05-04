// Lands.cpp
#include "stdafx.h"
#include "Tranzx.h"
#include "Lands.h"

#include "TranzxView.h"
#include "TranzxDoc.h"
#include "Hole.h"
#include "Range.h"

IMPLEMENT_DYNAMIC(CLands, CWnd)

CLands::CLands(CWnd* pParent)
{
	m_pParent = pParent;
	m_crBkgnd = RGB(255, 255, 255);
	m_crGrid = RGB(64, 64, 64);
	m_brBkgnd.CreateSolidBrush(m_crBkgnd);
	m_nGrid = -1;
	m_ptCur = CPoint(0, 0);
	m_rect = CRect(0, 100, 0, 100);
	//m_pBitmap = NULL;
	m_contentsList.RemoveAll();
}

CLands::~CLands()
{
	m_brBkgnd.DeleteObject();
	//if (m_pBitmap != NULL)
	//	m_dc.SelectObject(m_pBitmap);
	for (POSITION pos = m_contentsList.GetHeadPosition(); pos != NULL; ) {
		PCONTENTS pContents = (PCONTENTS)m_contentsList.GetNext(pos);
		if (pContents != NULL) {
			CItem* pItem = pContents->pItem;
			if (pItem != NULL) {
				delete pItem;
				pItem = NULL;
			}
			delete pContents;
		}
	}
	m_contentsList.RemoveAll();
}

bool CLands::Create(DWORD dwStyle, const RECT& rect, CWnd* pParent, UINT nID)
{
	static CString strClassName = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW);

	if (CWnd::Create(strClassName, NULL, dwStyle, rect, pParent, nID)) {
		m_pParent = pParent;

		CClientDC dc(this);
		GetClientRect(m_rect);
		m_rect.bottom = rect.top + (int)((MAX_Y - SPACE_TOP - SPACE_BOTTOM) * dc.GetDeviceCaps(LOGPIXELSY) / 25.4);
		SetWindowPos(NULL, 0, 0, m_rect.Width(), m_rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

		CString str;
		str.Format(L"[LANDS] 프레임 left=%d top=%d right=%d bottom=%d", m_rect.left, m_rect.top, m_rect.right, m_rect.bottom);
		Log(str);

		return true;
	}
	return false;
}

//============================================================================

/*void CLands::InvalidateCtrl()
{
	CClientDC dc(this);

	if (m_dc.GetSafeHdc() == NULL) {
		m_dc.CreateCompatibleDC(&dc);
		m_bitmap.CreateCompatibleBitmap(&dc, m_rect.Width(), m_rect.Height();
		m_pBitmap = (CBitmap*)m_dc.SelectObject(&m_bitmap);
	}

}*/

void CLands::Log(CString str)
{
	CTranzxView* pView = (CTranzxView*)m_pParent->GetParent();
	ASSERT(pView);

	pView->Log(str);
}

int CLands::GetLastBottom()
{
	POSITION pos = m_contentsList.GetTailPosition();
	if (pos != NULL) {
		PCONTENTS pContents = (PCONTENTS)m_contentsList.GetPrev(pos);
		return pContents->rect.bottom;
	}
	return 0;
}

CLands::PCONTENTS CLands::GetContentsByPosition(int y)
{
	for (POSITION pos = m_contentsList.GetHeadPosition(); pos != NULL; ) {
		PCONTENTS pContents = (PCONTENTS)m_contentsList.GetNext(pos);
		if (pContents != NULL) {
			CItem* pItem = pContents->pItem;
			if (pItem != NULL) {
				if (y < pContents->rect.bottom)	return pContents;
			}
		}
	}
	return NULL;
}

CAidTitle* CLands::GetPartByID(int id)
{
	for (POSITION pos = m_contentsList.GetHeadPosition(); pos != NULL; ) {
		PCONTENTS pContents = (PCONTENTS)m_contentsList.GetNext(pos);
		if (pContents != NULL) {
			CItem* pItem = pContents->pItem;
			if (pItem != NULL) {
				CAidTitle* pTitle = pItem->GetTitle(id);
				if (pTitle != NULL) {
					//pItem->SendMessage(WM_PARTDELETE, (WPARAM)id);
					return pTitle;
				}
			}
		}
	}
	return NULL;
}

bool CLands::CheckRange(int id)
{
	CTranzxView* pView = (CTranzxView*)m_pParent->GetParent();
	ASSERT(pView);
	CTranzxDoc* pDoc = (CTranzxDoc*)pView->GetDocument();
	ASSERT(pDoc);

	int64_t min = pDoc->GetLBArchivePscMin(id);
	int64_t max = pDoc->GetLBArchivePscMax(id);
	if (min >= max) {
		CRange dlg(this);
		dlg.m_var = 0;
		dlg.m_min = min;
		dlg.m_max = max;
		if (dlg.DoModal() != IDOK)	return FALSE;
		pDoc->SetLBArchivePscMin(id, dlg.m_min);
		pDoc->SetLBArchivePscMax(id, dlg.m_max);
		//pDoc->SetOperArchMax(id, min + 100);
	}
	return true;
}

bool CLands::AppendItem(int id, CString strName, COLORREF color)
{
	CTranzxView* pView = (CTranzxView*)m_pParent->GetParent();
	ASSERT(pView);
	CTranzxDoc* pDoc = (CTranzxDoc*)pView->GetDocument();
	ASSERT(pDoc);

	CRect rect;
	rect = m_rect;
	rect.top = GetLastBottom() + 1;

	TCHAR type = pDoc->GetLBArchiveType(id);
	if (type == 'N')	return FALSE;
	else if (type != 'S' && type != 'U')	rect.bottom = rect.top + ITEM_DIGITAL_Y;
	else {
		int64_t max = max(pDoc->GetLBArchivePscOffset(id), pDoc->GetLBArchiveLogOffset(id));
		max *= 4;
		max += 4;
		if (max > ITEM_ANALOG_Y)	rect.bottom = rect.top + ITEM_ANALOG_Y;
		else	rect.bottom = rect.top + (int)max ;
	}

	CString str;
	if (rect.bottom > m_rect.bottom) {
		str.Format(L"[LANDS] 보기 공간이 부족합니다!!! %d-%d", rect.bottom, m_rect.bottom);
		Log(str);
		MessageBox(str, L"Land", MB_ICONWARNING);
		return false;
	}

	CItem* pItem = new CItem(this);
	if (pItem->Create(WS_CHILD | WS_VISIBLE, rect, this)) {
		pItem->AddPart(id, strName, color);
		pItem->SetPos(m_ptCur);
		pItem->SetGrid(m_nGrid);
		PCONTENTS pContents = new CONTENTS;
		pContents->pItem = pItem;
		pContents->rect = rect;
		m_contentsList.AddTail(pContents);
		str.Format(L"[LANDS] 아이템 %d-%s(%d) left=%d top=%d right=%d bottom=%d", id,
			pDoc->GetLBArchiveName(id), m_contentsList.GetCount(),
			rect.left, rect.top, rect.right, rect.bottom);
		Log(str);
		return true;
	}
	else {
		str = L"[LANDS] 아이템을 생성할 수 없습니다!!!";
		Log(str);
		MessageBox(str, L"Land", MB_ICONWARNING);
		delete pItem;
	}
	return false;
}

bool CLands::Entrance(int id, CString strName, COLORREF color, CPoint* pPoint)
{
	CString str;
	if (GetPartByID(id) != NULL) {
		CTranzxView* pView = (CTranzxView*)m_pParent->GetParent();
		ASSERT(pView);

		pView->SendMessage(WM_PARTSELECT, (WPARAM)id);
		//str.Format(L"[LANDS] Item %d has been viewer!", id);
		//Log(str);
		return false;
	}

	CTranzxView* pView = (CTranzxView*)m_pParent->GetParent();
	ASSERT(pView);
	CTranzxDoc* pDoc = (CTranzxDoc*)pView->GetDocument();
	ASSERT(pDoc);
	TCHAR type = pDoc->GetLBArchiveType(id);
	if ((type == 'S' || type == 'U') && !CheckRange(id))	return false;
	//if (!CheckRange(id))	return false;

	if (pPoint != NULL) {
		ScreenToClient(pPoint);
		PCONTENTS pContents = GetContentsByPosition(pPoint->y);
		if (pContents != NULL) {
			CItem* pItem = pContents->pItem;
			if (pItem != NULL) {
				TCHAR type = pItem->GetFirstTitleType();
				if (type == 'S' || type == 'U')	return pItem->AddPart(id, strName, color);
			}
		}
	}
	return AppendItem(id, strName, color);
}

bool CLands::EntranceA(int id, CString strName, COLORREF color, bool bNew)
{
	CTranzxView* pView = (CTranzxView*)m_pParent->GetParent();
	ASSERT(pView);
	CTranzxDoc* pDoc = (CTranzxDoc*)pView->GetDocument();
	ASSERT(pDoc);
	TCHAR type = pDoc->GetLBArchiveType(id);
	if ((type == 'S' || type == 'U') && !CheckRange(id)) {
		CTranzxView* pView = (CTranzxView*)m_pParent->GetParent();
		ASSERT(pView);
		CTranzxDoc* pDoc = (CTranzxDoc*)pView->GetDocument();
		ASSERT(pDoc);

		int64_t min = pDoc->GetLBArchivePscMin(id);
		pDoc->SetLBArchivePscMax(id, min + 100);
	}
	if (!bNew) {
		POSITION pos = m_contentsList.GetTailPosition();
		if (pos != NULL) {
			PCONTENTS pContents = (PCONTENTS)m_contentsList.GetPrev(pos);
			if (pContents != NULL) {
				CItem* pItem = pContents->pItem;
				if (pItem != NULL) {
					pItem->AddPart(id, strName, color);
					return true;
				}
			}
		}
	}
	return AppendItem(id, strName, color);
}

void CLands::Redraw(CPoint pt, CRect rect)
{
	m_rect.left = rect.left;
	m_rect.right = rect.right;
	SetWindowPos(NULL, 0, 0, m_rect.Width(), m_rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

	CClientDC dc(this);
	dc.FillRect(m_rect, &m_brBkgnd);

	m_ptCur = pt;
	for (POSITION pos = m_contentsList.GetHeadPosition(); pos != NULL; ) {
		PCONTENTS pContents = (PCONTENTS)m_contentsList.GetNext(pos);
		if (pContents != NULL) {
			CItem* pItem = pContents->pItem;
			if (pItem != NULL) {
				CRect rt = pContents->rect;
				rt.left = m_rect.left;
				rt.right = m_rect.right;
				pItem->SetBound(rt);
				pItem->SetPos(m_ptCur);
				pItem->Redraw();
			}
		}
	}
}

void CLands::SetPos(CPoint pt)
{
	m_ptCur = pt;
	for (POSITION pos = m_contentsList.GetHeadPosition(); pos != NULL; ) {
		PCONTENTS pContents = (PCONTENTS)m_contentsList.GetNext(pos);
		if (pContents != NULL) {
			CItem* pItem = pContents->pItem;
			if (pItem != NULL)	pItem->SetPos(m_ptCur);
		}
	}
}

void CLands::SetPos(int y)
{
	m_ptCur.y = y;
	SetWindowPos(NULL, 0, - m_ptCur.y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
}

void CLands::SetGrid(int grid)
{
	m_nGrid = grid;
	for (POSITION pos = m_contentsList.GetHeadPosition(); pos != NULL; ) {
		PCONTENTS pContents = (PCONTENTS)m_contentsList.GetNext(pos);
		if (pContents != NULL) {
			CItem* pItem = pContents->pItem;
			if (pItem != NULL)	pItem->SetGrid(m_nGrid);
		}
	}
}

//int CLands::GetScale()
//{
//	CHole* pHole = (CHole*)m_pParent;
//	ASSERT(pHole);
//
//	return pHole->GetScale();
//}
//
//void CLands::SetScale(int scale)
//{
//	CHole* pHole = (CHole*)m_pParent;
//	ASSERT(pHole);
//
//	pHole->SetScale(scale);
//}

CRect CLands::PartSelect(int id)
{
	CRect rect(0, 0, 0, 0);
	for (POSITION pos = m_contentsList.GetHeadPosition(); pos != NULL; ) {
		PCONTENTS pContents = (PCONTENTS)m_contentsList.GetNext(pos);
		if (pContents != NULL) {
			CItem* pItem = pContents->pItem;
			if (pItem != NULL && pItem->PartSelect(id)) {
				rect = pContents->rect;
				/*CString str;
				str.Format(L"[LAND] left=%d top=%d right=%d bottom=%d", rect.left, rect.top, rect.right, rect.bottom);
				Log(str);*/
			}
		}
	}
	return rect;
}

void CLands::Copy(CDC* pDC, int nPage, int sizex, CSize factor, CRect& rect)
{
	for (POSITION pos = m_contentsList.GetHeadPosition(); pos != NULL; ) {
		PCONTENTS pContents = (PCONTENTS)m_contentsList.GetNext(pos);
		if (pContents != NULL) {
			CItem* pItem = pContents->pItem;
			if (pItem != NULL) {
				pItem->Copy(pDC, nPage, sizex, factor, rect);
				rect.left = 0;
				rect.top = rect.bottom;
			}
		}
	}
}

CString CLands::GetInfo()
{
	CString str = L"";
	for (POSITION pos = m_contentsList.GetHeadPosition(); pos != NULL; ) {
		PCONTENTS pContents = (PCONTENTS)m_contentsList.GetNext(pos);
		if (pContents != NULL) {
			CItem* pItem = pContents->pItem;
			if (pItem != NULL) {
				str += pItem->GetInfo();
				str += L";\r\n";
			}
		}
	}
	return str;
}


//============================================================================

BEGIN_MESSAGE_MAP(CLands, CWnd)
	ON_WM_SIZE()
	ON_MESSAGE(WM_ITEMDELETE, OnItemDelete)
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

BOOL CLands::OnEraseBkgnd(CDC* pDC)
{
	CRect rect;
	GetClientRect(rect);

	pDC->FillRect(rect, &m_brBkgnd);

	return CWnd::OnEraseBkgnd(pDC);
}

void CLands::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
}

void CLands::OnLButtonUp(UINT nFlags, CPoint point)
{
	CString str;
	str.Format(L"[LAND] x=%d y=%d", point.x, point.y);
	Log(str);

	CTranzxView* pView = (CTranzxView*)m_pParent->GetParent();
	ASSERT(pView);
	pView->SetGrid(point.x);

	CWnd::OnLButtonUp(nFlags, point);
}

afx_msg LRESULT CLands::OnItemDelete(WPARAM wParam, LPARAM lParam)
{
	int id = (int)wParam;
	bool bDeleted = false;
	int offset = 0;

	for (POSITION pos = m_contentsList.GetHeadPosition(); pos != NULL; ) {
		POSITION pos2 = pos;
		PCONTENTS pContents = (PCONTENTS)m_contentsList.GetNext(pos);
		if (pContents != NULL) {
			CItem* pItem = pContents->pItem;
			if (pItem != NULL) {
				if (bDeleted) {
					pContents->rect.OffsetRect(0, - offset);
					pItem->SetWindowPos(NULL, pContents->rect.left, pContents->rect.top, 0, 0,
										SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
					pItem->Redraw();
				}
				else if (pItem->GetTitle(id) != NULL) {
					CRect rect = m_rect;
					rect.top = pContents->rect.top;
					offset = pContents->rect.Height();
					delete pItem;
					delete pContents;
					m_contentsList.RemoveAt(pos2);

					CClientDC dc(this);
					dc.FillRect(rect, &m_brBkgnd);
					bDeleted = true;
				}
			}
		}
	}

	return 0;
}
