// Cabin.cpp
#include "stdafx.h"
#include "Dts.h"
#include "Cabin.h"
#include "afxdialogex.h"

#include "Intro.h"
#include <math.h>

#define	SETUPGATE_TIMER		2
#define	TIME_SETUPGATE		100

IMPLEMENT_DYNAMIC(CCabin, CDialogEx)

const UINT CCabin::m_nRudderKeys[] = {
	RKEY_FORWARD,
	RKEY_NEUTRAL,
	RKEY_REVERSE,
};

PCTSTR CCabin::m_pstrRudder[] = {
	L"F", L"N", L"R"
};

PCTSTR CCabin::m_pstrRudderA[] = {
	L"FORWARD",	L"NEUTRAL",	L"REVERSE"
};

PCTSTR CCabin::m_pstrMaster[] = {
	L"EB", L"B7", L"B6", L"B5",
	L"B4", L"B3", L"B2", L"B1",
	L"C",  L"P1", L"P2", L"P3",
	L"P4"
};

PCTSTR CCabin::m_pstrHvacOp[] = {
	L"Test",
	L"Half Cool",
	L"Full Cool",
	L"Evapo. Fan",
	L"Off",
	L"Auto",
	L"1/3 Heat",
	L"2/3 Heat",
	L"Full Heat"
};

const UINT CCabin::m_nHvacOp[] = {
	1,
	8,
	4,
	3,
	5,
	9,
	7,
	11,
	15
};

const CCabin::MASTERREF CCabin::m_mastRef[] = {
	{	MASTERPOS_EB,	940,	100	},
	{	MASTERPOS_B7,	900,	100	},	// 1
	{	MASTERPOS_B6,	800,	86	},	// 2
	{	MASTERPOS_B5,	700,	73	},	// 3
	{	MASTERPOS_B4,	590,	60	},	// 4
	{	MASTERPOS_B3,	490,	47	},	// 5
	{	MASTERPOS_B2,	380,	34	},	// 6
	{	MASTERPOS_B1,	280,	22	},	// 7
	{	MASTERPOS_C,	100,	0	},	// 8
	{	MASTERPOS_P1,	360,	28	},	// 9
	{	MASTERPOS_P2,	540,	52	},	// 10
	{	MASTERPOS_P3,	720,	75	},	// 11
	{	MASTERPOS_P4,	900,	100	}	// 12
};

CCabin::CCabin(CWnd* pParent /*=NULL*/)
	: CDialogEx(CCabin::IDD, pParent)
{
	m_pParent = pParent;
	m_pAto = NULL;
	m_pLamp[0] = m_pLamp[1] = NULL;
	m_strDoor[0] = m_strDoor[1] = L"";
	m_nRudder = m_nMaster = 0;
	m_cDi = 0;
}

CCabin::CCabin(CWnd* pParent, CDato* pAto)
	: CDialogEx(CCabin::IDD, pParent)
{
	m_pParent = pParent;
	m_pAto = pAto;
	m_pAto->SetParent(this);
	m_pLamp[0] = m_pLamp[1] = NULL;
	m_strDoor[0] = m_strDoor[1] = L"";
	m_nRudder = m_nMaster = 0;
	m_cDi = m_cDo = 0;
	m_atoCtrl.cType = m_atoCtrl.uPwm = 0;
}

CCabin::~CCabin()
{
	if (m_pLamp[0] != NULL)	KILL(m_pLamp[0]);
	if (m_pLamp[1] != NULL)	KILL(m_pLamp[1]);
}

// CCabin::RudderCtrl() -> CIntro::OnRudderCtrl() -> CCabin::Activate() -> CCabin::MasterCtrl()
void CCabin::RudderAddition()
{
	int nPos = m_sliderRudder.GetPos();
	m_editRudder.SetWindowTextW(m_pstrRudder[nPos]);
	BitCtrlD(TUDIB_FORWARD, m_nRudderKeys[nPos] == RKEY_FORWARD ? BST_CHECKED : BST_UNCHECKED);
	BitCtrlD(TUDIB_REVERSE, m_nRudderKeys[nPos] == RKEY_REVERSE ? BST_CHECKED : BST_UNCHECKED);
}

void CCabin::RudderCtrl()
{
	int nPos = m_sliderRudder.GetPos();
	if (m_nRudder != nPos) {
		m_nRudder = nPos;
		RudderAddition();
	}
}

void CCabin::RudderCtrl(int nPos)
{
	if (nPos >= RUDDERPOS_FORWARD && nPos < RUDDERPOS_MAX && m_nRudder != nPos) {
		m_nRudder = nPos;
		m_sliderRudder.SetPos(nPos);
		//UpdateData(false);
		RudderAddition();
	}
}

