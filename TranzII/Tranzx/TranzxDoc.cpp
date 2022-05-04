// TranzxDoc.cpp
#include "stdafx.h"
#include "Tranzx.h"

#include "TranzxDoc.h"
#include "MainFrm.h"
#include "TroubleItem.h"

#include <time.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

IMPLEMENT_DYNCREATE(CTranzxDoc, CDocument)

BEGIN_MESSAGE_MAP(CTranzxDoc, CDocument)
END_MESSAGE_MAP()

#include "LogbookArch.h"
//#include "OperArchExt.h"

CTranzxDoc::CTranzxDoc()
{
	m_nType = FILETYPE_NON;
	m_strPathName = L"";
	m_pLogbook = NULL;
	m_pInspect = NULL;
	m_uLogbookLength = 0;
	m_uTroubleLength = 0;
	m_uScale = 0;
	ZeroMemory(m_scale, sizeof(_SCALE) * SIZE_SCALE);
	m_pRange = NULL;
	TRACE(L"CTranzxDoc()\n");
}

CTranzxDoc::~CTranzxDoc()
{
	DeleteLogbookList();
	DeleteTroubleList();
	if (m_pLogbook != NULL) {
		delete [] m_pLogbook;
		m_pLogbook = NULL;
	}
	if (m_pInspect != NULL) {
		delete [] m_pInspect;
		m_pInspect = NULL;
	}
	if (m_pRange != NULL) {
		delete [] m_pRange;
		m_pRange = NULL;
	}
	TRACE(L"~CTranzxDoc()\n");
}

//============================================================================

//const int CTranzxDoc::m_nFactor[SIZE_SCALE] = {
//	1, 2, 5, 10
//};
//
//const CString CTranzxDoc::m_strFactor[SIZE_SCALE] = {
//	L"DIV 1", L"DIV 2", L"DIV 5", L"DIV 10"
//};

void CTranzxDoc::DeleteLogbookList()
{
	for (POSITION pos = m_logbookList.GetHeadPosition(); pos != NULL; ) {
		PLOGBOOKBLOCK pLogBlock = (PLOGBOOKBLOCK)m_logbookList.GetNext(pos);
		if (pLogBlock != NULL) {
			delete pLogBlock;
			pLogBlock = NULL;
		}
	}
	m_logbookList.RemoveAll();
}

void CTranzxDoc::DeleteTroubleList()
{
	for (POSITION pos = m_troubleList.GetHeadPosition(); pos != NULL; ) {
		PTROUBLEARCH pTrouble = (PTROUBLEARCH)m_troubleList.GetNext(pos);
		if (pTrouble != NULL) {
			delete pTrouble;
			pTrouble = NULL;
		}
	}
	m_troubleList.RemoveAll();
}

//void CTranzxDoc::DeleteLog()
//{
//	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
//	ASSERT(pFrame);
//	CString strLog = pFrame->GetRoot();
//	strLog += L"\\log.txt";
//	DeleteFile(strLog);
//}
//
void CTranzxDoc::Log(CString str)
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

int CTranzxDoc::CountSetBit(BYTE ch)
{
	int leng = 0;
	for (int n = 0; n < 8; n ++) {
		if (ch & (1 << n))	++ leng;
	}
	return leng;
}

int CTranzxDoc::CountShiftToB0(BYTE ch)
{
	int shift = 0;
	for (int n = 0; n < 8; n ++) {
		if (ch & 1)	return shift;
		ch >>= 1;
		++ shift;
	}
	return 8;
}

//uint64_t CTranzxDoc::GetValidBegin()
//{
//	uint64_t begin = 0;
//	CTick tick, cur;
//	for (uint64_t id = 0; id < m_uLogbookLength; id ++) {
//		if (begin == 0) {
//			begin = id;
//			GetLogbookTime(id, tick);
//		}
//		else {
//			GetLogbookTime(id, cur);
//			if (tick != cur) {
//				++ tick;
//				if (tick != cur)	begin = id;
//				tick = cur;
//			}
//		}
//	}
//	return begin;
//}
//
void CTranzxDoc::InsertNullLogbook(int diff, BYTE* pBuf)
{
	if (diff < 5)	diff = 4;
	else if (diff < 60)	diff = 10;
	else	diff = 20;
	for (int n = 0; n < diff; n ++) {
		PLOGBOOKBLOCK pLogBlock = new LOGBOOKBLOCK;
		ZeroMemory(pLogBlock, sizeof(LOGBOOKBLOCK));
		pLogBlock->nCrevice = diff;
		memset(&pLogBlock->c[2], 0xff, sizeof(DWORD));
		m_logbookList.AddTail(pLogBlock);
	}
	InsertLogbook(pBuf);
	//CString str;
	//str.Format(L"[DOC] Insert Null Logbook!!!");
	//Log(str);
}

