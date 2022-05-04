// Cook.cpp
#include "stdafx.h"
#include "Dts.h"
#include "Cook.h"

#include "Intro.h"
#include "Doc.h"

#include "Tools.h"

//#define	__EDIFICE_CHECK__

extern "C"
void CALLBACK TimeFunc(UINT id, UINT msg, DWORD dwUser, DWORD dw1, DWORD dw2)
{
	CCook* pCook = (CCook*)dwUser;
	pCook->Revolve();
}

IMPLEMENT_DYNAMIC(CCook, CWnd)

//PCTSTR	CCook::m_pstrKeyword[] = {
//	L"WAIT",
//	L"GOTO",
//	L"PAUSE",
//	L"STCK",
//	L"MSTC",
//	L"TIN",
//	L"CIN",
//	L"OUT",
//	L"AIN",	
//	L"ATO",
//	L"PAU",
//	L"PIS",
//	L"TRS",
//	L"SIV",
//	L"V3F",
//	L"ECU",
//	L"HVAC",
//	L"DCU"
//};
//
const CCook::KEYWORD	CCook::m_keyword[] = {
	{ KW_END,	L"END" },
	{ KW_WAIT,	L"WAIT" },
	{ KW_GOTO,	L"GOTO" },
	{ KW_PAUSE,	L"PAUSE" },
	{ KW_WATCH,	L"WATCH" },
	{ KW_CHK,	L"CHK" },
	{ KW_STCK,	L"STICK" },
	{ KW_MSTC,	L"MASCON" },
	{ KW_TIN,	L"TIN" },
	{ KW_CIN,	L"CIN" },
	{ KW_OUT,	L"OUT" },
	{ KW_AIN,	L"AIN" },	
	{ KW_ATO,	L"ATO" },
	{ KW_PAU,	L"PAU" },
	{ KW_PIS,	L"PIS" },
	{ KW_TRS,	L"TRS" },
	{ KW_SIV,	L"SIV" },
	{ KW_V3F,	L"V3F" },
	{ KW_ECU,	L"ECU" },
	{ KW_HVAC,	L"HVAC" },
	{ KW_DCU,	L"DCU" }
};

CCook::CCook()
{
	m_pParent = NULL;
	m_bTimerReady = false;
	m_timerID = NULL;
	InitStep();
	for (int n = 0; n < LENGTH_TCU; n ++)
		ZeroMemory(&m_tcu[n], sizeof(TCUINFORM));
}

CCook::~CCook()
{
	if (m_timerID) {
		StopTimer();
		m_timerID = NULL;
	}
	if (m_bTimerReady) {
		timeEndPeriod(m_timerRes);
		m_bTimerReady = false;
	}
	for (int n = 0; n < PARSID_MAX; n ++)
		DeleteEdifice(&m_edif[n]);
}

bool CCook::StartTimer(UINT ms)
{
	m_timerID = timeSetEvent(ms, m_timerRes, TimeFunc, (DWORD)this, TIME_PERIODIC);
	if (!m_timerID) {
		Logr(L"timeSetEvent() failed!\r\n");
		return false;
	}
	return true;
}

bool CCook::StopTimer()
{
	if (m_timerID) {
		MMRESULT res = timeKillEvent(m_timerID);
		if (res != TIMERR_NOERROR) {
			Logr(L"timeKillEvent() failed!\r\n");
			return false;
		}
	}
	return true;
}

void CCook::DeleteEdifice(PEDIFICE pEdifice)
{
	for (POSITION pos = pEdifice->GetHeadPosition(); pos != NULL; ) {
		PMODULE pMod = (PMODULE)pEdifice->GetNext(pos);
		if (pMod != NULL) {
			if (pMod->edif.GetCount() != 0)	DeleteEdifice(&pMod->edif);
			delete pMod;
			pMod = NULL;
		}
	}
	pEdifice->RemoveAll();
}

void CCook::Log(CString strLog, COLORREF crText)
{
	CIntro* pIntro = (CIntro*)m_pParent;
	ASSERT(pIntro);
	pIntro->Log(strLog, crText);
}

void CCook::Logr(CString strLog, COLORREF crText)
{
	CIntro* pIntro = (CIntro*)m_pParent;
	ASSERT(pIntro);
	pIntro->Logr(strLog, crText);
}

//CString CCook::ExtractUnit(CString strUnits)
//{
//}
//
CString CCook::LoadFile(CString strFile)
{
	CString str;
	str.Format(L"load %s...\r\n", (LPCTSTR)strFile);
	Logr(str);

	CIntro* pIntro = (CIntro*)m_pParent;
	ASSERT(pIntro);
	CDoc* pDoc = pIntro->GetDoc();
	ASSERT(pDoc);

	CString strContext = pDoc->LoadFile(strFile);
	if (strContext.IsEmpty()) {
		Logr(L"ERROR>empty file!\r\n", COLOR_RED);
		return L"";
	}
	return strContext;
}

CString CCook::LoadSentence(CString strFile, CString strHead)
{
	CString strContext = LoadFile(strFile);
	if (strContext.IsEmpty())	return strContext;

	strContext.MakeUpper();
	CString strFind = strHead;
	strFind.MakeUpper();

	CString str;
	int n = strContext.Find((LPCTSTR)strFind);
	if (n < 0) {
		str.Format(L"not found %s!\r\n", (LPCTSTR)strHead);
		Logr(str, COLOR_RED);
		return L"";
	}
	strContext = strContext.Mid(n + strFind.GetLength());
	n = strContext.Find('{');
	if (n < 0 || n > 4) {
		Logr(L"not found open brackets!\r\n", COLOR_RED);
		return L"";
	}
	strContext = strContext.Mid(n + 1);
	n = strContext.Find('}');
	if (n < 0) {
		Logr(L"not found close brackets!\r\n", COLOR_RED);
		return L"";
	}
	strContext = strContext.Left(n);
	strContext = strContext.TrimLeft();
	strContext = strContext.TrimRight();
	RemoveComments(strContext);
	return strContext;
}

