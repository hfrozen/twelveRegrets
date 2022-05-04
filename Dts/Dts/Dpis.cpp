// Dpis.cpp
#include "stdafx.h"
#include "Dts.h"
#include "Dpis.h"
#include "afxdialogex.h"

#include "Cabin.h"
#include "Panel.h"

IMPLEMENT_DYNAMIC(CDpis, CDialogEx)

CDpis::CDpis(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDpis::IDD, pParent)
{
	m_pParent = pParent;
	m_nTID = m_nGate = -1;
	m_bManual = false;
	m_cCtrl = 0;
	m_pLamp = NULL;
}

CDpis::~CDpis()
{
	if (m_pLamp != NULL)	KILL(m_pLamp);
}

void CDpis::SetTitle(CString strCarName, CString strDevName)
{
	SetCarName(strCarName);
	SetDevName(strDevName);
}

void CDpis::SetDevName(CString strDevName)
{
	((CButton*)GetDlgItem(IDC_BUTTON_DEV0))->SetWindowTextW(strDevName);
}

void CDpis::Ready(bool bCmd)
{
	for (UINT id = IDC_DEVCTRL_BEGIN; id < IDC_LAST_CONTROL; id ++) {
		CWnd* pWnd = (CWnd*)GetDlgItem(id);
		if (pWnd != NULL)	pWnd->EnableWindow(bCmd);
	}
}

void CDpis::DisableLine()
{
	CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_ARROW0);
	if (pButton == NULL)	return;
	BYTE cDis = pButton->GetCheck() == BST_CHECKED ? 1 : 0;

	ASSERT(m_nTID > -1);
	ASSERT(m_nGate > -1);

	CCabin* pCabin = (CCabin*)m_pParent;
	ASSERT(pCabin);
	CPanel* pPanel = pCabin->GetPanel();
	ASSERT(pPanel);
	//pPanel->SendToSocket(m_nTID == 0 ? 2 : 13, 7, 6, 0x12, 0x50 | m_nGate, 0x60, 2, SDID_PIS | cDis, 0x32 | 0x80);
	pPanel->SendToSocket(m_nTID == 0 ? 2 : 13, 10, 9, 0x18, 0x50 | m_nGate, 0, SDID_PIS, 0x60, 2, 0x1b, 2, cDis);
}

void CDpis::BitCtrl(UINT id, int nState)
{
	CButton* pButton = (CButton*)GetDlgItem(id);
	int check = nState;
	if (pButton != NULL) {
		int check = pButton->GetCheck();
		if (check != nState) {
			pButton->SetCheck(nState);
			CollectBuild();
		}
	}
}

void CDpis::Initial()
{
	m_bManual = false;
	if (m_pLamp != NULL)	m_pLamp->Set(0, LS_OFF);
}

void CDpis::SetupGate(bool bCmd, int nTID, int nGate)
{
	ASSERT(nTID > -1);
	ASSERT(nGate > -1);
	m_nTID = nTID;
	m_nGate = nGate;

#if !defined(SETUP_SKIP)
	BYTE cDis = 0;
	CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_ARROW0);
	if (pButton != NULL)	cDis = pButton->GetCheck() == BST_CHECKED ? 1 : 0;

	CCabin* pCabin = (CCabin*)m_pParent;
	ASSERT(pCabin);
	CPanel* pPanel = pCabin->GetPanel();
	ASSERT(pPanel);
	//pPanel->SendToSocket(nTID == 0 ? 2 : 13, 7, 6, bCmd ? 0x11 : 0x12, 0x50 | nGate, 0x60, 2, SDID_PIS | cDis, 0x32 | 0x80);
	pPanel->SendToSocket(m_nTID == 0 ? 2 : 13, 10, 9, bCmd ? 0x17 : 0x18, 0x50 | m_nGate, 0, SDID_PIS, 0x60, 2, 0x1b, 2, cDis);
#endif
}

void CDpis::CollectBuild()
{
	PISAINFO pis;
	ZeroMemory(&pis, SIZE_PISSA);

	pis.t.s.cFlow = 0x30;
	CString str;
	for (int n = 1; n < sizeof(PISATEXT); n ++) {
		if ((n >= 2 && n <= 5) || n == 28) {
			((CEdit*)GetDlgItem(IDC_EDIT_DEV0 + n))->GetWindowTextW(str);
			if (n == 5) {
				WORD w = (WORD)atoi((CT2CA)str);
				pis.t.c[5] = TOBCD(w / 100);
				pis.t.c[6] = TOBCD(w % 100);
			}
			else	pis.t.c[n] = (BYTE)atoi((CT2CA)str);
		}
		else {
			for (int m = 0; m < 8; m ++) {
				CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_DEV0 + n * 8 + m);
				if (pButton != NULL && pButton->GetCheck() == BST_CHECKED)	pis.t.c[n] |= (1 << m);
			}
		}
	}
	pis.cCtrl = 0x13;
	pis.wAddr = 0x260;

	if (m_nTID > -1 && m_nGate > -1) {
		m_mons[1].SetText((BYTE*)&pis, SIZE_PISSA);
		CASTONCABIN(m_nTID == 0 ? 2 : 13, m_nGate, (BYTE*)&pis, SIZE_PISSA);
	}
}

