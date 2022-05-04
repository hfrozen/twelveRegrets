// Slate.cpp
#include "stdafx.h"
#include "Tranzx.h"
#include "Slate.h"
#include "MainFrm.h"
#include "TranzxDoc.h"

IMPLEMENT_DYNAMIC(CSlate, CWnd)

BEGIN_MESSAGE_MAP(CSlate, CWnd)
END_MESSAGE_MAP()

CSlate::CSlate()
{
	for (int n = 0; n < SIZE_TROUBLETEXTLIST; n ++)
		m_troubleTextList[n].nCode = -1;
	m_bMakedTroubleTextList = false;

	m_pXel = NULL;
	m_pFindTrouble = NULL;
}

CSlate::~CSlate()
{
	if (m_pXel != NULL) {
		delete m_pXel;
		m_pXel = NULL;
	}
	if (m_pFindTrouble != NULL) {
		delete m_pFindTrouble;
		m_pFindTrouble = NULL;
	}
}

const BSTR CSlate::m_lpStick[] = {
	L"EB",	L"B7",	L"B6",	L"B5",
	L"B4",	L"B3",	L"B2",	L"B1",
	L"N",	L"P0.5",L"P1",	L"P2",
	L"P3",	L"P4",	L"NON"
};

const BSTR CSlate::m_lpInspMode[] = {
	L"NON",		L"PDT",		L"DAILY",	L"MONTHLY"
	L"EACH",	L"VIEW",	L"MAX"
};

const BSTR CSlate::m_lpInspOper[] = {
	L"NON",		L"OK",		L"NG",		L"TEST",
	L"통신 고장",	L"통신 고장",	L"통신 고장",	L"통신 고장"
};

void CSlate::Log(CString str)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	ASSERT(pFrame);
	pFrame->Log(str);
}

bool CSlate::PickupWord(BYTE* p, WORD offset, WORD& value)
{
	if (offset >= 768)	return false;
	BYTE n = *(p + offset);
	BYTE m = *(p + offset + 1);
	value = (((WORD)n << 8) & 0xff00) | ((WORD)m & 0xff);
	return true;
}

bool CSlate::ParseInspType(CString str, INSPF& insp)
{
	ZeroMemory(&insp, sizeof(INSPF));
	insp.wOffset = atoi((CT2CA)str);
	if (insp.wOffset == 0)	return false;
	int n = str.FindOneOf(L"-*&");
	if (n >= 0) {
		TCHAR type = str.GetAt(n);
		str = str.Mid(n + 1);
		switch (type) {
		case (TCHAR)'-' :
			insp.nType = INSPTYPE_BIT;
			insp.nIndex[0] = (BYTE)atoi((CT2CA)str);
			++ insp.nIndexLength;
			n = str.Find((TCHAR)',');
			if (n >= 0) {
				str = str.Mid(n + 1);
				insp.nIndex[1] = (BYTE)atoi((CT2CA)str);
				++ insp.nIndexLength;
				n = str.Find((TCHAR)',');
				if (n >= 0) {
					str = str.Mid(n + 1);
					insp.nIndex[2] = (BYTE)atoi((CT2CA)str);
					++ insp.nIndexLength;
				}
			}
			break;
		case (TCHAR)'*' :
			insp.nType = INSPTYPE_VALUE;
			insp.factor = atof((CT2CA)str);
			if (insp.factor == 0.0)	return FALSE;
			break;
		case (TCHAR)'&' :
			insp.nType = INSPTYPE_FORM;
			insp.nIndex[0] = (BYTE)atoi((CT2CA)str);
			break;
		default :
			return false;
			break;
		}
	}
	else	return false;

	return true;
}

void CSlate::InspectItem(CString str, int col, int row, BYTE* p)
{
	if (str == L"" || str.GetAt(0) != L'[' ||
		col < 0 || row < 0 || p == NULL)	return;

	str = str.Mid(1);
	INSPF insp;
	if (ParseInspType(str, insp)) {
		if (insp.wOffset >= 919) {
			insp.wOffset -= 919;
			insp.wOffset *= 2;
		}
		WORD w;
		switch (insp.nType) {
		case INSPTYPE_BIT :
			if (PickupWord(p, insp.wOffset, w)) {
				BYTE bf = 0;
				for (int n = 0; n < insp.nIndexLength; n ++) {
					if (n >= 2) {
						if (!PickupWord(p, insp.wOffset + 2, w)) {
							w = 0;
							insp.nIndex[n] -= 16;
						}
					}
					if (w & (1 << insp.nIndex[n]))	bf |= (1 << n);
				}
				switch (insp.nIndexLength) {
				case 1 :
					str = bf != 0 ? L"ON" : L"OFF";
					m_pXel->SetCellValue(col, row, str);
					break;
				case 2 :
				case 3 :
					str.Format(L"%s", (LPCTSTR)m_lpInspOper[bf]);
					m_pXel->SetCellValue(col, row, str);
					break;
				default :
					break;
				}
			}
			break;
		case INSPTYPE_VALUE :
			if (PickupWord(p, insp.wOffset, w)) {
				double v = (double)w * insp.factor;
				if (insp.factor < 1) {
					if (insp.factor < 0.1)	str.Format(L"%.2f", v);
					else	str.Format(L"%.1f", v);
				}
				else	str.Format(L"%u", w);
				m_pXel->SetCellValue(col, row, str);
			}
			break;
		case INSPTYPE_FORM :
			switch (insp.nIndex[0]) {
			case 0 :
				str.Format(L"%02x%02x-%02x-%02x",
					(BYTE)(*(p + insp.wOffset)), (BYTE)(*(p + insp.wOffset + 1)),
					(BYTE)(*(p + insp.wOffset + 2)), (BYTE)(*(p + insp.wOffset + 3)));
				m_pXel->SetCellValue(col, row, str);
				break;
			case 1 :
				str.Format(L"%02x:%02x:%02x",
					(BYTE)(*(p + insp.wOffset)), (BYTE)(*(p + insp.wOffset + 1)),
					(BYTE)(*(p + insp.wOffset + 2)));
				m_pXel->SetCellValue(col, row, str);
				break;
			case 2 :
				if (PickupWord(p, insp.wOffset, w)) {
					if (w >= (SIZE_INSPMODE - 1))	w = SIZE_INSPMODE - 1;
					str.Format(L"%s", (LPCTSTR)m_lpInspMode[w]);
					m_pXel->SetCellValue(col, row, str);
				}
				break;
			case 3 :
				if (PickupWord(p, insp.wOffset, w)) {
					if (w >= (SIZE_STICK - 1))	w = SIZE_STICK - 1;
					str.Format(L"%s", (LPCTSTR)m_lpStick[w]);
					m_pXel->SetCellValue(col, row, str);
				}
				break;
			default :
				break;
			}
			break;
		default :
			break;
		}
	}
}

