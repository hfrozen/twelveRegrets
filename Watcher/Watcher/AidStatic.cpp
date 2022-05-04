// AidStatic.cpp
#include "stdafx.h"
#include "Watcher.h"
#include "AidStatic.h"

CAidStatic::CAidStatic()
{
	m_pParent = NULL;
	m_nID = -1;
	m_bInv = false;
	m_crText = COLOR_TEXT;
	m_crBkgnd = COLOR_BKGND;
	m_brBkgnd.CreateSolidBrush(m_crBkgnd);
}

CAidStatic::CAidStatic(CWnd* pParent)
{
	m_pParent = pParent;
	m_nID = -1;
	m_bInv = false;
	m_crText = COLOR_TEXT;
	m_crBkgnd = COLOR_BKGND;
	m_brBkgnd.CreateSolidBrush(m_crBkgnd);
}

CAidStatic::~CAidStatic()
{
	m_brBkgnd.DeleteObject();
}

void CAidStatic::SetColor(COLORREF crText, COLORREF crBkgnd)
{
	m_crText = crText;
	m_crBkgnd = crBkgnd;
	m_brBkgnd.DeleteObject();
	m_brBkgnd.CreateSolidBrush(m_crBkgnd);
	Invalidate();
}

void CAidStatic::SetTextColor(COLORREF crText)
{
	SetColor(crText, m_crBkgnd);
}

void CAidStatic::SetBkColor(COLORREF crBkgnd)
{
	SetColor(m_crText, crBkgnd);
}

BEGIN_MESSAGE_MAP(CAidStatic, CStatic)
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_ERASEBKGND()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

HBRUSH CAidStatic::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	pDC->SetTextColor(m_crText);
	pDC->SetBkColor(m_crBkgnd);
	return (HBRUSH)m_brBkgnd;
}

BOOL CAidStatic::OnEraseBkgnd(CDC* pDC) 
{
	return CStatic::OnEraseBkgnd(pDC);
}

void CAidStatic::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	if (m_nID >= 0 && m_pParent != NULL)	m_pParent->SendMessage(WM_LBTNDBCLK, (WPARAM)m_nID);

	CStatic::OnLButtonDblClk(nFlags, point);
}

void CAidStatic::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_nID >= 0) {
		m_bInv = m_bInv ? false : true;
		if (m_bInv)	SetColor(COLOR_INVTEXT, COLOR_INVBKGND);
		else	SetColor(COLOR_TEXT, COLOR_BKGND);
	}
	CStatic::OnLButtonUp(nFlags, point);
}
