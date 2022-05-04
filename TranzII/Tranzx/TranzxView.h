// TranzxView.h
#pragma once

#include "TranzxDoc.h"

#include "FactPanel.h"
#include "TickPanel.h"
#include "Hole.h"
#include "FindTroubles.h"

#define	MAX_X			21600	// 3 * 7200 = 3 hours

#define	MAX_Y			297		// mm A3 landscape
#define	SPACE_TOP		10		// mm
#define	SPACE_BOTTOM	8		// mm

#define	SIZE_SCROLLBAR	16
#define	SIZE_BUTTON		14

#define	ITEM_NAME_X		150
#define	ITEM_NAME_XC	28
#define	ITEM_TIME_Y		20
#define	ITEM_ANALOG_Y	84		//104
#define	ITEM_DIGITAL_Y	14

//class CFindAlarm;

class CTranzxView : public CView
{
protected:
	CTranzxView();
	DECLARE_DYNCREATE(CTranzxView)

public:
	CTranzxDoc* GetDocument() const;

public:
	virtual void OnDraw(CDC* pDC);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
protected:
	virtual void OnInitialUpdate();
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

public:
	virtual ~CTranzxView();
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	CPoint		m_ppi;
	TCHAR		m_nType;

	int			m_nGrid;
	CSize		m_sizeTotal;
	CRect		m_rectFack;
	CRect		m_rectTick;
	CRect		m_rectHole;
	CFactPanel*	m_pFactPanel;
	CTickPanel*	m_pTickPanel;
	CHole*		m_pHole;
	CScrollBar*	m_pHorz;
	CScrollBar*	m_pVert;
	CFont		m_fontPage;
	CPoint		m_ptCur;
	COLORREF	m_crDefault;

	//CFont		m_font;
	//COLORREF	m_crBkgnd;
	//COLORREF	m_crText;
	//CBrush		m_brBkgnd;
	//CBrush		m_brText;
	//CString		m_strTick;
	//CString		m_strFact;

	//===== Trouble
	CListCtrl*	m_pListTrouble;
	CFont		m_fontTroubleList;
	UINT		m_nTroubleType;

	typedef struct _tagLISTFORM {
		BSTR	pHeadName;
		BSTR	pWidth;
	} LISTFORM;
	static const LISTFORM	m_listForm[10];

	//===== Inspect
	enum  { INSPTYPE_NON, INSPTYPE_BIT, INSPTYPE_VALUE, INSPTYPE_FORM, INSPTYPE_MAX };

	typedef struct	_tagINSPSAMPLE {
		BYTE	nType;
		WORD	wOffset;
		BYTE	nIndex[3];
		BYTE	nIndexLength;
		double	factor;
	} INSPF, *PINSPF;

#define	SIZE_STICK		15
	static const BSTR	m_pStick[SIZE_STICK];

#define	SIZE_TROUBLE	8
	static const BSTR	m_pTroubleType[SIZE_TROUBLE];

#define	SIZE_INSPMODE	7
	static const BSTR	m_pInspMode[SIZE_INSPMODE];

#define	SIZE_INSPOPER	8
	static const BSTR	m_pInspOper[SIZE_INSPOPER];

	//bool	PickupWord(BYTE* p, WORD offset, WORD& value);
	//bool	ParseInspType(CString str, INSPF& insp);
	//void	InspectItem(CString str, int col, int row, BYTE* p);
	void	MakeHistory();
	void	BuildByHistory();
	bool	ParseHistory(int& id, COLORREF& color, CString& strName, CString& str);
	void	GetDrawRect(CRect& rect);
	void	Redraw();
	void	SetScrollBar(CSize size);
	bool	SetGridTime();
	void	Reposition(CRect rect);
	void	Draw();
	//CSize	GetFullFrameSize();

	void	Enumerate();
	void	Arrange(UINT troubleType);

public:
	void	Log(CString str, bool bPos = false);
	//void	FrameLog(CString str);
	//CString	GetFrameLog();
	int		FindTroubleCode(UINT nFindCode, int nRow);
	void	SetGrid(int grid);
	void	FindLogbookTime(CTick time);
	//int		GetEnableScale();
	TCHAR	GetType()			{ return m_nType; }
	UINT	GetTroubleType()	{ return m_nTroubleType; }
	//int		GetScale();
	//void	SetScale(int scale);

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnFilePrintPreview();
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg LRESULT	OnSelItem(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnDropItem(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnPartSelect(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSelectTrouble(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
	DECLARE_MESSAGE_MAP()
};

#ifndef _DEBUG  // TranzxView.cpp의 디버그 버전
inline CTranzxDoc* CTranzxView::GetDocument() const
   { return reinterpret_cast<CTranzxDoc*>(m_pDocument); }
#endif

