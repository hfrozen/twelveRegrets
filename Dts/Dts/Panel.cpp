// Panel.cpp
#include "stdafx.h"
#include "Dts.h"
#include "Panel.h"
#include "afxdialogex.h"

#include "Intro.h"

#define	SOCKETSCAN_TIMER	2
#define	TIME_SOCKETSCAN		500

IMPLEMENT_DYNAMIC(CPanel, CDialogEx)

PCWSTR CPanel::m_pstrPanelNames[] = {
	L"TC0", L"CC0", L"CC1", L"CC2",
	L"CC3", L"CC4", L"CC5", L"CC6",
	L"CC7", L"CC8", L"CC9", L"TC9",
};

const int CPanel::m_nDioMap[2][MAX_LEDS] = {
	{
		7,		6,		5,		4,		3,		2,		1,		0,		// 0, output #0
		15,		14,		13,		12,		11,		10,		9,		8,		// 8
		23,		22,		21,		20,		19,		18,		17,		16,		// 16
		107,	106,	105,	104,	103,	102,	101,	100,	// 24, output #1
		115,	114,	113,	112,	111,	110,	109,	108,	// 32
		123,	122,	121,	120,	119,	118,	117,	116,	// 40
		410,	411,	408,	409,	-2,		-2,		-2,		-2,		// 423,	422,	421,	420,	// 48
		-2,		-2,		-2,		-2,		-2,		-2,		-2,		-2		// 56
	},
	{
		207,	206,	205,	204,	203,	202,	201,	200,	// 0, output #2
		223,	222,	221,	220,	219,	218,	217,	216,	// 8
		215,	214,	213,	212,	211,	210,	209,	208,	// 23
		307,	306,	305,	304,	303,	302,	301,	300,	// 24, output #3
		315,	314,	313,	312,	311,	310,	309,	308,	// 32
		323,	322,	321,	320,	319,	318,	317,	316,	// 40
		-2,		-2,		-2,		-2,		-2,		-2,		-2,		-2,		// 48
		-2,		-2,		-2,		-2,		-2,		-2,		-2,		-2		// 56
	}
};

#if	!defined(_SIMM_SOCKET_)
const CPanel::IPPORT CPanel::m_ips[] = {
/* 00 */	{	L"1F",	L"193.168.0.50",	5000	},	// 1f
/* 01 */	{	L"2F",	L"193.168.0.51",	5000	},	// 2f
/* 02 */	{	L"3F0",	L"193.168.0.53",	5000	},	// 3f0, tc0
/* 03 */	{	L"3F1",	L"193.168.0.54",	5000	},	// 3f1, cc0
/* 04 */	{	L"3F2",	L"193.168.0.55",	5000	},	// 3f2, cc1
/* 05 */	{	L"3F3",	L"193.168.0.56",	5000	},	// 3f3, cc2
/* 06 */	{	L"3F4",	L"193.168.0.57",	5000	},	// 3f4, cc3
/* 07 */	{	L"3F5",	L"193.168.0.58",	5000	},	// 3f5, cc4
/* 08 */	{	L"3F6",	L"193.168.0.59",	5000	},	// 3f6, cc5
/* 09 */	{	L"3F7",	L"193.168.0.60",	5000	},	// 3f7, cc6
/* 10 */	{	L"3F8",	L"193.168.0.61",	5000	},	// 3f8, cc7
/* 11 */	{	L"3F9",	L"193.168.0.62",	5000	},	// 3f9, cc8
/* 12 */	{	L"3FA",	L"193.168.0.63",	5000	},	// 3fa, cc9
/* 13 */	{	L"3FB",	L"193.168.0.64",	5000	},	// 3fb, tc9
/* 14 */	{	L"3FC",	L"193.168.0.65",	5000	},	// 3fc, tc0-ato
/* 15 */	{	L"3FD",	L"193.168.0.66",	5000	}	// 3fd, tc9-ato
};
#else
const CPanel::IPPORT CPanel::m_ips[] = {
/* 00 */	{	L"1F",	L"127.0.0.1",	5000	},
/* 01 */	{	L"2F",	L"127.0.0.1",	5000	},
/* 02 */	{	L"3F0",	L"127.0.0.1",	5000	},
/* 03 */	{	L"3F1",	L"127.0.0.1",	5000	},
/* 04 */	{	L"3F2",	L"127.0.0.1",	5000	},
/* 05 */	{	L"3F3",	L"127.0.0.1",	5000	},
/* 06 */	{	L"3F4",	L"127.0.0.1",	5000	},
/* 07 */	{	L"3F5",	L"127.0.0.1",	5000	},
/* 08 */	{	L"3F6",	L"127.0.0.1",	5000	},
/* 09 */	{	L"3F7",	L"127.0.0.1",	5000	},
/* 10 */	{	L"3F8",	L"127.0.0.1",	5000	},
/* 11 */	{	L"3F9",	L"127.0.0.1",	5000	},
/* 12 */	{	L"3Fa",	L"127.0.0.1",	5000	},
/* 13 */	{	L"3Fb",	L"127.0.0.1",	5000	}
/* 14 */	{	L"3Fc",	L"127.0.0.1",	5000	},
/* 15 */	{	L"3Fd",	L"127.0.0.1",	5000	}
};
#endif

