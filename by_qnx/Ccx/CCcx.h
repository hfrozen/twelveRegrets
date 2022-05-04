/*
 * CCcx.h
 *
 *  Created on: 2010. 12. 23
 *      Author: Che
 */

#ifndef CCCX_H_
#define CCCX_H_

#include <sys/syspage.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <pthread.h>

#include <Draft.h>
#include <CAnt.h>
#include <CTimer.h>
#include <Msg.h>
#include "TrainArch.h"
#include "CPanelCc.h"
#include "CPocket.h"
#include "CBusCarx.h"


#include <CProbe.h>
#include <CPwm.h>

#define	VERSION_CCX		1.48
#define	REBUILD_CCX		131102
#define	GETCCCX(p)	CCcx* p = (CCcx)c_pParent

class CCcx	: public CAnt
{
public:
	CCcx();
	virtual ~CCcx();

protected:
	CCREFERENCE	c_ccRef;
	PCCDOZ		c_pDoz;
	CCDOZ		c_doz;
	SIVRINFO	c_rSiv[3];
	V3FRINFO	c_rV3f[3];
	ECURINFO	c_rEcu[3];
	CMSBRINFO	c_rCmsb[2];
	DCURINFO	c_rDcu[2];
	SIVAINFO	c_aSiv;
	V3FAINFO	c_aV3f;
	ECUAINFO	c_aEcu;
	DCUAINFO	c_aDcuc[2];

	BOOL		c_bTog;

private:
	int			c_mail;
	BOOL		c_bMfb;
	pthread_mutex_t	c_mutex;
	pthread_mutex_t	c_mutexA;
	long		c_syncCur, c_syncMax, c_syncMin;
	uint64_t	c_logTriggerTime;

	TBTIME		c_tbTime;
	DEVTIME		c_devTime;
	CPanelCc	c_panel;
	CProbe		c_probe;
	CPwm		c_pwm;
	CTimer*		c_pTimer;
	CPocket*	c_pPocket;
	PBUCKET		c_pBucket;
	int			c_hShm;

	typedef struct _tagBUSARCH {
		WORD	wState;
#define	BUSSTATE_ENABLE		1
#define	BUSSTATE_WAIT		2

		CBusCarx*	pLine;
	} BUSARCH;
	BUSARCH		c_main;
	BUSARCH		c_resv;

	static const PSZ	c_timeNames[];
	OPERATETIME	c_opTimes[CTIME_MAXSHEET];

	SIVCHASE	c_sivch;
	V3FCHASE	c_v3fch;
	ECUCHASE	c_ecuch;

protected:
	void	MaAccept(DWORD dwParam);
	void	MaSendBus(DWORD dwParam);
	void	MaReceiveBus(DWORD dwParam);
	void	MaReceiveFault(DWORD dwParam);
	void	MaReceivePocket(DWORD dwParam);
	DECLARE_MAILMAP();

	static void	Interval(PVOID pVoid);
	BOOL	Trigger();
	BOOL	Seed();
	BOOL	BusSetup();
	BOOL	OpenPost();
	BOOL	OpenBeil();
	BOOL	TakingOff();
	void	SysTimer();
	void	BusMonit();

	void	CopyTime(PDEVTIME pDevt, PTBTIME pTbt);
	void	CopyOut();
	void	OutputD();
	void	Header(char* p, int head);
	void	CalcTimes();
	void	BusPresent();
	void	BusPresentEntry();
	void	IoTest();
	BYTE	BitwiseIn(BYTE in);
	BYTE	BitwiseOut(BYTE out);
	BYTE	ToHex(BYTE bcd);
	BYTE	BitIndex(BYTE bits);
	void	LongToByte(BYTE* p, long v);
	void	CalcSpeedBase(UCURV n);

	void	AbstractSiv();
	void	AbstractV3f();
	void	AbstractEcu();
	void	AbstractCmsb();
	void	AbstractDcu();

	void	CalcV3f();
	void	CalcSiv();
	void	SimulSiv();
	void	SimulV3f();

public:
	BOOL	Run(int aich);
	void	DevTrigger();
	void	Destroy();
	void	RecordTime(UCURV n);
	void	Lock()					{ pthread_mutex_lock(&c_mutex); }
	void	Unlock()				{ pthread_mutex_unlock(&c_mutex); }
	void	LockA()					{ pthread_mutex_lock(&c_mutexA); }
	void	UnlockA()				{ pthread_mutex_unlock(&c_mutexA); }
	void	SetDoorFlow(BYTE flow)	{ c_ccRef.real.nDoorFlow = flow; }
	void	SetDevDeb(WORD* p)		{ memcpy((PVOID)&c_ccRef.real.wDevDeb[0], (PVOID)p, DEVID_MAX * sizeof(WORD)); }
	void	SetDoorDeb(WORD* p)		{ memcpy((PVOID)&c_ccRef.real.wDoorDeb[0], (PVOID)p, 8 * sizeof(WORD)); }
	WORD	GetActiveDev()			{ return c_ccRef.real.wActiveDev; }
	PCCDOZ		GetCcDoz()			{ return &c_doz; }
	PSIVRINFO	GetrSivInfo()		{ return &c_rSiv[1]; }
	UCURV		GetrSivLength()		{ return (c_rSiv[1].nFlow == DEVFLOW_TRACER ? SIZE_SIVTR : SIZE_SIVSR); }
	PV3FRINFO	GetrV3fInfo()		{ return &c_rV3f[1]; }
	UCURV		GetrV3fLeng()		{ return (c_rV3f[1].nFlow == DEVFLOW_TRACER ? SIZE_V3FTR : SIZE_V3FSR); }
	PECURINFO	GetrEcuInfo()		{ return &c_rEcu[1]; }
	UCURV		GetrEcuLength()		{ return (c_rEcu[1].nFlow == ECUFLOW_TDTA ? SIZE_ECUTR : SIZE_ECUSR); }
	PCMSBRINFO	GetrCmsbInfo()		{ return &c_rCmsb[1]; }
	PDCURINFO	GetrDcuInfo()		{ return &c_rDcu[1]; }
	PSIVAINFO	GetaSivInfo()		{ return &c_aSiv; }
	PV3FAINFO	GetaV3fInfo()		{ return &c_aV3f; }
	PECUAINFO	GetaEcuInfo()		{ return &c_aEcu; }
	PDCUAINFO	GetaDculInfo()		{ return &c_aDcuc[0]; }
	PDCUAINFO	GetaDcurInfo()		{ return &c_aDcuc[1]; }
};

#endif /* CCCX_H_ */
