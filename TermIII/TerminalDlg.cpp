// TerminalDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Terminal.h"
#include "TerminalDlg.h"

#include "Open.h"
#include "CtrlChar.h"
#include "Sum.h"
#include "MacroEdit.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTerminalDlg dialog

#define	IsHex(c)	((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F'))

CTerminalDlg::CTerminalDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTerminalDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTerminalDlg)
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_nPort = -1;
	m_pat.Initial(0, 19200, 8, 0, 0, 0, 0);

	for (int i = 0; i < 9; i ++)
		m_flowByte[i] = flowByte[i];
	m_sumForm.nSumType = m_sumForm.nForm = m_sumForm.nLength = 0;
	m_sumForm.bTimeDisp = m_sumForm.bHexDisp = FALSE;
	for (int i = 0; i < MAX_MACRO; i ++) {
		m_macro[i].nAtt = 0;
		m_macro[i].nKey = 0;
		m_macro[i].strContent = _T("");
	}
	m_nMacroLength = 0;
	m_strMacroFileName = _T("Tmacro");
	m_pMacroEdit = NULL;

	m_nRi = 0;
	m_bRxWaiting = FALSE;
	m_bSumWithoutCtrl = TRUE;
}

BYTE CTerminalDlg::atoh(BYTE ch)
{
	if (ch > 'F')	ch &= 0xdf;
	ch -= '0';
	if (ch > 9)	ch -= 7;
	return ch;
}

void CTerminalDlg::SendH(CString str)
{
	int leng = str.GetLength();

	BYTE cHex = 0;
	bool bAlter = false;
	int ti = 0;
	for (int i = 0; i < leng; i ++) {
		TCHAR tch = str.GetAt(i);
		char ch = (char)tch;
		if (ch == '$') {
			if (bAlter) {
				m_tbuf[ti ++] = cHex;
				cHex = 0;
				bAlter = false;
			}
			BYTE sum = 0;
			for (int m = 0; m < ti; m ++)
				sum += m_tbuf[m];
			m_tbuf[ti ++] = sum;
		}
		else {
			cHex <<= 4;
			cHex |= atoh((char)ch);
			if (!bAlter)	bAlter = true;
			else {
				m_tbuf[ti ++] = cHex;
				cHex = 0;
				bAlter = false;
			}
		}
	}
	m_tbuf[ti] = NULL;
	Log(TRUE, m_tbuf, ti);
	m_port.Send(m_tbuf, ti);
}