void CTranzxDoc::InsertLogbook(BYTE* pBuf, bool bHalf)
{
	PLOGBOOKBLOCK pLogBlock = new LOGBOOKBLOCK;
	pLogBlock->nCrevice = bHalf ? 1 : 0;
	memcpy(pLogBlock->c, pBuf, SIZE_LOGBOOKBLOCK);
	WORD chk = 0;
	int n = 0;
	for ( ; n < (SIZE_LOGBOOKBLOCK - sizeof(WORD)); n ++)
		chk += (WORD)pLogBlock->c[n];
	WORD sum = MAKEWORD(pLogBlock->c[n], pLogBlock->c[n + 1]);
	if (sum != chk) {
		++ m_nFailLogBlock;
		CString str;
		str.Format(L"[DOC] Failed logbook block %d(0x%04x-0x%04x)!!!", m_nFailLogBlock, sum, chk);
		Log(str);
	}
	m_logbookList.AddTail(pLogBlock);
}

void CTranzxDoc::ModifyTailLogbook(bool bHalf)
{
	POSITION pos = m_logbookList.GetTailPosition();
	PLOGBOOKBLOCK pLogBlock = (PLOGBOOKBLOCK)m_logbookList.GetNext(pos);
	pLogBlock->nCrevice = bHalf ? 1 : 0;
}

