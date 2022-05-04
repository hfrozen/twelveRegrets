// Decu.cpp
#include "stdafx.h"
#include "Dts.h"
#include "Decu.h"
#include "afxdialogex.h"

#include "Car.h"
#include "Intro.h"
#include "Panel.h"

#include "Tools.h"

#define	ECU_TIMER			2

#define	ECU_TESTENDTIME		(5000 / TIME_DEVPROC)
#define	ECU_TESTCLOSETIME	(8000 / TIME_DEVPROC)

IMPLEMENT_DYNAMIC(CDecu, CDialogEx)

const double CDecu::m_dbAsp[] = {
	2.29f,	1.94f,	2.08f,	2.26f,	2.08f,
	2.16f,	2.26f,	1.94f,	2.08f,	2.29f
};

const WORD CDecu::m_wLw[] = {
	355,	357,	366,	296,	366,
	332,	297,	357,	366,	355
};

CDecu::CDecu(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDecu::IDD, pParent)
{
	m_pParent = pParent;
	m_nCID = -1;
	m_nGate = -1;
	m_pLampDc = NULL;
	m_nTestStep = -1;
	m_nTestTime = 0;
	ZeroMemory(&m_sr[0], SIZE_ECUSR);
	ZeroMemory(&m_sr[1], SIZE_ECUSR);
}

CDecu::~CDecu()
{
	if (m_pLampDc != NULL)	KILL(m_pLampDc);
}

void CDecu::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

void CDecu::SetTitle(CString strCarName, CString strDevName)
{
	SetCarName(strCarName);
	SetDevName(strDevName);
}

void CDecu::SetDevName(CString strDevName)
{
	((CButton*)GetDlgItem(IDC_BUTTON_DEV0))->SetWindowTextW(strDevName);
}

void CDecu::Ready(bool bCmd)
{
	for (UINT id = IDC_DEVCTRL_BEGIN; id < IDC_LAST_CONTROL; id ++) {
		CWnd* pWnd = (CWnd*)GetDlgItem(id);
		if (pWnd != NULL)	pWnd->EnableWindow(bCmd);
	}
}

void CDecu::DisableLine()
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
	//pPanel->SendToSocket(m_nCID + 3, 7, 6, 0x12, 0x50 | m_nGate, 0x10, 0xff, SDID_ECU | cDis, 0x32 | 0x80);
	pPanel->SendToSocket(m_nCID + 3, 10, 9, 0x18, 0x50 | m_nGate, 0, SDID_ECU, 0x10, 0xff, 0x1b, 2, cDis);
}

bool CDecu::GetButton(int nButtonID)
{
	CButton* pButton = (CButton*)GetDlgItem(nButtonID);
	if (pButton != NULL)	return pButton->GetCheck() == BST_CHECKED ? true : false;
	return false;
}

void CDecu::ButtonCheck(int nButtonID, bool bCheck)
{
	CButton* pButton = (CButton*)GetDlgItem(nButtonID);
	if (pButton != NULL)	pButton->SetCheck(bCheck ? BST_CHECKED : BST_UNCHECKED);
}

void CDecu::EditWrite(int nEditID, int nNum)
{
	CWnd* pWnd = GetFocus();
	CEdit* pEdit = (CEdit*)GetDlgItem(nEditID);
	if (pEdit != NULL && pEdit->GetSafeHwnd() != pWnd->GetSafeHwnd()) {
		CString str;
		str.Format(L"%d", nNum);
		pEdit->SetWindowTextW(str);
	}
}

void CDecu::SetupGate(bool bCmd, int nCID, int nGate)
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
	ASSERT(pPanel);
	//pPanel->SendToSocket(m_nCID + 3, 7, 6, bCmd ? 0x11 : 0x12, 0x50 | m_nGate, 0x10, 0xff, SDID_ECU | cDis, 0x32 | 0x80);
	pPanel->SendToSocket(m_nCID + 3, 10, 9, bCmd ? 0x17 : 0x18, 0x50 | m_nGate, 0, SDID_ECU, 0x10, 0xff, 0x1b, 2, cDis);