void CTerminalDlg::Send(CString str)
{
	CString strTemp;

	int ti = 0;
	int leng = str.GetLength();

	TCHAR ch;
	BYTE hex;
	m_wSum = 0;
	for (int i = 0; i < leng; i ++) {
		if ((ch = str.GetAt(i)) == '%') {
			++ i;
			ch = str.GetAt(i);
			switch (ch) {
			case '1' :
				m_tbuf[ti] = m_flowByte[SOH];
				if (!m_bSumWithoutCtrl)	CalcSum(m_tbuf[ti]);
				++ ti;
				break;
			case '2' :
				m_tbuf[ti] = m_flowByte[STX];
				if (!m_bSumWithoutCtrl)	CalcSum(m_tbuf[ti]);
				++ ti;
				break;
			case '3' :
				m_tbuf[ti] = m_flowByte[ETX];
				if (!m_bSumWithoutCtrl)	CalcSum(m_tbuf[ti]);
				++ ti;
				break;
			case '4' :
				m_tbuf[ti] = m_flowByte[EOT];
				if (!m_bSumWithoutCtrl)	CalcSum(m_tbuf[ti]);
				++ ti;
				break;
			case '5' :
				m_tbuf[ti] = m_flowByte[ENQ];
				if (!m_bSumWithoutCtrl)	CalcSum(m_tbuf[ti]);
				++ ti;
				break;
			case '6' :
				m_tbuf[ti] = m_flowByte[ACK];
				if (!m_bSumWithoutCtrl)	CalcSum(m_tbuf[ti]);
				++ ti;
				break;
			case '7' :
				m_tbuf[ti] = m_flowByte[NAK];
				if (!m_bSumWithoutCtrl)	CalcSum(m_tbuf[ti]);
				++ ti;
				break;
			case '8' :
				m_tbuf[ti] = m_flowByte[CR];
				if (!m_bSumWithoutCtrl)	CalcSum(m_tbuf[ti]);
				++ ti;
				break;
			case '9' :
				if (!m_bSumWithoutCtrl)	m_tbuf[ti] = m_flowByte[LF];
				CalcSum(m_tbuf[ti ++]);
				break;
			case '0' :
				if (m_sumForm.nSumType == 2)	++ m_wSum;
				else if (m_sumForm.nSumType == 3)	-- m_wSum;
				if (m_sumForm.nForm != 0) {			// binary
					if (m_sumForm.nLength != 0) {	// word
						m_tbuf[ti ++] = (char)((m_wSum >> 8) & 0xff);
						m_tbuf[ti ++] = (char)(m_wSum & 0xff);
					}
					else {							// byte
						m_tbuf[ti ++] = (char)(m_wSum & 0xff);
					}
				}
				else {								// ascii
					if (m_sumForm.nLength != 0) {	// word
						//sprintf(&m_tbuf[ti], "%04X", m_wSum);
						sprintf_s(&m_tbuf[ti], sizeof(m_tbuf), "%04X", m_wSum);
						ti += 4;
					}
					else {							// byte
						//sprintf(&m_tbuf[ti], "%02X", (m_wSum & 0xff));
						sprintf_s(&m_tbuf[ti], sizeof(m_tbuf), "%02X", (m_wSum & 0xff));
						ti += 2;
					}
				}
				break;
			default :
				m_tbuf[ti] = ch;
				CalcSum(m_tbuf[ti ++]);
			}
		}
		else if (ch == '\\') {
			++ i;
			ch = str.GetAt(i);
			switch (ch) {
			case 't' :
				m_tbuf[ti ++] = '\t';
				break;
			case 'r' :
				m_tbuf[ti ++] = '\r';
				break;
			case 'n' :
				m_tbuf[ti ++] = '\n';
				break;
			case '\\' :
				m_tbuf[ti ++] = '\\';
				break;
			default :
				m_tbuf[ti ++] = ch;
				break;
			}
		}
		else if (ch == '[') {
			++ i;
			hex = 0;
			while (i < leng) {
				ch = str.GetAt(i);
				if (ch == ']')	break;
				hex <<= 4;
				hex |= atoh(ch);
				++ i;
			}
			m_tbuf[ti] = hex;
			CalcSum(m_tbuf[ti ++]);
		}
		else {
			m_tbuf[ti] = ch;
			CalcSum(m_tbuf[ti ++]);
		}
	}
	m_tbuf[ti] = NULL;
	Log(TRUE, m_tbuf, ti);
	m_port.Send(m_tbuf, ti);
	//if (m_sumForm.bTimeDisp) {
	//	SYSTEMTIME stime;
	//	GetSystemTime(&stime);
	//	CString strA;
	//	strA.Format(_T(":%02d:%03d "), stime.wSecond, stime.wMilliseconds);
	//	Log(strA, RGB(0, 0, 255));
	//}
}

void CTerminalDlg::CalcSum(TCHAR ch)
{
	if (m_sumForm.nSumType > 0)	m_wSum ^= (WORD)ch;
	else	m_wSum += (WORD)(ch & 0xff);
}

int CTerminalDlg::MacroFind(UINT nAtt, UINT nKey)
{
	for (int i = 0; i < MAX_MACRO; i ++) {
		if (m_macro[i].nKey == 0)	break;
		if (m_macro[i].nAtt == nAtt && m_macro[i].nKey == nKey)
			return i;
	}
	return -1;
}

void CTerminalDlg::Log(CString strLog, COLORREF color)
{
	if (strLog.IsEmpty())	return;

	CHARFORMAT cf;
	m_editRx.GetDefaultCharFormat(cf);
	cf.dwMask = CFM_COLOR;
	cf.dwEffects = NULL;
	cf.crTextColor = color;
	m_editRx.SetWordCharFormat(cf);
	m_editRx.SetSel(-1, -1);
	m_editRx.ReplaceSel(strLog);
	cf.crTextColor = RGB(0, 0, 0);
	m_editRx.SetWordCharFormat(cf);
}

void CTerminalDlg::Log(BOOL bPlace, char* pszStr, int leng)
{
	CString str = _T("");

	if (leng == 0)	leng = strlen(pszStr);
	for (int m = 0; m < leng; m ++) {
		CString strTemp;
		BYTE n = (BYTE)*pszStr;
		if (n < ' ' || n & 0x80 || m_sumForm.bHexDisp)
			strTemp.Format(_T("[%02X]"), n);
		else	strTemp.Format(_T("%c"), n);
		str += strTemp;
		++ pszStr;
	}
	Log(str, bPlace ? RGB(0, 0, 255) : RGB(0, 0, 0));
}

