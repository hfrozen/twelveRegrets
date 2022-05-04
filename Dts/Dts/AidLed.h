#pragma once

#define	LS_RED		CAidLed::LCOLOR_RED
#define	LS_YELLOW	CAidLed::LCOLOR_YELLOW
#define	LS_BLUE		CAidLed::LCOLOR_BLUE
#define	LS_GREEN	CAidLed::LCOLOR_GREEN
#define	LS_ORANGE	CAidLed::LCOLOR_ORANGE
#define	LS_PURPLE	CAidLed::LCOLOR_PURPLE

#define	LS_DISABLE	CAidLed::STATE_DISABLE
#define	LS_OFF		CAidLed::STATE_OFF
#define	LS_ON		CAidLed::STATE_ON
#define	LS_FLASH	CAidLed::STATE_FLASH

#define	SIZE_LED	16

#define	WM_CLICKLED		16

class CAidLed : public CStatic
{
	DECLARE_DYNAMIC(CAidLed)

public:
	CAidLed();
	CAidLed(int nID, CWnd* pParent);
	virtual ~CAidLed();

protected:
	CWnd*	m_pParent;
	int		m_nID;
	int		m_nColor;
	int		m_nState;
	CBitmap	m_bitmapLed;
	bool	m_bFlash;

public:
	enum {
		LCOLOR_RED = 0,	// * SIZE_LED,
		LCOLOR_YELLOW,	// = 1 * SIZE_LED,
		LCOLOR_BLUE,		// = 2 * SIZE_LED,
		LCOLOR_GREEN,	// = 3 * SIZE_LED,
		LCOLOR_ORANGE,	// = 4 * SIZE_LED,
		LCOLOR_PURPLE,	// = 5 * SIZE_LED,
		LCOLOR_GRAY,		// = 6 * SIZE_LED,
		LCOLOR_BLACK,	// = 7 * SIZE_LED
	};
	enum {
		STATE_DISABLE = -1,
		STATE_OFF = 0,
		STATE_ON = 1,
		STATE_FLASH = 2
	};

	void	Set(int nColor, int nState);
	void	SetState(int nState);
	void	SetColor(int nColor);
	int		GetState() { return m_nState; }

	void	DrawLed(CDC* pDC, int nColor, int nState);

protected:
	DECLARE_MESSAGE_MAP()

	afx_msg void OnDestroy();
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};