void CCook::RemoveComments(CString& strContext)
{
	for (int n = 0; n >= 0; ) {
		n = strContext.Find(L"//");
		if (n >= 0) {
			CString strLeft = strContext.Left(n);
			strContext = strContext.Mid(n);
			n = strContext.Find(L"\r\n");
			if (n >= 0)	strContext = strLeft + strContext.Mid(n);
			else	strContext = strLeft;
		}
	}
	for (int n = 0; n >= 0; ) {
		n = strContext.Find(L"/*");
		if (n >= 0) {
			CString strLeft = strContext.Left(n);
			strContext = strContext.Mid(n);
			n = strContext.Find(L"*/");
			if (n >= 0)	strContext = strLeft + strContext.Mid(n + 2);
			else {
				Logr(L"not found close comments!\r\n");
				strContext = strLeft;
			}
		}
	}
}

CString CCook::ExtractWord(CString& strContext)
{
	int n = strContext.FindOneOf(L" \t,;");
	if (n < 0)	return L"";
	CString strWord = strContext.Left(n);
	strWord.TrimLeft();
	strWord.TrimRight();
	strContext = strContext.Mid(n + 1);
	strContext.TrimLeft();
	return strWord;
}

CString CCook::ExtractSentence(CString& strContext)
{
	int n = strContext.Find(';');
	if (n < 0)	return L"";
	CString strSentence = strContext.Left(n + 1);
	strSentence.TrimLeft();
	strSentence.TrimRight();
	strContext = strContext.Mid(n + 2);
	strContext.TrimLeft();
	return strSentence;
}

bool CCook::ExtractEdificeIo(CString strFile, CString strHead, PEDIFICE pEdifice)
{
	CString strContext = LoadSentence(strFile, strHead);
	if (strContext.IsEmpty())	return false;

	CString str;
	UINT nLine = 1;
	UINT nID = 0;
	while (!strContext.IsEmpty()) {
		CString strEnum = ExtractWord(strContext);
		if (strEnum.IsEmpty())	break;
		int n = strEnum.Find('_');
		if (n < 0) {
			str.Format(L"+%d:%s - not foune \'_\' !\r\n", nLine, (LPCTSTR)strEnum);
			Logr(str, COLOR_RED);
			return false;
		}
		strEnum = strEnum.Mid(n + 1);
		if (!strEnum.IsEmpty()) {
			n = strEnum.Find(L"SPR");
			if (n < 0 && strEnum.Compare(L"MAX")) {
				PMODULE pMod = new MODULE;
				pMod->nID = nID;
				pMod->cSize = pMod->cType = 0;
				pMod->strName = strEnum;
				pEdifice->AddTail(pMod);
			}
		}
		++ nID;
		++ nLine;
		if (!strContext.IsEmpty() && strContext.GetAt(0) == '=') {
			strContext = strContext.Mid(4);
			strContext.TrimLeft();
		}
	}
	str.Format(L"parse completed %s(%d line)\r\n", (LPCTSTR)strFile, nLine);
	Logr(str);
	return true;
}

void CCook::AnnexEdificeIo(CString strFile, PEDIFICE pEdifice)
{
#if	defined(__EDIFICE_CHECK__)
	CString strContext = L"";
	for (POSITION pos = pEdifice->GetHeadPosition(); pos != NULL; ) {
		PMODULE pMod = (PMODULE)pEdifice->GetNext(pos);
		if (pMod != NULL) {
			CString strLine;
			strLine.Format(L"%03d\t%s\r\n", pMod->nID, (LPCTSTR)pMod->strName);
			strContext += strLine;
		}
	}

	CIntro* pIntro = (CIntro*)m_pParent;
	ASSERT(pIntro);
	CDoc* pDoc = pIntro->GetDoc();
	ASSERT(pDoc);

	CString strPath = L"\\";
	strPath += strFile;
	pDoc->SaveFile(strPath, strContext);
#endif
}