bool CTranzxDoc::LoadFile(CString strPath, bool bLog)
{
	CString str;
	str.Format(L"[DOC] \"%s\" 읽습니다.", strPath);
	Log(str);

	if (strPath.IsEmpty()) {
		str.Format(L"[DOC] 패스 이름이 비었습니다!");
		Log(str);
		if (bLog)	AfxMessageBox(str, MB_ICONWARNING);
		return false;
	}

	CString strFile = strPath.Mid(strPath.ReverseFind(_T('\\')) + 1);
	if (strFile.IsEmpty()) {
		str.Format(L"[DOC] 파일 이름이 비었습니다!");
		Log(str);
		if (bLog)	AfxMessageBox(str, MB_ICONWARNING);
		return false;
	}

	TCHAR type = strFile.GetAt(0);
	//if (type != FILETYPE_LOGBOOK && type != FILETYPE_TROUBLE &&
	//	type != FILETYPE_INSPECTPDT && type != FILETYPE_INSPECTDAILY && type != FILETYPE_INSPECTMONTHLY) {
	if (type != FILETYPE_LOGBOOK && type != FILETYPE_TROUBLE) {
		str.Format(L"[DOC] 알수 없는 파일입니다!(%s)", strFile);
		Log(str);
		if (bLog)	AfxMessageBox(str, MB_ICONWARNING);
		return false;
	}

	CFile file;
	CFileException error;
	if (!file.Open(strPath, CFile::modeRead, &error)) {
		TCHAR szError[1024];
		error.GetErrorMessage(szError, 1024);
		str.Format(L"[DOC] \"%s\"을 읽을 수 없습니다!(%s).", strPath, szError);
		Log(str);
		if (bLog)	AfxMessageBox(str, MB_ICONWARNING);
		return false;
	}

	uint64_t size = file.GetLength();
	BYTE* pBuf = new BYTE[(unsigned int)size];
	if (pBuf == NULL) {
		file.Close();
		str = L"[DOC] 읽기 메모리를 할당할 수 없습니다!";
		Log(str);
		if (bLog)	AfxMessageBox(str, MB_ICONWARNING);
		return false;
	}
	ZeroMemory(pBuf, (unsigned int)size);
	file.Read(pBuf, (unsigned int)size);
	file.Close();

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	ASSERT(pFrame);

	switch (type) {
	case FILETYPE_LOGBOOK :
		if (!((size - SIZE_LOGBOOKHEAD) % SIZE_LOGBOOKBLOCK)) {
			memcpy(m_logHead.c, pBuf, sizeof(LOGBOOKHEADU));
			size -= SIZE_LOGBOOKHEAD;
			BYTE* pTemp = pBuf + SIZE_LOGBOOKHEAD;

			DeleteLogbookList();
			m_nFailLogBlock = 0;
			size /= SIZE_LOGBOOKBLOCK;
			CTick prev, cur;
			bool bHalfSec = false;
			int nFlow = 0;
			int diff;
			int loss = 0;
			for (int n = 0; n < size; n ++) {
				if (GetLogbookTime(pTemp, cur)) {
					switch (nFlow) {
					case 1 :
						diff = cur.Differ(prev);
						if (diff < 2) {
							bHalfSec = diff ? false : true;
							ModifyTailLogbook(bHalfSec ? false : true);
							InsertLogbook(pTemp, bHalfSec);
							bHalfSec ^= true;
							++ prev;
							nFlow = 2;
							// * diff = 0	: ex)prev 1.0 -> cur 1.5	: next is 2.0 then ++ prev
							// * diff = 1	: ex)prev 1.5 -> cur 2.0	: next is 2.5 then prev = cur (++ prev)
						}
						else {
							CString str;
							str.Format(L"[DOC] Insert Null Logbook #1!!!");
							Log(str);
							InsertNullLogbook(cur.Differ(prev), pTemp);
							nFlow = 1;
							prev = cur;
							++ loss;
						}
						break;
					case 2 :
						if (prev == cur) {
							InsertLogbook(pTemp, bHalfSec);
							bHalfSec ^= true;
							if (!bHalfSec)	++ prev;
						}
						else {
							CString str;
							str.Format(L"[DOC] Insert Null Logbook %02d/%02d %02d:%02d:%02d -> %02d/%02d %02d:%02d:%02d!!!",
								prev.m_nMon, prev.m_nDay, prev.m_nHour, prev.m_nMin, prev.m_nSec,
								cur.m_nMon, cur.m_nDay, cur.m_nHour, cur.m_nMin, cur.m_nSec);
							Log(str);
							InsertNullLogbook(cur.Differ(prev), pTemp);
							nFlow = 1;
							prev = cur;
							++ loss;
						}
						break;
					default :
						InsertLogbook(pTemp);
						nFlow = 1;
						prev = cur;
						break;
					}
				}
				pTemp += SIZE_LOGBOOKBLOCK;
			}
			delete [] pBuf;

			m_uLogbookLength = m_logbookList.GetCount();
			m_pLogbook = new BYTE[(int)m_uLogbookLength * sizeof(LOGBOOKBLOCK)];
			if (m_pLogbook == NULL) {
				DeleteLogbookList();
				str = L"[DOC] 정렬 메모리를 할당할 수 없습니다!";
				Log(str);
				if (bLog)	AfxMessageBox(str, MB_ICONWARNING);
				return false;
			}
			pBuf = m_pLogbook;
			for (POSITION pos = m_logbookList.GetHeadPosition(); pos != NULL; ) {
				PLOGBOOKBLOCK pLogBlock = (PLOGBOOKBLOCK)m_logbookList.GetNext(pos);
				if (pLogBlock != NULL) {
					memcpy(pBuf, pLogBlock, sizeof(LOGBOOKBLOCK));
					pBuf += sizeof(LOGBOOKBLOCK);
				}
			}
			DeleteLogbookList();
			if (m_nFailLogBlock) {
				str.Format(L"[DOC] Failed logbook block %d!!!", m_nFailLogBlock);
				Log(str);
				if (bLog)	AfxMessageBox(str, MB_ICONWARNING);
			}

			CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
			ASSERT(pFrame);
			CString strConv = pFrame->GetRoot();
			strConv += L"\\ConvLog.txt";

			if (file.Open(strConv, CFile::modeCreate | CFile::modeWrite, &error)) {
				file.Write(m_pLogbook, (int)(m_uLogbookLength * sizeof(LOGBOOKBLOCK)));
				file.Close();
			}

			m_nType = type;
			m_strPathName = strPath;
			str.Format(L"[DOC] 운행 기록 \"%s\"(%lld) 읽기가 끝났습니다.", m_strPathName, m_uLogbookLength);
			Log(str);
			str.Format(L"[DOC] 잃어버린 값은 %d입니다.", loss);
			Log(str);

			ZeroMemory(m_scale, sizeof(_SCALE) * SIZE_SCALE);
			m_nRange = sizeof(m_lbiArchive) / sizeof(LBITEMARCH);
			if (m_pRange != NULL)	delete m_pRange;
			m_pRange = new RANGE[m_nRange];
			ZeroMemory(m_pRange, m_nRange * sizeof(RANGE));
			str.Format(L"[DOC] 범위 값을 %d에 설정합니다.", m_nRange);
		}
		else {
			delete [] pBuf;
			str.Format(L" 운행 기록 크기가 맞지않습니다!(%u)", size);
			Log(str);
			if (bLog)	AfxMessageBox(str, MB_ICONWARNING);
			return false;
		}
		pFrame->ShowProperties(false);
		pFrame->EnableSelectTrouble(false);
		break;

	case FILETYPE_TROUBLE :
		if (!(size % SIZE_TROUBLEBLOCK)) {
			m_uTroubleLength = MergeTrouble(pBuf, size / SIZE_TROUBLEBLOCK);
			delete [] pBuf;
			if (m_uTroubleLength > 0) {
				m_nType = type;
				m_strPathName = strPath;
				if (bLog) {
					str.Format(L"[DOC] 고장 기록 \"%s\"(%lld) 읽기가 끝났습니다.", m_strPathName, m_uTroubleLength);
					Log(str);
				}

				GETTROUBLEITEM(pTroubleItem);
				if (!pTroubleItem->GetInitialState())	pTroubleItem->Initial();
			}
			else {
				if (bLog) {
					str.Format(L"[DOC] 고장 기록 파일(\"%s\")이 비었습니다!!!", strPath);
					Log(str);
					AfxMessageBox(str, MB_ICONWARNING);
				}
			}
		}
		else {
			delete [] pBuf;
			if (bLog) {
				str.Format(L"[DOC] 고장 기록 파일 크기가 맞지않습니다!(%u)", size);
				Log(str);
				AfxMessageBox(str, MB_ICONWARNING);
			}
			return false;
		}
		pFrame->ShowProperties(true);
		pFrame->EnableSelectTrouble(true);
		pFrame->SetSelectTroubleState(TROUBLE_ALL);
		break;

	case FILETYPE_INSPECTPDT :
	case FILETYPE_INSPECTDAILY :
	case FILETYPE_INSPECTMONTHLY :
		/*if (size == SIZE_INSPECTFILE) {
			if (m_pInspect != NULL)	delete [] m_pInspect;
			m_pInspect = pBuf;
			m_strInspectFileName = strPath;
			str.Format(L"[DOC]Inspect file \"%s\" read OK!", m_strInspectFileName);
			Log(str);
		}
		else {
			delete [] pBuf;
			str.Format(L"Unmatch inspect data size %u.", size);
			Log(str);
			AfxMessageBox(str, MB_ICONWARNING);
			return false;
		}*/
		delete [] pBuf;
		break;
	default :
		delete [] pBuf;
		return false;
		break;
	}
	return true;
}

