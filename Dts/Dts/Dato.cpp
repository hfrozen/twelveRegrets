// Dato.cpp
#include "stdafx.h"
#include "Dts.h"
#include "Dato.h"
#include "afxdialogex.h"

#include "Cabin.h"
#include "Panel.h"

IMPLEMENT_DYNAMIC(CDato, CDialogEx)

PCWSTR CDato::m_strStations[] = {
	L"����",			L"�Ǵ� �Ա�",		L"����",			L"����",
	L"��� ����",		L"���",			L"��õ",			L"���� ���",
	L"���",			L"����",			L"����",			L"����",
	L"����",			L"����",			L"���",			L"���",
	L"������",		L"�����",		L"��õ",			L"�Ÿ�",
	L"�Ŵ��",		L"���� ������",	L"�븲",			L"�ŵ���",
	L"����",			L"������ ��û",	L"���",			L"����",
	L"ȫ�� �Ա�",		L"����",			L"�̴�",			L"����",
	L"������",		L"��û",			L"������ �Ա�",	L"������ 3��",
	L"������ 4��",	L"���빮 ����",	L"�Ŵ�",			L"��սʸ�",
	L"�սʸ�",		L"�Ѿ��",		L"�Ҽ�",			L"����",
	L"�ż���",		L"���",			L"�Ŵ�",			L"���",
	L"����õ",		L"��õ��",		L"������",		L"��ġ��"
};

CDato::CDato(CWnd* pParent /*=NULL*/)
	: CDialogEx(CDato::IDD, pParent)
{
	m_pParent = pParent;
	m_nTID = m_nGate = -1;
}

CDato::~CDato()
{
}

void CDato::SetEffort(bool bFrom)
{
	if (((CButton*)GetDlgItem(IDC_CHECK_DEV113))->GetCheck() == BST_CHECKED) {
		((CEdit*)GetDlgItem(IDC_EDIT_DEV13))->SetWindowTextW(L"0");
		m_sliderEffort.SetPos(255 - 0);
	}
	else {
		CString str;
		if (bFrom) {
			int pos = m_sliderEffort.GetPos();
			if (pos > 255)	pos %= 255;
			pos = 255 - pos;
			str.Format(L"%d", pos);
			((CEdit*)GetDlgItem(IDC_EDIT_DEV13))->SetWindowTextW(str);
		}
		else {
			((CEdit*)GetDlgItem(IDC_EDIT_DEV13))->GetWindowTextW(str);
			int pos = atoi((CT2CA)str);
			if (pos > 255)	pos %= 255;
			str.Format(L"%d", pos);
			((CEdit*)GetDlgItem(IDC_EDIT_DEV13))->SetWindowTextW(str);
			m_sliderEffort.SetPos(255 - pos);
		}
	}
}

void CDato::SetStation(int idc)
{
	CString str;
	((CEdit*)GetDlgItem(idc))->GetWindowTextW(str);
	int si = atoi((CT2CA)str);
	if (si >= _countof(m_strStations))	si %= _countof(m_strStations);
	str.Format(L"%d", si);
	((CEdit*)GetDlgItem(idc))->SetWindowTextW(str);
	((CEdit*)GetDlgItem(idc + 1))->SetWindowTextW(m_strStations[si]);
}

void CDato::SetTitle(CString strCarName, CString strDevName)
{
	SetCarName(strCarName);
	SetDevName(strDevName);
}

void CDato::SetDevName(CString strDevName)
{
	((CButton*)GetDlgItem(IDC_BUTTON_DEV0))->SetWindowTextW(strDevName);
}

void CDato::Ready(bool bCmd)
{
	for (UINT id = IDC_DEVCTRL_BEGIN; id < IDC_LAST_CONTROL; id ++) {
		CWnd* pWnd = (CWnd*)GetDlgItem(id);
		if (pWnd != NULL)	pWnd->EnableWindow(bCmd);
	}
}

void CDato::DisableLine()
{
	CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_ARROW0);
	if (pButton == NULL)	return;
	BYTE cDis = pButton->GetCheck() == BST_CHECKED ? 0x10 : 0;

	ASSERT(m_nTID > -1);
	ASSERT(m_nGate > -1);

	CCabin* pCabin = (CCabin*)m_pParent;
	ASSERT(pCabin);
	CPanel* pPanel = pCabin->GetPanel();
	ASSERT(pPanel);
	pPanel->SendToSocket(m_nTID == 0 ? 14 : 15, 7, 6, 0x12, 0x50 | m_nGate, 0, 0, SDID_ATO | cDis, 0x21 | 0x80);
}

