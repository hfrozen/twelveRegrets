// Intro.cpp
#include "stdafx.h"
#include "Dts.h"
#include "Intro.h"
#include "afxdialogex.h"

#include <math.h>
//#include "Log.h"
//#include "Car.h"
//#include "Cabin.h"

#include "Rule.h"

#define M_PI				3.14159265358979323846

#define	WAKEUP_TIMER		2
#define	TIME_WAKEUP			500
#define	SPEEDCTRL_TIMER		4
#define	TIME_SPEEDCTRL		1000
#define	SPEEDCTRLA_TIMER	6
#define	TIME_SPEEDCTRLA		200

IMPLEMENT_DYNAMIC(CIntro, CDialogEx)

const WORD CIntro::m_wCarOrgnz[] = {
	CARF_LEFT | CARF_M1L | CARF_M2L | CARF_T1L | CARF_M2 | CARF_T2 | CARF_T1R | CARF_M1R | CARF_M2R | CARF_RIGHT,
	CARF_LEFT | CARF_M1L | CARF_M2L | CARF_T1L | CARF_M2 | CARF_T2 | CARF_T1R | CARF_M1R | CARF_M2R | CARF_RIGHT,
	CARF_LEFT | CARF_M1L | CARF_M2L | CARF_T1L | CARF_M2 | CARF_T2 | CARF_T1R | CARF_M1R | CARF_M2R | CARF_RIGHT,
	CARF_LEFT | CARF_M1L | CARF_M2L | CARF_T1L | CARF_M2 | CARF_T2 | CARF_T1R | CARF_M1R | CARF_M2R | CARF_RIGHT,
	CARF_LEFT | CARF_M1L | CARF_M2L | CARF_T1L | CARF_M2 | CARF_T2 | CARF_T1R | CARF_M1R | CARF_M2R | CARF_RIGHT
};

const WORD CIntro::m_wDevs[] = {
/* DEVID_LEFT */	DEVF_DCUL | DEVF_DCUR | DEVF_HVAC | DEVF_ECU | DEVF_SIV | DEVF_CM,	// | DEVF_TERM | DEVF_SDOOR,
/* DEVID_M1L */		DEVF_DCUL | DEVF_DCUR | DEVF_HVAC | DEVF_ECU | DEVF_V3F,	// | DEVF_SDOOR,
/* DEVID_M2L */		DEVF_DCUL | DEVF_DCUR | DEVF_HVAC | DEVF_ECU | DEVF_V3F | DEVF_PAN | DEVF_ESK,	// | DEVF_SDOOR,
/* DEVID_T1L */		DEVF_DCUL | DEVF_DCUR | DEVF_HVAC | DEVF_ECU,	// | DEVF_SDOOR,
/* DEVID_M2 */		DEVF_DCUL | DEVF_DCUR | DEVF_HVAC | DEVF_ECU | DEVF_V3F | DEVF_PAN,	// | DEVF_SDOOR,
/* DEVID_T2 */		DEVF_DCUL | DEVF_DCUR | DEVF_HVAC | DEVF_ECU | DEVF_SIV | DEVF_CM,	//SB | DEVF_SDOOR,
/* DEVID_T1R */		DEVF_DCUL | DEVF_DCUR | DEVF_HVAC | DEVF_ECU | DEVF_ESK,	// | DEVF_SDOOR,
/* DEVID_M1R */		DEVF_DCUL | DEVF_DCUR | DEVF_HVAC | DEVF_ECU | DEVF_V3F,	// | DEVF_SDOOR,
/* DEVID_M2R */		DEVF_DCUL | DEVF_DCUR | DEVF_HVAC | DEVF_ECU | DEVF_V3F | DEVF_PAN,	// | DEVF_SDOOR,
/* DEVID_RIGHT */	DEVF_DCUL | DEVF_DCUR | DEVF_HVAC | DEVF_ECU | DEVF_SIV | DEVF_CM,	//SB | DEVF_TERM | DEVF_SDOOR,
};

const CARTYPE CIntro::m_cCarTypes[] = {
	CARTYPE_TC,	CARTYPE_M,	CARTYPE_M,	CARTYPE_T1,	CARTYPE_M,
	CARTYPE_T2,	CARTYPE_T1,	CARTYPE_M,	CARTYPE_M,	CARTYPE_TC
};

CIntro::CIntro(CWnd* pParent /*=NULL*/)
	: CDialogEx(CIntro::IDD, pParent)
{
	m_pParent = pParent;
	m_nMonitor = 1;
	m_bFreeze = false;
	ZeroMemory(&m_speed, sizeof(CALCSPEED));
	ZeroMemory(&m_spda, sizeof(CALCSPEEDA));
	m_dbDia = 0.86f;	// 860mm

	m_coi = CARORGNZ_10;
	m_pLog[0] = m_pLog[1] = m_pLog[2] = NULL;
	for (int n = 0; n < CARPOS_MAX; n ++)
		m_pCar[n] = NULL;
	m_pCabin[0] = m_pCabin[1] = NULL;
	m_pAto[0] = m_pAto[1] = NULL;
	m_pDashb = NULL;
	m_pPanel = NULL;

	//m_nTrainDir = 0;
	m_nBow = -1;

	m_pServerSocket = NULL;
}

