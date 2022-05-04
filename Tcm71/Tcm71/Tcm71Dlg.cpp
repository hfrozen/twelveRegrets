// Tcm71Dlg.cpp : 구현 파일
//

#include "stdafx.h"
#include "time.h"

#include "Tcm71.h"
#include "Tcm71Dlg.h"
#include "Ip.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CTcm71Dlg 대화 상자




CTcm71Dlg::CTcm71Dlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTcm71Dlg::IDD, pParent)
{
	AfxInitRichEdit();
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pClientSock = NULL;
	m_strIp = _T("192.168.0.220");
	for (int n = 0; n < SIZE_TEXT; n ++)
		m_pText[n] = NULL;
	for (int n = 0; n < SIZE_TITLE; n ++)
		m_pTitle[n] = NULL;
	for (int n = 0; n < SIZE_DASH; n ++)
		m_pDash[n] = NULL;
	m_nTextSize = 0;
	ClearIntervals();
	ClearArches();
	ZeroMemory(m_cBuf, SIZE_SBUF);
	m_nCurFrame = m_nChgFrame = -1;
	m_nChkFrame = 2;
	m_dwCycle = 0l;
	m_uRecvi = 0;
	m_bChkFrame = FALSE;
	m_bOpen = FALSE;
	m_bTimer = FALSE;
}

CTcm71Dlg::~CTcm71Dlg()
{
	if (m_pClientSock != NULL) {
		m_pClientSock->Close();
		delete m_pClientSock;
		m_pClientSock = NULL;
	}
	if (m_bTimer) {
		KillTimer(TIMER_WATCH);
		m_bTimer = FALSE;
	}
	ClearTexts();
}

void CTcm71Dlg::ClearTexts()
{
	for (int n = 0; n < SIZE_TEXT; n ++) {
		if (m_pText[n] != NULL) {
			delete m_pText[n];
			m_pText[n] = NULL;
		}
	}
	for (int n = 0; n < SIZE_TITLE; n ++) {
		if (m_pTitle[n] != NULL) {
			delete m_pTitle[n];
			m_pTitle[n] = NULL;
		}
	}
	for (int n = 0; n < SIZE_DASH; n ++) {
		if (m_pDash[n] != NULL) {
			delete m_pDash[n];
			m_pDash[n] = NULL;
		}
	}
}

void CTcm71Dlg::ClearIntervals()
{
	for (int n = 0; n < SIZE_INTERVAL; n ++) {
		m_interval[n].cur =
		m_interval[n].avr =
		m_interval[n].max = -1.f;
		m_interval[n].min = 1000.f;
	}
}

void CTcm71Dlg::ClearArches()
{
	m_nArchSize = 0;
	for (int n = 0; n < SIZE_ARCH; n ++) {
		m_arch[n].nLength = 0;
		m_arch[n].nSize = 0;
		for (int m = 0; m < SIZE_BLOCK; m ++) {
			m_arch[n].block[m].nSize = 0;
			ZeroMemory(m_arch[n].block[m].name, SIZE_NAME);
		}
	}
}

void CTcm71Dlg::Scribbling(CString str, COLORREF color)
{
	/*if (bClr) {
		m_edit.SetReadOnly(FALSE);
		m_edit.SetSel(0, -1);
		m_edit.Clear();
		m_edit.SetReadOnly();
	}*/

	CHARFORMAT cf;
	m_edit.GetDefaultCharFormat(cf);
	cf.dwMask = CFM_COLOR;
	cf.dwEffects = NULL;
	cf.crTextColor = color;
	m_edit.SetWordCharFormat(cf);
	m_edit.SetSel(-1, -1);
	m_edit.ReplaceSel(str);
	//cf.crTextColor = RGB(16, 16, 16);
	//m_edit.SetWordCharFormat(cf);
}

