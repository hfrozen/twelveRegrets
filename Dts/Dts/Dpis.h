#pragma once

#include "AidLed.h"
#include "AidTexts.h"

#include "../../../Qnx2/Inform2/DevInfo2/PisInfo.h"

class CDpis : public CDialogEx
{
	DECLARE_DYNAMIC(CDpis)

public:
	CDpis(CWnd* pParent = NULL);
	virtual ~CDpis();

	enum { IDD = IDD_DIALOG_DPIS };

protected:
	CWnd*	m_pParent;
	int		m_nTID;
	int		m_nGate;

	bool	m_bManual;
	BYTE	m_cCtrl;
	CStatic	m_sttCtrlCode;
	CAidTexts	m_mons[MONCOM_MAX];
	CAidLed*	m_pLamp;

	void	DisableLine();
	void	BitCtrl(UINT id, int nState);

public:
	void	Initial();
	void	SetTitle(CString strCarName, CString strDevName);
	void	SetDevName(CString strDevName);
	void	SetCarName(CString strCarName)		{ SetWindowTextW(strCarName); }
	void	Ready(bool bCmd = true);
	void	SetupGate(bool bCmd, int nTID, int nGate);
	void	CollectBuild();
	void	RecvSdr(BYTE* p, int length);
	void	SetParent(CWnd* pParent)		{ m_pParent = pParent; }

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy();
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
