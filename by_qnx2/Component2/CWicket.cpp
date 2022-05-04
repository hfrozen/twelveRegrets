/*
 * CWicket.cpp
 *
 *  Created on: 2015. 11. 25.
 *      Author: Che
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <sys/neutrino.h>

#include "Track.h"
#include "CPaper.h"
#include "CVerse.h"
#include "CProse.h"
#include "CArch.h"
#include "CBand.h"
#include "CFsc.h"

#if	defined(_AT_TS_)
#include "../ts/CLand.h"
#endif

#include "CWicket.h"

const PSZ	CWicket::c_szInit = (PSZ)"INIT>";
const PSZ	CWicket::c_szNorm = (PSZ)"NORM>";
const PSZ	CWicket::c_szCont = (PSZ)"CONT>";

CWicket::CWicket()
	: CPump((PSZ)"WICK")
{
	c_hMcq = INVALID_HANDLE;
	c_bOpen = false;
	c_bIdle = false;
	c_runIdle = 0;
}

CWicket::~CWicket()
{
	Close();
}

bool CWicket::Open()
{
	c_hMcq = mq_open((char*)ENTQ_STR, O_WRONLY);
	if (Validity(c_hMcq)) {
		memset(&c_mcm, 0, sizeof(MCMSG));
		c_mcm.dwLength = (DWORD)this;
		c_mcm.wID = MCMSG_CONTS;
		mq_send(c_hMcq, (char*)&c_mcm, sizeof(MCMSG), 0);		// PRIORITY_POST
		return true;
	}
	TRACK("%s>MON>ERR:can not message queue to socket!(%s)\n", c_szOwnerName, strerror(errno));
	return false;
}

void CWicket::Close()
{
	if (Validity(c_hMcq))	mq_close(c_hMcq);
	c_bOpen = false;
}

void CWicket::SendContinue()
{
	if (!Validity(c_hMcq))	return;

	memset(&c_mcm, 0, sizeof(MCMSG));
	strncpy((char*)&c_mcm.cBuf[6], c_szCont, strlen(c_szCont));
	Send((WORD)(strlen(c_szCont) + 6));
}

void CWicket::Send(bool bType)
{
	if (!c_bOpen || !Validity(c_hMcq))	return;

	memset(&c_mcm, 0, sizeof(MCMSG));
	BYTE* p = &c_mcm.cBuf[6];
	WORD sum = 6;
	if (bType) {
		strncpy((char*)p, c_szInit, strlen(c_szInit));
		p += strlen(c_szInit);
		sum += strlen(c_szInit);

		int leng;
		GETBAND(pBand);
		leng = pBand->Bale(true, p);		// bus time
		p += leng;
		sum += leng;

		sprintf((char*)p, "$VAR,%d,(", SIZE_RECIP);	// tc or cc variables
		while (*p != '\0')	{ ++ p; ++ sum; }

#if	defined(_AT_TS_)
		CLand* pLand = (CLand*)c_pParent;
		leng = pLand->Bale(true, p);
		p += leng;
		sum += leng;
		*p ++ = ')';
		*p ++ = ',';
		sum += 2;
#endif

		GETPAPER(pPaper);				// bus
		leng = pPaper->Bale(true, p);
		p += leng;
		sum += leng;
		WORD w = sum;

		//GETVERSE(pVerse);
		//leng = pVerse->BaleSvcRef(true, p);
		//leng = pVerse->BaleHduRef(true, p);
		// 200610
		//GETARCHIVE(pArch);
		//leng = pArch->Bale(true, p);
		//p += leng;
		//sum += leng;
		GETPROSE(pProse);
		leng = pProse->Bale(true, p);
		p += leng;
		sum += leng;

		GETFSC(pFsc);
		leng = pFsc->Bale(true, p);
		p += leng;
		sum += leng;

		*p = '\0';
		++ sum;
#if	_DEBL(DEBUG_INFO)
		TRACK("%s>MON:init %d %d.\n", c_szOwnerName, w, sum);
#endif
	}
	else {
		if (c_bIdle) {
			uint64_t clk = ClockCycles();
			double sec = (double)(clk - c_runIdle) / (double)SYSPAGE_ENTRY(qtime)->cycles_per_sec;
			if (sec >= 1.0f)	c_bIdle = false;
			else	return;
		}

		strncpy((char*)p, c_szNorm, strlen(c_szNorm));
		p += strlen(c_szNorm);
		sum += strlen(c_szNorm);

		int leng;
		GETBAND(pBand);
		leng = pBand->Bale(false, p);
		p += leng;
		sum += leng;

#if	defined(_AT_TS_)
		CLand* pLand = (CLand*)c_pParent;
		leng = pLand->Bale(false, p);
		p += SIZE_RECIP;
		sum += SIZE_RECIP;
#endif

		GETPAPER(pPaper);
		leng = pPaper->Bale(false, p);
		p += leng;
		sum += leng;

		//GETVERSE(pVerse);
		//leng = pVerse->BaleSvcRef(false, p);
		//leng = pVerse->BaleHduRef(false, p);
		// 200610
		//GETARCHIVE(pArch);
		//leng = pArch->Bale(false, p);
		//p += leng;
		//sum += leng;
		GETPROSE(pProse);
		leng = pProse->Bale(false, p);
		p += leng;
		sum += leng;

		GETFSC(pFsc);
		leng = pFsc->Bale(false, p);
		p += leng;
		sum += leng;
	}
	Send((WORD)(sum + 6));
}

void CWicket::Send(WORD wLength)
{
	BYTE buf[10];
	sprintf((char*)buf, "%06d", wLength);
	memcpy(c_mcm.cBuf, buf, 6);

	c_mcm.dwLength = wLength;
	c_mcm.wID = MCMSG_SEND;
	mq_send(c_hMcq, (char*)&c_mcm, sizeof(MCMSG), 0);
}

void CWicket::TakeLinkMc(DWORD dw)
{
	bool bCmd = c_bOpen ? false : true;
	c_bOpen = true;
//#if	_DEBL(DEBUG_INFO)
//	TRACK("%s>MON:link %d.\n", c_szOwnerName, bCmd);
//#endif
	//Send(true);
	Send(bCmd);
	c_runIdle = ClockCycles();
	c_bIdle = true;
}

void CWicket::TakeUnlinkMc(DWORD dw)
{
	c_bOpen = false;
//#if	_DEBL(DEBUG_INFO)
//	TRACK("%s>MON:unlink.\n", c_szOwnerName);
//#endif
}

void CWicket::TakeDebugMc(DWORD dw)
{
	if (c_bOpen) {
		if (dw < PSCCHB_MAX) {
			GETFSC(pFsc);
			pFsc->SetRxChMonit((int)dw);
		}
		//TRACK("%s>MON:debug %d.\n", c_szOwnerName, dw);
	}
}

BEGIN_TAGTABLE(CWicket, CPump)
	_TAGMAP(TAG_LINKMC, CWicket::TakeLinkMc)
	_TAGMAP(TAG_UNLINKMC, CWicket::TakeUnlinkMc)
	_TAGMAP(TAG_DEBUGMC, CWicket::TakeDebugMc)
END_TAGTABLE();

