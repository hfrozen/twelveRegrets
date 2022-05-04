// tdsDlg.cpp
#include "stdafx.h"
#include "tds.h"
#include "tdsDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define	CRED	RGB(255, 0, 0)

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


CtdsDlg::CtdsDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CtdsDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pDummy = NULL;
	m_dwDummy = 0;
	m_dwDumOfs = 0;
	m_cDumCnt = 0;
}

CtdsDlg::~CtdsDlg()
{
	if (m_pDummy != NULL)	delete [] m_pDummy;
	if (m_port.IsOpen())	m_port.Close();
}

void CtdsDlg::Log(CString strLog, COLORREF crText)
{
	if (strLog.IsEmpty())	return;

	COleDateTime dt = COleDateTime::GetCurrentTime();
	CString str = dt.Format(_T("%H:%M:%S:"));
	str += strLog;

	CHARFORMAT cf;
	m_editLog.GetDefaultCharFormat(cf);
	cf.dwMask = CFM_COLOR;
	cf.dwEffects = NULL;
	cf.crTextColor = crText;
	m_editLog.SetWordCharFormat(cf);
	m_editLog.SetSel(-1, -1);
	m_editLog.ReplaceSel(str);
}

BYTE CtdsDlg::ToBcd(BYTE c)
{
	return ((((c / 10) & 0xf) << 4) | (c % 10));
}

void CtdsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT_LOG, m_editLog);
	DDX_Control(pDX, IDC_EDIT_PORTNO, m_editPortNo);
	DDX_Control(pDX, IDC_BUTTON_CAST, m_btnCast);
	DDX_Control(pDX, IDC_BUTTON_RESET, m_btnReset);
	DDX_Control(pDX, IDC_EDIT_REALDATE, m_editRealDate);
	DDX_Control(pDX, IDC_EDIT_DBFDATE, m_editDbfDate);
}

BEGIN_MESSAGE_MAP(CtdsDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_CAST, &CtdsDlg::OnBnClickedButtonCast)
	ON_BN_CLICKED(IDC_BUTTON_RESET, &CtdsDlg::OnBnClickedButtonReset)
	ON_BN_CLICKED(IDC_BUTTON_CONV2DBFT, &CtdsDlg::OnBnClickedButtonConv2dbft)
END_MESSAGE_MAP()

BOOL CtdsDlg::OnInitDialog()
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

	m_editPortNo.SetWindowTextW(L"27");
	CString str;
	str.Format(L"bubble size %d\n", sizeof(m_bubble));
	Log(str);
	str.Format(L"bcdtime size %d\n", sizeof(BCDT));
	Log(str);

	CFile file;
	CFileException error;
	if (file.Open(L"D://TrainProj//vc//tds//tds//TestDoc.docx", CFile::modeRead, &error)) {
		m_dwDummy = (DWORD)file.GetLength();
		m_pDummy = new BYTE[m_dwDummy];
		file.Read(m_pDummy, m_dwDummy);
		file.Close();
		str.Format(L"TestDoc reading OK (%dbytes)\n", m_dwDummy);
		Log(str);
	}
	else	Log(L"TestDoc reading failure!\n", CRED);

	return TRUE;
}

void CtdsDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else {
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CtdsDlg::OnPaint()
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

HCURSOR CtdsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

BOOL CtdsDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN && (pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
		return TRUE;
	return CDialogEx::PreTranslateMessage(pMsg);
}

BOOL CtdsDlg::OnCommand(WPARAM wParam, LPARAM lParam)
{
	return CDialogEx::OnCommand(wParam, lParam);
}

void CtdsDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == CAST_TIMER) {
		COleDateTime dt = COleDateTime::GetCurrentTime();
		if (dt.GetSecond() != m_odt.GetSecond()) {
			m_odt = dt;
			bool bClose = false;
			if ((m_dwDumOfs + SIZE_BUBBLEDUMMY) > m_dwDummy) {
				WORD w = (WORD)(m_dwDummy - m_dwDumOfs);
				memcpy(m_bubble.dummy, m_pDummy + m_dwDumOfs, w);
				m_dwDumOfs = SIZE_BUBBLEDUMMY - w;
				memcpy(&m_bubble.dummy[w], m_pDummy, m_dwDumOfs);
				bClose = true;
			}
			else {
				memcpy(m_bubble.dummy, m_pDummy + m_dwDumOfs, SIZE_BUBBLEDUMMY);
				m_dwDumOfs += SIZE_BUBBLEDUMMY;
			}
			m_bubble.t.year.a = ToBcd((BYTE)(m_odt.GetYear() % 100));
			m_bubble.t.mon.a = ToBcd((BYTE)m_odt.GetMonth());
			m_bubble.t.day.a = ToBcd((BYTE)m_odt.GetDay());
			m_bubble.t.hour.a = ToBcd((BYTE)m_odt.GetHour());
			m_bubble.t.min.a = ToBcd((BYTE)m_odt.GetMinute());
			m_bubble.t.sec.a = ToBcd((BYTE)m_odt.GetSecond());
			m_bubble.n.th.a = ToBcd(12);
			m_bubble.n.to.a = ToBcd(34);
			m_bubble.cnt = m_cDumCnt ++;
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
			m_port.Send((char*)&m_bubble.stx, sizeof(BUBBLE));
			CString str;
			str.Format(L"cast %d\n", m_cDumCnt);
			Log(str);
			if (bClose)	Log(L"cast cycle turn\n");
		}
	}
	CDialogEx::OnTimer(nIDEvent);
}

void CtdsDlg::OnBnClickedButtonCast()
{
	CString str;
	m_btnCast.GetWindowTextW(str);
	if (!str.Compare(L"시작")) {
		m_btnReset.EnableWindow(false);
		m_btnCast.SetWindowTextW(L"중단");
		if (m_pDummy == NULL || m_dwDummy == 0) {
			Log(L"송신할 내용이 없습니다!\n", CRED);
			return;
		}
		if (!m_port.IsOpen()) {
			m_editPortNo.GetWindowTextW(str);
			int pn = atoi((CT2CA)str);
			int iRes = m_port.Initial(this, pn, 115200);
			if (iRes > 0) {
				str.Format(L"com.%d can not open!\n", pn);
				Log(str, CRED);
				return;
			}
			else {
				if (m_port.Takeoff()) {
					m_port.Close();
					str.Format(L"com.%d thread error!\n", pn);
					Log(str, CRED);
					return;
				}
			}
		}
		Log(L"전송 시작\n");
		SetTimer(CAST_TIMER, TIME_CAST, NULL);
		m_odt = COleDateTime::GetCurrentTime();
	}
	else {
		m_btnReset.EnableWindow(true);
		m_btnCast.SetWindowTextW(L"시작");
		KillTimer(CAST_TIMER);
		Log(L"전송 중단\n");
	}
}

void CtdsDlg::OnBnClickedButtonReset()
{
	m_dwDumOfs = 0;
	m_cDumCnt = 0;
}

void CtdsDlg::OnBnClickedButtonConv2dbft()
{
	CString str;
	m_editRealDate.GetWindowTextW(str);
	if (!str.IsEmpty()) {
	__int64 v = _atoi64((CT2CA)str);
		DBFTIME dbf;
		dbf.t.year = (WORD)(v / 100000000);
		v %= 100000000;
		dbf.t.mon = (WORD)(v / 1000000);
		v %= 1000000;
		dbf.t.day = (WORD)(v / 10000);
		v %= 10000;
		dbf.t.hour = (WORD)(v / 100);
		dbf.t.min = (WORD)(v % 100);
		dbf.t.sec = 0;

		str.Format(L"%08X", dbf.dw);
		m_editDbfDate.SetWindowTextW(str);
	}
}