CPanel::CPanel(CWnd* pParent /*=NULL*/)
	: CDialogEx(CPanel::IDD, pParent)
{
	m_pParent = pParent;
	for (int n = 0; n < MAX_PANELS; n ++)	m_pLeds[n] = NULL;
	for (int n = 0; n < 10; n ++)	m_cDies[n] = 0;
	for (int n = 0; n < MAX_FLOORS; n ++) {
		m_floor[n].pSock = NULL;
		m_floor[n].bConnect = false;
		m_floor[n].bInitial = false;
		m_floor[n].nRi = 0;
		m_floor[n].nConCycle = 0;
		ZeroMemory(m_floor[n].sRbuf, SIZE_SOCKRECVBUFFER);
	}
#if	defined(TCPSEQOPEN)
	m_nConnSeq = 0;
#endif
	m_bAlarmMsg = true;
}

CPanel::~CPanel()
{
	for (int n = 0; n < MAX_PANELS; n ++) {
		KILL(m_pLeds[n]);
	}
	for (int n = 0; n < MAX_FLOORS; n ++) {
		if (m_floor[n].pSock != NULL) {
			if (m_floor[n].bConnect) {
				m_floor[n].pSock->Close();
				m_floor[n].bConnect = false;
			}
			delete m_floor[n].pSock;
			m_floor[n].pSock = NULL;
		}
	}
}

void CPanel::Log(CString strLog, COLORREF crText)
{
	CIntro* pIntro = (CIntro*)m_pParent;
	ASSERT(pIntro);
	pIntro->Log(strLog, crText);
}

void CPanel::InterLog(CString strLog, COLORREF crText)
{
	if (strLog == L"")	return;

	//COleDateTime dt = COleDateTime::GetCurrentTime();
	//CString str = dt.Format(L"%H:%M:%S ");
	//str += strLog;

	if (m_editLog.GetSafeHwnd()) {
		CHARFORMAT cf;	//, cfb;
		m_editLog.GetDefaultCharFormat(cf);
		//memcpy(&cfb, &cf, sizeof(CHARFORMAT));
		cf.dwMask = CFM_COLOR;
		cf.dwEffects = NULL;
		cf.crTextColor = crText;
		m_editLog.SetWordCharFormat(cf);
		m_editLog.SetSel(-1, -1);
		//m_editLog.ReplaceSel(str);
		m_editLog.ReplaceSel(strLog);
		//m_editLog.SetDefaultCharFormat(cfb);
	}
	CIntro* pIntro = (CIntro*)m_pParent;
	ASSERT(pIntro);
	pIntro->Logp(strLog, crText);
}

void CPanel::InterLog(int id, BYTE* pBuf, int leng, COLORREF crText)
{
	CString str;
	str.Format(L"%02df", id);
	for (int n = 0; n < leng; n ++) {
		CString strTemp;
		if (*pBuf < 0xff)	strTemp.Format(L"-%02X", *pBuf ++);
		else {
			strTemp = L"-FF";
			++ pBuf;
		}
		str += strTemp;
	}
	str += L"\r\n";
	InterLog(str, crText);
}