bool CCook::ExtractEdificeSa(CString strFile, CString strHead, PEDIFICE pEdifice)
{
	CString strContext = LoadSentence(strFile, strHead);
	if (strContext.IsEmpty())	return false;

	CString str;
	UINT nLine = 1;
	UINT nID = 0;
	while (!strContext.IsEmpty()) {
		CString strSentence = ExtractSentence(strContext);	// untill ';'
		if (strSentence.IsEmpty())	break;
		CString strKey = ExtractWord(strSentence);
		if (strKey.IsEmpty())	break;
		if (!strKey.Compare(L"BYTE") || !strKey.Compare(L"WORD")) {
			BYTE cType = !strKey.Compare(L"BYTE") ? MODTYPE_BYTE : MODTYPE_WORD;
			CString strName = ExtractWord(strSentence);
			strSentence.TrimRight();
			if (strName.IsEmpty() || !strSentence.IsEmpty()) {
				str.Format(L"+%d:%s has not name(%s) or empty or wrong charictors(%s)!\r\n",
					nLine, cType == MODTYPE_BYTE ? L"BYTE" : L"WORD", (LPCTSTR)strName, (LPCTSTR)strSentence);
				Logr(str, COLOR_RED);
				return false;
			}
			PMODULE pMod = new MODULE;

			pMod->nID = nID;
			pMod->cSize = cType == MODTYPE_BYTE ? 1 : 2;
			pMod->cType = cType;
			pMod->strName = strName;
			pEdifice->AddTail(pMod);
			if (cType == MODTYPE_BYTE)	++ nID;
			else	nID += 2;
			++ nLine;
		}
		else if (!strKey.Compare(L"__BC(") || !strKey.Compare(L"__BW(")) {
			BYTE cType = !strKey.Compare(L"__BC(") ? MODTYPE_BUNION : MODTYPE_WUNION;
			CString strName = ExtractWord(strSentence);
			if (strName.IsEmpty()) {
				str.Format(L"+%d:%s has not name!\r\n", nLine, cType == MODTYPE_BUNION ? L"__BC(" : L"__BW(");
				Logr(str, COLOR_RED);
				return false;
			}
			++ nLine;

			PMODULE pMod = new MODULE;
			pMod->nID = nID;
			pMod->cSize = cType == MODTYPE_BUNION ? 1 : 2;
			pMod->cType = cType;
			pMod->strName = strName;
			if (cType == MODTYPE_BUNION)	++ nID;
			else	nID += 2;
			UINT nSubID = 0;
			while (!strSentence.IsEmpty()) {
				strName = ExtractWord(strSentence);
				if (strSentence.IsEmpty()) {
					str.Format(L"+%d:%s has not size!\r\n", nLine, (LPCTSTR)strName);
					Logr(str, COLOR_RED);
					return false;
				}
				strKey = ExtractWord(strSentence);
				int size = atoi((CT2CA)strKey);
				if (size < 1) {
					str.Format(L"+%d:%s has illegal size!\r\n", nLine, (LPCTSTR)strName);
					Logr(str, COLOR_RED);
					return false;
				}
				PMODULE pSubm = new MODULE;
				pSubm->nID = nSubID;
				pSubm->cSize = (BYTE)size;
				pSubm->cType = 0;
				pSubm->strName = strName;
				pMod->edif.AddTail(pSubm);
				nSubID += (UINT)size;
				++ nLine;
			}
			if ((cType == MODTYPE_BUNION && nSubID != 8) || (cType == MODTYPE_WUNION && nSubID != 16)) {
				str.Format(L"+%d:%s has wrong size(%d)!\r\n", nLine, (LPCTSTR)pMod->strName, nSubID);
				Logr(str, COLOR_RED);
				return false;
			}
			pEdifice->AddTail(pMod);
		}
		else if (!strKey.Compare(L"DBFTIME")) {
			BYTE cType = MODTYPE_DWORD;
			CString strName = ExtractWord(strSentence);
			strSentence.TrimRight();
			if (strName.IsEmpty() || !strSentence.IsEmpty()) {
				str.Format(L"+%d:%s has not name(%s) or empty or wrong charictors(%s)!\r\n",
					nLine, L"DBFTIME", (LPCTSTR)strName, (LPCTSTR)strSentence);
				Logr(str, COLOR_RED);
				return false;
			}
			PMODULE pMod = new MODULE;

			pMod->nID = nID;
			pMod->cSize = 4;
			pMod->cType = cType;
			pMod->strName = strName;
			pEdifice->AddTail(pMod);
			nID += 4;
			++ nLine;
		}
		else {
			CString strErr;
			strErr.Format(L"%d:%s", nLine, strSentence);
			//Logr(strSentence);
			Logr(strErr);
			Logr(L"\r\nunknown keyword!\r\n", COLOR_RED);
			return false;
		}
	}
	str.Format(L"parse completed %s(%d line)\r\n", (LPCTSTR)strFile, nLine);
	Logr(str);
	return true;
}

void CCook::AnnexEdificeSa(CString strFile, PEDIFICE pEdifice)
{
#if	defined(__EDIFICE_CHECK__)
	CString strContext = L"";
	for (POSITION pos = pEdifice->GetHeadPosition(); pos != NULL; ) {
		PMODULE pMod = (PMODULE)pEdifice->GetNext(pos);
		if (pMod != NULL) {
			CString strLine;
			if (pMod->cType == MODTYPE_BYTE || pMod->cType == MODTYPE_WORD) {
				strLine.Format(L"%03d %d %s\t%s;\r\n", pMod->nID, pMod->cSize,
					pMod->cType == MODTYPE_BYTE ? L"BYTE" : L"WORD", (LPCTSTR)pMod->strName);
			}
			else if (pMod->cType == MODTYPE_BUNION || pMod->cType == MODTYPE_WUNION) {
				strLine.Format(L"%03d %d %s\t%s,\r\n", pMod->nID, pMod->cSize,
					pMod->cType == MODTYPE_BUNION ? L"__BC(" : L"__BW(", (LPCTSTR)pMod->strName);
				if (pMod->edif.GetCount() != 0) {
					for (POSITION spos = pMod->edif.GetHeadPosition(); spos != NULL; ) {
						PMODULE pSubm = (PMODULE)pMod->edif.GetNext(spos);
						if (pSubm != NULL) {
							CString strSub;
							strSub.Format(L"\t\t%03d %s\t%d,\r\n", pSubm->nID, (LPCTSTR)pSubm->strName, pSubm->cSize);
							strLine += strSub;
						}
					}
					strLine += L");\r\n";
				}
			}
			strContext += strLine;
		}
	}

	CIntro* pIntro = (CIntro*)m_pParent;
	ASSERT(pIntro);
	CDoc* pDoc = pIntro->GetDoc();
	ASSERT(pDoc);

	pDoc->SaveFile(strFile, strContext);
#endif
}