CIntro::~CIntro()
{
	KILL(m_pLog[0]);
	KILL(m_pLog[1]);
	KILL(m_pLog[2]);
	for (int n = 0; n < CARPOS_MAX; n ++)
		KILL(m_pCar[n]);
	KILL(m_pCabin[0]);
	KILL(m_pCabin[1]);
	KILL(m_pAto[0]);
	KILL(m_pAto[1]);
	KILL(m_pDashb);
	KILL(m_pPanel);
	ERASE(m_font);

	if (m_pServerSocket != NULL) {
		m_pServerSocket->Close();
		delete m_pServerSocket;
		m_pServerSocket = NULL;
	}
	for (POSITION pos = m_tcmes.GetHeadPosition(); pos != NULL; ) {
		CTcm* pTcm = (CTcm*)m_tcmes.GetNext(pos);
		if (pTcm != NULL) {
			delete pTcm;
			pTcm = NULL;
		}
	}
	m_tcmes.RemoveAll();
}

void CIntro::Log(CString strLog, COLORREF crText)
{
	if (m_pLog[0] != NULL)	m_pLog[0]->Trace(strLog, crText);
}

void CIntro::Logp(CString strLog, COLORREF crText)
{
	if (m_pLog[1] != NULL)	m_pLog[1]->Track(strLog, crText);
}

void CIntro::Logr(CString strLog, COLORREF crText)
{
	if (m_pLog[2] != NULL)	m_pLog[2]->Track(strLog, crText);
}

void CIntro::DistributeDies(int cid, BYTE ch)
{
	if (cid >= CARPOS_MAX)	return;
	if (m_pCar[cid] != NULL)	m_pCar[cid]->DiReaction(ch);
	if (cid == CARPOS_LEFT || cid == CARPOS_RIGHT) {
		if (m_pCabin[cid == CARPOS_LEFT ? 0 : 1] != NULL)	m_pCabin[cid == CARPOS_LEFT ? 0 : 1]->DiReaction(ch);
	}
}

void CIntro::DistributeDoes(int cid, BYTE cDO)
{
	if (cid >= CARPOS_MAX)	return;
	if (m_pCar[cid] != NULL)	m_pCar[cid]->DoReaction(cDO);
	if (cid == CARPOS_LEFT || cid == CARPOS_RIGHT) {
		if (m_pCabin[cid == CARPOS_LEFT ? 0 : 1] != NULL)	m_pCabin[cid == CARPOS_LEFT ? 0 : 1]->DoReaction(cDO);
	}
}

void CIntro::DistributeRecv(int id, BYTE* p, int leng)
{
	// id : 0 -> 3f0 = left tc, 1~10 = each cc, 11 -> 3fb = right cc
	if (id == 0 || id == 11)	m_pCabin[id == 0 ? 0 : 1]->SdrReaction(p, leng);
	else if (id < 11)	m_pCar[id - 1]->SdrReaction(p, leng);
}

void CIntro::PantoCtrl(bool bCtrl)
{
	for (int n = CARPOS_LEFT; n < CARPOS_MAX; n ++) {
		if (m_wDevs[n] & DEVF_PAN)
			m_pCar[n]->SetPanto(bCtrl);
	}
}

void CIntro::DoorCtrl(DOORCMD cmd, bool bSide)
{
	for (int n = CARPOS_LEFT; n < CARPOS_MAX; n ++) {
		PBODYSTYLE pBs = m_pCar[n]->GetBody();
		if (pBs->bypass != CARFORM_BPON)		m_pCar[n]->SetDoor(cmd, bSide);
	}
}

void CIntro::SendBinCmd(int id, int leng, ...)
{
	if (m_pPanel != NULL) {
		va_list vargs;
		va_start(vargs, leng);
		m_pPanel->SendToSocket(id, leng, vargs);
		va_end(vargs);
	}
}

//void CIntro::SpeedCtrlA(UINT cid, int notch)
//{
//	m_spda.cid = cid;
//	if (notch > MASTERPOS_C) {
//		m_spda.vTar = MAX_SPEED * (notch - MASTERPOS_C) / (MASTERPOS_P4 - MASTERPOS_C);
//		if (m_spda.vCur > m_spda.vTar)	m_spda.vStep = DEC_SPEED;
//		else if (m_spda.vCur < m_spda.vTar)	m_spda.vStep = INC_SPEED;
//		else	m_spda.vStep = 0;
//	}
//	else if (notch < MASTERPOS_C) {
//		m_spda.vTar = 0;
//		if (m_spda.vCur > 0)	m_spda.vStep = DEC_SPEED * (MASTERPOS_C - notch);
//		else	m_spda.vStep = 0;
//	}
//	else {
//		m_spda.vStep = 0;
//	}
//	if (m_spda.vStep > 0)	SetTimer(SPEEDCTRLA_TIMER, TIME_SPEEDCTRLA, NULL);
//}
//
void CIntro::SpeedCtrlFromPwm(UINT cid, int iPwm)	// iPwm = 100 ~ 900, -100 ~ -940
{
	if (cid > 1)	return;

	m_spda.cid = cid + CARPOS_MAX;
	if (iPwm > 100) {
		m_spda.vTar = MAX_SPEED * (iPwm - 100) / 800;
		if (m_spda.vCur > m_spda.vTar)	m_spda.vStep = DEC_SPEED;
		else if (m_spda.vCur < m_spda.vTar)	m_spda.vStep = INC_SPEED;
		else	m_spda.vStep = 0;
	}
	else if (iPwm < -100) {
		m_spda.vTar = 0;
		if (m_spda.vCur > 0)	m_spda.vStep = DEC_SPEED * (abs(iPwm) - 100) / 840;
		else	m_spda.vStep = 0;
	}
	else	m_spda.vStep = 0;

	if (m_spda.vStep > 0)	SetTimer(SPEEDCTRLA_TIMER, TIME_SPEEDCTRLA, NULL);
}