void CPanel::GetDies(CString strDies)
{
	if (strDies.GetLength() < 7)	return;
	CString str = strDies.Left(2);
	strDies = strDies.Mid(2);
	int cid = atoi((CT2CA)str);
	if (cid < 0 || cid > 3)	return;
	str = strDies.Left(2);
	strDies = strDies.Mid(2);
	int sid = atoi((CT2CA)str);
	if (sid < 0 || sid > 2)	return;
	BYTE ch = (BYTE)atoi((CT2CA)strDies);
	cid = cid * 3 + sid;
	if (cid > 9)	return;
	if (m_cDies[cid] != ch) {
		for (int n = 0; n < 8; n ++) {
			if ((m_cDies[cid] & (1 << n)) != (ch & (1 << n)))
				m_pLeds[cid + 1]->BitCtrl(56 + n, (ch & (1 << n)) ? true : false);
		}
		m_cDies[cid] = ch;
		CIntro* pIntro = (CIntro*)m_pParent;
		pIntro->DistributeDies(cid, m_cDies[cid]);
	}
}

int CPanel::GetDioMap(UINT cid, UINT sid)
{
	return m_nDioMap[cid > 0 && cid < 11 ? 1 : 0][sid];
}

void CPanel::UnitClr(UINT cid)
{
	int id;
	if (cid == CARPOS_MAX)	id = 0;
	else if (cid > CARPOS_MAX)	id = 11;
	else	id = cid + 1;
	m_pLeds[id]->UnitClr();

	int floor;
	int bid;
	if (cid > CARPOS_LEFT && cid < CARPOS_RIGHT) {
		bid = (cid - CARPOS_M1L) * 2;
		floor = 1;
	}
	else {
		if (cid >= CARPOS_MAX)	bid = cid == CARPOS_MAX ? 0 : 4;
		else	bid = cid == CARPOS_LEFT ? 2 : 6;
		floor = 0;
	}
	SendToSocket(floor, 6, 5, 0x33, 0x50 | bid, 0, 0, 0);
	SendToSocket(floor, 6, 5, 0x33, 0x50 | bid + 1, 0, 0, 0);
}

void CPanel::BitCtrl(UINT cid, UINT sid, bool bCtrl)
{
	//char cBuf[256];
	int id;
	if (cid == CARPOS_MAX)	id = 0;
	else if (cid > CARPOS_MAX)	id = 11;
	else	id = cid + 1;
	m_pLeds[id]->BitCtrl(sid, bCtrl);

	CString str;
	id = m_nDioMap[cid < CARPOS_MAX ? 1 : 0][sid];	// cc has 2XX
	if (id < 0) {
		if (m_bAlarmMsg) {
			str.Format(L"[PAN]Bit control error %d at car#%d!\r\n", sid, cid);
			Log(str, COLOR_RED);
		}
		return;
	}
	int floor;
	int bid = id / 100;		// cc has 2XX
	id %= 100;
	if (bid >= 4) {
		if (id == 0) {	// 400 for Xn030
			if (cid < CARPOS_MAX)
				SendToSocket(0, 5, 4, 0x34, 0x58, cid, bCtrl ? 1 : 0);
			else {
				if (m_bAlarmMsg) {
					str.Format(L"[PAN]Bit control error 4%02d at car#%d!\r\n", id, cid);
					Log(str, COLOR_RED);
				}
			}
		}
		else if (id >= 8 && id <= 15) {
			if (cid == CARPOS_MAX || cid == (CARPOS_MAX + 1)) {
				SendToSocket(0, 5, 4, 0x34, 0x58, cid == CARPOS_MAX ? id : id + 8, bCtrl ? 1 : 0);
			}
			else {
				if (m_bAlarmMsg) {
					str.Format(L"[PAN]Bit control error 4%02d at car#%d!\r\n", id, cid);
					Log(str, COLOR_RED);
				}
			}
		}
		else {
			if (m_bAlarmMsg) {
				str.Format(L"[PAN]Bit control error 4%02d at car#%d!\r\n", id, cid);
				Log(str, COLOR_RED);
			}
		}
	}
	else {
		if (cid > CARPOS_LEFT && cid < CARPOS_RIGHT) {
			if (bid < 2) {
				str.Format(L"[PAN]Bit control error %03d at CC#%d!\r\n", bid, cid);
				Log(str, COLOR_RED);
			}
			else {
				bid = (cid - CARPOS_M1L) * 2 + bid - 2;
				floor = 1;
			}
		}
		else {
			if (cid == CARPOS_RIGHT || cid > CARPOS_MAX)	bid += 4;
			floor = 0;
		}
		SendToSocket(floor, 5, 4, 0x34, 0x50 | bid, id, bCtrl ? 1 : 0);
	}
}

