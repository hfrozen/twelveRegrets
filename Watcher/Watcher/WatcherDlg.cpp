// WatcherDlg.cpp
#include "stdafx.h"
#include "Watcher.h"
#include "WatcherDlg.h"
#include "afxdialogex.h"

#include "IpEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);

protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

//=============================================================================

#define	RECCNT_TIMER		2
#define	TIME_RECCNT			1500
#define	SPEEDREPORT_TIMER	4
#define	TIME_SPEEDREPORT	500

//const char CWatcherDlg::m_nHeadTitle[] = "*TUREPORT:";

CWatcherDlg::CWatcherDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CWatcherDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pTcmsSocket = m_pDtsSocket = NULL;
	m_strIp = L"";
	m_nUnitID = 0;
	m_nObligePos = m_nSpeedPos = m_nDistancePos = -1;
	m_bConcept = false;
	m_bBuild = false;
	for (int i = 0; i < SIZE_NAME; i ++)	m_pName[i] = NULL;
	for (int i = 0; i < SIZE_TEXT; i ++)	m_pText[i] = NULL;
	m_uRecv = 0;
	m_nPage = -1;
	//m_iLengthHead = (int)strlen(m_nHeadTitle);
	m_wOblige = 0xffff;
	m_wSpeedK = 0;
	m_dbDistance = 0.f;
	for (int n = 0; n < 5; n ++)
		m_wSvcCycle[n] = 0;
}

CWatcherDlg::~CWatcherDlg()
{
	Close();
	//ClearBand();
	ClearDrum();
	for (int i = 0; i < 2; i ++)	m_font[i].DeleteObject();
}

int CWatcherDlg::atoh(BYTE ch)
{
	if (ch >= '0' && ch <= '9')	return (int)(ch - '0');
	else if (ch >= 'A' && ch <= 'F')	return (int)(ch - 55);
	else if (ch >= 'a' && ch <= 'f')	return (int)(ch - 87);
	return -1;
}

int CWatcherDlg::strtoh(CString str)
{
	int i = 0;
	for (int n = 0; n < str.GetLength(); n ++) {
		TCHAR ch = str.GetAt(n);
		int m = atoh((BYTE)ch);
		if (m < 0)	return i;
		i <<= 4;
		i |= (int)(m & 0xf);
	}
	return i;
}

void CWatcherDlg::Log(CString strLog, COLORREF color)
{
	CHARFORMAT cf;
	m_log.GetDefaultCharFormat(cf);
	cf.dwMask = CFM_COLOR;
	cf.dwEffects = NULL;
	cf.crTextColor = color;
	m_log.SetWordCharFormat(cf);
	m_log.SetSel(-1, -1);
	m_log.ReplaceSel(strLog);
}

CAidStatic* CWatcherDlg::CreateStatic(bool bType, CRect rt)
{
	CAidStatic* pStatic = new CAidStatic(this);
	DWORD dwStyle = WS_CHILD | WS_VISIBLE;
	if (bType) {
		rt.InflateRect(0, 0, rt.Width() + 2, 0);
		dwStyle |= SS_RIGHT;
	}
	else	dwStyle |= (SS_CENTER | SS_CENTERIMAGE | SS_NOTIFY);
	pStatic->Create(L"", dwStyle, rt, this);
	pStatic->SetWindowPos(NULL, rt.left, rt.top, rt.Width(), rt.Height(), SWP_NOZORDER | SWP_NOACTIVATE);
	pStatic->EnableWindow();
	pStatic->SetColor(bType ? COLOR_NAMETEXT : COLOR_TEXT, bType ? COLOR_NAMEBTN : COLOR_BKGND);
	pStatic->ShowWindow(SW_SHOW);
	pStatic->SetFont(&m_font[0]);
	return pStatic;
}

void CWatcherDlg::ClearPeriod()
{
	for (int i = 0; i < 3; i ++) {
		for (int j = 0; j < SIZE_BAND; j ++) {
			m_band[i][j].cur = m_band[i][j].max = 0;
			m_band[i][j].min = 0xffff;
		}
	}
	m_list.DeleteAllItems();
	m_listf.DeleteAllItems();
}

void CWatcherDlg::InsertPeriod(CString str)
{
	int n = m_list.GetItemCount();
	LV_ITEM lv;
	lv.mask = LVIF_TEXT;
	lv.iItem = n;
	lv.iSubItem = 0;
	lv.pszText = str.GetBuffer(str.GetLength());
	m_list.InsertItem(&lv);
	str.ReleaseBuffer();
}