#endif
}

void CDecu::CollectBuild()
{
	ECUAINFO ecu;
	ZeroMemory(&ecu, SIZE_ECUSA);

	ecu.t.s.cFlow = 0x30;
	CString str;
	for (int n = 1; n < sizeof(SAECU); n ++) {
		if (n >= 2 && n <= 8) {
			CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_DEV0 + n);
			if (pEdit != NULL) {
				pEdit->GetWindowTextW(str);
				WORD w = (WORD)atoi((CT2CA)str);
				w &= 0xff;
				ecu.t.c[n] = (BYTE)w;
				str.Format(L"%u", w);
				pEdit->SetWindowTextW(str);
			}
		}
		else {
			for (int m = 0; m < 8; m ++) {
				CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_DEV0 + n * 8 + m);
				if (pButton != NULL && pButton->GetCheck() == BST_CHECKED)	ecu.t.c[n] |= (1 << m);
			}
		}
	}

	ecu.cCtrl = 0x13;
	ecu.wAddr = 0xff10;

	if (m_nCID > -1 && m_nGate > -1) {
		m_mons[1].SetText((BYTE*)&ecu, SIZE_ECUSA);
		CASTONCAR(m_nCID + 3, m_nGate, (BYTE*)&ecu, SIZE_ECUSA);
	}
}

void CDecu::RecvSdr(BYTE* p, int length)
{
	if (length != SIZE_ECUSR)	*p = 0xff;
	m_mons[0].SetText(p, length);

	if (*p != 0xff) {
		memcpy(&m_sr[0], p, SIZE_ECUSR);
		m_pLampDc->SetState(m_sr[0].t.s.cmd.b.dc ? LS_OFF : LS_ON);
		if (!m_sr[1].t.s.cmd.b.btr && m_sr[0].t.s.cmd.b.btr) {
			m_nTestStep = 0;
			m_nTestTime = 0;
			ButtonCheck(IDC_CHECK_DEV86, false);
			ButtonCheck(IDC_CHECK_DEV87, false);
		}
		if (!m_sr[1].t.s.cmd.b.astr && m_sr[0].t.s.cmd.b.astr) {
			m_nTestStep = 10;
			m_nTestTime = 0;
			ButtonCheck(IDC_CHECK_DEV84, false);
			ButtonCheck(IDC_CHECK_DEV85, false);
		}
		memcpy(&m_sr[1], &m_sr[0], SIZE_ECUSR);
	}
}


BEGIN_MESSAGE_MAP(CDecu, CDialogEx)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CDecu::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	for (int n = 0; n < MONCOM_MAX; n ++) {
		CStatic* pStatic = (CStatic*)GetDlgItem(IDC_STATIC_DEVMON0 + n);
		if (pStatic != NULL) {
			CRect rect;
			pStatic->GetWindowRect(&rect);
			ScreenToClient(&rect);
			pStatic->ShowWindow(SW_HIDE);
			m_mons[n].Create(n == 0 ? SIZE_ECUSR : SIZE_ECUSA, rect, this);
		}
	}

	CStatic* pStatic = (CStatic*)GetDlgItem(IDC_STATICLP_DEV0);
	if (pStatic != NULL) {
		CRect rc;
		pStatic->GetWindowRect(&rc);
		ScreenToClient(&rc);
		pStatic->EnableWindow(false);
		pStatic->ShowWindow(SW_HIDE);
		m_pLampDc = new CAidLed();
		m_pLampDc->Create(L"", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, rc, this);
		m_pLampDc->EnableWindow(true);
		m_pLampDc->ShowWindow(SW_SHOW);
		m_pLampDc->Set(0, LS_OFF);
	}

	((CEdit*)GetDlgItem(IDC_EDIT_DEV2))->SetWindowTextW(L"0");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV3))->SetWindowTextW(L"0");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV4))->SetWindowTextW(L"0");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV5))->SetWindowTextW(L"0");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV6))->SetWindowTextW(L"0");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV7))->SetWindowTextW(L"0");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV8))->SetWindowTextW(L"0");
	SetTimer(ECU_TIMER, TIME_DEVPROC, NULL);

	return TRUE;
}

