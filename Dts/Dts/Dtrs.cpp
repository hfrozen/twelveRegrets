// Dtrs.cpp
#include "stdafx.h"
#include "Dts.h"
#include "Dtrs.h"
#include "afxdialogex.h"

#include "Cabin.h"
#include "Panel.h"

IMPLEMENT_DYNAMIC(CDtrs, CDialogEx)

CDtrs::CDtrs(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDtrs::IDD, pParent)
{
	m_pParent = pParent;
	m_nTID = m_nGate = -1;
}

CDtrs::~CDtrs()
{
}

void CDtrs::SetTitle(CString strCarName, CString strDevName)
{
	SetCarName(strCarName);
	SetDevName(strDevName);
}

void CDtrs::SetDevName(CString strDevName)
{
	((CButton*)GetDlgItem(IDC_BUTTON_DEV0))->SetWindowTextW(strDevName);
}

void CDtrs::Ready(bool bCmd)
{
	for (UINT id = IDC_DEVCTRL_BEGIN; id < IDC_LAST_CONTROL; id ++) {
		CWnd* pWnd = (CWnd*)GetDlgItem(id);
		if (pWnd != NULL)	pWnd->EnableWindow(bCmd);
	}
}

void CDtrs::DisableLine()
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
	//pPanel->SendToSocket(m_nTID == 0 ? 2 : 13, 7, 6, 0x12, 0x50 | m_nGate, 0x60, 4, SDID_TRS | cDis, 0x32 | 0x80);
	pPanel->SendToSocket(m_nTID == 0 ? 2 : 13, 10, 9, 0x18, 0x50 | m_nGate, 0, SDID_TRS, 0x60, 4, 0x1b, 2, cDis);
}

void CDtrs::SetupGate(bool bCmd, int nTID, int nGate)
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
	//pPanel->SendToSocket(nTID == 0 ? 2 : 13, 7, 6, bCmd ? 0x11 : 0x12, 0x50 | nGate, 0x60, 4, SDID_TRS | cDis, 0x32 | 0x80);
	pPanel->SendToSocket(m_nTID == 0 ? 2 : 13, 10, 9, bCmd ? 0x17 : 0x18, 0x50 | m_nGate, 0, SDID_TRS, 0x60, 4, 0x1b, 2, cDis);
#endif
}

void CDtrs::CollectBuild()
{
	TRSAINFO trs;
	ZeroMemory(&trs, SIZE_TRSSA);

	trs.t.s.cFlow = 0x30;
	CString str;
	for (int n = 1; n < sizeof(TRSATEXT); n ++) {
		if (n == 1 || n == 4 || n == 5) {
			((CEdit*)GetDlgItem(IDC_EDIT_DEV0 + n))->GetWindowTextW(str);
			if (n == 1) {
				WORD w = (WORD)atoi((CT2CA)str);
				trs.t.c[1] = TOBCD(w / 100);
				trs.t.c[2] = TOBCD(w % 100);
			}
			else	trs.t.c[n] = (BYTE)atoi((CT2CA)str);
		}
		else {
			for (int m = 0; m < 8; m ++) {
				CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_DEV0 + n * 8 + m);
				if (pButton != NULL && pButton->GetCheck() == BST_CHECKED)	trs.t.c[n] |= (1 << m);
			}
		}
	}
	trs.cCtrl = 0x13;
	trs.wAddr = 0x460;

	if (m_nTID > -1 && m_nGate > -1) {
		m_mons[1].SetText((BYTE*)&trs, SIZE_TRSSA);
		CASTONCABIN(m_nTID == 0 ? 2 : 13, m_nGate, (BYTE*)&trs, SIZE_TRSSA);
	}
}

void CDtrs::RecvSdr(BYTE* p, int length)
{
	if (length != SIZE_TRSSR)	*p = 0xff;
	m_mons[0].SetText(p, length);
}

void CDtrs::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDtrs, CDialogEx)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CDtrs::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	for (int n = 0; n < MONCOM_MAX; n ++) {
		CStatic* pStatic = (CStatic*)GetDlgItem(IDC_STATIC_DEVMON0 + n);
		if (pStatic != NULL) {
			CRect rect;
			pStatic->GetWindowRect(&rect);
			ScreenToClient(&rect);
			pStatic->ShowWindow(SW_HIDE);
			m_mons[n].Create(n == 0 ? SIZE_TRSSR : SIZE_TRSSA, rect, this);
		}
	}
	for (UINT id = IDC_DEVCTRL_BEGIN; id < IDC_LAST_CONTROL; id ++) {
		CWnd* pWnd = (CWnd*)GetDlgItem(id);
		if (pWnd != NULL)	pWnd->EnableWindow(false);
	}

	CCabin* pCabin = (CCabin*)m_pParent;
	ASSERT(pCabin);
	CString str;
	str.Format(L"%d", pCabin->GetFormationNo());
	((CEdit*)GetDlgItem(IDC_EDIT_DEV1))->SetWindowTextW(str);
	((CEdit*)GetDlgItem(IDC_EDIT_DEV4))->SetWindowTextW(L"55");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV5))->SetWindowTextW(L"12");

	return TRUE;
}

BOOL CDtrs::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)) {
		if (pMsg->wParam == VK_RETURN)	CollectBuild();
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

BOOL CDtrs::OnCommand(WPARAM wParam, LPARAM lParam)
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
		else if (id == IDC_BUTTON_DEV14)	SetupGate(true, m_nTID, 7);
		else if (id == IDC_BUTTON_DEV15)	CollectBuild();
		else if (id == IDC_CHECK_ARROW0)	DisableLine();
	}

	return CDialogEx::OnCommand(wParam, lParam);
}

void CDtrs::OnDestroy()
{
	CDialogEx::OnDestroy();
}

BOOL CDtrs::OnEraseBkgnd(CDC* pDC)
{
	return CDialogEx::OnEraseBkgnd(pDC);
}

void CDtrs::OnTimer(UINT_PTR nIDEvent)
{
	CDialogEx::OnTimer(nIDEvent);
}
