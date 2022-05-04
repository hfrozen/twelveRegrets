#pragma once

#include "AidStatic.h"

class CAidTexts : public CWnd
{
	DECLARE_DYNAMIC(CAidTexts)

public:
	CAidTexts();
	virtual ~CAidTexts();

protected:
	CWnd*	m_pParent;
	int		m_nLength;

#define	MAX_TEXTS	256
	CAidStatic*	m_pTexts[MAX_TEXTS];

public:
	void	SetText(BYTE* pBuf, int nLength);
	virtual BOOL Create(int nLength, const RECT& rect, CWnd* pParentWnd);

protected:
	DECLARE_MESSAGE_MAP()
};


