#pragma once

#include "AidStatic.h"

class CDashb : public CDialogEx
{
	DECLARE_DYNAMIC(CDashb)

public:
	CDashb(CWnd* pParent = NULL);
	virtual ~CDashb();

	enum { IDD = IDD_DIALOG_DASHBOARD };

protected:
	CWnd*	m_pParent;
	CFont	m_font[6];

	enum DASHITEM {
		DITEM_DATE = 0,
		DITEM_TIME,
		DITEM_SPEEDN,
		DITEM_SPEED,
		DITEM_DISTN,
		DITEM_DIST,
		DITEM_CURRN,
		DITEM_CURR,
		DITEM_NEXTN,
		DITEM_NEXT,
		DITEM_DESTN,
		DITEM_DEST,
		DITEM_TBEN,
		DITEM_MAX
	};

	CAidStatic	m_item[DITEM_MAX];

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
