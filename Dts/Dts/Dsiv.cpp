// Dsiv.cpp
#include "stdafx.h"
#include "Dts.h"
#include "Dsiv.h"
#include "afxdialogex.h"

#include "Car.h"
#include "Intro.h"
#include "Panel.h"

#include "Tools.h"

#define	SIV_TIMER			2

#define	ESV_NORMAL			1600
#define	ISV_NORMAL			2
#define	VOV_NORMAL			370
#define	IOV_NORMAL			6
#define	FOV_NORMAL			60
#define	VSV_NORMAL			1600
#define	BSV_NORMAL			100

#define	SIV_TESTENDTIME		(5000 / TIME_DEVPROC)
#define	SIV_TESTCLOSETIME	(8000 / TIME_DEVPROC)

IMPLEMENT_DYNAMIC(CDsiv, CDialogEx)

CDsiv::CDsiv(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDsiv::IDD, pParent)
{
	m_pParent = pParent;
	m_nCID = -1;
	m_nGate = -1;
	m_nPantoID = -1;
	m_bPanto = false;
	m_nTestStep = -1;
	m_nTestTime = 0;
	ZeroMemory(&m_sr[0], SIZE_SIVSR);
	ZeroMemory(&m_sr[1], SIZE_SIVSR);
}

CDsiv::~CDsiv()
{
}

void CDsiv::SetTitle(CString strCarName, CString strDevName)
{
	SetCarName(strCarName);
	SetDevName(strDevName);
}

void CDsiv::SetDevName(CString strDevName)
{
	((CButton*)GetDlgItem(IDC_BUTTON_DEV0))->SetWindowTextW(strDevName);
}

void CDsiv::Ready(bool bCmd)
{
	for (UINT id = IDC_DEVCTRL_BEGIN; id < IDC_LAST_CONTROL; id ++) {
		CWnd* pWnd = (CWnd*)GetDlgItem(id);
		if (pWnd != NULL)	pWnd->EnableWindow(bCmd);
	}
}

void CDsiv::DisableLine()
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
	//pPanel->SendToSocket(m_nCID + 3, 7, 6, 0x12, 0x50 | m_nGate, 0x90, 0xff, SDID_SIV | cDis, 0x32 | 0x80);
	pPanel->SendToSocket(m_nCID + 3, 10, 9, 0x18, 0x50 | m_nGate, 0, SDID_SIV, 0x90, 0xff, 0x1b, 2, cDis);
}

bool CDsiv::GetButton(int nButtonID)
{
	CButton* pButton = (CButton*)GetDlgItem(nButtonID);
	if (pButton != NULL)	return pButton->GetCheck() == BST_CHECKED ? true : false;
	return false;
}

void CDsiv::ButtonCheck(int nButtonID, bool bCheck)
{
	CButton* pButton = (CButton*)GetDlgItem(nButtonID);
	if (pButton != NULL)	pButton->SetCheck(bCheck ? BST_CHECKED : BST_UNCHECKED);
}

void CDsiv::EditWrite(int nEditID, int nNum)
{
	CWnd* pWnd = GetFocus();
	CEdit* pEdit = (CEdit*)GetDlgItem(nEditID);
	if (pEdit != NULL && pEdit->GetSafeHwnd() != pWnd->GetSafeHwnd()) {
		CString str;
		str.Format(L"%d", nNum);
		pEdit->SetWindowTextW(str);
	}
}

void CDsiv::SetupGate(bool bCmd, int nCID, int nGate)
{
	ASSERT(nCID > -1);
	ASSERT(nGate > -1);
	m_nCID = nCID;
	m_nGate = nGate;
	switch (m_nCID) {
	case 0 :	m_nPantoID = 2;	break;
	case 5 :	m_nPantoID = 4;	break;
	case 9 :	m_nPantoID = 8;	break;
	default :	break;
	}

#if !defined(SETUP_SKIP)
	BYTE cDis = 0;
	CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_ARROW0);
	if (pButton != NULL)	cDis = pButton->GetCheck() == BST_CHECKED ? 1 : 0;

	CCar* pCar = (CCar*)m_pParent;
	ASSERT(pCar);
	CPanel* pPanel = pCar->GetPanel();
	ASSERT(pPanel);
	//pPanel->SendToSocket(m_nCID + 3, 7, 6, bCmd ? 0x11 : 0x12, 0x50 | m_nGate, 0x90, 0xff, SDID_SIV | cDis, 0x32 | 0x80);
	pPanel->SendToSocket(m_nCID + 3, 10, 9, bCmd ? 0x17 : 0x18, 0x50 | m_nGate, 0, SDID_SIV, 0x90, 0xff, 0x1b, 2, cDis);