void CCabin::MasterAddition()
{
	int nPos = m_sliderMaster.GetPos();
	m_editMaster.SetWindowTextW(m_pstrMaster[nPos]);

	CIntro* pIntro = (CIntro*)m_pParent;
	ASSERT(pIntro);
	if (!pIntro->GetTBitD(m_nTID, UDIB2IDC(TUDIB_PBR)) && !pIntro->GetTBitD(m_nTID == 0 ? 1 : 0, UDIB2IDC(TUDIB_PBR)))
		BitCtrlD(TUDIB_POWERING, nPos > MASTERPOS_C ? BST_CHECKED : BST_UNCHECKED);
	BitCtrlD(TUDIB_BRAKING, nPos < MASTERPOS_C ? BST_CHECKED : BST_UNCHECKED);
	CheckCtrl(UDIB2IDC(TUDIB_MCEBN), nPos == MASTERPOS_EB ? false : true);
	CheckCtrl(UDIB2IDC(TUDIB_EBSLR), EbState());
	PwmCtrl(PWMCH_RUDDER, m_mastRef[m_nMaster].pwm);
	//if (nPos != MASTERPOS_C) {
	//	if (nPos < MASTERPOS_C ||
	//		(!EbState() && !GetCheckState(UDIB2IDC(TUDIB_PBR)) && !GetCheckState(UDIB2IDC(TUDIB_SBR)))) {
	//		CIntro* pIntro = (CIntro*)m_pParent;
	//		ASSERT(pIntro);
	if (pIntro->GetTBitD(m_nTID, UDIB2IDC(TUDIB_POWERING)))	pIntro->SpeedCtrlFromPwm(m_nTID, (int)m_mastRef[m_nMaster].pwm);
	else	pIntro->SpeedCtrlFromPwm(m_nTID, (int)(0 - m_mastRef[m_nMaster].pwm));
	//	}
	//	//if (nPos < MASTERPOS_C) {
	//	//	int nLevel = MASTERPOS_B1 - nPos;
	//	//	if (nLevel >= (MAX_SPEEDCTRLLEVEL - 1))	nLevel = (MAX_SPEEDCTRLLEVEL - 1);
	//	//	pIntro->SpeedCtrl(m_nTID + CARPOS_MAX, false, nLevel);
	//	//}
	//	//else {
	//	//	pIntro->SpeedCtrl(m_nTID + CARPOS_MAX, (int)(MAX_SPEED / 4 * (nPos - MASTERPOS_C)));
	//	//}
	//}
}

void CCabin::MasterCtrl()
{
	int nPos = m_sliderMaster.GetPos();
	if (m_nMaster != nPos) {
		m_nMaster = nPos;
		MasterAddition();
	}
}

void CCabin::MasterCtrl(int nPos)
{
	if (nPos >= MASTERPOS_EB && nPos < MASTERPOS_MAX && m_nMaster != nPos) {
		m_nMaster = nPos;
		m_sliderMaster.SetPos(nPos);
		//UpdateData(false);
		MasterAddition();
	}
}

//void CCabin::SpeedCtrl()
//{
//	CString str;
//	m_editSpeed.GetWindowTextW(str);
//	int speed = atol((CT2CA)str);
//	if (speed < 0 || speed > 120000) {
//		str.Format(L"%d-error", speed);
//		m_editSpeed.SetWindowTextW(str);
//		return;
//	}
//	CIntro* pIntro = (CIntro*)m_pParent;
//	ASSERT(pIntro);
//	pIntro->SpeedCtrl(m_nTID + CARPOS_MAX, speed);
//}
//
void CCabin::BatteryCtrl()
{
	CString str;
	m_editBattery.GetWindowTextW(str);
	double db = atof((CT2CA)str);
	if (db > 100.f)	db = fmod(db, (double)100.f);
	str.Format(L"%.1f", db);
	m_editBattery.SetWindowTextW(str);
	UINT uPwm = (UINT)(db * 10.f);
	//PwmCtrl(PWMCH_BATTERY, uPwm);
}

void CCabin::TemperatureCtrl()
{
	CString str;
	m_editTemperature.GetWindowTextW(str);
	UINT v = (UINT)atoi((CT2CA)str);
	//PwmCtrl(PWMCH_TEMPERATURE, v);
}

//void CCabin::OrganizeCtrl()
//{
//	CString str;
//	m_editOrganize.GetWindowTextW(str);
//	int nOrganize = atoi((CT2CA)str);
//	nOrganize %= 100;
//	nOrganize = ((nOrganize / 10) << 4) | (nOrganize % 10);
//	str.Format(L"%02x", nOrganize);
//	m_editOrganize.SetWindowTextW(str);
//
//	UINT id = TOF_TID80;
//	for (int n = 0; n < 8; n ++)
//		BitCtrl(id ++, (nOrganize & (0x80 >> n)) ? BST_CHECKED : BST_UNCHECKED);
//}

void CCabin::HvacCtrl()
{
	int cur = m_comboHvac.GetCurSel();
	if (cur < HVACOPID_MAX) {
		BYTE code = (BYTE)m_nHvacOp[cur];
		for (int n = 0; n < 4; n ++)
			BitCtrlD(TUDIB_CHCS1 + n, (code & (1 << n)) ? BST_CHECKED : BST_UNCHECKED);
	}
}

void CCabin::DistanceCtrl()
{
}

void CCabin::UnitClr()
{
	CIntro* pIntro = (CIntro*)m_pParent;
	ASSERT(pIntro);
	pIntro->UnitClr(m_nTID + CARPOS_MAX);
}

void CCabin::BitCtrlD(UINT id, int nState)
{
	BitCtrl(UDIB2IDC(id), nState);
}

