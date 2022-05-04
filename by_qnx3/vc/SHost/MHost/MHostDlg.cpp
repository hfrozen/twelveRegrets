// MHostDlg.cpp
#include "stdafx.h"
#include "MHost.h"
#include "MHostDlg.h"
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

#define	MARGIN_X	64

CMHostDlg::CMHostDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMHostDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_pServerSock = NULL;
}

CMHostDlg::~CMHostDlg()
{
	if (m_pServerSock != NULL) {
		m_pServerSock->Close();
		delete m_pServerSock;
		m_pServerSock = NULL;
	}
	for (POSITION pos = m_twig.GetHeadPosition(); pos != NULL; ) {
		CNode* pNode = (CNode*)m_twig.GetNext(pos);
		if (pNode != NULL) {
			delete pNode;
			pNode = NULL;
		}
	}
	m_twig.RemoveAll();
}

void CMHostDlg::AlignScreen()
{
	CRect rect = m_rectBase;
	rect.OffsetRect(-m_rectBase.left, -m_rectBase.top);
	int leng = m_twig.GetCount();
	if (leng > 13) {
		CString str;
	}
	int x, y;
	x = y = 0;
	if (leng > 0) {
		int xb = m_pt.x / m_rectNode.Width();
		if (leng < xb) {
			y = 1;
			x = leng;
		}
		else {
			x = leng % xb;
			y = leng / xb;
			if (!x)	-- y;
			++ y;
			x = xb;
		}
		rect.right = rect.left + m_rectNode.Width() * x;
		rect.bottom += m_rectNode.Height() * y;
	}
	CPoint pt;
	pt.x = (m_pt.x - rect.Width()) / 2 + MARGIN_X;
	pt.y = (m_pt.y - rect.Height()) / 2;
	SetWindowPos(NULL, pt.x, pt.y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
	if (leng > 0) {
		CPoint cpt = pt;
		cpt.y += m_rectBase.Height();
		for (POSITION pos = m_twig.GetHeadPosition(); pos != NULL; ) {
			CNode* pNode = (CNode*)m_twig.GetNext(pos);
			if (pNode == NULL)	break;
			pNode->SetWindowPos(NULL, cpt.x, cpt.y, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
			cpt.x += m_rectNode.Width();
			if ((cpt.x + m_rectNode.Width()) > m_pt.x) {
				cpt.x = pt.x;
				cpt.y += m_rectNode.Height();
			}
		}
	}
}

void CMHostDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_LENGTH, m_editLength);
}

BEGIN_MESSAGE_MAP(CMHostDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_MESSAGE(WM_SACCEPT, &CMHostDlg::OnAccept)
	ON_MESSAGE(WM_NODECLOSE, &CMHostDlg::OnNodeClose)
END_MESSAGE_MAP()

BOOL CMHostDlg::OnInitDialog()
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

	m_pt.x = ::GetSystemMetrics(SM_CXSCREEN) - MARGIN_X;
	m_pt.y = ::GetSystemMetrics(SM_CYSCREEN);

	m_pServerSock = new CServerSock();
	m_pServerSock->SetHwnd(GetSafeHwnd());
	m_pServerSock->Create(50091);
	m_pServerSock->Listen();

	GetWindowRect(&m_rectBase);
	SetWindowTextW(L"¼­¹ö [50091]");

	return TRUE;
}

BOOL CMHostDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
		return TRUE;
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CMHostDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else	CDialogEx::OnSysCommand(nID, lParam);
}

void CMHostDlg::OnPaint()
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

HCURSOR CMHostDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CMHostDlg::OnAccept(WPARAM wParam, LPARAM lParam)
{
	CClientSock* pClientSock = (CClientSock*)lParam;
	CNode* pNode = new CNode(this);
	pNode->Create(IDD_DIALOG_CLIENT, this);
	pNode->ShowWindow(SW_SHOW);
	pNode->m_pClientSock = pClientSock;
	pNode->m_pClientSock->SetHwnd(pNode->GetSafeHwnd());
	m_twig.AddTail(pNode);
	pNode->GetWindowRect(&m_rectNode);
	AlignScreen();
	CString str;
	str.Format(L"%d", m_twig.GetCount());
	m_editLength.SetWindowTextW(str);
	str.Format(L"Node#%d", m_twig.GetCount());
	pNode->SetWindowTextW(str);
	return 0;
}

LRESULT CMHostDlg::OnNodeClose(WPARAM wParam, LPARAM lParam)
{
	CNode* pSelNode = (CNode*)lParam;
	POSITION posb;
	for (POSITION pos = m_twig.GetHeadPosition(); (posb = pos) != NULL; ) {
		CNode* pNode = (CNode*)m_twig.GetNext(pos);
		if (pNode == pSelNode) {
			delete pNode;
			pNode = NULL;
			m_twig.RemoveAt(posb);
			break;
		}
	}
	CString str;
	str.Format(L"%d", m_twig.GetCount());
	m_editLength.SetWindowTextW(str);
	AlignScreen();
	return 0;
}
