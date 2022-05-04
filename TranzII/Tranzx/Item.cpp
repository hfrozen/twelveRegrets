// Item.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "Tranzx.h"
#include "Item.h"

#include "TranzxView.h"
#include "TranzxDoc.h"
#include "Hole.h"

// CItem

IMPLEMENT_DYNAMIC(CItem, CWnd)

CItem::CItem(CWnd* pParent)
{
	m_pParent = pParent;
	m_partList.RemoveAll();
	m_ptCur.x = m_ptCur.y = 0;
	m_nGrid = -1;
	m_brBkgnd.DeleteObject();
}

CItem::~CItem()
{
	for (POSITION pos = m_partList.GetHeadPosition(); pos != NULL; ) {
		PPART pPart = (PPART)m_partList.GetNext(pos);
		if (pPart != NULL) {
			if (pPart->pTitle != NULL)	delete pPart->pTitle;
			delete pPart;
			pPart = NULL;
		}
	}

	m_partList.RemoveAll();
}

bool CItem::Create(DWORD dwStyle, const RECT& rect, CWnd* pParent, UINT nID)
{
	static CString strClassName = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW);

	//return CWnd::Create(strClassName, NULL, dwStyle, rect, pParent, nID);
	if (CWnd::Create(strClassName, NULL, dwStyle, rect, pParent, nID)) {
		m_pParent = pParent;
		GetClientRect(m_rect);
		m_rectContext = m_rect;
		m_rectContext.left += ITEM_NAME_X;
		m_rectTitle = m_rect;
		m_rectTitle.right = m_rectTitle.left + ITEM_NAME_X;
		if (m_context.Create(WS_VISIBLE | WS_CHILD, m_rectContext, this))
			m_context.SetMainView(m_pParent->GetParent()->GetParent());
		m_rectContext.OffsetRect(-ITEM_NAME_X, 0);
		return true;
	}
	return false;
}

int CItem::GetNextRight()
{
	int right = m_rectTitle.right;
	for (POSITION pos = m_partList.GetHeadPosition(); pos != NULL; ) {
		PPART pPart = (PPART)m_partList.GetNext(pos);
		if (pPart != NULL) {
			if (pPart->pTitle != NULL) {
				right -= pPart->pTitle->GetWidth();
			}
		}
	}
	return right;
}

void CItem::Log(CString str)
{
	CTranzxView* pView = (CTranzxView*)m_pParent->GetParent()->GetParent();
	pView->Log(str);
}

CAidTitle* CItem::GetTitle(int id)
{
	for (POSITION pos = m_partList.GetHeadPosition(); pos != NULL; ) {
		PPART pPart = (PPART)m_partList.GetNext(pos);
		if (pPart != NULL && pPart->id == id && pPart->pTitle != NULL)
			return pPart->pTitle;
	}
	return NULL;
}

TCHAR CItem::GetFirstTitleType()
{
	for (POSITION pos = m_partList.GetHeadPosition(); pos != NULL; ) {
		PPART pPart = (PPART)m_partList.GetNext(pos);
		if (pPart != NULL && pPart->pTitle != NULL)
			return pPart->pTitle->GetType();
	}
	return 0;
}