void CDato::SetupGate(bool bCmd, int nTID, int nGate)
{
	ASSERT(nTID > -1);
	ASSERT(nGate > -1);
	m_nTID = nTID;
	m_nGate = nGate;

	CButton* pButton = (CButton*)GetDlgItem(IDC_BUTTON_DEV0);
	if (pButton != NULL) {
		CString str;
		str.Format(L"ATP/ATO%d", nTID);
		pButton->SetWindowTextW(str);
	}

#if !defined(SETUP_SKIP)
	BYTE cDis = 0;
	pButton = (CButton*)GetDlgItem(IDC_CHECK_ARROW0);
	if (pButton != NULL)	cDis = pButton->GetCheck() == BST_CHECKED ? 0x10 : 0;

	CCabin* pCabin = (CCabin*)m_pParent;
	ASSERT(pCabin);
	CPanel* pPanel = pCabin->GetPanel();
	ASSERT(pPanel);
	pPanel->SendToSocket(nTID == 0 ? 14 : 15, 7, 6, bCmd ? 0x11 : 0x12, 0x50 | nGate, 0, 0, SDID_ATO | cDis, 0x21 | 0x80);
#endif
}

void CDato::CollectBuild()
{
	ATOATEXT ans;
	ZeroMemory(&ans, sizeof(ATOATEXT));

	CString str;
	for (int n = 2; n < sizeof(ATOATEXT); n ++) {
		if (n == 2 || n == 7 || n == 10 || n == 11 || n == 16) {
			for (int m = 0; m < 8; m ++) {
				CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_DEV0 + n * 8 + m);
				if (pButton != NULL && pButton->GetCheck() == BST_CHECKED)	ans.c[n] |= (1 << m);
			}
		}
		else if (n == 8) {
			((CEdit*)GetDlgItem(IDC_EDIT_DEV8))->GetWindowTextW(str);
			WORD w = (WORD)atoi((CT2CA)str);
			ans.c[8] = TOBCD(w / 100);
			ans.c[9] = TOBCD(w % 100);
		}
		else if (n == 14) {
			for (int m = 0; m < 4; m ++) {
				CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_DEV0 + n * 8 + m);
				if (pButton != NULL && pButton->GetCheck() == BST_CHECKED) {
					switch (m) {
					case 0 :	ans.c[n] = 1;	break;
					case 1 :	ans.c[n] = 0;	break;
					case 2 :	ans.c[n] = 2;	break;
					default :	ans.c[n] = 0xff;	break;
					}
				}
			}
		}
		else if (n != 5 && n != 7) {
			CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_DEV0 + n);
			if (pEdit != NULL) {
				pEdit->GetWindowTextW(str);
				ans.c[n] = (BYTE)(atoi((CT2CA)str) & 0xff);
			}
		}
	}

	if (m_nTID > -1 && m_nGate > -1) {
		m_mons[1].SetText((BYTE*)&ans, sizeof(ATOATEXT));
		CASTONCABIN(m_nTID == 0 ? 14 : 15, m_nGate, (BYTE*)&ans, sizeof(ATOATEXT));
		int mode = GetMode();
		if (IsAtoCtrl(mode)) {
			CCabin* pCabin = (CCabin*)m_pParent;
			ASSERT(pCabin);
			pCabin->AtoCtrl(GetEffortType(), GetEffort());
		}
	}
}

void CDato::RecvSdr(BYTE* p, int length)
{
	if (length != SIZE_ATOSR)	*p = 0xff;
	m_mons[0].SetText(p, length);
}

int CDato::GetMode()
{
	for (int n = 0; n < 5; n ++) {
		CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_DEV128 + n);
		if (pButton != NULL && pButton->GetCheck() == BST_CHECKED)	return OPMODE_AR + n;
	}
	return OPMODE_NON;
}

BYTE CDato::GetEffortType()
{
	int n = 0;
	for ( ; n < 3; n ++) {
		CButton* pButton = (CButton*)GetDlgItem(IDC_CHECK_DEV112 + n);
		if (pButton != NULL && pButton->GetCheck() == BST_CHECKED)	break;
	}
	switch (n) {
	case 0 :	return EFFORT_BRAKE;	break;
	case 1 :	return EFFORT_COAST;	break;
	case 2 :	return EFFORT_POWER;	break;
	default :	break;
	}
	return EFFORT_NOATC;
}

UINT CDato::GetEffort()
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_DEV13);
	if (pEdit != NULL) {
		CString str;
		pEdit->GetWindowTextW(str);
		int n = atoi((CT2CA)str);
		return ((UINT)(n * 800 / 255 + 100));		// 100 ~ 900
	}
	return 0;
}

int CDato::GetStationID(int nWhere)
{
	CString str;
	if (nWhere == WHERE_DEST)	((CEdit*)GetDlgItem(IDC_EDIT_DEV6))->GetWindowTextW(str);
	else	((CEdit*)GetDlgItem(IDC_EDIT_DEV4))->GetWindowTextW(str);
	int sid = atoi((CT2CA)str);
	if (nWhere == WHERE_CURRENT) {
		if (sid == 0)	sid = _countof(m_strStations) - 1;
		else	-- sid;
	}
	return sid;
}

CString CDato::GetStationName(int nID)
{
	if (nID >= _countof(m_strStations))	nID %= _countof(m_strStations);
	return m_strStations[nID];
}

int CDato::GetFormationNo()
{
	CString str;
	((CEdit*)GetDlgItem(IDC_EDIT_DEV8))->GetWindowTextW(str);
	return atoi((CT2CA)str);
}

void CDato::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER_DEV0, m_sliderEffort);
}