void CWatcherDlg::InsertCycle(CString str)
{
	int n = m_listf.GetItemCount();
	LV_ITEM lv;
	lv.mask = LVIF_TEXT;
	lv.iItem = n;
	lv.iSubItem = 0;
	lv.pszText = str.GetBuffer(str.GetLength());
	m_listf.InsertItem(&lv);
	str.ReleaseBuffer();
}

void CWatcherDlg::ChgDiffer(CListCtrl* pList, int iItem, int iSubItem, WORD w)
{
	CString str;
	if (w < 0xffff)	str.Format(L"%.2f", (double)w / 100.f);
	else	str = L"---";
	CString strTemp = pList->GetItemText(iItem, iSubItem);
	if (str.Compare(strTemp))	pList->SetItemText(iItem, iSubItem, str);
}

void CWatcherDlg::ClearPage()
{
	for (int n = 0; n < LENGTH_PAGE; n ++) {
		m_page[n].wSize = 0;
		m_page[n].strConcept = L"";
	}
	for (int n = m_combo.GetCount() - 1; n >= 0; n --)
		m_combo.DeleteString(n);
	m_nPage = -1;
}

void CWatcherDlg::ClearDrum()
{
	for (int i = 0; i < SIZE_NAME; i ++) {
		if (m_pName[i] != NULL) {
			delete m_pName[i];
			m_pName[i] = NULL;
		}
	}
	for (int i = 0; i < SIZE_TEXT; i ++) {
		if (m_pText[i] != NULL) {
			delete m_pText[i];
			m_pText[i] = NULL;
		}
	}
}

void CWatcherDlg::BuildDrum()
{
	ClearDrum();
	int cur = m_combo.GetCurSel();
	if (cur == m_nPage)	return;

	m_bBuild = true;
	m_nPage = cur;
	CString strConcept = m_page[cur].strConcept;

	CRect rtl;
	CButton* pButton = (CButton*)GetDlgItem(IDOK);
	ASSERT(pButton);
	pButton->GetWindowRect(&rtl);
	ScreenToClient(&rtl);

	CRect rt;
	m_combo.GetWindowRect(&rt);
	ScreenToClient(&rt);
	rt.OffsetRect(0, rt.Height() + 2);
	rt.right = rt.left + 20;
	rt.bottom = rt.top + 16;

	CString str = L"";
	int ni = 0;
	int si = 0;
	int tab = 0;
	int row = 0;
	CRect rc = rt;
	bool bDummy = false;
	for (int i = 0; i < (int)m_page[cur].wSize; ) {
		if (!strConcept.IsEmpty() && si == 0) {
			while (!strConcept.IsEmpty() && ((str = PokeText(strConcept)) == L""));
			if (!str.IsEmpty())	si = atoi((CT2CA)str);
			if (si > 0)	str.Empty();
		}
		if (!str.IsEmpty() || bDummy) {
			if (strConcept.IsEmpty())	str = L"Dummy";
			m_pName[ni] = CreateStatic(true, rc);
			m_pName[ni ++]->SetWindowTextW(str);
			str.Empty();
			rc.OffsetRect((int)(rc.Width() * 2 + 2) + 2, 0);
			bDummy = false;
			tab = 0;
		}
		else {
			m_pText[i] = CreateStatic(false, rc);
			m_pText[i]->SetID(i);
			CString strTemp;
			BYTE c = (BYTE)(i & 0xff);
			if (c < 0xff)	strTemp.Format(L"%02X", c);
			else	strTemp = L"FF";
			m_pText[i ++]->SetWindowTextW(strTemp);
			if (si > 0 && -- si == 0 && strConcept.IsEmpty())	bDummy = true;
			int m = (!((++ tab) % 4)) ? 8 : 2;
			rc.OffsetRect(rc.Width() + m, 0);
		}
		if (rc.right > rtl.right) {
			int m = (!((++ row) % 4)) ? 8 : 2;
			rt.OffsetRect(0, rt.Height() + m);
			rc = rt;
			tab = 0;
		}
	}
	m_bBuild = false;
}

