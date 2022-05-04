// Leds.cpp
#include "stdafx.h"
#include "Dts.h"
#include "Leds.h"
#include "afxdialogex.h"

#include "Panel.h"

IMPLEMENT_DYNAMIC(CLeds, CDialogEx)

CLeds::CLeds(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLeds::IDD, pParent)
{
	m_pParent = pParent;
	m_nCID = 0;
	for (int n = 0; n < MAX_LEDS; n ++)
		m_pLed[n] = NULL;
}

CLeds::~CLeds()
{
	for (int n  = 0; n < MAX_LEDS; n ++) {
		KILL(m_pLed[n]);
	}
}

void CLeds::UnitClr()
{
	for (int n = 0; n < MAX_LEDS; n ++) {
		if (m_pLed[n] != NULL && m_pLed[n]->GetState() != LS_DISABLE)
			m_pLed[n]->SetState(LS_OFF);
	}
}

void CLeds::BitCtrl(UINT bid, bool bCtrl)
{
	if (bid < MAX_LEDS) {
		if (m_pLed[bid] != NULL)	m_pLed[bid]->SetState(bCtrl ? LS_ON : LS_OFF);
	}
}

bool CLeds::GetBit(UINT bid)
{
	if (bid < MAX_LEDS)	return m_pLed[bid]->GetState() == LS_ON ? true : false;
	return false;
}

void CLeds::SetName(CString strName)
{
	((CStatic*)GetDlgItem(IDC_STATIC_LNAME))->SetWindowTextW(strName);
}

void CLeds::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CLeds, CDialogEx)
	ON_MESSAGE(WM_CLICKLED, &CLeds::OnClickLed)
END_MESSAGE_MAP()

BOOL CLeds::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	int cy = 1;
	CRect rc;
	CStatic* pStatic = (CStatic*)GetDlgItem(IDC_STATIC_LNAME);
	if (pStatic != NULL) {
		pStatic->GetWindowRect(&rc);
		rc.right = rc.left + SIZE_LED * 2;
		pStatic->SetWindowPos(NULL, 0, 0, rc.Width(), rc.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
		cy += rc.Height();
	}
	rc = CRect(0, cy, SIZE_LED, cy + SIZE_LED);
	CRect rcb = rc;
	CPanel* pPanel = (CPanel*)m_pParent;
	ASSERT(pPanel);
	for (int n = 0; n < MAX_LEDS; n ++) {
		m_pLed[n] = new CAidLed(n, this);
		m_pLed[n]->Create(L"", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE | SS_NOTIFY, rc, this);
		int map = pPanel->GetDioMap(m_nCID, n);
		m_pLed[n]->Set(1, map >= 0 ? LS_OFF : LS_DISABLE);		//(n < 56 ? 1 : 0);
		if ((n % 16) == 15) {
			rc = rcb;
			switch (n) {
			case 15 :	rc.OffsetRect(rc.Width(), 0);	break;
			case 31 :	rc.OffsetRect(0, rc.Height() * 16 + 4);	break;
			case 47 :	rc.OffsetRect(rc.Width(), rc.Height() * 16 + 4);	break;
			default :	rc.OffsetRect(0, rc.Height() * 32 + 8);	break;
			}
		}
		else {
			rc.OffsetRect(0, rc.Height());
			if ((n % 8) == 7)	rc.OffsetRect(0, 2);
		}
	}
	SetWindowPos(NULL, 0, 0, SIZE_LED * 2 + 2, rc.top + 2, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

	return TRUE;
}

BOOL CLeds::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)) {
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

LRESULT CLeds::OnClickLed(WPARAM wParam, LPARAM lParam)
{
	int lid = (int)wParam;
	if (m_pParent != NULL)	m_pParent->SendMessage(WM_CLICKLED, wParam, (LPARAM)m_nCID);

	return 0;
}