BOOL CDecu::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)) {
		if (pMsg->wParam == VK_RETURN)	CollectBuild();
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

BOOL CDecu::OnCommand(WPARAM wParam, LPARAM lParam)
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
		else if (id == IDC_BUTTON_DEV14)	SetupGate(true, m_nCID, 3);
		else if (id == IDC_BUTTON_DEV15 || id == IDC_CHECK_DEV81)	CollectBuild();
		else if (id == IDC_CHECK_ARROW0)	DisableLine();
	}

	return CDialogEx::OnCommand(wParam, lParam);
}

void CDecu::OnDestroy()
{
	CDialogEx::OnDestroy();
	KillTimer(ECU_TIMER);
}

BOOL CDecu::OnEraseBkgnd(CDC* pDC)
{
	return CDialogEx::OnEraseBkgnd(pDC);
}

void CDecu::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == ECU_TIMER) {
		if (m_nTestStep > -1) {
			switch (m_nTestStep) {
			case 0 :
				if (++ m_nTestTime >= ECU_TESTENDTIME) {
					if (!GetButton(IDC_CHECK_NG))	ButtonCheck(IDC_CHECK_DEV87, true);
					ButtonCheck(IDC_CHECK_DEV86, false);
					m_nTestStep = 1;
				}
				else	ButtonCheck(IDC_CHECK_DEV86, true);
				break;
			case 1 :
				if (++ m_nTestTime >= ECU_TESTCLOSETIME) {
					ButtonCheck(IDC_CHECK_DEV86, false);
					ButtonCheck(IDC_CHECK_DEV87, false);
					m_nTestStep = -1;
					m_nTestTime = 0;
				}
				break;
			case 10 :
				if (++ m_nTestTime >= ECU_TESTENDTIME) {
					if (!GetButton(IDC_CHECK_NG))	ButtonCheck(IDC_CHECK_DEV85, true);
					ButtonCheck(IDC_CHECK_DEV84, false);
					m_nTestStep = 11;
				}
				else	ButtonCheck(IDC_CHECK_DEV84, true);
				break;
			case 11 :
				if (++ m_nTestTime >= ECU_TESTCLOSETIME) {
					ButtonCheck(IDC_CHECK_DEV86, false);
					ButtonCheck(IDC_CHECK_DEV87, false);
					m_nTestStep = -1;
					m_nTestTime = 0;
				}
				break;
			default :	break;
			}
		}

		CCar* pCar = (CCar*)m_pParent;
		CIntro* pIntro = (CIntro*)pCar->GetParent();
		int nEffortType = pIntro->GetEffortType();
		int nEffort = pIntro->GetEffort();
		EditWrite(IDC_EDIT_DEV2, nEffortType == EFFORT_BRAKE ? nEffort : 0);
		WORD w = (WORD)(m_dbAsp[m_nCID] * 100.f / 1.01972f);	// bar * 100
		GETTOOLS(pTools);
		EditWrite(IDC_EDIT_DEV3, pTools->Proportional(pTools->Rand(w, 2), 0, 1000, 0, 255));
		//EditWrite(IDC_EDIT_DEV4, pTools->Proportional(pTools->Rand(m_wLw[m_nCID], 2), 0, 550, 0, 255));
		EditWrite(IDC_EDIT_DEV5, nEffortType == EFFORT_BRAKE ? nEffort : 0);
		EditWrite(IDC_EDIT_DEV6, nEffortType == EFFORT_BRAKE ? nEffort : 0);
		//EditWrite(IDC_EDIT_DEV8, pTools->Rand(40, 5));
		CollectBuild();
	}
	CDialogEx::OnTimer(nIDEvent);
}
