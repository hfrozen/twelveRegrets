#pragma once

#include "AidLed.h"

#include "Dato.h"
#include "Dpau.h"
#include "Dpis.h"
#include "Dtrs.h"
//#include "Drtd.h"
#include "Panel.h"

#include "../../../Qnx2/Inform2/Pio.h"

class CCabin : public CDialogEx
{
	DECLARE_DYNAMIC(CCabin)

public:
	CCabin(CWnd* pParent = NULL);
	CCabin(CWnd* pParent, CDato* pAto);
	virtual ~CCabin();

	enum { IDD = IDD_DIALOG_CABIN };

protected:
	CWnd*	m_pParent;
	CRect	m_rect;

	CSliderCtrl m_sliderRudder;
	CSliderCtrl m_sliderMaster;
	CEdit	m_editRudder;
	CEdit	m_editMaster;
	CEdit	m_editBattery;
	CEdit	m_editTemperature;
	CEdit	m_editOrganize;
	CEdit	m_editDistance;
	CEdit	m_editCarTemp;
	CEdit	m_editSpeed;
	CComboBox	m_comboHvac;

	CAidLed*	m_pLamp[2];

	CDato*	m_pAto;
	CDpau	m_pau;
	CDpis	m_pis;
	CDtrs	m_trs;
	//CDrtd	m_rtd;

	int		m_nTID;			// 0 or 1
	CString	m_strDoor[2];
	int		m_nRudder;
	int		m_nMaster;
	BYTE	m_cDi;
	BYTE	m_cDo;
	int		m_nGateID;

	typedef struct _tagATOCTRL {
		BYTE	cType;
		UINT	uPwm;
	} ATOCTRL;
	ATOCTRL	m_atoCtrl;

	static const UINT	m_nRudderKeys[RUDDERPOS_MAX];
	static PCTSTR	m_pstrRudder[RUDDERPOS_MAX];
	static PCTSTR	m_pstrRudderA[RUDDERPOS_MAX];
	static PCTSTR	m_pstrMaster[MASTERPOS_MAX];

public:
	typedef struct _tagMASTERREF {
		MASTERPOS	pos;
		UINT		pwm;
		int			tbe;
	} MASTERREF;

protected:
	static const MASTERREF	m_mastRef[MASTERPOS_MAX];

	// * HVAC operation
	//	CHCS4	CHCS3	CHCS2	CHCS1
	//	0		0		0		1		: test, 1
	//	0		0		1		1		: line flow fan, 3
	//	0		0		1		0		: full cooling, 2
	//	0		1		0		0		: half cooling, 4
	//	1		0		0		0		: evaporator fan, 8
	//	0		1		0		1		: off, 5
	//	1		1		1		1		: auto, 15
	//	1		0		0		1		: 1/3 heating, 9
	//	1		1		1		0		: 2/3 heating, 14
	//	1		1		0		1		: full heating, 13
	enum  {
		HVACOPID_TEST = 0,
		HVACOPID_HC,
		HVACOPID_FC,
		HVACOPID_EVPF,
		HVACOPID_OFF,
		HVACOPID_AUTO,
		HVACOPID_1H,
		HVACOPID_2H,
		HVACOPID_FH,
		HVACOPID_MAX
	};
	static PCTSTR		m_pstrHvacOp[HVACOPID_MAX];
	static const UINT	m_nHvacOp[HVACOPID_MAX];

	void	RudderAddition();
	void	RudderCtrl();
	void	MasterAddition();
	void	MasterCtrl();
	//void	SpeedCtrl();
	void	BatteryCtrl();
	void	TemperatureCtrl();
	//void	OrganizeCtrl();
	void	HvacCtrl();
	void	DistanceCtrl();
	void	EbCtrl(UINT ebid, int nState);
	void	UnitClr();
	void	BitCtrl(UINT id, int nState = -1);
	void	CheckCtrl(UINT id, bool bCtrl);
	bool	EbState();
	bool	GetCheckState(UINT id);
	void	PwmCtrl(UINT sid, UINT uPwm);

public:
	void	Ready(bool bCmd = true);
	void	Activate(CABINCMD cmd);
	void	RudderCtrl(int nPos);
	void	MasterCtrl(int nPos);
	void	BitCtrlD(UINT id, int nState = -1);
	BYTE	GetHtcr();
	int		GetRudderKey();
	int		GetTrainNo();
	void	SetTrainID(int nID);
	void	SetTachoText(UINT uTacho);
	void	DiReaction(BYTE ch);
	void	DoReaction(BYTE cDO);
	void	SdrReaction(BYTE* p, int leng);
	void	SetupGates();
	CPanel*	GetPanel();
	int		CheckRudderKeyword(CString str);
	int		CheckMasterKeyword(CString str);
	void	AtoCtrl(BYTE cType, UINT uEffort);
	int		GetEffortType();
	int		GetEffort();

	void	SetAto(CDato* pAto)			{ m_pAto = pAto; }
	int		GetStationID(int nWhere)	{ return m_pAto->GetStationID(nWhere); }
	CString	GetStationName(int nID)		{ return m_pAto->GetStationName(nID); }
	int		GetFormationNo()			{ return m_pAto->GetFormationNo(); }
	int		GetTrainID()				{ return m_nTID; }
	CWnd*	GetParent()					{ return m_pParent; }

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
