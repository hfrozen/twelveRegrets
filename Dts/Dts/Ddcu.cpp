// Ddcu.cpp
#include "stdafx.h"
#include "Dts.h"
#include "Ddcu.h"
#include "afxdialogex.h"

#include "Car.h"
#include "Panel.h"

IMPLEMENT_DYNAMIC(CDdcu, CDialogEx)

CDdcu::CDdcu(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDdcu::IDD, pParent)
{
	m_pParent = pParent;
	m_nCID = -1;
	m_nGate = -1;
	m_bSide = false;
}

CDdcu::~CDdcu()
{
}

void CDdcu::SetTitle(CString strCarName, CString strDevName)
{
	SetCarName(strCarName);
	SetDevName(strDevName);
}

void CDdcu::SetDevName(CString strDevName)
{
	((CButton*)GetDlgItem(IDC_BUTTON_DEV0))->SetWindowTextW(strDevName);
}

void CDdcu::Ready(bool bCmd)
{
	for (UINT id = IDC_DEVCTRL_BEGIN; id < IDC_LAST_CONTROL; id ++) {
		CWnd* pWnd = (CWnd*)GetDlgItem(id);
		if (pWnd != NULL)	pWnd->EnableWindow(bCmd);
	}
}

void CDdcu::SetFullOpen(bool bOpen)
{
	CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_DEV18);
	if (pButton != NULL) {
		pButton->SetCheck(bOpen ? BST_CHECKED : BST_UNCHECKED);
		CollectBuild();
	}
}

void CDdcu::DisableLine()
{
	BYTE cDis = 0;
	for (int n = 0; n < 4; n ++) {
		CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_ARROW0 + n);
		if (pButton == NULL)	continue;
		if (pButton->GetCheck() == BST_CHECKED)	cDis |= (1 << n);
	}
	ASSERT(m_nCID > -1);
	ASSERT(m_nGate > -1);

	CCar* pCar = (CCar*)m_pParent;
	ASSERT(pCar);
	CPanel* pPanel = pCar->GetPanel();
	ASSERT(pPanel);
	//pPanel->SendToSocket(m_nCID + 3, 7, 6, 0x12, 0x50 | m_nGate, 0x70, m_bSide ? 0x11 : 1, (m_bSide ? SDID_DCUR : SDID_DCUL) | cDis, 0x32 | 0x80);
	pPanel->SendToSocket(m_nCID + 3, 10, 9, 0x18, 0x50 | m_nGate, 0,
							m_bSide ? SDID_DCUR : SDID_DCUL, 0x70, m_bSide ? 0x11 : 1, 0x19, 2, cDis);
}

void CDdcu::SetupGate(bool bCmd, int nCID, int nGate, bool bSide)
{
	ASSERT(nCID > -1);
	ASSERT(nGate > -1);
	m_nCID = nCID;
	m_nGate = nGate;
	m_bSide = bSide;

#if !defined(SETUP_SKIP)
	BYTE cDis = 0;
	for (int n = 0; n < 4; n ++) {
		CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_ARROW0 + n);
		if (pButton == NULL)	continue;
		if (pButton->GetCheck() == BST_CHECKED)	cDis |= (1 << n);
	}

	CCar* pCar = (CCar*)m_pParent;
	ASSERT(pCar);
	CPanel* pPanel = pCar->GetPanel();
	ASSERT(pPanel);
	//pPanel->SendToSocket(m_nCID + 3, 7, 6, bCmd ? 0x11 : 0x12, 0x50 | m_nGate, 0x70, m_bSide ? 0x11 : 1, (m_bSide ? SDID_DCUR : SDID_DCUL) | cDis, 0x32 | 0x80);
	pPanel->SendToSocket(m_nCID + 3, 10, 9, bCmd ? 0x17 : 0x18, 0x50 | m_nGate, 0,
							m_bSide ? SDID_DCUR : SDID_DCUL, 0x70, m_bSide ? 0x11 : 1, 0x19, 2, cDis);
#endif

	if (!m_bSide) {
		CButton* pButton = (CButton*)GetDlgItem(IDC_BUTTON_DEV14);
		if (pButton != NULL)	pButton->ShowWindow(SW_HIDE);
	}
}

void CDdcu::CollectBuild()
{
	DCUAINFO dcu;
	ZeroMemory(&dcu, SIZE_DCUSA);

	dcu.t.s.cFlow = 0x30;
	CString str;
	for (int n = 1; n < sizeof(SADCU); n ++) {
		if (n == 5){
			CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_DEV0 + n);
			if (pEdit != NULL) {
				pEdit->GetWindowTextW(str);
				BYTE c = (BYTE)atoi((CT2CA)str);
				dcu.t.c[n] = c;
				str.Format(L"%u", c);
				pEdit->SetWindowTextW(str);
			}
		}
		else {
			for (int m = 0; m < 8; m ++) {
				CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_DEV0 + n * 8 + m);
				if (pButton != NULL && pButton->GetCheck() == BST_CHECKED)	dcu.t.c[n] |= (1 << m);
			}
		}
	}

	dcu.cCtrl = 0x13;
	dcu.wAddr = 0x0070;

	if (m_nCID > -1 && m_nGate > -1) {
		m_mons[1].SetText((BYTE*)&dcu, SIZE_DCUSA);
		CASTONCAR(m_nCID + 3, m_nGate, (BYTE*)&dcu, SIZE_DCUSA);
	}
}

void CDdcu::RecvSdr(BYTE* p, int length)
{
	if (length != SIZE_DCUSR)	*p = 0xff;
	m_mons[0].SetText(p, length);
}

void CDdcu::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDdcu, CDialogEx)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CDdcu::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	for (int n = 0; n < MONCOM_MAX; n ++) {
		CStatic* pStatic = (CStatic*)GetDlgItem(IDC_STATIC_DEVMON0 + n);
		if (pStatic != NULL) {
			CRect rect;
			pStatic->GetWindowRect(&rect);
			ScreenToClient(&rect);
			pStatic->ShowWindow(SW_HIDE);
			m_mons[n].Create(n == 0 ? SIZE_DCUSR : SIZE_DCUSA, rect, this);
		}
	}

	((CEdit*)GetDlgItem(IDC_EDIT_DEV5))->SetWindowTextW(L"12");

	return TRUE;
}

BOOL CDdcu::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)) {
		if (pMsg->wParam == VK_RETURN)	CollectBuild();
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

BOOL CDdcu::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int wNotifyMsg = (int)HIWORD(wParam);
	UINT id = (UINT)LOWORD(wParam);
	if (wNotifyMsg == CBN_SELCHANGE) {
	}
	else if (wNotifyMsg == EN_KILLFOCUS) {
	}
	else {
		if (id == IDC_BUTTON_DEV0) {
			EnableWindow(false);
			ShowWindow(SW_HIDE);
		}
		else if (id == IDC_BUTTON_DEV14)	SetupGate(true, m_nCID, m_bSide ? 7 : 6, m_bSide);
		else if (id == IDC_BUTTON_DEV15)	CollectBuild();
		else if (id >= IDC_CHECK_ARROW0 && id <= IDC_CHECK_ARROW3)	DisableLine();
	}

	return CDialogEx::OnCommand(wParam, lParam);
}

void CDdcu::OnDestroy()
{
	CDialogEx::OnDestroy();
}

BOOL CDdcu::OnEraseBkgnd(CDC* pDC)
{
	return CDialogEx::OnEraseBkgnd(pDC);
}

void CDdcu::OnTimer(UINT_PTR nIDEvent)
{
	CDialogEx::OnTimer(nIDEvent);
}