PMODULE CCook::FindEdifice(PEDIFICE pEdif, CString strLinear, CString strQuadratic)
{
	for (POSITION pos = pEdif->GetHeadPosition(); pos != NULL; ) {
		PMODULE pMod = (PMODULE)pEdif->GetNext(pos);
		if (!strLinear.Compare(pMod->strName)) {
			if (!strQuadratic.IsEmpty()) {
				if (pMod->edif.GetCount() != 0)	return FindEdifice(&pMod->edif, strQuadratic);
				return NULL;
			}
			return pMod;
		}
	}
	return NULL;
}

bool CCook::Initial()
{
	CString str;
	TIMECAPS tc;
	MMRESULT res = timeGetDevCaps(&tc, sizeof(TIMECAPS));
	if (res != TIMERR_NOERROR) {
		str.Format(L"timeGetDevCaps() error!(%d)\r\n", res);
		Logr(str, COLOR_RED);
		return false;
	}
	m_timerRes = min(max(tc.wPeriodMin, 1), tc.wPeriodMax);
	res = timeBeginPeriod(m_timerRes);
	if (res != TIMERR_NOERROR) {
		str.Format(L"timeBeginPeriod() error!(%d)\r\n", res);
		Logr(str, COLOR_RED);
		return false;
	}
	m_bTimerReady = true;

	if (ExtractEdificeSa(L"d:/TcmsWork/DuoLine/Qnx2/Inform2/DevInfo2/AtoInfo.h", L"_tagSAATO", &m_edif[PARSID_ATO]))
		AnnexEdificeSa(L"/Check/AtoSa", &m_edif[PARSID_ATO]);
	else {
		Logr(L"d:/TcmsWork/DuoLine/Qnx2/Inform2/DevInfo2/AtoInfo.h load fail!\r\n", COLOR_RED);
		return false;
	}

	if (ExtractEdificeSa(L"d:/TcmsWork/DuoLine/Qnx2/Inform2/DevInfo2/PauInfo.h", L"_tagSAPAU", &m_edif[PARSID_PAU]))
		AnnexEdificeSa(L"/Check/PauSa", &m_edif[PARSID_PAU]);
	else {
		Logr(L"d:/TcmsWork/DuoLine/Qnx2/Inform2/DevInfo2/PauInfo.h load fail!\r\n", COLOR_RED);
		return false;
	}

	if (ExtractEdificeSa(L"d:/TcmsWork/DuoLine/Qnx2/Inform2/DevInfo2/PisInfo.h", L"_tagSAPIS", &m_edif[PARSID_PIS]))
		AnnexEdificeSa(L"/Check/PisSa", &m_edif[PARSID_PIS]);
	else {
		Logr(L"d:/TcmsWork/DuoLine/Qnx2/Inform2/DevInfo2/PisInfo.h load fail!\r\n", COLOR_RED);
		return false;
	}

	if (ExtractEdificeSa(L"d:/TcmsWork/DuoLine/Qnx2/Inform2/DevInfo2/TrsInfo.h", L"_tagSATRS", &m_edif[PARSID_TRS]))
		AnnexEdificeSa(L"/Check/TrsSa", &m_edif[PARSID_TRS]);
	else {
		Logr(L"d:/TcmsWork/DuoLine/Qnx2/Inform2/DevInfo2/TrsInfo.h load fail!\r\n", COLOR_RED);
		return false;
	}

	if (ExtractEdificeSa(L"d:/TcmsWork/DuoLine/Qnx2/Inform2/DevInfo2/SivInfo.h", L"_tagSASIV", &m_edif[PARSID_SIV]))
		AnnexEdificeSa(L"/Check/SivSa", &m_edif[PARSID_SIV]);
	else {
		Logr(L"d:/TcmsWork/DuoLine/Qnx2/Inform2/DevInfo2/SivInfo.h load fail!\r\n", COLOR_RED);
		return false;
	}

	if (ExtractEdificeSa(L"d:/TcmsWork/DuoLine/Qnx2/Inform2/DevInfo2/V3fInfo.h", L"_tagSAV3F", &m_edif[PARSID_V3F]))
		AnnexEdificeSa(L"/Check/V3fSa", &m_edif[PARSID_V3F]);
	else {
		Logr(L"d:/TcmsWork/DuoLine/Qnx2/Inform2/DevInfo2/V3fInfo.h load fail!\r\n", COLOR_RED);
		return false;
	}

	if (ExtractEdificeSa(L"d:/TcmsWork/DuoLine/Qnx2/Inform2/DevInfo2/EcuInfo.h", L"_tagSAECU", &m_edif[PARSID_ECU]))
		AnnexEdificeSa(L"/Check/EcuSa", &m_edif[PARSID_ECU]);
	else {
		Logr(L"d:/TcmsWork/DuoLine/Qnx2/Inform2/DevInfo2/EcuInfo.h load fail!\r\n", COLOR_RED);
		return false;
	}

	if (ExtractEdificeSa(L"d:/TcmsWork/DuoLine/Qnx2/Inform2/DevInfo2/HvacInfo.h", L"_tagSAHVAC", &m_edif[PARSID_HVAC]))
		AnnexEdificeSa(L"/Check/HvacSa", &m_edif[PARSID_HVAC]);
	else {
		Logr(L"d:/TcmsWork/DuoLine/Qnx2/Inform2/DevInfo2/HvacInfo.h load fail!\r\n", COLOR_RED);
		return false;
	}

	if (ExtractEdificeSa(L"d:/TcmsWork/DuoLine/Qnx2/Inform2/DevInfo2/DcuInfo.h", L"_tagSADCU", &m_edif[PARSID_DCU]))
		AnnexEdificeSa(L"/Check/DcuSa", &m_edif[PARSID_DCU]);
	else {
		Logr(L"d:/TcmsWork/DuoLine/Qnx2/Inform2/DevInfo2/DcuInfo.h load fail!\r\n", COLOR_RED);
		return false;
	}

	if (ExtractEdificeIo(L"d:/TcmsWork/DuoLine/Qnx2/Inform2/Pio.h", L"enTUDINP", &m_edif[PARSID_TIN]))
		AnnexEdificeIo(L"/Check/Tin", &m_edif[PARSID_TIN]);
	else {
		Logr(L"d:/TcmsWork/DuoLine/Qnx2/Inform2/Pio.h-enTUDINP load fail!\r\n", COLOR_RED);
		return false;
	}

	if (ExtractEdificeIo(L"d:/TcmsWork/DuoLine/Qnx2/Inform2/Pio.h", L"enCUDINP", &m_edif[PARSID_CIN]))
		AnnexEdificeIo(L"/Check/Cin", &m_edif[PARSID_CIN]);
	else {
		Logr(L"d:/TcmsWork/DuoLine/Qnx2/Inform2/Pio.h-enCUDINP load fail!\r\n", COLOR_RED);
		return false;
	}

	if (ExtractEdificeIo(L"d:/TcmsWork/DuoLine/Qnx2/Inform2/Pio.h", L"enUDOUP", &m_edif[PARSID_OUT]))
		AnnexEdificeIo(L"/Check/Out", &m_edif[PARSID_OUT]);
	else {
		Logr(L"d:/TcmsWork/DuoLine/Qnx2/Inform2/Pio.h-enUDOUP load fail!\r\n", COLOR_RED);
		return false;
	}

	if (ExtractEdificeIo(L"d:/TcmsWork/DuoLine/Qnx2/Inform2/Pio.h", L"enUAINP", &m_edif[PARSID_AIN]))
		AnnexEdificeIo(L"/Check/Ain", &m_edif[PARSID_AIN]);
	else {
		Logr(L"d:/TcmsWork/DuoLine/Qnx2/Inform2/Pio.h-enUAINP load fail!\r\n", COLOR_RED);
		return false;
	}

	Logr(L"Cook-recipe load ok!\r\n");
	return true;
}