int CWatcherDlg::FindHead(int i, int iLength)
{
	for ( ; i < (iLength - 6); i ++) {
		if (isdigit(m_rBuf[i]) && isdigit(m_rBuf[i + 1]) && isdigit(m_rBuf[i + 2]) &&
			isdigit(m_rBuf[i + 3]) && isdigit(m_rBuf[i + 4]) && isdigit(m_rBuf[i + 5]))
			return i;
	}
	return -1;
}

CString CWatcherDlg::PokeText(CString& str)
{
	CString strText = L"";
	int fi = str.Find((TCHAR)',');
	if (fi < 0) {
		strText = str;
		str.Empty();
	}
	else {
		strText = str.Left(fi);
		str = str.Mid(fi + 1);
	}
	return strText;
}

CString CWatcherDlg::GetBlacketText(CString str)
{
	int fi = str.Find((TCHAR)'(');
	if (fi < 0)	return L"";
	str = str.Mid(fi + 1);
	fi = str.Find((TCHAR)')');
	if (fi >= 0)	str = str.Left(fi);
	return str;
}

CString CWatcherDlg::GetConceptBlock(CString& strRec)
{
	CString strBlock = L"";
	int fi = strRec.Find((TCHAR)'$');
	if (fi < 0)	return strBlock;
	strRec = strRec.Mid(fi + 1);
	fi = strRec.Find((TCHAR)'$');
	if (fi > 0) {
		strBlock = strRec.Left(fi);
		strRec = strRec.Mid(fi);
	}
	else {
		strBlock = strRec;
		strRec.Empty();
	}
	return strBlock;
}

void CWatcherDlg::FindReportPosToDts(CString strConcept)
{
	bool bNo = false;
	int pos = 0;
	while (!strConcept.IsEmpty()) {
		int n = strConcept.Find(L',');
		if (n < 0)	return;
		CString str = strConcept.Left(n);
		strConcept = strConcept.Mid(n + 1);
		if (bNo)	pos += atoi((CT2CA)str);
		else {
			if (m_nObligePos < 0 && !str.Compare(L"BAS"))	m_nObligePos = pos;
			if (m_nSpeedPos < 0 && !str.Compare(L"SPD"))	m_nSpeedPos = pos;
			if (m_nDistancePos < 0 && !str.Compare(L"DIST"))	m_nDistancePos = pos;
			if (m_nObligePos >= 0 && m_nSpeedPos >= 0 && m_nDistancePos >= 0)	return;
		}
		bNo ^= true;
	}
}

int CWatcherDlg::GetConcept(CString strRec)
{
	ClearPeriod();
	ClearPage();
	strRec = strRec.Mid(11);
	CString strBlock;
	int nPage = 0;
	bool bTimeChart = false;
	while ((strBlock = GetConceptBlock(strRec)) != L"") {
		CString strTitle = strBlock.Left(3);
		strBlock = strBlock.Mid(4);		// without ','
		if (!strTitle.Compare(L"TIM")) {
			m_nBandWidth = atoi((CT2CA)strBlock);
			CString strText = GetBlacketText(strBlock);
			if (strText != L"") {
				int fi;
				int leng = 0;
				while ((fi = strText.Find((TCHAR)',')) >= 0) {
					CString strName = strText.Left(fi);
					if (leng ++ < 16)	InsertPeriod(strName);
					else	InsertCycle(strName);
					strText = strText.Mid(fi + 1);
					fi = strText.Find((TCHAR)',');
					if (fi >= 0)	strText = strText.Mid(fi + 1);
				}
				bTimeChart = true;
			}
		}
		else {
			int leng = atoi((CT2CA)strBlock);
			if (leng > 0) {
				m_page[nPage].wSize = (WORD)leng;
				m_page[nPage].strConcept = GetBlacketText(strBlock);
				if (!strTitle.Compare(L"VAR"))	FindReportPosToDts(m_page[nPage].strConcept);
				++ nPage;
				CString strName;
				strName.Format(L"%s(%d)", strTitle, leng);
				m_combo.AddString(strName);
			}
		}
	}
	if (m_nPage < 0 || m_nPage >= m_combo.GetCount())	m_combo.SetCurSel(0);
	else {
		m_combo.SetCurSel(m_nPage);
		m_nPage = -1;
	}
	BuildDrum();
	return m_combo.GetCount();
}

