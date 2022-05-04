#include "stdafx.h"
#include "Dts.h"
#include "AidLed.h"

#define	LED_TIMER	2

IMPLEMENT_DYNAMIC(CAidLed, CStatic)

CAidLed::CAidLed()
{
	m_bitmapLed.LoadBitmap(IDB_BITMAP_LED);
	m_pParent = NULL;
	m_nID = 0;
	m_nColor = COLOR_RED;
	m_nState = STATE_OFF;
	m_bFlash = false;
}

CAidLed::CAidLed(int nID, CWnd* pParent)
{
	m_bitmapLed.LoadBitmap(IDB_BITMAP_LED);
	m_pParent = pParent;
	m_nID = nID;
	m_nColor = COLOR_RED;
	m_nState = STATE_OFF;
	m_bFlash = false;
}

CAidLed::~CAidLed()
{
	VERIFY(m_bitmapLed.DeleteObject());
}

void CAidLed::DrawLed(CDC* pDC, int nColor, int nState)
{
	CRect rect, rc;
	GetClientRect(&rect);
	rc = rect;

	if (rect.Width() > SIZE_LED || rect.Height() > SIZE_LED) {
		int nWidth = rect.Width();
		int nHeight = rect.Height();
		rect.left += (nWidth - SIZE_LED) / 2;
		rect.right = rect.left + SIZE_LED;		//(nWidth - SIZE_LED) / 2;
		rect.top += (nHeight - SIZE_LED) / 2;
		rect.bottom = rect.top + SIZE_LED;		//(nHeight - SIZE_LED) / 2;
	}

	CBitmap transBitmap;
	transBitmap.CreateBitmap(SIZE_LED, SIZE_LED, 0, 0, NULL);
	CBitmap bitmapTemp;
	CBitmap* pBitmap = &m_bitmapLed;

	CDC srcDC;
	CDC dcMask;
	CDC tempDC;
	srcDC.CreateCompatibleDC(pDC);
	dcMask.CreateCompatibleDC(pDC);
	tempDC.CreateCompatibleDC(pDC);

	CBitmap* pOldBitmap = srcDC.SelectObject(pBitmap);
	CBitmap* pOldMaskbitmap = dcMask.SelectObject(&transBitmap);
	bitmapTemp.CreateCompatibleBitmap(pDC, SIZE_LED, SIZE_LED);

	CBitmap* pOldBitmapTemp = tempDC.SelectObject(&bitmapTemp);
	tempDC.BitBlt(0, 0, SIZE_LED, SIZE_LED, pDC, rect.left, rect.top, SRCCOPY);

	COLORREF oldBkColor = srcDC.SetBkColor(RGB(255, 0, 255));
	//dcMask.BitBlt(0, 0, SIZE_LED, SIZE_LED, &srcDC, nMode * SIZE_LED, nColor/* + nShape*/, SRCCOPY);
	int type;
	switch (nState) {
	case STATE_DISABLE :	type = 7;	break;
	case STATE_OFF :		type = 6;	break;
	default :				type = nColor;	break;
	}
	dcMask.BitBlt(0, 0, SIZE_LED, SIZE_LED, &srcDC, type * SIZE_LED, 0, SRCCOPY);
	tempDC.SetBkColor(oldBkColor);

	tempDC.BitBlt(0, 0, SIZE_LED, SIZE_LED, &srcDC, type * SIZE_LED, 0, SRCINVERT);
	tempDC.BitBlt(0, 0, SIZE_LED, SIZE_LED, &dcMask, 0, 0, SRCAND);
	tempDC.BitBlt(0, 0, SIZE_LED, SIZE_LED, &srcDC, type * SIZE_LED, 0, SRCINVERT);

	pDC->FillSolidRect(&rc, RGB(255, 255, 255));
	pDC->BitBlt(rect.left, rect.top, SIZE_LED, SIZE_LED, &tempDC, 0, 0, SRCCOPY);

	srcDC.SelectObject(pOldBitmap);
	dcMask.SelectObject(pOldMaskbitmap);
	tempDC.SelectObject(pOldBitmapTemp);

	VERIFY(srcDC.DeleteDC());
	VERIFY(dcMask.DeleteDC());
	VERIFY(tempDC.DeleteDC());
	VERIFY(transBitmap.DeleteObject());
	VERIFY(bitmapTemp.DeleteObject());
}

void CAidLed::Set(int nColor, int nState)
{
	m_nColor = nColor;
	m_nState = nState;

	CDC* pDC;
	pDC = GetDC();
	ASSERT(pDC);
	DrawLed(pDC, m_nColor, m_nState);
	ReleaseDC(pDC);
}

void CAidLed::SetState(int nState)
{
	Set(m_nColor, nState);
}

void CAidLed::SetColor(int nColor)
{
	Set(nColor, m_nState);
}

/*void CAidLed::Set(int state)	// -1:disable, 0:off, 1:on, 2:flash
{
	int mode;
	if (state < 2) {	// static
		if (m_bFlash) {
			m_bFlash = false;
			KillTimer(LED_TIMER);
		}
		if (state < 0)	mode = 2;		// disable
		else if (state > 0)	mode = 0;	// on
		else	mode = 1;				// off
	}
	else {	// flash
		mode = 0;	// on
		if (!m_bFlash) {
			m_bFlash = true;
			SetTimer(LED_TIMER, 500, NULL);
		}
	}
	SetLed(m_nLedColor, mode);	//, m_nLedShape);
}
*/

BEGIN_MESSAGE_MAP(CAidLed, CStatic)
	ON_WM_DESTROY()
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_LBUTTONUP()
END_MESSAGE_MAP()

void CAidLed::OnDestroy()
{
	CWnd::OnDestroy();
	KillTimer(LED_TIMER);
}

void CAidLed::OnPaint() 
{
	CPaintDC dc(this);
	DrawLed(&dc, m_nColor, m_nState);
}

void CAidLed::OnTimer(UINT nIDEvent)
{
	if (nIDEvent == LED_TIMER) {
		//m_nLedMode = !m_nLedMode ? 1 : 0;
		//SetLed(m_nLedColor, m_nLedMode);	//, m_nLedShape);
	}
	CStatic::OnTimer(nIDEvent);
}

void CAidLed::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_pParent != NULL && m_nState >= STATE_OFF)	m_pParent->SendMessage(WM_CLICKLED, (WPARAM)MAKELONG((WORD)m_nID, (WORD)m_nState));
	CStatic::OnLButtonUp(nFlags, point);
}
