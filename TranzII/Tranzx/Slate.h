#pragma once

#include "Xleza.h"

//class CFindTroubles;
#include "FindTroubles.h"

class CSlate : public CWnd
{
	DECLARE_DYNAMIC(CSlate)

public:
	CSlate();
	virtual ~CSlate();

protected:
	CXlezA*		m_pXel;
	CFindTroubles*	m_pFindTrouble;

	int64_t		m_uTrouble;

	bool		m_bMakedTroubleTextList;
	typedef struct _tagTROUBLETEXTLIST {
		int		nCode;
		CString	strText;
	} TROUBLETEXTLIST;
#define	SIZE_TROUBLETEXTLIST	2000
	TROUBLETEXTLIST	m_troubleTextList[SIZE_TROUBLETEXTLIST];

	enum  { INSPTYPE_NON, INSPTYPE_BIT, INSPTYPE_VALUE, INSPTYPE_FORM, INSPTYPE_MAX };

	typedef struct	_tagINSPSAMPLE {
		BYTE	nType;
		WORD	wOffset;
		BYTE	nIndex[3];
		BYTE	nIndexLength;
		double	factor;
	} INSPF, *PINSPF;

#define	SIZE_STICK		15
	static const BSTR	m_lpStick[SIZE_STICK];

#define	SIZE_INSPMODE	7
	static const BSTR	m_lpInspMode[SIZE_INSPMODE];

#define	SIZE_INSPOPER	8
	static const BSTR	m_lpInspOper[SIZE_INSPOPER];

	void	Log(CString str);
	bool	PickupWord(BYTE* p, WORD offset, WORD& value);
	bool	ParseInspType(CString str, INSPF& insp);
	void	InspectItem(CString str, int col, int row, BYTE* p);
	void	MakeTroubleTextList();
	CString	GetTroubleText(int code);

public:
	bool	OpenFile(CString strPath, UINT nType, bool bLog = true);

	DECLARE_MESSAGE_MAP()
};