void CWatcherDlg::FillContext(CString strRec)
{
	BYTE* p = &m_cBuf[11];

	for (int n = 0; n < 16; n ++) {
		m_band[0][n].cur = MAKEWORD(p[0], p[1]);
		m_band[1][n].cur = MAKEWORD(p[2], p[3]);
		for (int m = 0; m < 2; m ++) {
			if (m_band[m][n].cur > m_band[m][n].max)	m_band[m][n].max = m_band[m][n].cur;
			if (m_band[m][n].cur < m_band[m][n].min)	m_band[m][n].min = m_band[m][n].cur;
			ChgDiffer(&m_list, n, m * 3 + 1, m_band[m][n].cur);
			ChgDiffer(&m_list, n, m * 3 + 2, m_band[m][n].min);
			ChgDiffer(&m_list, n, m * 3 + 3, m_band[m][n].max);
		}
		p += 4;
	}
	for (int n = 0; n < 30; n ++) {
		m_band[2][n].cur = MAKEWORD(p[0], p[1]);
		if (m_band[2][n].cur > m_band[2][n].max)	m_band[2][n].max = m_band[2][n].cur;
		if (m_band[2][n].cur < m_band[2][n].min)	m_band[2][n].min = m_band[2][n].cur;
		ChgDiffer(&m_listf, n, 1, m_band[2][n].cur);
		ChgDiffer(&m_listf, n, 2, m_band[2][n].min);
		ChgDiffer(&m_listf, n, 3, m_band[2][n].max);
		p += 2;
	}

	p = &m_cBuf[11 + m_nBandWidth];
	BYTE* pOblige = m_nObligePos < 0 ? NULL : p + m_nObligePos;
	BYTE* pSpeed = m_nSpeedPos < 0 ? NULL : p + m_nSpeedPos;
	BYTE* pDist = m_nDistancePos < 0 ? NULL : p + m_nDistancePos;
	int n = 0;
	for ( ; n < m_combo.GetCurSel(); n ++)	p += m_page[n].wSize;
	BYTE *sp = p;
	for (int m = 0; m < m_page[n].wSize; m ++) {
		if (m_pText[m] != NULL) {
			CString str;
			if (*p != 0xff)	str.Format(L"%02X", *p);
			else	str = L"FF";
			COLORREF color = m_pText[m]->GetInvert() ? COLOR_INVTEXT : COLOR_TEXT;
			CString strTemp;
			m_pText[m]->GetWindowTextW(strTemp);
			if (str.Compare(strTemp)) {
				m_pText[m]->SetWindowTextW(str);
				color = m_pText[m]->GetInvert() ? COLOR_INVTEXTHIGH : COLOR_TEXTHIGH;
			}
			if (m_pText[m]->GetTextColor() != color)		m_pText[m]->SetTextColor(color);
		}
		++ p;
	}

	if (n > 0 && n < 5) {
		BYTE id = *(sp + 105);
		if (id == 1 || id == 2 || id == 4 || id == 8 || id == 0xff) {
			CEdit* pEdit;
			int size;
			int ci;
			switch (id) {
			case 1 :	ci = 0;	size = 36;	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SVCA);	break;
			case 2 :	ci = 1;	size = 69;	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SVCB);	break;
			case 4 :	ci = 2;	size = 19;	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SVCC);	break;
			case 8 :	ci = 3;	size = 56;	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SVCD);	break;
			default :	ci = 4;	size = 40;	pEdit = (CEdit*)GetDlgItem(IDC_EDIT_SVCE);	break;
			}
			if (pEdit != NULL) {
				++ m_wSvcCycle[ci];
				CString str;
				str.Format(L"%5d ", m_wSvcCycle[ci]);
				for (int n = 0; n < size; n ++) {
					//if (!(n % 32))	str += L"\r\n";
					CString strTemp;
					BYTE m = *(sp + 104 + n);
					if (m != 0xff)	strTemp.Format(L"%02X ", m);
					else	strTemp = L"FF ";
					str += strTemp;
				}
				pEdit->SetWindowTextW(str);
			}
		}
	}

	CString str;
	DEGDBC dbc;
	if (pOblige != NULL) {
		_DUET du;
		du.c[1] = *pOblige ++;
		du.c[0] = *pOblige;
		if (m_wOblige != du.w) {
			m_wOblige = du.w;
			str.Format(L"OB=%04X", m_wOblige);
			SendToDts(str, 1);
		}
	}

	if (pSpeed != NULL) {
		CStatic* pStatic = (CStatic*)GetDlgItem(IDC_STATIC_DB);
		if (pStatic != NULL) {
			memcpy(&dbc.c[0], pSpeed, sizeof(double));
			str.Format(L"%f", dbc.db);
			pStatic->SetWindowTextW(str);
			m_wSpeedK = (WORD)(dbc.db / 1000.f);
		}
	}

	if (pSpeed != NULL) {
		memcpy(&dbc.c[0], pDist, sizeof(double));
		m_dbDistance = dbc.db;
	}
}

