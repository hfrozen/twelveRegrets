/*
 * CRtd.h
 *
 *  Created on: 2016. 10. 9.
 *      Author: Che
 */
#pragma once

#include "../Inform2/Arrange.h"
#include "../Inform2/Slight.h"

#include "../Base2/CPump.h"
#include "../Component2/CFio.h"
#include "../Component2/CFsc.h"
#include "../Component2/CTimer.h"
#include "../Component2/CDev.h"
#include "../Component2/CPop.h"

#include "../Inform2/Docu.h"
#include "../Inform2/DevInfo2/DevInfo.h"

class CRtd :	public CPump
{
public:
	CRtd();
	virtual ~CRtd();

private:
#define	PERIOD_TICK		REAL2PERIOD(200)
	BYTE	c_cTick;

#define	PERIOD_SEQ		15
	BYTE	c_cTextTick;
	bool	c_bTextSeqChg;
	BYTE	c_cTextSeq;

	uint64_t	c_clock;
	DTSTIME	c_dt;

	CMutex	c_mtxf;
	CMutex	c_mtxr;
	CFio	c_fio;
	CTimer*	c_pTuner;
	CDev*	c_pDev[2];

	typedef struct _tagDEVRECPERIOD {
		bool	bCont;
		uint64_t	clk;
		WORD	wPeriod;
		WORD	wPeriodBuf;
		WORD	wCnt;
		WORD	wCntBuf;
	} DEVRECP;
	DEVRECP	c_devr[2];

#define	CYCLE_RECEIVEA		REAL2PERIOD(3000)
#define	CYCLE_RECEIVEB		REAL2PERIOD(500)
	WORD	c_wCycle[2];
	WORD	c_wRed[2][LCF_MAX];
	DWORD	c_dwArch[2];

	enum {
		RTDR_ID = 0,
		RTDA_ID,
		RTDID_MAX
	};
	RTDBINFO	c_rInfo[RTDID_MAX];

	static const LOCALLAYERA	c_layer[];

	void	Destroy();
	static void	IntroTune(PVOID pVoid);
	void	Arteriam();
	void	CalcPeriod(int nId, uint64_t clk);
	void	Receive(PRTDBINFO pRtd, int nId, WORD wLength);
	void	IncreaseRed(int nId, WORD wErr);

protected:
	void	TakeReceiveDev(DWORD dw);
	PUBLISH_TAGTABLE();

public:
	bool	Launch();
};
