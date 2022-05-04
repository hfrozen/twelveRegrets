// tddDlg.cpp
#include "stdafx.h"
#include "tdd.h"
#include "tddDlg.h"
#include "afxdialogex.h"

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


CtddDlg::CtddDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CtddDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pDummy = NULL;
	m_dwDummy = 0;
	m_dwDumOfs = 0;
}

CtddDlg::~CtddDlg()
{
	if (m_pDummy != NULL)	delete [] m_pDummy;
}

void CtddDlg::Log(CString strLog, COLORREF crText)
{
	if (strLog.IsEmpty())	return;

	COleDateTime dt = COleDateTime::GetCurrentTime();
	CString str = dt.Format(_T("%H:%M:%S:"));
	str += strLog;

	CHARFORMAT cf;
	m_log.GetDefaultCharFormat(cf);
	cf.dwMask = CFM_COLOR;
	cf.dwEffects = NULL;
	cf.crTextColor = crText;
	m_log.SetWordCharFormat(cf);
	m_log.SetSel(-1, -1);
	m_log.ReplaceSel(str);
}

BYTE CtddDlg::ToBcd(BYTE c)
{
	return ((((c / 10) & 0xf) << 4) | (c % 10));
}

void CtddDlg::MakeBlock(BYTE* pBuf, int year, int mon, int day, int hour, int min, int ti)
{
	for (int n = 0; n < 300; n ++) {
		if ((m_dwDumOfs + SIZE_BUBBLEDUMMY) > m_dwDummy) {
			WORD w = (WORD)(m_dwDummy - m_dwDumOfs);
			memcpy(m_bubble.dummy, m_pDummy + m_dwDumOfs, w);
			m_dwDumOfs = SIZE_BUBBLEDUMMY - w;
			memcpy(&m_bubble.dummy[w], m_pDummy, m_dwDumOfs);
		}
		else {
			memcpy(m_bubble.dummy, m_pDummy + m_dwDumOfs, SIZE_BUBBLEDUMMY);
			m_dwDumOfs += SIZE_BUBBLEDUMMY;
		}
		m_bubble.t.year.a = ToBcd((BYTE)year);
		m_bubble.t.mon.a = ToBcd((BYTE)mon);
		m_bubble.t.day.a = ToBcd((BYTE)day);
		m_bubble.t.hour.a = ToBcd((BYTE)hour);
		m_bubble.t.min.a = ToBcd((BYTE)min);
		m_bubble.n.th.a = ToBcd((BYTE)(ti / 100));
		m_bubble.n.to.a = ToBcd((BYTE)(ti % 100));
		m_bubble.cnt = (BYTE)n;
		m_bubble.ctrl = 0x20;
		m_bubble.stx = 2;
		m_bubble.etx = 3;

		BYTE* p = &m_bubble.ctrl;
		m_bubble.bcco = m_bubble.bcce = 0;
		for (int n = 0; n < SIZE_BUBBLEDUMMY + 11; n ++) {
			if (n & 1)	m_bubble.bcco ^= *p;
			else	m_bubble.bcce ^= *p;
			++ p;
		}
		memcpy(pBuf, &m_bubble, sizeof(BUBBLE));
		pBuf += sizeof(BUBBLE);
	}
}

void CtddDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT, m_edit);
	DDX_Control(pDX, IDC_RICHEDIT, m_log);
	DDX_Control(pDX, IDC_BUTTON_MAKE, m_btnMake);
}

BEGIN_MESSAGE_MAP(CtddDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_MAKE, &CtddDlg::OnBnClickedButtonMake)
	ON_BN_CLICKED(IDC_BUTTON_TEST, &CtddDlg::OnBnClickedButtonTest)
END_MESSAGE_MAP()

