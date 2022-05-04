// Body.cpp
#include "stdafx.h"
#include "Dts.h"
#include "Body.h"

IMPLEMENT_DYNAMIC(CBody, CWnd)

CBody::CBody()
{
	m_pParent = NULL;
	m_crBkgnd = RGB(255, 255, 255);
	m_brBkgnd.CreateSolidBrush(m_crBkgnd);

	ZeroMemory(&m_style, sizeof(BODYSTYLE));

	for (int n = BID_PANTOUP; n < BID_MAX; n ++)
		m_const[n].pBm = NULL;
	for (int n = 0; n < AREA_MAX; n ++)
		m_rectPart[n] = CRect(0, 0, 0, 0);
	m_bEnableClick = false;
}

CBody::~CBody()
{
	for (int n = BID_PANTOUP; n < BID_MAX; n ++) {
		if (m_const[n].pBm != NULL) {
			m_const[n].dc.SelectObject(m_const[n].pBm);
			m_const[n].dc.DeleteDC();
		}
	}
	m_brBkgnd.DeleteObject();
	m_font.DeleteObject();
}

void CBody::SetStyle(BODYSTYLE style)
{
	SetNumber(style.nID);
	SetType(style.nType);
	SetPanto(style.pan);
	SetCoach(style.coach);
	SetWheel(style.wheel);
	SetDoor(style.door);
	SetBypass(style.bypass);
	SetIHandle(style.ihandle);
	SetEHandle(style.ehandle);
	SetSide(style.side, true);
}

void CBody::SetNumber(int nID, bool bRedraw)
{
	m_style.nID = nID;
	if (bRedraw)	InvalidateRect(&m_rect);
}

void CBody::SetPanto(CARFORM_PANTO pan[2], bool bRedraw)
{
	for (int n = 0; n < 2; n ++) {
		if (pan[n] == CARFORM_PANDN || pan[n] == CARFORM_PANUP || pan[n] == CARFORM_PANBP)
			m_style.pan[n] = pan[n];
		else	m_style.pan[n] = CARFORM_PANX;
	}
	if (bRedraw)	InvalidateRect(&m_rect);
}

void CBody::SetCoach(CARFORM_COACH coach, bool bRedraw)
{
	if (coach == CARFORM_COACHL || coach == CARFORM_COACHR)	m_style.coach = coach;
	else	m_style.coach = CARFORM_COACHN;
	if (bRedraw)	InvalidateRect(&m_rect);
}

void CBody::SetWheel(CARFORM_WHEEL wheel, bool bRedraw)
{
	if (wheel == CARFORM_WHEELD)	m_style.wheel = wheel;
	else	m_style.wheel = CARFORM_WHEELU;
	if (bRedraw)	InvalidateRect(&m_rect);
}

void CBody::SetDoor(CARFORM_DOOR door[8], bool bRedraw)
{
	for (int n = 0; n < 8; n ++) {
		if (door[n] == CARFORM_DOOROP || door[n] == CARFORM_DOORBP)	m_style.door[n] = door[n];
		else	m_style.door[n] = CARFORM_DOORCL;
	};
	if (bRedraw)	InvalidateRect(&m_rect);
}

void CBody::SetSide(CARFORM_SIDE side, bool bRedraw)
{
	if (side == CARFORM_SIDENR || side == CARFORM_SIDEFL)	m_style.side = side;
	else	m_style.side = CARFORM_SIDEX;
	if (bRedraw)	InvalidateRect(&m_rect);
}

void CBody::SetBypass(CARFORM_BP bp, bool bRedraw)
{
	if (bp == CARFORM_BPON)	m_style.bypass = CARFORM_BPON;
	else	m_style.bypass = CARFORM_BPNR;
	if (bRedraw)	InvalidateRect(&m_rect);
}

void CBody::SetIHandle(CARFORM_HAND hand, bool bRedraw)
{
	if (hand == CARFORM_HANDON)	m_style.ihandle = CARFORM_HANDON;
	else	m_style.ihandle = CARFORM_HANDNR;
	if (bRedraw)	InvalidateRect(&m_rect);
}

