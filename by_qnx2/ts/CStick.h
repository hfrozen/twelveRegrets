/*
 * CStick.h
 *
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once

#include "../Component2/CPop.h"
#include "Infer.h"

class CStick
{
public:
	CStick();
	virtual ~CStick();

private:
	PVOID	c_pParent;
	//DTSTIME	c_stime;		// F/R이 투입되는 시간
	bool	c_bDomin;
	bool	c_bHcrOff;
	WORD	c_wWarmupTimer;

#define	TPERIOD_WARMUPA			REALBYSPREAD(3000)		// 제어 감시가 활성화 된 상태(OBLIGE_ACTIVE)에서 설정되는 값, 전원 투입에서 설정되는 값
#define	TPERIOD_WARMUPB			REALBYSPREAD(2000)		// REALBYSPREAD(10000)		// 선두 차가 바뀔 때..

#define	DEB_REVERSALA			REALBYSPREAD(600)
#define	DEB_REVERSALB			REALBYSPREAD(300)
#define	DEB_MUTUAL				REALBYSPREAD(300)
#define	DEB_RUDDER				REALBYSPREAD(300)
#define	DEB_HCRMON				REALBYSPREAD(300)

	typedef struct _tagMCDDEB {
		bool	bCur;
		bool	bBuf;
		BYTE	cDeb;
	} MCDDEB;

	typedef struct _tagREVERSAL {	// mcd of draft
		// 190924
		//CCalm <bool>	d;
		MCDDEB	md;
		bool	bFlow;				// state of flow chart
	} REVERSAL;
	REVERSAL	c_reversal;

	typedef struct _tagMUTUAL {		// both HCR/TCR
		CCalm <BYTE>	d;
		BYTE			cPattern;
		BYTE			cError;
	} MUTUAL;
	MUTUAL		c_mut;

	typedef struct _tagRUDDER {		// both F/R
		CCalm <BYTE>	d;
		BYTE			cPattern;
	} RUDDER;
	RUDDER		c_rud;

	// 171226
	// 170907
	//typedef struct _tagHCRMON {
	//	CCalm <bool>	b;
	//	bool			bPrev;
	//	bool			bCatchDown;
	//} HCRMON;
	//HCRMON		c_hm;

	CMutex	c_mtx;

	enum enMUTUAL_PATTERN {
		MUTUALPAT_UNKNOWN = 0,
		MUTUALPAT_CONTINUE,
		MUTUALPAT_CHANGE,
		MUTUALPAT_DOWN
	};
	static const WORD	c_wMutualPattern[20][2];

public:
	void	Initial();
	void	Control(PRECIP pRecip);
	void	Monitor();
	void	CheckDomination();
	//bool	GetStableHcr()				{ return c_hm.b.cur; }
	void	ClearReversal()				{ memset(&c_reversal, 0, sizeof(REVERSAL)); }
	void	SetParent(PVOID pParent)	{ c_pParent = pParent; }

	PUBLICY_CONTAINER();
};
