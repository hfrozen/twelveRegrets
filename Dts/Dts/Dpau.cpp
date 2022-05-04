// CDpa.cpp
#include "stdafx.h"
#include "Dts.h"
#include "Dpau.h"
#include "afxdialogex.h"

#include "Cabin.h"
#include "Panel.h"

IMPLEMENT_DYNAMIC(CDpau, CDialogEx)

CDpau::CDpau(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDpau::IDD, pParent)
{
	m_pParent = pParent;
	m_nTID = m_nGate = -1;
	m_bManual = false;
	m_cCtrl = 0;
	m_pLamp = NULL;
}

CDpau::~CDpau()
{
	if (m_pLamp != NULL)	KILL(m_pLamp);
}

void CDpau::SetTitle(CString strCarName, CString strDevName)
{
	SetCarName(strCarName);
	SetDevName(strDevName);
}

void CDpau::SetDevName(CString strDevName)
{
	((CButton*)GetDlgItem(IDC_BUTTON_DEV0))->SetWindowTextW(strDevName);
}

void CDpau::Ready(bool bCmd)
{
	for (UINT id = IDC_DEVCTRL_BEGIN; id < IDC_LAST_CONTROL; id ++) {
		CWnd* pWnd = (CWnd*)GetDlgItem(id);
		if (pWnd != NULL)	pWnd->EnableWindow(bCmd);
	}
}

void CDpau::DisableLine()
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
	//pPanel->SendToSocket(m_nTID == 0 ? 2 : 13, 7, 6, 0x12, 0x50 | m_nGate, 0x60, 1, SDID_PAU | cDis, 0x32 | 0x80);
	pPanel->SendToSocket(m_nTID == 0 ? 2 : 13, 10, 9, 0x18, 0x50 | m_nGate, 0, SDID_PAU, 0x60, 1, 0x1b, 2, cDis);
}

void CDpau::BitCtrl(UINT id, int nState)
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

void CDpau::Initial()
{
	m_bManual = false;
	if (m_pLamp != NULL)	m_pLamp->Set(0, LS_OFF);
}

void CDpau::SetupGate(bool bCmd, int nTID, int nGate)
{
	ASSERT(nTID > -1);
	ASSERT(nGate > -1);
	m_nTID = nTID;
	m_nGate = nGate;

#if !defined(SETUP_SKIP)
	BYTE cDis = 0;
	CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_ARROW0);
	if (pButton != NULL)	cDis = pButton->GetCheck() == BST_CHECKED ? 0x10 : 0;

	CCabin* pCabin = (CCabin*)m_pParent;
	ASSERT(pCabin);
	CPanel* pPanel = pCabin->GetPanel();
	ASSERT(pPanel);
	//pPanel->SendToSocket(nTID == 0 ? 2 : 13, 7, 6, bCmd ? 0x11 : 0x12, 0x50 | nGate, 0x60, 1, SDID_PAU | cDis, 0x32 | 0x80);
	pPanel->SendToSocket(m_nTID == 0 ? 2 : 13, 10, 9, bCmd ? 0x17 : 0x18, 0x50 | m_nGate, 0, SDID_PAU, 0x60, 1, 0x1b, 2, cDis);
#endif
}

void CDpau::CollectBuild()
{
	PAUAINFO pau;
	ZeroMemory(&pau, SIZE_PAUSA);

	pau.t.s.cFlow = 0x30;
	CString str;
	for (int n = 1; n < sizeof(PAUATEXT); n ++) {
		if (n < 5) {
			for (int m = 0; m < 8; m ++) {
				CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_DEV0 + n * 8 + m);
				if (pButton != NULL && pButton->GetCheck() == BST_CHECKED)	pau.t.c[n] |= (1 << m);
			}
		}
		else if (n == 5) {
			((CEdit*)GetDlgItem(IDC_EDIT_DEV5))->GetWindowTextW(str);
			pau.t.c[5] = (BYTE)atoi((CT2CA)str);
		}
	}
	pau.cCtrl = 0x13;
	pau.wAddr = 0x0160;

	if (m_nTID > -1 && m_nGate > -1) {
		m_mons[1].SetText((BYTE*)&pau, SIZE_PAUSA);
		CASTONCABIN(m_nTID == 0 ? 2 : 13, m_nGate, (BYTE*)&pau, SIZE_PAUSA);
	}
}

void CDpau::RecvSdr(BYTE* p, int length)
{
	if (length != SIZE_PAUSR)	*p = 0xff;
	else {
		PPAURINFO pPaur = (PPAURINFO)p;
		if (m_cCtrl != pPaur->t.s.ais.cCode) {
			m_cCtrl = pPaur->t.s.ais.cCode;
			CString str;
			str.Format(L"%02X", m_cCtrl);
			m_sttCtrlCode.SetWindowTextW(str);
		}
		/*if (m_bManual != pPaur->t.s.ais.ref.b.manu) {
			m_bManual = pPaur->t.s.ais.ref.b.manu;
			m_pLamp->Set(0, m_bManual ? LS_ON : LS_OFF);
		}*/
	}
	m_mons[0].SetText(p, length);
}

void CDpau::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_CTRLCODE, m_sttCtrlCode);
}

BEGIN_MESSAGE_MAP(CDpau, CDialogEx)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CDpau::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	for (int n = 0; n < MONCOM_MAX; n ++) {
		CStatic* pStatic = (CStatic*)GetDlgItem(IDC_STATIC_DEVMON0 + n);
		if (pStatic != NULL) {
			CRect rect;
			pStatic->GetWindowRect(&rect);
			ScreenToClient(&rect);
			pStatic->ShowWindow(SW_HIDE);
			m_mons[n].Create(n == 0 ? SIZE_PAUSR : SIZE_PAUSA, rect, this);
		}
	}
	for (UINT id = IDC_DEVCTRL_BEGIN; id < IDC_LAST_CONTROL; id ++) {
		CWnd* pWnd = (CWnd*)GetDlgItem(id);
		if (pWnd != NULL)	pWnd->EnableWindow(false);
	}
	((CEdit*)GetDlgItem(IDC_EDIT_DEV5))->SetWindowTextW(L"12");

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

BOOL CDpau::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)) {
		if (pMsg->wParam == VK_RETURN)	CollectBuild();
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

BOOL CDpau::OnCommand(WPARAM wParam, LPARAM lParam)
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
		else if (id == IDC_BUTTON_DEV14)	SetupGate(true, m_nTID, 3);
		else if (id == IDC_BUTTON_DEV15)	CollectBuild();
		else if (id == IDC_CHECK_ARROW0)	DisableLine();
	}

	return CDialogEx::OnCommand(wParam, lParam);
}

void CDpau::OnDestroy()
{
	CDialogEx::OnDestroy();
}

BOOL CDpau::OnEraseBkgnd(CDC* pDC)
{
	return CDialogEx::OnEraseBkgnd(pDC);
}

void CDpau::OnTimer(UINT_PTR nIDEvent)
{
	CDialogEx::OnTimer(nIDEvent);
}
