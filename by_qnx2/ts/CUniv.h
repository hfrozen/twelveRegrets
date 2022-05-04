/*
 * CUniv.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once

#include "Infer.h"

class CUniv {
public:
	CUniv();
	virtual ~CUniv();

private:
	PVOID	c_pParent;

	// 171113
	// 180917
//#define	TIME_NRBDETECT		REALBYSPREAD(7000)		// 171219, 5000)
//	WORD	c_wNrbdTime[ECU_MAX];

#define	DEB_DOORMODE	REALBYSPREAD(300)

	enum {
		DOORMODE_NON = 0,
		DOORMODE_AOAC,
		DOORMODE_AOMC,
		DOORMODE_MOMC
	};

	typedef struct _tagDOORMODE {
		CCalm <BYTE>	d;
		BYTE			mode;
		BYTE			verf;
	} DOORMODE;
	DOORMODE	c_dm;

	enum {
		DOORCTRL_LDOS = 0,
		DOORCTRL_RDOS,
		DOORCTRL_DCS
	};

	BYTE	c_dcPrev;

	typedef struct _tagINSPECTAID {
		bool	bExit;
		bool	bResult1[CID_MAX];
		bool	bResult2[CID_MAX];
		WORD	wSeq[CID_MAX];
		_QUARTET	qu[0];
	} INSPECTAID;

	INSPECTAID	c_inspAid;
	WORD	c_wP1DelayTimer;
#define		TIME_P1DELAY		20

private:
	void	CheckV3f();
	void	CheckEcu(PRECIP pRecip);
	void	CheckEcu2(PRECIP pRecip);
	void	CheckHvac();
	bool	CheckFire();
	// 200218
	void	CheckFire2();
	void	CheckPassenger();
	void	CheckDoorLine(int cid, PDCUFLAP pDcu);
	void	CheckDoorMode(PRECIP pRecip);
	void	CheckDoorCmd();
	void	CheckDoorEach();

public:
	void	Initial();
	void	Monitor(PRECIP pRecip);
	void	Control(PRECIP pRecip);
	void	Arteriam();

	void	SetParent(PVOID pParent)			{ c_pParent = pParent; }

	PUBLICY_CONTAINER();
};
