// Tcm.cpp
#include "stdafx.h"
#include "Dts.h"
#include "Tcm.h"
#include "afxdialogex.h"

#include "Tools.h"

IMPLEMENT_DYNAMIC(CTcm, CDialogEx)

CTcm::CTcm(CWnd* pParent /*=NULL*/)
	: CDialogEx(CTcm::IDD, pParent)
{
	m_pParent = pParent;
	m_pClientSocket = NULL;
	m_nID = -1;
	m_wSpeed = 0;
	m_dwDistan = 0;
}

CTcm::~CTcm()
{
	if (m_pClientSocket != NULL) {
		m_pClientSocket->Close();
		delete m_pClientSocket;
		m_pClientSocket = NULL;
	}
}

void CTcm::Log(CString strLog, COLORREF crText)
{
	if (strLog == L"")	return;

	if (m_log.GetSafeHwnd()) {
		CHARFORMAT cf, cfb;
		m_log.GetDefaultCharFormat(cf);
		memcpy(&cfb, &cf, sizeof(CHARFORMAT));
		cf.dwMask = CFM_COLOR;
		cf.dwEffects = NULL;
		cf.crTextColor = crText;
		m_log.SetWordCharFormat(cf);
		m_log.SetSel(-1, -1);
		m_log.ReplaceSel(strLog);
		m_log.SetDefaultCharFormat(cfb);
	}
}


void CTcm::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT_LOG, m_log);
}

BEGIN_MESSAGE_MAP(CTcm, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_DEV1, &CTcm::OnClickedButtonClear)
	ON_BN_CLICKED(IDC_BUTTON_DEV0, &CTcm::OnClickedButtonHide)
	ON_WM_TIMER()
	ON_MESSAGE(WM_SCONNECT, &CTcm::OnConnect)
	ON_MESSAGE(WM_SRECEIVE, &CTcm::OnReceive)
	ON_MESSAGE(WM_SCLOSE, &CTcm::OnClose)
END_MESSAGE_MAP()

BOOL CTcm::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	return TRUE;
}

BOOL CTcm::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
		return TRUE;
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CTcm::OnTimer(UINT_PTR nIDEvent)
{
	CDialogEx::OnTimer(nIDEvent);
}

void CTcm::OnClickedButtonClear()
{
	m_log.SetSel(0, -1);
	m_log.Clear();
}

void CTcm::OnClickedButtonHide()
{
	ShowWindow(SW_HIDE);
}

LRESULT CTcm::OnConnect(WPARAM wParam, LPARAM lParam)
{
	// int nID = (int)wParam;
	if (m_pClientSocket != NULL) {
		CString strPeer;
		UINT nPeer;
		m_pClientSocket->GetPeerName(strPeer, nPeer);
		CString str;
		str.Format(L"Connect to %s[%d].\r\n", strPeer, nPeer);
		Log(str);
	}
	return 0;
}

LRESULT CTcm::OnReceive(WPARAM wParam, LPARAM lParam)
{
	// int nID = (int)wParam;
	if (m_pClientSocket != NULL) {
		BYTE temp[4096];
		int nRec = m_pClientSocket->Receive(temp, 4096);
		temp[nRec] = 0;
		CString str = CString(temp);
		str += L"\r\n";
		//Log(str, RGB(0, 0, 255));
		int n;
		if ((n = str.Find(L"IP=")) >= 0) {
			CString strTemp = str.Mid(n + strlen("IP="));
			SetWindowTextW(strTemp);
			for (int m = 0; m < 3; m ++) {
				n = strTemp.FindOneOf(L".");
				if (n > 0)	strTemp = strTemp.Mid(n + 1);
			}
			if (!strTemp.IsEmpty())	m_nID = atoi((CT2CA)strTemp);
			if (m_pParent != NULL)	m_pParent->SendMessage(WM_INFORMTCM, (WPARAM)MAKEWORD(m_nID, INFTCM_LINK), (LPARAM)0);
			str.Format(L"ID is %d\r\n", m_nID);
			Log(str, RGB(0, 0, 255));
		}
		else if ((n = str.Find(L"OB=")) >= 0) {
			CString strTemp = str.Mid(n + strlen("OB="));
			GETTOOLS(pTools);
			WORD w = pTools->ahtoi(strTemp);
			if (m_pParent != NULL)	m_pParent->SendMessage(WM_INFORMTCM, (WPARAM)MAKEWORD(m_nID, INFTCM_OBLIGE), (LPARAM)w);
			Log(str, RGB(0, 0, 255));
		}
		else if ((n = str.Find(L"SP=")) >= 0) {
			CString strTemp = str.Mid(n + strlen("SP="));
			WORD w = (WORD)atoi((CT2CA)strTemp);
			if (m_pParent != NULL)	m_pParent->SendMessage(WM_INFORMTCM, (WPARAM)MAKEWORD(m_nID, INFTCM_SPEED), (LPARAM)w);
		}
		else if ((n = str.Find(L"DT=")) >= 0) {
			CString strTemp = str.Mid(n + strlen("DT="));
			DWORD dw = (DWORD)atol((CT2CA)strTemp);
			if (m_pParent != NULL)	m_pParent->SendMessage(WM_INFORMTCM, (WPARAM)MAKEWORD(m_nID, INFTCM_DISTAN), (LPARAM)dw);
		}
	}
	return 0;
}

LRESULT CTcm::OnClose(WPARAM wParam, LPARAM lParam)
{
	// int nID = (int)wParam;
	if (m_pClientSocket != NULL) {
		CString strPeer;
		UINT nPeer;
		m_pClientSocket->GetPeerName(strPeer, nPeer);
		CString str;
		str.Format(L"Close to %s[%d].\r\n", strPeer, nPeer);
		Log(str);
		m_pClientSocket->Close();
		delete m_pClientSocket;
		m_pClientSocket = NULL;
		if (m_pParent != NULL)	m_pParent->SendMessage(WM_CLOSETCM, (WPARAM)m_nID, (LPARAM)this);
	}
	return 0;
}