uint64_t CTranzxDoc::MergeTrouble(BYTE* pSrc, uint64_t uLength)
{
	DeleteTroubleList();
	uint64_t leng = 0;
	int loss = 0;
	int odd = 0;
	for (uint64_t id = 0; id < uLength; id ++) {
		TROUBLEARCH trouble;
		memcpy(&trouble, pSrc, sizeof(TROUBLEARCH));
		if (!trouble.b.down) {
			PTROUBLEARCH pTrouble = new TROUBLEARCH;
			pTrouble->a = trouble.a;
			m_troubleList.AddTail(pTrouble);
			++ leng;
		}
		else if (trouble.b.down) {
			if (!MergeDownTrouble(trouble))	++ loss;
		}
		else	++ odd;
		pSrc += sizeof(TROUBLEARCH);
	}
	CString str;
	str.Format(L"[DOC] 고장 코드를 합쳤습니다. length=%d, loss=%d, unknown=%d", leng, loss, odd);

	return leng;
}

bool CTranzxDoc::MergeDownTrouble(TROUBLEARCH tr)
{
	for (POSITION pos = m_troubleList.GetHeadPosition(); pos != NULL; ) {
		PTROUBLEARCH pTrouble = (PTROUBLEARCH)m_troubleList.GetNext(pos);
		if (pTrouble != NULL && !pTrouble->b.down && pTrouble->b.code == tr.b.code && pTrouble->b.cid == tr.b.cid) {
			pTrouble->b.dhour = tr.b.dhour;
			pTrouble->b.dmin = tr.b.dmin;
			pTrouble->b.dsec = tr.b.dsec;
			pTrouble->b.down = true;
			return true;
		}
	}
	return false;
}

