#pragma once

#include "Body.h"
#include "AidLed.h"

#include "Ddcu.h"
#include "Dsiv.h"
#include "Dv3f.h"
#include "Decu.h"
#include "Dhvac.h"
#include "Panel.h"

#include "../../../Qnx2/Inform2/Pio.h"

class CCar : public CDialogEx
{
	DECLARE_DYNAMIC(CCar)

public:
	CCar(CWnd* pParent = NULL);
	CCar(CWnd* pParent, int nID);
	virtual ~CCar();

	enum { IDD = IDD_DIALOG_CAR };

protected:
	CWnd*	m_pParent;
	CBody	m_body;
	CRect	m_rect;
	CRect	m_rtDev;
	int		m_nCID;			// 0 ~ 9
	WORD	m_wDevs;
	BYTE	m_cDi;
	BYTE	m_cDo;
	int		m_nGateID;

	//CEdit	m_editCID;
	//CEdit	m_editEcuPress;
	//CSliderCtrl m_sliderEcuPress;

	CButton m_btnHidden;
	CButton m_btnDcuL;
	CButton m_btnDcuR;
	CButton m_btnHvac;
	CButton m_btnEcu;
	CButton m_btnV3f;
	CButton m_btnSiv;

	CStatic m_sttSiv;
	CStatic m_sttV3f;
	CStatic m_sttCmsb;
	CStatic m_sttEsk;
	CStatic m_sttCmk;

	CButton m_checkVff;
	CButton m_checkIes;
	CButton m_checkSivk;
	CButton m_checkSivf;
	CButton m_checkBvr;
	CButton	m_checkUops;
	CButton	m_checkUos;
	CButton m_checkCmf;
	CButton m_checkCmbp;
	CButton m_checkCmk;
	CButton m_checkCmg;
	CButton	m_checkCmn;
	CButton	m_checkMrcs;
	CButton m_checkEsk;
	CButton	m_checkIpdr;

	CButton	m_checkNest[4];

	CAidLed	m_lampEsk;
	CAidLed	m_lampCmk;
	//CAidLed	m_lampAlbo;

	CDdcu	m_dcu[2];
	CRect	m_rectDcu[2];
	CDsiv*	m_pSiv;
	CRect	m_rectSiv;
	CDv3f*	m_pV3f;
	CRect	m_rectV3f;
	CDecu	m_ecu;
	CRect	m_rectEcu;
	CDhvac	m_hvac;
	CRect	m_rectHtc;

	void	BitCtrl(UINT id, int nState = -1);
	//void	EcuPressureCtrl(bool bFrom);
	void	SetDevices();
	void	FeedIn(CButton* pButton, UINT id, bool bChk);

public:
	void	Ready(bool bCmd = true);
	void	BitCtrlDG(UINT id, int nState = -1);
	void	BitCtrlD(UINT id, int nState = -1);
	void	SetBody(BODYSTYLE bs);
	void	SetPanto(bool bCtrl);
	void	SetDoor(DOORCMD cmd, bool bSide);
	void	DiReaction(BYTE ch);
	void	DoReaction(BYTE cDO);
	void	SdrReaction(BYTE* p, int leng);
	void	SetupGates();
	CPanel*	GetPanel();
	PBODYSTYLE	GetBody()			{ return m_body.GetStyle(); }
	CWnd*	GetParent()				{ return m_pParent; }

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
	//afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg	LRESULT	OnBodyClick(WPARAM wParam, LPARAM lParam);
};