int CTcm71Dlg::FindHead(int n, int length)
{
	for ( ; n < (length - 8); n ++) {
		if (m_rBuf[n] == '\t' && isdigit(m_rBuf[n + 1]) && isdigit(m_rBuf[n + 2]) && isdigit(m_rBuf[n + 3]) &&
			isdigit(m_rBuf[n + 4]) && isdigit(m_rBuf[n + 5]) && isdigit(m_rBuf[n + 6]) && m_rBuf[n + 7] == ':')
			return n;
	}
	return -1;
}

BOOL CTcm71Dlg::Open()
{
	if (m_bTimer)	KillTimer(TIMER_WATCH);
	SetTimer(TIMER_WATCH, WATCH_TIME, NULL);
	m_bTimer = TRUE;
	if (m_pClientSock != NULL) {
		CString str;
		str.Format(_T("Close B.\r\n"));
		Scribbling(str, RGB(0, 0, 255));
		m_pClientSock->Close();
		delete m_pClientSock;
		m_pClientSock = NULL;
	}
	m_bOpen = FALSE;
	m_pClientSock = new CClientSocket();
	if (m_pClientSock->Create()) {
		m_pClientSock->SetHwnd(this->m_hWnd);
		if (!m_pClientSock->Connect(m_strIp, 5000)) {
			int nError = GetLastError();
			if (nError != WSAEWOULDBLOCK) {
				CString str;
				str.Format(_T("Can not connect.(%d)\r\n"), nError);
				Scribbling(str, RGB(255, 0, 0));
				m_pClientSock->Close();
				delete m_pClientSock;
				m_pClientSock = NULL;
				return FALSE;
			}
			else {
				CString str;
				str.Format(_T("Connect B.\r\n"));
				Scribbling(str, RGB(0, 0, 255));
			}
		}
		else {
			CString str;
			str.Format(_T("Connect fail.\r\n"));
			Scribbling(str, RGB(255, 0, 0));
		}
	}
	else {
		Scribbling(_T("Socket create failure!\r\n"), RGB(255, 0, 0));
		delete m_pClientSock;
		m_pClientSock = NULL;
		return FALSE;
	}
	m_bOpen = TRUE;
	return TRUE;
}

void CTcm71Dlg::Assort()
{
	if (m_cBuf[4] != ':')	return;
	m_cBuf[4] = 0;
	if (!strcmp((char*)m_cBuf, "INIT"))	Initial();
	else if (!strcmp((char*)m_cBuf, "VARS"))	Monitor();
	ZeroMemory(m_cBuf, SIZE_SBUF);
}

void CTcm71Dlg::Initial()
{
	CString str;
	str.Format(_T("%s"), (CA2CT)(char*)&m_cBuf[5]);

	int fi;
	if ((fi = str.Find(_T("TCYCLE:"))) >= 0) {
		CString strTemp = str.Mid(strlen("TCYCLE:") + fi);
		if ((fi = strTemp.Find(';')) >= 0) {
			strTemp = strTemp.Left(fi);
			InitialInterval(strTemp);
		}
	}
	if ((fi = str.Find(_T("BUSMON:"))) >= 0) {
		CString strTemp = str.Mid(strlen("BUSMON:") + fi);
		if ((fi = strTemp.Find(';')) >= 0) {
			strTemp = strTemp.Left(fi);
			InitialFrame(strTemp);
		}
	}
}

void CTcm71Dlg::InitialInterval(CString str)
{
	ClearIntervals();
	m_list.DeleteAllItems();
	int length = 0;
	int fi;
	while ((fi = str.Find('|')) >= 0) {
		CString strTemp = str.Left(fi);
		TRACE(_T("add %S\r\n"), strTemp);
		InsertList(strTemp);
		str = str.Mid(fi + 1);
		++ length;
	}
	TRACE(_T("add total %d\r\n"), length);
}

void CTcm71Dlg::InsertList(CString str)
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

