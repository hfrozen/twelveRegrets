// AidTitle.cpp : 구현 파일입니다.
//

#include "stdafx.h"
#include "Tranzx.h"
#include "AidTitle.h"

#include "TranzxView.h"
#include "Item.h"

#include <math.h>

// CAidTitle

IMPLEMENT_DYNAMIC(CAidTitle, CWnd)

CAidTitle::CAidTitle(CWnd* pParent)
{
	m_pParent = pParent;
	m_pscMin = m_pscMax = m_logMin = m_logMax = 0;
	m_strName = L"";
	m_strUnit = L"";
	m_nDecimal = 0;
	m_bSelected = false;
	m_crBkgnd = RGB(255, 255, 255);
	m_crText = RGB(16, 16, 16);
	m_crProg = RGB(170, 251, 72);
	m_brBkgnd.CreateSolidBrush(m_crBkgnd);
	m_brProg.CreateSolidBrush(m_crProg);
	m_ptCur = CPoint(0, 0);
	m_pBitmap = NULL;
	m_pProgress = NULL;
	m_nID = -1;
	m_nType = ' ';
}

CAidTitle::~CAidTitle()
{
	if (m_pBitmap != NULL)
		m_dc.SelectObject(m_pBitmap);
	m_font.DeleteObject();
	m_fontHz.DeleteObject();
	m_brBkgnd.DeleteObject();
	m_brProg.DeleteObject();
	if (m_pProgress != NULL) {
		delete m_pProgress;
		m_pProgress = NULL;
	}
}

void CAidTitle::InvalidateCtrl()
{
	CRect rect;
	GetClientRect(rect);

	CClientDC dc(this);
	if (m_dc.GetSafeHdc() == NULL) {
		m_dc.CreateCompatibleDC(&dc);
		m_bitmap.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
		m_pBitmap = (CBitmap*)m_dc.SelectObject(&m_bitmap);
	}

	m_dc.SetBkColor(m_crBkgnd);
	m_dc.FillRect(rect, &m_brBkgnd);

	CFont* pFondOld = (CFont*)m_dc.SelectObject(&m_font);
	m_dc.SetTextColor(m_crText);
	//CPen solid(PS_SOLID, 0, m_crText);
	//CPen* pPenOld = (CPen*)m_dc.SelectObject(&solid);

	if (m_nID > -1) {
		CPoint pt = rect.CenterPoint();
		if (m_pProgress)
			m_pProgress->SetWindowPos(NULL, pt.x, rect.top + HEIGHT_TEXT, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);

		CString str;
		if (m_nType == 'S' || m_nType == 'U') {
			int64_t cv = m_logMax == 0 ? m_pscMax : m_logMax;
			if (m_nDecimal != 0) {
				double db = (double)cv / pow(10.f, m_nDecimal);
				str.Format(L"%.*f", m_nDecimal, db);
			}
			else	str.Format(L"%lld", cv);
			if (!m_strUnit.IsEmpty())	str += m_strUnit;
			m_dc.SetTextAlign(TA_BOTTOM | TA_RIGHT);
			m_dc.TextOut(rect.right, rect.top + HEIGHT_TEXT, str);

			cv = m_logMax == 0 ? m_pscMin : m_logMin;
			if (m_nDecimal != 0) {
				double db = (double)cv / pow(10.f, m_nDecimal);
				str.Format(L"%.*f", m_nDecimal, db);
			}
			else	str.Format(L"%lld", cv);
			if (!m_strUnit.IsEmpty())	str += m_strUnit;
			m_dc.SetTextAlign(TA_TOP | TA_RIGHT);
			m_dc.TextOut(rect.right, rect.bottom - HEIGHT_TEXT, str);
			m_dc.FrameRect(m_rtProg, &m_brProg);

			if (m_bSelected) {
				m_dc.SetBkColor(RGB(54, 96, 239));
				m_dc.SetTextColor(RGB(255, 255, 255));
			}
			str.Format(L"%s", m_strName);
			m_dc.SetTextAlign(TA_TOP | TA_RIGHT);
			m_dc.TextOutW(rect.right - (PROGRESSBAR_X + 4), rect.top + ((rect.Height() - 1) / 2) - (HEIGHT_TEXT / 2), str);
		}
		else {
			if (m_bSelected) {
				m_dc.SetBkColor(RGB(71, 112, 252));
				m_dc.SetTextColor(RGB(255, 255, 255));
			}
			str.Format(L"%s", m_strName);
			m_dc.SetTextAlign(TA_RIGHT | TA_BOTTOM);
			m_dc.TextOut(rect.right - 5, rect.bottom - 1, str);
		}
	}
	m_dc.SelectObject(pFondOld);

	InvalidateRect(rect);
}