void CWatcherDlg::Assort(int iLength)
{
	m_cBuf[iLength] = NULL;
	CString strRec(m_cBuf);
	if (strRec.GetLength() < 11)	return;

	CString strText = strRec.Mid(6, 5);
	if (!strText.Compare(L"NORM>")) {
		if (m_bConcept)	FillContext(strRec);
	}
	else if (!strText.Compare(L"INIT>")) {
		//TRACE(L"Concept from %dbyte\n", iLength);
		if (GetConcept(strRec) > 0)	m_bConcept = true;
	}
	else if (!strText.Compare(L"CONT>")) {
		Demand();
	}
	strText.Format(L"%d", iLength);
	m_sttRecCnt.SetWindowTextW(strText);
	m_sttRecCnt.SetTextColor(RGB(0, 0, 0));
	KillTimer(RECCNT_TIMER);
	SetTimer(RECCNT_TIMER, TIME_RECCNT, NULL);
}

bool CWatcherDlg::Open(int nID, CString strIp, UINT nPort)
{
	CClientSocket* pClientSocket = new CClientSocket;
	if (pClientSocket->Create()) {
		pClientSocket->SetHwnd(GetSafeHwnd());
		pClientSocket->SetID(nID);
		if (!pClientSocket->Connect(strIp, nPort)) {
			int nError = GetLastError();
			if (nError != WSAEWOULDBLOCK) {
				CString str;
				str.Format(L"Can not connect!(%d)", nError);
				Log(str, RGB(255, 0, 0));
				pClientSocket->Close();
				delete pClientSocket;
				pClientSocket = NULL;
				return false;
			}
			else {	// wait connect
			}
		}
		else {
			m_uRecv = 0;
			CString strPeer;
			UINT nPeer;
			pClientSocket->GetPeerName(strPeer, nPeer);
			CString str;
			str.Format(L"Connect to %s[%d]!\r\n", strPeer, nPeer);
			Log(str, RGB(0, 0, 255));
			m_dwRecCycle = 0;
		}
	}
	else {
		Log(L"Socket creation failure!", RGB(255, 0, 0));
		delete pClientSocket;
		pClientSocket = NULL;
		return false;
	}
	if (nID == 0)	m_pTcmsSocket = pClientSocket;
	else	m_pDtsSocket = pClientSocket;
	return true;
}

bool CWatcherDlg::Opens(CString strIp)
{
	Close();
	bool bResa = Open(0, strIp, 5000);
	bool bResb = Open(1, L"127.0.0.1", 5000);
	if (bResa && bResb)	return true;
	return false;
}

void CWatcherDlg::Close()
{
	if (m_pTcmsSocket != NULL) {
		m_pTcmsSocket->Close();
		delete m_pTcmsSocket;
		m_pTcmsSocket = NULL;
	}
	if (m_pDtsSocket != NULL) {
		m_pDtsSocket->Close();
		delete m_pDtsSocket;
		m_pDtsSocket = NULL;
	}
}

void CWatcherDlg::SendToTcms(CString strSend, int nID)
{
	Log(strSend + L"\r\n", RGB(0, 0, 255));
	int len = strSend.GetLength();
	char* p = new char[len];
	WideCharToMultiByte(CP_ACP, 0, strSend, -1, p, len, NULL, NULL);
	if (nID == 0 && m_pTcmsSocket != NULL)	m_pTcmsSocket->Send(p, len);
	delete [] p;
}

void CWatcherDlg::SendToDts(CString strSend, int nID)
{
	Log(strSend + L"\r\n", RGB(0, 0, 255));
	int len = strSend.GetLength();
	char* p = new char[len];
	WideCharToMultiByte(CP_ACP, 0, strSend, -1, p, len, NULL, NULL);
	if (nID != 0 && m_pDtsSocket != NULL)	m_pDtsSocket->Send(p, len);
	delete [] p;
}

