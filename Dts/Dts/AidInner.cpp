// AidInner.cpp
#include "stdafx.h"
#include "AidInner.h"

IMPLEMENT_DYNAMIC(CAidInner, CEdit)

CAidInner::CAidInner()
{
}

CAidInner::CAidInner(int nItem, int nSubItem, CString strInit, COLORREF crText, COLORREF crBkgnd)
{
	m_nItem = nItem;
	m_nSubItem = nSubItem;
	m_strInit = strInit;
	m_crText = crText;
	m_crBkgnd = crBkgnd;
	m_brBkgnd.CreateSolidBrush(m_crBkgnd);
	m_bEsc = false;
}

CAidInner::~CAidInner()
{
	m_brBkgnd.DeleteObject();
}

BEGIN_MESSAGE_MAP(CAidInner, CEdit)
	ON_WM_CREATE()
	ON_WM_CHAR()
	ON_WM_KILLFOCUS()
	ON_WM_NCDESTROY()
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()

BOOL CAidInner::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN) {
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_DELETE ||
			pMsg->wParam == VK_ESCAPE || GetKeyState(VK_CONTROL)) {
			if (pMsg->wParam == VK_RETURN) {
				CWnd* pWnd = GetParent();
				if (pWnd)	pWnd->SendMessage(WM_NEXTITEM);
			}
			::TranslateMessage(pMsg);
			::DispatchMessage(pMsg);
			return TRUE;
		}
	}

	return CEdit::PreTranslateMessage(pMsg);
}

int CAidInner::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CEdit::OnCreate(lpCreateStruct) == -1)
		return -1;

	CFont* pFont = GetParent()->GetFont();
	SetFont(pFont);
	SetWindowText(m_strInit);
	SetFocus();
	SetSel(0, -1);

	return 0;
}

void CAidInner::OnChar(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	if (nChar == VK_ESCAPE || nChar == VK_RETURN) {
		if (nChar == VK_ESCAPE)	m_bEsc = true;
		GetParent()->SetFocus();
		return;
	}
	CEdit::OnChar(nChar, nRepCnt, nFlags);

	CString str;
	GetWindowText(str);
	CWindowDC dc(this);
	CFont* pFont = GetParent()->GetFont();
	CFont* pOldFont = dc.SelectObject(pFont);
	CSize size = dc.GetTextExtent(str);
	dc.SelectObject(pOldFont);
	size.cx += 5;

	CRect rca, rcb;
	GetClientRect(&rca);
	GetParent()->GetClientRect(&rcb);
	ClientToScreen(&rca);
	GetParent()->ScreenToClient(&rca);

	if (size.cx > rca.Width()) {
		if ((size.cx + rca.left) < rcb.right)
			rca.right = rca.left + size.cx;
		else	rca.right = rcb.right;
		MoveWindow(&rca);
	}
}

void CAidInner::OnKillFocus(CWnd* pNewWnd)
{
	CEdit::OnKillFocus(pNewWnd);

	CString str;
	GetWindowText(str);

	LV_DISPINFO di;
	di.hdr.hwndFrom = GetParent()->m_hWnd;
	di.hdr.idFrom = GetDlgCtrlID();
	di.hdr.code = LVN_ENDLABELEDIT;
	di.item.mask = LVIF_TEXT;
	di.item.iItem = m_nItem;
	di.item.iSubItem = m_nSubItem;
	di.item.pszText = m_bEsc ? NULL : (LPWSTR)(LPCTSTR)str;
	di.item.cchTextMax = str.GetLength();
	GetParent()->GetParent()->SendMessage(WM_NOTIFY, GetParent()->GetDlgCtrlID(), (LPARAM)&di);
	DestroyWindow();
}

void CAidInner::OnNcDestroy()
{
	CEdit::OnNcDestroy();
	delete this;
}

HBRUSH CAidInner::CtlColor(CDC* pDC, UINT nCtlColor)
{
	pDC->SetTextColor(m_crText);
	pDC->SetBkColor(m_crBkgnd);
	return (HBRUSH)m_brBkgnd;
}

