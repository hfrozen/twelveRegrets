// Node.cpp
#include "stdafx.h"
#include "MHost.h"
#include "Node.h"
#include "afxdialogex.h"

#define	RECCRLF_TIMER	2
#define	TIME_RECCRLF	50

IMPLEMENT_DYNAMIC(CNode, CDialogEx)

CNode::CNode(CWnd* pParent /*=NULL*/)
	: CDialogEx(CNode::IDD, pParent)
{
	m_pParent = pParent;
	m_pClientSock = NULL;
}

CNode::~CNode()
{
	if (m_pClientSock != NULL) {
		m_pClientSock->Close();
		delete m_pClientSock;
		m_pClientSock = NULL;
	}
}

void CNode::Log(CString strLog, COLORREF crText)
{
	if (strLog == L"")	return;

	if (m_edit.GetSafeHwnd()) {
		CHARFORMAT cf, cfb;
		m_edit.GetDefaultCharFormat(cf);
		memcpy(&cfb, &cf, sizeof(CHARFORMAT));
		cf.dwMask = CFM_COLOR;
		cf.dwEffects = NULL;
		cf.crTextColor = crText;
		m_edit.SetWordCharFormat(cf);
		m_edit.SetSel(-1, -1);
		m_edit.ReplaceSel(strLog);
		m_edit.SetDefaultCharFormat(cfb);
	}
}

BYTE CNode::ToBin(BYTE c)
{
	return (((c & 0xf0) >> 4) * 10 + (c & 0xf));
}

void CNode::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_LOG, m_edit);
}

BEGIN_MESSAGE_MAP(CNode, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_CLEAR, &CNode::OnClickedButtonClear)
	//ON_BN_CLICKED(IDC_BUTTON_SEND, &CNode::OnClickedButtonSend)
	ON_WM_TIMER()
	ON_MESSAGE(WM_SCONNECT, &CNode::OnConnect)
	ON_MESSAGE(WM_SRECEIVE, &CNode::OnReceive)
	ON_MESSAGE(WM_SCLOSE, &CNode::OnClose)
END_MESSAGE_MAP()

BOOL CNode::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	return TRUE;
}

BOOL CNode::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
		return TRUE;
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CNode::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == RECCRLF_TIMER) {
		KillTimer(RECCRLF_TIMER);
		Log(L"\r\n");
	}
	CDialogEx::OnTimer(nIDEvent);
}

void CNode::OnClickedButtonClear()
{
	m_edit.SetSel(0, -1);
	m_edit.Clear();
}

/*void CNode::OnClickedButtonSend()
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SEND);
	if (pEdit != NULL) {
		CString str;
		pEdit->GetWindowTextW(str);
		if (!str.IsEmpty()) {
			BYTE buf[128];
			ZeroMemory(buf, 128);
			int m = 0;
			BYTE c = 0;
			for (int n = 0; n < str.GetLength(); n ++) {
				TCHAR ch = str.GetAt(n);
				if ((ch >= '0' && ch <= '9') || (ch >= 'a' && ch <= 'f') || (ch >= 'A' && ch <= 'F')) {
					if (ch >= 'a' && ch <= 'f')	ch &= ~0x20;
					BYTE c1 = (BYTE)(ch - '0');
					if (c1 > 9)	c1 -= 7;
					if (n & 1) {
						c <<= 4;
						c |= (c1 & 0xf);
						buf[m ++] = c;
					}
					else	c = c1 & 0xf;
				}
			}
			if (m_pClientSock != NULL)	m_pClientSock->Send(buf, m);
		}
	}
}
*/
LRESULT CNode::OnConnect(WPARAM wParam, LPARAM lParam)
{
	// int nID = (int)wParam;
	if (m_pClientSock != NULL) {
		CString strPeer;
		UINT nPeer;
		m_pClientSock->GetPeerName(strPeer, nPeer);
		CString str;
		str.Format(L"Connect to %s[%d].\r\n", strPeer, nPeer);
		Log(str);
	}
	return 0;
}

LRESULT CNode::OnReceive(WPARAM wParam, LPARAM lParam)
{
	// int nID = (int)wParam;
	if (m_pClientSock != NULL) {
		BYTE temp[4096];
		int nRec = m_pClientSock->Receive(temp, 4096);
		CString str;
		if (nRec == sizeof(TODT)) {
			TODT todt;
			memcpy(&todt, temp, sizeof(TODT));
			str.Format(L"%03d %02d/%02d/%02d %02d:%02d:%02d\n", todt.cnt,
				ToBin(todt.t.year.a), ToBin(todt.t.mon.a), ToBin(todt.t.day.a),
				ToBin(todt.t.hour.a), ToBin(todt.t.min.a), ToBin(todt.t.sec.a));
		}
		else	str.Format(L"wrong length! %d\n", nRec);
		Log(str, RGB(0, 0, 255));
	}
	return 0;
}

LRESULT CNode::OnClose(WPARAM wParam, LPARAM lParam)
{
	// int nID = (int)wParam;
	if (m_pClientSock != NULL) {
		CString strPeer;
		UINT nPeer;
		m_pClientSock->GetPeerName(strPeer, nPeer);
		CString str;
		str.Format(L"Close to %s[%d].\r\n", strPeer, nPeer);
		Log(str);
		m_pClientSock->Close();
		delete m_pClientSock;
		m_pClientSock = NULL;
		if (m_pParent != NULL)	m_pParent->SendMessage(WM_NODECLOSE, (WPARAM)0, (LPARAM)this);
	}
	return 0;
}
