// Dv3f.cpp
#include "stdafx.h"
#include "Dts.h"
#include "Dv3f.h"
#include "afxdialogex.h"

#include "Car.h"
#include "Intro.h"
#include "Panel.h"

#include "Tools.h"

#define	V3F_TIMER			2

#define	FCV_NORMAL			1600

#define	V3F_TESTENDTIME		(5000 / TIME_DEVPROC)
#define	V3F_TESTCLOSETIME	(8000 / TIME_DEVPROC)

IMPLEMENT_DYNAMIC(CDv3f, CDialogEx)

CDv3f::CDv3f(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDv3f::IDD, pParent)
{
	m_pParent = pParent;
	m_nCID = -1;
	m_nGate = -1;
	m_bRun = false;
	m_nTestStep = -1;
	m_nTestTime = 0;
	ZeroMemory(&m_sr[0], SIZE_V3FSR);
	ZeroMemory(&m_sr[1], SIZE_V3FSR);
}

CDv3f::~CDv3f()
{
}

void CDv3f::SetTitle(CString strCarName, CString strDevName)
{
	SetCarName(strCarName);
	SetDevName(strDevName);
}

void CDv3f::SetDevName(CString strDevName)
{
	((CButton*)GetDlgItem(IDC_BUTTON_DEV0))->SetWindowTextW(strDevName);
}

void CDv3f::Ready(bool bCmd)
{
	for (UINT id = IDC_DEVCTRL_BEGIN; id < IDC_LAST_CONTROL; id ++) {
		CWnd* pWnd = (CWnd*)GetDlgItem(id);
		if (pWnd != NULL)	pWnd->EnableWindow(bCmd);
	}
}

void CDv3f::DisableLine()
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
	//pPanel->SendToSocket(m_nCID + 3, 7, 6, 0x12, 0x50 | m_nGate, 0x20, 0xff, SDID_V3F | cDis, 0x32 | 0x80);
	pPanel->SendToSocket(m_nCID + 3, 10, 9, 0x18, 0x50 | m_nGate, 0, SDID_V3F, 0x20, 0xff, 0x1b, 2, cDis);
}

bool CDv3f::GetButton(int nButtonID)
{
	CButton* pButton = (CButton*)GetDlgItem(nButtonID);
	if (pButton != NULL)	return pButton->GetCheck() == BST_CHECKED ? true : false;
	return false;
}

void CDv3f::ButtonCheck(int nButtonID, bool bCheck)
{
	CButton* pButton = (CButton*)GetDlgItem(nButtonID);
	if (pButton != NULL)	pButton->SetCheck(bCheck ? BST_CHECKED : BST_UNCHECKED);
}

void CDv3f::EditWrite(int nEditID, int nNum)
{
	CWnd* pWnd = GetFocus();
	CEdit* pEdit = (CEdit*)GetDlgItem(nEditID);
	if (pEdit != NULL && pEdit->GetSafeHwnd() != pWnd->GetSafeHwnd()) {
		CString str;
		str.Format(L"%d", nNum);
		pEdit->SetWindowTextW(str);
	}
}

void CDv3f::SetupGate(bool bCmd, int nCID, int nGate)
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
	//pPanel->SendToSocket(m_nCID + 3, 7, 6, bCmd ? 0x11 : 0x12, 0x50 | m_nGate, 0x20, 0xff, SDID_V3F | cDis, 0x32 | 0x80);
	pPanel->SendToSocket(m_nCID + 3, 10, 9, bCmd ? 0x17 : 0x18, 0x50 | m_nGate, 0, SDID_V3F, 0x20, 0xff, 0x1b, 2, cDis);
#endif
}