//***** PARSER *****

#define	CUR_LINE()		(m_scr.nTop + m_scr.nCur + 1)

void CCook::LoadScript(CString strFile)
{
	m_scr.strText = LoadFile(strFile);
	if (m_scr.strText.IsEmpty())	return;

	m_scr.strText.MakeUpper();
	m_scr.nTotal = m_scr.nTop = m_scr.nCur = m_scr.nWords = 0;
	while (PickupLine(m_scr.nTotal)) {
		++ m_scr.nTotal;
		if (!m_scr.strLine.Compare(L"BEGIN;"))	m_scr.nTop = m_scr.nTotal;
	}
	if (m_scr.nTotal < 1 || (m_scr.nTotal - m_scr.nTop) < 2) {
		Logr(L"empty or very small script!\r\n", COLOR_RED);
		m_scr.strText.Empty();
		m_scr.strLine.Empty();
		m_scr.nTotal = m_scr.nTop = 0;
	}
}

// nLine from 0
bool CCook::PickupLine(int nLine)
{
	if (m_scr.strText.IsEmpty())	return false;

	int s, l;
	s = l = 0;
	for (int n = 0; n < (nLine + 1); n ++) {
		if (l != 0)	s = l + 1;
		l = m_scr.strText.Find(L"\r\n", s);
		if (l < 0)	return false;
	}
	m_scr.strLine = m_scr.strText.Mid(s, l - s);
	m_scr.strLine.TrimLeft();
	m_scr.strLine.TrimRight();
	return true;
}

bool CCook::SeparateLine()
{
	for (int n = 0; n < 10; n ++)	m_scr.strWords[n] = L"";
	m_scr.nWords = 0;

	CString str;
	int leng = 0;
	CString strLine = m_scr.strLine;
	while (!strLine.IsEmpty()) {
		int n;
		if ((n = strLine.FindOneOf(L" ,\t\r\n")) < 0) {
			m_scr.strWords[leng] = strLine;
			m_scr.strWords[leng].TrimRight();
			if ((n = m_scr.strWords[leng].FindOneOf(L":;")) < 0) {
				str.Format(L"%d:%s not found end charactor!\r\n", CUR_LINE(), (LPCTSTR)m_scr.strLine);
				Logr(str, COLOR_RED);
				return false;
			}
			if ((n + 1) != m_scr.strWords[leng].GetLength()) {
				str.Format(L"%d:%s - %s wrong statement!\r\n", CUR_LINE(), (LPCTSTR)m_scr.strLine, (LPCTSTR)m_scr.strWords[leng]);
				Logr(str, COLOR_RED);
				return false;
			}
			if (m_scr.strWords[leng].GetAt(n) == ';')
				m_scr.strWords[leng] = m_scr.strWords[leng].Left(n);
			m_scr.nWords = leng + 1;
			return true;
		}
		m_scr.strWords[leng ++] = strLine.Left(n);
		strLine = strLine.Mid(n + 1);
		strLine.TrimLeft();
	}
	if (leng > 0) {
		str.Format(L"%d:%s not found semicolon!\r\n", CUR_LINE(), m_scr.strLine);
		Logr(str, COLOR_RED);
		return false;
	}
	return true;
}

