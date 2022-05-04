// FindTroubles.cpp
#include "stdafx.h"
#include "Tranzx.h"
#include "FindTroubles.h"
#include "afxdialogex.h"

#include "MainFrm.h"
#include "TranzxDoc.h"
//#include "TranzxView.h"

IMPLEMENT_DYNAMIC(CFindTroubles, CDialogEx)

CFindTroubles::CFindTroubles(CWnd* pParent /*=NULL*/)
	: CDialogEx(CFindTroubles::IDD, pParent)
{
	m_pParent = pParent;
	m_bFindFolder = false;
	m_strFindPath = L"";
}

CFindTroubles::CFindTroubles(CWnd* pParent, bool bFolder)
	: CDialogEx(CFindTroubles::IDD, pParent)
{
	m_pParent = pParent;
	m_bFindFolder = bFolder;
	m_strFindPath = L"";
}

CFindTroubles::~CFindTroubles()
{
}

void CFindTroubles::Log(CString strLog, bool bPos)
{
	CMainFrame* pFrame = (CMainFrame*)m_pParent;
	pFrame->Log(strLog, bPos);
}

bool CFindTroubles::IsSysFolder(CString strPath, TCHAR* pFile)
{
	if ((_tcslen(pFile) == 1 && *(pFile + 0) == '.') ||
		(_tcslen(pFile) == 2 && *(pFile + 0) == '.' && *(pFile + 1) == '.') ||
		(PathIsSystemFolder(strPath + L"\\" + pFile, NULL)))
		return true;
	return false;
}

bool CFindTroubles::SearchCode(UINT code, CString strPath, TCHAR* pFile)
{
	CString strFile;
	strFile.Format(L"%s", pFile);

	CString str;
	strFile.Format(L"%s\\%s", strPath, pFile);
	CT2CA ascii(strFile);
	TRACE("%s search\n", ascii.m_psz);
	CFile file;
	CFileException error;
	if (!file.Open(strFile, CFile::modeRead, &error)) {
		TCHAR szError[1024];
		error.GetErrorMessage(szError, 1024);
		str.Format(L"[FIND] \"%s\"을 읽을 수 없습니다!(%s)", strPath, szError);
		Log(str, true);
		return false;
	}

	uint64_t size = file.GetLength();
	BYTE* pBuf = new BYTE[(unsigned int)size];
	if (pBuf == NULL) {
		file.Close();
		str = L"[FIND]Can not allocate read memory!";
		Log(str, true);
		return false;
	}
	ZeroMemory(pBuf, (unsigned int)size);
	file.Read(pBuf, (unsigned int)size);
	file.Close();

	uint64_t rem;
	if (code != SEARCH_QWORDCODE)	rem = size % SIZE_TROUBLEBLOCK;
	else	rem = (size - SIZE_LOGBOOKHEAD) % SIZE_LOGBOOKBLOCK;
	if (rem != 0) {
		delete [] pBuf;
		str.Format(L"[FIND] %s 파일 크기가 맞지 않습니다!(%u)", code != SEARCH_QWORDCODE ? L"고장" : L"운행", size);
		Log(str, true);
		return false;
	}

	BYTE* pSrc = pBuf;
	if (code != SEARCH_QWORDCODE) {
		uint64_t length = size / SIZE_TROUBLEBLOCK;
		for (uint64_t id = 0; id < length; id ++) {
			TROUBLEARCH trouble;
			memcpy(&trouble, pSrc, sizeof(TROUBLEARCH));
			if (!trouble.b.down && trouble.b.code == code) {
				strFile += L"\r\n";
				Log(strFile, true);
				delete [] pBuf;
				return true;
			}
			pSrc += SIZE_TROUBLEBLOCK;
		}
	}
	else {
		pSrc += SIZE_LOGBOOKHEAD;
		size -= SIZE_LOGBOOKHEAD;
		int max = (int)(size / SIZE_LOGBOOKBLOCK);
		for (int n = 0; n < max; n ++) {
			BYTE lc[SIZE_LOGBOOKBLOCK];
			memcpy(lc, pSrc, SIZE_LOGBOOKBLOCK);
			WORD seq = MAKEWORD(lc[0], lc[1]);
			QWORD qw = MAKEQWORD(&lc[256]);
			if (qw == 0xffffffffffffffff) {
				str.Format(L"%s %d dist. error!\r\n", strFile, seq);
				Log(str, true);
				delete [] pBuf;
				return true;
			}
			BYTE* p = &lc[378];
			for (int m = 0; m < 13; m ++) {
				QWORD qw = MAKEQWORD(p);
				if (qw == 0xffffffffffffffff) {
					str.Format(L"%s %d pwr%d error!\r\n", strFile, seq, m + 1);
					Log(str, true);
					delete [] pBuf;
					return true;
				}
				p += sizeof(QWORD);
			}
			pSrc += SIZE_LOGBOOKBLOCK;
			size -= SIZE_LOGBOOKBLOCK;
			TRACE("%d\n", n);
		}
	}
	delete [] pBuf;
	return false;
}

