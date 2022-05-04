#pragma once

#include "AidLed.h"

#define	MAX_LEDS		64

class CLeds : public CDialogEx
{
	DECLARE_DYNAMIC(CLeds)

public:
	CLeds(CWnd* pParent = NULL);
	virtual ~CLeds();

	enum { IDD = IDD_DIALOG_LEDI };

protected:
	CWnd*		m_pParent;
	UINT		m_nCID;
	CAidLed*	m_pLed[MAX_LEDS];

public:
	void	UnitClr();
	void	BitCtrl(UINT bid, bool bCtrl);
	bool	GetBit(UINT bid);
	void	SetName(CString strName);
	void	SetCID(UINT nCID)		{ m_nCID = nCID; }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg LRESULT OnClickLed(WPARAM wParam, LPARAM lParam);
};