void CTcm71Dlg::InitialFrame(CString str)
{
	ClearArches();
	for (int n = m_combo.GetCount() - 1; n >= 0; n --)
		m_combo.DeleteString(n);

	m_dwCycle = 0;
	int fi;
	int a = 0;
	int b = 0;
	while ((fi = str.Find('|')) >= 0) {
		CString strBlock = str.Left(fi);
		str = str.Mid(fi + 1);	// for next
		CString strTitle;
		strTitle.Empty();
		if ((fi = strBlock.Find('(')) >= 0)
			strTitle = strBlock.Left(fi);
		while ((fi = strBlock.Find('(')) >= 0) {
			strBlock = strBlock.Mid(fi + 1);
			if ((fi = strBlock.Find(')')) >= 0) {
				CString strUnit = strBlock.Left(fi);
				strBlock = strBlock.Mid(fi + 1);
				if ((fi = strUnit.Find(':')) >= 0) {
					CString strName = strUnit.Left(fi);
					strcpy_s(m_arch[a].block[b].name, SIZE_NAME, (CT2CA)strName);
					strUnit = strUnit.Mid(fi + 1);
					m_arch[a].block[b].nSize = (int)atoi((CT2CA)strUnit);
					++ m_arch[a].nLength;
					m_arch[a].nSize += m_arch[a].block[b ++].nSize;
				}
			}
		}
		if (strTitle.IsEmpty())	strTitle.Format(_T("%d"), a);
		m_combo.AddString(strTitle);
		TRACE(_T("Frame %d-%d\r\n"), m_arch[a].nLength, m_arch[a].nSize);
		++ a;
		b = 0;
	}
	m_nArchSize = a;
	TRACE(_T("Frame total %d\r\n"), m_nArchSize);
	m_combo.SetCurSel(0);
	m_nCurFrame = 0;
	BuildFrame();
	m_nChgFrame = 0;
}

#define	SIZEX	20
#define	SIZEN	54
void CTcm71Dlg::BuildFrame()
{
	ClearTexts();
	CRect rt;
	m_combo.GetWindowRect(&rt);
	ScreenToClient(&rt);
	int sizey = rt.Height();
	rt.OffsetRect(0, sizey + 4);
	rt.right = rt.left + SIZEN;

	CRect rect = rt;
	CPoint pt(SIZEX, 0);
	int t = 0;
	int d = 0;
	for (int n = 0; n < m_arch[m_nCurFrame].nLength; n ++) {
		m_pTitle[n] = new CAidStatic();
		m_pTitle[n]->Create((CA2CT)m_arch[m_nCurFrame].block[n].name,
								WS_CHILD | WS_VISIBLE, rect, this);
		m_pTitle[n]->SetWindowPos(NULL, rect.left, rect.top, rect.Width() - 8, rect.Height(),
								SWP_NOZORDER | SWP_NOACTIVATE);
		m_pTitle[n]->EnableWindow();
		m_pTitle[n]->ShowWindow(SW_SHOW);
		CRect rct = rect;
		rct.left = rct.right;
		rct.right = rct.left + SIZEX;
		int cs = 0;
		for (int m = 0; m < m_arch[m_nCurFrame].block[n].nSize; m ++) {
			m_pText[t] = new CAidStatic();
			m_pText[t]->Create(_T("88"), WS_CHILD | WS_VISIBLE, rct, this);
			m_pText[t]->SetWindowPos(NULL, rct.left, rct.top, rct.Width(), rct.Height(),
										SWP_NOZORDER | SWP_NOACTIVATE);
			m_pText[t]->EnableWindow();
			m_pText[t]->ShowWindow(SW_SHOW);
			++ t;
			++ cs;
			if (cs > 29) {
				if ((m + 1) < m_arch[m_nCurFrame].block[n].nSize) {
				cs = 0;
				rect.OffsetRect(0, sizey);
				rct = rect;
				rct.left = rct.right;
				rct.right = rct.left + SIZEX;
				}
			}
			else if (cs == 10 || cs == 20) {
				rct.OffsetRect(pt);
				m_pDash[d] = new CAidStatic();
				m_pDash[d]->Create(_T("-"), WS_CHILD | WS_VISIBLE, rct, this);
				m_pDash[d]->SetWindowPos(NULL, rct.left, rct.top, rct.Width() / 2, rct.Height(),
											SWP_NOZORDER | SWP_NOACTIVATE);
				m_pText[d]->EnableWindow();
				m_pText[d]->ShowWindow(SW_SHOW);
				++ d;
				rct.OffsetRect(pt.x / 2, 0);
			}
			else	rct.OffsetRect(pt);
		}
		rect.OffsetRect(0, sizey);
	}
	m_nTextSize = t;
}