bool CFindTroubles::SearchFiles(UINT code, CString strParent)
{
	CString strPath = strParent + L"\\*";
	WIN32_FIND_DATA fd;
	HANDLE hFind = FindFirstFile(strPath, &fd);
	if (hFind == INVALID_HANDLE_VALUE) {
		CString strError;
		strError.Format(L"Invalid find handle %u.\r\n", hFind);
		Log(strError, true);
		return false;
	}

	if (!IsSysFolder(strParent, fd.cFileName)) {
		if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			SearchFiles(code, strParent + L"\\" + fd.cFileName);
		else if ((fd.cFileName[0] == FILETYPE_TROUBLE && code != SEARCH_QWORDCODE) ||
			(fd.cFileName[0] == FILETYPE_LOGBOOK && code == SEARCH_QWORDCODE)) {
			if (SearchCode(code, strParent, fd.cFileName))	++ m_nFindLength;
		}
	}

	while (FindNextFile(hFind, &fd) != 0) {
		if (!IsSysFolder(strParent, fd.cFileName)) {
			if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
				SearchFiles(code, strParent + L"\\" + fd.cFileName);
			else if ((fd.cFileName[0] == FILETYPE_TROUBLE && code != SEARCH_QWORDCODE) ||
				(fd.cFileName[0] == FILETYPE_LOGBOOK && code == SEARCH_QWORDCODE)) {
				if (SearchCode(code, strParent, fd.cFileName))	++ m_nFindLength;
			}
		}
	}

	DWORD dwError = GetLastError();
	FindClose(hFind);
	if (dwError != ERROR_NO_MORE_FILES) {
		CString strError;
		strError.Format(L"FindNextFiles error %u.\r\n", dwError);
		Log(strError, true);
		return false;
	}
	return true;
}

void CFindTroubles::SearchTrouble()
{
	CString strTrouble;
	GetDlgItem(IDC_EDIT_FINDTROUBLES_CODE)->GetWindowText(strTrouble);
	UINT code = (UINT)atoi(CT2CA(strTrouble));
	if ((code < 1 || code > 1000) && code != SEARCH_QWORDCODE) {
		MessageBox(L"고장 번호는 1 에서 999까지입니다!", L"고장 찾기", MB_ICONWARNING);
		return;
	}
	if (code == SEARCH_QWORDCODE)	m_bFindFolder = true;

	if (!m_bFindFolder)	m_pParent->SendMessage(WM_FINDTROUBLECODE, (WPARAM)code);	// find at view's list
	else {
		CString strPath;
		GetDlgItem(IDC_EDIT_FINDTROUBLES_FOLDER)->GetWindowText(strPath);
		if (strPath.IsEmpty()) {
			MessageBox(L"빈 파일입니다!", L"고장 찾기", MB_ICONWARNING);
			return;
		}

		CMainFrame* pFrame = (CMainFrame*)m_pParent;
		pFrame->DeleteLog(true);
		pFrame->SetTroubleCode(code);

		CString str;
		str.Format(L"찾기 시작 %d.\r\n", code);
		Log(str, true);
		m_nFindLength = 0;
		SearchFiles(code, strPath);
		str.Format(L"%d 찾음.\r\n", m_nFindLength);
		Log(str, true);
		m_strFindPath = strPath;
		Log(L"찾기 끝,", true);
		m_pParent->SendMessage(WM_CLOSEFINDTROUBLE);
	}
}

void CFindTroubles::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_FINDTROUBLES_FOLDER, m_editFolder);
	DDX_Control(pDX, IDC_BUTTON_FINDTROUBLES_FOLDER, m_btnFolder);
}