void CDv3f::CollectBuild()
{
	V3FAINFO v3f;
	ZeroMemory(&v3f, sizeof(V3FAINFO));

	v3f.t.s.cFlow = 0x30;
	CString str;
	for (int n = 1; n < sizeof(SAV3F); n ++) {
		if ((n >= 10 && n <= 24) || n == 30 || n == 31) {
			CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_DEV0 + n);
			if (pEdit != NULL) {
				pEdit->GetWindowTextW(str);
				WORD w = (WORD)atoi((CT2CA)str);
				if (n == 30) {
					w <<= 4;
					w &= 0xf0;
					v3f.t.c[n] |= w;
					w >>= 4;
				}
				else if (n == 31) {
					v3f.t.c[n] = (BYTE)(w / 100);
					v3f.t.c[n + 1] = (BYTE)(w % 100);
				}
				else if (n >= 13 && n <= 16) {
					w &= 0xff;
					v3f.t.c[n] = (BYTE)w;
				}
				else if (n >= 17 && n <= 23) {
					short v = (short)atoi((CT2CA)str);
					v3f.t.c[n] = HIBYTE(v);
					v3f.t.c[n + 1] = LOBYTE(v);
				}
				else {
					v3f.t.c[n] = HIBYTE(w);
					v3f.t.c[n + 1] = LOBYTE(w);
				}
				str.Format(L"%u", w);
				pEdit->SetWindowTextW(str);
			}
			if (n == 30) {
				BYTE c = v3f.t.c[n] & 0xf0;
				for (int m = 0; m < 4; m ++) {
					CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_DEV0 + n * 8 + m);
					if (pButton != NULL && pButton->GetCheck() == BST_CHECKED)	v3f.t.c[n] |= (1 << m);
				}
			}
		}
		else {
			for (int m = 0; m < 8; m ++) {
				CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_DEV0 + n * 8 + m);
				if (pButton != NULL && pButton->GetCheck() == BST_CHECKED)	v3f.t.c[n] |= (1 << m);
			}
		}
	}

	v3f.cCtrl = 0x13;
	v3f.wAddr = 0xff20;

	if (m_nCID > -1 && m_nGate > -1) {
		m_mons[1].SetText((BYTE*)&v3f, sizeof(V3FAINFO));
		CASTONCAR(m_nCID + 3, m_nGate, (BYTE*)&v3f, sizeof(V3FAINFO));
	}
}

void CDv3f::RecvSdr(BYTE* p, int length)
{
	if (length != SIZE_V3FSR)	*p = 0xff;
	m_mons[0].SetText(p, length);

	if (*p != 0xff) {
		memcpy(&m_sr[0], p, SIZE_V3FSR);
		if (!m_sr[1].t.s.ref.b.tdc && m_sr[0].t.s.ref.b.tdc) {
			CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_DEV30);
			if (pEdit != NULL) {
				pEdit->SetWindowTextW(L"0");
				CollectBuild();
			}
		}
		else if (!m_sr[1].t.s.cmd.b.tsr && m_sr[0].t.s.cmd.b.tsr) {
			m_nTestStep = m_nTestTime = 0;
			for (int n = 0; n < 4; n ++)
				ButtonCheck(IDC_CHECK_DEV204 + n, false);
		}
		memcpy(&m_sr[1], &m_sr[0], SIZE_V3FSR);
	}
}

void CDv3f::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CDv3f, CDialogEx)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CDv3f::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	for (int n = 0; n < MONCOM_MAX; n ++) {
		CStatic* pStatic = (CStatic*)GetDlgItem(IDC_STATIC_DEVMON0 + n);
		if (pStatic != NULL) {
			CRect rect;
			pStatic->GetWindowRect(&rect);
			ScreenToClient(&rect);
			pStatic->ShowWindow(SW_HIDE);
			m_mons[n].Create(n == 0 ? SIZE_V3FSR : sizeof(V3FAINFO), rect, this);
		}
	}
	((CEdit*)GetDlgItem(IDC_EDIT_DEV11))->SetWindowTextW(L"0");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV13))->SetWindowTextW(L"0");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV14))->SetWindowTextW(L"0");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV15))->SetWindowTextW(L"0");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV16))->SetWindowTextW(L"0");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV17))->SetWindowTextW(L"0");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV19))->SetWindowTextW(L"0");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV21))->SetWindowTextW(L"0");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV23))->SetWindowTextW(L"0");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV30))->SetWindowTextW(L"0");
	((CEdit*)GetDlgItem(IDC_EDIT_DEV31))->SetWindowTextW(L"100");
	SetTimer(V3F_TIMER, TIME_DEVPROC, NULL);

	return TRUE;
}

BOOL CDv3f::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)) {
		if (pMsg->wParam == VK_RETURN)	CollectBuild();
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

BOOL CDv3f::OnCommand(WPARAM wParam, LPARAM lParam)
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
		else if (id == IDC_BUTTON_DEV14)	SetupGate(true, m_nCID, 5);
		else if (id == IDC_BUTTON_DEV15)	CollectBuild();
		else if (id == IDC_CHECK_ARROW0)	DisableLine();
	}

	return CDialogEx::OnCommand(wParam, lParam);
}