void CCabin::BitCtrl(UINT id, int nState)
{
	CIntro* pIntro = (CIntro*)m_pParent;
	ASSERT(pIntro);
	CButton* pButton = (CButton*)GetDlgItem(id);
	int check = nState;
	if (pButton != NULL) {
		if (nState == BST_UNCHECKED || nState == BST_CHECKED)
			pButton->SetCheck(nState);
		check = pButton->GetCheck();
	}
	if (id < IDC_CHECK_DEV0)	pIntro->BitCtrl(m_nTID + CARPOS_MAX, id + IDC_CHECK_DEV0, check == BST_CHECKED ? true : false);
	else	pIntro->BitCtrl(m_nTID + CARPOS_MAX, id, check == BST_CHECKED ? true : false);

	if (id == UDIB2IDC(TUDIB_HCR) || id == UDIB2IDC(TUDIB_TCR)) {
		UINT sid = id == UDIB2IDC(TUDIB_HCR) ? UDIB2IDC(TUDIB_TCR) : UDIB2IDC(TUDIB_HCR);
		if (check == BST_CHECKED && GetCheckState(sid))	CheckCtrl(sid, false);
		BYTE htcr = GetHtcr();
		htcr |= (pIntro->GetHtcr(m_nTID != 0 ? 0 : 1)) << 2;
		int bow;
		if (htcr == 9)	bow = (int)m_nTID;
		else if (htcr == 6)	bow = m_nTID != 0 ? 0 : 1;
		else	bow = -1;
		m_pParent->SendMessage(WM_DIRECTIONCTRL, (WPARAM)bow);
		//m_pParent->SendMessage(WM_DIRECTIONCTRL, (WPARAM)MAKELONG(m_nTID, m_nRudderKeys[nPos]));
		// press hcr after opposite tcr or neutral
	}
	else if (id == UDIB2IDC(TUDIB_EPANDS)) {
		CString str;
		((CButton*)GetDlgItem(IDC_BUTTON_DEV2))->GetWindowTextW(str);
		if (!str.Compare(TEXT_PANTOGRAPHDOWN)) {
			pIntro->PantoCtrl(false);
			((CButton*)GetDlgItem(IDC_BUTTON_DEV2))->SetWindowTextW(TEXT_PANTOGRAPHUP);
		}
	}
	else if (id >= UDIB2IDC(TUDIB_DMAA) && id <= UDIB2IDC(TUDIB_DMMM)) {
		for (UINT sid = UDIB2IDC(TUDIB_DMAA); sid <= UDIB2IDC(TUDIB_DMMM); sid ++) {
			if (sid != id)	CheckCtrl(sid, false);
		}
	}
	else if (id == UDIB2IDC(TUDIB_MCEBN) || id == UDIB2IDC(TUDIB_ATPEB) || id == UDIB2IDC(TUDIB_MRPS) ||
		id == UDIB2IDC(TUDIB_EBSH) || id == UDIB2IDC(TUDIB_EBST)) {
		CheckCtrl(UDIB2IDC(TUDIB_EBSLR), EbState());
	}
}

void CCabin::CheckCtrl(UINT id, bool bCtrl)
{
	CButton* pButton = (CButton*)GetDlgItem(id);
	if (pButton != NULL)
		pButton->SetCheck(bCtrl ? BST_CHECKED : BST_UNCHECKED);
	CIntro* pIntro = (CIntro*)m_pParent;
	ASSERT(pIntro);
	pIntro->BitCtrl(m_nTID + CARPOS_MAX, id, bCtrl);
}

bool CCabin::EbState()
{
	int pos = m_sliderMaster.GetPos();
	if (pos == MASTERPOS_EB)	return true;

	CIntro* pIntro = (CIntro*)m_pParent;
	ASSERT(pIntro);
	if (pIntro->GetTBitD(m_nTID, UDIB2IDC(TUDIB_ATPEB)) || !pIntro->GetTBitD(m_nTID, UDIB2IDC(TUDIB_MRPS)) ||
		!pIntro->GetTBitD(m_nTID, UDIB2IDC(TUDIB_EBSH)) || !pIntro->GetTBitD(m_nTID, UDIB2IDC(TUDIB_EBST)))
		return true;
	return false;
}

bool CCabin::GetCheckState(UINT id)
{
	CButton* pButton = (CButton*)GetDlgItem(id);
	if (pButton == NULL)	return false;
	int state = pButton->GetCheck();
	return state == BST_CHECKED ? true : false;
}

void CCabin::PwmCtrl(UINT sid, UINT uPwm)
{
	CIntro* pIntro = (CIntro*)m_pParent;
	ASSERT(pIntro);
	pIntro->PwmCtrl(m_nTID + CARPOS_MAX, sid, uPwm);
}

void CCabin::Ready(bool bCmd)
{
	for (UINT id = IDC_DEVCTRL_BEGIN; id < IDC_LAST_CONTROL; id ++) {
		CWnd* pWnd = (CWnd*)GetDlgItem(id);
		if (pWnd != NULL)	pWnd->EnableWindow(bCmd);
	}
	if (bCmd) {
		Activate(CABINCMD_NON);
		BatteryCtrl();
		TemperatureCtrl();
		//OrganizeCtrl();
		DistanceCtrl();
		HvacCtrl();
	}
	m_pAto->Ready();
	m_pau.Ready();
	m_pis.Ready();
	m_trs.Ready();
	//m_rtd.Ready();
	BitCtrlD(TUDIB_DLCS, BST_CHECKED);
	BitCtrlD(TUDIB_MCEBN, BST_CHECKED);
	BitCtrlD(TUDIB_MRPS, BST_CHECKED);
	BitCtrlD(TUDIB_DIR, BST_CHECKED);
	BitCtrlD(TUDIB_DIAPDC, BST_CHECKED);
	BitCtrlD(TUDIB_SIVCN1, BST_CHECKED);
	BitCtrlD(TUDIB_SIVCN2, BST_CHECKED);
	BitCtrlD(TUDIB_EBSH, BST_CHECKED);
	BitCtrlD(TUDIB_EBST, BST_CHECKED);
}