bool CItem::Draw(int id)
{
	CTranzxView* pView = (CTranzxView*)m_pParent->GetParent()->GetParent();
	ASSERT(pView);
	CTranzxDoc* pDoc = (CTranzxDoc*)pView->GetDocument();
	ASSERT(pDoc);

	BYTE length = pDoc->GetLBArchiveFlag(id);
	CAidTitle* pTitle = GetTitle(id);
	CString str;
	if (pTitle == NULL) {
		str.Format(L"[ITEM] \"%s\"(%d) 찾을 수 없습니다!", pDoc->GetLBArchiveName(id), id);
		Log(str);
		MessageBox(str, L"Item", MB_ICONWARNING);
		return false;
	}

	TCHAR type = pTitle->GetType();
	int64_t pscMin = pTitle->GetPscMin();
	int64_t pscMax = pTitle->GetPscMax();
	//int64_t absmin = _abs64(pscMin);
	//if (_abs64(pscMax - pscMin) < 100)	pscMax = pscMin + 100;
	//pscMax += (pscMax - pscMin) / 100;
	COLORREF color = pTitle->GetColor();
	int64_t pv = 0x7fffffffffffffff;
	int pn = 0;

	m_context.InitPlot();
	int maxx = (int)pDoc->GetMaxX();
	for (int n = 0; n < maxx; n ++) {
		_variant_t var;
		int y;
		int64_t v;
		int space = pDoc->GetLogbookItem(n, id, var);
		if (space < 0)	break;
		else if (space > 1) {
			n += (space - 1);
			m_context.InitPlot();
			continue;
		}

		//if (!pDoc->GetLogbookItem(n, id, var))	break;
		if (type == 'D') {
			if (var.vt == VT_BOOL)	y = var.bVal != 0 ? 12 : 2;
			else {
				v = (int64_t)var.bVal;
				length = 2;
				type = 'H';
			}
		}
		else {
			if (length > 4)	v = type == 'S' || type == 'K' ? var.llVal : var.ullVal;
			else if (length > 2)	v = type == 'S' || type == 'K' ? var.lVal : var.ulVal;
			else if (length > 1)	v = type == 'S' || type == 'K' ? var.iVal : var.uiVal;
			else	v = var.bVal;
			if (type == 'S' || type == 'U')	y = (int)((v - pscMin) * (int64_t)(m_rectContext.Height() - 2) / (pscMax - pscMin));
			else if ((type == 'K' || type == 'L') && pscMin > 0)	v /= pscMin;
			//if (type == 'S' || type == 'K')	y = (int)((absmin + v) * (int64_t)m_rectContext.Height() / (pscMax - pscMin));
			//else	y = (int)((v - pscMin) * (int64_t)m_rectContext.Height() / (pscMax - pscMin));
		}

		if (type == 'D' || type == 'S' || type == 'U')
			m_context.AppendPoint(m_rectContext.left + n, y, color);
		else {
			if (pv != v) {
				if (type == 'H') {
					if (length > 4)	str.Format(L"0x%08X", v);
					else if (length > 2)	str.Format(L"0x%04X", v);
					else	str.Format(L"0x%02X", v);
				}
				else if (type == 'L') {
					if (length > 4)	str.Format(L"%llu", v);
					else if (length > 2)	str.Format(L"%lu", v);
					else	str.Format(L"%u", v);
				}
				else {
					if (length > 4)	str.Format(L"%lld", v);
					else if (length > 2)	str.Format(L"%ld", v);
					else	str.Format(L"%d", v);
				}
				if (length < 2)
					m_context.AppendText(m_rectContext.left + n, str, color);
				else if (n == 0 || n > pn) {
					m_context.AppendText(m_rectContext.left + n, str, color);
					CSize size = m_context.GetTextWidth(str);
					pn = n + size.cx + 10;
				}
				pv = v;
			}
		}
	}
	return true;
}

bool CItem::AddPart(int id, CString strName, COLORREF color)
{
	CTranzxView* pView = (CTranzxView*)m_pParent->GetParent()->GetParent();
	CTranzxDoc* pDoc = (CTranzxDoc*)pView->GetDocument();
	ASSERT(pDoc);

	CString str;
	TCHAR type = pDoc->GetLBArchiveType(id);
	if (type == 'N') {
		str.Format(L"[ITEM] N 타입은 추가할 수 없습니다! id = %d", id);
		Log(str);
		MessageBox(str, L"Item", MB_ICONWARNING);
		return false;
	}

	int right = GetNextRight();
	if (right < ITEM_NAME_XC) {
		str.Format(L"[ITEM] 더이상 아이템을 추가할 수 없습니다!\r\n현재 아이템을 지우고 다시 추가하십시오. %d", m_partList.GetCount());
		Log(str);
		MessageBox(str, L"Item", MB_ICONWARNING);
		return false;
	}

	CRect rect = m_rectTitle;
	rect.right = right;
	//if (type == 'S' || type == 'U')
	//	rect.left = rect.right - ITEM_NAME_XC;
	if (strName == L"" || strName.IsEmpty())
		strName = pDoc->GetLBArchiveName(id);
	else	strName.Replace('|', '/');
	CAidTitle* pTitle = new CAidTitle(this);
	if (pTitle->Create(WS_CHILD | WS_VISIBLE, rect, this)) {
		if (pTitle->SetItem(id, type, pDoc->GetLBArchiveFlag(id), strName,
				pDoc->GetLBArchivePscMin(id), pDoc->GetLBArchivePscMax(id),
				pDoc->GetLBArchiveLogMin(id), pDoc->GetLBArchiveLogMax(id),
				pDoc->GetLBArchiveUnit(id), color)) {
			PPART pPart = new PART;
			pPart->id = id;
			pPart->pTitle = pTitle;
			pPart->color = color;
			pPart->rect = rect;
			m_partList.AddTail(pPart);
		}
		else {
			delete pTitle;
			str.Format(L"[ITEM] \"%s\"을 설정할 수 없습니다!!!", strName);
			Log(str);
			MessageBox(str, L"Item", MB_ICONWARNING);
			return false;
		}
	}
	else {
		delete pTitle;
		str = L"[ITEM] 아이템 제목 생성할 수 없습니다!!!";
		Log(str);
		MessageBox(str, L"Item", MB_ICONWARNING);
		return false;
	}

	if (m_partList.GetCount() < 2)
		m_context.SetRange(0, m_rectContext.Width(), 0, m_rectContext.Height() - OFFSETY_GRAPH);
	return Draw(id);
}