void CDv3f::OnDestroy()
{
	CDialogEx::OnDestroy();
	KillTimer(V3F_TIMER);
}

BOOL CDv3f::OnEraseBkgnd(CDC* pDC)
{
	return CDialogEx::OnEraseBkgnd(pDC);
}

void CDv3f::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == V3F_TIMER) {
		if (m_nTestStep > -1) {
			switch (m_nTestStep) {
			case 0 :
				if (++ m_nTestTime >= V3F_TESTENDTIME) {
					if (GetButton(IDC_CHECK_NG))	ButtonCheck(IDC_CHECK_DEV204, true);
					else	ButtonCheck(IDC_CHECK_DEV205, true);
					ButtonCheck(IDC_CHECK_DEV206, false);
					ButtonCheck(IDC_CHECK_DEV207, true);
					m_nTestStep = 1;
				}
				else {
					ButtonCheck(IDC_CHECK_DEV206, true);
					ButtonCheck(IDC_CHECK_DEV207, false);
				}
				break;
			case 1 :
				if (++ m_nTestTime >= V3F_TESTCLOSETIME) {
					for (int n = 0; n < 4; n ++)
						ButtonCheck(IDC_CHECK_DEV204 + n, false);
					ButtonCheck(IDC_CHECK_DEV37, true);		// lb2
					ButtonCheck(IDC_CHECK_DEV38, true);		// lb1
					ButtonCheck(IDC_CHECK_DEV39, true);		// hb
					m_nTestStep = -1;
					m_nTestTime = 0;
				}
				break;
			default :	break;
			}
		}

		CCar* pCar = (CCar*)m_pParent;
		CIntro* pIntro = (CIntro*)pCar->GetParent();
		if (pIntro->GetBowTBitD(UDIB2IDC(TUDIB_POWERING)) || pIntro->GetBowTBitD(UDIB2IDC(TUDIB_BRAKING)))	m_bRun = true;
		GETTOOLS(pTools);
		EditWrite(IDC_EDIT_DEV14, pTools->Rand(31, 5));						// LV
		int nEffortType = pIntro->GetEffortType();
		int nEffort = pIntro->GetEffort();
		int v = pTools->Rand(nEffort * 9, 5);								// 100% -> 900A
		EditWrite(IDC_EDIT_DEV15, nEffort);									// TBE
		EditWrite(IDC_EDIT_DEV16, nEffortType == EFFORT_BRAKE ? nEffort : 0);	// BED
		if (!pIntro->GetCBitD(2, UDIB2IDC(CUDIB_PANPS1)) || !pIntro->GetCBitD(2, UDIB2IDC(CUDIB_PANPS2)) ||
			!pIntro->GetCBitD(4, UDIB2IDC(CUDIB_PANPS1)) || !pIntro->GetCBitD(4, UDIB2IDC(CUDIB_PANPS2)) ||
			!pIntro->GetCBitD(8, UDIB2IDC(CUDIB_PANPS1)) || !pIntro->GetCBitD(8, UDIB2IDC(CUDIB_PANPS2))) {
			EditWrite(IDC_EDIT_DEV13, pTools->Proportional(v, 0, 2000, 0, 255));							// IQ
			v = pTools->Rand(nEffort * 8800 / 100, 5);
			if (nEffortType == EFFORT_BRAKE)	v = -v;
			EditWrite(IDC_EDIT_DEV17, v);																	// APBR
			EditWrite(IDC_EDIT_DEV19, pTools->Rand(FCV_NORMAL, 5) * 10);									// ES
			if (nEffortType == EFFORT_POWER)	EditWrite(IDC_EDIT_DEV21, 3 * nEffort / 10);					// IDC
			else if (nEffortType == EFFORT_BRAKE)	EditWrite(IDC_EDIT_DEV21, -(pIntro->GetSpeed() / 10000));
			EditWrite(IDC_EDIT_DEV23, pTools->Rand(FCV_NORMAL, 5) * 10);						// FC
		}
		else {
			EditWrite(IDC_EDIT_DEV13, 0);					// IQ
			EditWrite(IDC_EDIT_DEV17, 0);					// APBR
			EditWrite(IDC_EDIT_DEV19, 0);					// ES
			EditWrite(IDC_EDIT_DEV21, 0);					// IDC
			EditWrite(IDC_EDIT_DEV23, 0);					// FC
		}
		CollectBuild();
	}
	CDialogEx::OnTimer(nIDEvent);
}