//void CIntro::SpeedCtrl(UINT cid, int vSpeed)
//{
//	if (vSpeed < 0)	return;
//
//	if (m_speed.vTarget != vSpeed) {
//		if (m_speed.bCtrl) {
//			KillTimer(SPEEDCTRL_TIMER);
//			m_speed.bCtrl = false;
//		}
//		m_speed.cid = cid;
//		m_speed.vTarget = vSpeed;
//		m_speed.bTenor = (m_speed.vTarget > m_speed.vCurrent) ? true : false;	// true:up, false:down
//		double diff = fabs((double)m_speed.vTarget - (double)m_speed.vCurrent);
//		int n = 0;
//		for ( ; n < MAX_SPEEDCTRLLEVEL; n ++) {
//			if (diff < (double)(MAX_SPEED * (n + 1) / MAX_SPEEDCTRLLEVEL))	break;
//		}
//		if (n >= MAX_SPEEDCTRLLEVEL)	n = MAX_SPEEDCTRLLEVEL - 1;
//		m_speed.nLevel = n;
//		m_speed.db[0] = 3.f;
//		m_speed.db[1] = 2.3f + (m_speed.nLevel * 0.1f);
//		m_speed.db[2] = 2.f;
//		m_speed.db[3] = 150.f + (m_speed.nLevel * 25.f);
//		m_speed.nCount = 0;
//		m_speed.nBend = -1;
//		m_speed.bCtrl = true;
//		SetTimer(SPEEDCTRL_TIMER, TIME_SPEEDCTRL, NULL);
//	}
//}
//
//void CIntro::SpeedCtrl(UINT cid, bool bRise, int nLevel)
//{
//	if (nLevel >= MAX_SPEEDCTRLLEVEL)	return;
//
//	if (m_speed.bCtrl) {
//		KillTimer(SPEEDCTRL_TIMER);
//		m_speed.bCtrl = false;
//	}
//	m_speed.cid = cid;
//	m_speed.vTarget = bRise ? MAX_SPEED : 0;
//	m_speed.bTenor = bRise;	// true:up, false:down
//	m_speed.nLevel = nLevel;
//	m_speed.db[0] = 3.f;
//	m_speed.db[1] = 2.3f + (m_speed.nLevel * 0.1f);
//	m_speed.db[2] = 2.f;
//	m_speed.db[3] = 150.f + (m_speed.nLevel * 25.f);
//	m_speed.nCount = 0;
//	m_speed.nBend = -1;
//	m_speed.bCtrl = true;
//	SetTimer(SPEEDCTRL_TIMER, TIME_SPEEDCTRL, NULL);
//}
//

void CIntro::StirWatcher(CString strIp)
{
	ShellExecute(NULL, L"open", L"D:/TcmsWork/DuoLine/VC/Watcher/Debug/Watcher.exe", strIp, NULL, SW_SHOW);
}

void CIntro::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CHECK_ARROW0, m_checkRxHide);
	DDX_Control(pDX, IDC_CHECK_ARROW1, m_checkTxHide);
	DDX_Control(pDX, IDC_BUTTON_DEV0, m_btnAct);
	DDX_Control(pDX, IDC_BUTTON_DEV1, m_btnPanel);
	DDX_Control(pDX, IDC_BUTTON_DEV13, m_btnDebuger);
	DDX_Control(pDX, IDC_BUTTON_DEV14, m_btnPort);
	DDX_Control(pDX, IDC_BUTTON_DEV15, m_btnExit);
}

BEGIN_MESSAGE_MAP(CIntro, CDialogEx)
	ON_WM_DESTROY()
	ON_WM_TIMER()
	ON_MESSAGE(WM_DIRECTIONCTRL, &CIntro::OnDirectionCtrl)
	ON_MESSAGE(WM_CONNECTDIO, &CIntro::OnConnectDio)
	ON_MESSAGE(WM_CONNECTSCU, &CIntro::OnConnectScu)
	ON_MESSAGE(WM_SACCEPT, &CIntro::OnAccept)
	ON_MESSAGE(WM_INFORMTCM, &CIntro::OnInformTcm)
	ON_MESSAGE(WM_CLOSETCM, &CIntro::OnCloseTcm)
END_MESSAGE_MAP()

