#pragma once

#include "AidLed.h"
#include "AidTexts.h"

#include "../../../Qnx2/Inform2/DevInfo2/EcuInfo.h"

class CDecu : public CDialogEx
{
	DECLARE_DYNAMIC(CDecu)

public:
	CDecu(CWnd* pParent = NULL);
	virtual ~CDecu();

	enum { IDD = IDD_DIALOG_DECU };

protected:
	CWnd*	m_pParent;
	CAidTexts	m_mons[MONCOM_MAX];
	int		m_nCID;
	int		m_nGate;
	int		m_nTestStep;
	int		m_nTestTime;

	CAidLed*	m_pLampDc;
	ECURINFO	m_sr[2];

	static const double	m_dbAsp[10];
	static const WORD	m_wLw[10];

	void	DisableLine();
	bool	GetButton(int nButtonID);
	void	ButtonCheck(int nButtonnID, bool bCheck);
	void	EditWrite(int nEditID, int nNum);

public:
	void	SetTitle(CString strCarName, CString strDevName);
	void	SetDevName(CString strDevName);
	void	SetCarName(CString strCarName)		{ SetWindowTextW(strCarName); }
	void	Ready(bool bCmd = true);
	void	SetupGate(bool bCmd, int nCID, int nGate);
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
