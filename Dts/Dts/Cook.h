#pragma once

#pragma comment(lib, "winmm.lib")

#include <MMSystem.h>

class CCook : public CWnd
{
	DECLARE_DYNAMIC(CCook)

public:
	CCook();
	virtual ~CCook();

protected:
	CWnd*		m_pParent;
	bool		m_bTimerReady;
	UINT		m_timerRes;
	MMRESULT	m_timerID;

	WORD		m_wTick;
	WORD		m_wTickLimit;
	bool		m_bHold;
	bool		m_bBreak;

	enum enPARSID {
		PARSID_TIN,
		PARSID_CIN,
		PARSID_OUT,
		PARSID_AIN,
		PARSID_ATO,
		PARSID_PAU,
		PARSID_PIS,
		PARSID_TRS,
		PARSID_SIV,
		PARSID_V3F,
		PARSID_ECU,
		PARSID_HVAC,
		PARSID_DCU,
		PARSID_MAX
	};

	EDIFICE m_edif[PARSID_MAX];

	typedef struct _tagSCRIPT {
		int		nTop;		// from 0
		int		nCur;		// from 0
		int		nTotal;		// ex:0~9 -> 10 line
		int		nWords;
		CString	strText;
		CString	strCur;
		CString	strLine;
		CString	strWords[10];
	} SCRIPT;
	SCRIPT	m_scr;
	bool	m_bCheck;

	enum enKEYWORD {
		KW_END = 0, KW_WAIT,	KW_GOTO,	KW_PAUSE,
		KW_WATCH,	KW_CHK,		KW_STCK,	KW_MSTC,
		KW_TIN,		KW_CIN,		KW_OUT,		KW_AIN,
		KW_ATO,		KW_PAU,		KW_PIS,		KW_TRS,
		KW_SIV,		KW_V3F,		KW_ECU,		KW_HVAC,
		KW_DCU,		KW_MAX
	};

	enum enTAKEOUT {
		TAKEOUT_ERROR = 0,
		TAKEOUT_FAIL,
		TAKEOUT_END,
		TAKEOUT_CONTINUE
	};

#define	LENGTH_TCU		12
	typedef struct _tagTCUINFORM {
		bool	bReady;
		bool	bLink;
		int		ip;
		WORD	wOblige;
		WORD	wSpeed;
		DWORD	dwDistan;
	} TCUINFORM;
	TCUINFORM	m_tcu[LENGTH_TCU];

	CString	m_strFailer;

	//static PCTSTR	m_pstrKeyword[KW_MAX];
	typedef struct _tagKEYWORD {
		int		nID;
		CString	strKey;
	} KEYWORD;
	static const KEYWORD	m_keyword[KW_MAX];

	bool	StartTimer(UINT ms);
	bool	StopTimer();
	void	DeleteEdifice(PEDIFICE pEdifice);
	void	Log(CString strLog, COLORREF crText = COLOR_BLACK);
	void	Logr(CString strLog, COLORREF crText = COLOR_BLACK);
	//CString	ExtractUnit(CString strUnits);
	CString	LoadFile(CString strFile);
	CString	LoadSentence(CString strFile, CString strHead);
	void	RemoveComments(CString& strContext);
	CString	ExtractWord(CString& strContext);
	CString	ExtractSentence(CString& strContext);
	bool	ExtractEdificeIo(CString strFile, CString strHead, PEDIFICE pEdifice);
	void	AnnexEdificeIo(CString strFile, PEDIFICE pEdifice);
	bool	ExtractEdificeSa(CString strFile, CString strHead, PEDIFICE pEdifice);
	void	AnnexEdificeSa(CString strFile, PEDIFICE pEdifice);
	PMODULE	FindEdifice(PEDIFICE pEdif, CString strLinear, CString strQuadratic = L"");

	void	LoadScript(CString strFile);
	bool	PickupLine(int nLine);
	bool	SeparateLine();
	int		LinearKeyword();
	int		FindLabel(CString strLabel);
	bool	CheckArgLength(const int nValid);
	int		ExcerptInt(CString str);
	int		ExcerptCid(CString str);
	void	InitStep();
	UINT	Takeout(bool bReal);

public:
	bool	Initial();
	bool	Revolve();
	bool	Propel(CString strFile);
	void	TcmChange(int nID, int nItem, variant_t var);
	void	Terminate()					{ m_bBreak = true; }
	void	SetParent(CWnd* pParent)	{ m_pParent = pParent; }

	DECLARE_MESSAGE_MAP()
};


