// AidStatic.cpp
#include "stdafx.h"
#include "Dts.h"
#include "AidStatic.h"

IMPLEMENT_DYNAMIC(CAidStatic, CStatic)

CAidStatic::CAidStatic()
{
	m_crText = ::GetSysColor(COLOR_WINDOWTEXT);
	m_crBkgnd = ::GetSysColor(COLOR_BTNFACE);
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

BEGIN_MESSAGE_MAP(CAidStatic, CStatic)
	ON_WM_CTLCOLOR_REFLECT()
	ON_WM_ERASEBKGND()
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