int CCook::LinearKeyword()
{
	for (int n = 0; n < KW_MAX; n ++) {
		if (!m_scr.strWords[0].Compare((LPCTSTR)m_keyword[n].strKey))	return m_keyword[n].nID;
		//if (!m_scr.strWords[0].Compare((LPCTSTR)m_pstrKeyword[n]))	return n;
	}
	return -1;
}

int CCook::FindLabel(CString strLabel)
{
	strLabel += L":";
	for (int n = 0; n < (m_scr.nTotal - m_scr.nTop); n ++) {
		PickupLine(m_scr.nTop + n);
		if (!m_scr.strLine.IsEmpty()) {
			SeparateLine();
			if (m_scr.nWords == 1 && !m_scr.strWords[0].Compare(strLabel))	return n;
		}
	}
	return -1;
}

bool CCook::CheckArgLength(const int nValid)
{
	if (nValid != m_scr.nWords) {
		CString str;
		str.Format(L"%d:%s, invalid arg. length!(%d, %d)\r\n", CUR_LINE(), (LPCTSTR)m_scr.strWords[0], nValid, m_scr.nWords);
		return false;
	}
	return true;
}

int CCook::ExcerptInt(CString str)
{
	int n;
	if ((n = str.Find(L"0x")) >= 0) {
		GETTOOLS(pTools);
		return pTools->ahtoi(str.Mid(n + 2));
	}
	else if ((n = str.Find(L"0X")) >= 0) {
		GETTOOLS(pTools);
		return pTools->ahtoi(str.Mid(n + 2));
	}
	return atoi((CT2CA)str);
}

int CCook::ExcerptCid(CString str)
{
	//for (int n = 0; n < 10; n ++) {
	//	CString strNo;
	//	strNo.Format(L"%d", n);
	//	if (!strNo.Compare(str))	return n;
	//}
	int n = ExcerptInt(str);
	if (n >= 0 && n < 10)	return n;
	return -1;
}

void CCook::InitStep()
{
	m_scr.nCur = 0;
	m_wTick = m_wTickLimit = 0;
	m_bHold = m_bBreak = false;
	//for (int n = 0; n < LENGTH_TU; n ++)
	//	ZeroMemory(&m_tu[n], sizeof(TUINFORM));
}

