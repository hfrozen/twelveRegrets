// TranzxDoc.h :
#pragma once

enum enFILETYPE {
	FILETYPE_NON			= TCHAR('0'),
	FILETYPE_TROUBLE		= TCHAR('t'),
	//FILETYPE_ARMLOG,
	//FILETYPE_LOG,
	FILETYPE_LOGBOOK		= TCHAR('l'),
	FILETYPE_INSPECTPDT		= TCHAR('p'),
	FILETYPE_INSPECTDAILY	= TCHAR('i'),
	FILETYPE_INSPECTMONTHLY	= TCHAR('m'),
	FILETYPE_MAX
};

//enum enOPERITEM {
//	OPERITEM_HIERARCHY,
//	OEPRITEM_TYPE,
//	OPERITEM_NAME,
//	OPERITEM_FLAG,
//	OPERITEM_OFFSET,
//	OPERITEM_REF,
//	OPERITEM_MAX
//};
//
//#define	WM_OPENFILE		WM_USER + 22
#define	WM_SELITEM		WM_USER + 24
#define	WM_DROPITEM		WM_USER + 26
#define	WM_PARTSELECT	WM_USER + 28
#define	WM_PARTDELETE	WM_USER + 30
#define	WM_ITEMDELETE	WM_USER + 32

#pragma pack(push, 1)

typedef union	_tagDUET {
	BYTE	c[2];
	WORD	w;
} _DUET;

typedef union	_tagQUARTET {
	BYTE	c[4];
	WORD	w[2];
	DWORD	dw;
} _QUARTET;

typedef union	_tagOCTET {
	BYTE	c[8];
	WORD	w[4];
	DWORD	dw[2];
	QWORD	qw;
} _OCTET;

typedef union _tagDBFTIME {
	struct {
		DWORD	year	: 6;	// 0~63 -> 2010~2073
		DWORD	mon		: 4;
		DWORD	day		: 5;
		DWORD	hour	: 5;
		DWORD	min		: 6;
		DWORD	sec		: 6;
	} t;						// 2/4
	DWORD	dw;
	int32_t	i;
} DBFTIME, *PDBFTIME;

typedef union _tagTROUBLEARCHIVE {
	struct {
		QWORD	down	:  1;	// 지워진..
		QWORD	alarm	:  1;	// type중에서 사용자가 참조하는 고장일 때 1이 된다. 코더의 고장은 감춘다.
		QWORD	type	:  4;	// see ALARMTYPE enum, 없앨 수 있지 않을까?
		QWORD	cid		:  4;	// 0~9, 객차 번호
		QWORD	code	: 11;	// 고장 코드, 0x7ff-2047
		QWORD	mon		:  4;	// 0~11, 발생한 달(1~12)
		QWORD	day		:  5;	// 0~30, ...    날(1~31)
		QWORD	uhour	:  5;	// 0~23, ...    시(0~23)
		QWORD	umin	:  6;	// 0~59, ...    분(0~59)
		QWORD	usec	:  6;	// 0~59, ...    초(0~59)
		QWORD	dhour	:  5;	// 0~23, 소멸한 시(0~23)
		QWORD	dmin	:  6;	// 0~59, ...    분(0~59)
		QWORD	dsec	:  6;	// 0~59, ...    초(0~59)
	} b;
	QWORD	a;
} TROUBLEARCH, *PTROUBLEARCH;

enum enTROUBLETYPE {
	TROUBLE_NON = 0,
	TROUBLE_TEXT,		// 001
	TROUBLE_EVENT,		// 010
	TROUBLE_STATUS,		// 011
	TROUBLE_WARNING,	// 100
	TROUBLE_LIGHT,		// 101
	TROUBLE_SEVERE,		// 110
	TROUBLE_EMERGENCY	// 111
};
#define	TROUBLE_ALL		0xf

typedef struct _tagDEVTIME {
	BYTE	year;		// 0, 0~99
	BYTE	mon;		// 1, 1~12
	BYTE	day;		// 2, 1~31
	BYTE	hour;		// 3, 0~23
	BYTE	min;		// 4, 0~59
	BYTE	sec;		// 5, 0~59
} DEVTIME, *PDEVTIME;

