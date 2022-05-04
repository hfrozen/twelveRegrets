#pragma once

#include "AidTexts.h"

#include "../../../Qnx2/Inform2/DevInfo2/AtoInfo.h"

class CDato : public CDialogEx
{
	DECLARE_DYNAMIC(CDato)

public:
	CDato(CWnd* pParent = NULL);
	virtual ~CDato();

	enum { IDD = IDD_DIALOG_DATO };

protected:
	CWnd*	m_pParent;
	CSliderCtrl m_sliderEffort;
	CAidTexts	m_mons[MONCOM_MAX];
	int		m_nTID;
	int		m_nGate;

	static PCWSTR	m_strStations[];

	void	SetEffort(bool bFrom);
	void	SetStation(int idc);
	void	DisableLine();
	BYTE	GetEffortType();
	UINT	GetEffort();

public:
	void	SetTitle(CString strCarName, CString strDevName);
	void	SetDevName(CString strDevName);
	void	SetCarName(CString strCarName)		{ SetWindowTextW(strCarName); }
	void	Ready(bool bCmd = true);
	void	SetupGate(bool bCmd, int nTID, int nGate);
	void	CollectBuild();
	void	RecvSdr(BYTE* p, int length);
	int		GetMode();
	int		GetStationID(int nWhere);
	CString	GetStationName(int nID);
	int		GetFormationNo();
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
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
};