UINT CCook::Takeout(bool bReal)
{
	while ((m_scr.nCur + m_scr.nTop) < m_scr.nTotal) {
		PickupLine(m_scr.nTop + m_scr.nCur);
		SeparateLine();
		CString str;
		int ki = LinearKeyword();
		switch (ki) {
		case KW_END :	return TAKEOUT_END;	break;
		case KW_WAIT :
			{
				// parse
				if (!CheckArgLength(2))	return false;
				double db = atof((CT2CA)m_scr.strWords[1]);
				if (db < 0.1f || db > 600) {	// limit 10min
					str.Format(L"%d:invalid time!(%.1f)\r\n", CUR_LINE(), db);
					Logr(str, COLOR_RED);
					return TAKEOUT_ERROR;
				}
				// exec
				if (!m_bCheck) {
					str.Format(L"%d:wait %.1fsec.\r\n", CUR_LINE(), db);
					Logr(str);
				}
				else {
					if (m_wTickLimit == 0) {
						m_wTickLimit = (WORD)(db * 10.f);
						m_wTick = 0;
						m_bHold = true;
						str.Format(L"%d:wait %.1fsec.\r\n", CUR_LINE(), db);
						Logr(str);
					}
					else if (++ m_wTick >= m_wTickLimit) {
						m_wTick = m_wTickLimit = 0;
						m_bHold = false;
					}
				}
			}
			break;
		case KW_GOTO :
			{
				// parse
				if (!CheckArgLength(2))	return false;
				int n = FindLabel(m_scr.strWords[1]);
				if (n < 0 && n > (m_scr.nTotal - m_scr.nTop)) {
					str.Format(L"%d:not found label!(%s)\r\n", CUR_LINE(), (LPCTSTR)m_scr.strWords[1]);
					Logr(str, COLOR_RED);
					return TAKEOUT_ERROR;
				}
				// exec
				PickupLine(m_scr.nTop + n);
				str.Format(L"%d:goto %d:%s\r\n", CUR_LINE(), n, (LPCTSTR)m_scr.strLine);
				Logr(str);
				if (m_bCheck)	m_scr.nCur = n;
			}
			break;
		case KW_PAUSE :
			{
				int n = m_scr.nCur + 1;
				PickupLine(m_scr.nTop + n);
				str.Format(L"%d:pause. - %s\r\n", CUR_LINE(), (LPCTSTR)m_scr.strLine);
				Logr(str);
			}
			if (m_bCheck) {
				StopTimer();
				if (MessageBox(L"pause... continue?", L"COOK", MB_YESNO) == IDNO)
					return TAKEOUT_ERROR;
				StartTimer(100);
			}
			break;
		case KW_WATCH :
			{
				// parse
				if (!CheckArgLength(2))	return false;
				int nID = ExcerptInt(m_scr.strWords[1]);
				if (nID < 0 || nID > 11) {
					str.Format(L"%d:wrong watcher id!(%d)\r\n", CUR_LINE(), nID);
					Logr(str, COLOR_RED);
					return TAKEOUT_ERROR;
				}
				if (!m_tcu[nID].bLink) {
					if (!m_tcu[nID].bReady) {
						str.Format(L"193.168.0.%03d", nID + 100);
						CIntro* pIntro = (CIntro*)m_pParent;
						ASSERT(pIntro);
						StopTimer();
						pIntro->StirWatcher(str);
						StartTimer(100);
						m_wTick = 0;
						m_tcu[nID].bReady = true;
						m_bHold = true;
					}
					else {
						if (++ m_wTick >= 30) {
							m_wTick = 0;
							m_strFailer.Format(L"can not stir watcher 193.168.0.%03d", nID + 100);
							return TAKEOUT_FAIL;
						}
					}
				}
				else {
					m_bHold = false;
					m_wTick = 0;
				}
			}
			break;
		case KW_CHK :
			{
				// parse
				if (!CheckArgLength(4))	return false;
				int nTid = ExcerptCid(m_scr.strWords[1]);
				if (nTid < 0 || nTid > 3) {
					str.Format(L"%d:wrong TID!(%d)\r\n", CUR_LINE(), nTid);
					Logr(str, COLOR_RED);
					return TAKEOUT_ERROR;
				}
				int nItem = ExcerptInt(m_scr.strWords[2]);
				int nTarget = ExcerptInt(m_scr.strWords[3]);
				if (bReal) {
					switch (nItem) {
					case 0 :	// oblige
						if (!m_tcu[nTid].bLink || m_tcu[nTid].wOblige != (WORD)nTarget) {
							m_strFailer.Format(L"FAIL:TU%d oblige %04X (%04X)!!!\n", nTid, m_tcu[nTid].wOblige, nTarget);
							return TAKEOUT_FAIL;
						}
						break;
					case 1 :	// speed
						{
							int min = nTarget > 5 ? nTarget - 5 : 0;
							int max = nTarget + 5;
							if (!m_tcu[nTid].bLink || m_tcu[nTid].wSpeed < (WORD)min || m_tcu[nTid].wSpeed > (WORD)max) {
								m_strFailer.Format(L"FAIL:TU%d speed %d (%d)!!!\n", nTid, m_tcu[nTid].wSpeed, nTarget);
								return TAKEOUT_FAIL;
							}
						}
						break;
					case 2 :	// distance
						break;
					default :	break;
					}
				}
			}
			break;
		case KW_STCK :
			{
				// parse
				if (!CheckArgLength(3))	return false;
				int nCid = ExcerptCid(m_scr.strWords[1]);
				if (nCid != 0 && nCid != 9) {
					str.Format(L"%d:wrong CID!(%d)\r\n", CUR_LINE(), nCid);
					Logr(str, COLOR_RED);
					return TAKEOUT_ERROR;
				}
				CIntro* pIntro = (CIntro*)m_pParent;
				ASSERT(pIntro);
				int rudder = pIntro->CheckRudderKeyword(nCid, m_scr.strWords[2]);
				if (rudder < 0 || rudder > 2) {
					str.Format(L"%d:unknown command!(%s)\r\n", CUR_LINE(), (LPCTSTR)m_scr.strWords[2]);
					Logr(str, COLOR_RED);
					return TAKEOUT_ERROR;
				}
				// exec
				CString strTemp = m_scr.strWords[2];
				strTemp.MakeLower();
				str.Format(L"%d:stick %d %s\r\n", CUR_LINE(), nCid, (LPCTSTR)strTemp);
				Logr(str);
				if (m_bCheck)	pIntro->RudderCtrl(nCid, rudder);
			}
			break;
		case KW_MSTC :
			{
				// parse
				if (!CheckArgLength(3))	return false;
				int nCid = ExcerptCid(m_scr.strWords[1]);
				if (nCid != 0 && nCid != 9) {
					str.Format(L"%d:wrong CID!(%d)\r\n", CUR_LINE(), nCid);
					Logr(str, COLOR_RED);
					return TAKEOUT_ERROR;
				}
				CIntro* pIntro = (CIntro*)m_pParent;
				ASSERT(pIntro);
				int pos = pIntro->CheckMasterKeyword(nCid, m_scr.strWords[2]);
				if (pos < 0 || pos > 12) {
					str.Format(L"%d:unknown position!(%s)\r\n", CUR_LINE(), (LPCTSTR)m_scr.strWords[2]);
					Logr(str, COLOR_RED);
					return TAKEOUT_ERROR;
				}
				// exec
				CString strTemp = m_scr.strWords[2];
				strTemp.MakeLower();
				str.Format(L"%d:master controller %d %d\r\n", CUR_LINE(), nCid, (LPCTSTR)strTemp);
				Logr(str);
				if (m_bCheck)	pIntro->MasterCtrl(nCid, pos);
			}
			break;
		case KW_TIN :
			{
				// parse
				if (!CheckArgLength(4))	return false;
				int nCid = ExcerptCid(m_scr.strWords[1]);
				if (nCid != 0 && nCid != 9) {
					str.Format(L"%d:wrong CID!(%d)\r\n", CUR_LINE(), nCid);
					Logr(str, COLOR_RED);
					return TAKEOUT_ERROR;
				}
				PMODULE pMod = FindEdifice(&m_edif[PARSID_TIN], m_scr.strWords[2]);
				if (pMod == NULL) {
					str.Format(L"%d:unknown input!(%s)\r\n", CUR_LINE(), (LPCTSTR)m_scr.strWords[2]);
					Logr(str, COLOR_RED);
					return TAKEOUT_ERROR;
				}
				if (m_scr.strWords[3].Compare(L"0") && m_scr.strWords[3].Compare(L"1")) {
					str.Format(L"%d:invalid value!(%s)\r\n", CUR_LINE(), (LPCTSTR)m_scr.strWords[3]);
					Logr(str, COLOR_RED);
					return TAKEOUT_ERROR;
				}
				int state = atoi((CT2CA)m_scr.strWords[3]);
				// exec
				CString strTemp = m_scr.strWords[2];
				strTemp.MakeLower();
				str.Format(L"%d:tin %d %s to %d\r\n", CUR_LINE(), nCid, (LPCTSTR)strTemp, state);
				Logr(str);
				if (m_bCheck) {
					CIntro* pIntro = (CIntro*)m_pParent;
					ASSERT(pIntro);
					pIntro->TBitCtrlD(nCid, pMod->nID, state != 0 ? true : false);
				}
			}
			break;
		case KW_CIN :
			{
				// parse
				if (!CheckArgLength(4))	return false;
				int nCid = ExcerptCid(m_scr.strWords[1]);
				if (nCid < 0 || nCid > 9) {
					str.Format(L"%d:wrong CID!(%d)\r\n", CUR_LINE(), nCid);
					Logr(str, COLOR_RED);
					return TAKEOUT_ERROR;
				}
				PMODULE pMod = FindEdifice(&m_edif[PARSID_CIN], m_scr.strWords[2]);
				if (pMod == NULL) {
					str.Format(L"%d:unknown input!(%s)\r\n", CUR_LINE(), (LPCTSTR)m_scr.strWords[2]);
					Logr(str, COLOR_RED);
					return TAKEOUT_ERROR;
				}
				if (m_scr.strWords[3].Compare(L"0") && m_scr.strWords[3].Compare(L"1")) {
					str.Format(L"%d:invalid value!(%s)\r\n", CUR_LINE(), (LPCTSTR)m_scr.strWords[3]);
					Logr(str, COLOR_RED);
					return TAKEOUT_ERROR;
				}
				int state = atoi((CT2CA)m_scr.strWords[3]);
				// exec
				CString strTemp = m_scr.strWords[2];
				strTemp.MakeLower();
				str.Format(L"%d:cin %d %s to %d\r\n", CUR_LINE(), nCid, (LPCTSTR)strTemp, state);
				Logr(str);
				if (m_bCheck) {
					CIntro* pIntro = (CIntro*)m_pParent;
					ASSERT(pIntro);
					pIntro->CBitCtrlD(nCid, pMod->nID, state != 0 ? true : false);
				}
			}
			break;
		case KW_OUT :
		case KW_AIN :
		case KW_ATO :
		case KW_PAU :
		case KW_PIS :
		case KW_TRS :
		case KW_SIV :
		case KW_V3F :
		case KW_ECU :
		case KW_HVAC :
		case KW_DCU :
		default :
			break;
		}
		if (!m_bHold)	++ m_scr.nCur;
		return TAKEOUT_CONTINUE;
	}
	return TAKEOUT_END;
}