void CAidTitle::Current(int64_t v, COLORREF color)
{
	if (m_pProgress != NULL) {
		m_pProgress->SetPos((int)v);
	}
}

bool CAidTitle::SetItem(int nID, TCHAR nType, BYTE nLeng, CString strName,
						int64_t pscMin, int64_t pscMax, int64_t logMin, int64_t logMax,
						CString strUnit, COLORREF color = RGB(255, 255, 255))
{
	if (nID < 1)	return false;
	if (nType == 'D') {
	}
	else if (nType == 'H' || nType == 'S' || nType == 'U' || nType == 'K' || nType == 'L') {
		if (nLeng != 1 && nLeng != 2 && nLeng != 4 && nLeng != 6 && nLeng != 8)	return false;
	}
	else	return false;
	if (strName.IsEmpty() || strName == L"")	return false;
	if (nType != 'K' && nType != 'L' && pscMin > pscMax)	return false;

	m_nID = nID;
	m_nType = nType;
	m_nLength = nLeng;
	m_strName = strName;
	m_pscMin = pscMin;
	m_pscMax = pscMax;
	m_logMin = logMin;
	m_logMax = logMax;
	m_strUnit = strUnit;
	m_crText = color;
	m_nID = nID;
	if (m_pProgress != NULL)	m_pProgress->SetRange32((int)m_pscMin, (int)m_pscMax);
	if (m_strUnit.GetLength() > 0 && m_strUnit.GetAt(0) == L'.') {
		TCHAR ch = m_strUnit.GetAt(1);
		if (ch >= L'0' && ch <= L'9') {
			m_nDecimal = ch - L'0';
			m_strUnit = m_strUnit.Mid(2);
		}
	}
	InvalidateCtrl();
	return true;
}

/*void CAidTitle::SetPos(CPoint pt)
{
	m_ptCur = pt;
	Invalidate();
}*/