void CSlate::MakeTroubleTextList()
{
	if (m_bMakedTroubleTextList)	return;
	if (m_pXel != NULL)	delete m_pXel;
	m_pXel = new CXlezA(FALSE);

	int xrow, lrow;
	xrow = 3;
	lrow = 0;
	CString strFile = L"D:/TcmsWork/Hduw/TroubleCodeList.xlsx";
	if (m_pXel->OpenFile(strFile)) {
		int xcode;
		do {
			CString str = m_pXel->GetCellValue(1, xrow);
			xcode = 0;
			if (!str.IsEmpty()) {
				xcode = atoi((CT2CA)str);
				if (xcode > 0 && xcode < 1025) {
					CString strText = m_pXel->GetCellValue(4, xrow);
					if (!strText.IsEmpty()) {
						m_troubleTextList[lrow].nCode = xcode;
						m_troubleTextList[lrow].strText = strText;
						m_troubleTextList[++ lrow].nCode = -1;
					}
				}
				else	xcode = -1;
			}
			else	xcode = -1;
			++ xrow;
		} while (xcode > -1);
		m_pXel->CloseFile();
		//if (lrow > 0)	m_bMakedTroubleTextList = true;
		m_bMakedTroubleTextList = true;
	}
}

CString CSlate::GetTroubleText(int code)
{
	if (!m_bMakedTroubleTextList)	return L"";

	int row = 0;
	while (m_troubleTextList[row].nCode > -1) {
		if (m_troubleTextList[row].nCode == code)
			return m_troubleTextList[row].strText;
		++ row;
	}
	return L"";
}

bool CSlate::OpenFile(CString strPath, UINT nType, bool bLog)
{
	if (strPath.IsEmpty() || nType >= FILETYPE_MAX)	return false;

	CString str;
	CFile file;
	CFileException error;
	if (!file.Open(strPath, CFile::modeRead, &error)) {
		if (bLog) {
			TCHAR szError[1024];
			error.GetErrorMessage(szError, 1024);
			str.Format(L"Can not read %s(%s).", strPath, szError);
			Log(str);
			AfxMessageBox(str, MB_ICONWARNING);
		}
		return false;
	}

	uint64_t size = file.GetLength();
	BYTE* pBuf = new BYTE[(unsigned int)size];
	if (pBuf == NULL) {
		file.Close();
		if (bLog) {
			str = L"Can not allocate memory!";
			Log(str);
			AfxMessageBox(str, MB_ICONWARNING);
		}
		return false;
	}
	ZeroMemory(pBuf, (unsigned int)size);
	file.Read(pBuf, (unsigned int)size);
	file.Close();

	switch (nType) {
	case FILETYPE_TROUBLE :
		if (!(size % SIZE_TROUBLEBLOCK)) {
			m_uTrouble = MergeTrouble(pBuf, size / SIZE_TROUBLEBLOCK);
			if (m_uTrouble > 0) {
				m_strTroubleFileName = strPath;
				if (bLog) {
					str.Format(L"[DOC] Trouble file \"%s\"(%lld) read OK!", m_strTroubleFileName, m_uTrouble);
					Log(str);
				}
			}
			else {
				if (bLog) {
					str.Format(L"[DOC] Trouble file \"%s\" empty!!!", strPath);
					Log(str);
					AfxMessageBox(str, MB_ICONWARNING);
				}
			}
		}
		else {
			if (bLog) {
				str.Format(L"Unmatch alarm data size %u.", size);
				Log(str);
				AfxMessageBox(str, MB_ICONWARNING);
			}
			delete [] pBuf;
			return false;
		}
		break;
	case FILETYPE_INSPECT :
		if (size == SIZE_INSPECTFILE) {
			if (m_pInspect != NULL)	delete [] m_pInspect;
			m_pInspect = pBuf;
			m_strInspectFileName = strPath;
			str.Format(L"[DOC] Inspect file \"%s\" read OK!", m_strInspectFileName);
			Log(str);
		}
		else {
			str.Format(L"Unmatch inspect data size %u.", size);
			Log(str);
			delete [] pBuf;
			AfxMessageBox(str, MB_ICONWARNING);
			return false;
		}
		break;
	default :
		delete [] pBuf;
		return false;
		break;
	}

	return true;
}
