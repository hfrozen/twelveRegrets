// Dhvac.cpp
#include "stdafx.h"
#include "Dts.h"
#include "Dhvac.h"
#include "afxdialogex.h"

#include "Car.h"
#include "Intro.h"
#include "Panel.h"

#include "Tools.h"

#define	HVAC_TIMER			2

#define	HVAC_TESTENDTIME	(10000 / TIME_DEVPROC)
#define	HVAC_TESTCLOSETIME	(12000 / TIME_DEVPROC)

IMPLEMENT_DYNAMIC(CDhvac, CDialogEx)

CDhvac::CDhvac(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDhvac::IDD, pParent)
{
	m_pParent = pParent;
	m_nCID = -1;
	m_nGate = -1;
	m_nTestStep = -1;
	m_nTestTime = 0;
	m_bCollect = false;
	ZeroMemory(&m_sr[0], SIZE_HVACSR);
	ZeroMemory(&m_sr[1], SIZE_HVACSR);
}

CDhvac::~CDhvac()
{
}

void CDhvac::SetTitle(CString strCarName, CString strDevName)
{
	SetCarName(strCarName);
	SetDevName(strDevName);
}

void CDhvac::SetDevName(CString strDevName)
{
	((CButton*)GetDlgItem(IDC_BUTTON_DEV0))->SetWindowTextW(strDevName);
}

void CDhvac::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

void CDhvac::Ready(bool bCmd)
{
	for (UINT id = IDC_DEVCTRL_BEGIN; id < IDC_LAST_CONTROL; id ++) {
		CWnd* pWnd = (CWnd*)GetDlgItem(id);
		if (pWnd != NULL)	pWnd->EnableWindow(bCmd);
	}
}

void CDhvac::DisableLine()
{
	CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_ARROW0);
	if (pButton == NULL)	return;
	BYTE cDis = pButton->GetCheck() == BST_CHECKED ? 1 : 0;

	ASSERT(m_nCID > -1);
	ASSERT(m_nGate > -1);

	CCar* pCar = (CCar*)m_pParent;
	ASSERT(pCar);
	CPanel* pPanel = pCar->GetPanel();
	ASSERT(pPanel);
	//pPanel->SendToSocket(m_nCID + 3, 7, 6, 0x12, 0x50 | m_nGate, 0x70, 0xff, SDID_HVAC | cDis, 0x32 | 0x80);
	pPanel->SendToSocket(m_nCID + 3, 10, 9, 0x18, 0x50 | m_nGate, 0, SDID_HVAC, 0x70, 0xff, 0x1b, 2, cDis);
}

bool CDhvac::GetButton(int nButtonID)
{
	CButton* pButton = (CButton*)GetDlgItem(nButtonID);
	if (pButton != NULL)	return pButton->GetCheck() == BST_CHECKED ? true : false;
	return false;
}

void CDhvac::ButtonCheck(int nButtonID, bool bCheck)
{
	CButton* pButton = (CButton*)GetDlgItem(nButtonID);
	if (pButton != NULL)	pButton->SetCheck(bCheck ? BST_CHECKED : BST_UNCHECKED);
}

void CDhvac::EditWrite(int nEditID, int nNum)
{
	CWnd* pWnd = GetFocus();
	CEdit* pEdit = (CEdit*)GetDlgItem(nEditID);
	if (pEdit != NULL && pEdit->GetSafeHwnd() != pWnd->GetSafeHwnd()) {
		CString str;
		str.Format(L"%d", nNum);
		pEdit->SetWindowTextW(str);
	}
}

void CDhvac::SetupGate(bool bCmd, int nCID, int nGate)
{
	ASSERT(nCID > -1);
	ASSERT(nGate > -1);
	m_nCID = nCID;
	m_nGate = nGate;

#if !defined(SETUP_SKIP)
	BYTE cDis = 0;
	CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_ARROW0);
	if (pButton != NULL)	cDis = pButton->GetCheck() == BST_CHECKED ? 1 : 0;

	CCar* pCar = (CCar*)m_pParent;
	ASSERT(pCar);
	CPanel* pPanel = pCar->GetPanel();
	//pPanel->SendToSocket(m_nCID + 3, 7, 6, bCmd ? 0x11 : 0x12, 0x50 | m_nGate, 0x70, 0xff, SDID_HVAC | cDis, 0x32 | 0x80);
	pPanel->SendToSocket(m_nCID + 3, 10, 9, bCmd ? 0x17 : 0x18, 0x50 | m_nGate, 0, SDID_HVAC, 0x70, 0xff, 0x1b, 2, cDis);
#endif
}