bool CTranzxDoc::GetLogbookTime(BYTE* pBuf, CTick& time)
{
	if (pBuf == NULL)	return false;
	DBFTIME dbt;
	memcpy(&dbt.dw, pBuf + 2, sizeof(DWORD));
	time.Set((int)dbt.t.year + 2010, (int)dbt.t.mon, (int)dbt.t.day, (int)dbt.t.hour, (int)dbt.t.min, (int)dbt.t.sec);
	return true;
}

int CTranzxDoc::GetLogbookTime(uint64_t uID, CTick& time)
{
	if (m_pLogbook == NULL)	return -1;
	if (uID >= m_uLogbookLength)	return -1;

	int nCrevice;
	memcpy(&nCrevice, m_pLogbook + uID * sizeof(LOGBOOKBLOCK), sizeof(int));
	if (nCrevice > 1)	return nCrevice;

	DBFTIME dbt;
	memcpy(&dbt.dw, m_pLogbook + uID * sizeof(LOGBOOKBLOCK) + sizeof(int) + 2, sizeof(DWORD));
	time.Set((int)dbt.t.year + 2010, (int)dbt.t.mon, (int)dbt.t.day, (int)dbt.t.hour, (int)dbt.t.min, (int)dbt.t.sec);
	return nCrevice;
}

int64_t CTranzxDoc::GetLogbookIDByTime(CTick time)
{
	if (m_pLogbook == NULL)	return -1;
	for (uint64_t n = 0; n < m_uLogbookLength; n ++) {
		int nCrevice;
		memcpy(&nCrevice, m_pLogbook + n * sizeof(LOGBOOKBLOCK), sizeof(int));
		if (nCrevice > 1)	continue;

		DBFTIME dbt;
		memcpy(&dbt.dw, m_pLogbook + n * sizeof(LOGBOOKBLOCK) + sizeof(int) + 2, sizeof(DWORD));
		if (dbt.t.mon == time.m_nMon && dbt.t.day == time.m_nDay &&
			dbt.t.hour == time.m_nHour && dbt.t.min == time.m_nMin && dbt.t.sec == time.m_nSec)
			return n;
	}
	return -1;
}

//int CTranzxDoc::GetLogbookTimeFromScale(uint64_t uID, CTick& time)
//{
//	uint64_t offset = m_scale[m_uScale].uBeginID + uID * m_nFactor[m_uScale];
//	return GetLogbookTime(m_scale[m_uScale].uBeginID + uID * m_nFactor[m_uScale], time);
//}
//
//int CTranzxDoc::GetMaxXbyScale()
//{
//	int range = (int)(((m_scale[m_uScale].uEndID - m_scale[m_uScale].uBeginID) / m_nFactor[m_uScale]) + 1);
//	return range;
//}
//
//int CTranzxDoc::GetMaxXbyScale(int scale)
//{
//	if (scale >= SIZE_SCALE)	return 0;
//	int range = (int)(((m_scale[scale].uEndID - m_scale[scale].uBeginID) / m_nFactor[scale]) + 1);
//	return range;
//}
//

int64_t CTranzxDoc::GetLBArchivePscOffset(int nArchID)
{
	TCHAR type = GetLBArchiveType(nArchID);
	if (type != 'H' && type != 'S' && type != 'U' && type != 'K' && type != 'L')	return 0;

	return ((int64_t)(m_lbiArchive[nArchID].pscMax - m_lbiArchive[nArchID].pscMin));
}

int64_t CTranzxDoc::GetLBArchiveLogOffset(int nArchID)
{
	TCHAR type = GetLBArchiveType(nArchID);
	if (type != 'H' && type != 'S' && type != 'U' && type != 'K' && type != 'L')	return 0;

	return ((int64_t)(m_lbiArchive[nArchID].logMax - m_lbiArchive[nArchID].logMin));
}