BEGIN_MESSAGE_MAP(CDato, CDialogEx)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_VSCROLL()
END_MESSAGE_MAP()

BOOL CDato::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	for (int n = 0; n < MONCOM_MAX; n ++) {
		CStatic* pStatic = (CStatic*)GetDlgItem(IDC_STATIC_DEVMON0 + n);
		if (pStatic != NULL) {
			CRect rect;
			pStatic->GetWindowRect(&rect);
			ScreenToClient(&rect);
			pStatic->ShowWindow(SW_HIDE);
			m_mons[n].Create(n == 0 ? sizeof(ATORTEXT) : sizeof(ATOATEXT), rect, this);
		}
	}
	for (UINT id = IDC_DEVCTRL_BEGIN; id < IDC_LAST_CONTROL; id ++) {
		CWnd* pWnd = (CWnd*)GetDlgItem(id);
		if (pWnd != NULL)	pWnd->EnableWindow(false);
	}
	((CEdit*)GetDlgItem(IDC_EDIT_DEV3))->SetWindowTextW(L"12");	// version
	((CEdit*)GetDlgItem(IDC_EDIT_DEV4))->SetWindowTextW(L"3");	// next station
	((CEdit*)GetDlgItem(IDC_EDIT_DEV5))->SetWindowTextW(m_strStations[3]);
	CString str;
	str.Format(L"%d", _countof(m_strStations) - 1);
	((CEdit*)GetDlgItem(IDC_EDIT_DEV6))->SetWindowTextW(str);	// destination
	((CEdit*)GetDlgItem(IDC_EDIT_DEV7))->SetWindowTextW(m_strStations[_countof(m_strStations) - 1]);
	((CEdit*)GetDlgItem(IDC_EDIT_DEV8))->SetWindowTextW(L"1234");	// formation no.
	((CEdit*)GetDlgItem(IDC_EDIT_DEV12))->SetWindowTextW(L"0");	// wheel size index

	((CEdit*)GetDlgItem(IDC_EDIT_DEV13))->SetWindowTextW(L"0");	// effort
	m_sliderEffort.SetRange(0, 255);
	m_sliderEffort.SetPos(255 - 0);
	((CButton*)GetDlgItem(IDC_CHECK_DEV113))->SetCheck(BST_CHECKED);		// neutral

	((CEdit*)GetDlgItem(IDC_EDIT_DEV15))->SetWindowTextW(L"0");	// speed
	((CButton*)GetDlgItem(IDC_CHECK_DEV131))->SetCheck(BST_CHECKED);		// mcs
	((CButton*)GetDlgItem(IDC_CHECK_DEV0))->SetCheck(BST_CHECKED);

	return TRUE;
}

BOOL CDato::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)) {
		if (pMsg->wParam == VK_RETURN) {
			CWnd* pWnd = GetFocus();
			if (pWnd == (CWnd*)GetDlgItem(IDC_EDIT_DEV4) || pWnd == (CWnd*)GetDlgItem(IDC_EDIT_DEV6))
				SetStation(pWnd == (CWnd*)GetDlgItem(IDC_EDIT_DEV4) ? IDC_EDIT_DEV4 : IDC_EDIT_DEV6);
			else if (pWnd == (CWnd*)GetDlgItem(IDC_EDIT_DEV13))	SetEffort(false);
			//else
			CollectBuild();
		}
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

BOOL CDato::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int wNotifyMsg = (int)HIWORD(wParam);
	UINT id = (UINT)LOWORD(wParam);
	if (wNotifyMsg == CBN_SELCHANGE) {
	}
	else if (wNotifyMsg == EN_KILLFOCUS) {
		CString str;
		if (id == IDC_EDIT_DEV4 || id == IDC_EDIT_DEV6)	SetStation(id);
		else if (id == IDC_EDIT_DEV13)	SetEffort(false);
		CollectBuild();
	}
	else {
		if (id > IDC_CHECK_DEV0 && id < IDC_CHECK_DEV133) {
			if (IsCheckIdc(id, 112, 115)) {
				if (((CButton*)GetDlgItem(id))->GetCheck() == BST_CHECKED) {
					UnCheck(id, 112, 115);
					m_sliderEffort.SetPos(255 - 0);
					SetEffort(true);
				}
			}
			else if (IsCheckIdc(id, 128, 132)) {
				if (((CButton*)GetDlgItem(id))->GetCheck() == BST_CHECKED)	UnCheck(id, 128, 132);
			}
			CollectBuild();
		}
		else if (id == IDC_BUTTON_DEV15)	CollectBuild();
		else if (id == IDC_CHECK_ARROW0)	DisableLine();
	}

	return CDialogEx::OnCommand(wParam, lParam);
}

void CDato::OnDestroy()
{
	CDialogEx::OnDestroy();
}

BOOL CDato::OnEraseBkgnd(CDC* pDC)
{
	return CDialogEx::OnEraseBkgnd(pDC);
}

void CDato::OnTimer(UINT_PTR nIDEvent)
{
	CDialogEx::OnTimer(nIDEvent);
}

void CDato::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar == (CScrollBar*)&m_sliderEffort) {
		SetEffort(true);
		CollectBuild();
	}

	CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
}
