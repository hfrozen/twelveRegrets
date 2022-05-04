// TranzxView.cpp
#include "stdafx.h"
#include "Tranzx.h"

#include "TranzxDoc.h"
#include "TranzxView.h"
#include "MainFrm.h"
#include "ChildFrm.h"

#include "FindTroubles.h"
#include "TroubleItem.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define	SETITEM(p, l, r, n, s)\
	do {\
		l.mask = LVIF_TEXT;\
		l.iItem = r;\
		l.iSubItem = n;\
		l.pszText = s.GetBuffer(s.GetLength());\
		p->SetItem(&l);\
		s.ReleaseBuffer();\
	} while (0)

IMPLEMENT_DYNCREATE(CTranzxView, CView)

BEGIN_MESSAGE_MAP(CTranzxView, CView)
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CTranzxView::OnFilePrintPreview)
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	//ON_MESSAGE(WM_OPENFILE, OnOpenFile)
	ON_MESSAGE(WM_SELITEM, &CTranzxView::OnSelItem)
	ON_MESSAGE(WM_DROPITEM, &CTranzxView::OnDropItem)
	ON_MESSAGE(WM_PARTSELECT, &CTranzxView::OnPartSelect)
	ON_MESSAGE(WM_SELECTTROUBLE, &CTranzxView::OnSelectTrouble)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_TROUBLE, &CTranzxView::OnDblclkList)
END_MESSAGE_MAP()

CTranzxView::CTranzxView()
{
	m_nType = L'0';
	m_pHorz = m_pVert = NULL;
	m_sizeTotal.cx = m_sizeTotal.cy = 100;
	m_pFactPanel = NULL;
	m_pTickPanel = NULL;
	m_pHole = NULL;
	m_pHorz = NULL;
	m_pVert = NULL;
	m_nGrid = -1;
	//m_font.CreateFont(14, 0, 0, 0, 300, FALSE, FALSE, 0,
	//		ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
	//		DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	m_fontPage.CreateFont(-80, 0, 0, 0, 300, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
	m_crDefault = RGB(16, 16, 16);
	//m_crBkgnd = RGB(255, 255, 255);
	//m_crText = RGB(16, 16, 16);
	//m_brBkgnd.CreateSolidBrush(m_crBkgnd);
	//m_brText.CreateSolidBrush(m_crText);
	m_ptCur.x = m_ptCur.y = 0;
	//m_strTick = L"";
	//m_strFact = L"";

	m_pListTrouble = NULL;
	m_nTroubleType = 0;
	//m_pFindTroubles = NULL;
	//m_strFindPath = L"";
	TRACE(L"CTranzxView()\n");
}

CTranzxView::~CTranzxView()
{
	MakeHistory();
	if (m_pHorz != NULL) {
		delete m_pHorz;
		m_pHorz = NULL;
	}
	if (m_pVert != NULL) {
		delete m_pVert;
		m_pVert = NULL;
	}
	if (m_pFactPanel != NULL) {
		delete m_pFactPanel;
		m_pFactPanel = NULL;
	}
	if (m_pTickPanel != NULL) {
		delete m_pTickPanel;
		m_pTickPanel = NULL;
	}
	if (m_pHole != NULL) {
		delete m_pHole;
		m_pHole = NULL;
	}
	if (m_pListTrouble != NULL) {
		//if (m_pListTrouble->GetItemCount() > 0)	m_pListTrouble->DeleteAllItems();
		delete m_pListTrouble;
		m_pListTrouble = NULL;
	}
	m_fontTroubleList.DeleteObject();
	m_fontPage.DeleteObject();
	//m_font.DeleteObject();
	//m_brBkgnd.DeleteObject();
	//m_brText.DeleteObject();
	TRACE(L"~CTranzxView()\n");
}

const CTranzxView::LISTFORM CTranzxView::m_listForm[] = 
{
	{	L"순서",		L"WWWW"	},
	{	L"발생 시간",	L"WWWWWWWWWWWW"	},
	{	L"소거 시간",	L"WWWWWWWW"	},
	{	L"고장 번호",	L"WWWWWWWW"	},
	{	L"객차 번호",	L"WWWWWW"	},
	{	L"고장 종류",	L"WWWWWWWW"	},
	{	L"고장 내용",	L"WWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWWW"	},
	{	L"",		L""	},
	{	L"",		L""	},
	{	L"",		L""	}
};

//============================================================================

const BSTR CTranzxView::m_pStick[] = {
	L"EB",	L"B7",	L"B6",	L"B5",
	L"B4",	L"B3",	L"B2",	L"B1",
	L"N",	L"P0.5",L"P1",	L"P2",
	L"P3",	L"P4",	L"NON"
};

const BSTR CTranzxView::m_pTroubleType[] = {
	L"NON",		L"문자",		L"알림",		L"상태",
	L"주의",		L"경 고장",	L"중 고장",	L"비상"
};

const BSTR CTranzxView::m_pInspMode[] = {
	L"NON",		L"PDT",		L"DAILY",	L"MONTHLY"
	L"EACH",	L"VIEW",	L"MAX"
};

const BSTR CTranzxView::m_pInspOper[] = {
	L"NON",		L"OK",		L"NG",		L"TEST",
	L"통신 고장",	L"통신 고장",	L"통신 고장",	L"통신 고장"
};

void CTranzxView::Log(CString str, bool bPos)
{
	if (bPos) {
		CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
		ASSERT(pFrame);
		pFrame->Log(str, true);
	}
	else {
		CTranzxDoc* pDoc = (CTranzxDoc*)GetDocument();
		ASSERT(pDoc);
		pDoc->Log(str);
	}
}

/*void CTranzxView::FrameLog(CString str)
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	pFrame->SetFrameLog(str);
}

CString CTranzxView::GetFrameLog()
{
	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	return pFrame->GetFrameLog();
}
*/

