// AidStatic.cpp : ���� �����Դϴ�.
//

#include "stdafx.h"
#include "Tcm71.h"
#include "AidStatic.h"


// CAidStatic

IMPLEMENT_DYNAMIC(CAidStatic, CStatic)

CAidStatic::CAidStatic()
{
	m_crText = RGB(16, 16, 16);
	m_crBkgnd = RGB(236, 233, 216);
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
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()



// CAidStatic �޽��� ó�����Դϴ�.



BOOL CAidStatic::OnEraseBkgnd(CDC* pDC)
{
	// TODO: ���⿡ �޽��� ó���� �ڵ带 �߰� ��/�Ǵ� �⺻���� ȣ���մϴ�.

	return CStatic::OnEraseBkgnd(pDC);
}

HBRUSH CAidStatic::CtlColor(CDC* pDC, UINT /*nCtlColor*/)
{
	// TODO:  ���⼭ DC�� Ư���� �����մϴ�.
	pDC->SetTextColor(m_crText);
	pDC->SetBkColor(m_crBkgnd);
	return (HBRUSH)m_brBkgnd;
}