void CCabin::Activate(CABINCMD cmd)
{
	CIntro* pIntro = (CIntro*)m_pParent;
	ASSERT(pIntro);
	pIntro->EnableAlarm(false);
	//for (UINT id = TUDIB_DEADMAN; id < TUDIB_MAX; id ++)
	//	BitCtrlD(id, BST_UNCHECKED);
	//UnitClr();
	pIntro->EnableAlarm(true);
	m_pau.Initial();
	m_pis.Initial();

	//for (UINT id = IDC_DEVCTRL_BEGIN; id < IDC_LAST_CONTROL; id ++) {
	//	CWnd* pWnd = (CWnd*)GetDlgItem(id);
	//	if (pWnd != NULL && pWnd->GetSafeHwnd() != m_sliderRudder.GetSafeHwnd()) {
	//		pWnd->EnableWindow(cmd == CABINCMD_HCR ? true : false);
	//	}
	//}
	/*if (cmd == CABINCMD_HCR) {
		BitCtrlD(TUDIB_HCR, BST_CHECKED);
		BitCtrlD(TUDIB_TCR, BST_UNCHECKED);
		BitCtrlD(TUDIB_DEADMAN, BST_CHECKED);
		MasterCtrl();
	}
	else if (cmd == CABINCMD_TCR) {
		BitCtrlD(TUDIB_HCR, BST_UNCHECKED);
		BitCtrlD(TUDIB_TCR, BST_CHECKED);
		BitCtrlD(TUDIB_DEADMAN, BST_UNCHECKED);
	}
	else {
		BitCtrlD(TUDIB_HCR, BST_UNCHECKED);
		BitCtrlD(TUDIB_TCR, BST_UNCHECKED);
		BitCtrlD(TUDIB_DEADMAN, BST_UNCHECKED);
	}
	BitCtrlD(TUDIB_MCEBN, BST_CHECKED);
	BitCtrlD(TUDIB_ATPEBN, BST_CHECKED);*/
}

BYTE CCabin::GetHtcr()
{
	BYTE ht = 0;
	CIntro* pIntro = (CIntro*)m_pParent;
	ASSERT(pIntro);
	if (pIntro->GetTBitD(m_nTID, UDIB2IDC(TUDIB_HCR)))	ht |= 1;
	if (pIntro->GetTBitD(m_nTID, UDIB2IDC(TUDIB_TCR)))	ht |= 2;
	return ht;
}

int CCabin::GetRudderKey()
{
	int pos = m_sliderRudder.GetPos();
	return m_nRudderKeys[pos];
}

int CCabin::GetTrainNo()
{
	CString str;
	m_editOrganize.GetWindowTextW(str);
	return (atoi((CT2CA)str));
}

void CCabin::SetTrainID(int nID)
{
	m_nTID = nID;		// 0 or 1
	CString str;
	str.Format(L"CABIN %d", m_nTID == 0 ? 0 : 9);
	m_pAto->SetCarName(str);
	m_pau.SetCarName(str);
	m_pis.SetCarName(str);
	m_trs.SetCarName(str);
	//m_rtd.SetCarName(str);
	int tid = 29;
	str.Format(L"%02d", tid);
	m_editOrganize.SetWindowTextW(str);
	m_editCarTemp.SetWindowTextW(L"24");

	m_strDoor[0] = m_nTID == 0 ? TEXT_LEFTDOOROPEN : TEXT_RIGHTDOOROPEN;
	m_strDoor[1] = m_nTID == 0 ? TEXT_RIGHTDOOROPEN : TEXT_LEFTDOOROPEN;

	((CButton*)GetDlgItem(IDC_BUTTON_DEV1))->SetWindowTextW(m_strDoor[0]);
	((CButton*)GetDlgItem(IDC_BUTTON_DEV2))->SetWindowTextW(m_strDoor[1]);
}

void CCabin::SetTachoText(UINT uTacho)
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_DEV8);
	if (pEdit != NULL) {
		CString str;
		str.Format(L"%d", uTacho);
		pEdit->SetWindowTextW(str);
	}
}

void CCabin::DiReaction(BYTE ch)
{
	ch &= 0x60;
	if (m_cDi != ch) {
		if ((m_cDi & 0x20) != (ch & 0x20)) {
			m_pLamp[0]->SetState((ch & 0x20) ? LS_ON : LS_OFF);
		}
		if ((m_cDi & 0x40) != (ch & 0x40))
			m_pLamp[1]->SetState((ch & 0x40) ? LS_ON : LS_OFF);
		m_cDi = ch;
	}
}

void CCabin::DoReaction(BYTE cDO)
{
	for (int n = 0; n < 8; n ++) {
		if ((cDO & (1 << n)) && !(m_cDo & (1 << n))) {	// on
			m_cDo |= (1 << n);
			if (n == 5) {
				m_pLamp[0]->SetState(LS_ON);
			}
			else if (n == 6) {
				m_pLamp[1]->SetState(LS_ON);
			}
		}
		else if (!(cDO & (1 << n)) && (m_cDo & (1 << n))) {	// off
			m_cDo &= ~(1 << n);
			if (n == 5) {
				m_pLamp[0]->SetState(LS_OFF);
			}
			else if (n == 6) {
				m_pLamp[1]->SetState(LS_OFF);
			}
		}
	}
}