#endif
}

void CDsiv::CollectBuild()
{
	SIVAINFOEX siv;
	ZeroMemory(&siv, sizeof(SIVAINFOEX));

	siv.t.s.cFlow = 0x30;
	CString str;
	for (int n = 1; n < sizeof(SASIVEX); n ++) {
		if ((n >= 6 && n <= 12) || n == 14 || n == 15) {
			CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_DEV0 + n);
			if (pEdit != NULL) {
				pEdit->GetWindowTextW(str);
				WORD w = (WORD)atoi((CT2CA)str);
				if (n == 12) {
					w <<= 4;
					w &= 0xf0;
					siv.t.c[n] |= w;
					w >>= 4;
				}
				else if (n == 15) {
					siv.t.c[n] = (BYTE)(w / 100);
					siv.t.c[n + 1] = (BYTE)(w % 100);
				}
				else {
					w &= 0xff;
					siv.t.c[n] = (BYTE)w;
				}
				str.Format(L"%u", w);
				pEdit->SetWindowTextW(str);
			}
			if (n == 12) {
				BYTE c = siv.t.c[n] & 0xf0;
				for (int m = 0; m < 4; m ++) {
					CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_DEV0 + n * 8 + m);
					if (pButton != NULL && pButton->GetCheck() == BST_CHECKED)	siv.t.c[n] |= (1 << m);
				}
			}
		}
		else if (n < 16) {
			for (int m = 0; m < 8; m ++) {
				CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_DEV0 + n * 8 + m);
				if (pButton != NULL && pButton->GetCheck() == BST_CHECKED)	siv.t.c[n] |= (1 << m);
			}
		}
	}

	siv.cCtrl = 0x13;
	siv.wAddr = 0xff90;

	if (m_nCID > -1 && m_nGate > -1) {
		m_mons[1].SetText((BYTE*)&siv, sizeof(SIVAINFOEX));
		CASTONCAR(m_nCID + 3, m_nGate, (BYTE*)&siv, sizeof(SIVAINFOEX));
	}
}

void CDsiv::RecvSdr(BYTE* p, int length)
{
	if (length != SIZE_SIVSR)	*p = 0xff;
	m_mons[0].SetText(p, length);

	if (*p != 0xff) {
		memcpy(&m_sr[0], p, SIZE_SIVSR);
		if (!m_sr[1].t.s.cmd.b.tdc && m_sr[0].t.s.cmd.b.tdc) {
			CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_DEV12);
			if (pEdit != NULL) {
				pEdit->SetWindowTextW(L"0");
				CollectBuild();
			}
		}
		else if (!m_sr[1].t.s.cmd.b.tsr && m_sr[0].t.s.cmd.b.tsr) {
			m_nTestStep = m_nTestTime = 0;
			for (int n = 0; n < 8; n ++)
				ButtonCheck(IDC_CHECK_DEV8 + n, false);
		}
		memcpy(&m_sr[1], &m_sr[0], SIZE_SIVSR);
	}
}

void CDsiv::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDsiv, CDialogEx)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CDsiv::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	for (int n = 0; n < MONCOM_MAX; n ++) {
		CStatic* pStatic = (CStatic*)GetDlgItem(IDC_STATIC_DEVMON0 + n);
		if (pStatic != NULL) {
			CRect rect;
			pStatic->GetWindowRect(&rect);
			ScreenToClient(&rect);
			pStatic->ShowWindow(SW_HIDE);
			m_mons[n].Create(n == 0 ? SIZE_SIVSR : sizeof(SIVAINFOEX), rect, this);
		}
	}

	((CEdit*)GetDlgItem(IDC_EDIT_DEV6))->SetWindowTextW(L"80");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV7))->SetWindowTextW(L"0");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV8))->SetWindowTextW(L"0");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV9))->SetWindowTextW(L"0");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV10))->SetWindowTextW(L"0");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV11))->SetWindowTextW(L"0");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV12))->SetWindowTextW(L"0");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV14))->SetWindowTextW(L"0");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV15))->SetWindowTextW(L"102");
	((CButton*)GetDlgItem(IDC_CHECK_DEV29))->SetCheck(BST_CHECKED);
	SetTimer(SIV_TIMER, TIME_DEVPROC, NULL);

	return TRUE;
}