bool CCook::Revolve()
{
	int res;
	if (m_bBreak || (res = Takeout(true)) != TAKEOUT_CONTINUE) {
		StopTimer();
		CString str;
		if (m_bBreak)	str = L"User break!\r\n";
		else if (res == TAKEOUT_END)	str = L"end...\r\n";
		else if (res == TAKEOUT_FAIL) {
			CIntro* pIntro = (CIntro*)m_pParent;
			ASSERT(pIntro);
			//pIntro->TBitCtrlD(0, TUDIB_HCR, false);
			//pIntro->TBitCtrlD(0, TUDIB_TCR, false);
			//pIntro->TBitCtrlD(9, TUDIB_HCR, false);
			//pIntro->TBitCtrlD(9, TUDIB_TCR, false);
			MessageBox(m_strFailer, L"COOK FAIL", MB_ICONWARNING);
		}
		else	str = L"error!\r\n";
		Logr(str);
		return false;
	}
	return true;
}

bool CCook::Propel(CString strFile)
{
	CString str = SCRIPTPATH;
	str += (L"/" + strFile);
	LoadScript(str);
	if (m_scr.strText.IsEmpty()) {
		MessageBox(L"Empty script!!!");
		return false;
	}
	InitStep();
	m_bCheck = false;
	int n;
	while ((n = Takeout(false)) == TAKEOUT_CONTINUE);
	if (n != TAKEOUT_END)	return false;

	Logr(L"script check ok! run...\r\n");
	InitStep();
	m_bCheck = true;
	StartTimer(100);
	return true;
}

void CCook::TcmChange(int nID, int nItem, variant_t var)
{
	if (nID < 100 || nID > 103)	return;
	nID -= 100;
	switch (nItem) {
	case INFTCM_LINK :
		m_tcu[nID].bLink = var.boolVal ? true : false;
		m_tcu[nID].bReady = false;
		break;
	case INFTCM_OBLIGE :
		m_tcu[nID].wOblige = (WORD)var.uiVal;
		break;
	case INFTCM_SPEED :
		m_tcu[nID].wSpeed = (WORD)var.ulVal;
		break;
	case INFTCM_DISTAN :
		m_tcu[nID].dwDistan = (DWORD)var.ulVal;
		break;
	default :	break;
	}
}

BEGIN_MESSAGE_MAP(CCook, CWnd)
END_MESSAGE_MAP()