CString CItem::GetName(int id)
{
	CTranzxView* pView = (CTranzxView*)m_pParent->GetParent()->GetParent();
	CTranzxDoc* pDoc = (CTranzxDoc*)pView->GetDocument();
	ASSERT(pDoc);

	return (CString)pDoc->GetLBArchiveName(id);
}

int64_t CItem::GetPscMin(int id)
{
	CTranzxView* pView = (CTranzxView*)m_pParent->GetParent()->GetParent();
	CTranzxDoc* pDoc = (CTranzxDoc*)pView->GetDocument();
	ASSERT(pDoc);

	return pDoc->GetLBArchivePscMin(id);
}

int64_t CItem::GetPscMax(int id)
{
	CTranzxView* pView = (CTranzxView*)m_pParent->GetParent()->GetParent();
	CTranzxDoc* pDoc = (CTranzxDoc*)pView->GetDocument();
	ASSERT(pDoc);

	return pDoc->GetLBArchivePscMax(id);
}

int64_t CItem::GetLogMin(int id)
{
	CTranzxView* pView = (CTranzxView*)m_pParent->GetParent()->GetParent();
	CTranzxDoc* pDoc = (CTranzxDoc*)pView->GetDocument();
	ASSERT(pDoc);

	return pDoc->GetLBArchiveLogMin(id);
}

int64_t CItem::GetLogMax(int id)
{
	CTranzxView* pView = (CTranzxView*)m_pParent->GetParent()->GetParent();
	CTranzxDoc* pDoc = (CTranzxDoc*)pView->GetDocument();
	ASSERT(pDoc);

	return pDoc->GetLBArchiveLogMax(id);
}

void CItem::Redraw()
{
	m_context.ErasePlot();
	for (POSITION pos = m_partList.GetHeadPosition(); pos != NULL; ) {
		PPART pPart = (PPART)m_partList.GetNext(pos);
		if (pPart != NULL) {
			CAidTitle* pTitle = pPart->pTitle;
			if (pTitle != NULL)	pTitle->Redraw();
			Draw(pPart->id);
		}
	}		
}

void CItem::SetPos(CPoint pt)
{
	m_ptCur = pt;
	//CString str;
	//str.Format(L"[ITEM] Setpos %d %d", pt.x, pt.y);
	//Log(str);
	/*for (POSITION pos = m_partList.GetHeadPosition(); pos != NULL; ) {
		PPART pPart = (PPART)m_partList.GetNext(pos);
		if (pPart != NULL) {
			CAidTitle* pTitle = pPart->pTitle;
			if (pTitle != NULL)	pTitle->SetPos(m_ptCur);
		}
	}*/		
	m_context.SetPos(m_ptCur);
}

void CItem::SetGrid(int grid)
{
	m_context.SetGrid(grid);
	m_nGrid = grid;
	if (m_nGrid > 0) {
		for (POSITION pos = m_partList.GetHeadPosition(); pos != NULL; ) {
			PPART pPart = (PPART)m_partList.GetNext(pos);
			if (pPart != NULL) {
				CTranzxView* pView = (CTranzxView*)m_pParent->GetParent()->GetParent();
				CTranzxDoc* pDoc = (CTranzxDoc*)pView->GetDocument();
				ASSERT(pDoc);

				TCHAR type = pDoc->GetLBArchiveType(pPart->id);
				BYTE leng = pDoc->GetLBArchiveFlag(pPart->id);
				if ((type == 'S' || type == 'U') && pPart->pTitle != NULL) {
					_variant_t var;
					int space = pDoc->GetLogbookItem(m_ptCur.x + m_nGrid, pPart->id, var);
					if (space == 0 || space == 1)
						pPart->pTitle->SetValue(type, leng, var);
				}
			}
		}
	}
}

CSize CItem::GetFrameSize()
{
	CTranzxView* pView = (CTranzxView*)m_pParent->GetParent()->GetParent();
	CTranzxDoc* pDoc = (CTranzxDoc*)pView->GetDocument();
	ASSERT(pDoc);

	CSize fsize(0, 0);
	fsize.cy = m_rectTitle.Height();
	fsize.cx = (int)pDoc->GetMaxX() + ITEM_NAME_X;
	return fsize;
}

