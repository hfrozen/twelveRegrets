
#include "stdafx.h"
#include "ViewTree.h"

#include "ClassView.h"
#include "TranzxDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewTree

CViewTree::CViewTree()
{
	m_pParent = NULL;
	m_bEnableDrag = false;
	m_bDraggin = false;
	m_hDraggedItem = NULL;
	//m_hDropItem = NULL;
	m_pDraggedImage = NULL;
}

CViewTree::~CViewTree()
{
	if (m_pDraggedImage != NULL) {
		delete m_pDraggedImage;
		m_pDraggedImage = NULL;
	}
}

bool CViewTree::IsEnableDrag(HTREEITEM hItem)
{
	if (!m_bEnableDrag)		return false;
	CClassView* pView = (CClassView*)m_pParent;
	if (pView == NULL)	return false;
	CTranzxDoc* pDoc = (CTranzxDoc*)pView->GetDocument();
	if (pDoc == NULL)	return false;

	if (m_bDraggin)	return false;
	CString str = pView->GetFullNodeName(hItem);
	int id = pView->FindItem(str);
	if (id >= 0) {
		TCHAR type = pDoc->GetLBArchiveType(id);
		if (type == 'D' || type == 'S' || type == 'U')	return true;
	}
	return false;
}

BEGIN_MESSAGE_MAP(CViewTree, CTreeCtrl)
	//ON_NOTIFY_REFLECT(TVN_SELCHANGING, &CViewTree::OnTvnSelchanging)
	ON_NOTIFY_REFLECT(TVN_SELCHANGED, &CViewTree::OnTvnSelchanging)
	ON_NOTIFY_REFLECT(NM_DBLCLK, &CViewTree::OnNMDblclk)
	ON_NOTIFY_REFLECT(TVN_BEGINDRAG, &CViewTree::OnTvnBegindrag)
	ON_WM_MOUSEMOVE()
	ON_WM_LBUTTONUP()
	ON_NOTIFY_REFLECT(NM_RCLICK, &CViewTree::OnNMRClick)
	//ON_WM_CONTEXTMENU()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewTree 메시지 처리기

BOOL CViewTree::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	BOOL bRes = CTreeCtrl::OnNotify(wParam, lParam, pResult);

	NMHDR* pNMHDR = (NMHDR*)lParam;
	ASSERT(pNMHDR != NULL);

	if (pNMHDR && pNMHDR->code == TTN_SHOW && GetToolTips() != NULL)
	{
		GetToolTips()->SetWindowPos(&wndTop, -1, -1, -1, -1, SWP_NOMOVE | SWP_NOACTIVATE | SWP_NOSIZE);
	}

	return bRes;
}

void CViewTree::OnTvnSelchanging(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	HTREEITEM hCurItem = pNMTreeView->itemNew.hItem;
	if (m_pParent != NULL)	m_pParent->SendMessage(WM_SELECT, (WPARAM)hCurItem);

	*pResult = 0;
}


void CViewTree::OnNMDblclk(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	HTREEITEM hCurItem = GetSelectedItem();
	if (m_pParent != NULL)	m_pParent->SendMessage(WM_SELECTED, (WPARAM)hCurItem);

	*pResult = 0;
}

void CViewTree::OnTvnBegindrag(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);

	if (IsEnableDrag(pNMTreeView->itemNew.hItem)) {
		m_hDraggedItem = pNMTreeView->itemNew.hItem;
		//m_hDropItem = NULL;
		m_pDraggedImage = CreateDragImage(m_hDraggedItem);
		if (!m_pDraggedImage)	return;
		m_bDraggin = true;
		m_pDraggedImage->BeginDrag(0, CPoint(-15, -15));
		POINT pt = pNMTreeView->ptDrag;
		ClientToScreen(&pt);
		m_pDraggedImage->DragEnter(NULL, pt);
		SetCapture();
	}

	*pResult = 0;
}

void CViewTree::OnMouseMove(UINT nFlags, CPoint point)
{
	if (m_bDraggin) {
		POINT pt = point;
		ClientToScreen(&pt);
		CImageList::DragMove(pt);
		/*HTREEITEM hItem;
		UINT flags;
		if ((hItem = HitTest(point, &flags)) != NULL) {
			CImageList::DragShowNolock(FALSE);
			SelectDropTarget(hItem);
			m_hDropItem = hItem;
			CImageList::DragShowNolock(TRUE);
		}*/
	}

	CTreeCtrl::OnMouseMove(nFlags, point);
}

void CViewTree::OnLButtonUp(UINT nFlags, CPoint point)
{
	CTreeCtrl::OnLButtonUp(nFlags, point);

	if (m_bDraggin) {
		m_bDraggin = false;
		CImageList::DragLeave(this);
		CImageList::EndDrag();
		ReleaseCapture();

		delete m_pDraggedImage;
		m_pDraggedImage = NULL;
		SelectDropTarget(NULL);
		if (m_pParent != NULL) {
			POINT pt = point;
			ClientToScreen(&pt);
			m_pParent->SendMessage(WM_DROPED, (WPARAM)m_hDraggedItem, (LPARAM)&pt);
		}
	}
}

void CViewTree::OnNMRClick(NMHDR *pNMHDR, LRESULT *pResult)
{
	//LPNMTREEVIEW pNMTreeView = reinterpret_cast<LPNMTREEVIEW>(pNMHDR);
	//UINT mask = pNMTreeView->itemNew.mask;
	//CString str = L"";
	//if (mask & TVIF_CHILDREN)	str += L" CHILDREN";
	//if (mask & TVIF_HANDLE)		str += L" HANDLE";
	//if (mask & TVIF_IMAGE)		str += L" IMAGE";
	//if (mask & TVIF_PARAM)		str += L" PARAM";
	//if (mask & TVIF_SELECTEDIMAGE)	str += L" IMAGE";
	//if (mask & TVIF_STATE)		str += L" STATE";
	//if (mask & TVIF_TEXT)		str += L" TEXT";
	//HTREEITEM hItem = pNMTreeView->itemNew.hItem;
	//CString str = (CString)pNMTreeView->itemNew.pszText;
	//AfxMessageBox(str);
	//if (m_pParent != NULL)	m_pParent->SendMessage(WM_RIGHTCLICK, (WPARAM)hItem);

	TRACE0("View right click\n");
	CPoint pt = (CPoint)GetMessagePos();
	ScreenToClient(&pt);
	UINT uFlags;
	HTREEITEM htItem = HitTest(pt, &uFlags);
	if (htItem != NULL && (uFlags & TVHT_ONITEM) && m_pParent != NULL)
		m_pParent->SendMessage(WM_RIGHTCLICK, (WPARAM)htItem, (LPARAM)GetMessagePos());

	*pResult = 1;
}

//void CViewTree::OnContextMenu(CWnd* pWnd, CPoint point)
//{
//	if (point.x == -1 && point.y == -1)
//		point = (CPoint)GetMessagePos();
//
//	ScreenToClient(&point);
//	UINT uFlags;
//	HTREEITEM htItem = HitTest(point, &uFlags);
//
//	if (htItem == NULL)	return;
//	CString str = htItem.
//}
//