#pragma once

#include "AidTexts.h"

#include "../../../Qnx2/Inform2/DevInfo2/SivInfo.h"

class CDsiv : public CDialogEx
{
	DECLARE_DYNAMIC(CDsiv)

public:
	CDsiv(CWnd* pParent = NULL);
	virtual ~CDsiv();

	enum { IDD = IDD_DIALOG_DSIV };

protected:
	CWnd*	m_pParent;
	CAidTexts	m_mons[MONCOM_MAX];
	int		m_nCID;
	int		m_nGate;
	int		m_nPantoID;
	bool	m_bPanto;
	int		m_nTestStep;
	int		m_nTestTime;

	SIVRINFO	m_sr[2];

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