typedef struct _tagLOGBOOKHEAD {
	BYTE	title[12];		// 0:11
	WORD	wVer;			// 12:13
	WORD	wPermNo;		// 14:15
	BYTE	cLength;		// 16:0
	BYTE	cRate;			// 17:0
	DEVTIME	devt;			// 18:23
	BYTE	cAux;			// 24:		// 보조 TC
	BYTE	cDummy[7];		// 25:31
} LOGBOOKHEAD;

#pragma pack(pop)

#define	SIZE_TROUBLEBLOCK	sizeof(TROUBLEARCH)
#define	SIZE_LOGBOOKBLOCK	504
#define	SIZE_LOGBOOKHEAD	sizeof(LOGBOOKHEAD)
#define	SIZE_INSPECTFILE	768

#define MAKEQWORD(p)	((QWORD)*((BYTE*)p) | \
						((QWORD)*((BYTE*)(p + 1)) << 8) | \
						((QWORD)*((BYTE*)(p + 2)) << 16) | \
						((QWORD)*((BYTE*)(p + 3)) << 24) | \
						((QWORD)*((BYTE*)(p + 4)) << 32) | \
						((QWORD)*((BYTE*)(p + 5)) << 40) | \
						((QWORD)*((BYTE*)(p + 6)) << 48) | \
						((QWORD)*((BYTE*)(p + 7)) << 56))

class CTick
{
public:
	int		m_nYear;
	int		m_nMon;
	int		m_nDay;
	int		m_nHour;
	int		m_nMin;
	int		m_nSec;

	CTick::CTick()
	{
		m_nYear = m_nMon = m_nDay = m_nHour = m_nMin = m_nSec = 0;
	}

	DWORD	DailyToSec()
	{
		return (m_nHour * 3600 + m_nMin * 60 + m_nSec);
	}

	CString	Format()
	{
		CString str;
		str.Format(L"%04d/%02d/%02d %02d:%02d:%02d", m_nYear, m_nMon, m_nDay, m_nHour, m_nMin, m_nSec);
		return str;
	}

	CString	FomatD()
	{
		CString str;
		str.Format(L"%04d/%02d/%02d", m_nYear, m_nMon, m_nDay);
		return str;
	}

	CString	FormatT()
	{
		CString str;
		str.Format(L"%02d:%02d:%02d", m_nHour, m_nMin, m_nSec);
		return str;
	}

	DBFTIME FormatDBF()
	{
		DBFTIME dbt;
		dbt.t.year = (BYTE)(m_nYear - 2010);
		dbt.t.mon = (BYTE)m_nMon;
		dbt.t.day = (BYTE)m_nDay;
		dbt.t.hour = (BYTE)m_nHour;
		dbt.t.min = (BYTE)m_nMin;
		dbt.t.sec = (BYTE)m_nSec;
		return dbt;
	}

	void	Init()
	{
		m_nYear = m_nMon = m_nDay = m_nHour = m_nMin = m_nSec = 0;
	}

	void	Set(int nYear, int nMon, int nDay, int nHour, int nMin, int nSec)
	{
		m_nYear = nYear; m_nMon = nMon; m_nDay = nDay; m_nHour = nHour; m_nMin = nMin; m_nSec = nSec;
	}

	void	SetDate(int nYear, int nMon, int nDay)
	{
		m_nYear = nYear; m_nMon = nMon; m_nDay = nDay;
	}

	void	SetTime(int nHour, int nMin, int nSec)
	{
		m_nHour = nHour; m_nMin = nMin; m_nSec = nSec;
	}

	CTick& operator = (const CTick& tick)
	{
		m_nYear = tick.m_nYear; m_nMon = tick.m_nMon; m_nDay = tick.m_nDay; m_nHour = tick.m_nHour;
		m_nMin = tick.m_nMin; m_nSec = tick.m_nSec;
		return *this;
	}

	CTick& operator ++ ()
	{
		if (++ m_nSec >= 60) {
			m_nSec = 0;
			if (++ m_nMin >= 60) {
				m_nMin = 0;
				if (++ m_nHour >= 24) {
					m_nHour = 0; ++ m_nDay;
				}
			}
		}
		return *this;
	}