bool CPanel::GetBit(UINT cid, UINT sid)
{
	int id;
	if (cid == CARPOS_MAX)	id = 0;
	else if (cid > CARPOS_MAX)	id = 11;
	else	id = cid + 1;
	return m_pLeds[id]->GetBit(sid);
}

void CPanel::PwmCtrl(UINT cid, UINT sid, UINT uPwm)
{
	if (cid != CARPOS_MAX && cid != (CARPOS_MAX + 1))	return;
	if (sid > 4)	return;
	SendToSocket(0, 6, 5, 0x43, 0x50 | (cid - CARPOS_MAX), sid, HIBYTE(uPwm), LOBYTE(uPwm));

	if (sid == PWMCH_TACHO) {
		CIntro* pIntro = (CIntro*)m_pParent;
		ASSERT(pIntro);
		pIntro->SetTachoText(cid - CARPOS_MAX, uPwm);
	}

	CString str;
	str.Format(L"PWM %#X\n", uPwm);
	Log(str);
}

#if	defined(TCPSEQOPEN)
void CPanel::Connect()
{
	CString str;
	while (m_nConnSeq < MAX_FLOORS) {
		if (m_floor[m_nConnSeq].pSock != NULL) {
			m_floor[m_nConnSeq].pSock->Close();
			delete m_floor[m_nConnSeq].pSock;
		}
		m_floor[m_nConnSeq].pSock = new CClientSocket;
		if (m_floor[m_nConnSeq].pSock->Create()) {
			BOOL bOpt = true;
			m_floor[m_nConnSeq].pSock->SetSockOpt(TCP_NODELAY, &bOpt, sizeof(BOOL), IPPROTO_TCP);
			m_floor[m_nConnSeq].pSock->SetHwnd(GetSafeHwnd());
			m_floor[m_nConnSeq].pSock->SetID(m_nConnSeq);
			m_floor[m_nConnSeq].nConCycle = 0;
			if (!m_floor[m_nConnSeq].pSock->Connect(m_ips[m_nConnSeq].strIp, m_ips[m_nConnSeq].nPort)) {
				int nError = m_floor[m_nConnSeq].pSock->GetLastError();
				if (nError != WSAEWOULDBLOCK) {
					m_floor[m_nConnSeq].pSock->Close();
					delete m_floor[m_nConnSeq].pSock;
					m_floor[m_nConnSeq].pSock = NULL;
					str.Format(L"[PAN]can not connnect to %s, %s[%d]!\r\n",
						m_ips[m_nConnSeq].strName, m_ips[m_nConnSeq].strIp, m_ips[m_nConnSeq].nPort);
					Log(str, COLOR_RED);
					++ m_nConnSeq;
				}
				else {	// block... wait OnConnect()
					//str.Format(L"[PAN]block to %s, %s[%d]!\r\n",
					//	m_ips[m_nConnSeq].strName, m_ips[m_nConnSeq].strIp, m_ips[m_nConnSeq].nPort);
					//Log(str);
					return;
				}
			}
			else {		// connected
				//m_floor[m_nConnSeq].bInitial = false;
				//m_floor[m_nConnSeq].bConnect = true;
				//CString strPeer;
				//UINT nPeer;
				//m_floor[m_nConnSeq].pSock->GetPeerName(strPeer, nPeer);
				//str.Format(L"[PAN]connect to %s, %s[%d].\r\n", m_ips[m_nConnSeq].strName, strPeer, nPeer);
				//Log(str);
				return;
			}
		}
		else {		// create failure
			delete m_floor[m_nConnSeq].pSock;
			m_floor[m_nConnSeq].pSock = NULL;
			str.Format(L"[PAN]sock #%d create failed!\r\n", m_nConnSeq);
		}
		++ m_nConnSeq;
	}
}

void CPanel::Open()
{
	m_nConnSeq = 0;
	SetTimer(SOCKETSCAN_TIMER, TIME_SOCKETSCAN, NULL);
	ConnectSeq();
}