void CCabin::SdrReaction(BYTE* p, int leng)
{
	WORD wAdd = MAKEWORD(*(p + 1), *(p + 2));
	int ch = (int)(*p & 0xf);
	++ p;
	-- leng;
	switch (ch) {
	case 0 :	case 1 :	// ATO
		if (m_pAto != NULL)	m_pAto->RecvSdr(p, leng);
		break;
	case 2 :	case 3 :	// PAU
		if (wAdd == LADD_PAU)	m_pau.RecvSdr(p, leng);
		break;
	case 4 :	case 5 :	// PIS
		if (wAdd == LADD_PIS)	m_pis.RecvSdr(p, leng);
		break;
	case 6 :	case 7 :	// TRS
		if (wAdd == LADD_TRS)	m_trs.RecvSdr(p, leng);
		break;
	default :	break;
	}
}

void CCabin::SetupGates()
{
	m_nGateID = 0;
	SetTimer(SETUPGATE_TIMER, TIME_SETUPGATE, NULL);
}

CPanel* CCabin::GetPanel()
{
	CIntro* pIntro = (CIntro*)m_pParent;
	ASSERT(pIntro);
	return pIntro->GetPanel();
}

int CCabin::CheckRudderKeyword(CString str)
{
	for (int n = 0; n < RUDDERPOS_MAX; n ++) {
		if (!str.Compare(m_pstrRudderA[n]))	return n;
	}
	return -1;
}

int CCabin::CheckMasterKeyword(CString str)
{
	for (int n = 0; n < MASTERPOS_MAX; n ++) {
		if (!str.Compare(m_pstrMaster[n]))	return n;
	}
	return -1;
}

void CCabin::AtoCtrl(BYTE cType, UINT uEffort)
{
	int mode = m_pAto->GetMode();
	CIntro* pIntro = (CIntro*)m_pParent;
	ASSERT(pIntro);
	if (!pIntro->GetTBitD(m_nTID, UDIB2IDC(TUDIB_AFS)) && (mode > OPMODE_NON && mode < OPMODE_MANU)) {
		m_sliderMaster.SetPos(MASTERPOS_C);
		m_atoCtrl.cType = cType;		// 0:coast, 1:brake, 2:traction
		m_atoCtrl.uPwm = uEffort;		// 100 ~ 900 * 0.1 %
		if (m_atoCtrl.cType == EFFORT_POWER) {
			BitCtrlD(TUDIB_BRAKING, BST_UNCHECKED);
			if (!pIntro->GetTBitD(m_nTID, UDIB2IDC(TUDIB_PBR)) && !pIntro->GetTBitD(m_nTID == 0 ? 1 : 0, UDIB2IDC(TUDIB_PBR)))
				BitCtrlD(TUDIB_POWERING, BST_CHECKED);
		}
		else if (m_atoCtrl.cType == EFFORT_BRAKE) {
			BitCtrlD(TUDIB_POWERING, BST_UNCHECKED);
			BitCtrlD(TUDIB_BRAKING, BST_CHECKED);
		}
		else {
			BitCtrlD(TUDIB_POWERING, BST_UNCHECKED);
			BitCtrlD(TUDIB_BRAKING, BST_UNCHECKED);
		}
		PwmCtrl(PWMCH_RUDDER, m_atoCtrl.uPwm);
		//if (m_atoCtrl.cType == EFFORT_BRAKE ||
		//	(m_atoCtrl.cType == EFFORT_POWER && !EbState() && !GetCheckState(UDIB2IDC(TUDIB_PBR)) && !GetCheckState(UDIB2IDC(TUDIB_SBR)))) {
		//	CIntro* pIntro = (CIntro*)m_pParent;
		//	ASSERT(pIntro);
		if (pIntro->GetTBitD(m_nTID, UDIB2IDC(TUDIB_POWERING)))	pIntro->SpeedCtrlFromPwm(m_nTID, (int)m_atoCtrl.uPwm);
		else	pIntro->SpeedCtrlFromPwm(m_nTID, (int)(0 - m_atoCtrl.uPwm));
		//}
		//1. 여기서 ato effort type을 정하고 그에 따라 P/B를 켜고 pwm을 출력한다.
		//2. ATO에서는 슬라이더 컨트롤과 에디터 컨트롤에 따라 이 함수를 호출한다.
	}
}

int CCabin::GetEffortType()
{
	//CIntro* pIntro = (CIntro*)m_pParent;
	//if (pIntro->GetTBitD(m_nTID, UDIB2IDC(TUDIB_AFS))) {
		if (m_nMaster < MASTERPOS_C)	return EFFORT_BRAKE;
		else if (m_nMaster > MASTERPOS_C)	return EFFORT_POWER;
		else	return EFFORT_COAST;
	//}
	//else if (m_pAto != NULL)	return m_pAto->GetEffortType();
	//return EFFORT_NOATC;
}