void CDpis::RecvSdr(BYTE* p, int length)
{
	if (length != SIZE_PISSR)	*p = 0xff;
	else {
		PPISRINFO pPisr = (PPISRINFO)p;
		if (m_cCtrl != pPisr->t.s.ais.cCode) {
			m_cCtrl = pPisr->t.s.ais.cCode;
			CString str;
			str.Format(L"%02X", m_cCtrl);
			m_sttCtrlCode.SetWindowTextW(str);
		}
		/*if (m_bManual != pPisr->t.s.ais.ref.b.manu) {
			m_bManual = pPisr->t.s.ais.ref.b.manu;
			m_pLamp->Set(0, m_bManual ? LS_ON : LS_OFF);
			BitCtrl(IDC_CHECK_DEV59, m_bManual ? BST_CHECKED : BST_UNCHECKED);
			CollectBuild();
		}*/
	}
	m_mons[0].SetText(p, length);
}

void CDpis::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_CTRLCODE, m_sttCtrlCode);
}

BEGIN_MESSAGE_MAP(CDpis, CDialogEx)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CDpis::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	for (int n = 0; n < MONCOM_MAX; n ++) {
		CStatic* pStatic = (CStatic*)GetDlgItem(IDC_STATIC_DEVMON0 + n);
		if (pStatic != NULL) {
			CRect rect;
			pStatic->GetWindowRect(&rect);
			ScreenToClient(&rect);
			pStatic->ShowWindow(SW_HIDE);
			m_mons[n].Create(n == 0 ? SIZE_PISSR : SIZE_PISSA, rect, this);
		}
	}
	for (UINT id = IDC_DEVCTRL_BEGIN; id < IDC_LAST_CONTROL; id ++) {
		CWnd* pWnd = (CWnd*)GetDlgItem(id);
		if (pWnd != NULL)	pWnd->EnableWindow(false);
	}

	CCabin* pCabin = (CCabin*)m_pParent;
	ASSERT(pCabin);
	CString str;
	str.Format(L"%d", pCabin->GetStationID(WHERE_CURRENT));
	((CEdit*)GetDlgItem(IDC_EDIT_DEV2))->SetWindowTextW(str);
	str.Format(L"%d", pCabin->GetStationID(WHERE_NEXT));
	((CEdit*)GetDlgItem(IDC_EDIT_DEV3))->SetWindowTextW(str);
	str.Format(L"%d", pCabin->GetStationID(WHERE_DEST));
	((CEdit*)GetDlgItem(IDC_EDIT_DEV4))->SetWindowTextW(str);
	str.Format(L"%d", pCabin->GetFormationNo());
	((CEdit*)GetDlgItem(IDC_EDIT_DEV5))->SetWindowTextW(str);
	((CEdit*)GetDlgItem(IDC_EDIT_DEV28))->SetWindowTextW(L"12");

	CStatic* pStatic = (CStatic*)GetDlgItem(IDC_STATICLP_DEV0);
	if (pStatic != NULL) {
		CRect rc;
		pStatic->GetWindowRect(&rc);
		ScreenToClient(&rc);
		pStatic->EnableWindow(false);
		pStatic->ShowWindow(SW_HIDE);
		m_pLamp = new CAidLed();
		m_pLamp->Create(L"", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, rc, this);
		m_pLamp->EnableWindow(true);
		m_pLamp->ShowWindow(SW_SHOW);
		m_pLamp->Set(0, LS_OFF);
	}

	return TRUE;
}

BOOL CDpis::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)) {
		if (pMsg->wParam == VK_RETURN)	CollectBuild();
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

BOOL CDpis::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int wNotifyMsg = (int)HIWORD(wParam);
	UINT id = (UINT)LOWORD(wParam);
	if (wNotifyMsg == CBN_SELCHANGE) {
	}
	else if (wNotifyMsg == EN_KILLFOCUS) {
	}
	else {
		if (id == IDC_BUTTON_DEV0 && IsWindowVisible()) {
			EnableWindow(false);
			ShowWindow(SW_HIDE);
		}
		else if (id == IDC_BUTTON_DEV14)	SetupGate(true, m_nTID, 5);
		//else if (id == IDC_BUTTON_DEV15)	CollectBuild();
		else if (id == IDC_CHECK_ARROW0)	DisableLine();
		CollectBuild();
	}

	return CDialogEx::OnCommand(wParam, lParam);
}

void CDpis::OnDestroy()
{
	CDialogEx::OnDestroy();
}

BOOL CDpis::OnEraseBkgnd(CDC* pDC)
{
	return CDialogEx::OnEraseBkgnd(pDC);
}

void CDpis::OnTimer(UINT_PTR nIDEvent)
{
	CDialogEx::OnTimer(nIDEvent);
}