// INSPECT ===================================================================

/*bool CTranzxView::PickupWord(BYTE* p, WORD offset, WORD& value)
{
	if (offset >= 768)	return false;
	BYTE n = *(p + offset);
	BYTE m = *(p + offset + 1);
	value = (((WORD)n << 8) & 0xff00) | ((WORD)m & 0xff);
	return true;
}

bool CTranzxView::ParseInspType(CString str, INSPF& insp)
{
	ZeroMemory(&insp, sizeof(INSPF));
	insp.wOffset = atoi(CT2CA(str));
	if (insp.wOffset == 0)	return false;
	int n = str.FindOneOf(L"-*&");
	if (n >= 0) {
		TCHAR type = str.GetAt(n);
		str = str.Mid(n + 1);
		switch (type) {
		case (TCHAR)'-' :
			insp.nType = INSPTYPE_BIT;
			insp.nIndex[0] = (BYTE)atoi(CT2CA(str));
			++ insp.nIndexLength;
			n = str.Find((TCHAR)',');
			if (n >= 0) {
				str = str.Mid(n + 1);
				insp.nIndex[1] = (BYTE)atoi(CT2CA(str));
				++ insp.nIndexLength;
				n = str.Find((TCHAR)',');
				if (n >= 0) {
					str = str.Mid(n + 1);
					insp.nIndex[2] = (BYTE)atoi(CT2CA(str));
					++ insp.nIndexLength;
				}
			}
			break;
		case (TCHAR)'*' :
			insp.nType = INSPTYPE_VALUE;
			insp.factor = atof(CT2CA(str));
			if (insp.factor == 0.0)	return FALSE;
			break;
		case (TCHAR)'&' :
			insp.nType = INSPTYPE_FORM;
			insp.nIndex[0] = (BYTE)atoi(CT2CA(str));
			break;
		default :
			return false;
			break;
		}
	}
	else	return false;

	return true;
}

void CTranzxView::InspectItem(CString str, int col, int row, BYTE* p)
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
					str.Format(L"%s", (LPCTSTR)m_pInspOper[bf]);
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
					str.Format(L"%s", (LPCTSTR)m_pInspMode[w]);
					m_pXel->SetCellValue(col, row, str);
				}
				break;
			case 3 :
				if (PickupWord(p, insp.wOffset, w)) {
					if (w >= (SIZE_STICK - 1))	w = SIZE_STICK - 1;
					str.Format(L"%s", (LPCTSTR)m_pStick[w]);
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
*/
// LOGBOOK ===================================================================

void CTranzxView::MakeHistory()
{
	if (m_pHole == NULL)	return;

	CString strInfo = m_pHole->GetInfo();
	BSTR bstrInfo = strInfo.AllocSysString();
	char* pBuf = _com_util::ConvertBSTRToString(bstrInfo);
	size_t leng = strlen(pBuf);

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	ASSERT(pFrame);
	CString strHist = pFrame->GetRoot();
	strHist += L"\\history.txt";

	CFile file;
	CFileException error;
	if (file.Open(strHist, CFile::modeCreate | CFile::modeWrite, &error)) {
		file.Write(pBuf, leng);
		file.Close();
	}
}

void CTranzxView::BuildByHistory()
{
	if (m_pHole == NULL)	return;

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	ASSERT(pFrame);
	CString strHist = pFrame->GetRoot();
	strHist += L"\\history.txt";

	CFile file;
	CFileException error;
	CString str;
	if (!file.Open(strHist, CFile::modeRead, &error)) {
		str = L"[VIEW] 히스토리 파일을 찾을 수 없습니다!";
		Log(str);
		return;
	}
	DWORD dw = (DWORD)file.GetLength();
	char* pBuf = new char[dw];
	ZeroMemory(pBuf, dw);
	file.Read(pBuf, dw);
	file.Close();
	CString strInfo = (CString)pBuf;
	//strInfo.Format(L"%S", pBuf);
	delete [] pBuf;

	int id;
	COLORREF color;
	CString strName;
	bool bNew, bFirst;
	bFirst = true;
	do {
		bNew = ParseHistory(id, color, strName, strInfo);
		if (id > -1) {
			if (bNew || bFirst) {
				bFirst = false;
				bNew = true;
			}
			m_pHole->EntranceA(id, strName, color, bNew);
		}
	} while (id > -1);
}

bool CTranzxView::ParseHistory(int& id, COLORREF& color, CString& strName, CString& str)
{
	bool bNew = false;
	id = -1;
	color = NULL;
	strName = L"";

	str.TrimLeft();
	int n;
	if ((n = str.Find(';')) < 2) {
		bNew = true;
		str = str.Mid(n + 1);
		str.TrimLeft();
	}
	if (!str.IsEmpty()) {
		if ((n = str.Find(']')) > 0) {
			CString strTemp = str.Left(n);
			str = str.Mid(n + 1);
			str.TrimLeft();
			id = atoi(CT2CA(strTemp));
			if ((n = strTemp.Find('<')) > 0) {
				strTemp = strTemp.Mid(n + 1);
				color = atol(CT2CA(strTemp));
			}
			if ((n = strTemp.Find('[')) > 0)
				strName = strTemp.Mid(n + 1);
		}
	}
	return bNew;
}

void CTranzxView::GetDrawRect(CRect& rect)
{
	//AfxGetMainWnd()->GetClientRect(rect);
	GetClientRect(rect);
	if (m_pVert != NULL)	rect.right -= SIZE_SCROLLBAR;
	if (m_pHorz != NULL)	rect.bottom -= SIZE_SCROLLBAR;
}

void CTranzxView::Redraw()
{
	CTranzxDoc* pDoc = (CTranzxDoc*)GetDocument();
	ASSERT(pDoc);

	m_sizeTotal.cx = (long)pDoc->GetMaxX();
	m_sizeTotal.cy = m_pHole != NULL ? m_pHole->GetHeight() : 100;
	SetScrollBar(m_sizeTotal);
	CString str;
	str.Format(L"[VIEW] 스크롤 크기 x = %d, y = %d.", m_sizeTotal.cx, m_sizeTotal.cy);
	Log(str);

	m_ptCur.x = m_ptCur.y = 0;
	if (m_pFactPanel != NULL) {
		if (!SetGridTime())	m_nGrid = -1;
	}
	else	m_nGrid = -1;

	if (m_pTickPanel != NULL) {
		m_pTickPanel->Draw(&m_ptCur);
		m_pTickPanel->SetGrid(m_nGrid);
	}
	if (m_pHole != NULL) {
		m_pHole->Redraw(m_ptCur);
		m_pHole->SetGrid(m_nGrid);
	}
}

void CTranzxView::SetScrollBar(CSize size)
{
	if (size.cx < 100 || size.cy < 100)	return;
	CRect rect;
	GetClientRect(rect);

	CRect rt(0, 0, 100, SIZE_SCROLLBAR);
	if (m_pHorz == NULL) {
		m_pHorz = new CScrollBar();
		m_pHorz->Create(WS_CHILD | WS_VISIBLE | SBS_HORZ, rt, this, 0);
	}
	m_pHorz->SetWindowPos(NULL, rect.left + ITEM_NAME_X, rect.bottom - SIZE_SCROLLBAR,
							rect.Width() - ITEM_NAME_X - SIZE_SCROLLBAR, SIZE_SCROLLBAR,
							SWP_NOZORDER | SWP_NOACTIVATE);

	rt = CRect(0, 0, SIZE_SCROLLBAR, 100);
	if (m_pVert == NULL) {
		m_pVert = new CScrollBar();
		m_pVert->Create(WS_CHILD | WS_VISIBLE | SBS_VERT, rt, this, 0);
	}
	m_pVert->SetWindowPos(NULL, rect.right - SIZE_SCROLLBAR, rect.top,
							SIZE_SCROLLBAR, rect.Height() - SIZE_SCROLLBAR - ITEM_TIME_Y,
							SWP_NOZORDER | SWP_NOACTIVATE);

	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_ALL;
	m_pHorz->GetScrollInfo(&si);
	si.nMin = 0;
	si.nMax = size.cx;
	si.nPage = rect.Width() - ITEM_NAME_X - SIZE_SCROLLBAR;
	si.nPos = 0;
	m_pHorz->SetScrollInfo(&si);
	m_pHorz->SetScrollPos(si.nPos, TRUE);

	//CTranzxDoc* pDoc = (CTranzxDoc*)GetDocument();
	//CString str;
	//str.Format(L"스크롤 nPos=%d nMax=%d nPage=%d MaxX=%d", si.nPos, si.nMax, si.nPage, pDoc->GetMaxX());
	//Log(str);
	//
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_ALL;
	m_pVert->GetScrollInfo(&si);
	si.nMin = 0;
	si.nMax = size.cy;
	si.nPage = rect.Height() - ITEM_TIME_Y - SIZE_SCROLLBAR;
	si.nPos = 0;
	m_pVert->SetScrollInfo(&si);
	m_pVert->SetScrollPos(si.nPos, TRUE);

	m_ptCur.x = m_ptCur.y = 0;
}

bool CTranzxView::SetGridTime()
{
	CTranzxDoc* pDoc = (CTranzxDoc*)GetDocument();
	ASSERT(pDoc);

	bool bRes = false;
	//m_strTick = L"";
	if (m_nGrid > 0) {
		CTick tick;
		int space = pDoc->GetLogbookTime(m_ptCur.x + m_nGrid, tick);
		if (space == 0 || space == 1) {
		//if (pDoc->GetLogbookTime(m_ptCur.x + m_nGrid, tick) < 2) {
			CString strTick;
			strTick.Format(L"%02d:%02d:%02d", tick.m_nHour, tick.m_nMin, tick.m_nSec);
			if (m_pFactPanel != NULL)	m_pFactPanel->Draw(strTick);
			bRes = true;
		}
	}
	//m_strFact = pDoc->GetFactStrByScale();
	//if (m_pFactPanel != NULL)	m_pFactPanel->Draw(m_strFact, m_strTick);
	return bRes;
}

void CTranzxView::SetGrid(int grid)
{
	m_nGrid = grid;
	if (!SetGridTime())	m_nGrid = -1;
	if (m_pHole != NULL)	m_pHole->SetGrid(m_nGrid);
	if (m_pTickPanel != NULL)	m_pTickPanel->SetGrid(m_nGrid);
}

void CTranzxView::FindLogbookTime(CTick time)
{
	if (m_nType != FILETYPE_LOGBOOK)	return;

	CTranzxDoc* pDoc = (CTranzxDoc*)GetDocument();
	int pos = (int)pDoc->GetLogbookIDByTime(time);
	if (pos < 0) {
		CString str;
		str.Format(L"%02d/%02d %02d:%02d:%02d 시간을 찾을 수 없습니다!", time.m_nMon, time.m_nDay, time.m_nHour, time.m_nMin, time.m_nSec);
		MessageBox(str, L"찾기", MB_ICONWARNING);
		return;
	}

	if (m_pHorz != NULL) {
		SCROLLINFO si;
		si.cbSize = sizeof(SCROLLINFO);
		si.fMask = SIF_ALL;
		m_pHorz->GetScrollInfo(&si);
		int x;
		if ((pos - (int)(si.nPage / 2)) <= 0)	x = 0;
		else if ((pos + (int)(si.nPage / 2)) > si.nMax)	x = si.nMax - si.nPage;
		else	x = pos - (int)(si.nPage / 2);
		m_ptCur.x = x;
		m_pHorz->SetScrollPos(x);

		if (m_pTickPanel != NULL)	m_pTickPanel->SetPos(m_ptCur);
		if (m_pHole != NULL)	m_pHole->SetPos(m_ptCur);
	}
	SetGrid(pos - m_ptCur.x);
}

//CSize CTranzxView::GetFullFrameSize()
//{
//	CSize fsize(0, 0);
//	for (POSITION pos = m_itemList.GetHeadPosition(); pos != NULL; ) {
//		CItem* pItem = (CItem*)m_itemList.GetNext(pos);
//		if (pItem != NULL) {
//			CSize size = pItem->GetFrameSize();
//			fsize.cy += size.cy;
//			if (fsize.cx < size.cx)	fsize.cx = size.cx;
//		}
//	}
//	fsize.cy += ITEM_TIME_Y;
//	return fsize;
//}
//
//int CTranzxView::GetEnableScale()
//{
//	CTranzxDoc* pDoc = (CTranzxDoc*)GetDocument();
//	ASSERT(pDoc);
//
//	CRect rect;
//	GetTranzxRect(rect);
//	rect.right -= ITEM_NAME_X;
//
//	for (int scale = 0; scale < SIZE_SCALE; scale ++) {
//		int maxx = (int)pDoc->GetMaxX();
//		if (maxx < rect.Width())	return scale;
//	}
//	return SIZE_SCALE - 1;
//}
//
//int CTranzxView::GetScale()
//{
//	CTranzxDoc* pDoc = (CTranzxDoc*)GetDocument();
//	ASSERT(pDoc);
//
//	return pDoc->GetScale();
//}
//
//void CTranzxView::SetScale(int scale)
//{
//	CTranzxDoc* pDoc = (CTranzxDoc*)GetDocument();
//	ASSERT(pDoc);
//
//	pDoc->SetScale(scale);
//	Redraw();
//	SetGrid(m_nGrid);
//}

void CTranzxView::Reposition(CRect rect)
{
	if (m_nType == FILETYPE_LOGBOOK) {
		if (m_pHole != NULL) {
			m_rectHole = rect;
			m_rectHole.bottom -= (ITEM_TIME_Y + 1);
			m_pHole->SetWindowPos(NULL, m_rectHole.left, m_rectHole.top, m_rectHole.Width(), m_rectHole.Height(),
									SWP_NOZORDER | SWP_NOACTIVATE);
			m_pHole->Redraw(m_ptCur);
		}
		if (m_pFactPanel != NULL) {
			m_rectFack = rect;
			m_rectFack.top = m_rectFack.bottom - ITEM_TIME_Y;
			m_rectFack.right = m_rectFack.left + ITEM_NAME_X;
			m_pFactPanel->SetWindowPos(NULL, m_rectFack.left, m_rectFack.top, 0, 0,
									SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
		}
		if (m_pTickPanel != NULL) {
			m_rectTick = rect;
			m_rectTick.top = m_rectTick.bottom - ITEM_TIME_Y;
			m_rectTick.left += ITEM_NAME_X;
			m_pTickPanel->SetWindowPos(NULL, m_rectTick.left, m_rectTick.top, m_rectTick.Width(), m_rectTick.Height(),
									SWP_NOZORDER | SWP_NOACTIVATE);
			//m_pTickPanel->SetBound(m_rectTick);
			m_pTickPanel->Draw();
		}
		if (m_pHole != NULL) {
			//m_sizeTotal.cx = pDoc->GetMaxXbyScale();
			m_sizeTotal.cy = m_pHole->GetHeight();
			SetScrollBar(m_sizeTotal);
		}
	}
	else if (m_nType == FILETYPE_TROUBLE) {
		if (m_pListTrouble != NULL) {
			CRect rect, rectList;
			GetClientRect(rect);
			m_pListTrouble->GetWindowRect(rectList);
			if (rect.Height() > rectList.Height())
				m_pListTrouble->SetWindowPos(NULL, 0, 0, rectList.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
		}
	}
}

void CTranzxView::Draw()
{
	//pDoc->SetScale(0);

	CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
	ASSERT(pFrame);
	CTranzxDoc* pDoc = (CTranzxDoc*)GetDocument();
	ASSERT(pDoc);
	if (pFrame->GetItemType() != FILETYPE_LOGBOOK)	pFrame->MakeItemList((CDocument*)pDoc, FILETYPE_LOGBOOK);

	CString str;
	CTick start, end;
	if (pDoc->GetLogbookTime((uint64_t)0, start) < 0 || pDoc->GetLogbookTime((uint64_t)pDoc->GetLogbookLength() - 1, end) < 0) {
		str.Format(L"[VIEW] 시간대가 맞지않습니다!\n");
		return;
	}
	str.Format(L"[VIEW] 시작=%s  끝=%s.", start.Format(), end.Format());
	Log(str);

	BeginWaitCursor();
	m_sizeTotal.cx = (long)pDoc->GetMaxX();
	m_sizeTotal.cy = 100;
	SetScrollBar(m_sizeTotal);
	str.Format(L"[VIEW] 스크롤 크기 x=%d y=%d", m_sizeTotal.cx, m_sizeTotal.cy);
	Log(str);

	CRect rect;
	GetDrawRect(rect);
	rect.right = rect.left + MAX_X;
	//rect.right = rect.left + m_sizeTotal.cx;
	str.Format(L"[VIEW] left=%d right=%d top=%d bottom=%d.", rect.left, rect.right, rect.top, rect.bottom);
	Log(str);

	if (m_pHole != NULL)	delete m_pHole;
	m_pHole = new CHole(this);
	m_rectHole = rect;
	m_rectHole.bottom -= (ITEM_TIME_Y + 1);
	m_pHole->Create(WS_VISIBLE | WS_CHILD, m_rectHole, this);

	if (m_pFactPanel != NULL)	delete m_pFactPanel;
	m_pFactPanel = new CFactPanel(this);
	m_rectFack = rect;
	m_rectFack.top = m_rectFack.bottom - ITEM_TIME_Y;
	m_rectFack.right = m_rectFack.left + ITEM_NAME_X;
	m_pFactPanel->Create(WS_VISIBLE | WS_CHILD, m_rectFack, this);
	//m_strFact = pDoc->GetFactStrByScale();
	//m_pFactPanel->Draw(m_strFact, m_strTick);
	//m_pFactPanel->Draw(m_strTick);

	if (m_pTickPanel != NULL)	delete m_pTickPanel;
	m_pTickPanel = new CTickPanel(this);
	m_rectTick = rect;
	m_rectTick.top = m_rectTick.bottom - ITEM_TIME_Y;
	m_rectTick.left += ITEM_NAME_X;
	//m_pTickPanel->Create(WS_VISIBLE | WS_CHILD | WS_BORDER, m_rectTick, this);
	m_pTickPanel->Create(WS_VISIBLE | WS_CHILD, m_rectTick, this);
	m_pTickPanel->Draw();

	if (m_pHole != NULL) {
		m_sizeTotal.cy = m_pHole->GetHeight();
		SetScrollBar(m_sizeTotal);
	}

	BuildByHistory();
	EndWaitCursor();
}

// TROUBLES ==================================================================

void CTranzxView::Enumerate()
{
	if (m_pListTrouble != NULL) {
		if (m_pListTrouble->GetItemCount() > 0)	m_pListTrouble->DeleteAllItems();
		delete m_pListTrouble;
	}

	m_fontTroubleList.CreateFontW(-12, 0, 0, 0, 400, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
					CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("맑은 고딕"));
	CRect rect;
	GetDrawRect(rect);
	m_pListTrouble = new CListCtrl();
	m_pListTrouble->Create(WS_CHILD | WS_VISIBLE | LVS_REPORT | LVS_SHOWSELALWAYS, rect, this, IDC_LIST_TROUBLE);
	m_pListTrouble->SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_pListTrouble->SetFont(&m_fontTroubleList);
	m_pListTrouble->ShowWindow(SW_SHOW);

	int width = 0;
	LV_COLUMN lc;
	CString str;
	for (int n = 0; m_listForm[n].pHeadName != L""; n ++) {
		lc.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
		lc.fmt = LVCFMT_CENTER;
		str = (LPCTSTR)m_listForm[n].pWidth;
		lc.cx = m_pListTrouble->GetStringWidth(str);
		width += lc.cx;
		str = (LPCTSTR)m_listForm[n].pHeadName;
		lc.pszText = str.GetBuffer(str.GetLength());
		lc.iSubItem = n;
		m_pListTrouble->InsertColumn(n, &lc);
		str.ReleaseBuffer();
	}
	rect.right = rect.left + width + 18;
	m_pListTrouble->SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
	m_pListTrouble->GetHeaderCtrl()->EnableWindow(FALSE);

	Arrange(TROUBLE_ALL);
}

void CTranzxView::Arrange(UINT troubleType)
{
	if (m_pListTrouble == NULL)	return;
	if (m_pListTrouble->GetItemCount() > 0)	m_pListTrouble->DeleteAllItems();

	CString str;
	CTranzxDoc* pDoc = (CTranzxDoc*)GetDocument();
	ASSERT(pDoc);

	GETTROUBLEITEM(pTroubleItem);

	CPtrList* pTroubleList = pDoc->GetTroubleList();
	if (pTroubleList->GetCount() > 0) {
		BeginWaitCursor();
		int line = 0;
		LV_ITEM li;
		for (POSITION pos = pTroubleList->GetHeadPosition(); pos != NULL; ) {
			PTROUBLEARCH pTrouble = (PTROUBLEARCH)pTroubleList->GetNext(pos);
			if (pTrouble != NULL) {
				if (((pTrouble->b.type == TROUBLE_TEXT || pTrouble->b.type == TROUBLE_EVENT) && (troubleType & 1)) ||
					((pTrouble->b.type == TROUBLE_STATUS || pTrouble->b.type == TROUBLE_WARNING) && (troubleType & 2)) ||
					(pTrouble->b.type == TROUBLE_LIGHT && (troubleType & 4)) ||
					((pTrouble->b.type == TROUBLE_SEVERE || pTrouble->b.type == TROUBLE_EMERGENCY) && (troubleType & 8))) {
					str.Format(L"%d", line + 1);
					li.mask = LVIF_TEXT;
					li.iItem = line;
					li.iSubItem = 0;
					li.pszText = str.GetBuffer(str.GetLength());
					m_pListTrouble->InsertItem(&li);
					str.ReleaseBuffer();

					str.Format(L"%02d/%02d %02d:%02d:%02d",
						(int)pTrouble->b.mon, (int)pTrouble->b.day, (int)pTrouble->b.uhour, (int)pTrouble->b.umin, (int)pTrouble->b.usec);
					SETITEM(m_pListTrouble, li, line, 1, str);

					if (pTrouble->b.down) {
						str.Format(L"%02d:%02d:%02d", (int)pTrouble->b.dhour, (int)pTrouble->b.dmin, (int)pTrouble->b.dsec);
						SETITEM(m_pListTrouble, li, line, 2, str);
					}

					str.Format(L"%d", pTrouble->b.code);
					SETITEM(m_pListTrouble, li, line, 3, str);

					str.Format(L"%d", pTrouble->b.cid);
					SETITEM(m_pListTrouble, li, line, 4, str);

					str = (LPCTSTR)m_pTroubleType[pTrouble->b.type];
					SETITEM(m_pListTrouble, li, line, 5, str);

					str = pTroubleItem->GetTroubleContext(pTrouble->b.code);
					SETITEM(m_pListTrouble, li, line, 6, str);
					++ line;
				}
			}
		}
		m_pListTrouble->SetRedraw(true);
		m_nTroubleType = troubleType;
		EndWaitCursor();
	}
	else {
		str.Format(L"[VIEW] 빈 파일입니다!!!");
		Log(str);
		MessageBox(str, L"읽기", MB_ICONWARNING);
	}
}

int CTranzxView::FindTroubleCode(UINT nFindCode, int nRow)
{
	if (m_nType != FILETYPE_TROUBLE || m_pListTrouble == NULL)	return -1;

	while (nRow < m_pListTrouble->GetItemCount()) {
		if (nRow > -1)
			m_pListTrouble->SetItemState(nRow, ~(LVIS_SELECTED | LVIS_FOCUSED), LVIS_SELECTED | LVIS_FOCUSED);
		++ nRow;
		CString str = m_pListTrouble->GetItemText(nRow, 3);
		UINT code = (UINT)atoi(CT2CA(str));
		if (code == nFindCode) {
			m_pListTrouble->SetItemState(nRow, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
			//m_pListTrouble->SetSelectionMark(nRow);
			m_pListTrouble->EnsureVisible(nRow, FALSE);
			m_pListTrouble->SetFocus();
			return nRow;
		}
	}
	MessageBox(L"검색이 끝났습니다", L"찾기", MB_ICONSTOP);
	return -1;
}

//============================================================================

BOOL CTranzxView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: CREATESTRUCT cs를 수정하여 여기에서
	//  Window 클래스 또는 스타일을 수정합니다.

	return CView::PreCreateWindow(cs);
}

BOOL CTranzxView::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN) {
		if (m_nType == FILETYPE_LOGBOOK) {
			if (pMsg->wParam == VK_LEFT || pMsg->wParam == VK_RIGHT) {
				if (m_pTickPanel != NULL && m_nGrid > -1) {
					int grid = m_nGrid;
					if (pMsg->wParam == VK_LEFT) {
						if (GetKeyState(VK_CONTROL))	grid -= 10;
						else	-- grid;
					}
					else {
						if (GetKeyState(VK_CONTROL))	grid += 10;
						else	++ grid;
					}
					CRect rect;
					m_pTickPanel->GetClientRect(rect);
					if (grid > rect.left && grid < rect.right)	SetGrid(grid);
				}

				::TranslateMessage(pMsg);
				::DispatchMessage(pMsg);
				return TRUE;
			}
		}
		else if (m_nType == FILETYPE_TROUBLE) {
			if ((GetKeyState(VK_CONTROL) && pMsg->wParam == 'F') || pMsg->wParam == VK_RETURN) {
				if (pMsg->wParam == VK_RETURN) {
					CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
					ASSERT(pFrame);
					if (pFrame->IsFindingTroubles())	pFrame->SearchTrouble();
				}
				else {
					bool bFolder = (GetKeyState(VK_SHIFT) & 0x8000) ? true : false;
					CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
					pFrame->LaunchFindTroubles(bFolder);
				}
				::TranslateMessage(pMsg);
				::DispatchMessage(pMsg);
				return TRUE;
			}
		}
	}

	return CView::PreTranslateMessage(pMsg);
}

int CTranzxView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void CTranzxView::OnDraw(CDC* /*pDC*/)
{
}

void CTranzxView::OnSize(UINT nType, int cx, int cy)
{
	CView::OnSize(nType, cx, cy);

	CRect rect;
	GetDrawRect(rect);
	CSize sz = rect.Size();
	if (sz.cx > 0 && sz.cy > 0)	Reposition(rect);
}

void CTranzxView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	CClientDC dc(this);
	m_ppi.x = dc.GetDeviceCaps(LOGPIXELSX);
	m_ppi.y = dc.GetDeviceCaps(LOGPIXELSY);

	CRect rect;
	GetClientRect(rect);
	TRACE(L"[VIEW]begin OnInitialUpdate() left=%d right=%d top=%d bottom=%d\n", rect.left, rect.right, rect.top, rect.bottom);

	CTranzxDoc* pDoc = (CTranzxDoc*)GetDocument();
	m_nType = pDoc->GetType();
	if (m_nType == FILETYPE_LOGBOOK) {
		CString strPath = pDoc->GetPathName();
		GetParent()->SetWindowTextW(strPath);
		Draw();
	}
	else if (m_nType == FILETYPE_TROUBLE) {
		CString strPath = pDoc->GetPathName();
		GetParent()->SetWindowTextW(strPath);
		Enumerate();
	}
	TRACE(L"[VIEW]end OnInitialUpdate()\n");
}

void CTranzxView::OnRButtonUp(UINT nFlags, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CTranzxView::OnContextMenu(CWnd* pWnd, CPoint point)
{
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
}

void CTranzxView::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_ALL;
	m_pHorz->GetScrollInfo(&si);

	switch (nSBCode) {
	case SB_TOP :		si.nPos = 0;		break;
	case SB_BOTTOM :	si.nPos = si.nMax;	break;
	case SB_PAGEUP :	si.nPos += si.nPage;	break;
	case SB_PAGEDOWN :	si.nPos -= si.nPage;	break;
	case SB_LINEUP :	si.nPos -= 1;		break;
	case SB_LINEDOWN :	si.nPos += 1;		break;
	case SB_THUMBTRACK :
	case SB_THUMBPOSITION :	si.nPos = nPos;	break;
	case SB_ENDSCROLL:	return;
	default :	break;
	}
	if (si.nPos < 0)	si.nPos = 0;
	else if (si.nPos > si.nMax)	si.nPos = si.nMax;
	m_pHorz->SetScrollInfo(&si);

	m_ptCur.x = si.nPos;
	//TRACE("[VIEW]HSCROLL %d\n", m_ptCur.x);

	if (m_pTickPanel != NULL)	m_pTickPanel->SetPos(m_ptCur);
	if (m_pHole != NULL) {
		m_pHole->SetPos(m_ptCur);
		m_pHole->SetGrid(m_nGrid);
	}

	SetGridTime();

	/*CString str;
	str.Format(L"H pos=%d", si.nPos);
	Log(str);*/

	//CView::OnHScroll(nSBCode, nPos, pScrollBar);
}

void CTranzxView::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.fMask = SIF_ALL;
	m_pVert->GetScrollInfo(&si);

	switch (nSBCode) {
	case SB_TOP :		si.nPos = 0;		break;
	case SB_BOTTOM :	si.nPos = si.nMax;	break;
	case SB_PAGEUP :	si.nPos += si.nPage;	break;
	case SB_PAGEDOWN :	si.nPos -= si.nPage;	break;
	case SB_LINEUP :	si.nPos -= 1;		break;
	case SB_LINEDOWN :	si.nPos += 1;		break;
	case SB_THUMBTRACK :
	case SB_THUMBPOSITION :	si.nPos = nPos;	break;
	case SB_ENDSCROLL:	return;
	default :	break;
	}
	if (si.nPos < 0)	si.nPos = 0;
	else if (si.nPos > si.nMax)	si.nPos = si.nMax;
	m_pVert->SetScrollInfo(&si);

	m_ptCur.y = si.nPos;
	if (m_pHole != NULL)	m_pHole->SetPos(m_ptCur.y);

	//CString str;
	//str.Format(L"V pos=%d", si.nPos);
	//Log(str);

	//CView::OnVScroll(nSBCode, nPos, pScrollBar);
}

LRESULT CTranzxView::OnSelItem(WPARAM wParam, LPARAM lParam)
{
	int id = (int)wParam;
	CTranzxDoc* pDoc = (CTranzxDoc*)GetDocument();
	ASSERT(pDoc);

	TCHAR type = pDoc->GetLBArchiveType(id);
	if ((type == 'H' || type == 'K' || type == 'L' || type == 'D' || type == 'S' || type == 'U') && m_pHole != NULL) {
		CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
		m_pHole->Entrance(id, pFrame->GetSelectedItemName(), m_crDefault, NULL);
	}

	return 0;
}

LRESULT CTranzxView::OnDropItem(WPARAM wParam, LPARAM lParam)
{
	int id = (int)wParam;
	CPoint* pPoint = (CPoint*)lParam;
	CTranzxDoc* pDoc = (CTranzxDoc*)GetDocument();
	ASSERT(pDoc);

	TCHAR type = pDoc->GetLBArchiveType(id);
	if ((type == 'H' || type == 'K' || type == 'L' || type == 'D' || type == 'S' || type == 'U') && m_pHole != NULL) {
		CColorDialog dlg(0, 0, this);
		if (dlg.DoModal() == IDOK) {
			COLORREF color = dlg.m_cc.rgbResult;
			CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
			m_pHole->Entrance(id, pFrame->GetSelectedItemName(), color,
								type == 'D' || type == 'S' || type == 'U' ? pPoint : NULL);
		}
	}

	return 0;
}

LRESULT CTranzxView::OnPartSelect(WPARAM wParam, LPARAM lParam)
{
	int id = (int)wParam;
	CRect rect = m_pHole->PartSelect(id);
	CString str;
	str.Format(L"[LAND] left=%d top=%d right=%d bottom=%d", rect.left, rect.top, rect.right, rect.bottom);
	Log(str);

	CRect rectHole = m_pHole->GetRect();
	rectHole.OffsetRect(0, m_ptCur.y);
	if (rectHole.top <= rect.top && rect.bottom <= rectHole.bottom) {
	}
	else {
		if (rect.top < rectHole.top)	m_ptCur.y -= (rectHole.top - rect.top);
		else	m_ptCur.y += (rect.bottom - rectHole.bottom);
		m_pHole->SetPos(m_ptCur.y);
		m_pVert->SetScrollPos(m_ptCur.y);
	}
	return 0;
}

LRESULT CTranzxView::OnSelectTrouble(WPARAM wParam, LPARAM lParam)
{
	if (m_nType == FILETYPE_TROUBLE) {
		UINT troubleType = (UINT)wParam;
		if (troubleType != m_nTroubleType)	Arrange(troubleType);
	};
	return 0;
}

void CTranzxView::OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult)
{
	if (m_pListTrouble != NULL) {
		NM_LISTVIEW* pNmListView = (NM_LISTVIEW*)pNMHDR;
		int iItem = pNmListView->iItem;
		if (iItem >= 0 && iItem < m_pListTrouble->GetItemCount()) {
			if (MessageBox(L"해당 시간의 운행 기록을 보시겠습니까?", L"읽기", MB_ICONQUESTION | MB_YESNO) == IDYES) {
				CString str = m_pListTrouble->GetItemText(iItem, 3);
				UINT code = (UINT)atoi(CT2CA(str));
				CString strTime = m_pListTrouble->GetItemText(iItem, 1);
				strTime.Replace(TCHAR('/'), TCHAR(' '));
				strTime.Replace(TCHAR(':'), TCHAR(' '));
				CTick time;
				const WCHAR* pszTime = (const WCHAR*)strTime;
				int res = swscanf_s(pszTime, L"%d %d %d %d %d", &time.m_nMon, &time.m_nDay, &time.m_nHour, &time.m_nMin, &time.m_nSec);

				CTranzxDoc* pDoc = (CTranzxDoc*)GetDocument();
				CString strPath = pDoc->GetPathName();
				if (strPath.IsEmpty())
					MessageBox(L"파일 이름을 찾을 수 없습니다", L"읽기", MB_ICONWARNING);
				else {
					int prev = -1;
					for (int n = 0; n >= 0; ) {
						prev = n;
						n = strPath.Find((TCHAR)'\\', n + 1);
					}
					if (prev > 0 && (prev + 1) < strPath.GetLength() && strPath.GetAt(prev + 1) == L't') {
						CString strLogbook;
						strLogbook.Format(L"%s\\%c", (LPCTSTR)strPath.Left(prev), FILETYPE_LOGBOOK);
						strLogbook += strPath.Mid(prev + 2);
						TRACE(L"Logbook load \"%s\"\n", strLogbook);

						CTranzxApp* pApp = (CTranzxApp*)AfxGetApp();
						pApp->OpenDocumentFile(strLogbook);
						TRACE(L"find time now!!!\n");

						CMainFrame* pFrame = (CMainFrame*)AfxGetMainWnd();
						ASSERT(pFrame);
						CChildFrame* pChild = (CChildFrame*)pFrame->GetActiveFrame();
						ASSERT(pChild);
						CTranzxView* pView = (CTranzxView*)pChild->GetActiveView();
						ASSERT(pView);
						pView->FindLogbookTime(time);
					}
					else	MessageBox(L"형식에 맞지않는 파일 이름입니다!", L"읽기", MB_ICONWARNING);
				}
			}
		}
	}
	*pResult = 0;
}