int CCabin::GetEffort()
{
	int mode = m_pAto->GetMode();
	CIntro* pIntro = (CIntro*)m_pParent;
	if (!pIntro->GetTBitD(m_nTID, UDIB2IDC(TUDIB_AFS)) && (mode > OPMODE_NON && mode < OPMODE_MANU))
		return (int)m_atoCtrl.uPwm;
	else	return m_mastRef[m_nMaster].tbe;
}

void CCabin::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_SLIDER_DEV0, m_sliderRudder);
	DDX_Control(pDX, IDC_SLIDER_DEV1, m_sliderMaster);
	DDX_Control(pDX, IDC_EDIT_DEV0, m_editRudder);
	DDX_Control(pDX, IDC_EDIT_DEV1, m_editMaster);
	DDX_Control(pDX, IDC_EDIT_DEV2, m_editBattery);
	DDX_Control(pDX, IDC_EDIT_DEV3, m_editTemperature);
	DDX_Control(pDX, IDC_EDIT_DEV4, m_editOrganize);
	DDX_Control(pDX, IDC_EDIT_DEV5, m_editDistance);
	DDX_Control(pDX, IDC_EDIT_DEV6, m_editCarTemp);
	DDX_Control(pDX, IDC_EDIT_DEV7, m_editSpeed);
	DDX_Control(pDX, IDC_COMBO_DEV0, m_comboHvac);
}

BEGIN_MESSAGE_MAP(CCabin, CDialogEx)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
	ON_WM_TIMER()
	ON_WM_VSCROLL()
END_MESSAGE_MAP()

BOOL CCabin::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_sliderRudder.SetRange(0, RUDDERPOS_MAX - 1);
	m_sliderRudder.SetTic(RUDDERPOS_MAX);
	//m_sliderRudder.SetBuddy(&m_editRudder, FALSE);
	m_sliderRudder.SetPos(RUDDERPOS_NEUTRAL);
	m_editRudder.SetWindowTextW(m_pstrRudder[RUDDERPOS_NEUTRAL]);
	m_nRudder = RUDDERPOS_NEUTRAL;

	m_sliderMaster.SetRange(0, MASTERPOS_MAX - 1);
	m_sliderMaster.SetTic(MASTERPOS_MAX);
	//m_sliderMaster.SetBuddy(&m_editMaster, FALSE);
	m_sliderMaster.SetPos(MASTERPOS_C);
	m_sliderMaster.SetWindowTextW(m_pstrMaster[MASTERPOS_C]);
	m_nMaster = MASTERPOS_C;

	m_editBattery.SetWindowTextW(L"81.4");
	m_editTemperature.SetWindowTextW(L"21.7");
	m_editOrganize.SetWindowTextW(L"99");
	m_editDistance.SetWindowTextW(L"0");

	for (int n = HVACOPID_TEST; n < HVACOPID_MAX; n ++)
		m_comboHvac.AddString(m_pstrHvacOp[n]);
	m_comboHvac.SetCurSel(HVACOPID_OFF);

	for (int n = 0; n < 2; n ++) {
		CStatic* pStatic = (CStatic*)GetDlgItem(IDC_STATICLP_DEV0 + n);
		if (pStatic != NULL) {
			CRect rc;
			pStatic->GetWindowRect(&rc);
			ScreenToClient(&rc);
			pStatic->EnableWindow(false);
			pStatic->ShowWindow(SW_HIDE);
			m_pLamp[n] = new CAidLed();
			m_pLamp[n]->Create(L"", WS_CHILD | WS_VISIBLE | SS_CENTER | SS_CENTERIMAGE, rc, this);
			m_pLamp[n]->EnableWindow(true);
			m_pLamp[n]->ShowWindow(SW_SHOW);
			m_pLamp[n]->Set(n ? 2 : 0, LS_OFF);
		}
	}
	for (UINT id = IDC_DEVCTRL_BEGIN; id < IDC_LAST_CONTROL; id ++) {
		CWnd* pWnd = (CWnd*)GetDlgItem(id);
		if (pWnd != NULL)	pWnd->EnableWindow(false);
	}

	m_pAto->SetParent(this);
	m_pAto->SetTitle(L"CABIN", L"ATP/ATO");
	m_pAto->ShowWindow(SW_SHOW);

	m_pau.SetParent(this);
	m_pau.Create(IDD_DIALOG_DPA, this);
	m_pau.SetTitle(L"CABIN", L"PAU");
	m_pau.EnableWindow(false);
	m_pau.ShowWindow(SW_HIDE);

	m_pis.SetParent(this);
	m_pis.Create(IDD_DIALOG_DPIS, this);
	m_pis.SetTitle(L"CABIN", L"PIS");
	m_pis.EnableWindow(false);
	m_pis.ShowWindow(SW_HIDE);

	m_trs.SetParent(this);
	m_trs.Create(IDD_DIALOG_DTRS, this);
	m_trs.SetTitle(L"CABIN", L"TRS");
	m_trs.EnableWindow(false);
	m_trs.ShowWindow(SW_HIDE);

	//m_rtd.SetParent(this);
	//m_rtd.Create(IDD_DIALOG_DRTD, this);
	//m_rtd.SetTitle(L"CABIN", L"RTD");
	//m_rtd.EnableWindow(false);
	//m_rtd.ShowWindow(SW_HIDE);
	MasterCtrl();

	return TRUE;
}