#else
bool CPanel::Connect(int nID)
{
	if (!m_floor[nID].pSock->Connect(m_ips[nID].strIp, m_ips[nID].nPort)) {
		int nError = m_floor[nID].pSock->GetLastError();
		if (nError != WSAEWOULDBLOCK) {
			m_floor[nID].pSock->Close();
			delete m_floor[nID].pSock;
			m_floor[nID].pSock = NULL;
			CString str;
			str.Format(L"[PAN]Can not connect to %s, %s[%d]!\r\n", m_ips[nID].strName, m_ips[nID].strIp, m_ips[nID].nPort);
			Log(str, COLOR_RED);
			return false;
		}
		else {	// block
		}
	}
	else {	// connected
		m_floor[nID].bInitial = false;
		m_floor[nID].bConnect = true;
		CString strPeer;
		UINT nPeer;
		m_floor[nID].pSock->GetPeerName(strPeer, nPeer);
		CString str;
		str.Format(L"[PAN]Connect to %s, %s[%d].\r\n", m_ips[nID].strName, strPeer, nPeer);
		Log(str);
	}
	return true;
}

void CPanel::Open()
{
	CString str;
	for (int n = 0; n < MAX_FLOORS; n ++) {
		if (m_floor[n].pSock != NULL) {
			m_floor[n].pSock->Close();
			delete m_floor[n].pSock;
		}
		m_floor[n].pSock = new CClientSocket;
		if (m_floor[n].pSock->Create()) {
			BOOL bOpt = TRUE;
			m_floor[n].pSock->SetSockOpt(TCP_NODELAY, &bOpt, sizeof(BOOL), IPPROTO_TCP);
			m_floor[n].pSock->SetHwnd(GetSafeHwnd());
			m_floor[n].pSock->SetID(n);
			m_floor[n].nConCycle = 0;
			Connect(n);
		}
		else {	// create fail
			delete m_floor[n].pSock;
			m_floor[n].pSock = NULL;
			str.Format(L"[PAN]Socket #%d create failed!\r\n", n);
		}
#if	defined(_SIMM_SOCKET_)
		Sleep(100);
#else
		Sleep(100);
#endif
	}
	SetTimer(SOCKETSCAN_TIMER, TIME_SOCKETSCAN, NULL);
}
#endif

void CPanel::Close()
{
	for (int n = 0; n < MAX_FLOORS; n ++) {
		if (m_floor[n].pSock != NULL && m_floor[n].bConnect) {
			m_floor[n].pSock->Close();
			m_floor[n].bConnect = false;
		}
	}
}

void CPanel::SendToSocket(int id, char* pBuf, int leng)
{
	if (id < MAX_FLOORS && m_floor[id].pSock != NULL && m_floor[id].bConnect) {
		m_floor[id].cs.Lock();
		m_floor[id].pSock->Send(pBuf, leng);		// !!!!!
		Sleep(1);
		m_floor[id].cs.Unlock();
	}
	CIntro* pIntro = (CIntro*)m_pParent;
	ASSERT(pIntro);
	if (pIntro->GetTxHideCheck() != BST_CHECKED)	InterLog(id, (BYTE*)pBuf, leng);
}

void CPanel::SendToSocket(int id, CString strSend)
{
	if (id < MAX_FLOORS && m_floor[id].pSock != NULL && m_floor[id].bConnect) {
		int leng = strSend.GetLength();
		//int leng = WideCharToMultiByte(CP_ACP, 0, strSend, -1, NULL, 0, NULL, NULL);
		char* pBuf = new char[leng];
		WideCharToMultiByte(CP_ACP, 0, strSend, -1, pBuf, leng, NULL, NULL);
		SendToSocket(id, pBuf, leng);
		delete [] pBuf;
	}
	//InterLog(strSend);
}

void CPanel::SendToSocket(int id, int leng, ...)
{
	if (id >= MAX_FLOORS || m_floor[id].pSock == NULL || !m_floor[id].bConnect)	return;

	va_list vargs;
	va_start(vargs, leng);
	SendToSocket(id, leng, vargs);
	va_end(vargs);
}