void CTcm71Dlg::Monitor()
{
	if (m_cBuf[11] != ':')	return;
	m_cBuf[11] = 0;
	if (!strcmp((char*)&m_cBuf[5], "TCYCLE")) {
		CString str;
		str.Format(_T("%s"), (CA2CT)(char*)&m_cBuf[12]);
		MonitorInterval(str);
	}
	else if (!strcmp((char*)&m_cBuf[5], "BUSMON")) {
		MonitorFrame(12);
		if (m_bChkFrame)	CheckFrame(m_nChkFrame, 12);
	}
}

void CTcm71Dlg::MonitorInterval(CString str)
{
	int leng = 0;
	int fi;
	while ((fi = str.Find('|')) >= 0) {
		CString strTemp = str.Left(fi);
		str = str.Mid(fi + 1);
		int id = atoi((CT2CA)strTemp);
		if (id < 1)	continue;
		-- id;
		if ((fi = strTemp.Find(':')) >= 0) {
			strTemp = strTemp.Mid(fi + 1);
			if ((fi = strTemp.Find('=')) >= 0)	EmptyInterval(id);
			else	PutInterval(id, atof((CT2CA)strTemp));
		}
	}
}

void CTcm71Dlg::EmptyInterval(int n)
{
	int leng = m_list.GetItemCount();
	if (n >= leng)	return;

	LV_ITEM lv;
	lv.mask = LVIF_TEXT;
	lv.iItem = n;
	CString str = _T("---");
	lv.iSubItem = 1;
	lv.pszText = str.GetBuffer(str.GetLength());
	m_list.SetItem(&lv);
	str.ReleaseBuffer();
	UpdateData();
}

void CTcm71Dlg::PutInterval(int n, double v)
{
	int leng = m_list.GetItemCount();
	if (n >= leng)	return;

	m_interval[n].cur = v;
	if (m_interval[n].min > v)	m_interval[n].min = v;
	if (m_interval[n].max < v)	m_interval[n].max = v;

	LV_ITEM lv;
	lv.mask = LVIF_TEXT;
	lv.iItem = n;
	lv.iSubItem = 1;
	CString str;
	str.Format(_T("%.4f"), m_interval[n].cur);
	lv.pszText = str.GetBuffer(str.GetLength());
	m_list.SetItem(&lv);
	str.ReleaseBuffer();

	if (m_interval[n].min < 1000.f) {
		lv.iSubItem = 2;
		str.Format(_T("%.4f"), m_interval[n].min);
		CString strVer = m_list.GetItemText(lv.iItem, lv.iSubItem);
		if (str.Compare(strVer)) {
			lv.pszText = str.GetBuffer(str.GetLength());
			m_list.SetItem(&lv);
			str.ReleaseBuffer();
		}
	}
	if (m_interval[n].max > 0.f) {
		lv.iSubItem = 3;
		str.Format(_T("%.4f"), m_interval[n].max);
		CString strVer = m_list.GetItemText(lv.iItem, lv.iSubItem);
		if (str.Compare(strVer)) {
			lv.pszText = str.GetBuffer(str.GetLength());
			m_list.SetItem(&lv);
			str.ReleaseBuffer();
		}
	}
	UpdateData();
}