int64_t CTranzxDoc::GetLBArchivePscMin(int nArchID)
{
	TCHAR type = GetLBArchiveType(nArchID);
	if (type != 'H' && type != 'S' && type != 'U' && type != 'K' && type != 'L')	return 0;

	return ((int64_t)m_lbiArchive[nArchID].pscMin);
}
//	if (m_lbiArchive[nArchID].pscMin != 0)	return ((int64_t)m_lbiArchive[nArchID].pscMin);
//	if ((m_pRange + nArchID)->pscMin != 0)	return ((m_pRange + nArchID)->pscMin);
//
//	int64_t min = 0x7fffffffffffffff;
//	uint64_t pos = 0;
//	for (uint64_t n = m_scale[0].uBeginID; n <= m_scale[0].uEndID; n ++) {
//		_variant_t var;
//		if (GetLogbookItem(n, nArchID, var)) {
//			switch (var.vt) {
//			case VT_I8 :
//				if (min > var.llVal) {
//					min = var.llVal;
//					pos = n;
//				}
//				break;
//			case VT_UI8 :
//				if (min > (int64_t)var.ullVal) {
//					min = (int64_t)var.ullVal;
//					pos = n;
//				}
//				break;
//			case VT_I4 :
//				if (min > (int64_t)var.lVal) {
//					min = (int64_t)var.lVal;
//					pos = n;
//				}
//				break;
//			case VT_UI4 :
//				if (min > (int64_t)var.ulVal) {
//					min = (int64_t)var.ulVal;
//					pos = n;
//				}
//				break;
//			case VT_I2 :
//				if (min > (int64_t)var.iVal) {
//					min = (int64_t)var.iVal;
//					pos = n;
//				}
//				break;
//			case VT_UI2 :
//				if (min > (int64_t)var.uiVal) {
//					min = (int64_t)var.uiVal;
//					pos = n;
//				}
//				break;
//			default :
//				if (min > (int64_t)var.bVal) {
//					min = (int64_t)var.bVal;
//					pos = n;
//				}
//				break;
//			}
//		}
//	}
//	//CString str;
//	//str.Format(L"[DOC]min = %lld(%u)", min, pos);
//	//Log(str);
//
//	return min;
//}

void CTranzxDoc::SetLBArchivePscMin(int nArchID, int64_t min)
{
	if (m_pRange != NULL && nArchID < m_nRange)	(m_pRange + nArchID)->pscMin = min;
}

int64_t CTranzxDoc::GetLBArchiveLogMin(int nArchID)
{
	TCHAR type = GetLBArchiveType(nArchID);
	if (type != 'H' && type != 'S' && type != 'U' && type != 'K' && type != 'L')	return 0;
	return m_lbiArchive[nArchID].logMin;
}

void CTranzxDoc::SetLBArchiveLogMin(int nArchID, int64_t min)
{
	if (m_pRange != NULL && nArchID < m_nRange)	(m_pRange + nArchID)->logMin = min;
}

int64_t CTranzxDoc::GetLBArchivePscMax(int nArchID)
{
	TCHAR type = GetLBArchiveType(nArchID);
	if (type != 'H' && type != 'S' && type != 'U' && type != 'K' && type != 'L')	return 0;

	return ((int64_t)m_lbiArchive[nArchID].pscMax);
}

//	if (m_lbiArchive[nArchID].pscMax != 0)	return ((int64_t)m_lbiArchive[nArchID].pscMax);
//	if ((m_pRange + nArchID)->pscMax != 0)	return ((m_pRange + nArchID)->pscMax);
//
//	int64_t max = 0x8000000000000000;
//	uint64_t pos = 0;
//	for (uint64_t n = m_scale[0].uBeginID; n <= m_scale[0].uEndID; n ++) {
//		_variant_t var;
//		if (GetLogbookItem(n, nArchID, var)) {
//			switch (var.vt) {
//			case VT_I8 :
//				if (max < var.llVal) {
//					max = var.llVal;
//					pos = n;
//				}
//				break;
//			case VT_UI8 :
//				if (max < (int64_t)var.ullVal) {
//					max = (int64_t)var.ullVal;
//					pos = n;
//				}
//				break;
//			case VT_I4 :
//				if (max < (int64_t)var.lVal) {
//					max = (int64_t)var.lVal;
//					pos = n;
//				}
//				break;
//			case VT_UI4 :
//				if (max < (int64_t)var.ulVal) {
//					max = (int64_t)var.ulVal;
//					pos = n;
//				}
//				break;
//			case VT_I2 :
//				if (max < (int64_t)var.iVal) {
//					max = (int64_t)var.iVal;
//					pos = n;
//				}
//				break;
//			case VT_UI2 :
//				if (max < (int64_t)var.uiVal) {
//					max = (int64_t)var.uiVal;
//					pos = n;
//				}
//				break;
//			default :
//				if (max < (int64_t)var.bVal) {
//					max = (int64_t)var.bVal;
//					pos = n;
//				}
//				break;
//			}
//		}
//	}
//	//CString str;
//	//str.Format(L"[DOC]max = %lld(%u)", max, pos);
//	//Log(str);
//
//	return max;
//}
//
void CTranzxDoc::SetLBArchivePscMax(int nArchID, int64_t max)
{
	if (m_pRange != NULL && nArchID < m_nRange)	(m_pRange + nArchID)->pscMax = max;
}

