#pragma once

#include "AidTexts.h"

#include "../../../Qnx2/Inform2/DevInfo2/TrsInfo.h"

class CDtrs : public CDialogEx
{
	DECLARE_DYNAMIC(CDtrs)

public:
	CDtrs(CWnd* pParent = NULL);
	virtual ~CDtrs();

	enum { IDD = IDD_DIALOG_DTRS };

protected:
	CWnd*	m_pParent;
	CAidTexts	m_mons[MONCOM_MAX];
	int		m_nTID;
	int		m_nGate;

	void	DisableLine();

public:
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