void CTcm71Dlg::MonitorFrame(int si)
{
	if (m_nChgFrame != m_nCurFrame) {
		m_nCurFrame = m_nChgFrame;
		BuildFrame();
	}
	int n = m_nCurFrame;
	int m = 0;
	while (n > 0) {
		si += m_arch[m ++].nSize;
		-- n;
	}

	CString str;
	if (m_nCurFrame == 14) {
		m_editDu.GetWindowText(str);
		WORD w = (WORD)atoi((CT2CA)str);
		if (w > 10) {
			WORD v = ((WORD)m_cBuf[si + 2] << 8) | (WORD)m_cBuf[si + 3];
			if (w != v)	return;
		}
	}
	for (n = 0; n < m_nTextSize; n ++) {
		if (n < m_arch[m].nSize) {
			unsigned char ch = m_cBuf[si ++];
			if (ch != 0xff)	str.Format(_T("%02X"), ch);
			else	str = _T("FF");

			if (m_pText[n] != NULL) {
				CString strVer;
				m_pText[n]->GetWindowText(strVer);
				if (str.Compare(strVer))	m_pText[n]->SetWindowText(str);
			}
		}
	}
	++ m_dwCycle;
	str.Format(_T("size=%d-%d"), m_arch[m].nSize, m_dwCycle);
	m_editCycle.SetWindowText(str);
}

void CTcm71Dlg::CheckFrame(int page, int si)
{
	int n = page;
	int size = 0;
	while (n > 0) {
		si += m_arch[size ++].nSize;
		-- n;
	}

	size = m_arch[size].nSize;
	BOOL bDiff = FALSE;
	CString strDiff = _T("");
	for (n = 0; n < size; n ++) {
		if ((m_cBuf[si + n] | m_nMask[n]) != (m_cBuf[si + n + size] | m_nMask[n])) {
			bDiff = TRUE;
			strDiff += _T("XX ");
		}
		else {
			CString strTemp;
			if (m_nMask[n] != 0xff)	strTemp.Format(_T("%02X "), m_nMask[n]);
			else	strTemp = _T("FF ");
			strDiff += strTemp;
		}
	}
	strDiff += _T("\r\n");
	if (bDiff) {
		CString str = _T("\r\n");
		//str.Format(_T("\r\n%d(%d)\r\n"), n, n * 3);
		//str += MakeDestField(m_nMask, 0, size);
		str += strDiff;
		str += MakeDestField(m_cBuf, si, si + size);
		str += MakeDestField(m_cBuf, si + size, si + size + size);
		StoreDestination(str);
		str.Format(_T("Unmatch frame %d\r\n"), ++ m_nUnMatch);
		Scribbling(str, RGB(0, 0, 255));
	}
}

BOOL CTcm71Dlg::LoadMask()
{
	ZeroMemory(m_nMask, SIZE_MASK);

	CString strFile;
	m_editMask.GetWindowText(strFile);
	if (strFile.IsEmpty())	return FALSE;
	CFile file;
	CFileException error;
	if (!file.Open(strFile, CFile::modeRead, &error))
		return FALSE;
	DWORD dw = (DWORD)file.GetLength();
	char* pBuf = new char[dw + 10];
	ZeroMemory(pBuf, dw + 10);
	file.Read(pBuf, dw);
	file.Close();

	CString str;
	str.Format(_T("%s"), (CA2CT)pBuf);
	delete [] pBuf;
	int n = 0;
	int fi;
	while (!str.IsEmpty()) {
		str = str.TrimLeft();
		if (str.GetLength() > 1 && str.GetAt(0) == '/' && str.GetAt(1) == '/') {
			fi = str.Find('\n');
			if (fi >= 0)	str = str.Mid(fi + 1);
			else	str.Empty();
		}
		else {
			int x;
			sscanf_s((CT2CA)str, "%X", &x, sizeof(int));
			m_nMask[n ++] = (unsigned char)x;
			fi = str.FindOneOf(_T("; ,-|\r\n"));
			if (fi >= 0)	str = str.Mid(fi + 1);
			else	str.Empty();
		}
	}
	TRACE(_T("Load mask %d\r\n"), n);
	return TRUE;
}

