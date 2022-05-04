#pragma once

#include "AidTexts.h"

#include "../../../Qnx2/Inform2/DevInfo2/DcuInfo.h"

class CDdcu : public CDialogEx
{
	DECLARE_DYNAMIC(CDdcu)

public:
	CDdcu(CWnd* pParent = NULL);
	virtual ~CDdcu();

	enum { IDD = IDD_DIALOG_DDCU };

protected:
	CWnd*	m_pParent;
	CAidTexts	m_mons[MONCOM_MAX];
	int		m_nCID;
	int		m_nGate;
	bool	m_bSide;

	void	DisableLine();

public:
	void	SetTitle(CString strCarName, CString strDevName);
	void	SetDevName(CString strDevName);
	void	SetCarName(CString strCarName)		{ SetWindowTextW(strCarName); }
	void	Ready(bool bCmd = true);
	void	SetFullOpen(bool bOpen);
	void	SetupGate(bool bCmd, int nCID, int nGate, bool bSide);
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