void CBody::SetEHandle(CARFORM_HAND hand, bool bRedraw)
{
	if (hand == CARFORM_HANDON)	m_style.ehandle = CARFORM_HANDON;
	else	m_style.ehandle = CARFORM_HANDNR;
	if (bRedraw)	InvalidateRect(&m_rect);
}

void CBody::SetLamp(BYTE nLamp, bool bRedraw)
{
	m_style.nLamp = nLamp;
	if (bRedraw)	InvalidateRect(&m_rect);
}

void CBody::SetLampA(BYTE nLamp, bool bCtrl, bool bRedraw)
{
	if (bCtrl)	m_style.nLamp |= nLamp;
	else	m_style.nLamp &= ~nLamp;
	if (bRedraw)	InvalidateRect(&m_rect);
}

BOOL CBody::Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID)
{
	static CString strClassName = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW);
	m_pParent = pParentWnd;

	BOOL bResult = CWnd::CreateEx(/*WS_EX_CLIENTEDGE | WS_EX_STATICEDGE*/0, strClassName,
		NULL, dwStyle, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top,
		pParentWnd->GetSafeHwnd(), (HMENU)nID);
	if (bResult) {
		CClientDC dc(this);
		for (int n = 0; n < BID_MAX; n ++) {
			if (m_const[n].dc.GetSafeHdc() == NULL) {
				m_const[n].dc.CreateCompatibleDC(&dc);
				if (m_const[n].bm.LoadBitmapW(IDB_BITMAP_TPANDN + n)) {
					BITMAP bm;
					m_const[n].bm.GetBitmap(&bm);
					m_const[n].size.cx = bm.bmWidth;
					m_const[n].size.cy = bm.bmHeight;
				}
				m_const[n].pBm = (CBitmap*)m_const[n].dc.SelectObject(&m_const[n].bm);
			}
		}

		int cx = m_const[BID_COACHN].size.cx;
		int cy = m_const[BID_PANTOUP].size.cy + m_const[BID_COACHN].size.cy + m_const[BID_WHEELU].size.cy +
				m_const[BID_DOORC].size.cy * 2 + m_const[BID_CORRI].size.cy + SIZE_ROLLING * 2 + 10;
		m_rect = CRect(0, 0, cx, cy);
		SetWindowPos(NULL, 0, 0, m_rect.Width(), m_rect.Height(), SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOMOVE);
		m_font.CreateFont(-20, 0, 0, 0, 300, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
										DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, _T("Arial"));

		InvalidateRect(&m_rect);
	}
	return bResult;
}


BEGIN_MESSAGE_MAP(CBody, CWnd)
	ON_WM_PAINT()
	ON_WM_SIZE()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_WM_MOUSEMOVE()
END_MESSAGE_MAP()