int64_t CTranzxDoc::GetLBArchiveLogMax(int nArchID)
{
	TCHAR type = GetLBArchiveType(nArchID);
	if (type != 'H' && type != 'S' && type != 'U' && type != 'K' && type != 'L')	return 0;

	return ((int64_t)m_lbiArchive[nArchID].logMax);
}

void CTranzxDoc::SetLBArchiveLogMax(int nArchID, int64_t max)
{
	if (m_pRange != NULL && nArchID < m_nRange)	(m_pRange + nArchID)->logMax = max;
}

int CTranzxDoc::GetLogbookItem(uint64_t uID, int nArchID, _variant_t& var)
{
	if (m_pLogbook == NULL)	return -1;
	if (uID >= m_uLogbookLength)	return -1;

	TCHAR type = GetLBArchiveType(nArchID);
	BYTE length = GetLBArchiveFlag(nArchID);
	WORD offset = GetLBArchiveOffset(nArchID);
	BYTE flag = 0;
	if (type == 'D') {
		flag = length;
		length = 1;
	}

	CTick temp;
	int space = GetLogbookTime(uID, temp);
	// 'H'-hex,			'S'-sign graph,		'U'-unsign graph
	// 'K'-numeric,		'L'-unsign numeric,	'D'-0/1
	if (space < 2) {
		if ((type == 'H' || type == 'S' || type == 'U' ||
			type == 'K' || type == 'L' || type == 'D') && length > 0 && offset > 0) {
			BYTE c = 0;
			_OCTET oct;
			oct.qw = 0;
			switch (length) {
			case 8 :	case 7 :	case 6 :	case 5 :
				for (int n = 0; n < length; n ++)
					oct.c[n] = *(m_pLogbook + uID * sizeof(LOGBOOKBLOCK) + sizeof(int) + offset + n);
				if (type == 'S' || type == 'K') {
					var.llVal = (int64_t)oct.qw;
					var.vt = VT_I8;
				}
				else {
					var.ullVal = (uint64_t)oct.qw;
					var.vt = VT_UI8;
				}
				break;
			case 4 :	case 3 :
				for (int n = 0; n < length; n ++)
					oct.c[n] = *(m_pLogbook + uID * sizeof(LOGBOOKBLOCK) + sizeof(int) + offset + n);
				if (type == 'S' || type == 'K') {
					var.lVal = (int32_t)oct.dw[0];
					var.vt = VT_I4;
				}
				else {
					var.ulVal = (uint32_t)oct.dw[0];
					var.vt = VT_UI4;
				}
				break;
			case 2 :
				for (int n = 0; n < length; n ++)
					oct.c[n] = *(m_pLogbook + uID * sizeof(LOGBOOKBLOCK) + sizeof(int) + offset + n);
				if (type == 'S' || type == 'K') {
					var.iVal = (int16_t)oct.w[0];
					var.vt = VT_I2;
				}
				else {
					var.uiVal = (uint16_t)oct.w[0];
					var.vt = VT_UI2;
				}
				break;
			case 1 :
				c = *(m_pLogbook + uID * sizeof(LOGBOOKBLOCK) + sizeof(int) + offset);
				var.bVal = c;
				var.vt = VT_UI1;
				break;
			default :
				return false;
				break;
			}
			if (type == 'D') {
				if (CountSetBit(flag) > 1) {
					var.bVal = c & flag;
					var.bVal >>= CountShiftToB0(flag);
					var.vt = VT_UI1;
				}
				else {
					var.bVal = c & flag ? 1 : 0;
					var.vt = VT_BOOL;
				}
			}
		}
	}
	return space;
}