void CPanel::SendToSocket(int id, int leng, va_list vargs)
{
	if (id >= MAX_FLOORS || m_floor[id].pSock == NULL || !m_floor[id].bConnect)	return;

	for (int n = 0; n < leng; n ++)
		m_floor[id].sSbuf[n] = va_arg(vargs, char);
	SendToSocket(id, m_floor[id].sSbuf, leng);
	//InterLog(id, (BYTE*)m_floor[id].sSbuf, leng);
}

void CPanel::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT_LOG, m_editLog);
	DDX_Control(pDX, IDC_BUTTON_DEV0, m_btnClear);
	DDX_Control(pDX, IDC_BUTTON_DEV1, m_btnExit);
}

BEGIN_MESSAGE_MAP(CPanel, CDialogEx)
	ON_WM_TIMER()
	ON_MESSAGE(WM_SCONNECT, &CPanel::OnConnect)
	ON_MESSAGE(WM_SRECEIVE, &CPanel::OnReceive)
	ON_MESSAGE(WM_SCLOSE, &CPanel::OnClose)
	ON_MESSAGE(WM_CLICKLED, &CPanel::OnClickLed)
END_MESSAGE_MAP()

BOOL CPanel::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	int cx = 0;
	CRect rc;
	for (int n = 0; n < MAX_PANELS; n ++) {
		if (m_pLeds[n] != NULL)	delete m_pLeds[0];
		m_pLeds[n] = new CLeds(this);
		m_pLeds[n]->SetCID(n);
		m_pLeds[n]->Create(IDD_DIALOG_LEDI, this);
		m_pLeds[n]->SetName(m_pstrPanelNames[n]);
		m_pLeds[n]->SetWindowPos(NULL, cx, 0, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
		m_pLeds[n]->GetWindowRect(&rc);
		cx += (rc.Width() - 1);
		m_pLeds[n]->ShowWindow(SW_SHOW);
	}
	CRect rcb;
	m_btnClear.GetWindowRect(&rcb);
	CRect rce;
	m_editLog.GetWindowRect(&rce);
	m_editLog.SetWindowPos(NULL, cx + 2, 0, rce.Width(), rc.Height() - rcb.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	//m_editLog.SetOptions(ECOOP_XOR, ECO_SAVESEL);
	m_btnClear.SetWindowPos(NULL, cx + 2, rc.Height() - rcb.Height(), 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
	m_btnExit.SetWindowPos(NULL, cx + rcb.Width() + 4, rc.Height() - rcb.Height(), 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
	SetWindowPos(NULL, 0, 0, cx + rce.Width() + 7 + 2, rc.Height() + 28, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);

#if	defined(_SIMM_SOCKET_)
	m_editLog.SetReadOnly(false);
#endif

	return TRUE;
}

BOOL CPanel::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)) {
#if	defined(_SIMM_SOCKET_)
		if (pMsg->wParam == VK_RETURN) {
			CWnd* pWnd = GetFocus();
			HWND hWnd = pWnd->GetSafeHwnd();
			if (hWnd == m_editLog.GetSafeHwnd()) {
				int count = m_editLog.GetLineCount();
				int ls = count > 0 ? m_editLog.LineIndex(count - 1) : 0;
				m_editLog.SetSel(ls, -1);
				CString str = m_editLog.GetSelText();
				CString strTemp = L"\r\n";
				m_editLog.SetSel(-1, -1);
				m_editLog.ReplaceSel(strTemp);
				if (str.GetLength() == 8 && str.GetAt(0) == (TCHAR)'G') {
					str = str.Mid(1);
					GetDies(str);
				}
			}
		}
#endif
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

BOOL CPanel::OnCommand(WPARAM wParam, LPARAM lParam)
{
	int wNotifyMsg = (int)HIWORD(wParam);
	UINT id = (UINT)LOWORD(wParam);
	CIntro* pIntro = (CIntro*)m_pParent;
	if (wNotifyMsg == CBN_SELCHANGE) {
	}
	else if (wNotifyMsg == EN_KILLFOCUS) {
	}
	else {
		if (id == IDC_BUTTON_DEV0) {
#if	!defined(_SIMM_SOCKET_)
			m_editLog.SetReadOnly(false);
#endif
			m_editLog.SetSel(0, -1);
			m_editLog.Clear();
#if	!defined(_SIMM_SOCKET_)
			m_editLog.SetReadOnly();
#endif
		}
		else if (id == IDC_BUTTON_DEV1) {
			ShowWindow(SW_HIDE);
		}
	}
	return CDialogEx::OnCommand(wParam, lParam);
}

void CPanel::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == SOCKETSCAN_TIMER) {
		if (m_floor[0].bConnect && m_floor[1].bConnect &&
			(!m_floor[0].bInitial || !m_floor[1].bInitial)) {
			if (m_pParent != NULL)	m_pParent->SendMessage(WM_CONNECTDIO);
			m_floor[0].bInitial = m_floor[1].bInitial = true;
		}
		int n;
		for (n = 2; n < MAX_FLOORS; n ++) {
			if (m_floor[n].bConnect && !m_floor[n].bInitial) {
				if (m_pParent != NULL)	m_pParent->SendMessage(WM_CONNECTSCU, (WPARAM)n);
				m_floor[n].bInitial = true;
			}
		}
		for (n = 0; n < MAX_FLOORS; n ++) {
			if (!m_floor[n].bInitial)	break;
		}
		if (n >= MAX_FLOORS)	KillTimer(SOCKETSCAN_TIMER);
	}
	CDialogEx::OnTimer(nIDEvent);
}

#if	defined(TCPSEQOPEN)
LRESULT CPanel::OnConnect(WPARAM wParam, LPARAM lParam)
{
	CString str;
	//int nID = (int)wParam;
	int nError = (int)lParam;
	if (nError == 0) {
		m_floor[m_nConnSeq].bInitial = false;
		m_floor[m_nConnSeq].bConnect = true;
		CString strPeer;
		UINT nPeer;
		m_floor[m_nConnSeq].pSock->GetPeerName(strPeer, nPeer);
		str.Format(L"[PAN]connect to %s, %s[%d] by msg.\r\n", m_ips[m_nConnSeq].strName, strPeer, nPeer);
		Log(str);
		char cBuf[16];
		cBuf[0] = cBuf[1] = 1;
		SendToSocket(m_nConnSeq, cBuf, 2);
	}
	else {
		str.Format(L"[PAN]connect fail %d to %s!\r\n", nError, m_ips[m_nConnSeq].strName);
		Log(str);
	}
	++ m_nConnSeq;
	ConnectSeq();

	return 0;
}
#else
LRESULT CPanel::OnConnect(WPARAM wParam, LPARAM lParam)
{
	int nID = (int)wParam;
	int nError = (int)lParam;
	if (nError == 0) {
		//CClientSocket* pSock = (CClientSocket*)lParam;
		//m_cs.Lock();
		m_floor[nID].bInitial = false;
		m_floor[nID].bConnect = true;
		CString strPeer;
		UINT nPeer;
		m_floor[nID].pSock->GetPeerName(strPeer, nPeer);
		CString str;
		str.Format(L"[PAN]Connect to %s, %s[%d] by msg.\r\n", m_ips[nID].strName, strPeer, nPeer);
		Log(str);
		char cBuf[16];
		cBuf[0] = cBuf[1] = 1;
		SendToSocket(nID, cBuf, 2);
		//m_cs.Unlock();
	}
	else if (nError == WSAETIMEDOUT) {
		if (m_floor[nID].nConCycle ++ < CYCLE_CONNECT)	Connect(nID);
		else {
			CString str;
			str.Format(L"[PAN]Connect timeout to %s.\r\n", m_ips[nID].strName);
			Log(str);
		}
	}
	else {
		CString str;
		str.Format(L"[PAN]Connect fail to %s.\r\n", m_ips[nID].strName);
		Log(str);
	}

	return 0;
}
#endif

LRESULT CPanel::OnReceive(WPARAM wParam, LPARAM lParam)
{
	int nID = (int)wParam;
	if (nID < 0 || nID >= MAX_FLOORS || m_floor[nID].pSock == NULL)	return 0;

	int nRead = m_floor[nID].pSock->Receive(&m_floor[nID].sRbuf[m_floor[nID].nRi], SIZE_SOCKRECVBUFFER - m_floor[nID].nRi);
	int nTotal = m_floor[nID].nRi + nRead;
	int n = 0;
	while (n < nTotal) {
		while ((n + 9) <= nTotal && m_floor[nID].sRbuf[n] == 'C' && m_floor[nID].sRbuf[n + 1] == 'H' &&
			m_floor[nID].sRbuf[n + 4] == ' ' && m_floor[nID].sRbuf[n + 5] == 'O' && m_floor[nID].sRbuf[n + 6] == 'K' &&
			m_floor[nID].sRbuf[n + 7] == '!' ) {	//&& m_floor[nID].sRbuf[n + 8] == '\r' && m_floor[nID].sRbuf[n + 9] == 'n') {
			n += 10;
		}
		if (m_floor[nID].sRbuf[n] > 1 && m_floor[nID].sRbuf[n] < 64) {
			if (n + 1 < nTotal) {
				if (m_floor[nID].sRbuf[n + 1] > 0x10 && m_floor[nID].sRbuf[n + 1] < 0x44) {
					if (n + 2 < nTotal) {
						if ((m_floor[nID].sRbuf[n + 2] & 0xf0) == 0xa0) {
							if (m_floor[nID].sRbuf[n] + 1 <= nTotal) {
								CIntro* pIntro = (CIntro*)m_pParent;
								ASSERT(pIntro);
								if (pIntro->GetRxHideCheck() != BST_CHECKED)
									InterLog(nID, (BYTE*)&m_floor[nID].sRbuf[n], m_floor[nID].sRbuf[n] + 1, RGB(192, 0, 0));
								switch (m_floor[nID].sRbuf[n + 1]) {
								case 0x16 :
									if (nID > 1)
										pIntro->DistributeRecv(nID - 2, (BYTE*)&m_floor[nID].sRbuf[n + 2], m_floor[nID].sRbuf[n] - 3);
									break;
								case 0x21 :
									if ((nID == 0 || nID == 1) && m_floor[nID].sRbuf[n] == 5) {
										int id = (int)(m_floor[nID].sRbuf[n + 2] & 0xf) * 3;
										int vi = 5;
										for (int m = 0; m < 3; m ++) {
											if (id > CARPOS_RIGHT)	break;
											pIntro->DistributeDoes(id ++, (BYTE)m_floor[nID].sRbuf[n + vi --]);
										}
									}
									break;
								default :
									break;
								}
								n += m_floor[nID].sRbuf[n] + 1;
							}
							else {
								int m = 0;
								for ( ; n < nTotal; n ++, m ++)
									m_floor[nID].sRbuf[m] = m_floor[nID].sRbuf[n];
								m_floor[nID].nRi = m;
								return 0;
							}
						}
						else	++ n;
					}
					else {
						for (int m = 0; m < 2; m ++, n ++)
							m_floor[nID].sRbuf[m] = m_floor[nID].sRbuf[n];
						m_floor[nID].nRi = 2;
						return 0;
					}
				}
				else	++ n;
			}
			else {
				m_floor[nID].sRbuf[0] = m_floor[nID].sRbuf[n];
				m_floor[nID].nRi = 1;
				return 0;
			}
		}
		else	++ n;
	}

	m_floor[nID].nRi = 0;
	return 0;
}

LRESULT CPanel::OnClose(WPARAM wParam, LPARAM lParam)
{
	//m_cs.Lock();
	int nID = (int)wParam;
	//CClientSocket* pSock = (CClientSocket*)lParam;
	CString strPeer;
	UINT nPeer;
	m_floor[nID].pSock->GetPeerName(strPeer, nPeer);
	CString str;
	str.Format(L"[PAN]Close to %s, %s[%d].\r\n", m_ips[nID].strName, strPeer, nPeer);
	Log(str);
	m_floor[nID].pSock->Close();
	delete m_floor[nID].pSock;
	m_floor[nID].pSock = NULL;
	m_floor[nID].bConnect = false;
	m_floor[nID].bInitial = false;
	//m_cs.Unlock();

	return 0;
}

LRESULT CPanel::OnClickLed(WPARAM wParam, LPARAM lParam)
{
	UINT lid = LOWORD(wParam);
	UINT state = HIWORD(wParam);
	UINT cid = (UINT)lParam;
	if (cid < MAX_PANELS && lid <= MAX_LEDS) {
		UINT id;
		if (cid == 0)	id = CARPOS_MAX;
		else if (cid <= CARPOS_MAX)	id = cid - 1;
		else	id = cid;
		BitCtrl(id, lid, state ? false : true);
	}
	return 0;
}