BOOL CTcm71Dlg::StoreDestination(CString strBuf)
{
	CString strFile;
	m_editDest.GetWindowText(strFile);
	if (strFile.IsEmpty())	return FALSE;

	CFile file;
	CFileException error;
	CString str = _T("");
	if (strBuf != _T("")) {
		int size = strBuf.GetLength();
		char* pBuf = new char[size + 10];
		strcpy_s(pBuf, size + 10, (CT2CA)strBuf);
		if (file.Open(strFile, CFile::modeWrite, &error)) {
			file.SeekToEnd();
			file.Write(pBuf, size);
			file.Close();
			str.ReleaseBuffer();
			delete [] pBuf;
			return TRUE;
		}
		delete [] pBuf;
		return FALSE;
	}
	else {
		MakeDestHead(m_nChkFrame);
		int size = m_strDestHead.GetLength();
		char* pBuf = new char[size + 10];
		strcpy_s(pBuf, size + 10, (CT2CA)m_strDestHead);
		if (file.Open(strFile, CFile::modeCreate | CFile::modeWrite, &error)) {
			file.Write(pBuf, size);
			file.Close();
			m_strDestHead.ReleaseBuffer();
			delete [] pBuf;
			return TRUE;
		}
		delete [] pBuf;
		return FALSE;
	}
}

void CTcm71Dlg::MakeDestHead(int id)
{
	CString strText = _T("");
	CString strLine = _T("");
	for (int n = 0; n < m_arch[id].nLength; n ++) {
		CString strTextA;
		strTextA.Format(_T("%s"), (CA2CT)m_arch[id].block[n].name);
		CString strLineA = _T("");
		for (int m = 0; m < m_arch[id].block[n].nSize; m ++) {
			CString strTemp;
			strTemp.Format(_T("%02d "), m + 1);
			strLineA += strTemp;
		}
		int text = strTextA.GetLength();
		int line = strLineA.GetLength();
		if (text < line) {
			while (text < line) {
				strTextA += _T(" ");
				++ text;
			}
		}
		else if (text > line)
			strTextA = strTextA.Left(line);

		strText += strTextA;
		strLine += strLineA;
	}
	m_strDestHead = strText + _T("\r\n") + strLine + _T("\r\n");
}

CString CTcm71Dlg::MakeDestField(unsigned char* p, int si, int ei)
{
	CString str = _T("");
	while (si < ei) {
		unsigned char ch = *(p + si ++);
		CString strTemp;
		if (ch != 0xff)	strTemp.Format(_T("%02X "), ch);
		else	strTemp = _T("FF ");
		str += strTemp;
	}
	str += _T("\r\n");
	return str;
}


void CTcm71Dlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST, m_list);
	DDX_Control(pDX, IDC_RICHEDIT, m_edit);
	DDX_Control(pDX, IDC_COMBO, m_combo);
	DDX_Control(pDX, IDC_EDIT_CYCLE, m_editCycle);
	DDX_Control(pDX, IDC_EDIT_MASK, m_editMask);
	DDX_Control(pDX, IDC_EDIT_DEST, m_editDest);
	DDX_Control(pDX, IDC_CHECK_CMP, m_chkCmp);
	DDX_Control(pDX, IDC_EDIT_DU, m_editDu);
}