void CAidTitle::SetValue(TCHAR type, BYTE leng, _variant_t var)
{
	int64_t v;
	if (leng > 4) {
		if (type == 'S' || type == 'K')	v = var.llVal;
		else	v = var.ullVal;
	}
	else if (leng > 2) {
		if (type == 'S' || type == 'K')	v = var.lVal;
		else	v = var.ulVal;
	}
	else if (leng > 1) {
		if (type == 'S' || type == 'K')	v = var.iVal;
		else	v = var.uiVal;
	}
	else	v = var.bVal;
	if (m_pProgress != NULL)	m_pProgress->SetPos((int)v);

	int64_t	cv = v;
	if (m_logMax != 0 && (m_pscMax - m_pscMin) > 0)
		cv = (v - m_pscMin) * (m_logMax - m_logMin) / (m_pscMax - m_pscMin) + m_logMin;
	else if (m_logMin != 0)	cv /= m_logMin;

	CString str;
	if (m_nDecimal != 0) {
		double db = (double)cv / pow(10.f, m_nDecimal);
		str.Format(L"%.*f", m_nDecimal, db);
	}
	else {
		if (leng > 4) {
			if (type == 'S' || type == 'K')	str.Format(L"%lld", cv);
			else {
				if (type == 'H')	str.Format(L"0x%08X", cv);
				else	str.Format(L"%llu", cv);
			}
		}
		else if (leng > 2) {
			if (type == 'S' || type == 'K')	str.Format(L"%ld", cv);
			else {
				if (type == 'H')	str.Format(L"0x%04X", cv);
				else	str.Format(L"%lu", cv);
			}
		}
		else if (leng > 1) {
			if (type == 'S' || type == 'K')	str.Format(L"%d", cv);
			else {
				if (type == 'H')	str.Format(L"0x%02X", cv);
				else	str.Format(L"%u", cv);
			}
		}
		else {
			if (type == 'H')	str.Format(L"0x%02X", cv);
			else	str.Format(L"%u", cv);
		}
	}
	if (!m_strUnit.IsEmpty())	str += m_strUnit;

	//CClientDC dc(this);
	if (m_dc.GetSafeHdc() == NULL)	return;
	if (m_pscMax <= m_pscMin)	return;

	m_dc.SetBkColor(m_crBkgnd);
	m_dc.FillRect(m_rtProg, &m_brBkgnd);
	m_dc.FrameRect(m_rtProg, &m_brProg);

	int y = (int)((v - m_pscMin) * m_rtProg.Height() / (m_pscMax - m_pscMin));
	if (y > m_rtProg.Height())	y = m_rtProg.Height();
	CRect rect = m_rtProg;
	rect.top = rect.bottom - y;
	m_dc.FillRect(rect, &m_brProg);

	CFont* pFontOld = (CFont*)m_dc.SelectObject(&m_fontHz);
	m_dc.SetTextColor(m_crText);
	m_dc.SetTextAlign(TA_CENTER | TA_TOP);
	m_dc.SetBkMode(TRANSPARENT);
	//m_dc.TextOut(m_rtProg.left, m_rtProg.left + m_rtProg.Height() / 2, str);
	m_dc.TextOut(m_rtProg.left, m_rtProg.top + m_rtProg.Height() / 2, str);

	m_dc.SelectObject(pFontOld);
	InvalidateRect(m_rtProg);
}

/*void CAidTitle::SetGridColor(COLORREF color)
{
	m_crGrid = color;
	InvalidateCtrl();
}*/

void CAidTitle::SetBkColor(COLORREF color)
{
	m_crBkgnd = color;
	m_brBkgnd.DeleteObject();
	m_brBkgnd.CreateSolidBrush(m_crBkgnd);
	InvalidateCtrl();
}

void CAidTitle::SetTextColor(COLORREF color)
{
	m_crText = color;
	InvalidateCtrl();
}

int CAidTitle::GetWidth()
{
	CRect rect;
	GetClientRect(rect);

	return rect.Width();
}

bool CAidTitle::PartSelect(int id)
{
	m_bSelected = id == m_nID ? true : false;
	InvalidateCtrl();
	return m_bSelected;
}

bool CAidTitle::Copy(CDC* pDC, CSize factor, CRect& fRect)
{
	CRect rect;
	GetClientRect(rect);

	CDC dcMem;
	dcMem.CreateCompatibleDC(pDC);

	CBitmap bitmapMem;
	bitmapMem.CreateCompatibleBitmap(pDC, rect.Width(), rect.Height());
	CBitmap* pBitmapOld = (CBitmap*)dcMem.SelectObject(&bitmapMem);

	if (dcMem.GetSafeHdc() == NULL) {
		dcMem.SelectObject(pBitmapOld);
		return false;
	}
	dcMem.BitBlt(0, 0, rect.Width(), rect.Height(), &m_dc, 0, 0, SRCCOPY);
	pDC->StretchBlt(fRect.left, fRect.top, rect.Width() * factor.cx, rect.Height() * factor.cy,
		&dcMem, 0, 0, rect.Width(), rect.Height(), SRCCOPY);

	fRect.right = fRect.left + rect.Width() * factor.cx;
	fRect.bottom = fRect.top + rect.Height() * factor.cy;

	dcMem.SelectObject(pBitmapOld);
	return true;
}

void CAidTitle::SetBound(CRect rect)
{
	SetWindowPos(NULL, 0, 0, rect.Width(), rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
}

CString CAidTitle::GetInfo()
{
	CString str;
	str.Format(L"%u<%u>[%s]", m_nID, m_crText, m_strName);
	return str;
}

BOOL CAidTitle::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	static CString strClassName = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW);

	if (CWnd::CreateEx(0, strClassName, NULL, dwStyle,
		rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, pParentWnd->GetSafeHwnd(), (HMENU)nID)) {
		m_pParent = pParentWnd;

		m_font.CreateFont(HEIGHT_TEXT, 0, 0, 0, 300, false, false, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
		m_fontHz.CreateFont(HEIGHT_TEXT, 0, 900, 0, 300, false, false, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");

		if (m_pProgress != NULL) {
			delete m_pProgress;
			m_pProgress = NULL;
		}

		CRect clientRect;
		GetClientRect(clientRect);
		m_rtProg.left = clientRect.right - (PROGRESSBAR_X + 2);	//left + clientRect.Width() / 2;
		m_rtProg.right = clientRect.right - 2;
		m_rtProg.top = clientRect.top + HEIGHT_TEXT;
		m_rtProg.bottom = clientRect.bottom - HEIGHT_TEXT;

		CRect btr = clientRect;
		btr.right = btr.left + SIZE_BUTTON;
		btr.bottom = btr.top + SIZE_BUTTON;
		m_button.Create(L"X", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, btr, this, 0xffffffff);
		m_button.EnableWindow(false);
		m_button.ShowWindow(SW_HIDE);

		/*m_pProgress = new CProgressCtrl();
		if (m_pProgress->Create(WS_CHILD | WS_VISIBLE | PBS_VERTICAL | PBS_SMOOTH, prt, this, 0xffffffff)) {
			m_pProgress->SetWindowPos(NULL, rt.left + rt.Width() / 2,
				rect.top + HEIGHT_TEXT, 0, 0, SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOSIZE);
			m_pProgress->SetRange(0, 100);
		}*/
		InvalidateCtrl();
		return true;
	}
	return false;
}

BEGIN_MESSAGE_MAP(CAidTitle, CWnd)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_LBUTTONUP()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

void CAidTitle::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	// TODO: 여기에 메시지 처리기 코드를 추가합니다.
	// 그리기 메시지에 대해서는 CWnd::OnPaint()을(를) 호출하지 마십시오.

	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);

	CRect rect;
	GetClientRect(rect);

	CBitmap bitmapMem;
	bitmapMem.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
	CBitmap* pBitmap = (CBitmap*)dcMem.SelectObject(&bitmapMem);

	if (dcMem.GetSafeHdc() != NULL) {
		dcMem.BitBlt(0, 0, rect.Width(), rect.Height(), &m_dc, 0, 0, SRCCOPY);
		dc.BitBlt(0, 0, rect.Width(), rect.Height(), &dcMem, 0, 0, SRCCOPY);
	}
	dcMem.SelectObject(pBitmap);
}

void CAidTitle::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
}

void CAidTitle::OnLButtonUp(UINT nFlags, CPoint point)
{
	CRect rect;
	m_button.GetClientRect(rect);
	bool bInhibit = false;
	if (rect.PtInRect(point)) {
		ReleaseCapture();
		CItem* pItem = (CItem*)m_pParent;
		m_pParent->SendMessage(WM_PARTDELETE, (WPARAM)m_nID);
		bInhibit = true;
	}
	else {
		CTranzxView* pView = (CTranzxView*)m_pParent->GetParent()->GetParent()->GetParent();
		pView->SendMessage(WM_PARTSELECT, (WPARAM)m_nID);
	}

	if (!bInhibit)
		CWnd::OnLButtonUp(nFlags, point);	// !!!!!very important!!!!!
}

void CAidTitle::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	CRect rect;
	GetClientRect(rect);

	if (rect.PtInRect(point)) {
		SetCapture();
		m_button.EnableWindow(true);
		m_button.ShowWindow(SW_SHOW);
	}
	else {
		m_button.EnableWindow(false);
		m_button.ShowWindow(SW_HIDE);
		ReleaseCapture();
	}

	CWnd::OnMouseMove(nFlags, point);
}
