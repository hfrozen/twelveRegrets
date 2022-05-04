// Watcher.cpp
#include "stdafx.h"
#include "Watcher.h"
#include "WatcherDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

BEGIN_MESSAGE_MAP(CWatcherApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CWatcherApp::CWatcherApp()
{
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
}

CWatcherApp theApp;

BOOL CWatcherApp::InitInstance()
{
	/*HANDLE hMutex = CreateMutex(NULL, TRUE, L"Watcher.exe");
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		ReleaseMutex(hMutex);
		CWnd* pWnd, *pWndChild;
		if (pWnd = CWnd::FindWindowW(NULL, L"Watcher.exe")) {
			pWndChild = pWnd->GetLastActivePopup();
			if (pWnd->IsIconic())	pWnd->ShowWindow(SW_RESTORE);
			pWndChild->SetForegroundWindow();
		}
		return FALSE;
	}
	ReleaseMutex(hMutex);*/

	if (!AfxSocketInit()) {
		AfxMessageBox(_T("Socket creation failure!\r\n"));
		return FALSE;
	}
	AfxInitRichEdit2();

	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	CShellManager *pShellManager = new CShellManager;

	SetRegistryKey(_T("���� ���� ���α׷� �����翡�� ������ ���� ���α׷�"));

	CWatcherDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK) {
		// TODO: ���⿡ [Ȯ��]�� Ŭ���Ͽ� ��ȭ ���ڰ� ������ �� ó����
		//  �ڵ带 ��ġ�մϴ�.
	}
	else if (nResponse == IDCANCEL) {
		// TODO: ���⿡ [���]�� Ŭ���Ͽ� ��ȭ ���ڰ� ������ �� ó����
		//  �ڵ带 ��ġ�մϴ�.
	}

	if (pShellManager != NULL) {
		delete pShellManager;
	}

	return FALSE;
}