BEGIN_MESSAGE_MAP(CTcm71Dlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_WM_TIMER()
	ON_CBN_SELCHANGE(IDC_COMBO, &CTcm71Dlg::OnCbnSelchangeCombo)
	ON_MESSAGE(WM_SCONNECT, &CTcm71Dlg::OnConnect)
	ON_MESSAGE(WM_SRECEIVE, &CTcm71Dlg::OnReceive)
	ON_MESSAGE(WM_SCLOSE, &CTcm71Dlg::OnClose)
	ON_BN_CLICKED(IDC_BUTTON_MASK, &CTcm71Dlg::OnBnClickedButtonMask)
	ON_BN_CLICKED(IDC_BUTTON_DEST, &CTcm71Dlg::OnBnClickedButtonDest)
	ON_BN_CLICKED(IDOK, &CTcm71Dlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_CHECK_CMP, &CTcm71Dlg::OnBnClickedCheckCmp)
END_MESSAGE_MAP()


// CTcm71Dlg 메시지 처리기

BOOL CTcm71Dlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다. 응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	CString str;
	m_list.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	LV_COLUMN lc;
	for (int n = 0; n < 5; n ++) {
		lc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lc.fmt = LVCFMT_CENTER;
		if (!n) {
			str = _T("WWWWWWWWWWWWWWWW");
			lc.cx = m_list.GetStringWidth((LPCTSTR)str);
		}
		else {
			str = _T("WWW.WWW.WW");
			lc.cx = m_list.GetStringWidth((LPCTSTR)str);
		}
		lc.pszText = str.GetBuffer(str.GetLength());
		lc.iSubItem = n;
		m_list.InsertColumn(n, &lc);
		str.ReleaseBuffer();
	}

	CWinApp* pApp = AfxGetApp();
	str = pApp->GetProfileString(_T("Tcms_Monit"), _T("IP"), 0);
	if (str.IsEmpty())	str = m_strIp;

	CIp dlg;
	dlg.m_strIp = str;
	if (dlg.DoModal() != IDOK)	return FALSE;
	m_strIp = dlg.m_strIp;

	pApp->WriteProfileString(_T("Tcms_Monit"), _T("IP"), m_strIp);

	str = pApp->GetProfileString(_T("Tcms_Monit"), _T("Mask"), 0);
	m_editMask.SetWindowText(str);
	str = pApp->GetProfileString(_T("Tcms_Monit"), _T("Destination"), 0);
	m_editDest.SetWindowText(str);
	m_chkCmp.SetCheck(BST_UNCHECKED);

	Open();
	/*if (m_pClientSock != NULL)	delete m_pClientSock;
	m_pClientSock = new CClientSocket();
	if (m_pClientSock->Create()) {
		m_pClientSock->SetHwnd(this->m_hWnd);
		if (!m_pClientSock->Connect(m_strIp, 5000)) {
			int nError = GetLastError();
			if (nError != WSAEWOULDBLOCK) {
				CString str;
				str.Format(_T("Can not connecct.(%d)"), nError);
				MessageBox(str);
				m_pClientSock->Close();
				delete m_pClientSock;
				m_pClientSock = NULL;
				return FALSE;
			}
		}
	}
	else {
		MessageBox(_T("Socket create failure!"));
		delete m_pClientSock;
		m_pClientSock = NULL;
		return FALSE;
	}*/

	SetWindowText(m_strIp);
	m_editMask.GetWindowText(str);
	if (!str.IsEmpty())	OnBnClickedCheckCmp();

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CTcm71Dlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다. 문서/뷰 모델을 사용하는 MFC 응용 프로그램의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CTcm71Dlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CTcm71Dlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


BOOL CTcm71Dlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (pMsg->message == WM_KEYDOWN) {
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_DELETE ||
			pMsg->wParam == VK_ESCAPE || GetKeyState(VK_CONTROL)) {
			::TranslateMessage(pMsg);
			::DispatchMessage(pMsg);
			return TRUE;
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CTcm71Dlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (nIDEvent == TIMER_WATCH) {
		//if (m_bTimer)	KillTimer(TIMER_WATCH);
		//SetTimer(TIMER_WATCH, WATCH_TIME, NULL);
		//m_bTimer = TRUE;
		//if (m_bOpen)	Scribbling(_T("Close.\r\n"), RGB(255, 0, 0));
		//Open();
	}

	CDialog::OnTimer(nIDEvent);
}

void CTcm71Dlg::OnCbnSelchangeCombo()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nChgFrame = m_combo.GetCurSel();
}

LRESULT CTcm71Dlg::OnConnect(WPARAM wParam, LPARAM lParam)
{
	Scribbling(_T("Connect A.\r\n"), RGB(255, 0, 0));
	if (m_bTimer)	KillTimer(TIMER_WATCH);
	SetTimer(TIMER_WATCH, WATCH_TIME, NULL);
	m_bTimer = TRUE;
	return 0L;
}

