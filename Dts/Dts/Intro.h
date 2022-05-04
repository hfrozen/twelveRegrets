#pragma once

#include "Doc.h"

#include "Panel.h"

#include "Log.h"
#include "Cook.h"
#include "Car.h"
#include "Cabin.h"
#include "Dato.h"
#include "Dashb.h"

#include "ServerSocket.h"
#include "Tcm.h"

#define	MAX_SPEEDCTRLLEVEL	8
#define	TARGETSPEED_BENDING	0.82f

class CIntro : public CDialogEx
{
	DECLARE_DYNAMIC(CIntro)

public:
	CIntro(CWnd* pParent = NULL);
	virtual	~CIntro();

	enum { IDD = IDD_DIALOG_INTRO };

protected:
	CWnd*	m_pParent;
	int		m_nMonitor;
	CRect	m_rect[10];
	CFont	m_font[FONTS_LENGTH];
	bool	m_bFreeze;

	typedef struct _tagCALCSPEED {
		UINT	cid;
		int		vCurrent;
		int		vTarget;
		bool	bTenor;
		bool	bCtrl;
		int		nCount;
		int		nBend;
		int		nLevel;
		double	db[4];
	} CALCSPEED;
	CALCSPEED	m_speed;

	typedef struct _tagCALCSPEEDA {
		UINT	cid;
		int		vCur;
		int		vTar;
		int		vStep;
	} CALCSPEEDA;
	CALCSPEEDA	m_spda;

	double		m_dbDia;

	CButton m_checkRxHide;
	CButton m_checkTxHide;

	CButton	m_btnAct;
	CButton m_btnPanel;
	CButton m_btnDebuger;
	CButton	m_btnPort;
	CButton m_btnExit;

	CARORGNZ	m_coi;
	CDoc	m_doc;
	CCook	m_cook;
	CLog*	m_pLog[3];
	CCar*	m_pCar[CARPOS_MAX];
	CCabin*	m_pCabin[2];
	CDato*	m_pAto[2];
	CDashb*	m_pDashb;
	CPanel*	m_pPanel;
	CBitmap	m_bitmap;

	int		m_nBow;
	//BYTE	m_nTrainDir;
	// B3 B2	B1 B0
	//  0  0     0  0
	//  1  1     1  1
	//  0  1     X  X	- TC9 Forward
	//  1  0     X  X	- TC9 Reverse
	//  X  X     0  1	- TC0 Forward
	//  X  X     1  0	- TC0 Reverse

	static const WORD	m_wCarOrgnz[CARORGNZ_MAX];
	static const WORD	m_wDevs[CARPOS_MAX];
	static const CARTYPE	m_cCarTypes[CARPOS_MAX];

	CServerSocket*	m_pServerSocket;

	typedef CTypedPtrList <CPtrList, CTcm*> TCMES, *PTCMES;
	TCMES	m_tcmes;

public:
	void	Log(CString strLog, COLORREF crText = COLOR_BLACK);
	void	Logp(CString strLog, COLORREF crText = COLOR_BLACK);
	void	Logr(CString strLog, COLORREF crText = COLOR_BLACK);
	void	DistributeDies(int cid, BYTE ch);
	void	DistributeDoes(int cid, BYTE cDO);
	void	DistributeRecv(int id, BYTE* p, int leng);
	void	PantoCtrl(bool bCtrl);
	void	DoorCtrl(DOORCMD cmd, bool bSide);
	void	SendBinCmd(int id, int leng, ...);
	//void	SpeedCtrlA(UINT cid, int notch);
	void	SpeedCtrlFromPwm(UINT cid, int iPwm);
	//void	SpeedCtrl(UINT cid, int vSpeed);
	//void	SpeedCtrl(UINT cid, bool bRise, int nLevel);
	int		GetSpeed()								{ return m_spda.vCur; }
	void	UnitClr(UINT cid)						{ if (m_pPanel != NULL)	m_pPanel->UnitClr(cid); }
	void	BitCtrl(UINT cid, UINT sid, bool bCtrl)	{ if (m_pPanel != NULL)	m_pPanel->BitCtrl(cid, sid - IDC_CHECK_DEV0, bCtrl); }
	void	PwmCtrl(UINT cid, UINT sid, UINT uPwm)	{ if (m_pPanel != NULL)	m_pPanel->PwmCtrl(cid, sid, uPwm); }
	void	EnableAlarm(bool bAlarm)				{ if (m_pPanel != NULL)	m_pPanel->EnableAlarm(bAlarm); }
	CPanel*	GetPanel()			{ return m_pPanel; }
	CDoc*	GetDoc()			{ return &m_doc; }
	WORD	GetDevices(int id)	{ return m_wDevs[id]; }
	CRect	GetWorkRect()		{ return m_rect[9]; }

	BYTE	GetHtcr(UINT nCid)							{ return m_pCabin[nCid != 0 ? 1 : 0]->GetHtcr(); }
	void	TBitCtrlD(UINT nCid, UINT nID, bool bState)	{ m_pCabin[nCid != 0 ? 1 : 0]->BitCtrlD(nID, bState ? BST_CHECKED : BST_UNCHECKED); }
	void	CBitCtrlD(UINT nCid, UINT nID, bool bState)	{ m_pCar[nCid]->BitCtrlDG(nID, bState ? BST_CHECKED : BST_UNCHECKED); }
	bool	GetBowTBitD(UINT nID)						{ if (m_nBow > -1)	return m_pPanel->GetBit(CARPOS_MAX + m_nBow, nID - IDC_CHECK_DEV0); return false; }
	int		GetEffortType()								{ if (m_nBow > -1)	return m_pCabin[m_nBow]->GetEffortType(); return EFFORT_NOATC; }
	int		GetEffort()									{ if (m_nBow > -1)	return m_pCabin[m_nBow]->GetEffort(); return 0; }
	bool	GetTBitD(UINT nCid, UINT nID)				{ if (m_pPanel != NULL) return m_pPanel->GetBit(CARPOS_MAX + (nCid & 1), nID - IDC_CHECK_DEV0); return false; }
	bool	GetCBitD(UINT nCid, UINT nID)				{ if (m_pPanel != NULL) return m_pPanel->GetBit(nCid, nID - IDC_CHECK_DEV0); return false; }
	void	RudderCtrl(UINT nCid, int nPos)				{ m_pCabin[nCid != 0 ? 1 : 0]->RudderCtrl(nPos); }
	void	MasterCtrl(UINT nCid, int nPos)				{ m_pCabin[nCid != 0 ? 1 : 0]->MasterCtrl(nPos); }
	void	SetTachoText(UINT nCid, UINT uTacho)		{ m_pCabin[nCid != 0 ? 1 : 0]->SetTachoText(uTacho); }
	int		CheckRudderKeyword(UINT nCid, CString str)	{ return m_pCabin[nCid != 0 ? 1 : 0]->CheckRudderKeyword(str); }
	int		CheckMasterKeyword(UINT nCid, CString str)	{ return m_pCabin[nCid != 0 ? 1 : 0]->CheckMasterKeyword(str); }
	int		GetRxHideCheck()							{ return m_checkRxHide.GetCheck(); }
	int		GetTxHideCheck()							{ return m_checkTxHide.GetCheck(); }

	void	StirWatcher(CString strIp);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg	LRESULT	OnDirectionCtrl(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnConnectDio(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnConnectScu(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnAccept(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnInformTcm(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT	OnCloseTcm(WPARAM wParam, LPARAM lParam);
};
