// Dts.cpp
#include "stdafx.h"
#include "Dts.h"
#include "DtsDlg.h"

#include "afxsock.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define	_CRTDBG_MAP_ALLOC
#include "crtdbg.h"

BEGIN_MESSAGE_MAP(CDtsApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CDtsApp::CDtsApp()
{
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
}

CDtsApp theApp;

BOOL CDtsApp::InitInstance()
{
	HANDLE hMutex = CreateMutex(NULL, TRUE, L"Dts.exe");
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		ReleaseMutex(hMutex);
		CWnd* pWnd;
		if (pWnd = CWnd::FindWindowW(NULL, L"Dts.exe")) {
			CWnd* pWndChild;
			pWndChild = pWnd->GetLastActivePopup();
			if (pWnd->IsIconic())	pWnd->ShowWindow(SW_RESTORE);
			pWndChild->SetForegroundWindow();
		}
		return FALSE;
	}
	ReleaseMutex(hMutex);

	AfxInitRichEdit2();
	if (!AfxSocketInit()) {
		AfxMessageBox(L"Window Socket initial failed!");
		return FALSE;
	}

	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	AfxEnableControlContainer();

	CShellManager *pShellManager = new CShellManager;

	SetRegistryKey(_T("로컬 응용 프로그램 마법사에서 생성된 응용 프로그램"));

	CDtsDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK) {
	}
	else if (nResponse == IDCANCEL) {
	}

	if (pShellManager != NULL)
	{
		delete pShellManager;
	}

	return FALSE;
}