	int Differ (CTick dif)
	{
		int curs = m_nDay * 86400 + m_nHour * 3600 + m_nMin * 60 + m_nSec;
		int difs = dif.m_nDay * 86400 + dif.m_nHour * 3600 + dif.m_nMin * 60 + dif.m_nSec;
		return curs - difs;
	}

	bool operator > (const CTick& tick) const
	{
		if (m_nYear > tick.m_nYear)	return true;
		if (m_nYear < tick.m_nYear)	return false;
		if (m_nMon > tick.m_nMon)	return true;
		if (m_nMon < tick.m_nMon)	return false;
		if (m_nDay > tick.m_nDay)	return true;
		if (m_nDay < tick.m_nDay)	return false;
		if (m_nHour > tick.m_nHour)	return true;
		if (m_nHour < tick.m_nHour)	return false;
		if (m_nMin > tick.m_nMin)	return true;
		if (m_nMin < tick.m_nMin)	return false;
		if (m_nSec > tick.m_nSec)	return true;
		return false;
	}

	bool operator < (const CTick& tick) const
	{
		if (m_nYear < tick.m_nYear)	return true;
		if (m_nYear > tick.m_nYear)	return false;
		if (m_nMon < tick.m_nMon)	return true;
		if (m_nMon > tick.m_nMon)	return false;
		if (m_nDay < tick.m_nDay)	return true;
		if (m_nDay > tick.m_nDay)	return false;
		if (m_nHour < tick.m_nHour)	return true;
		if (m_nHour > tick.m_nHour)	return false;
		if (m_nMin < tick.m_nMin)	return true;
		if (m_nMin > tick.m_nMin)	return false;
		if (m_nSec < tick.m_nSec)	return true;
		return false;
	}

	bool operator == (const CTick& tick) const
	{
		return (m_nYear == tick.m_nYear && m_nMon == tick.m_nMon && m_nDay == tick.m_nDay &&
			m_nHour == tick.m_nHour && m_nMin == tick.m_nMin && m_nSec == tick.m_nSec);
	}

	bool operator != (const CTick& tick) const
	{
		return !operator == (tick);
	}

	//void	Increase(int sec)
	//{
	//	m_nSec += sec;
	//	if (m_nSec >= 60) {
	//		m_nSec -= 60;
	//		if (++ m_nMin >= 60) {
	//			m_nMin = 0;
	//			if (++ m_nHour >= 24) {
	//				m_nHour = 0;
	//				if (++ m_nDay >= 0)	m_nDay = 0;
	//			}
	//		}
	//	}
	//}
};

class CTranzxDoc : public CDocument
{
protected:
	CTranzxDoc();
	DECLARE_DYNCREATE(CTranzxDoc)

public:
	virtual BOOL OnNewDocument();
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
	virtual void Serialize(CArchive& ar);

public:
	virtual ~CTranzxDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:
	DECLARE_MESSAGE_MAP()

private:
	TCHAR		m_nType;
	CString		m_strPathName;

//===== LOGBOOK
	typedef union _tagLOGBOOKHEADU {
		LOGBOOKHEAD	head;
		BYTE	c[SIZE_LOGBOOKHEAD];
	} LOGBOOKHEADU;

	typedef struct _tagLOGBOOKBLOCK {
		int		nCrevice;
		BYTE	c[SIZE_LOGBOOKBLOCK];
	} LOGBOOKBLOCK, *PLOGBOOKBLOCK;
	CPtrList	m_logbookList;

	LOGBOOKHEADU	m_logHead;
	BYTE*		m_pLogbook;
	uint64_t	m_uLogbookLength;
	int			m_nFailLogBlock;
	UINT		m_uScale;

#define	SIZE_SCALE	4
	typedef struct _tagSCALE {
		uint64_t	uBeginID;
		uint64_t	uEndID;
	} _SCALE;
	_SCALE		m_scale[SIZE_SCALE];

//===== TROUBLE
	CPtrList	m_troubleList;
	uint64_t	m_uTroubleLength;

//===== INSPECT
	BYTE*		m_pInspect;