BEGIN_MESSAGE_MAP(CFindTroubles, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_FINDTROUBLES_FOLDER, &CFindTroubles::OnBnClickedButtonFolder)
	ON_BN_CLICKED(IDC_BUTTON_FINDTROUBLES_FIND, &CFindTroubles::OnBnClickedButtonFind)
	ON_BN_CLICKED(IDOK, &CFindTroubles::OnBnClickedOK)
	ON_BN_CLICKED(IDCANCEL, &CFindTroubles::OnBnClickedCancel)
END_MESSAGE_MAP()

BOOL CFindTroubles::OnInitDialog()
{
	CDialog::OnInitDialog();

	if (m_bFindFolder) {
		m_editFolder.EnableWindow(TRUE);
		m_btnFolder.EnableWindow(TRUE);
		GetDlgItem(IDC_EDIT_FINDTROUBLES_FOLDER)->SetWindowText(m_strFindPath);
	}
	else {
		m_editFolder.EnableWindow(FALSE);
		m_btnFolder.EnableWindow(FALSE);
		GetDlgItem(IDC_EDIT_FINDTROUBLES_FOLDER)->SetWindowTextW(L"");
	}

	return TRUE;
}

BOOL CFindTroubles::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN) {
		if (pMsg->wParam == VK_RETURN || pMsg->wParam == VK_DELETE ||
			pMsg->wParam == VK_ESCAPE || GetKeyState(VK_CONTROL)) {
			if (pMsg->wParam == VK_RETURN)
				SearchTrouble();

			::TranslateMessage(pMsg);
			::DispatchMessage(pMsg);
			return TRUE;
		}
	}

	return CDialog::PreTranslateMessage(pMsg);
}

void CFindTroubles::PostNcDestroy()
{
	delete this;
}

void CFindTroubles::OnBnClickedOK()
{
	if (m_bFindFolder)	GetDlgItem(IDC_EDIT_FINDTROUBLES_FOLDER)->GetWindowText(m_strFindPath);
	m_pParent->SendMessage(WM_CLOSEFINDTROUBLE);
	//DestroyWindow();
	OnOK();
}

void CFindTroubles::OnBnClickedCancel()
{
	if (m_bFindFolder)	GetDlgItem(IDC_EDIT_FINDTROUBLES_FOLDER)->GetWindowText(m_strFindPath);
	m_pParent->SendMessage(WM_CLOSEFINDTROUBLE);
	//DestroyWindow();
	//OnCancel();
}

void CFindTroubles::OnBnClickedButtonFolder()
{
	if (m_strFindPath == L"") {
		MessageBox(L"빈 폴더 입니다!", L"고장 찾기", MB_ICONWARNING);
		return;
	}
	LPSHELLFOLDER pShellFolder = NULL;
	OLECHAR wszPath[MAX_PATH] = { 0 };
	ULONG nCharsParsed = 0;
	LPITEMIDLIST lpIdl = NULL;

	HRESULT hres = SHGetDesktopFolder(&pShellFolder);
	if (FAILED(hres)) {
		MessageBox(L"데스크탑 폴더를 찾을 수 없습니다!", L"고장 찾기", MB_ICONWARNING);
		return;
	}
	BSTR bstrDesktop = m_strFindPath.AllocSysString();
	hres = pShellFolder->ParseDisplayName(NULL, NULL, bstrDesktop, &nCharsParsed, &lpIdl, NULL);
	if (FAILED(hres)) {
		MessageBox(L"루트 폴더를 찾을 수 없습니다!", L"고장 찾기", MB_ICONWARNING);
		return;
	}

	BROWSEINFO bi;
	ZeroMemory(&bi, sizeof(BROWSEINFO));
	bi.hwndOwner = GetSafeHwnd();
	bi.pidlRoot = lpIdl;
	//bi.pszDisplayName =
	bi.lpszTitle = L"Folder select";
	bi.ulFlags = BIF_RETURNONLYFSDIRS;

	LPITEMIDLIST lpIdlRes = SHBrowseForFolder(&bi);
	TCHAR szFolder[MAX_PATH * 2];
	szFolder[0] = L'\0';
	if (lpIdlRes) {
		if (SHGetPathFromIDList(lpIdlRes, szFolder)) {
			CString strPath = (LPCTSTR)szFolder;
			GetDlgItem(IDC_EDIT_FINDTROUBLES_FOLDER)->SetWindowText(strPath);
		}
	}
}

void CFindTroubles::OnBnClickedButtonFind()
{
	SearchTrouble();
}