void CTerminalDlg::Log(UINT leng)
{
	CString str = _T("");
	for (UINT i = 0; i < leng; i ++) {
		CString strTemp;
		BYTE n = m_rbuf[i];
		if (n < ' ' || n & 0x80 || m_sumForm.bHexDisp)
			strTemp.Format(_T(" %02X"), n);
		else	strTemp.Format(_T(" %c-%02X"), n, n);
		str += strTemp;
	}
	Log(str);
}

void CTerminalDlg::Log(unsigned char ch)
{
	CString str;
	ch &= 0xff;
	//if (ch < ' ' || ch & 0x80 || m_sumForm.bHexDisp)
		str.Format(_T("[%02X]"), ch);
	//else	str.Format(_T("%c"), ch);
	Log(str);
	//if (ch == 0xa && m_sumForm.bTimeDisp) {
	//	SYSTEMTIME stime;
	//	GetSystemTime(&stime);
	//	str.Format(_T(":%02d:%03d "), stime.wSecond, stime.wMilliseconds);
	//	Log(str);
	//}
}

void CTerminalDlg::Print(BOOL bWhere, char* p, int leng)
{
	CString strTemp = _T("");

	if (bWhere)	strTemp += _T("<");
	if (leng < 1)	leng = strlen(p);
	for (int m = 0; m < leng; m ++) {
	//while (*p != NULL) {
		/*CString str;
		if (*p < ' ' || *p & 0x80) {
			BYTE n = *p;
			str.Format(_T("[%02X]"), n);
		}
		else	str.Format(_T("%c"), *p);*/

		CString str;
		BYTE n = *p;
		if (n < ' ' || n & 0x80 || m_sumForm.bHexDisp)
			str.Format(_T("[%02X]"), n);
		else	str.Format(_T("%c"), n);
		strTemp += str;
		++ p;
		/*CString str;
		BYTE n = *p;
		str.Format(_T("%02X,"), n);
		strTemp += str;
		++ p;*/
	}
	if (bWhere)	strTemp += _T(">");
	//m_editRx.AddString(strTemp);
}

void CTerminalDlg::Print(UINT leng)
{
	UINT i;

	CString strTemp = _T("");

	for (i = 0; i < leng; i ++) {
		CString str;
		BYTE n = m_rbuf[i];
		if (n < ' ' || n & 0x80)
			str.Format(_T(" %02X"), n);
		else	str.Format(_T(" %c-%02X"), n, n);
		strTemp += str;
	}
	//m_editRx.AddString(strTemp);
}

void CTerminalDlg::Print(unsigned char c)
{
	CString str;

	c &= 0xff;
	//str.Format(_T("%02X "), c & 0xff);
	//m_editRx.AddString(str);
	if (c < ' ' || c & 0x80 || m_sumForm.bHexDisp)
		str.Format(_T("[%02X]"), c);
	else	str.Format(_T("%c"), c);
	//if (c < 0x20)	str.Format(_T("<%X>"), c);
	//else	str.Format(_T("%c"), c);
	//m_editRx.AddString(str);
	if (c == 0xa && m_sumForm.bTimeDisp) {
		SYSTEMTIME stime;
		GetSystemTime(&stime);
		str.Format(_T(":%02d:%03d "), stime.wSecond, stime.wMilliseconds);
		//m_editRx.AddString(str);
	}
}

void CTerminalDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_TX, m_editTx);
	DDX_Control(pDX, IDC_EDIT_RX, m_editRx);
}

BEGIN_MESSAGE_MAP(CTerminalDlg, CDialog)
	//{{AFX_MSG_MAP(CTerminalDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(IDC_OPEN, OnOpen)
	ON_COMMAND(IDC_CONTROL_CHAR, OnControlChar)
	ON_COMMAND(IDC_CHKSUM, OnChksum)
	ON_COMMAND(IDC_MACRO_EDIT, OnMacroEdit)
	ON_WM_DESTROY()
	ON_COMMAND(IDC_CLEAR_RX, OnClearMonitor)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_MESSAGE(WM_RXCHAR, OnRxChar)
	ON_MESSAGE(WM_CLOSEMACRO, OnCloseMacro)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTerminalDlg message handlers