LRESULT CTcm71Dlg::OnReceive(WPARAM wParam, LPARAM lParam)
{
	if (m_pClientSock != NULL) {
		if (m_bTimer)	KillTimer(TIMER_WATCH);
		SetTimer(TIMER_WATCH, WATCH_TIME, NULL);
		m_bTimer = TRUE;
		int ri = m_pClientSock->Receive(&m_rBuf[m_uRecvi], SIZE_SBUF - m_uRecvi);
		TRACE(_T("Receive %d at %d\r\n"), ri, m_uRecvi);
		ri += m_uRecvi;
		int tabi;
		int	bi = 0;
		CString str;
		while ((tabi = FindHead(bi, ri)) >= bi) {
			int req = atoi((char*)&m_rBuf[tabi + 1]);
			int cur = ri - (tabi + 8);
			if (cur >= req) {
				clock_t start = clock();
				memcpy(m_cBuf, &m_rBuf[tabi + 8], req);
				Assort();
				bi = req + tabi + 8;
				clock_t finish = clock();
				double duration = (double)(finish - start) / CLOCKS_PER_SEC;
				str.Format(_T("%f"), duration);
				//m_editCycle.SetWindowText(str);
			}
			else {
				m_uRecvi = ri - tabi;
				if (tabi > 0)	memcpy(m_rBuf, &m_rBuf[tabi], m_uRecvi);
				break;
			}
		}
		if (tabi < bi)	m_uRecvi = 0;
	}
	return 0L;
}

LRESULT CTcm71Dlg::OnClose(WPARAM wParam, LPARAM lParam)
{
	Scribbling(_T("Close A.\r\n"), RGB(255, 0, 0));
	return 0L;
}

void CTcm71Dlg::OnBnClickedButtonMask()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CFileDialog dlg(TRUE, _T("txt"), NULL, OFN_LONGNAMES | OFN_READONLY, _T("Mask files(*.txt)|*.txt||"), NULL);
	if (dlg.DoModal() == IDOK) {
		CString str = dlg.GetPathName();
		m_editMask.SetWindowText(str);
	}
}

void CTcm71Dlg::OnBnClickedButtonDest()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CFileDialog dlg(FALSE, _T("txt"), NULL, OFN_LONGNAMES | OFN_READONLY, _T("Destination files(*.txt)|*.txt||"), NULL);
	if (dlg.DoModal() == IDOK) {
		CString str = dlg.GetPathName();
		m_editDest.SetWindowText(str);
	}
}

void CTcm71Dlg::OnBnClickedOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CWinApp* pApp = AfxGetApp();
	CString str;
	m_editMask.GetWindowText(str);
	pApp->WriteProfileString(_T("Tcms_Monit"), _T("Mask"), str);
	m_editDest.GetWindowText(str);
	pApp->WriteProfileString(_T("Tcms_Monit"), _T("Destination"), str);
	m_editDest.SetWindowText(str);

	OnOK();
}

void CTcm71Dlg::OnBnClickedCheckCmp()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_bChkFrame = FALSE;
	if (m_chkCmp.GetCheck() == BST_CHECKED) {
		if (!LoadMask()) {
			m_chkCmp.SetCheck(BST_UNCHECKED);
			Scribbling(_T("Not found mask file or invalid mask!\r\n"), RGB(255, 0, 0));
		}
		else {
			CString str;
			m_editDest.GetWindowText(str);
			if (str.IsEmpty())	OnBnClickedButtonDest();
			if (StoreDestination(_T(""))) {
				m_nUnMatch = 0;
				Scribbling(_T("Start comparison of block\r\n"), RGB(0, 0, 255));
				m_bChkFrame = TRUE;
			}
			else	Scribbling(_T("Can not start comparison of block\r\n"), RGB(255, 0, 0));
		}
	}
	else	Scribbling(_T("End comparison of block\r\n"), RGB(0, 0, 255));
}
