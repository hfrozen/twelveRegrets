// AidChkList.cpp
#include "stdafx.h"
#include "AidChkList.h"

#include "Dts.h"

IMPLEMENT_DYNAMIC(CAidChkList, CListCtrl)

CAidChkList::CAidChkList()
{
	m_pParent = NULL;
}

CAidChkList::CAidChkList(CWnd* pParent)
{
	m_pParent = pParent;
}

CAidChkList::~CAidChkList()
{
}

void CAidChkList::InvalidateGrid(int row, int col)
{
	if (row < 0 || col < 0)	return;
	CRect rc;
	if (col == 0)	GetItemRect(row, &rc, LVIR_LABEL);
	else	GetSubItemRect(row, col, LVIR_BOUNDS, rc);
	InvalidateRect(rc);
}

int CAidChkList::HitTestEx(CPoint& pt, int* pCol)
{
	if (pCol != NULL)	*pCol = 0;
	int row = GetTopIndex();
	int bottom = row + GetCountPerPage();

	for ( ; row < bottom; row ++) {
		CRect rect;
		GetItemRect(row, &rect, LVIR_BOUNDS);
		if (rect.PtInRect(pt)) {
			for (int col = 0; col < GetHeaderCtrl()->GetItemCount(); col ++) {
				GetSubItemRect(row, col, LVIR_BOUNDS, rect);
				if (!col)	rect.right = rect.left + GetColumnWidth(col);
				if (rect.PtInRect(pt)) {
					if (pCol != NULL)	*pCol = col;
					return row;
				}
			}
			return -1;
		}
	}
	return -1;
}

BEGIN_MESSAGE_MAP(CAidChkList, CListCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CAidChkList::OnNMCustomdraw)
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()

void CAidChkList::OnNMCustomdraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	//LPNMCUSTOMDRAW pNMCD = reinterpret_cast<LPNMCUSTOMDRAW>(pNMHDR);
	NMLVCUSTOMDRAW* pLVCD = (NMLVCUSTOMDRAW*) pNMHDR;
	*pResult = CDRF_DODEFAULT;

	DWORD dw = GetExtendedStyle();
	switch (pLVCD->nmcd.dwDrawStage) {
	case CDDS_PREPAINT :
		*pResult = CDRF_NOTIFYITEMDRAW;
		return;
	case CDDS_ITEMPREPAINT :
		*pResult = CDRF_NOTIFYSUBITEMDRAW;
		return;
	case CDDS_SUBITEM | CDDS_ITEMPREPAINT :
		if (!(dw & LVS_EX_FULLROWSELECT)) {
			*pResult = 0;
			int row = pLVCD->nmcd.dwItemSpec;
			int col = pLVCD->iSubItem;
			CString str = GetItemText(row, col);

			CRect rect;
			GetSubItemRect(row, col, LVIR_BOUNDS, rect);
			if (!col)	rect.right = rect.left + GetColumnWidth(col);
			rect.DeflateRect(1, 1);

			CDC* pDC = CDC::FromHandle(pLVCD->nmcd.hdc);
			ASSERT(pDC);
			COLORREF oldText = pDC->GetTextColor();
			COLORREF oldBk = pDC->GetBkColor();

			if (!col) {
				pDC->SetTextColor(RGB(0, 0, 0));
				CBrush brush(RGB(255, 255, 255));
				pDC->FillRect(&rect, &brush);
				pDC->DrawText(str, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			}
			else {
				CDC memDC;
				memDC.CreateCompatibleDC(pDC);
				CBitmap bitMap;
				if (str != L"0")	bitMap.LoadBitmapW(IDB_BITMAP_CHECK);
				else	bitMap.LoadBitmapW(IDB_BITMAP_UNCHECK);
				CBitmap* pOldBitmap = (CBitmap*)memDC.SelectObject(&bitMap);
				pDC->BitBlt(rect.left + rect.Width() / 2 - 8, rect.top + rect.Height() / 2 - 8, 16, 16, &memDC, 0, 0, SRCAND);
				memDC.SelectObject(pOldBitmap);
				memDC.DeleteDC();
			}
			pDC->SetTextColor(oldText);
			pDC->SetBkColor(oldBk);
			*pResult = CDRF_SKIPDEFAULT;
			break;
		}
	}
}

void CAidChkList::OnLButtonDown(UINT nFlags, CPoint point)
{
	int col;
	int row = HitTestEx(point, &col);
	if (row > -1 && col > 0) {
		CString str = GetItemText(row, col);
		str = str == L"0" ? L"1" : L"0";
		SetItemText(row, col, str);
	}

	CListCtrl::OnLButtonDown(nFlags, point);
}
