// Body.h
#pragma once

#define	SIZE_ROLLING	6

class CBody : public CWnd
{
	DECLARE_DYNAMIC(CBody)

public:
	CBody();
	virtual ~CBody();

	enum PART_AREA {
		AREA_PANTO = 0,		// has 2 area
		AREA_DOOR = 2,		// has 8 area
		AREA_SIDE = 10,		// has 1 area
		AREA_BYPASS,
		AREA_EHANDLE,
		AREA_IHANDLE,
		AREA_MAX
	};

protected:
	CWnd*	m_pParent;
	CRect	m_rect;
	CRect	m_rectPart[AREA_MAX];
	COLORREF m_crBkgnd;
	CBrush	m_brBkgnd;
	CFont	m_font;

	bool	m_bEnableClick;
	BODYSTYLE	m_style;

	enum BITMAP_ID {
		BID_PANTODN,
		BID_PANTOUP,
		BID_PANTOBP,
		BID_COACHN,
		BID_COACHL,
		BID_COACHR,
		BID_WHEELU,
		BID_WHEEL,
		BID_DOORC,
		BID_DOORO,
		BID_DOORB,
		BID_SIDEN,
		BID_SIDEF,
		BID_BPNR,
		BID_BPON,
		BID_IHDNR,
		BID_IHDON,
		BID_EHDNR,
		BID_EHDON,
		BID_ACLOFF,
		BID_ACLON,
		BID_DCLOFF,
		BID_DCLON,
		BID_CORRI,
		BID_MAX
	};

	typedef struct _tagTPART {
		CSize	size;
		CDC		dc;
		CBitmap	bm;
		CBitmap*	pBm;
	} TPART;

	CDC		m_dcBase;
	TPART	m_const[BID_MAX];

public:
	void	SetStyle(BODYSTYLE style);
	void	SetNumber(int nID, bool bRedraw = false);
	void	SetPanto(CARFORM_PANTO pan[2], bool bRedraw = false);
	void	SetCoach(CARFORM_COACH coach, bool bRedraw = false);
	void	SetWheel(CARFORM_WHEEL wheel, bool bRedraw = false);
	void	SetDoor(CARFORM_DOOR door[8], bool bRedraw = false);
	void	SetSide(CARFORM_SIDE side, bool bRedraw = false);
	void	SetBypass(CARFORM_BP bp, bool bRedraw = false);
	void	SetIHandle(CARFORM_HAND hand, bool bRedraw = false);
	void	SetEHandle(CARFORM_HAND hand, bool bRedraw = false);
	void	SetLamp(BYTE nLamp, bool bRedraw = false);
	void	SetLampA(BYTE nLamp, bool bCtrl, bool bRedraw = false);
	void	Ready(bool bCmd = true)		{ m_bEnableClick = bCmd; }
	void	SetType(CARTYPE type)		{ m_style.nType = type; }
	PBODYSTYLE	GetStyle()				{ return &m_style; }
	BYTE	GetLamp()					{ return m_style.nLamp; }
	void	SetParent(CWnd* pParent)	{ m_pParent = pParent; }
	CRect	GetRect()					{ return m_rect; }

	virtual BOOL Create(DWORD dwStyle, const RECT& rect, CWnd* pParentWnd, UINT nID = NULL);

protected:
	DECLARE_MESSAGE_MAP()
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
};