void CWatcherDlg::Demand()
{
	CString str;
	switch (m_nUnitID) {
	case 1 :	str = L"REPRU";	break;
	default :	str = L"REPTU";	break;
	}
	SendToTcms(str, 0);
}

void CWatcherDlg::TcmsDebug()
{
	if (m_pTcmsSocket != NULL) {
		CString str;
		m_editCmd.GetWindowTextW(str);
		if (!str.IsEmpty())	SendToTcms(str, 0);
	}
}

void CWatcherDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_LINK, m_btnLink);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Control(pDX, IDC_LISTF, m_listf);
	DDX_Control(pDX, IDC_RICHEDIT, m_log);
	DDX_Control(pDX, IDC_COMBO, m_combo);
	DDX_Control(pDX, IDC_EDIT_CMD, m_editCmd);
	DDX_Control(pDX, IDC_STATIC_POS, m_sttPos);
}

BEGIN_MESSAGE_MAP(CWatcherDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_CBN_SELCHANGE(IDC_COMBO, &CWatcherDlg::OnCbnSelchangeCombo)
	ON_BN_CLICKED(IDC_BUTTON_LINK, &CWatcherDlg::OnBnClickedButtonLink)
	ON_MESSAGE(WM_SCONNECT, &CWatcherDlg::OnConnect)
	ON_MESSAGE(WM_SRECEIVE, &CWatcherDlg::OnReceive)
	ON_MESSAGE(WM_SCLOSE, &CWatcherDlg::OnClose)
	ON_MESSAGE(WM_LBTNDBCLK, &CWatcherDlg::OnLBtnDbClk)
	ON_BN_CLICKED(IDC_BUTTON_REPORT, &CWatcherDlg::OnBnClickedButtonReport)
	ON_BN_CLICKED(IDC_BUTTON_CONV, &CWatcherDlg::OnBnClickedButtonConv)
	ON_BN_CLICKED(IDC_BUTTON_HEX, &CWatcherDlg::OnBnClickedButtonHex)
	ON_BN_CLICKED(IDC_BUTTON_CMD, &CWatcherDlg::OnBnClickedButtonCmd)
END_MESSAGE_MAP()

BOOL CWatcherDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL) {
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty()) {
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	m_font[0].CreateFontW(-14, 0, 0, 0, 400, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Courier New"));
	m_font[1].CreateFontW(-12, 0, 0, 0, 400, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Courier New"));

	m_sttRecCnt.SubclassDlgItem(IDC_STATIC_RECCNT, this);
	m_sttRecCnt.SetColor(RGB(0, 0, 0), RGB(255, 255, 255));

	m_list.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	LV_COLUMN lc;
	CString str;
	for (int i = 0; i < 7; i ++) {
		lc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lc.fmt = LVCFMT_CENTER;
		str = i ? L"WWWW" : L"WWWW";
		lc.cx = m_list.GetStringWidth(str);
		lc.pszText = str.GetBuffer(str.GetLength());
		lc.iSubItem = i;
		m_list.InsertColumn(i, &lc);
		str.ReleaseBuffer();
	}

	m_listf.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	for (int i = 0; i < 4; i ++) {
		lc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lc.fmt = LVCFMT_CENTER;
		str = i ? L"88888888" : L"WWWWWW";
		lc.cx = m_list.GetStringWidth(str);
		lc.pszText = str.GetBuffer(str.GetLength());
		lc.iSubItem = i;
		m_listf.InsertColumn(i, &lc);
		str.ReleaseBuffer();
	}

	m_strIp.Empty();
	int nArg;
	LPWSTR* pStr = CommandLineToArgvW(GetCommandLine(), &nArg);
	if (nArg > 1) {
		m_strIp.Format(L"%s", pStr[1]);
		m_nUnitID = 0;
	}
	if (m_strIp.IsEmpty()) {
		CWinApp* pApp = AfxGetApp();
		str = pApp->GetProfileStringW(L"TCMS_WATCHER", L"IP");

		CIpEdit dlg;
		dlg.m_strIp = str;
		dlg.m_nUnit = m_nUnitID;
		if (dlg.DoModal() != IDOK)	return FALSE;
		m_strIp = dlg.m_strIp;
		m_nUnitID = dlg.m_nUnit;
		pApp->WriteProfileStringW(L"TCMS_WATCHER", L"IP", m_strIp);
	}
	SetWindowText(m_strIp);

	if (Opens(m_strIp))	m_btnLink.SetWindowTextW(L"²÷ ±â");
	else	m_btnLink.SetWindowTextW(L"¿¬ °á");

	return TRUE;
}

BOOL CWatcherDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN)) {
		if (GetFocus()->GetSafeHwnd() == m_editCmd.GetSafeHwnd())	TcmsDebug();
		return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CWatcherDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else	CDialogEx::OnSysCommand(nID, lParam);
}

