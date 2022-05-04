// AidTexts.cpp
#include "stdafx.h"
#include "Dts.h"
#include "AidTexts.h"

IMPLEMENT_DYNAMIC(CAidTexts, CWnd)

CAidTexts::CAidTexts()
{
	m_nLength = -1;
	for (int n = 0; n < MAX_TEXTS; n ++)
		m_pTexts[n] = NULL;
}

CAidTexts::~CAidTexts()
{
	for (int n = 0; n < MAX_TEXTS; n ++) {
		if (m_pTexts[n] != NULL) {
			delete m_pTexts[n];
			m_pTexts[n] = NULL;
		}
	}
}

void CAidTexts::SetText(BYTE* pBuf, int nLength)
{
	int n;
	for (n = 0; n < nLength; n ++) {
		if (n >= m_nLength)	break;
		CString str;
		if (*pBuf != 0xff)	str.Format(L"%02X", *pBuf);
		else	str = L"FF";
		if (m_pTexts[n] != NULL) {
			CString strTemp;
			m_pTexts[n]->GetWindowTextW(strTemp);
			if (strTemp.Compare(str))	m_pTexts[n]->SetWindowTextW(str);
		}
		++ pBuf;
	}
	for ( ; n < m_nLength; n ++) {
		if (m_pTexts[n] != NULL)	m_pTexts[n]->SetWindowTextW(L"-");
	}
}

BOOL CAidTexts::Create(int nLength, const RECT& rect, CWnd* pParentWnd)
{
	static CString strClassName = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW);
	m_pParent = pParentWnd;

	CDC* pDC = m_pParent->GetDC();
	CSize size = pDC->GetTextExtent(L"88");
	size.cx = (int)(size.cx * 9 / 10);
	CRect rc = CRect(rect.left, rect.top, rect.left + size.cx, rect.top + size.cy);
	CRect rcorg = rc;
	CFont* pFont = m_pParent->GetFont();

	for (int n = 0; n < nLength; n ++) {
		if (m_pTexts[n] != NULL)	delete m_pTexts[n];
		CString str;
		str.Format(L"%02d", n);
		m_pTexts[n] = new CAidStatic();
		ASSERT(m_pTexts[n]->Create(str, WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, rc, m_pParent));
		m_pTexts[n]->SetFont(pFont);
		m_pTexts[n]->EnableWindow();
		m_pTexts[n]->ShowWindow(SW_SHOW);
		if (!(n % 4))	m_pTexts[n]->SetColor(::GetSysColor(COLOR_WINDOWTEXT), RGB(255, 255, 255));
		rc.OffsetRect(rc.Width(), 0);
		if (rc.right > rect.right) {
			rc.OffsetRect(0, rc.Height());
			rc.left = rcorg.left;
			rc.right = rcorg.right;
		}
	}
	m_nLength = nLength;

	return TRUE;
}

BEGIN_MESSAGE_MAP(CAidTexts, CWnd)
END_MESSAGE_MAP()

