/*
 * CLide.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once

#include "../Component2/CPop.h"

class CLide
{
public:
	CLide();
	virtual ~CLide();

private:
	PVOID	c_pParent;

#define	TIME_WAITFETCHBYRTD		REAL2PERIOD(90000)
// 201016
#define	COMPARE_SPEED		10.f	// speed comparison is more than 10Km/h

	typedef struct _tagRTDTIMEFETCH {
		WORD	wWaitTimer;
		BYTE	cSecond;
		bool	bFetch;
	} RTDTFETCH;
	RTDTFETCH	c_rtdtf;

	// 180717
	bool	c_bAclUnknown;			// 초기에는 1로 하여 모든 등을 켜도록한다.
	bool	c_bDclUnknown;
	// 180704
	//typedef struct _tagPISMANUALREQ {
	//	bool	bRequest;
	//	bool	bInit;
	// 1 상태에서는 bCurrent를 pis.sa.inf.b.manu에 따라 초기화한다.
	//	bool	bCurrent[2];
	//	bool	bStatus;
	//} PISMANUALREQ;
	//PISMANUALREQ	c_pisReq;

	void	PromptAto(PATOAINFO pAtoa, BYTE ci);
	void	CheckAto(bool bTenor, BYTE si, BYTE fid, BYTE ci);
	void	PromptPau(PPAUAINFO pPaua, BYTE ci, BYTE cFlow);
	void	CheckPau(bool bTenor, BYTE si, BYTE fid, BYTE ci);
	void	PromptPis(PPISAINFO pPisa, BYTE ci, BYTE cFlow);
	void	CheckPis(bool bTenor, BYTE si, BYTE fid, BYTE ci);
	void	PromptTrs(PTRSAINFO pTrsa, BYTE ci, BYTE cFlow);
	void	CheckTrs(bool bTenor, BYTE si, BYTE fid, BYTE ci);
	// 200218
	void	PromptFdu(PFDUAINFOAB pFdua, BYTE ci);
	void	CheckFdu(bool bTenor, BYTE si, BYTE fid, BYTE ci);
	void	PromptBms(PBMSAINFOAB pBmsa, BYTE ci);
	void	CheckBms(bool bTenor, BYTE si, BYTE fid, BYTE ci);

	void	PromptRtd(PRTDSTATEA pRtds, BYTE ci);
	void	CheckRtd(bool bTenor, BYTE si, BYTE fid, BYTE ci);

public:
	void	Initial();
	void	Control(PRECIP pRecip);
	void	LampControl(PRECIP pRecip);
	void	Monitor(PRECIP pRecip);
	void	Arteriam();
	// 181023
	void	ClearRtdTimeFetch()			{ c_rtdtf.bFetch = false; }

	//bool	GetDesiredPisMode()			{ return c_pisReq.bStatus; }
	//bool	GetPisModeChange()			{ return c_pisReq.bRequest; }
	//void	ClearPisModeChange()		{ c_pisReq.bRequest = false; }
	void	SetParent(PVOID pParent)	{ c_pParent = pParent; }

	//PUBLICY_CONTAINER();
};