void CWatcherDlg::OnPaint()
{
	if (IsIconic()) {
		CPaintDC dc(this);

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		dc.DrawIcon(x, y, m_hIcon);
	}
	else	CDialogEx::OnPaint();
}

HCURSOR CWatcherDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CWatcherDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == RECCNT_TIMER) {
		KillTimer(RECCNT_TIMER);
		m_sttRecCnt.SetTextColor(RGB(255, 0, 0));
	}
	else if (nIDEvent == SPEEDREPORT_TIMER) {
		CString str;
		if (m_nSpeedPos >= 0) {
			str.Format(L"SP=%d", m_wSpeedK);
			SendToDts(str, 1);
		}
		if (m_nDistancePos >= 0) {
			str.Format(L"DT=%d", (DWORD)(m_dbDistance / 1000.f));
			SendToDts(str, 1);
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}

void CWatcherDlg::OnCbnSelchangeCombo()
{
	BuildDrum();
}

LRESULT CWatcherDlg::OnConnect(WPARAM wParam, LPARAM lParam)
{
	int nID = (int)wParam;
	if (nID == 0 && m_pTcmsSocket != NULL) {
		m_uRecv = 0;
		CString strPeer;
		UINT nPeer;
		m_pTcmsSocket->GetPeerName(strPeer, nPeer);
		CString str;
		str.Format(L"Connect to %s[%d].\r\n", strPeer, nPeer);
		Log(str, RGB(0, 0, 255));
		TRACE(L"OnConnect.\n");
		m_dwRecCycle = 0;
		Demand();
	}
	else if (nID == 1 && m_pDtsSocket != NULL) {
		CString strPeer;
		UINT nPeer;
		m_pDtsSocket->GetPeerName(strPeer, nPeer);
		CString str;
		str.Format(L"Connect to %s[%d].\r\n", strPeer, nPeer);
		Log(str, RGB(0, 0, 255));
		str.Format(L"IP=%s", m_strIp);
		SendToDts(str, 1);
		SetTimer(SPEEDREPORT_TIMER, TIME_SPEEDREPORT, NULL);
	}
	return 0;
}

LRESULT CWatcherDlg::OnReceive(WPARAM wParam, LPARAM lParam)
{
	int nID = (int)wParam;
	if (nID == 0 && m_pTcmsSocket != NULL) {
		//if (m_pTcmsSocket == NULL)	return 0;
		int ri = m_pTcmsSocket->Receive(&m_rBuf[m_uRecv], SIZE_SBUF - m_uRecv);
		TRACE(L"Receive %d bytes at %d\n", ri, m_uRecv);
		ri += m_uRecv;
		int fi;
		int ni = 0;
		while ((fi = FindHead(ni, ri)) >= ni) {
			int rrec = atoi((char*)&m_rBuf[fi]);		// reserve length
			if (rrec <= (ri - fi)) {
				clock_t start = clock();
				memcpy(m_cBuf, &m_rBuf[fi], rrec);
				Assort(rrec);
				ni = rrec + fi;
				clock_t finish = clock();
				double duration = (double)(finish - start) / CLOCKS_PER_SEC;
			}
			else {
				m_uRecv = ri - fi;
				memcpy(m_rBuf, &m_rBuf[fi], m_uRecv);
				break;
			}
		}
		if (fi < ni)	m_uRecv = 0;
	}
	else if (nID == 1 && m_pDtsSocket != NULL) {
	}
	return 0;
}

LRESULT CWatcherDlg::OnClose(WPARAM wParam, LPARAM lParam)
{
	int nID = (int)wParam;
	if (nID == 0 && m_pTcmsSocket != NULL) {
		CString strPeer;
		UINT nPeer;
		m_pTcmsSocket->GetPeerName(strPeer, nPeer);
		CString str;
		str.Format(L"Close to %s[%d].\r\n", strPeer, nPeer);
		Log(str, RGB(0, 0, 255));
		TRACE(L"OnClose.\n");
	}
	else if (nID == 1 && m_pDtsSocket != NULL) {
	}
	return 0;
}

LRESULT CWatcherDlg::OnLBtnDbClk(WPARAM wParam, LPARAM lParam)
{
	CString str;
	str.Format(L"%d", (int)wParam);
	m_sttPos.SetWindowTextW(str);
	return 0;
}

void CWatcherDlg::OnBnClickedButtonLink()
{
	CString str;
	CButton* pButton = (CButton*)GetDlgItem(IDC_BUTTON_LINK);
	if (pButton == NULL)	return;
	pButton->GetWindowTextW(str);
	if (str == L"¿¬ °á") {
		CIpEdit dlg;
		dlg.m_strIp = m_strIp;
		dlg.m_nUnit = m_nUnitID;
		if (dlg.DoModal() == IDOK) {
			m_strIp = dlg.m_strIp;
			m_nUnitID = dlg.m_nUnit;
			CWinApp* pApp = AfxGetApp();
			pApp->WriteProfileStringW(L"TCMS_WATCHER", L"IP", m_strIp);
			if (Opens(m_strIp))	m_btnLink.SetWindowTextW(L"²÷ ±â");
		}
	}
	else {
		Close();
		m_btnLink.SetWindowTextW(L"¿¬ °á");
	}
}

void CWatcherDlg::OnBnClickedButtonReport()
{
	SendToTcms(L"REPTU", 0);
}

void CWatcherDlg::OnBnClickedButtonConv()
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_CONV);
	if (pEdit != NULL) {
		CString str;
		pEdit->GetWindowTextW(str);
		if (str.GetLength() == 8) {
			m_dt.dw = 0;
			for (int n = 0; n < str.GetLength(); n ++) {
				TCHAR ch = str.GetAt(n);
				int m = atoh((BYTE)ch);
				if (m < 0)	break;
				m_dt.dw |= ((DWORD)(BYTE)m << ((n ^ 1) * 4));
			}
			if (m_dt.dw > 0) {
				str.Format(L"%02d/%02d/%02d %02d:%02d:%02d", m_dt.t.year + 10, m_dt.t.mon, m_dt.t.day, m_dt.t.hour, m_dt.t.min, m_dt.t.sec);
				CStatic* pStatic = (CStatic*)GetDlgItem(IDC_STATIC_CONV);
				if (pStatic != NULL)	pStatic->SetWindowTextW(str);
			}
		}
		else if (str.GetLength() == 12) {
			CString strTemp = str.Left(2);
			str = str.Mid(2);
			m_dt.t.year = (BYTE)atoi((CT2CA)strTemp) - 10;
			strTemp = str.Left(2);
			str = str.Mid(2);
			m_dt.t.mon = (BYTE)atoi((CT2CA)strTemp);
			strTemp = str.Left(2);
			str = str.Mid(2);
			m_dt.t.day = (BYTE)atoi((CT2CA)strTemp);
			strTemp = str.Left(2);
			str = str.Mid(2);
			m_dt.t.hour = (BYTE)atoi((CT2CA)strTemp);
			strTemp = str.Left(2);
			str = str.Mid(2);
			m_dt.t.min = (BYTE)atoi((CT2CA)strTemp);
			strTemp = str;	//.Left(2);
			//str = str.Mid(2);
			m_dt.t.sec = (BYTE)atoi((CT2CA)strTemp);
			str.Format(L"%08X", m_dt.dw);
			CStatic* pStatic = (CStatic*)GetDlgItem(IDC_STATIC_CONV);
			if (pStatic != NULL)	pStatic->SetWindowTextW(str);
		}
	}
}

void CWatcherDlg::OnBnClickedButtonHex()
{
	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_HEX);
	if (pEdit != NULL) {
		CString str;
		pEdit->GetWindowTextW(str);
		if (str.GetLength() > 0) {
			int i = strtoh(str);
			str.Format(L"%d", i);
			CStatic* pStatic = (CStatic*)GetDlgItem(IDC_STATIC_HEX);
			if (pStatic != NULL)	pStatic->SetWindowTextW(str);
		}
	}
}

void CWatcherDlg::OnBnClickedButtonCmd()
{
	TcmsDebug();
}