void CDhvac::CollectBuild()
{
	HVACAINFO hvac;
	ZeroMemory(&hvac, SIZE_HVACSA);

	hvac.t.s.cFlow = 0x30;
	CString str;
	for (int n = 1; n < sizeof(SAHVAC); n ++) {
		if ((n >= 12 && n <= 14) || n == 18){
			CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_DEV0 + n);
			if (pEdit != NULL) {
				pEdit->GetWindowTextW(str);
				WORD w = (WORD)atoi((CT2CA)str);
				if (n != 14) {
					w &= 0xff;
					hvac.t.c[n] = (BYTE)w;
				}
				else {
					hvac.t.c[n] = LOBYTE(w);
					hvac.t.c[n + 1] = HIBYTE(w);
				}
				str.Format(L"%u", w);
				pEdit->SetWindowTextW(str);
			}
		}
		else {
			for (int m = 0; m < 8; m ++) {
				CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_DEV0 + n * 8 + m);
				if (pButton != NULL && pButton->GetCheck() == BST_CHECKED)	hvac.t.c[n] |= (1 << m);
			}
		}
	}

	hvac.cCtrl = 0x13;
	hvac.wAddr = 0xff70;

	if (m_nCID > -1 && m_nGate > -1) {
		m_mons[1].SetText((BYTE*)&hvac, SIZE_HVACSA);
		CASTONCAR(m_nCID + 3, m_nGate, (BYTE*)&hvac, SIZE_HVACSA);
	}
}

void CDhvac::RecvSdr(BYTE* p, int length)
{
	if (length != SIZE_HVACSR)	*p = 0xff;
	m_mons[0].SetText(p, length);

	if (*p != 0xff) {
		memcpy(&m_sr[0], p, SIZE_HVACSR);
		if (m_sr[1].t.s.ctrl.a != m_sr[0].t.s.ctrl.a) {
			if (m_sr[1].t.s.ctrl.b.cmd != m_sr[0].t.s.ctrl.b.cmd) {
				ButtonCheck(IDC_CHECK_DEV9, false);
				ButtonCheck(IDC_CHECK_DEV10, false);
				ButtonCheck(IDC_CHECK_DEV11, false);
				ButtonCheck(IDC_CHECK_DEV28, false);
				ButtonCheck(IDC_CHECK_DEV27, false);
				ButtonCheck(IDC_CHECK_DEV26, false);
				ButtonCheck(IDC_CHECK_DEV25, false);
				ButtonCheck(IDC_CHECK_DEV24, false);
				ButtonCheck(IDC_CHECK_DEV39, false);
				ButtonCheck(IDC_CHECK_DEV38, false);
				ButtonCheck(IDC_CHECK_DEV37, false);
				ButtonCheck(IDC_CHECK_DEV31, false);
				ButtonCheck(IDC_CHECK_DEV30, false);
				ButtonCheck(IDC_CHECK_DEV29, false);
				switch (m_sr[0].t.s.ctrl.b.cmd) {
				case 5 :	// stop
					ButtonCheck(IDC_CHECK_DEV9, true);
					ButtonCheck(IDC_CHECK_DEV28, true);
					break;
				case 4 :	// full cool
					ButtonCheck(IDC_CHECK_DEV38, true);
					ButtonCheck(IDC_CHECK_DEV37, true);
					ButtonCheck(IDC_CHECK_DEV11, true);
					break;
				case 8 :	// half cool
					ButtonCheck(IDC_CHECK_DEV24, true);
					ButtonCheck(IDC_CHECK_DEV39, true);
					ButtonCheck(IDC_CHECK_DEV11, true);
					break;
				case 3 :	// eva.
					ButtonCheck(IDC_CHECK_DEV26, true);
					ButtonCheck(IDC_CHECK_DEV25, true);
					ButtonCheck(IDC_CHECK_DEV11, true);
					break;
				case 0xf :	// 3/3 heat
					ButtonCheck(IDC_CHECK_DEV29, true);
					ButtonCheck(IDC_CHECK_DEV11, true);
					break;
				case 0xb :	// 2/3 heat
					ButtonCheck(IDC_CHECK_DEV30, true);
					ButtonCheck(IDC_CHECK_DEV11, true);
					break;
				case 7 :	// 1/3 heat
					ButtonCheck(IDC_CHECK_DEV31, true);
					ButtonCheck(IDC_CHECK_DEV11, true);
					break;
				case 9 :	// auto
					ButtonCheck(IDC_CHECK_DEV10, true);
					ButtonCheck(IDC_CHECK_DEV27, true);
					break;
				case 1 :	// test
					break;
				default :
					break;
				}
			}
			if (m_sr[1].t.s.ctrl.b.ef != m_sr[0].t.s.ctrl.b.ef)
				ButtonCheck(IDC_CHECK_DEV43, m_sr[0].t.s.ctrl.b.ef);
			if (m_sr[1].t.s.ctrl.b.apdk != m_sr[0].t.s.ctrl.b.apdk)
				ButtonCheck(IDC_CHECK_DEV40, m_sr[0].t.s.ctrl.b.apdk);
			if (m_sr[1].t.s.ctrl.b.elff != m_sr[0].t.s.ctrl.b.elff)
				ButtonCheck(IDC_CHECK_DEV41, m_sr[0].t.s.ctrl.b.elff);
			if (m_sr[1].t.s.ctrl.b.lff != m_sr[0].t.s.ctrl.b.lff)
				ButtonCheck(IDC_CHECK_DEV42, m_sr[0].t.s.ctrl.b.lff);
			//CollectBuild();
			m_bCollect = true;
		}
		if (!m_sr[1].t.s.tst.b.tsr && m_sr[0].t.s.tst.b.tsr) {
			m_nTestStep = m_sr[0].t.s.tst.b.item == 1 ? 0 : 10;
			m_nTestTime = 0;
			for (int n = 0; n < 8; n ++)	ButtonCheck(IDC_CHECK_DEV16 + n, false);
			ButtonCheck(IDC_CHECK_DEV32, false);
			ButtonCheck(IDC_CHECK_DEV33, false);
			//if (m_sr[0].t.s.tst.b.item == 1)	ButtonCheck(IDC_CHECK_DEV16, true);
			//else	ButtonCheck(IDC_CHECK_DEV17, true);
			m_bCollect = true;
		}
		if (!m_sr[1].t.s.tst.b.tsp && m_sr[0].t.s.tst.b.tsp) {
			m_nTestStep = m_nTestTime = -1;
			for (int n = 0; n < 8; n ++)	ButtonCheck(IDC_CHECK_DEV16 + n, false);
			ButtonCheck(IDC_CHECK_DEV32, false);
			ButtonCheck(IDC_CHECK_DEV33, false);
			m_bCollect = true;
		}
		memcpy(&m_sr[1], &m_sr[0], SIZE_HVACSR);
	}
}