BOOL CCabin::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)) {	// || pMsg->wParam == VK_F2 || pMsg->wParam == VK_F3 || pMsg->wParam == VK_F4)) {
		if (pMsg->wParam == VK_RETURN) {
			CWnd* pWnd = GetFocus();
			HWND hWnd = pWnd->GetSafeHwnd();
			if (hWnd == m_editBattery.GetSafeHwnd())	BatteryCtrl();
			else if (hWnd == m_editTemperature.GetSafeHwnd())	TemperatureCtrl();
			else if (hWnd == m_editOrganize.GetSafeHwnd())	{
				//OrganizeCtrl();
			}
			else if (hWnd == m_editCarTemp.GetSafeHwnd()) {
			}
			//else if (hWnd == m_editSpeed.GetSafeHwnd())	SpeedCtrl();
		}
		//else if (pMsg->wParam == VK_F2)	RudderCtrl(RUDDERPOS_FORWARD);
		//else if (pMsg->wParam == VK_F3)	RudderCtrl(RUDDERPOS_NEUTRAL);
		//else if (pMsg->wParam == VK_F4)	RudderCtrl(RUDDERPOS_REVERSE);
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

BOOL CCabin::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int wNotifyMsg = (int)HIWORD(wParam);
	UINT id = (UINT)LOWORD(wParam);
	CIntro* pIntro = (CIntro*)m_pParent;
	ASSERT(pIntro);
	if (wNotifyMsg == CBN_SELCHANGE) {
		if (id == IDC_COMBO_DEV0)	HvacCtrl();
	}
	else if (wNotifyMsg == EN_KILLFOCUS) {
		switch (id) {
		case IDC_EDIT_DEV2 :	BatteryCtrl();		break;
		case IDC_EDIT_DEV3 :	TemperatureCtrl();	break;
		case IDC_EDIT_DEV4 :	/*OrganizeCtrl();*/		break;
		case IDC_EDIT_DEV6 :	break;
		default :	break;
		}
	}
	else {
		if (id >= IDC_CHECK_DEV0 && id < IDC_CHECK_DEVCASCADE) {
			BitCtrl(id);
			if (id == IDC_CHECK_DEV20 || id == IDC_CHECK_DEV21) {
				bool bUp = GetCheckState(IDC_CHECK_DEV20);
				bool bDn = GetCheckState(IDC_CHECK_DEV21);
				if ((bUp && !bDn) || (!bUp && bDn)) {
					pIntro->PantoCtrl(bUp);
				}
			}
			else if (id == IDC_CHECK_DEV30) {
				if (GetCheckState(IDC_CHECK_DEV30)) {
					pIntro->TBitCtrlD(m_nTID, TUDIB_POWERING, false);
					pIntro->TBitCtrlD(m_nTID == 0 ? 1 : 0, TUDIB_POWERING, false);
				}
			}
		}
		else if (id == IDC_CHECK_DEVCASCADE) {
		}
		else if (id >= IDC_BUTTON_DEV0 && id <= IDC_BUTTON_DEV13) {
			CButton* pButton = (CButton*)GetDlgItem(id);
			if (pButton != NULL) {
				CString str;
				if (id >= IDC_BUTTON_DEV10 && id <= IDC_BUTTON_DEV13) {
					CRect rc;
					pButton->GetWindowRect(&rc);
					CWnd* pWnd = NULL;
					switch (id) {
					case IDC_BUTTON_DEV10 :	pWnd = (CWnd*)&m_pau;	break;	// pa dev
					case IDC_BUTTON_DEV11 :	pWnd = (CWnd*)&m_pis;	break;	// pis dev
					case IDC_BUTTON_DEV12 :	pWnd = (CWnd*)&m_trs;	break;	// trs dev
					//case IDC_BUTTON_DEV13 :	pWnd = (CWnd*)&m_rtd;	break;	// rtd dev
					default	:	break;
					}
					if (pWnd != NULL && !pWnd->IsWindowVisible()) {
						CRect rcd;
						pWnd->GetWindowRect(&rcd);
						CRect rcw = pIntro->GetWorkRect();
						if ((rc.left + rcd.Width()) > rcw.right)	rc.OffsetRect(-((rc.left + rcd.Width()) - rcw.right), 0);
						if ((rc.top + rcd.Height()) > rcw.bottom)	rc.OffsetRect(0, -((rc.top + rcd.Height()) - rcw.bottom));
						pWnd->SetWindowPos(NULL, rc.left, rc.top, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
						pWnd->EnableWindow();
						pWnd->ShowWindow(SW_SHOW);
					}
				}
				else {
					switch (id) {
					case IDC_BUTTON_DEV1 :	// left door control
					case IDC_BUTTON_DEV2 :	// right door control
						pButton->GetWindowTextW(str);
						if (!str.Compare(TEXT_DOORCLOSE)) {
							pIntro->DoorCtrl(DOORCMD_CLS, id == IDC_BUTTON_DEV1 ? false : true);
							((CButton*)GetDlgItem(IDC_BUTTON_DEV1))->SetWindowTextW(m_strDoor[0]);
							((CButton*)GetDlgItem(IDC_BUTTON_DEV2))->SetWindowTextW(m_strDoor[1]);
							BitCtrlD(TUDIB_DIR, BST_CHECKED);
							BitCtrlD(TUDIB_DIAPDC, BST_CHECKED);
						}
						else {
							int chk = BST_UNCHECKED;
							CButton* pCheck = (CButton*)GetDlgItem(UDIB2IDC(TUDIB_ADBS));
							if (pCheck != NULL)	chk = pCheck->GetCheck();
							if (chk != BST_CHECKED) {
								//pIntro->DoorCtrl(id == IDC_BUTTON_DEV1 ? DOORCMD_LOPEN : DOORCMD_ROPEN);
								pIntro->DoorCtrl(!str.Compare(TEXT_LEFTDOOROPEN) ? DOORCMD_LOPEN : DOORCMD_ROPEN, id == IDC_BUTTON_DEV1 ? false : true);
								pButton->SetWindowTextW(TEXT_DOORCLOSE);
								BitCtrlD(TUDIB_DIR, BST_UNCHECKED);
								BitCtrlD(TUDIB_DIAPDC, BST_UNCHECKED);
							}
						}
						break;
					case IDC_BUTTON_DEV3 :	// pantograph control
						pButton->GetWindowTextW(str);
						if (!str.Compare(TEXT_PANTOGRAPHUP)) {
							int chk = BST_UNCHECKED;
							CButton* pCheck = (CButton*)GetDlgItem(UDIB2IDC(TUDIB_EPANDS));
							if (pCheck != NULL)	chk = pCheck->GetCheck();
							if (chk != BST_CHECKED) {
								pIntro->PantoCtrl(true);
								pButton->SetWindowTextW(TEXT_PANTOGRAPHDOWN);
							}
						}
						else {
							pIntro->PantoCtrl(false);
							pButton->SetWindowTextW(TEXT_PANTOGRAPHUP);
						}
						break;
					//case IDC_BUTTON_DEV4 :
					//case IDC_BUTTON_DEV5 :
					//	pButton->GetWindowTextW(str);
					//	if (!str.Compare(id == IDC_BUTTON_DEV4 ? TEXT_DCLOADON : TEXT_ACLOADON)) {
					//		pIntro->BitCtrl(m_nTID + CARPOS_MAX, id == IDC_BUTTON_DEV4 ? UDIB2IDC(TUDIB_DLCS) : UDIB2IDC(TUDIB_ALCS), true);
					//		pButton->SetWindowTextW(id == IDC_BUTTON_DEV4 ? TEXT_DCLOADOFF : TEXT_ACLOADOFF);
					//	}
					//	else {
					//		pIntro->BitCtrl(m_nTID + CARPOS_MAX, id == IDC_BUTTON_DEV4 ? UDIB2IDC(TUDIB_DLCS) : UDIB2IDC(TUDIB_ALCS), false);
					//		pButton->SetWindowTextW(id == IDC_BUTTON_DEV4 ? TEXT_DCLOADON : TEXT_ACLOADON);
					//	}
					//	break;
					default :	break;
					}
				}
			}
		}
	}
	return CDialogEx::OnCommand(wParam, lParam);
}

void CCabin::OnDestroy()
{
	CDialogEx::OnDestroy();
}

BOOL CCabin::OnEraseBkgnd(CDC* pDC)
{
	return CDialogEx::OnEraseBkgnd(pDC);
}

void CCabin::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == SETUPGATE_TIMER) {
		//switch (m_nGateID) {
		//case 0 :	m_rtd.SetupGate(m_nTID, 0);	break;
		//case 1 :	m_rtd.SetupGate(m_nTID, 1);	break;
		//case 2 :	m_ato.SetupGate(m_nTID, 2);	break;
		//case 3 :	m_ato.SetupGate(m_nTID, 3);	break;
		//case 4 :	m_pau.SetupGate(m_nTID, 4);	break;
		//case 5 :	m_pis.SetupGate(m_nTID, 5);	break;
		//case 6 :	m_trs.SetupGate(m_nTID, 6);	break;
		//case 7 :	m_trs.SetupGate(m_nTID, 7);	break;
		//case 8 :	m_rtd.CollectBuild();	break;
		//case 9 :	m_ato.CollectBuild();	break;
		//case 10 :	m_pau.CollectBuild();	break;
		//case 11 :	m_pis.CollectBuild();	break;
		//case 12 :	m_trs.CollectBuild();	break;
		//default :	KillTimer(SETUPGATE_TIMER);
		//}
		switch (m_nGateID) {
		case 0 :	m_pAto->SetupGate(false, m_nTID, 0);	break;
		case 1 :	break;
		case 2 :	m_pau.SetupGate(false, m_nTID, 0);	break;
		case 3 :	break;
		case 4 :	m_pis.SetupGate(false, m_nTID, 1);	break;
		case 5 :	break;
		case 6 :	m_trs.SetupGate(false, m_nTID, 2);	break;
		case 7 :	break;
		case 8 :	/*m_rtd.CollectBuild();*/	break;
		case 9 :	m_pAto->CollectBuild();
			break;
		case 10 :	m_pau.CollectBuild();	break;
		case 11 :	m_pis.CollectBuild();	break;
		case 12 :	m_trs.CollectBuild();	break;
		default :	KillTimer(SETUPGATE_TIMER);
		}
		++ m_nGateID;
	}
	CDialogEx::OnTimer(nIDEvent);
}

void CCabin::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar == (CScrollBar*)&m_sliderRudder)	RudderCtrl();
	else if (pScrollBar == (CScrollBar*)&m_sliderMaster)	MasterCtrl();

	CDialogEx::OnVScroll(nSBCode, nPos, pScrollBar);
}