void CBody::OnPaint()
{
	CPaintDC dc(this);

	CDC memDC;
	memDC.CreateCompatibleDC(&dc);
	CBitmap memBitmap;
	memBitmap.CreateCompatibleBitmap(&dc, m_rect.Width(), m_rect.Height());
	CBitmap* pOldBitmap = (CBitmap*)memDC.SelectObject(&memBitmap);
	memDC.SetBkColor(m_crBkgnd);
	memDC.FillRect(m_rect, &m_brBkgnd);

	CString str;
	str.Format(L"%d", m_style.nID);
	CFont* pOldFont = memDC.SelectObject(&m_font);
	memDC.SetTextColor(RGB(0, 0, 0));
	memDC.SetTextAlign(TA_CENTER | TA_TOP);
	memDC.TextOutW(m_rect.Width() / 2, 1, str);

	if (memDC.GetSafeHdc() != NULL) {
		int id;
		int y = 2 + SIZE_ROLLING;
		if (m_style.pan[0] != CARFORM_PANX) {
			switch (m_style.pan[0]) {
			case CARFORM_PANUP :	id = BID_PANTOUP;	break;
			case CARFORM_PANBP :	id = BID_PANTOBP;	break;
			default :				id = BID_PANTODN;	break;
			}
			memDC.BitBlt(18, y, m_const[id].size.cx, m_const[id].size.cy, &m_const[id].dc, 0, 0, SRCAND);
			m_rectPart[AREA_PANTO] = CRect(18, y, 18 + m_const[id].size.cx,	y + m_const[id].size.cy);
		}
		if (m_style.pan[1] != CARFORM_PANX) {
			switch (m_style.pan[1]) {
			case CARFORM_PANUP :	id = BID_PANTOUP;	break;
			case CARFORM_PANBP :	id = BID_PANTOBP;	break;
			default :				id = BID_PANTODN;	break;
			}
			memDC.BitBlt(68, y, m_const[id].size.cx, m_const[id].size.cy, &m_const[id].dc, 0, 0, SRCAND);
			m_rectPart[AREA_PANTO + 1] = CRect(68, y, 68 + m_const[id].size.cx, y + m_const[id].size.cy);
		}
		y += m_const[BID_PANTOUP].size.cy;

		switch (m_style.coach) {
		case CARFORM_COACHL :	id = BID_COACHL;	break;
		case CARFORM_COACHR :	id = BID_COACHR;	break;
		default :				id = BID_COACHN;	break;
		}
		memDC.BitBlt(0, y, m_const[id].size.cx, m_const[id].size.cy, &m_const[id].dc, 0, 0, SRCCOPY);
		int sid = (m_style.nLamp & 1) ? BID_DCLON : BID_DCLOFF;
		memDC.BitBlt(10, y + 3, m_const[sid].size.cx, m_const[sid].size.cy, &m_const[sid].dc, 0, 0, SRCCOPY);
		sid = (m_style.nLamp & 2) ? BID_ACLON : BID_ACLOFF;
		memDC.BitBlt(33, y + 3, m_const[sid].size.cx, m_const[sid].size.cy, &m_const[sid].dc, 0, 0, SRCCOPY);
		sid = (m_style.nLamp & 4) ? BID_ACLON : BID_ACLOFF;
		memDC.BitBlt(58, y + 3, m_const[sid].size.cx, m_const[sid].size.cy, &m_const[sid].dc, 0, 0, SRCCOPY);
		y += m_const[id].size.cy;

		id = m_style.wheel == CARFORM_WHEELD ? BID_WHEEL : BID_WHEELU; 
		memDC.BitBlt(0, y, m_const[id].size.cx, m_const[id].size.cy, &m_const[id].dc, 0, 0, SRCCOPY);
		y += (m_const[id].size.cy + 2 + SIZE_ROLLING);

		int x = 16;
		for (int n = 0; n < 4; n ++) {
			switch (m_style.door[n]) {
			case CARFORM_DOOROP :	id = BID_DOORO;	break;
			case CARFORM_DOORBP :	id = BID_DOORB;	break;
			default :				id = BID_DOORC;	break;
			}
			memDC.BitBlt(n * 21 + x, y, m_const[id].size.cx, m_const[id].size.cy, &m_const[id].dc, 0, 0, SRCCOPY);
			m_rectPart[AREA_DOOR + n] = CRect(n * 21 + x, y, n * 21 + x + m_const[id].size.cx, y + m_const[id].size.cy);
		}
		y += m_const[id].size.cy;

		if (m_style.side != CARFORM_SIDEX) {
			id = m_style.side == CARFORM_SIDEFL ? BID_SIDEF : BID_SIDEN;
			memDC.BitBlt(1, y - 4, m_const[id].size.cx, m_const[id].size.cy, &m_const[id].dc, 0, 0, SRCCOPY);
			m_rectPart[AREA_SIDE] = CRect(1, y - 9, 1 + m_const[id].size.cx, y - 9 + m_const[id].size.cy);
		}

		id = m_style.bypass == CARFORM_BPON ? BID_BPON : BID_BPNR;
		memDC.BitBlt(16, y, m_const[id].size.cx, m_const[id].size.cy, &m_const[id].dc, 0, 0, SRCCOPY);
		m_rectPart[AREA_BYPASS] = CRect(16, y, 16 + m_const[id].size.cx, y + m_const[id].size.cy);

		id = m_style.ihandle == CARFORM_HANDON ? BID_IHDON : BID_IHDNR;
		memDC.BitBlt(58, y, m_const[id].size.cx, m_const[id].size.cy, &m_const[id].dc, 0, 0, SRCCOPY);
		m_rectPart[AREA_IHANDLE] = CRect(58, y, 58 + m_const[id].size.cx, y + m_const[id].size.cy);

		id = m_style.ehandle == CARFORM_HANDON ? BID_EHDON : BID_EHDNR;
		memDC.BitBlt(78, y, m_const[id].size.cx, m_const[id].size.cy, &m_const[id].dc, 0, 0, SRCCOPY);
		m_rectPart[AREA_EHANDLE] = CRect(78, y, 78 + m_const[id].size.cx, y + m_const[id].size.cy);

		//if (m_style.side[1] != CARFORM_SIDEX) {
		//	id = m_style.side[1] == CARFORM_SIDEFL ? BID_SIDEF : BID_SIDEN;
		//	memDC.BitBlt(99, y - 4, m_const[id].size.cx, m_const[id].size.cy, &m_const[id].dc, 0, 0, SRCCOPY);
		//	m_rectPart[AREA_SIDE + 1] = CRect(99, y - 9, 99 + m_const[id].size.cx, y - 9 + m_const[id].size.cy);
		//}
		y += m_const[AREA_EHANDLE].size.cy - 9;

		for (int n = 0, x = 16; n < 4; n ++) {
			switch (m_style.door[n + 4]) {
			case CARFORM_DOOROP :	id = BID_DOORO;	break;
			case CARFORM_DOORBP :	id = BID_DOORB;	break;
			default :				id = BID_DOORC;	break;
			}
			memDC.BitBlt(n * 21 + x, y, m_const[id].size.cx, m_const[id].size.cy, &m_const[id].dc, 0, 0, SRCCOPY);
			m_rectPart[AREA_DOOR + n + 4] = CRect(n * 21 + x, y, n * 21 + x + m_const[id].size.cx, y + m_const[id].size.cy);
		}
		dc.BitBlt(0, 0, m_rect.Width(), m_rect.Height(), &memDC, 0, 0, SRCCOPY);
	}
	memDC.SelectObject(pOldFont);
	memDC.SelectObject(pOldBitmap);
	memDC.DeleteDC();
	memBitmap.DeleteObject();
}

