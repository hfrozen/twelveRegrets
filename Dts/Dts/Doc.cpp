// Doc.cpp
#include "stdafx.h"
#include "Dts.h"
#include "Doc.h"

#include "Intro.h"

IMPLEMENT_DYNAMIC(CDoc, CWnd)

ARTICLE CDoc::m_article;

const ARTICLEITEM CDoc::m_articleItems[] = {
	{	ARTICLETYPE_WORD,	(PVOID)&(CDoc::m_article.wFbSig[0][0]),	L"LK1FBR:"	},
	{	ARTICLETYPE_WORD,	(PVOID)&(CDoc::m_article.wFbSig[0][1]),	L"LK1FBF:"	},
	{	ARTICLETYPE_WORD,	(PVOID)&(CDoc::m_article.wFbSig[1][0]),	L"LK2FBR:"	},
	{	ARTICLETYPE_WORD,	(PVOID)&(CDoc::m_article.wFbSig[1][1]),	L"LK2FBF:"	},
	{	ARTICLETYPE_WORD,	(PVOID)&(CDoc::m_article.wFbSig[2][0]),	L"CMKFBR:"	},
	{	ARTICLETYPE_WORD,	(PVOID)&(CDoc::m_article.wFbSig[2][1]),	L"CMKFBF:"	},
	{	ARTICLETYPE_WORD,	(PVOID)&(CDoc::m_article.wFbSig[3][0]),	L"ESKFBR:"	},
	{	ARTICLETYPE_WORD,	(PVOID)&(CDoc::m_article.wFbSig[3][1]),	L"ESKFBF:"	},
	{	ARTICLETYPE_NON,	NULL,	NULL	},
	{	ARTICLETYPE_NON,	NULL,	NULL	}
};

CDoc::CDoc()
{
	m_pParent = NULL;
	m_article.wFbSig[0][0] = m_article.wFbSig[0][1] =
	m_article.wFbSig[1][0] = m_article.wFbSig[1][1] = 0x3ff;
	m_article.wFbSig[2][0] = m_article.wFbSig[2][1] = 0x221;
	m_article.wFbSig[3][0] = m_article.wFbSig[3][1] = 0x48;
	m_bArticle = false;
}

CDoc::~CDoc()
{
}

void CDoc::Log(CString strLog, COLORREF crText)
{
	CIntro* pIntro = (CIntro*)m_pParent;
	ASSERT(pIntro);
	pIntro->Log(strLog, crText);
}

CString CDoc::CaptureText(PCWSTR pstrCapture, CString strText)
{
	CString strRes = L"";
	int n = strText.Find(pstrCapture);
	if (n >= 0) {
		strRes = strText.Mid(n + wcslen(pstrCapture));
		strRes.TrimLeft();
		n = strRes.FindOneOf(L";\r\n");
		if (n >= 0) {
			strRes = strRes.Left(n);
			strRes.TrimRight();
		}
	}
	strRes.Remove((TCHAR)'\"');
	return strRes;
}

void CDoc::OpenError(CString strFile, bool bDir, bool bRead)
{
	CString strError = bDir ? L"디렉토리 \"" : L"파일 \"";
	strError += strFile;
	strError += bRead ? L"\"을 열 수없습니다." : L"\"을 저장할 수 없습니다.";
	//MessageBox(strError, _T("경고"), MB_ICONWARNING);
	strError += L"\r\n";
	Log(strError, COLOR_RED);
}

bool CDoc::MakeDirectory(CString strDirectory)
{
	if (!CreateDirectory(strDirectory, NULL)) {
		if (GetLastError() != ERROR_ALREADY_EXISTS)	return false;
	}
	return true;
}

CString CDoc::MakePath(CString strPath)
{
	CString str;
	int n = strPath.Find(L"d:");
	if (n < 0)	str = ROOTPATH + strPath;
	else	str = strPath;

	CString strRoute = L"";
	if ((n = str.Find(L":/")) > -1) {
		strRoute = str.Left(n + 1);
		str = str.Mid(n + 2);
	}

	for (n = -1; (n = str.Find((TCHAR)'/')) > -1; ) {
		CString strTemp = str.Left(n);
		str = str.Mid(n + 1);
		strRoute += L"/";
		strRoute += strTemp;
		if (!MakeDirectory(strRoute)) {
			OpenError(strRoute, true, true);
			return L"";
		}
	}
	return strRoute + L"/" + str;
}