BOOL CIntro::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	m_nMonitor = ::GetSystemMetrics(SM_CMONITORS);
	int cx = ::GetSystemMetrics(SM_CXFULLSCREEN);
	int cy = ::GetSystemMetrics(SM_CYFULLSCREEN);
	m_rect[0] = m_rect[1] = CRect(0, 0, cx, cy);
	if (m_nMonitor > 1) {
		int ml = 2;
		int x = ::GetSystemMetrics(SM_CXVIRTUALSCREEN);
		POINT pt;
		pt.x = pt.y = 0;
		HMONITOR hMonitorl = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
		HMONITOR hMonitorr;
		for (pt.x = 0; pt.x < x; pt.x += 1000) {
			hMonitorr = ::MonitorFromPoint(pt, MONITOR_DEFAULTTONEAREST);
			if (hMonitorr != hMonitorl) {
				MONITORINFOEX mi;
				mi.cbSize = sizeof(MONITORINFOEX);
				::GetMonitorInfoW(hMonitorr, &mi);
				m_rect[ml ++] = CRect(mi.rcMonitor.left, mi.rcMonitor.top, mi.rcMonitor.right, mi.rcMonitor.bottom);
				hMonitorl = hMonitorr;
			}
		}
		//m_rect[0] = m_rect[m_nMonitor];
		m_rect[0] = m_rect[2];
	}
	SetWindowPos(NULL, m_rect[0].left, m_rect[0].top, m_rect[0].Width(), m_rect[0].Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	if (m_pParent != NULL)
		m_pParent->SetWindowPos(NULL, m_rect[0].left, m_rect[0].top, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
	m_rect[9] = m_rect[0];

	for (int n = 0; n < 3; n ++) {
		if (m_pLog[n] != NULL)	delete m_pLog[n];
		m_pLog[n] = new CLog(this);
		m_pLog[n]->Create(IDD_DIALOG_LOG, this);
		m_pLog[n]->ShowWindow(SW_SHOW);
		if (n == 0) {
#if	defined(_SIMM_SOCKET_)
			Log(L"TCMS Simulator start.(pseudo nodes)\r\n");
#else
			Log(L"TCMS Simulator start.\r\n");
#endif
		}
	}

	CRect rc;
	if (m_pPanel != NULL)	KILL(m_pPanel);
	m_pPanel = new CPanel(this);
	m_pPanel->Create(IDD_DIALOG_PANEL, this);
	m_pPanel->GetWindowRect(&rc);
	m_pPanel->SetWindowPos(NULL, m_rect[1].left + (m_rect[1].Width() - rc.Width()) / 2,
								m_rect[1].top + (m_rect[1].Height() - rc.Height()) / 2,
								0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
	m_pPanel->ShowWindow(SW_HIDE);

	CPoint pt(0, 0);
	for (int n = 0; n < 2; n ++) {
		CPoint pta = pt;
		if (m_pAto[n] != NULL)	KILL(m_pAto[n]);
		if (m_pCabin[n] != NULL)	KILL(m_pCabin[n]);
		m_pAto[n] = new CDato(this);
		m_pAto[n]->Create(IDD_DIALOG_DATO, this);
		//m_pAto[n]->ShowWindow(SW_HIDE);
		m_pAto[n]->SetWindowPos(NULL, pta.x, pta.y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
		m_pAto[n]->GetWindowRect(&rc);
		pta.y += rc.Height();
		m_pCabin[n] = new CCabin(this, m_pAto[n]);
		m_pCabin[n]->Create(IDD_DIALOG_CABIN, this);
		//m_pCabin[n]->ShowWindow(SW_HIDE);
		m_pCabin[n]->SetTrainID(n);
		m_pCabin[n]->SetWindowPos(NULL, pta.x, pta.y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
	}

	m_pCabin[0]->GetWindowRect(&rc);
	pt.x = rc.Width();
	m_pAto[0]->GetWindowRect(&rc);
	pt.y = rc.Height();
	for (int n = 0; n < CARPOS_MAX; n ++) {
		if (m_pCar[n] != NULL)	KILL(m_pCar[n]);
		if (m_wCarOrgnz[m_coi] & (1 << n)) {
			m_pCar[n] = new CCar(this, n);
			m_pCar[n]->Create(IDD_DIALOG_CAR, this);
			m_pCar[n]->SetWindowPos(NULL, pt.x, pt.y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
			m_pCar[n]->ShowWindow(SW_SHOW);

			BODYSTYLE bs;
			ZeroMemory(&bs, sizeof(BODYSTYLE));
			bs.nID = n;		// for body
			bs.nType = m_cCarTypes[n];
			bs.coach = CARFORM_COACHN;
			if (m_wDevs[n] & DEVF_PAN)	bs.pan[0] = bs.pan[1] = CARFORM_PANDN;
			if (m_wDevs[n] & DEVF_V3F)	bs.wheel = CARFORM_WHEELD;
			//if (m_wDevs[n] & DEVF_SDOOR)	bs.side = CARFORM_SIDENR;
			m_pCar[n]->SetBody(bs);
			m_pCar[n]->GetWindowRect(&rc);
			pt.x += rc.Width();
		}
	}
	m_pCabin[1]->SetWindowPos(NULL, pt.x, pt.y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);

	m_pCabin[1]->ShowWindow(SW_SHOW);
	m_pCabin[0]->ShowWindow(SW_SHOW);

	CRect rca;
	m_pCabin[0]->GetClientRect(&rca);
	m_pCabin[1]->GetWindowRect(&rc);
	int xl = pt.x;
	pt.x += rc.Width();
	pt.y = rc.bottom;
	int yl = pt.y;
	int yll = yl - 20;
	m_btnAct.GetWindowRect(&rc);
	m_btnAct.SetWindowPos(NULL, pt.x - rc.Width(), yll, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
	m_btnPanel.SetWindowPos(NULL, pt.x - rc.Width(), yll + rc.Height(), 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
	m_btnDebuger.SetWindowPos(NULL, pt.x - rc.Width(), yll + rc.Height() * 2, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
	m_btnPort.SetWindowPos(NULL, pt.x - rc.Width(), yll + rc.Height() * 3, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
	m_btnExit.SetWindowPos(NULL, pt.x - rc.Width(), yll + rc.Height() * 4, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
	m_pAto[1]->GetWindowRect(&rc);
	m_pAto[1]->SetWindowPos(NULL, pt.x - rc.Width(), 0, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
	m_pAto[0]->ShowWindow(SW_SHOW);
	m_pAto[1]->ShowWindow(SW_SHOW);
	m_pAto[0]->GetWindowRect(&rc);
	m_pAto[1]->GetWindowRect(&rc);
	m_rect[9].right = m_rect[9].left + pt.x;

	if (m_pDashb != NULL)	KILL(m_pDashb);
	m_pDashb = new CDashb(this);
	m_pDashb->Create(IDD_DIALOG_DASHBOARD, this);
	m_pDashb->GetWindowRect(&rc);
	m_pDashb->SetWindowPos(NULL, m_rect[9].left + (m_rect[9].Width() - rc.Width()) / 2, 0, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
	m_pDashb->ShowWindow(SW_HIDE);

	GetClientRect(&rc);
	m_pLog[0]->SetWindowPos(NULL, pt.x + 2, 0, rc.Width() - (pt.x + 2), rc.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	m_pLog[0]->Repos(rc.Width() - (pt.x + 2) - 6, rc.Height() - 6);
	//m_pLog[0]->Trace(L"log0");
	CRect rtb;
	m_checkRxHide.GetWindowRect(&rtb);
	//m_checkRxHide.SetWindowPos(NULL, 2, yl - 20, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
	m_checkRxHide.SetWindowPos(NULL, rca.right + 4, yl - 30 - rtb.Height(), 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
	m_checkRxHide.SetCheck(BST_CHECKED);
	//m_checkTxHide.SetWindowPos(NULL, 80, yl - 20, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
	m_checkTxHide.SetWindowPos(NULL, rca.right + 4 + rtb.Width(), yl - 30 - rtb.Height(), 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
	m_checkTxHide.SetCheck(BST_CHECKED);
#define	LOGY_OFFSET		14
	m_pLog[1]->SetWindowPos(NULL, 2, yl - 20, xl / 2 - 2, rc.Height() - yl + LOGY_OFFSET + 8, SWP_NOZORDER | SWP_NOACTIVATE);
	m_pLog[1]->Repos(xl / 2 - 8, rc.Height() - yl + LOGY_OFFSET);
	//m_pLog[1]->Trace(L"log1");
	m_pLog[2]->SetWindowPos(NULL, xl / 2 + 2, yl - 20, xl / 2 - 2, rc.Height() - yl + LOGY_OFFSET + 8, SWP_NOZORDER | SWP_NOACTIVATE);
	m_pLog[2]->Repos(xl / 2 - 8, rc.Height() - yl + LOGY_OFFSET);
	//m_pLog[2]->Trace(L"log2");
#undef	LOGY_OFFSET

	m_pServerSocket = new CServerSocket();
	m_pServerSocket->SetHwnd(GetSafeHwnd());
	m_pServerSocket->Create(5000);
	m_pServerSocket->Listen();

	m_doc.SetParent(this);
	m_cook.SetParent(this);
	SetTimer(WAKEUP_TIMER, TIME_WAKEUP, NULL);

	return TRUE;
}

BOOL CIntro::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)) {
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

BOOL CIntro::OnCommand(WPARAM wParam, LPARAM lParam)
{
	UINT id = (UINT)LOWORD(wParam);
	if (id == IDC_BUTTON_DEV0) {
		CString str;
		m_btnAct.GetWindowTextW(str);
		if (!str.Compare(L"동 작")) {
			CFileDialog dlg(TRUE, L"txt", NULL, OFN_LONGNAMES | OFN_NOCHANGEDIR | OFN_HIDEREADONLY, L"Script files(*.txt)|*.txt||", NULL);
			dlg.m_ofn.lpstrInitialDir = L"d:\\TcmsWork\\VC\\DtsDoc\\Script";
			if (dlg.DoModal() == IDOK) {
				CString strFile = dlg.GetFileName();
				if (m_cook.Propel(strFile))	m_btnAct.SetWindowTextW(L"정 지");
			}
		}
		else {
			m_cook.Terminate();
			m_btnAct.SetWindowTextW(L"동 작");
		}
	}
	else if (id == IDC_BUTTON_DEV1)	m_pPanel->ShowWindow(m_pPanel->IsWindowVisible() ? SW_HIDE : SW_SHOW);
	else if (id == IDC_BUTTON_DEV13)
		WinExec("D:/TcmsWork/DuoLine/VC/Watcher/Debug/Watcher.exe", SW_SHOW);
	else if (id == IDC_BUTTON_DEV14) {
		CString str;
		m_btnPort.GetWindowTextW(str);
		if (!str.Compare(L"연 결")) {
			m_pPanel->Open();
			m_btnPort.SetWindowTextW(L"끊 기");
		}
		else {
			m_pPanel->Close();
			m_btnPort.SetWindowTextW(L"연 결");
		}
	}
	else if (id == IDC_BUTTON_DEV15)	m_pParent->SendMessageW(WM_CLOSE, NULL, NULL);

	return CDialogEx::OnCommand(wParam, lParam);
}

void CIntro::OnDestroy()
{
	CDialogEx::OnDestroy();
}

void CIntro::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == WAKEUP_TIMER) {
		KillTimer(nIDEvent);
		m_cook.Initial();
		m_pPanel->Open();
		m_btnPort.SetWindowTextW(L"끊 기");
	}
	if (nIDEvent == SPEEDCTRLA_TIMER) {
		if (m_spda.vCur != m_spda.vTar) {
			if (m_spda.vCur > m_spda.vTar) {
				if ((m_spda.vCur - m_spda.vStep) > m_spda.vTar)		m_spda.vCur -= m_spda.vStep;
				else	m_spda.vCur = m_spda.vTar;
			}
			else {		// m_spda.vCur < m_spda.vTar
				if ((m_spda.vCur + m_spda.vStep) < m_spda.vTar)	m_spda.vCur += m_spda.vStep;
				else	m_spda.vCur = m_spda.vTar;
			}
			WORD wPeriod = (WORD)(1000.f / ((double)m_spda.vCur / 3600.f / (M_PI * m_dbDia) * 90.f) * 72.f);
			PwmCtrl(m_spda.cid, PWMCH_TACHO, (UINT)wPeriod);
			//CString str;
			//str.Format(L"pwm %d\n", wPeriod);
			//Log(str);
		}
		else	KillTimer(SPEEDCTRLA_TIMER);
	}
	/*if (nIDEvent == SPEEDCTRL_TIMER) {
		bool bGoal = false;
		if (m_speed.bTenor) {	// up
			if (m_speed.vCurrent < (int)((double)m_speed.vTarget * TARGETSPEED_BENDING))
				m_speed.vCurrent = (int)((double)m_speed.vCurrent + (m_speed.db[0] *
													pow(m_speed.nCount, m_speed.db[1])) / m_speed.db[2]);
			else {
				if (m_speed.nBend < 0)	m_speed.nBend = m_speed.nCount - 1;
				m_speed.vCurrent = (int)((double)m_speed.vCurrent + (m_speed.db[0] * m_speed.db[1] *
													m_speed.db[2] / (double)(m_speed.nCount - m_speed.nBend)) *
													m_speed.db[3]);
			}
			if (m_speed.vCurrent >= m_speed.vTarget) {
				m_speed.vCurrent = m_speed.vTarget;
				bGoal = true;
			}
		}
		else {		// down
			if (m_speed.vCurrent > (int)((double)m_speed.vTarget * (1.f - TARGETSPEED_BENDING)))
				m_speed.vCurrent = (int)((double)m_speed.vCurrent - (m_speed.db[0] *
													pow(m_speed.nCount, m_speed.db[1])) / m_speed.db[2]);
			else {
				if (m_speed.nBend < 0)	m_speed.nBend = m_speed.nCount - 1;
				m_speed.vCurrent = (int)((double)m_speed.vCurrent - (m_speed.db[0] * m_speed.db[1] *
													m_speed.db[2] / (double)(m_speed.nCount - m_speed.nBend)) *
													m_speed.db[3]);
			}
			if (m_speed.vCurrent <= m_speed.vTarget) {
				m_speed.vCurrent = m_speed.vTarget;
				bGoal = true;
			}
		}
		CString str;
		str.Format(L"speed %d\n", m_speed.vCurrent);
		Log(str);
		++ m_speed.nCount;
		WORD wPeriod = (WORD)(1000.f / ((double)m_speed.vCurrent / 3600.f / (M_PI * m_dbDia) * 90.f) * 72.f);
		PwmCtrl(m_speed.cid, PWMCH_TACHO, (UINT)wPeriod);
		if (bGoal) {
			m_speed.bCtrl = false;
			KillTimer(SPEEDCTRL_TIMER);
		}
	}*/
	CDialogEx::OnTimer(nIDEvent);
}

/*LRESULT CIntro::OnDirectionCtrl(WPARAM wParam, LPARAM lParam)
{
	//clock_t begin = clock();
	int nID = (int)LOWORD(wParam);		// 0/1
	BYTE nDir = (UINT)HIWORD(wParam);	// 0/1/2
	BYTE nMask = 3;
	if (nID) {
		nDir <<= 2;
		nMask <<= 2;
	}
	BYTE nTrainDir = m_nTrainDir;
	nTrainDir &= ~nMask;
	nTrainDir |= nDir;
	if (m_nTrainDir != nTrainDir) {
		m_nTrainDir = nTrainDir;
		if (m_nTrainDir == 0 || m_nTrainDir == 1 || m_nTrainDir == 2 || m_nTrainDir == 4 || m_nTrainDir == 8) {
			if (m_nTrainDir == 0) {
				m_pPanel->InterLog(L"Activate 0\n");
				m_pCabin[0]->Activate(CABINCMD_NON);
				m_pPanel->InterLog(L"Activate 1\n");
				m_pCabin[1]->Activate(CABINCMD_NON);
				m_pPanel->InterLog(L"SetBody 0\n");
				PBODYSTYLE pBs = m_pCar[CARPOS_LEFT]->GetBody();
				pBs->coach = CARFORM_COACHN;
				m_pCar[CARPOS_LEFT]->SetBody(*pBs);
				m_pPanel->InterLog(L"SetBody 1\n");
				pBs = m_pCar[CARPOS_RIGHT]->GetBody();
				pBs->coach = CARFORM_COACHN;
				m_pCar[CARPOS_RIGHT]->SetBody(*pBs);
			}
			else if (m_nTrainDir < 4) {
				m_pPanel->InterLog(L"Activate 0\n");
				m_pCabin[0]->Activate(CABINCMD_HCR);
				m_pPanel->InterLog(L"Activate 1\n");
				m_pCabin[1]->Activate(CABINCMD_TCR);
				m_pPanel->InterLog(L"SetBody 0\n");
				PBODYSTYLE pBs = m_pCar[CARPOS_LEFT]->GetBody();
				pBs->coach = CARFORM_COACHL;
				m_pCar[CARPOS_LEFT]->SetBody(*pBs);
				m_pPanel->InterLog(L"SetBody 1\n");
				pBs = m_pCar[CARPOS_RIGHT]->GetBody();
				pBs->coach = CARFORM_COACHN;
				m_pCar[CARPOS_RIGHT]->SetBody(*pBs);
			}
			else {
				m_pPanel->InterLog(L"Activate 0\n");
				m_pCabin[0]->Activate(CABINCMD_TCR);
				m_pPanel->InterLog(L"Activate 1\n");
				m_pCabin[1]->Activate(CABINCMD_HCR);
				m_pPanel->InterLog(L"SetBody 0\n");
				PBODYSTYLE pBs = m_pCar[CARPOS_LEFT]->GetBody();
				pBs->coach = CARFORM_COACHN;
				m_pCar[CARPOS_LEFT]->SetBody(*pBs);
				m_pPanel->InterLog(L"SetBody 1\n");
				pBs = m_pCar[CARPOS_RIGHT]->GetBody();
				pBs->coach = CARFORM_COACHR;
				m_pCar[CARPOS_RIGHT]->SetBody(*pBs);
			}
		}
		else {
			//m_pCabin[0]->Activate(false);
			//m_pCabin[1]->Activate(false);
			//m_pCabin[0]->SendMessage(WM_COMMAND, CCabin::TOF_HCR, 0);
			//m_pCabin[0]->SendMessage(WM_COMMAND, CCabin::TOF_TCR, 0);
			//m_pCabin[1]->SendMessage(WM_COMMAND, CCabin::TOF_HCR, 0);
			//m_pCabin[1]->SendMessage(WM_COMMAND, CCabin::TOF_TCR, 0);
		}
	}
	//clock_t end = clock();
	//double dur = (double)(end - begin) / CLOCKS_PER_SEC;
	//CString str;
	//str.Format(L"RudderCtrl %f.\n", dur);
	//Log(str);

	return 0;
}
*/
LRESULT CIntro::OnDirectionCtrl(WPARAM wParam, LPARAM lParam)
{
	//clock_t begin = clock();
	int nID = (int)wParam;		// 0/1
	if (m_nBow != nID) {
		m_nBow = nID;
		if (m_nBow == 0) {
			m_pPanel->InterLog(L"Activate 0\n");		// 0:L
			m_pCabin[0]->Activate(CABINCMD_HCR);
			m_pPanel->InterLog(L"Activate 1\n");
			m_pCabin[1]->Activate(CABINCMD_TCR);
			m_pPanel->InterLog(L"SetBody 0\n");
			PBODYSTYLE pBs = m_pCar[CARPOS_LEFT]->GetBody();
			pBs->coach = CARFORM_COACHL;
			m_pCar[CARPOS_LEFT]->SetBody(*pBs);
			m_pPanel->InterLog(L"SetBody 1\n");
			pBs = m_pCar[CARPOS_RIGHT]->GetBody();
			pBs->coach = CARFORM_COACHN;
			m_pCar[CARPOS_RIGHT]->SetBody(*pBs);
		}
		else if (m_nBow == 1) {
			m_pPanel->InterLog(L"Activate 0\n");		// 1:R
			m_pCabin[0]->Activate(CABINCMD_TCR);
			m_pPanel->InterLog(L"Activate 1\n");
			m_pCabin[1]->Activate(CABINCMD_HCR);
			m_pPanel->InterLog(L"SetBody 0\n");
			PBODYSTYLE pBs = m_pCar[CARPOS_LEFT]->GetBody();
			pBs->coach = CARFORM_COACHN;
			m_pCar[CARPOS_LEFT]->SetBody(*pBs);
			m_pPanel->InterLog(L"SetBody 1\n");
			pBs = m_pCar[CARPOS_RIGHT]->GetBody();
			pBs->coach = CARFORM_COACHR;
			m_pCar[CARPOS_RIGHT]->SetBody(*pBs);
		}
		else {
			m_pPanel->InterLog(L"Activate 0\n");		// -1:N
			m_pCabin[0]->Activate(CABINCMD_NON);
			m_pPanel->InterLog(L"Activate 1\n");
			m_pCabin[1]->Activate(CABINCMD_NON);
			m_pPanel->InterLog(L"SetBody 0\n");
			PBODYSTYLE pBs = m_pCar[CARPOS_LEFT]->GetBody();
			pBs->coach = CARFORM_COACHN;
			m_pCar[CARPOS_LEFT]->SetBody(*pBs);
			m_pPanel->InterLog(L"SetBody 1\n");
			pBs = m_pCar[CARPOS_RIGHT]->GetBody();
			pBs->coach = CARFORM_COACHN;
			m_pCar[CARPOS_RIGHT]->SetBody(*pBs);
		}
	}
	//clock_t end = clock();
	//double dur = (double)(end - begin) / CLOCKS_PER_SEC;
	//CString str;
	//str.Format(L"RudderCtrl %f.\n", dur);
	//Log(str);

	return 0;
}

LRESULT CIntro::OnConnectDio(WPARAM wParam, LPARAM lParam)
{
	Log(L"[INTRO]DIO Connect OK.\r\n");
	//for (int n = 0; n < 2; n ++)	m_pDato[n]->Ready();
	for (int n = 0; n < 2; n ++)	m_pCabin[n]->Ready();
	for (int n = 0; n < CARPOS_MAX; n ++)	m_pCar[n]->Ready();
	Sleep(100);

	for (int n = 0; n < 2; n ++)	m_pCabin[n]->Ready();
	for (int n = 0; n < CARPOS_MAX; n ++)	m_pCar[n]->Ready();

	Sleep(100);
	for (int n = 0; n < 2; n ++)	m_pCabin[n]->Ready();
	for (int n = 0; n < CARPOS_MAX; n ++)	m_pCar[n]->Ready();

	return 0;
}

LRESULT CIntro::OnConnectScu(WPARAM wParam, LPARAM lParam)
{
	int id = (int)wParam;
	if (id > 2 && id < 13)	m_pCar[id - 3]->SetupGates();
	else if (id == 2 || id == 13)	m_pCabin[id == 2 ? 0 : 1]->SetupGates();
	//CString str;
	//str.Format(L"[INTRO]SCU #%d Connect OK.\r\n", id - 2);
	//Log(str);
	return 0;
}

LRESULT CIntro::OnAccept(WPARAM wParam, LPARAM lParam)
{
	CClientSocket* pClientSocket = (CClientSocket*)lParam;
	CTcm* pTcm = new CTcm(this);
	pTcm->Create(IDD_DIALOG_TCM, this);
	pTcm->ShowWindow(SW_SHOW);
	pTcm->m_pClientSocket = pClientSocket;
	pTcm->m_pClientSocket->SetHwnd(pTcm->GetSafeHwnd());
	m_tcmes.AddTail(pTcm);

	return 0;
}

LRESULT CIntro::OnInformTcm(WPARAM wParam, LPARAM lParam)
{
	int nTuID = (int)LOBYTE((WORD)wParam);
	int nItem = (int)HIBYTE((WORD)wParam);
	variant_t var;
	switch (nItem) {
	case INFTCM_LINK :
		if (nTuID >= 100 && nTuID <= 103) {
			var.vt = VT_BOOL;
			var.boolVal = TRUE;
			m_cook.TcmChange(nTuID, INFTCM_LINK, var);
		}
		break;
	case INFTCM_OBLIGE :
		if (nTuID >= 100 && nTuID <= 103) {
			var.vt = VT_UI2;
			var.uiVal = (USHORT)lParam;
			m_cook.TcmChange(nTuID, INFTCM_OBLIGE, var);

			WORD wOblige = (WORD)lParam;
			CString str;
			str.Format(L"TU%03d has %04X\r\n", nTuID, wOblige);
			Log(str);
		}
		break;
	case INFTCM_SPEED :
		if (nTuID >= 100 && nTuID <= 103) {
			var.vt = VT_UI4;
			var.ulVal = (DWORD)lParam;
			m_cook.TcmChange(nTuID, INFTCM_SPEED, var);
		}
	case INFTCM_DISTAN :
		if (nTuID >= 100 && nTuID <= 103) {
			var.vt = VT_UI4;
			var.ulVal = (DWORD)lParam;
			m_cook.TcmChange(nTuID, INFTCM_DISTAN, var);
		}
		break;
	default :	break;
	}

	return 0;
}

LRESULT CIntro::OnCloseTcm(WPARAM wParam, LPARAM lParam)
{
	int nTuID = (int)wParam;
	if (nTuID >= 100 && nTuID <= 103) {
		variant_t var;
		var.vt = VT_BOOL;
		var.boolVal = FALSE;
		m_cook.TcmChange(nTuID, INFTCM_LINK, var);
	}
	CTcm* pClsTcm = (CTcm*)lParam;
	POSITION posf;
	for (POSITION pos = m_tcmes.GetHeadPosition(); (posf = pos) != NULL; ) {
		CTcm* pTcm = (CTcm*)m_tcmes.GetNext(pos);
		if (pTcm == pClsTcm) {
			delete pTcm;
			pTcm = NULL;
			m_tcmes.RemoveAt(posf);
			break;
		}
	}
	return 0;
}