bool CItem::PartSelect(int id)
{
	bool bResult = false;
	for (POSITION pos = m_partList.GetHeadPosition(); pos != NULL; ) {
		PPART pPart = (PPART)m_partList.GetNext(pos);
		if (pPart != NULL) {
			CAidTitle* pTitle = pPart->pTitle;
			if (pTitle != NULL) {
				if (pTitle->PartSelect(id))	bResult = true;
			}
		}
	}
	return bResult;
}

bool CItem::Copy(CDC* pDC, int nPage, int sizex, CSize factor, CRect& fRect)
{
	if (nPage < 1)	return false;

	CRect rect;
	bool half = false;
	if (nPage == 1) {
		for (POSITION pos = m_partList.GetTailPosition(); pos != NULL; ) {
			PPART pPart = (PPART)m_partList.GetPrev(pos);
			if (pPart != NULL) {
				CAidTitle* pTitle = pPart->pTitle;
				if (pTitle != NULL) {
					rect.left = fRect.left + pPart->rect.left * factor.cx;
					rect.top = fRect.top + pPart->rect.top * factor.cy;
					pTitle->Copy(pDC, factor, rect);
				}
			}
		}
		half = true;
	}
	POSITION pos = m_partList.GetHeadPosition();
	if (pos != NULL) {
		PPART pPart = (PPART)m_partList.GetNext(pos);
		if (pPart != NULL) {
			rect.left = fRect.left;
			if (half)	rect.left += (pPart->rect.right * factor.cx);
			rect.top = fRect.top + pPart->rect.top * factor.cy;
			m_context.Copy(pDC, nPage, sizex, factor, rect);
		}
	}
	fRect = rect;

	return true;
}

void CItem::SetBound(CRect rect)
{
	m_rect = rect;
	SetWindowPos(NULL, 0, 0, m_rect.Width(), m_rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
	m_rectContext = m_rect;
	m_rectContext.left += ITEM_NAME_X;
	m_rectTitle = m_rect;
	m_rectTitle.right = m_rectTitle.left + ITEM_NAME_X;
	m_rectContext.OffsetRect(-ITEM_NAME_X, 0);
	m_context.SetBound(m_rectContext);
	m_context.SetRange(0, m_rectContext.Width(), 0, m_rectContext.Height() - OFFSETY_GRAPH);
}

CString CItem::GetInfo()
{
	CString str = L"";
	for (POSITION pos = m_partList.GetHeadPosition(); pos != NULL; ) {
		PPART pPart = (PPART)m_partList.GetNext(pos);
		if (pPart != NULL) {
			CAidTitle* pTitle = pPart->pTitle;
			if (pTitle != NULL) {
				str += pTitle->GetInfo();
				str += L" ";
			}
		}
	}
	return str;
}

BEGIN_MESSAGE_MAP(CItem, CWnd)
	ON_WM_ERASEBKGND()
	ON_WM_SIZE()
	ON_MESSAGE(WM_PARTDELETE, OnPartDelete)
END_MESSAGE_MAP()

BOOL CItem::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CRect rect;
	GetClientRect(rect);

	//pDC->FillRect(rect, &m_brBkgnd);

	return CWnd::OnEraseBkgnd(pDC);
}

void CItem::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
}

LRESULT CItem::OnPartDelete(WPARAM wParam, LPARAM lParam)
{
	int id = (int)wParam;

	if (m_partList.GetCount() > 1) {
		CRect prevRect(0, 0, 0, 0);
		BOOL bDeleted = FALSE;
		for (POSITION pos = m_partList.GetHeadPosition(); pos != NULL; ) {
			POSITION pos2 = pos;
			PPART pPart = (PPART)m_partList.GetNext(pos);
			if (pPart != NULL) {
				if (bDeleted) {
					CRect nextRect = pPart->rect;
					pPart->rect = prevRect;
					prevRect = nextRect;
					CAidTitle* pTitle = pPart->pTitle;
					if (pTitle != NULL)
						pTitle->SetWindowPos(NULL, pPart->rect.left, pPart->rect.top, 0, 0,
												SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
				}
				else
				if (pPart->id == id) {
					prevRect = pPart->rect;
					if (pPart->pTitle != NULL)	delete pPart->pTitle;
					delete pPart;
					m_partList.RemoveAt(pos2);
					bDeleted = TRUE;
				}
			}
		}
		CClientDC dc(this);
		CRect rect;
		GetClientRect(rect);
		dc.FillRect(rect, &m_brBkgnd);

		Redraw();
	}
	else {
		CLands* pLands = (CLands*)m_pParent;
		m_pParent->SendMessage(WM_ITEMDELETE, (WPARAM)id);
	}

	return 0;
}
