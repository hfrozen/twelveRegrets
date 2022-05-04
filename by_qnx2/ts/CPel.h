/*
 * CPel.h	pantograph and extension supply and load reduction
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once

#include "../Component2/CPop.h"
#include "Infer.h"

class CPel
{
public:
	CPel();
	virtual ~CPel();

private:
	PVOID	c_pParent;
	CMutex	c_mtx;

	// 200218
	bool	c_bDevEx;		// cm operate for Alone, ?????
	bool	c_bCoolup;		// power on에서 CMSB를 동작시키는데 SIVK가 들어오고 7초를 기다린다.
	// FOR SAFETY
	bool	c_bEsk;
//	WORD	c_wSivStabilizeTime;
//#define	TIME_SIVSTABILIZE		REALBYSPREAD(4000);

//	//WORD	c_wPanBitmap;
//	//CCalm <BYTE>	c_pan[CID_MAX];
//	//WORD	c_wPanWait[CID_MAX];	// 팬터그래프 상승 후 가선 전압이 인가되길 기다리는 시간
	typedef struct _tagPANSTATE {
		WORD			wMap;
		struct {
			CCalm <BYTE>	st;
			WORD			wWait;	// 팬터그래프 상승 후 가선 전압이 인가되길 기다리는 시간
		} m[CID_MAX];
	} PANSTATE;
	PANSTATE	c_pans;
#define	DEB_PANTO				REALBYSPREAD(300)
#define	WAIT_PANTO				REAL2PERIOD(1000)

//#define	DEB_LOWBAT				5
//	WORD	c_wLowBatDeb[CID_MAX];

// 200218
//	//bool	c_bPerfectLoop;		// TC0, CC5, TC9의 DTB상태가 원할하면 1이 되어 연장 급전을 판단한다.
//	//WORD	c_wLoopOffTimer;	// DTB상태가 3초이상 원할하지 않으면 무조건 esk를 off한다.
	typedef struct _tagDTBSTATE {
		bool	bPerfect;		// TC0, CC5, TC9의 DTB상태가 원할하면 1이 되어 연장 급전을 판단한다.
		WORD	wOffTimer;		// DTB상태가 3초이상 원할하지 않으면 무조건 esk를 off한다.
	} DTBSTATE;
	DTBSTATE	c_dtbs;
#define	TIME_LOOPOFF			REAL2PERIOD(3000)

//	//typedef struct _tagSIVFMON {
//	//	bool	bCur;
//	//	WORD	wWaitKOff;
//	//} SIVFMON;
//	//SIVFMON	c_sivfm[CID_MAX];
//
	typedef struct _tagSIVSTATE {
		WORD	wStabilizeTime;
		struct {
			bool	bCur;
			WORD	wWaitKOff;
		} m[CID_MAX];
	} SIVSTATE;
	SIVSTATE	c_sivs;
#define	TIME_SIVSTABILIZE		REALBYSPREAD(4000);
#define	WAIT_SIVKOFF			REALBYSPREAD(2000)

//	WORD	c_wEskState;
//#define	TIME_LOADOFFAUTO		REAL2PERIOD(3000)
//#define	TIME_LOADOFFMANUAL		REAL2PERIOD(10000)
//	WORD	c_wLoadOffTimer;
//
//#define	WAIT_ESKON				REALBYSPREAD(3000)		// 171101
//
//	//typedef struct _tagESKMON {
//	//	bool	bCur;
//	//	WORD	wWait;
//	//} ESKMON;
//	//ESKMON	c_eskm[CID_MAX];
//
	typedef struct _tagESKSTATE {
		WORD	wState;
		WORD	wLoadOffTimer;
		struct {
			bool	bCur;
			WORD	wWait;
		} m[CID_MAX];
	} ESKSTATE;
	ESKSTATE	c_esks;
#define	WAIT_ESKON			REALBYSPREAD(3000)		// 171101
#define	TIME_LOADOFFAUTO	REAL2PERIOD(3000)
#define	TIME_LOADOFFMANUAL	REAL2PERIOD(10000)

//#define	WAIT_CMKON			REALBYSPREAD(6000)	// 170917, 3000)// 1000)
//	typedef struct _tagCMKMON {
//		bool	bCur;
//		WORD	wWait;
//	} CMKMON;
//	CMKMON	c_cmkm[CID_MAX];
//	//WORD	c_wCmkBitmap;
//#define	TIME_CMON			REAL2PERIOD(900000)	// 15분	//(180000)	// 3분
//	DWORD	c_dwCmonTimer;
//	bool	c_bCmonf;
//
//	typedef struct _tagCMKMON {
//		bool	bCur;
//		WORD	wWait;
//	} CMKMON;
//
	typedef struct _tagCMSTATE {
		bool	bFlag;
		DWORD	dwOnTimer;
		struct {
			bool	bCur;
			WORD	wWait;
		} m[CID_MAX];
	} CMSTATE;
	CMSTATE	c_cms;
#define	WAIT_CMKON		REALBYSPREAD(10000)
	// 201207(6000)	// 170917,3000)	//1000)
#define	WAIT_CMKONA		REALBYSPREAD(15000)	// 210219
	// 201016
#define	WAIT_CMKOFF		REALBYSPREAD(10000)
#define	TIME_CMON		REAL2PERIOD(900000)	// 15분	//(180000)	// 3분

	typedef struct _tagINSPECTAID {
		bool	bExit;
		bool	bResult[CID_MAX];
		WORD	wSeq[CID_MAX];
		WORD	w;
	} INSPECTAID;

	INSPECTAID	c_inspAid;

	void	CheckPantograph();
	void	CheckSiv(PRECIP pRecip);
	void	CheckCmsb();
	void	CheckEsk();
	void	CheckExtenstionPower(PRECIP pRecip);

public:
	void	Initial();
	void	Monitor(PRECIP pRecip);
	void	Control(PRECIP pRecip);
	void	Alone();
	void	Arteriam();

	BYTE	GetPantoState(BYTE id)		{ return c_pans.m[id].st.cur; }
	void	SetParent(PVOID pParent)	{ c_pParent = pParent; }

	PUBLICY_CONTAINER();
};