BOOL CtddDlg::OnInitDialog()
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

	CString str;
	CFile file;
	CFileException error;
	if (file.Open(L"D://TrainProj//vc//tds//tds//TestDoc.docx", CFile::modeRead, &error)) {
		m_dwDummy = (DWORD)file.GetLength();
		m_pDummy = new BYTE[m_dwDummy];
		file.Read(m_pDummy, m_dwDummy);
		file.Close();
		str.Format(L"TestDoc reading OK (%dbytes)\n", m_dwDummy);
		Log(str);
		m_btnMake.EnableWindow(true);
	}
	else {
		Log(L"TesetDoc reading failure!\n", RGB(255, 0, 0));
		m_btnMake.EnableWindow(false);
	}
	return TRUE;
}

BOOL CtddDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
		return TRUE;
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CtddDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else {
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CtddDlg::OnPaint()
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
	else {
		CDialogEx::OnPaint();
	}
}

HCURSOR CtddDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CtddDlg::OnBnClickedButtonMake()
{
	if (m_pDummy == NULL || m_dwDummy < SIZE_BUBBLEDUMMY)	return;

	CString str;
	m_edit.GetWindowTextW(str);
	if (str.IsEmpty()) {
		MessageBox(L"날짜를 입력하시오!");
		return;
	}

	_int64 v = _atoi64((CT2CA)str);
	int ti = (int)(v % 10000);
	v /= 10000;
	int min = (int)(v % 100);
	v /= 100;
	int hour = (int)(v % 100);
	v /= 100;
	int day = (int)(v % 100);
	v /= 100;
	int mon = (int)(v % 100);
	v /= 100;
	int year = (int)v;
	str.Format(L"%02d/%02d/%02d %02d:%02d %04d에서 작성?", year, mon, day, hour, min, ti);
	if (MessageBox(str, L"tdd", MB_YESNO) != IDYES)	return;

	for (int n = 0; n < DUMMYFILE_LENGTH; n ++) {
		if (min > 0)	min -= 5;
		else {
			min = 55;
			if (hour > 0)	-- hour;
			else {
				hour = 23;
				if (day > 1)	-- day;
				else {
					day = 31;
					if (mon > 1)	-- mon;
					else {
						mon = 12;
						-- year;
					}
				}
			}
		}
	}
	str.Format(L"%02d/%02d/%02d %02d:%02d %04d에서 작성", year, mon, day, hour, min, ti);
	MessageBox(str);

	m_dwDumOfs = 0;
	BYTE buf[sizeof(BUBBLE) * 300];
	for (int n = 0; n < DUMMYFILE_LENGTH; n ++) {
		MakeBlock(buf, year, mon, day, hour, min, ti);
		CString strFile;
		strFile.Format(L"D://TrainProj//vc//dum//c%02d%02d%02d%02d%02d%04d", year, mon, day, hour, min, ti);
		str.Format(L"%04d-%s\n", n, strFile);
		Log(str);

		CFile file;
		CFileException error;
		if (file.Open(strFile, CFile::modeCreate | CFile::modeWrite, &error)) {
			file.Write(buf, sizeof(BUBBLE) * 300);
			file.Close();
		}
		else {
			Log(L"file creation failure!\n", RGB(255, 0, 0));
			return;
		}
		
		min += 5;
		if (min >= 60) {
			min = 0;
			if (++ hour >= 24) {
				hour = 0;
				if (++ day >= 31) {
					day = 1;
					if (++ mon >= 12) {
						mon = 1;
						++ year;
					}
				}
			}
		}
	}
}

void CtddDlg::OnBnClickedButtonTest()
{
	_OCTET oct;
	for (int n = 0; n < 8; n ++)	oct.c[n] = n + 1;

	CString str;
	str.Format(L"%#x %#x\n", oct.dw[1], oct.dw[0]);
	Log(str);
	for (int n = 0; n < 16; n ++) {
		oct.qw >>= 4;
		str.Format(L"%#x %#x\n", oct.dw[1], oct.dw[0]);
		Log(str);
		if ((oct.c[0] & 3) == 3)	Log(L"capture 0x3\n");
		else if ((oct.c[0] & 3) == 2)	Log(L"capture 0x2\n");
	}
	str.Format(L"%#x %#x\n", oct.dw[1], oct.dw[0]);
	Log(str);
}
