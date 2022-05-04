// Log.cpp
#include "stdafx.h"
#include "Dts.h"
#include "Log.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(CLog, CDialogEx)

CLog::CLog(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLog::IDD, pParent)
{
	m_pParent = pParent;
}

CLog::~CLog()
{
}

void CLog::Repos(int cx, int cy)
{
	m_edit.SetWindowPos(NULL, 0, 0, cx, cy, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
}

void CLog::Trace(CString strLog, COLORREF crText)
{
	if (strLog == L"")	return;

	COleDateTime dt = COleDateTime::GetCurrentTime();
	CString str = dt.Format(L"%H:%M:%S ");
	str += strLog;

	if (m_edit.GetSafeHwnd()) {
		CHARFORMAT cf, cfb;
		m_edit.GetDefaultCharFormat(cf);
		memcpy(&cfb, &cf, sizeof(CHARFORMAT));
		cf.dwMask = CFM_COLOR;
		cf.dwEffects = NULL;
		cf.crTextColor = crText;
		m_edit.SetWordCharFormat(cf);
		m_edit.SetSel(-1, -1);
		m_edit.ReplaceSel(str);
		m_edit.SetDefaultCharFormat(cfb);
	}
}

void CLog::Track(CString strLog, COLORREF crText)
{
	if (strLog == L"")	return;

	if (m_edit.GetSafeHwnd()) {
		CHARFORMAT cf;	//, cfb;
		m_edit.GetDefaultCharFormat(cf);
		//memcpy(&cfb, &cf, sizeof(CHARFORMAT));
		cf.dwMask = CFM_COLOR;
		cf.dwEffects = NULL;
		cf.crTextColor = crText;
		m_edit.SetWordCharFormat(cf);
		m_edit.SetSel(-1, -1);
		m_edit.ReplaceSel(strLog);
		//m_edit.SetDefaultCharFormat(cfb);
	}
}

void CLog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT_LOG, m_edit);
}

BEGIN_MESSAGE_MAP(CLog, CDialogEx)
	ON_WM_DESTROY()
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

BOOL CLog::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	m_edit.SetOptions(ECOOP_XOR, ECO_SAVESEL);

	return TRUE;
}

BOOL CLog::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN &&
		(pMsg->wParam == VK_ESCAPE || pMsg->wParam == VK_RETURN))
		return TRUE;
	return CDialogEx::PreTranslateMessage(pMsg);
}

void CLog::OnDestroy()
{
	CDialogEx::OnDestroy();
}

BOOL CLog::OnEraseBkgnd(CDC* pDC)
{
	return CDialogEx::OnEraseBkgnd(pDC);
}