BOOL CTerminalDlg::PreTranslateMessage(MSG* pMsg) 
{
	// TODO: Add your specialized code here and/or call the base class
	if (pMsg->message == WM_KEYDOWN) {
		CWnd* pWnd = GetDlgItem(IDC_EDIT_TX);
		if (GetFocus() == pWnd) {
			if (pMsg->wParam == VK_RETURN) {
				CString str;
				m_editTx.GetWindowText(str);
				if (str.IsEmpty())
					MessageBox(_T("보낼 내용이 없습니다."), _T("송신 오류"),
						MB_OK | MB_ICONQUESTION);
				else	SendH(str);
				return TRUE;
			}
			else if (pMsg->wParam == VK_DELETE) {
				CString str;
				str = _T("");
				m_editTx.SetWindowText(str);
				return TRUE;
			}
			else if (pMsg->wParam > _T(' ')) {
				int n = 0;
				if (::GetKeyState(VK_SHIFT) < 0)	n |= SHIFT_ATT;
				if (::GetKeyState(VK_CONTROL) < 0)	n |= CONTROL_ATT;
				if (::GetKeyState(VK_MENU) < 0)		n |= ALT_ATT;
				if ((n = MacroFind(n, pMsg->wParam)) > -1) {
					CString str;
					/*m_editTx.GetWindowText(str);
					str += m_macro[n].strContent;*/
					str = m_macro[n].strContent;
					m_editTx.SetWindowText(str);
					return TRUE;
				}
			}
		}
		else if (pMsg->wParam == VK_ESCAPE)
			return TRUE;
	}
	if (CDialog::PreTranslateMessage(pMsg))
		return TRUE;
	return ((m_hAccelTable != NULL) && ::TranslateAccelerator(m_hWnd, m_hAccelTable, pMsg));
	//return CDialog::PreTranslateMessage(pMsg);
}

BOOL CTerminalDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
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

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	m_menu.LoadMenu(IDR_MENU_TERMINAL);
	SetMenu(&m_menu);

	m_hAccelTable = ::LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));

	m_editTx.EnableWindow(FALSE);

	CFile file;
	if (file.Open(m_strMacroFileName, CFile::modeRead)) {
		CWaitCursor wait;
		CArchive ar(&file, CArchive::load);
		Serialize(ar);
		//file.Close();
	}
	else	MessageBox(_T("매크로 파일을 불러올 수 없습니다."), _T("매크로 읽기 오류"),
		MB_OK | MB_ICONWARNING);
	SetTimer(RXGAP_TIMER, RXGAP_TIME, NULL);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTerminalDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CTerminalDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CTerminalDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CTerminalDlg::OnOpen() 
{
	// TODO: Add your command handler code here
	COpen dlg(this);

	dlg.m_pao = m_pat;
	if (dlg.DoModal() == IDOK) {
		m_pat = dlg.m_pao;

		//HWND hWnd = m_port.GetSafeHwnd();
		if (m_nPort >= 0)	m_port.Close();
		m_nPort = m_pat.nPort;

		int nResult;
		CString str;
		if ((nResult = m_port.Initial(this, m_pat.nPort, m_pat.dwBps, 
			//m_pat.nDatas, m_pat.nStps,m_pat.nParity, 
			m_pat.nDtr, m_pat.nRts)) > 0) {
			switch (nResult) {
			case 1 :
				str.Format(_T("%s"), _T("함수 SetCommStat()가 듣지 않음."));
				break;
			case 2 :
				str.Format(_T("%s"), _T("함수 GetCommStat()가 듣지 않음."));
				break;
			case 3 :
				str.Format(_T("%s"), _T("함수 SetCommMask()가 듣지 않음."));
				break;
			case 4 :
				str.Format(_T("%s"), _T("함수 SetCommTimeouts()가 듣지 않음."));
				break;
			default :
				str.Format(_T("%s"), _T("핸들을 가져올 수 없음."));
			}
			MessageBox(str, _T("통신 오류"), MB_OK | MB_ICONERROR);
		}
		else {
			if (!m_port.Takeoff()) {
				m_editTx.EnableWindow(TRUE);
				m_editTx.SetFocus();
				str.Format(_T("COM%d-%ld"), m_pat.nPort, m_pat.dwBps);
				SetWindowText(str);
			}
			else {
				MessageBox(_T("스레드를 열 수가 없습니다."), _T("통신 오류"),
					MB_OK | MB_ICONERROR);
			}
		}
	}
}

void CTerminalDlg::OnClearMonitor() 
{
	// TODO: Add your command handler code here
	//m_editRx.ClearAll();
	m_editRx.SetSel(0, -1);
	m_editRx.ReplaceSel(_T(""));
}