CString CDoc::LoadFile(CString strFile)
{
	if (strFile.IsEmpty())	return L"";

	strFile = MakePath(strFile);
	if (strFile == L"")	return strFile;

	CString strContext = L"";
	CFile file;
	CFileException error;
	if (file.Open(strFile, CFile::modeRead, &error)) {
		DWORD dw = (DWORD)file.GetLength();
		char* pBuf = new char[dw];
		file.Read(pBuf, dw);
		file.Close();
		strContext = (CString)pBuf;
		delete [] pBuf;
	}
	else	OpenError(strFile, false, true);
	return strContext;
}

void CDoc::SaveFile(CString strFile, CString strContext, bool bTail)
{
	if (strFile.IsEmpty() || strContext.IsEmpty())	return;
	strFile = MakePath(strFile);
	if (strFile == L"")	return;

	int leng = WideCharToMultiByte(CP_ACP, 0, strContext, -1, NULL, 0, NULL, NULL);
	char* pBuf = new char[leng];
	WideCharToMultiByte(CP_ACP, 0, strContext, -1, (char*)pBuf, leng, NULL, NULL);

	CFile file;
	CFileException error;
	UINT nFlags = CFile::modeCreate | CFile::modeWrite;
	if (bTail)	nFlags |= CFile::modeNoTruncate;

	if (file.Open(strFile, nFlags, &error)) {
		if (bTail)	file.SeekToEnd();
		file.Write(pBuf, strlen(pBuf));
		file.Close();
	}
	else	OpenError(strFile, false, false);
	delete [] pBuf;
}

void CDoc::LoadRule()
{
	Log(L"Load article file.\r\n");
	CString strContext = LoadFile(ARTICLEFILE);
	if (strContext.IsEmpty()) {
		Log(L"\tarticle file empty.\r\n");
		return;
	}
	for (int n = 0; m_articleItems[n].type != ARTICLETYPE_NON; n ++) {
		CString str = CaptureText(m_articleItems[n].pstrItem, strContext);
		PVOID pv = m_articleItems[n].pVoid;
		if (pv == NULL)	continue;
		switch (m_articleItems[n].type) {
		case ARTICLETYPE_BOOL :		(bool)(*((bool*)pv)) = str == L"0" ? false : true;	break;
		case ARTICLETYPE_BYTE :		(BYTE)(*((BYTE*)pv)) = (BYTE)atoi((CT2CA)str);		break;
		case ARTICLETYPE_WORD :		(WORD)(*((WORD*)pv)) = (WORD)atoi((CT2CA)str);		break;
		case ARTICLETYPE_DWORD :	(DWORD)(*((DWORD*)pv)) = (DWORD)atol((CT2CA)str);	break;
		case ARTICLETYPE_FLOAT :	(*((double*)pv)) = (double)atof((CT2CA)str);		break;
		default :	break;
		}
	}
	m_bArticle = true;
}

void CDoc::SaveRule()
{
	CString strContext = TEXT_ARTICLEIDENTITY;
	strContext += L"\r\n";
	for (int n = 0; m_articleItems[n].type != ARTICLETYPE_NON; n ++) {
		PVOID pv = m_articleItems[n].pVoid;
		if (pv == NULL)	continue;
		CString str;
		switch (m_articleItems[n].type) {
		case ARTICLETYPE_BOOL :		str.Format(L"\t%s%d;\r\n", m_articleItems[n].pstrItem, (bool)(*((bool*)pv)) ? 1 : 0);	break;
		case ARTICLETYPE_BYTE :		str.Format(L"\t%s%d;\r\n", m_articleItems[n].pstrItem, (BYTE)(*((BYTE*)pv)));			break;
		case ARTICLETYPE_WORD :		str.Format(L"\t%s%d;\r\n", m_articleItems[n].pstrItem, (WORD)(*((WORD*)pv)));			break;
		case ARTICLETYPE_DWORD :	str.Format(L"\t%s%ld;\r\n", m_articleItems[n].pstrItem, (DWORD)(*((DWORD*)pv)));		break;
		case ARTICLETYPE_FLOAT :	str.Format(L"\t%s%f;\r\n", m_articleItems[n].pstrItem, (double)(*((double*)pv)));		break;
		default :	str = L";\r\n";	break;
		}
		strContext += str;
	}
	strContext += TEXT_FILEENDIDENTITY;
	SaveFile(ARTICLEFILE, strContext);
	m_bArticle = true;
	Log(L"Save article file.\r\n");
}

BEGIN_MESSAGE_MAP(CDoc, CWnd)
END_MESSAGE_MAP()