void CBody::OnSize(UINT nType, int cx, int cy)
{
	CWnd::OnSize(nType, cx, cy);
}

void CBody::OnLButtonDown(UINT nFlags, CPoint point)
{
	int n = 0;
	for ( ; n < AREA_MAX; n ++) {
		if (m_bEnableClick && m_rectPart[n].PtInRect(point)) {
			if (m_pParent != NULL)	m_pParent->SendMessage(WM_BODYCLICK, (WPARAM)MAKELONG(n, 0));
			break;
		}
	}
	if (n >= AREA_MAX)	CWnd::OnLButtonDown(nFlags, point);
}

void CBody::OnRButtonDown(UINT nFlags, CPoint point)
{
	int n = 0;
	for ( ; n < AREA_MAX; n ++) {
		if (m_bEnableClick && m_rectPart[n].PtInRect(point)) {
			if (m_pParent != NULL)	m_pParent->SendMessage(WM_BODYCLICK, (WPARAM)MAKELONG(n, 1));
			break;
		}
	}
	if (n >= AREA_MAX)	CWnd::OnRButtonDown(nFlags, point);
}

void CBody::OnMouseMove(UINT nFlags, CPoint point)
{
	int n = 0;
	for ( ; n < AREA_MAX; n ++) {
		if (m_bEnableClick && m_rectPart[n].PtInRect(point)) {
			SetCursor(AfxGetApp()->LoadStandardCursor(IDC_HAND));
			break;
		}
	}
	if (n >= AREA_MAX) {
		SetCursor(AfxGetApp()->LoadStandardCursor(IDC_ARROW));
		CWnd::OnMouseMove(nFlags, point);
	}
}