// 인쇄 ========================================================================

void CTranzxView::OnFilePrintPreview()
{
	AFXPrintPreview(this);
}

void CTranzxView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	int hinch = (int)(pDC->GetDeviceCaps(HORZSIZE) / 25.4);
	int vinch = (int)(pDC->GetDeviceCaps(VERTSIZE) / 25.4);
	int xppi = pDC->GetDeviceCaps(LOGPIXELSX);
	int yppi = pDC->GetDeviceCaps(LOGPIXELSY);

	CSize factor;
	factor.cx = xppi / m_ppi.x;
	factor.cy = yppi / m_ppi.y;

	CRect rect = CRect(0, 0, 0, 0);
	rect.top = (int)(SPACE_TOP * yppi / 25.4);
	rect.bottom = rect.top + 100;

	if (m_pHole != NULL)	m_pHole->Copy(pDC, pInfo->m_nCurPage, hinch * m_ppi.x, factor, rect);
	CPoint org;
	org.x = 0;
	org.y = rect.bottom;
	if (pInfo->m_nCurPage == 1) {
		if (m_pFactPanel != NULL)	m_pFactPanel->Copy(pDC, org, factor, rect);
		org.x += (ITEM_NAME_X * factor.cx);
	}
	if (m_pTickPanel != NULL)	m_pTickPanel->Copy(pDC, org, pInfo->m_nCurPage, hinch * m_ppi.x, factor, rect);

	CFont* pFontOld = (CFont*)pDC->SelectObject(&m_fontPage);

	rect.top = rect.left = 0;
	rect.right = hinch * xppi;
	rect.bottom = (int)(SPACE_TOP * yppi / 25.4);
	CString str = L"운행 기록        ";
	//str += L"GetFrameLog()";
	CTranzxDoc* pDoc = (CTranzxDoc*)GetDocument();
	str += pDoc->GetPathName();
	pDC->DrawText(str, rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);

	rect.bottom = vinch * yppi;
	rect.top = rect.bottom - (int)(SPACE_BOTTOM * yppi / 25.4);

	str.Format(L"- %d - ", pInfo->m_nCurPage);
	pDC->DrawText(str, rect, DT_CENTER | DT_SINGLELINE | DT_VCENTER);
	pDC->SelectObject(pFontOld);

	CView::OnPrint(pDC, pInfo);
}

BOOL CTranzxView::OnPreparePrinting(CPrintInfo* pInfo)
{
	if (pInfo == NULL)	return FALSE;

	PRINTDLG pd;
	pd.lStructSize = (DWORD)sizeof(PRINTDLG);
	BOOL bRet = AfxGetApp()->GetPrinterDeviceDefaults(&pd);
	if (bRet) {
		LPDEVMODE pdm = (LPDEVMODE)::GlobalLock(pd.hDevMode);
		if (pdm->dmPaperSize == DMPAPER_A4)
			pdm->dmOrientation = DMORIENT_PORTRAIT;
		else	pdm->dmOrientation = DMORIENT_LANDSCAPE;
		::GlobalUnlock(pd.hDevMode);
	}

	CPrintDialog dlg(FALSE);
	if (!dlg.GetDefaults())	return FALSE;

	CDC dc;
	dc.Attach(dlg.GetPrinterDC());
	//CString str;
	//str.Format(L"Hsize=%d Vsize=%d Hres=%d Vres=%d Xlogp=%d Ylogp=%d aspectx=%d aspecty=%d",
	//	dc.GetDeviceCaps(HORZSIZE), dc.GetDeviceCaps(VERTSIZE), dc.GetDeviceCaps(HORZRES), dc.GetDeviceCaps(VERTRES),
	//	dc.GetDeviceCaps(LOGPIXELSX), dc.GetDeviceCaps(LOGPIXELSY), m_ppi.x, m_ppi.y);
	//MessageBox(str);

	CTranzxDoc* pDoc = GetDocument();
	int maxx = (int)pDoc->GetMaxX();
	int pgx = (int)(dc.GetDeviceCaps(HORZSIZE) / 25.4 * m_ppi.x);
	int maxpg = maxx / pgx;
	if (maxx % pgx)	++ maxpg;
	CString str;
	str.Format(L"[VIEW] print max page=%d", maxpg);
	Log(str);
	pInfo->SetMaxPage(maxpg);

	dc.Detach();

	return DoPreparePrinting(pInfo);
}

void CTranzxView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

void CTranzxView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
}

#ifdef _DEBUG
void CTranzxView::AssertValid() const
{
	CView::AssertValid();
}

void CTranzxView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CTranzxDoc* CTranzxView::GetDocument() const
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CTranzxDoc)));
	return (CTranzxDoc*)m_pDocument;
}
#endif //_DEBUG
