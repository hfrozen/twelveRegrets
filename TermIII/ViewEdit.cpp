// ViewEdit.cpp : implementation file
//

#include "stdafx.h"
#include "Terminal.h"
#include "ViewEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CViewEdit

CViewEdit::CViewEdit()
{
	m_crTextColor = RGB(0, 0, 0);
	m_crBkColor = RGB(255, 255, 255);
	m_brBkgnd.CreateSolidBrush(m_crBkColor);
}

CViewEdit::~CViewEdit()
{
}

void CViewEdit::AddString(CString strNew)
{
	SetSel(-1, -1);
	ReplaceSel(strNew);
}

void CViewEdit::ClearAll()
{
	SetReadOnly(FALSE);
	SetSel(0, -1);
	Clear();
	SetReadOnly(TRUE);
}

void CViewEdit::SetTextColor(COLORREF crColor)
{
	m_crTextColor = crColor;
	Invalidate(FALSE);
}

void CViewEdit::SetBkColor(COLORREF crColor)
{
	m_crBkColor = crColor;
	m_brBkgnd.DeleteObject();
	m_brBkgnd.CreateSolidBrush(m_crBkColor);
	Invalidate();
}

BEGIN_MESSAGE_MAP(CViewEdit, CEdit)
	//{{AFX_MSG_MAP(CViewEdit)
	ON_WM_CTLCOLOR_REFLECT()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CViewEdit message handlers

HBRUSH CViewEdit::CtlColor(CDC* pDC, UINT nCtlColor) 
{
	// TODO: Change any attributes of the DC here
	pDC->SetTextColor(m_crTextColor);
	pDC->SetBkColor(m_crBkColor);
	return (HBRUSH) m_brBkgnd;
}