BOOL CDsiv::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)) {
		if (pMsg->wParam == VK_RETURN)	CollectBuild();
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

BOOL CDsiv::OnCommand(WPARAM wParam, LPARAM lParam)
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
		else if (id == IDC_BUTTON_DEV14)	SetupGate(true, m_nCID, 5);
		else if (id == IDC_BUTTON_DEV15)	CollectBuild();
		else if (id == IDC_CHECK_ARROW0)	DisableLine();
	}

	return CDialogEx::OnCommand(wParam, lParam);
}

void CDsiv::OnDestroy()
{
	CDialogEx::OnDestroy();
	KillTimer(SIV_TIMER);
}

BOOL CDsiv::OnEraseBkgnd(CDC* pDC)
{
	return CDialogEx::OnEraseBkgnd(pDC);
}

void CDsiv::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == SIV_TIMER) {
		if (m_nTestStep > -1) {
			switch (m_nTestStep) {
			case 0 :
				if (++ m_nTestTime >= SIV_TESTENDTIME) {
					if (GetButton(IDC_CHECK_NG)) {
						ButtonCheck(IDC_CHECK_DEV10, true);
						ButtonCheck(IDC_CHECK_DEV12, true);
					}
					else {
						ButtonCheck(IDC_CHECK_DEV11, true);
						ButtonCheck(IDC_CHECK_DEV13, true);
					}
					ButtonCheck(IDC_CHECK_DEV8, false);
					ButtonCheck(IDC_CHECK_DEV9, true);
					m_nTestStep = 1;
				}
				else {
					ButtonCheck(IDC_CHECK_DEV8, true);
					ButtonCheck(IDC_CHECK_DEV9, false);
				}
				break;
			case 1 :
				if (++ m_nTestTime >= SIV_TESTCLOSETIME) {
					for (int n = 0; n < 8; n ++)
						ButtonCheck(IDC_CHECK_DEV8 + n, false);
					m_nTestStep = -1;
					m_nTestTime = 0;
				}
				break;
			default :	break;
			}
		}

		CCar* pCar = (CCar*)m_pParent;
		ASSERT(pCar);
		CIntro* pIntro = (CIntro*)pCar->GetParent();
		if (m_nPantoID > -1 &&
			(!pIntro->GetCBitD(m_nPantoID, UDIB2IDC(CUDIB_PANPS1)) ||
			!pIntro->GetCBitD(m_nPantoID, UDIB2IDC(CUDIB_PANPS2)))) {
			GETTOOLS(pTools);
			EditWrite(IDC_EDIT_DEV6, pTools->Proportional(pTools->Rand(ESV_NORMAL, 5), 0, 3000, 0, 250));
			EditWrite(IDC_EDIT_DEV7, pTools->Proportional(pTools->Rand(ISV_NORMAL, 5), 0, 500, 0, 250));
			EditWrite(IDC_EDIT_DEV8, pTools->Proportional(pTools->Rand(VOV_NORMAL, 5), 0, 750, 0, 250));
			EditWrite(IDC_EDIT_DEV9, pTools->Proportional(pTools->Rand(IOV_NORMAL, 5), 0, 750, 0, 250));
			EditWrite(IDC_EDIT_DEV10, pTools->Proportional(pTools->Rand(FOV_NORMAL, 5), 0, 125, 0, 250));
			EditWrite(IDC_EDIT_DEV11, pTools->Proportional(pTools->Rand(VSV_NORMAL, 5), 0, 3000, 0, 250));
			EditWrite(IDC_EDIT_DEV14, pTools->Rand(BSV_NORMAL, 3));
			CollectBuild();
			m_bPanto = true;
		}
		else {
			EditWrite(IDC_EDIT_DEV6, 0);
			EditWrite(IDC_EDIT_DEV7, 0);
			EditWrite(IDC_EDIT_DEV8, 0);
			EditWrite(IDC_EDIT_DEV9, 0);
			EditWrite(IDC_EDIT_DEV10, 0);
			EditWrite(IDC_EDIT_DEV11, 0);
			EditWrite(IDC_EDIT_DEV14, 0);
			if (m_bPanto) {
				m_bPanto = false;
				CollectBuild();
			}
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}