	void		DeleteLogbookList();
	void		DeleteTroubleList();
	//uint64_t	GetValidBegin();
	bool		GetLogbookTime(BYTE* pBuf, CTick& time);
	void		InsertLogbook(BYTE* pBuf, bool bHalf = false);
	void		ModifyTailLogbook(bool bHalf);
	void		InsertNullLogbook(int diff, BYTE* pBuf);

public:
	typedef struct _tagLOGBOOKITEMARCHIVE {
		BYTE	nHierarchy;
		TCHAR	type;
		CString	strName;
		BYTE	nFlag;
		WORD	wOffset;
		int64_t	pscMin;
		int64_t	pscMax;
		int64_t	logMin;
		int64_t	logMax;
		CString	strUnit;
	} LBITEMARCH, *PLBITEMARCH;
	static const LBITEMARCH	m_lbiArchive[];

	typedef struct _tagRANGE {
		int64_t	pscMin;
		int64_t	pscMax;
		int64_t	logMin;
		int64_t	logMax;
	} RANGE, *PRANGE;

	PRANGE		m_pRange;
	int			m_nRange;
	//static const int		m_nFactor[SIZE_SCALE];
	//static const CString	m_strFactor[SIZE_SCALE];

	//void		DeleteLog();
	void		Log(CString str);
	int			CountSetBit(BYTE ch);
	int			CountShiftToB0(BYTE ch);
	bool		LoadFile(CString strPath, bool bLog = true);
	bool		MakeTroubleCodes(CString strPath, bool bLog = true);
	uint64_t	MergeTrouble(BYTE* pSrc, uint64_t uLength);
	bool		MergeDownTrouble(TROUBLEARCH tr);
	int			GetLogbookTime(uint64_t uID, CTick& time);
	int64_t		GetLogbookIDByTime(CTick time);
	//int			GetLogbookTimeFromScale(uint64_t uID, CTick& time);
	int			GetLogbookItem(uint64_t uID, int nArchID, _variant_t& var);
	//bool		GetLogbookItemFromScale(uint64_t uID, int nArchID, _variant_t& var);
	uint64_t	GetMaxX()							{ return GetLogbookLength(); }
	//int			GetMaxXbyScale();
	//int			GetMaxXbyScale(int scale);
	uint64_t	GetLogbookLength()					{ return m_uLogbookLength; }
	BYTE		GetLBArchiveHierarchy(int nArchID)	{ return m_lbiArchive[nArchID].nHierarchy; }
	TCHAR		GetLBArchiveType(int nArchID)		{ return m_lbiArchive[nArchID].type; }
	CString		GetLBArchiveName(int nArchID)		{ return m_lbiArchive[nArchID].strName; }
	BYTE		GetLBArchiveFlag(int nArchID)		{ return m_lbiArchive[nArchID].nFlag; }
	WORD		GetLBArchiveOffset(int nArchID)		{ return m_lbiArchive[nArchID].wOffset; }
	LPCTSTR		GetLBArchiveUnit(int nArchID)		{ return m_lbiArchive[nArchID].strUnit; }
	int64_t		GetLBArchivePscOffset(int nArchID);
	int64_t		GetLBArchiveLogOffset(int nArchID);
	int64_t		GetLBArchivePscMin(int nArchID);
	void		SetLBArchivePscMin(int nArchID, int64_t min);
	int64_t		GetLBArchiveLogMin(int nArchID);
	void		SetLBArchiveLogMin(int nArchID, int64_t min);
	int64_t		GetLBArchivePscMax(int nArchID);
	void		SetLBArchivePscMax(int nArchID, int64_t max);
	int64_t		GetLBArchiveLogMax(int nArchID);
	void		SetLBArchiveLogMax(int nArchID, int64_t max);
	int			GetFrameNo();
	CString		FormatTimeRange();
	//void		SetScale(int scale)				{ m_uScale = scale; }
	//int			GetScale()						{ return m_uScale; }
	//int			GetTimeByScale()				{ return m_nFactor[m_uScale]; }
	//CString		GetFactStrByScale()				{ return m_strFactor[m_uScale]; }
	TCHAR		GetType()						{ return m_nType; }

	CPtrList*	GetTroubleList()				{ return &m_troubleList; }
	int64_t		GetTroubleMax()					{ return m_uTroubleLength; }
	BYTE*		GetInspectBuffer()				{ return m_pInspect; }
	CString		GetPathName()					{ return m_strPathName; }
	//void		SetRoot(CString strRootName)	{ m_strRootName = strRootName; }
	//CString		GetRoot()						{ return m_strRootName; }
};