void CTerminalDlg::OnControlChar() 
{
	// TODO: Add your command handler code here
	CCtrlChar dlg(this);

	for (int i = 0; i < 8; i ++)
		dlg.m_pItemStr[i]->Format(_T("%02X"), m_flowByte[i]);

	if (dlg.DoModal() == IDOK) {
		CString str;
		BYTE n;
		for (int i = 0; i < 8; i ++) {
			str = (CString)*dlg.m_pItemStr[i];
			sscanf_s((LPCTSTR)str, _T("%x"), &n);
			m_flowByte[i] = n;
		}
	}
}

void CTerminalDlg::OnChksum() 
{
	// TODO: Add your command handler code here
	CSum dlg(this);

	dlg.m_nSumType = m_sumForm.nSumType;
	dlg.m_nForm = m_sumForm.nForm;
	dlg.m_nLength = m_sumForm.nLength;
	dlg.m_bTimeDisp = m_sumForm.bTimeDisp;
	dlg.m_bHexDisp = m_sumForm.bHexDisp;
	if (dlg.DoModal() == IDOK) {
		m_sumForm.nSumType = dlg.m_nSumType;
		m_sumForm.nForm = dlg.m_nForm;
		m_sumForm.nLength = dlg.m_nLength;
		m_sumForm.bTimeDisp = dlg.m_bTimeDisp;
		m_sumForm.bHexDisp = dlg.m_bHexDisp;
	}
}

void CTerminalDlg::OnMacroEdit() 
{
	// TODO: Add your command handler code here
	if (m_pMacroEdit == NULL) {
		m_pMacroEdit = new CMacroEdit(m_macro, this);
		m_pMacroEdit->Create(IDD_MACRO_DIALOG, this);
		m_pMacroEdit->ShowWindow(TRUE);
	}
}

LONG CTerminalDlg::OnCloseMacro(WPARAM wParam, LPARAM lParam)
{
	for (m_nMacroLength = 0; m_macro[m_nMacroLength].nKey != 0; m_nMacroLength ++);
	CFile file;
	if (file.Open(m_strMacroFileName, CFile::modeCreate | CFile::modeWrite)) {
		CWaitCursor wait;
		CArchive ar(&file, CArchive::store);
		Serialize(ar);
		//file.Close();
	}
	else	MessageBox(_T("매크로 파일을 저장할 수가 없습니다."), _T("매크로 저장 오류"),
		MB_OK | MB_ICONWARNING);

	delete m_pMacroEdit;
	m_pMacroEdit = NULL;
	return 0L;
}

void CTerminalDlg::OnDestroy() 
{
	CDialog::OnDestroy();
	
	// TODO: Add your message handler code here
	if (m_bRxWaiting)	KillTimer(RXGAP_TIMER);
	if (m_pMacroEdit != NULL) {
		delete m_pMacroEdit;
		m_pMacroEdit = NULL;
	}
}

LONG CTerminalDlg::OnRxChar(WPARAM ch, LPARAM port)
{
	if ((int)port != m_nPort)	return -1L;

	if (m_bRxWaiting)	KillTimer(RXGAP_TIMER);
	//m_rbuf[m_nRi ++] = (char)ch;
	//if (m_nRi > (MAX_BUFF - 10)) {
	//	m_rbuf[m_nRi] = NULL;
	//	Print(m_nRi);
	//	m_nRi = 0;
	//	m_bRxWaiting = FALSE;
	//}
	//else {
	//	SetTimer(RXGAP_TIMER, RXGAP_TIME, NULL);
	//	m_bRxWaiting = TRUE;
	//}
	Log((unsigned char)ch);
	m_bRxWaiting = FALSE;
	return 0L;
}

void CTerminalDlg::Serialize(CArchive& ar) 
{
	if (ar.IsStoring())
	{	// storing code
		ar.WriteCount(m_nMacroLength);
		for (int i = 0; i < m_nMacroLength; i ++)
			ar << m_macro[i].nAtt << m_macro[i].nKey << m_macro[i].strContent;
	}
	else
	{	// loading code
		m_nMacroLength = ar.ReadCount();
		for (int i = 0; i < m_nMacroLength; i ++)
			ar >> m_macro[i].nAtt >> m_macro[i].nKey >> m_macro[i].strContent;
	}
}

void CTerminalDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if (nIDEvent == RXGAP_TIMER) {
		KillTimer(RXGAP_TIMER);
		m_bRxWaiting = FALSE;
		m_rbuf[m_nRi] = NULL;
		Log(m_nRi);
		m_nRi = 0;
	}
	else	CDialog::OnTimer(nIDEvent);
}
