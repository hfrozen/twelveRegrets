/*
 * CReview.h
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#pragma once

#include "Slight.h"
#include "Draft.h"
#include "Infer.h"
#include "Insp.h"

#define	GETREVIEW(p)	CReview* p = CReview::GetInstance()

class CReview
{
public:
	CReview();
	virtual ~CReview();

private:
	typedef struct _tagINSPECTCMD {
		WORD	wItem;
		WORD	wStep;
		WORD	wTime;
		WORD	w10m;
		WORD	wHold;
		bool	bHold;
	} INSPCMD;
	INSPCMD	c_insp;

public:
	typedef struct _tagLINEFAILCOUNT {
		WORD	wDtb[PID_MAX];			// Note It is "PID"
		WORD	wPis[2];
		WORD	wPau[2];
		WORD	wTrs[2];
		WORD	wRtd[2];
		// 200218
		WORD	wFdu[2];
		WORD	wSiv[SIV_MAX];
		WORD	wV3f[V3F_MAX];
		WORD	wEcu[ECU_MAX];
		WORD	wHvac[HVAC_MAX];
		// 200218
		WORD	wCmsb[SIV_MAX];
		WORD	wBms[SIV_MAX];
	} LFCOUNT, *PLFCOUNT;
	LFCOUNT	c_lfc;

private:
	void	SetInspectTime();

public:
	int		GetInspectHduPage();
	void	InspectRegister(WORD wItem);
	void	InspectClear();
	int		NextInspectStep();		// return Hdu's page
	void	InspectPause();
	void	InspectRetry();
	void	InspectSkip();
	bool	GetInspectLapse();
	WORD	ExcerptDuo(WORD w, int shift);
	DWORD	ExcerptDuo(DWORD dw, int shift);
	void	CoordDuo(WORD* pW, WORD duo, int shift);
	void	CoordDuo(DWORD* pDw, DWORD duo, int shift);
	//void	CopyInspectCmd(PINSPCMD pInsp);
	void	Arteriam();
	WORD	GetInspectItem()				{ return c_insp.wItem; }
	WORD	GetInspectStep()				{ return c_insp.wStep; }
	void	SetInspectStep(WORD wStep)		{ c_insp.wStep = wStep; }
	WORD	GetInspectTime()				{ return c_insp.wTime; }
	bool	GetInspectPause()				{ return c_insp.bHold; }
	PLFCOUNT	GetLineFaultCount()			{ return &c_lfc; }

	static CReview*	GetInstance()	{ static CReview review; return &review; }

	PUBLICY_CONTAINER();
};