BEGIN_MESSAGE_MAP(CDhvac, CDialogEx)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CDhvac::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	for (int n = 0; n < MONCOM_MAX; n ++) {
		CStatic* pStatic = (CStatic*)GetDlgItem(IDC_STATIC_DEVMON0 + n);
		if (pStatic != NULL) {
			CRect rect;
			pStatic->GetWindowRect(&rect);
			ScreenToClient(&rect);
			pStatic->ShowWindow(SW_HIDE);
			m_mons[n].Create(n == 0 ? SIZE_HVACSR : SIZE_HVACSA, rect, this);
		}
	}

	((CEdit*)GetDlgItem(IDC_EDIT_DEV12))->SetWindowTextW(L"18");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV13))->SetWindowTextW(L"20");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV14))->SetWindowTextW(L"2000");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV18))->SetWindowTextW(L"12");
	SetTimer(HVAC_TIMER, TIME_DEVPROC, NULL);

	return TRUE;
}

BOOL CDhvac::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)) {
		if (pMsg->wParam == VK_RETURN)	CollectBuild();
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

BOOL CDhvac::OnCommand(WPARAM wParam, LPARAM lParam)
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
		else if (id == IDC_BUTTON_DEV14)	SetupGate(true, m_nCID, 1);
		else if (id == IDC_BUTTON_DEV15)	CollectBuild();
		else if (id == IDC_CHECK_ARROW0)	DisableLine();
	}

	return CDialogEx::OnCommand(wParam, lParam);
}

void CDhvac::OnDestroy()
{
	CDialogEx::OnDestroy();
}

BOOL CDhvac::OnEraseBkgnd(CDC* pDC)
{
	return CDialogEx::OnEraseBkgnd(pDC);
}

void CDhvac::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == HVAC_TIMER) {
		if (m_nTestStep > -1) {
			switch (m_nTestStep) {
			case 0 :	case 10 :
				if (++ m_nTestTime == HVAC_TESTENDTIME) {
					if (!GetButton(IDC_CHECK_NG)) {
						ButtonCheck(IDC_CHECK_DEV22, false);
						if (m_nTestStep == 0)	ButtonCheck(IDC_CHECK_DEV23, true);
						else {
							ButtonCheck(IDC_CHECK_DEV32, true);
							ButtonCheck(IDC_CHECK_DEV33, true);
						}
					}
					else {
						ButtonCheck(IDC_CHECK_DEV22, true);
						ButtonCheck(IDC_CHECK_DEV23, false);
					}
					ButtonCheck(IDC_CHECK_DEV20, false);
					ButtonCheck(IDC_CHECK_DEV21, true);
					m_bCollect = true;
					++ m_nTestStep;
				}
				else if (m_nTestTime == 1) {
					ButtonCheck(IDC_CHECK_DEV20, true);
					ButtonCheck(IDC_CHECK_DEV21, false);
					m_bCollect = true;
				}
				break;
			case 1 :	case 11 :
				if (++ m_nTestTime >= HVAC_TESTCLOSETIME) {
					for (int n = 0; n < 8; n ++)	ButtonCheck(IDC_CHECK_DEV16 + n, false);
					ButtonCheck(IDC_CHECK_DEV32, false);
					ButtonCheck(IDC_CHECK_DEV33, false);
					m_nTestStep = -1;
					m_nTestTime = 0;
					m_bCollect = true;
				}
				break;
			default :	break;
			}
		}
		if (m_bCollect) {
			m_bCollect = false;
			CollectBuild();
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}
