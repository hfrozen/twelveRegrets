// TroubleItem.cpp 
#include "StdAfx.h"
#include "TroubleItem.h"

#include "MainFrm.h"

CTroubleItem::CTroubleItem()
{
	m_bInit = false;
}

CTroubleItem::~CTroubleItem()
{
}

void CTroubleItem::Log(CString str)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	ASSERT(pFrame);
	pFrame->Log(str);
	CString strLog = pFrame->GetRoot();
	strLog += L"\\log.txt";
	CFile file;
	CFileException error;
	if (file.Open(strLog, CFile::modeCreate | CFile::modeNoTruncate | CFile::modeWrite | CFile::shareDenyNone, &error)) {
		file.SeekToEnd();

		str += L"\r\n";
		BSTR bstrInfo = str.AllocSysString();
		char* pBuf = _com_util::ConvertBSTRToString(bstrInfo);
		size_t leng = strlen(pBuf);
		file.Write(pBuf, leng);
		file.Close();
		delete[] pBuf;
	}
}

bool CTroubleItem::MakeTroubleItem(CString strPath)
{
	CString str;
	CFile file;
	CFileException error;
	if (!file.Open(strPath, CFile::modeRead, &error)) {
		TCHAR szError[1024];
		error.GetErrorMessage(szError, 1024);
		str.Format(L"[TITEM] \"%s\"을 읽을 수 없습니다!(%s).", strPath, szError);
		Log(str);
		AfxMessageBox(str, MB_ICONWARNING);
		return false;
	}

	int	size = (int)file.GetLength();
	BYTE* pBuf = new BYTE[(unsigned int)size];
	if (pBuf == NULL) {
		file.Close();
		str = L"[TITEM] 읽기 메모리를 할당할 수 없습니다!";
		Log(str);
		AfxMessageBox(str, MB_ICONWARNING);
		return false;
	}
	ZeroMemory(pBuf, (unsigned int)size);
	file.Read(pBuf, (unsigned int)size);
	file.Close();

	for (int n = 0; n < SIZE_TROUBLEITEMS; n ++) {
		//m_troubleItems[n].code = -1;
		m_troubleItems[n].strText =
		m_troubleItems[n].strOccurence =
		m_troubleItems[n].strDestroy = L"";
	}

	CString strTemp(pBuf);
	int leng = 0;
	while(!strTemp.IsEmpty()) {
		strTemp.TrimLeft();
		int n = strTemp.FindOneOf(L"\r\n");
		if (n < 0)	break;
		CString strLine = strTemp.Left(n);
		strTemp = strTemp.Mid(n + 1);
		if ((n = strLine.Find(L',')) < 5) {
			CString strCol[12];
			for (int m = 0; m < 12; m ++)	strCol[m] = L"";
			for (int m = 0; m < 12; m ++) {
				int x = strLine.Find(L',');
				if (x < 0 && m == 10 && !strLine.IsEmpty()) {
					strCol[10] = strLine;
					break;
				}
				strCol[m] = strLine.Left(x);
				strLine = strLine.Mid(x + 1);
			}
			if ((n = atoi(CT2CA(strCol[0]))) > 1 && !strCol[3].IsEmpty()) {
				if (m_troubleItems[n].strText.IsEmpty()) {
					m_troubleItems[n].strText = strCol[3];
					m_troubleItems[n].strOccurence = strCol[9];
					m_troubleItems[n].strDestroy = strCol[10];
					++ leng;
					//TRACE("%d %s\n", n, m_troubleItems[n].strText);
				}
				else {
					str.Format(L"[MITEM] 중복 번호입니다!(%d)", n);
					Log(str);
					AfxMessageBox(str, MB_ICONWARNING);
				}
			}
		}
	}
	delete [] pBuf;
	if (leng < 10) {
		str = L"[MITEM] 고장 번호가 부족합니다!";
		Log(str);
		AfxMessageBox(str, MB_ICONWARNING);
		return false;
	}
	str.Format(L"[DOC] 고장 번호 %d개를 등록하였습니다.", leng);
	Log(str);
	return true;
}

void CTroubleItem::Initial()
{
	if (m_bInit)	return;
	bool bNewPath = false;
	while (!m_bInit) {
		CString strCodes = AfxGetApp()->GetProfileStringW(L"TranzxRef", L"TroubleCode");
		if (strCodes.IsEmpty()) {
			CFileDialog dlg(TRUE, NULL, NULL, OFN_HIDEREADONLY, L"Trouble(*.csv)|*.csv||");
			if (dlg.DoModal() == IDOK) {
				strCodes = dlg.GetPathName();
				bNewPath = true;
			}
		}
		m_bInit = MakeTroubleItem(strCodes);
		if (m_bInit && bNewPath)
			AfxGetApp()->WriteProfileStringW(L"TranzxRef", L"TroubleCode", strCodes);
	}
}

CString CTroubleItem::GetTroubleContext(int code)
{
	if (code > 1000)	return L"";
	return m_troubleItems[code].strText;
}