//bool CTranzxDoc::GetLogbookItemFromScale(uint64_t uID, int nArchID, _variant_t& var)
//{
//	TCHAR type = GetLBArchiveType(nArchID);
//	if (type != 'D' && type != 'S' && type != 'U')
//		return GetLogbookItem(uID, nArchID, var);
//
//	uint64_t from, qtt;
//	switch (m_uScale) {
//	case 3 :	from = uID * 10 - 4;	qtt = 10;	break;
//	case 2 :	from = uID * 5 - 2;		qtt = 5;	break;
//	case 1 :	from = uID * 2;			qtt = 2;	break;
//	default :	from = uID;				qtt = 1;	break;
//	}
//
//	from += m_scale[m_uScale].uBeginID;
//	int length = 0;
//	int64_t sum = 0;
//	while (qtt > 0) {
//		if (from >= 0 && from < m_uLogbookLength) {
//			GetLogbookItem(from, nArchID, var);
//			switch (var.vt) {
//			case VT_I8 :	sum += var.llVal;	break;
//			case VT_UI8 :	sum += (int64_t)var.ullVal;	break;
//			case VT_I4 :	sum += (int64_t)var.lVal;	break;
//			case VT_UI4 :	sum += (int64_t)var.ulVal;	break;
//			case VT_I2 :	sum += (int64_t)var.iVal;		break;
//			case VT_UI2 :	sum += (int64_t)var.uiVal;	break;
//			default :		sum += (int64_t)var.bVal;		break;
//			}
//			++ length;
//		}
//		++ from;
//		-- qtt;
//	}
//	if (length == 0)	sum = 0;
//	else {
//		if (type == 'D')	sum /= m_nFactor[m_uScale];
//		else	sum /= length;
//	}
//	switch (var.vt) {
//	case VT_I8 :	var.llVal = sum;			break;
//	case VT_UI8 :	var.ullVal = (uint64_t)sum;	break;
//	case VT_I4 :	var.lVal = (int32_t)sum;	break;
//	case VT_UI4 :	var.ulVal = (uint32_t)sum;	break;
//	case VT_I2 :	var.iVal = (int16_t)sum;	break;
//	case VT_UI2 :	var.uiVal = (uint16_t)sum;	break;
//	default :		var.bVal = (uint8_t)sum;	break;
//	}
//	return true;
//}

//============================================================================

int CTranzxDoc::GetFrameNo()
{
	_DUET du;
	du.c[0] = *(m_pLogbook + m_scale[0].uBeginID * sizeof(LOGBOOKBLOCK) + sizeof(int) + 0);
	du.c[1] = *(m_pLogbook + m_scale[0].uBeginID * sizeof(LOGBOOKBLOCK) + sizeof(int) + 1);
	return du.w;
}

CString CTranzxDoc::FormatTimeRange()
{
	CString str = L"";
	str.Format(L"%d %d ", m_scale[0].uBeginID, m_scale[0].uEndID);
	if (m_scale[0].uBeginID < m_scale[0].uEndID) {
		CTick tick;
		if (GetLogbookTime(m_scale[0].uBeginID, tick)) {
			str.Format(L"%s ~", tick.Format());
			if (GetLogbookTime(m_scale[0].uEndID, tick)) {
				CString strTemp;
				strTemp.Format(L" %s", tick.Format());
				str += strTemp;
			}
		}
	}
	return str;
}

//============================================================================

BOOL CTranzxDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	return TRUE;
}


BOOL CTranzxDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;

	TRACE(L"[DOC] Begin OnOpenDocument()\n");
	if (!LoadFile((CString)lpszPathName)) {
		m_nType = FILETYPE_NON;
		return FALSE;
	}
	TRACE(L"[DOC] End OnOpenDocument()\n");
	return TRUE;
}

void CTranzxDoc::Serialize(CArchive& ar)
{
	if (ar.IsStoring()) {
	}
	else {
	}
}

#ifdef _DEBUG
void CTranzxDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CTranzxDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG
