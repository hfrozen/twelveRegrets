#pragma once

#define	GETTROUBLEITEM(p)	CTroubleItem* p = CTroubleItem::GetInstance()

class CTroubleItem
{
public:
	CTroubleItem();
	~CTroubleItem();

private:
	bool	m_bInit;
	typedef struct _tagTROUBLEITEMS {
		CString	strText;
		CString	strOccurence;
		CString	strDestroy;
	} TROUBLEITEMS;
#define	SIZE_TROUBLEITEMS	2000
	TROUBLEITEMS	m_troubleItems[SIZE_TROUBLEITEMS];

	void Log(CString str);
	bool MakeTroubleItem(CString strPath);

public:
	void Initial();
	CString	GetTroubleContext(int code);
	bool	GetInitialState()			{ return m_bInit; }

	static CTroubleItem*	GetInstance()	{ static CTroubleItem troubleItem; return &troubleItem; }
};


